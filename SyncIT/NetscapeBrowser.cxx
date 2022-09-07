/*
* SyncIT/NetscapeBrowser.cxx
* Copyright(c) 1999, SyncIt.com  Inc.
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
* Last Modification: 24 October 2003
*
* Description:
*
*   NetscapeBrowser class: implements the Netscape 2 - 4 browsers.
*
*/
#include "NetscapeBrowser.h"
#include "BuiltinImages.h"

#include "SyncLib/FileInputStream.h"
#include "SyncLib/NetscapeInfo.h"

#include "BookmarkLib/NetscapeBookmarks.h"
#include "BookmarkLib/BookmarkEditor.h"

namespace syncit {

//------------------------------------------------------------------------------
// Constructor for derived classes
//
// Parameters:  ddeServerName   - name of DDE server
//              keyName         - a name for the directory key in the registry
//------------------------------------------------------------------------------
NetscapeBrowser::NetscapeBrowser() :
    MozillaBrowser(TEXT(""), TEXT("NETSCAPE"), TEXT("ns"))
{
}

//------------------------------------------------------------------------------
NetscapeBrowser::~NetscapeBrowser()
{
}

//------------------------------------------------------------------------------
bool NetscapeBrowser::locateBookmarksFile(tstring& profileDirectory, 
                                          tstring& bookmarksPath) const
{
    char achBuf[MAX_PATH];
    if (   GetNetscapeV45Info(achBuf, sizeof(achBuf)) 
        || GetNetscapeV40Info(achBuf, sizeof(achBuf))) 
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
            bookmarksPath = profileDirectory + T("\\BOOKMARK.HTM"); 
        }
        return true;
    }
    else if (GetNetscapeV20Info(achBuf, sizeof(achBuf))) 
    {
        bookmarksPath = achBuf;

        tstring::size_type len = bookmarksPath.rfind('\\');
        if (len != tstring::npos)
        {
            profileDirectory = bookmarksPath.substr(0, len);
        }
        else
        {
            profileDirectory = bookmarksPath;
        }

        return true;
    }
    else
    {
        return false;
    }
}


//------------------------------------------------------------------------------
bool NetscapeBrowser::readBookmarks(BookmarkSink* pbs, bool fForce) 
{
    FileInputStream f;
    if (f.open(m_bookmarksPath.c_str())) 
    {
        FILETIME ft;
        GetFileTime(f.getHandle(), NULL, NULL, &ft);

        if (fForce || CompareFileTime(&ft, &m_lastWriteTime) != 0)
        {
            m_lastWriteTime = ft;
            return NetscapeBookmarks::Read(BufferedInputStream(&f), pbs);
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void NetscapeBrowser::writeBookmarks(const BookmarkModel* newBookmarks,
                                     const char* backupFilename) 
{

    if (!isBrowserRunning()) 
    {
        BookmarkModel netscape;
        BookmarkContext bc(&netscape, gpLoader);

        if (NetscapeBookmarks::Read(m_bookmarksPath.c_str(), &bc)) 
        {
            // merge new and old bookmarks
            BookmarkEditor e(&netscape);
            if (diff(newBookmarks, m_bookmarks, &e) > 0) 
            {
                NetscapeBookmarks::Write(&netscape, m_bookmarksPath.c_str());
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
            NetscapeBookmarks::Write(newBookmarks, m_bookmarksPath.c_str());

            // write backup file
            XBELBookmarks::Write(newBookmarks, backupFilename);

            // replace current in-memory bookmarks with new bookmarks
            BookmarkModel::Detach(m_bookmarks);
            m_bookmarks = NEW BookmarkModel(*newBookmarks);
        }
    }
}

} // namespace syncit

