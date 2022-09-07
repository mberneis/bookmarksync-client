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
 *    This file implements the Browser base class.
 */
#include "Browser.h"

namespace syncit {

//------------------------------------------------------------------------------
Browser::Browser(const tchar_t* shortName) :
    m_shortName(shortName),
    m_bookmarks(NULL)
{
    m_directoryKey = m_shortName + "Dir";
    m_directoryKey[0] = toupper(m_directoryKey[0]);
}

//------------------------------------------------------------------------------
Browser::~Browser()
{
    if (m_bookmarks)
    {
        BookmarkModel::Detach(m_bookmarks);
    }
}

//------------------------------------------------------------------------------
BookmarkModel* Browser::getBackup() 
{
    return m_bookmarks;
}

//------------------------------------------------------------------------------
void Browser::setBackup(BookmarkModel* pb)
{
    BookmarkModel::Detach(m_bookmarks);
    m_bookmarks = pb;
}

//------------------------------------------------------------------------------
bool Browser::isInitialized() const
{
    return !getDirectory().empty();
}

//------------------------------------------------------------------------------
bool Browser::hasDirectoryChanged(RegKey& key)
{
    bool fResult;

    tstring newDirectory = getDirectory();
    if (newDirectory.empty()) 
    {
        key.deleteValue(m_directoryKey.c_str());
        fResult = false;
    }
    else 
    {
        char oldDirectory[MAX_PATH];
        if (key.queryValue(m_directoryKey.c_str(), oldDirectory, sizeof(oldDirectory)) == 0)
        {
            fResult = true;
        }
        else 
        {
            fResult = tstricmp(oldDirectory, newDirectory.c_str()) != 0;
        }

        key.setValue(m_directoryKey.c_str(), newDirectory.c_str());
    }

    return fResult;
}

//------------------------------------------------------------------------------
tstring Browser::getShortName() const
{
    return m_shortName;
}


} // namespace syncit
