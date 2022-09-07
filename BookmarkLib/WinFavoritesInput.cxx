/*
 * BookmarkLib/WinFavoritesInput.cxx
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
#include "SyncLib/FileInputStream.h"
#include "SyncLib/BufferedInputStream.h"
#include "SyncLib/RegKey.h"
#include "SyncLib/util.h"

#include "BrowserBookmarks.h"
#include "WinFavorites.h"

using namespace syncit;

static bool recurse(TCHAR *pach, size_t i, BookmarkSink *pbs);
static bool ReadBookmarkUrl(LPCTSTR pszFilename, BookmarkSink *pbs);
static int hex(const char **p);

bool WindowsFavorites::Read(LPCTSTR pszDirectory, BookmarkSink *pbs) {
   TCHAR achDirectory[MAX_PATH];

   TCHAR *pp;
   size_t cch = GetFullPathName(pszDirectory,
                                sizeof(achDirectory),
                                achDirectory,
                                &pp);

   if (cch == 0) {
      throw Win32Error("GetFullPathName");
   }

   if (achDirectory[cch - 1] != TEXT('\\')) {
      achDirectory[cch++] = TEXT('\\');
      achDirectory[cch] = 0;
   }

   return recurse(achDirectory, cch, pbs);
}

static void unpack(const WIN32_FIND_DATA *pfd,
                   TCHAR *pachBuffer,
                   DWORD *pdwAttributes,
                   DateTime *pdtModified,
                   unsigned long *pSize) {
   lstrcpy(pachBuffer, pfd->cFileName);
   *pdwAttributes = pfd->dwFileAttributes;
   pdtModified->setFileTime(pfd->ftLastWriteTime);
   *pSize = pfd->nFileSizeLow;
}

static HANDLE findFirst(LPCTSTR pszSearchPath,
                        TCHAR *pachBuffer,
                        DWORD *pdwAttributes,
                        DateTime *pdtModified,
                        unsigned long *pSize) {
   WIN32_FIND_DATA wfd;

   HANDLE h = FindFirstFile(pszSearchPath, &wfd);
   if (h != INVALID_HANDLE_VALUE) {
      unpack(&wfd, pachBuffer, pdwAttributes, pdtModified, pSize);
   }

   return h;
}

static bool findNext(HANDLE h,
                     TCHAR *pachBuffer,
                     DWORD *pdwAttributes,
                     DateTime *pdtModified,
                     unsigned long *pSize) {
   WIN32_FIND_DATA wfd;

   if (FindNextFile(h, &wfd)) {
      unpack(&wfd, pachBuffer, pdwAttributes, pdtModified, pSize);
      return true;
   }
   else {
      return false;
   }
}

/**
 * @param pach -- tricky, this.  pointer to buffer[MAX_PATH] of characters
 *                the first part (up to i, next parameter) is filled with a
 *                directory path terminated by '\\'.  The rest (from i to
 *                MAX_PATH) will be used as temporary buffer space.
 * @param i index past end of directory path name specified within pac
 * @param pbs   BookmarkSink to receive information about new bookmarks
 */
bool recurse(TCHAR *pach, size_t i, BookmarkSink *pbs) {
   DWORD dwAttributes;
   DateTime dt;
   unsigned long size;

   int retries = 3;
   unsigned long ulTimeout = 0;

   // pszDirectory[0..i] is now a path with terminating \
   //
   lstrcpy(pach + i, TEXT("*.*"));

   HANDLE h = findFirst(pach, pach + i, &dwAttributes, &dt, &size);

   while (h == INVALID_HANDLE_VALUE) {

      // even if no files in the directory, there is still '.' and '..'
      //
      // error
      DWORD dwError = GetLastError();

      if (dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_PATH_NOT_FOUND) {
         return false;
      }

      if (retries == 0 || (dwError != ERROR_SHARING_VIOLATION && dwError != ERROR_LOCK_VIOLATION)) {
         throw FileError(FileError::Access, pach, Win32Error("FindFirstFile", dwError));
      }

      retries--;
      ::Sleep(ulTimeout);
      ulTimeout = ulTimeout * 2 + 250;

      h = findFirst(pach, pach + i, &dwAttributes, &dt, &size);
   }

   pbs->pushFolder();

   do {
      size_t cchFile = lstrlen(pach + i);

      // directory or not
      //
      if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
         if (pach[i] != '.' && lstrcmpi(pach + i + cchFile - 4, WindowsFavorites::m_gszSuffix) == 0) {
            tchar_t achName[MAX_PATH];

            if (pach[i] == '%' && cchFile == 5) {
               // %.url
               achName[0] = 0;
            }
            else {
               Decode(pach + i, cchFile - 4, '%', WindowsFavorites::m_gachMap, ELEMENTS(WindowsFavorites::m_gachMap), achName, ELEMENTS(achName));
            }

            // regular file
            //
            pbs->startBookmark();
            pbs->setName(achName);
            pbs->setAdded(dt);
            if (ReadBookmarkUrl(pach, pbs)) {
               pbs->endBookmark();
            }
            else {
               pbs->undoCurrent();
            }
         }
      }
      else if ((dwAttributes & FILE_ATTRIBUTE_SYSTEM) == 0) {
         // directory
         //
         // !(wfd.cFileName == "." || wfd.cFileName == "..")
         // !(strcmp(wfd.cFileName, ".") == 0 || strcmp(wfd.cFileName, "..") == 0)
         //
         // strcmp(wfd.cFileName, ".") == 0   <==>   wfd.cFileName[0] == '.' && wfd.cFileName[1] == 0
         // strcmp(wfd.cFileName, "..") == 0  <==>   wfd.cFileName[0] == '.' && wfd.cFileName[1] == '.' && wfd.cFileName[2] == 0
         // 
         // !( (wfd.cFileName[0] == '.' && wfd.cFileName[1] == 0) ||
         //    (wfd.cFileName[0] == '.' && wfd.cFileName[1] == '.' && wfd.cFileName[2] == 0))
         // !( wfd.cFileName[0] == '.' && (wfd.cFileName[1] == 0 || (wfd.cFileName[1] == '.' && wfd.cFileName[2] == 0)
         // wfd.cFileName[0] != '.' || !(wfd.cFileName[1] == 0 || wfd.cFileName[1] == '.' && wfd.cFileName[2] == 0)
         // wfd.cFileName[0] != '.' || (wfd.cFileName[1] != 0 && (wfd.cFileName[1] != '.' || wfd.cFileName[2] != 0)
         //
       //if (pach[i] != '.' ||
       //    (cchFile != 1 &&
       //     (pach[i + 1] != '.' || cchFile != 2))) {
         if (pach[i] != '.') {
            tchar_t achName[MAX_PATH];

            if (pach[i] == '%' && cchFile == 1) {
               // %.url
               achName[0] = 0;
            }
            else {
               Decode(pach + i, cchFile, '%', WindowsFavorites::m_gachMap, ELEMENTS(WindowsFavorites::m_gachMap), achName, ELEMENTS(achName));
            }

            pbs->startFolder();
            pbs->setName(achName);
            pach[i + cchFile] = TEXT('\\');

            if (recurse(pach, i + cchFile + 1, pbs)) {
               pbs->endFolder();
            }
            else {
               pbs->undoCurrent();
            }
         }
      }
   } while (findNext(h, pach + i, &dwAttributes, &dt, &size));

   DWORD dwError = ::GetLastError();

   if (!::FindClose(h)) {
      // error
      throw Win32Error("FindClose");
   }

   if (dwError != ERROR_NO_MORE_FILES) {
      // error
      throw Win32Error("FindNextFile");
   }

   pbs->popFolder();

   return true;
}

static bool ReadBookmarkUrl(LPCTSTR pszFilename, BookmarkSink *pbs) {
   bool result = false;
   FileInputStream f;

   BufferedInputStream b(&f);
   char achBuffer[4100];

   if (f.open(pszFilename)) {
      while (b.readLine(achBuffer, sizeof(achBuffer)) && !result) {
         if (strcmp(achBuffer, WindowsFavorites::m_gszInternetShortcut) == 0) {

            while (b.readLine(achBuffer, sizeof(achBuffer)) &&
                   achBuffer[0] != '[') {

               if (memcmp(achBuffer, "URL=", 4) == 0) {
                  pbs->setBookmarkHref(achBuffer + 4);

                  result = true;
               }
               else if (memcmp(achBuffer, "Modified=", 9) == 0) {
                  const char *p = achBuffer + 9;
                  union {
                     unsigned char b[8];
                     FILETIME ft;
                  } u;
                  unsigned char b, cs = 0;

                  for (int i = 0; i < 8 && (b = hex(&p)) >= 0; i++) {
                     u.b[i] = b;
                     cs += b;
                  }

                  if (cs == hex(&p)) {
                     DateTime dt;
                     dt.setFileTime(u.ft);
                     pbs->setBookmarkModified(dt);
                  }
               }
            }
         }
      }

      b.close();
   }

   return result;
}

static int hex(const char **pp) {
   const char *p = *pp;
   int d1 = Character::digitValue(*p++);
   if (d1 >= 0) {
      int d2 = Character::digitValue(*p++);
      if (d2 >= 0) {
         *pp = p;
         return (d1 << 4) | d2;
      }
   }

   return -1;
}
