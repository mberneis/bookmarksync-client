/*
* SyncIT/MozillaBrowser.h
* Copyright (c) 1999, SyncIt.com  Inc.
* Copyright (c) 2003, Daniel Gehriger <gehriger@linkcad.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
* -----------------
* This program is GPL'd.  If you distribute this program or a derivative of
* this program publicly you must include the source code.  It is easy
* enough to drop me an email requesting a different license, if necessary.
*
* Authors:           Terence Way, 
*                    Daniel Gehriger
* Last Modification: 21 October 2003
*
* Description:
*
*   MozillaBrowser class: implements Mozilla-derived browsers, 
*   such as Netscape 6/7, Firebird, Mozilla.
*
*/
#pragma once

#include "Browser.h"
#include <map>
#include <list>
#include <ddeml.h>

namespace syncit {

class MozillaBrowser : public Browser
{
public:
    // constructor
    MozillaBrowser(const tchar_t* ddeServerName, const tchar_t* shortName, const tchar_t* appDataFolder);

    // destructor
    virtual ~MozillaBrowser();

    // intializes browser, returns true if successful
    virtual bool        initialize();

    // returns true if initialized
    virtual bool        isInitialized() const;

    // return handle used for detecting changes in bookmarks
    virtual HANDLE      FindFirstChangeNotification();

    // get bookmarks directory path
    virtual tstring     getDirectory() const;

    // serialize bookmarks
    virtual bool        readBookmarks(BookmarkSink* pbs, bool fForce);

    // deserialize bookmarks
    virtual void        writeBookmarks(const BookmarkModel* newBookmarks, const char* backupFilename);

protected:
    // parse user preferences file
    virtual bool        parsePrefs(const tstring& prefsPath, const tstring& name, std::string& value) const;

    // locate the bookmarks file
    virtual bool        locateBookmarksFile(tstring& profileDirectory, tstring& bookmarksPath) const;

    // check if this (or any browser sharing same bookmarks file) is running
    bool                isBrowserRunning() const;

private:
    // default constructor (disabled)
    MozillaBrowser();

    // copy constructor (disabled)
    MozillaBrowser(const MozillaBrowser&);

    // assignment operator (disabled)
    MozillaBrowser& operator=(const MozillaBrowser&);

    // get short version of bookmarks path
    tstring             getShortBookmarksPath() const;

    // initialize the DDE communication
    void                initDde(const tchar_t* ddeServerName);

    // static DDE callback function
    static HDDEDATA CALLBACK ddeCallbackStub(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD);

    // instance specific DDE callback function
    HDDEDATA            ddeCallback(UINT, HSZ);

protected:
    FILETIME            m_lastWriteTime;    // time stamp when bookmarks file last written
    tstring             m_bookmarksPath;    // path to bookmarks.html file

private:
    // maps DDE IDs to instances of MozillaBrowser objects
    typedef std::map<HSZ, MozillaBrowser*> InstanceMap;

    // list of browsers sharing same bookmarks file
    typedef std::list<MozillaBrowser*> SharingBrowsers;

    // map of bookmark file name to browser list
    typedef std::map<tstring, SharingBrowsers> BrowserMap;

    static BrowserMap   s_browserMap;       // maps bookmark file paths to sharing browsers
    static InstanceMap  s_instanceMap;      // maps DDE callbacks to browser instances
    tstring             m_profileDirectory; // directory containing user-specific information
    tstring             m_appDataFolder_;   // subfolder of %APPDATA% with profile
    DWORD               m_ddeId;            // DDE Instance ID
    HSZ                 m_serverHandle;     // server string handle
    bool                m_browserRunning;   // true if browser is running
};

} // namespace syncit
