/*
 * SyncIT/Browser.h
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
 * Author:            Terence Way
 * Modified by:       Daniel Gehriger
 * Last Modification: 15 October 2003
 *
 * Description:
 *
 *    This file declares the Browser base class.
 */
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */
#include <windows.h>

#include "BookmarkLib/BookmarkModel.h"

#include "SyncLib/BufferedInputStream.h"
#include "SyncLib/util.h"
#include "SyncLib/Regkey.h"

namespace syncit {

/**
 * Browser is the abstract class that represents
 * the bookmarks kept in a browser file or directory.
 * Classes that inherit from this handle the different
 * way bookmarks are stored in each different browser.
 * <p>
 * This class keeps track of of two BookmarkFolder classes:
 * the current set (last set that was read successfully) and
 * the new set (currently being read, or currently being sent
 * to the server).
 */
class Browser 
{
public:
    // destructor
    virtual ~Browser();

    // get the in-memory bookmark tree
    BookmarkModel*      getBackup();

    // replace in-memory bookmark tree
    void                setBackup(BookmarkModel* pb);

    /**
    * Check to see if the browser has just been recently installed
    * or has moved to a new (possibly empty) bookmark folder.
    *
    * @param hkey  a handle to the HKEY_CURRENT_USER\Software\SyncIT registry
    *              key
    * @return true if there is no saved directory in the registry, or if the
    *         current directory is different than the saved directory.
    */
    bool                hasDirectoryChanged(RegKey &key);

    // get the short browser name
    tstring             getShortName() const;

public:
    /**
    * Initialize the browser information.  Returns
    * whether the browser is installed.
    *
    * @return true if the browser is installed and initialization
    *          succeeded, false if the browser isn't installed
    * @exception Error on some IO error loading browser information
    */
    virtual bool        initialize() = 0;

    /**
    * Call the Win32 FindFirstChangeNotification
    * system routine on the directory/files that
    * the browser uses for the bookmarks.
    * <p>
    * Each browser subclass may want to handle this
    * differently: for instance Netscape is a single file
    * in a single directory (no subtrees) while Microsoft
    * has a directory structure.
    *
    * @return a HANDLE returned from ::FindFirstChangeNotification
    *
    * @see FindNextChangeNotification
    * @see FindCloseChangeNotification
    */
    virtual HANDLE      FindFirstChangeNotification() = 0;

    /**
    * Read the browser bookmarks into a BookmarkFolder.
    * This folder is the "top-level" bookmark folder, so
    * all bookmarks that are immediately visible to the browser
    * user should go into the top-level of the folder.
    */
    virtual bool        readBookmarks(BookmarkSink *pbs, bool fForce) = 0;

    /**
    * Write the browser bookmarks.
    *
    * The bookmarks to be written are pointed to by <i>pbfNew</i>,
    * <i>pbfOld</i> is just specified if (like IE) bookmarks are
    * stored as individual files and a delta is needed...
    */
    virtual void        writeBookmarks(const BookmarkModel* pbfNew, const char* pszBackupFilename) = 0;

    // get bookmarks directory path
    virtual tstring     getDirectory() const = 0;

    // return true if initialized
    virtual bool        isInitialized() const;
   
protected:
    // constructor for derived clases
    Browser(const tchar_t* shortName);

private:
    // default constructor (disabled)
    Browser();

    // copy constructor (disabled)
    Browser(const Browser&);

    // assignment operator (disabled)
    Browser& operator=(const Browser&);

protected:
   BookmarkModel*       m_bookmarks;
   tstring              m_directoryKey;
   tstring              m_shortName;
};

} // namespace syncit
