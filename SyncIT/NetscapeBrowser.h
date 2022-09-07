/*
* SyncIT/NetscapeBrowser.h
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
* Last Modification: 24 October 2003
*
* Description:
*
*   NetscapeBrowser class: implements the Netscape 2 - 4 browsers.
*
*/
#pragma once

#include "MozillaBrowser.h"
#include <map>

namespace syncit {

class NetscapeBrowser : public MozillaBrowser
{
public:
    // constructor
    NetscapeBrowser();

    // destructor
    virtual ~NetscapeBrowser();

    // serialize bookmarks
    virtual bool        readBookmarks(BookmarkSink* pbs, bool fForce);

    // deserialize bookmarks
    virtual void        writeBookmarks(const BookmarkModel* newBookmarks, const char* backupFilename);

protected:
    // locate the bookmarks file
    virtual bool        locateBookmarksFile(tstring& profileDirectory, tstring& bookmarksPath) const;

private:
    // copy constructor (disabled)
    NetscapeBrowser(const NetscapeBrowser&);

    // assignment operator (disabled)
    NetscapeBrowser& operator=(const NetscapeBrowser&);
};

} // namespace syncit
