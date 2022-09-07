/*
* SyncIT/OperaBrowser.h
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
*   OperaBrowser class: implements the Opera browser.
*
*/
#pragma once

#include "Browser.h"

namespace syncit {

class OperaBrowser : public Browser 
{
public:
    OperaBrowser();

    virtual ~OperaBrowser();

    virtual bool        initialize() /* throws Error */;

    tstring             getDirectory() const;

    virtual HANDLE      FindFirstChangeNotification();

    virtual bool        readBookmarks(BookmarkSink *pbs, bool fForce);
    virtual void        writeBookmarks(const BookmarkModel *pbfNew, const char *pszBackupFilename);

private:
    LPTSTR              m_pszDirectory;  // directory containing user-specific information
    LPTSTR              m_pszBookmarks;

    // disable copy constructor and assignment
    OperaBrowser(const OperaBrowser &rhs);
    OperaBrowser &operator=(const OperaBrowser &rhs);
};

} // namespace syncit
