/*
 * BookmarkLib/WinFavoritesOutput.cxx
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
 *    for the WindowsFavorites class, a subclass
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
#include <shlobj.h>
#include <shellapi.h>

#include "SyncLib/Character.h"
#include "SyncLib/FileOutputStream.h"
#include "SyncLib/BufferedOutputStream.h"
#include "SyncLib/PrintWriter.h"
#include "SyncLib/util.h"

#include "BrowserBookmarks.h"
#include "WinFavorites.h"

using namespace syncit;

static const char INTERNET_SHORTCUT[] = "[InternetShortcut]";
static const char URL_SUFFIX[] = ".url";

void WindowsFavorites::Write(const BookmarkModel *pb, LPCTSTR pszFilename) {
   WinFavoritesOutput out(pszFilename);

   BookmarkModel empty;

   diff(pb, &empty, &out);
}

WinFavoritesOutput::WinFavoritesOutput(LPCTSTR pszDirectory, HWND hwndProgress) {
   m_i = bufcopy(pszDirectory, m_achPath, sizeof(m_achPath));
   if (m_i > 0 && m_achPath[m_i - 1] == '\\') {
      m_i--;
   }

   m_dtInvalid = DateTime::now() - DeltaTime::SECOND;

   m_hwndProgress = hwndProgress;
}

//////////////////
// BookmarkSink...
//
void WinFavoritesOutput::progress() {
   if (m_hwndProgress != NULL) {
      SendMessage(m_hwndProgress, PBM_STEPIT, 0, 0);
   }
}

void WinFavoritesOutput::setName(const tchar_t *pszTitle) {
   pushPath(pszTitle);
}

void WinFavoritesOutput::startBookmark() {
   m_dtModified = m_dtInvalid;
}

void WinFavoritesOutput::setBookmarkHref(const char *pszHref) {
   m_href = pszHref;
}

void WinFavoritesOutput::setBookmarkModified(const DateTime &dt) {
   if (dt.isValid()) { m_dtModified = dt; }
}

void WinFavoritesOutput::endBookmark() {
   writeBookmark(m_href.c_str(), m_href.size(), m_dtModified);

   popFolder();
}

void WinFavoritesOutput::startFolder() {
}

void WinFavoritesOutput::startSubscription() {
   startFolder();
}

void WinFavoritesOutput::pushFolder() {
   if (m_achPath[m_i - 1] == '.') {
      m_achPath[m_i++] = '%';
      m_achPath[m_i] = 0;
   }

   CreateDirectory(m_achPath, NULL);
}

void WinFavoritesOutput::popFolder() {
   do {
      m_i--;
      assert(m_i >= 0);
   } while (m_achPath[m_i] != '\\');

   m_achPath[m_i] = 0;
}


void WinFavoritesOutput::endFolder() {
}

void WinFavoritesOutput::endSubscription() {
   endFolder();
}

void WinFavoritesOutput::undoCurrent() {
}
// ...BookmarkSink
//////////////////

/////////////////////////
// BookmarkDifferences...
//
void WinFavoritesOutput::addBookmark(const Bookmark *p) {
   char ach[4096];
   size_t cch = p->getHref().format(ach, sizeof(ach));
   int i = m_i;

   pushPath(p->getName());
   writeBookmark(ach, cch, p->getModified());

   m_achPath[m_i = i] = 0;
}

/* virtual */
void WinFavoritesOutput::delBookmark(const Bookmark *p) {
   int i = m_i;

   pushPath(p->getName());

   m_i += bufcopy(URL_SUFFIX, m_achPath + m_i, ELEMENTS(m_achPath) - m_i);
   del();

   m_achPath[m_i = i] = 0;
}

/* virtual */
void WinFavoritesOutput::pushFolder(const BookmarkFolder *pbf) {
   pushPath(pbf->getName());

   if (m_achPath[m_i - 1] == '.') {
      m_achPath[m_i++] = '%';
      m_achPath[m_i] = 0;
   }
}

/* virtual */
void WinFavoritesOutput::add0(const BookmarkFolder *p) {
   int i = m_i;

   pushPath(p->getName());

   if (m_achPath[m_i - 1] == '.') {
      m_achPath[m_i++] = '%';
      m_achPath[m_i] = 0;
   }

   CreateDirectory(m_achPath, NULL);
   m_achPath[m_i = i] = 0;
}

/* virtual */
void WinFavoritesOutput::delFolder(const BookmarkFolder *p) {
   int i = m_i;

   pushPath(p->getName());

   if (m_achPath[m_i - 1] == '.') {
      m_achPath[m_i++] = '%';
      m_achPath[m_i] = 0;
   }

   del();

   m_achPath[m_i = i] = 0;
}

/* virtual */
void WinFavoritesOutput::commit() {
}

void WinFavoritesOutput::del() {
   SHFILEOPSTRUCT shfop;

   assert(m_achPath[m_i] == 0);

   m_achPath[m_i + 1] = 0;

   shfop.hwnd = NULL;
   shfop.wFunc = FO_DELETE;
   shfop.pFrom = m_achPath;
   shfop.pTo = NULL;
   shfop.fFlags = FOF_ALLOWUNDO |
                  FOF_NOCONFIRMATION |
                //FOF_NOERRORUI |
                  FOF_SILENT;
   shfop.hNameMappings = NULL;
   shfop.lpszProgressTitle = NULL;

   SHFileOperation(&shfop);
}
// ...BookmarkDifferences
/////////////////////////

void WinFavoritesOutput::pushPath(const tchar_t *pszTitle) {
   m_achPath[m_i++] = '\\';

   if (pszTitle[0] == 0) {
      m_achPath[m_i++] = '%';
      m_achPath[m_i] = 0;
   }
   else {
      m_i += Encode(pszTitle, '%', WindowsFavorites::m_gachMap, ELEMENTS(WindowsFavorites::m_gachMap), m_achPath + m_i, sizeof(m_achPath) - m_i);
   }
}

void WinFavoritesOutput::writeBookmark(const char *pszUrl, size_t cchUrl, const DateTime &dt) {
   bufcopy(URL_SUFFIX, 4, m_achPath + m_i, sizeof(m_achPath) - m_i);

   FileOutputStream f;

   try {
      f.create0(m_achPath);
      BufferedOutputStream b(&f);
      PrintWriter p(&b);

      p.print(WindowsFavorites::m_gszInternetShortcut);
      p.print("\r\n"
              "URL=");
      p.write(pszUrl, cchUrl);
      p.print("\r\n");

      if (dt.isValid()) {
         FILETIME ft = dt.getFileTime();
         int j;
         unsigned char *pb = (unsigned char *) &ft;
         unsigned char total = 0;
         p.print("Modified=");

         for (j = 0; j < 8; j++) {
            p.printHex(pb[j]);
            total += pb[j];
         }

         p.printHex(total);
         p.print("\r\n");
      }

      p.close();
   }
   catch (BaseError &) {
   }
}
