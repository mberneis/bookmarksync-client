/*
 * SyncIT/MicrosoftBrowser.cxx
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
 * Last Modification: 2 Jan 1999
 *
 * Description:
 *    This module contains the definitions to methods
 *    for the MicrosoftBrowser class, a subclass
 *    of the Browser class.
 *
 *    The three basic things this module defines:
 *    1. FindFirstChangeNotification, to open a handle
 *       that is signalled when changes to the Microsoft
 *       Favorites directory are made;
 *    2. Get proxy information that IE uses when
 *       connecting to an HTTP: URL.
 *    3. Read the favorites directory into a BookmarkFolder
 *       object.
 *    4. Apply changes to the favorites directory based on
 *       a BookmarkFolder object.
 *
 * Steps 1, 3, and 4 require the directory that the user's
 * favorites are stored in.  This is kept in the registry key:
 * hkey:      HKEY_CURRENT_USER\
 * pszSubKey: Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders
 * pszValue:  Favorites
 *
 * If there is no key, then the favorites directory is assumed to be
 * the system directory with "Favorites" tacked on the end, for instance:
 *    c:\windows\favorites
 *
 * Each "bookmark" is a separate file with a '.URL' extension within this
 * directory.  Bookmark folders are subdirectories.  The name of the
 * file/directory is the name of the bookmark/folder.
 *
 * Each bookmark '.URL' file has the following format:
 * <pre>
 * [InternetShortcut]
 * URL=...
 * Modified=...
 * </pre>
 *
 * The URL= tag is followed by the text URL, the Modified= tag is followed
 * by an 18-digit hexadecimal number, the first sixteen digits is the 
 * binary of the FILETIME with the digits reversed, the last two digits are
 * a checksum.  For instance, the date:
 *    24 Sep 1998 18:52:59.260 GMT (2:52:59.260 EDT)
 * translates to a FILETIME of:
 *    01BDE7EC8C8D23C0
 * reversing the digits and appending a checksum leads to:
 *    C0238D8CECE7BD018D
 */
#pragma warning( disable : 4786 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>

#include "SyncLib/Errors.h"
#include "SyncLib/FileInputStream.h"
#include "SyncLib/FileOutputStream.h"
#include "SyncLib/BufferedInputStream.h"
#include "SyncLib/BufferedOutputStream.h"
#include "SyncLib/PrintWriter.h"
#include "SyncLib/util.h"

#include "MicrosoftBrowser.h"
#include "resource.h"
#include "BuiltinImages.h"

#include "BookmarkLib/BrowserBookmarks.h"
#include "BookmarkLib/WinFavorites.h"

namespace syncit {

namespace {

bool ReadMicrosoftFavorites(LPCTSTR pszDirectory, BookmarkFolder *pDirectory);

bool ReadBookmarkUrl(LPCTSTR pszFilename, char *pachUrl, size_t cchUrl);

void ConvertToByteArray(const char *psz, unsigned char *pb, size_t cb);

static const TCHAR FAVORITES[] = TEXT("Favorites");
static const TCHAR MicrosoftFolders[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
static const TCHAR INTERNET_SETTINGS[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");

static const tchar_t gachMap[] =
   "\"'"
   "*+"
   "/-"
   "\\_"
   ":;"
   "<{"
   ">}"
   "?~"
   "|!"
   "%%";

} // namespace

//------------------------------------------------------------------------------
MicrosoftBrowser::MicrosoftBrowser() : Browser("ie") 
{
   m_pszDirectory = NULL;
}

//------------------------------------------------------------------------------
MicrosoftBrowser::~MicrosoftBrowser() {
   u_free0(m_pszDirectory);
}

//------------------------------------------------------------------------------
tstring MicrosoftBrowser::getDirectory() const {
   return m_pszDirectory;
}

//------------------------------------------------------------------------------
HANDLE MicrosoftBrowser::FindFirstChangeNotification() {
   HANDLE h;

   h = ::FindFirstChangeNotification(m_pszDirectory,
                                     TRUE,                          // fSubDir
                                     FILE_NOTIFY_CHANGE_LAST_WRITE
                                   | FILE_NOTIFY_CHANGE_FILE_NAME
                                   | FILE_NOTIFY_CHANGE_DIR_NAME);  // dwFlags

   return h;
}

//------------------------------------------------------------------------------
bool MicrosoftBrowser::readBookmarks(BookmarkSink *pbs, bool fForce) {
   return WindowsFavorites::Read(m_pszDirectory, pbs);
}


//------------------------------------------------------------------------------
void MicrosoftBrowser::writeBookmarks(const BookmarkModel *pbfNew,
                                      const char *pszBackupFilename) {
   WinFavoritesOutput writer(m_pszDirectory);

   diff(pbfNew, m_bookmarks, &writer);

   XBELBookmarks::Write(pbfNew, pszBackupFilename);

   BookmarkObject::Detach(m_bookmarks);
   m_bookmarks = NEW BookmarkModel(*pbfNew);
}

//------------------------------------------------------------------------------
bool MicrosoftBrowser::initialize() /* throws Error */ {
   m_pszDirectory = GetFavoritesDirectory();
   return m_pszDirectory != NULL;
}

/**
* Microsoft Internet Explorer keeps favorites in
* a directory identified by the registry key
*
* HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Favorites
*/
LPTSTR MicrosoftBrowser::GetFavoritesDirectory() {
   char achBuf[MAX_PATH];
   RegKey key;

   if (key.open(HKEY_CURRENT_USER, MicrosoftFolders) &&
       key.queryValue("Favorites", achBuf, ELEMENTS(achBuf)) > 0) {
      return stralloc(achBuf);
   }
   else {
      UINT  u;

      // Look for Internet Explorer Favorites...
      //
      u = GetWindowsDirectory(achBuf,             // LPTSTR lpBuffer
                              ELEMENTS(achBuf));  // UINT   uSize

      if (u == 0) {
         throw Win32Error("GetWindowsDirectory");
      }

      return Combine(achBuf, FAVORITES);
   }
}

} // namespace syncit
