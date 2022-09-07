/*
* SyncIT/MozillaBrowser.h
* Copyright(c) 1999, SyncIt.com  Inc.
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
* Author:            Daniel Gehriger
* Last Modification: 21 October 2003
*
* Description:
*
*   MozillaBrowser class: common base for all Mozilla-type browsers, 
*   such as Netscape 6/7, Firebird, Mozilla.
*
*/
#include "MozillaBrowser.h"
#include "BuiltinImages.h"

#include "SyncLib/FileInputStream.h"
#include "SyncLib/NetscapeInfo.h"

#include "BookmarkLib/BookmarkModel.h"
#include "BookmarkLib/BookmarkEditor.h"
#include "BookmarkLib/BrowserBookmarks.h"
#include "BookmarkLib/MozillaBookmarks.h"

namespace syncit {

//------------------------------------------------------------------------------
// Static instance of the browser map
//------------------------------------------------------------------------------
MozillaBrowser::InstanceMap MozillaBrowser::s_instanceMap;

//------------------------------------------------------------------------------
// Static instance of the browser map
//------------------------------------------------------------------------------
MozillaBrowser::BrowserMap MozillaBrowser::s_browserMap;

//------------------------------------------------------------------------------
// Constructor for derived classes
//
// Parameters:  ddeServerName   - name of DDE server
//              keyName         - a name for the directory key in the registry
//------------------------------------------------------------------------------
MozillaBrowser::MozillaBrowser(const tchar_t* ddeServerName, 
                               const tchar_t* shortName,
                               const tchar_t* appDataFolder) :
    Browser(shortName),
    m_browserRunning(false),
    m_ddeId(0),
    m_serverHandle(NULL),
    m_appDataFolder_(appDataFolder)
{
    // append trailing slash to app data folder
    if (!m_appDataFolder_.empty() && m_appDataFolder_[m_appDataFolder_.length()-1] != '\\')
    {
        m_appDataFolder_ += '\\';
    }

    m_lastWriteTime.dwLowDateTime = m_lastWriteTime.dwHighDateTime = 0;
    initDde(ddeServerName);
}

//------------------------------------------------------------------------------
MozillaBrowser::~MozillaBrowser()
{
    if (m_ddeId != 0)
    {
        DdeFreeStringHandle(m_ddeId, m_serverHandle);
        DdeUninitialize(m_ddeId);
    }

    s_instanceMap.erase(m_serverHandle);

    // remove from list of sharing browsers
    BrowserMap::iterator it = s_browserMap.find(getShortBookmarksPath());
    if (it != s_browserMap.end())
    {
        SharingBrowsers& browsers = it->second;
        browsers.remove(this);

        if (browsers.empty())
        {
            s_browserMap.erase(it);
        }
    }
}

//------------------------------------------------------------------------------
tstring MozillaBrowser::getShortBookmarksPath() const
{
    TCHAR buf[MAX_PATH];
    DWORD len = GetShortPathName(m_bookmarksPath.c_str(), buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH)
        return buf;
    else
        return m_bookmarksPath;
}

//------------------------------------------------------------------------------
bool MozillaBrowser::initialize() 
{
    if (!locateBookmarksFile(m_profileDirectory, m_bookmarksPath))
        return false;

    // check if bookmarks file shared
    BrowserMap::iterator it = s_browserMap.find(getShortBookmarksPath());
    if (it != s_browserMap.end() && !it->second.empty())
    {
        // another browser already takes care of this bookmarks file...
        // add to list of sharing browsers, but return false
        it->second.push_back(this);
        return false;
    }
    else
    {
        // register with browser map
        s_browserMap[getShortBookmarksPath()].push_back(this);
    }

    // create a safe copy (don't overwrite the previously saved version)
    tstring save = m_bookmarksPath + ".sav";
    CopyFile(m_bookmarksPath.c_str(), save.c_str(), TRUE);

    // create a backup copy
    tstring backup = m_bookmarksPath + ".bck";
    if (!CopyFile(m_bookmarksPath.c_str(), backup.c_str(), FALSE)) 
    {
        m_profileDirectory.erase();
        m_bookmarksPath.erase();
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
bool MozillaBrowser::isInitialized() const
{
    return !m_profileDirectory.empty() && !m_bookmarksPath.empty();
}

//------------------------------------------------------------------------------
HANDLE MozillaBrowser::FindFirstChangeNotification() 
{
    tstring bookmarksDir(m_bookmarksPath, 0, m_bookmarksPath.rfind('\\'));
    return ::FindFirstChangeNotification(bookmarksDir.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
}

//------------------------------------------------------------------------------
tstring MozillaBrowser::getDirectory() const
{
    return m_profileDirectory;
}

//------------------------------------------------------------------------------
bool MozillaBrowser::isBrowserRunning() const
{
    // check if any of the sharing browsers is running
    BrowserMap::iterator it = s_browserMap.find(getShortBookmarksPath());
    if (it != s_browserMap.end())
    {
        SharingBrowsers& browsers = it->second;

        for (SharingBrowsers::const_iterator it = browsers.begin(); it != browsers.end(); ++it)
        {
            if ((*it)->m_browserRunning)
                return true;
        }

        return false;
    }

    assert(false);
    return false;
}

//------------------------------------------------------------------------------
bool MozillaBrowser::readBookmarks(BookmarkSink* pbs, bool fForce) 
{
    FileInputStream f;
    if (f.open(m_bookmarksPath.c_str())) 
    {
        FILETIME ft;
        GetFileTime(f.getHandle(), NULL, NULL, &ft);

        if (fForce || CompareFileTime(&ft, &m_lastWriteTime) != 0)
        {
            m_lastWriteTime = ft;
            return MozillaBookmarks::Read(BufferedInputStream(&f), pbs);
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void MozillaBrowser::writeBookmarks(const BookmarkModel* newBookmarks,
                                    const char* backupFilename) 
{
    if (!isBrowserRunning()) 
    {
        BookmarkModel netscape;
        BookmarkContext bc(&netscape, gpLoader);

        if (MozillaBookmarks::Read(m_bookmarksPath.c_str(), &bc)) 
        {
            // merge new and old bookmarks
            BookmarkEditor e(&netscape);
            if (diff(newBookmarks, m_bookmarks, &e) > 0) 
            {
                MozillaBookmarks::Write(&netscape, m_bookmarksPath.c_str());
            }

            // write backup file
            XBELBookmarks::Write(&netscape, backupFilename);

            // replace current in-memory bookmarks with new bookmarks
            BookmarkModel::Detach(m_bookmarks);
            m_bookmarks = NEW BookmarkModel(netscape);
        }
        else
        {
            // write new bookmarks
            MozillaBookmarks::Write(newBookmarks, m_bookmarksPath.c_str());

            // write backup file
            XBELBookmarks::Write(newBookmarks, backupFilename);

            // replace current in-memory bookmarks with new bookmarks
            BookmarkModel::Detach(m_bookmarks);
            m_bookmarks = NEW BookmarkModel(*newBookmarks);
        }
    }
}

//------------------------------------------------------------------------------
void MozillaBrowser::initDde(const tchar_t* serverName) 
{
    m_ddeId = 0;

    UINT u = DdeInitialize(&m_ddeId, ddeCallbackStub, APPCLASS_STANDARD | APPCMD_CLIENTONLY | CBF_FAIL_ALLSVRXACTIONS, 0);
    if (u == DMLERR_NO_ERROR)
    {
        m_serverHandle = DdeCreateStringHandle(m_ddeId, serverName, CP_WINANSI);
        HSZ hszWWWVersion = DdeCreateStringHandle(m_ddeId, TEXT("WWW_Version"), CP_WINANSI);
        HCONV hconv = DdeConnect(m_ddeId, m_serverHandle, hszWWWVersion, NULL);

        // register instance in map
        s_instanceMap[m_serverHandle] = this;

        if (hconv == NULL) 
        {
            m_browserRunning = false;
        }
        else 
        {
            DdeDisconnect(hconv);
            m_browserRunning = true;
        }

        DdeFreeStringHandle(m_ddeId, hszWWWVersion);
    }
}

//------------------------------------------------------------------------------
HDDEDATA CALLBACK MozillaBrowser::ddeCallbackStub(UINT uType, UINT, HCONV, HSZ hsz1, HSZ, HDDEDATA, DWORD, DWORD) 
{
    // note: map is accessed read-only, no mutex necessary
    InstanceMap::const_iterator it = s_instanceMap.find(hsz1);
    if (it != s_instanceMap.end())
    {
        return reinterpret_cast<MozillaBrowser*>(it->second)->ddeCallback(uType, hsz1);
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------
HDDEDATA MozillaBrowser::ddeCallback(UINT type, HSZ hsz1) 
{
    if (hsz1 == m_serverHandle)
    {
        switch (type)
        {
        case XTYP_REGISTER:     m_browserRunning = true; break;
        case XTYP_UNREGISTER:   m_browserRunning = false; break;
        }
    }

    return 0;
}

//------------------------------------------------------------------------------
bool MozillaBrowser::locateBookmarksFile(tstring& profileDirectory, 
                                         tstring& bookmarksPath) const
{
    char achBuf[MAX_PATH];
    if (GetMozillaInfo(m_appDataFolder_.c_str(), achBuf, sizeof(achBuf))) 
    {
        profileDirectory = achBuf;

        // check if bookmarks file set by preferences
        tstring value;
        if (parsePrefs(profileDirectory + T("\\user.js"), T("browser.bookmarks.file"), value))
        {
            bookmarksPath = value;
        }
        else if (parsePrefs(profileDirectory + T("\\prefs.js"), T("browser.bookmarks.file"), value))
        {
            bookmarksPath = value;
        }
        else
        {
            // use default
            bookmarksPath = profileDirectory + T("\\BOOKMARKS.HTML"); 
        }
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// parse preferences file
//------------------------------------------------------------------------------
bool MozillaBrowser::parsePrefs(const tstring& prefsPath, 
                                    const tstring& name, 
                                    std::string& value) const
{
    static const char userPref[] = "user_pref";

    if (name.empty())
        return false;

    bool success = false;
    FileInputStream f;
    if (f.open(prefsPath.c_str())) 
    {
        BufferedInputStream in(&f);

        char buf[4096];
        while (in.readLine(buf, sizeof(buf))) 
        {
            const char* p = buf;

            // skip whitespace
            while (*p && isspace(*p)) { ++p; }

            // check for user_pref setting
            if (strncmp(p, userPref, strlen(userPref)) != 0)
                continue;
            p += strlen(userPref);

            // skip whitespace
            while (*p && isspace(*p)) { ++p; }

            // open bracket
            if (*p != '(')
                continue;
            ++p;

            // skip whitespace
            while (*p && isspace(*p)) { ++p; }

            // quotes
            if (*p != '"')
                continue;
            ++p;

            if (strncmp(p, name.c_str(), name.length()) != 0)
                continue;
            p += name.length();

            // quotes
            if (*p != '"')
                continue;
            ++p;

            // skip whitespace
            while (*p && isspace(*p)) { ++p; }

            // comma
            if (*p != ',')
                continue;
            ++p;

            // skip whitespace
            while (*p && isspace(*p)) { ++p; }

            if (*p == '"')
            {
                // text value
                value.erase();
                const char* e = ++p;
                while (*e && *e != '"') 
                { 
                    if (*e != '\\')
                    {
                        value += *e;
                    }
                    else if (*++e)                        
                    {
                        value += *e;
                    }
                    else
                    {
                        break;
                    }
                    ++e; 
                }
                success = true;
                break;
            }
            else
            {
                // other value
                const char* e = ++p;
                while (*e && !isspace(*e) && *e != ')') { ++e; }
                value.assign(p, e - p);
                success = true;
                break;
            }
        }

        f.close();
    }

    return success;
}


} // namespace syncit
