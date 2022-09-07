/*
 * BookmarkLib/AOLInput.cxx
 * Copyright (C) 2003  SyncIT.com, Inc.
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
 * Description: BookmarkSync client software for Windows
 * Author:      Terence Way
 * Created:     October 1998
 * Modified:    September 2003 by Terence Way
 * E-mail:      mailto:tway@syncit.com
 * Web site:    http://www.syncit.com
 */
#pragma warning( disable : 4786 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "SyncLib/Errors.h"
#include "SyncLib/RegKey.h"
#include "SyncLib/Util.h"

#include "BookmarkModel.h"
#include "BrowserBookmarks.h"

using namespace syncit;

typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef          __int32  int32;

typedef int32 REGOFF;   /* offset into registry file */
typedef uint32 RECID;   /* one-origined record index */

struct AolFileHeader {
   char     start[16];  /* must be "AOLVM100" */
   REGOFF   index;
};

struct AolRecordHead {
   char    start[4]; /* must be 'R' 'S' ... */
   uint32  length;
};

struct AolRecordIndex {
   uint32 crud1;
   uint32 crud2;

   REGOFF index[1];
};

struct AolRecordTail {
   RECID  url;
   RECID  next;
   RECID  prev;
   RECID  parent;
   RECID  child;
};

static const HKEY AOL_HKEYS[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };

static struct {
   const char *pszKeyName;
   const char *pszValueName;
} AOL_VALUES[] = {
   { "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\AOL.EXE",   "Path" },
   { "Software\\America Online\\America Online\\4.0",                      "AppPath" },
   { "Software\\America Online\\AOL\\CurrentVersion",                      "AppPath" },

   { "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CS.EXE",    "Path" },
   { "Software\\CompuServe\\CompuServe\\2000.0",                           "AppPath" },
   { "Software\\CompuServe\\CompuServe\\CurrentVersion",                   "AppPath" }

};


static HANDLE OpenAolFile(LPCTSTR pszFilename) /* throws Win32Error */;
static void ReadRecord(HANDLE h, REGOFF index[], RECID recid, int tab,
                       BookmarkSink *pbs) /* throws Win32Error */;
static bool ReadBuffer(HANDLE h, REGOFF offset, void *pbBuffer, size_t cbBuffer) /* throws Win32Error */;

bool AolFavoritePlaces::Read(LPCTSTR pszFilename, BookmarkSink *pbs) /* throws Win32Error */ {
   HANDLE h = OpenAolFile(pszFilename); /* throws Win32Error */
   DWORD dwOffset;

   // offset of first record
   if (ReadBuffer(h, 16, (void *) &dwOffset, sizeof(dwOffset))) {
      AolRecordHead head;

      if (ReadBuffer(h, dwOffset, (void *) &head, sizeof(head))) {
         REGOFF *pr = (REGOFF *) u_malloc(head.length);

         if (ReadBuffer(h, dwOffset + sizeof(head), (void *) pr, head.length)) {
            ReadRecord(h, pr + 1, 1, 0, pbs); /* throws Win32Error */
         }

         u_free((void *) pr);
      }
   }

   CloseHandle(h);
   return true;
}

/**
 * Given a profile filename, get its profile name.  Return true if
 * it's an AOL profile db and we can retrieve the profile name, false otherwise
 *
 * @param pszProfileFilename  filename of AOL db
 * @param pach   buffer to store profile name
 * @param cch    size of buffer
 */
unsigned AolFavoritePlaces::GetProfileName(LPCTSTR pszProfileFilename,
                                           char *pach, size_t cch) {
   HANDLE h = OpenAolFile(pszProfileFilename);
   unsigned result = 0;

   AolFileHeader head;

   // offset of first record
   if (ReadBuffer(h, 0, (void *) &head, sizeof(head)) &&
      lstrcmpA(head.start, "AOLVM100") == 0) {

      AolRecordHead  indexHead;

      if (ReadBuffer(h, head.index, (void *) &indexHead, sizeof(indexHead)) &&
          indexHead.start[0] == 'R' && indexHead.start[1] == 'S' &&
          indexHead.length >= sizeof(AolRecordIndex)) {
         AolRecordIndex *pIndex = (AolRecordIndex *) u_malloc(indexHead.length);
         AolRecordHead first;

         if (ReadBuffer(h, head.index + sizeof(indexHead), (void *) pIndex, indexHead.length) &&
             ReadBuffer(h, pIndex->index[0], (void *) &first, sizeof(first)) &&
             first.start[0] == 'R' && first.start[1] == 'S' &&
             first.length > 18) {
            char *p = (char *) u_malloc(first.length - 18);

            if (ReadBuffer(h, pIndex->index[0] + sizeof(AolRecordHead) + 18, p, first.length - 18)) {
               size_t l = lstrlenA(p);

               if (l < first.length - 18) {
                  unsigned j;

                  for (j = 0; j * 4 < indexHead.length &&
                                  pIndex->index[j] != 0; j++) {
                  }

                  bufcopy(p, l, pach, cch);

                  result = j;
               }
            }

            u_free(p);
         }

         u_free((void *) pIndex);
      }
   }

   CloseHandle(h);

   return result;
}

/**
 * Open the specified AOL file for random binary access.
 *
 * @param pszProfileFilename -- fully qualified name of file
 *
 * @return the file handle
 * @exception Win32Error on any file open error (even file not found)
 */
static HANDLE OpenAolFile(LPCTSTR pszFilename) /* throws Win32Error */ {
   HANDLE h;

   h = CreateFile(pszFilename,
                  GENERIC_READ,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL,
                  OPEN_EXISTING,
                  FILE_FLAG_RANDOM_ACCESS,
                  NULL);

   if (h == INVALID_HANDLE_VALUE) {
      /*
       * The profile name was given to us by traversing the
       * directory, so the file definitely exists.  Therefore
       * throw an exception without checking if the error is
       * ERROR_FILE_NOT_FOUND
       */
      throw Win32Error("CreateFile");
   }

   return h;
}

static void ReadRecord(HANDLE h, REGOFF index[], RECID recid, int tab,
                       BookmarkSink *pbs) /* throws Win32Error */ {
   while (recid != 0) {
      AolRecordHead head;
      REGOFF offset = index[recid];

      if (ReadBuffer(h, offset, (void *) &head, sizeof(head))) {
         if (head.start[0] == 'R' && head.start[1] == 'S' && head.length > sizeof(AolRecordTail)) {
            unsigned char *p = (unsigned char *) u_malloc(head.length);

            if (ReadBuffer(h, offset + sizeof(head), (void *) p, head.length) && head.length > 18) {
               AolRecordTail *pt = (AolRecordTail *) (p + head.length - sizeof(AolRecordTail));
               char *pszName = (char *) p + 18;

               pbs->progress();

               if (pt->url != 0 && tab > 1) {
                  AolRecordHead headUrl;

                  if (ReadBuffer(h, index[pt->url], (void *) &headUrl, sizeof(headUrl))) {
                     char *pszUrl = (char *) u_malloc(headUrl.length);
                     if (ReadBuffer(h, index[pt->url] + sizeof(headUrl), pszUrl, headUrl.length)) {
                        pbs->startBookmark();
                        pbs->setName(pszName);
                        pbs->setBookmarkHref(pszUrl);
                        pbs->endBookmark();
                     }

                     u_free(pszUrl);
                  }

                  pbs->progress();
               }

               if (pt->child != 0) {
                  if (tab == 0 ||
                      (tab == 1 && (p[10] == 0xC8 || lstrcmpA(pszName, "Favorite Places") == 0))) {
                     ReadRecord(h, index, pt->child, tab + 1, pbs); /* throws Win32Error */
                  }
                  else if (tab > 1) {
                     pbs->startFolder();
                     pbs->setName(pszName);
                     pbs->pushFolder();

                     ReadRecord(h, index, pt->child, tab + 1, pbs);

                     pbs->popFolder();
                     pbs->endFolder();
                  }
               }

               recid = pt->next;
            }

            u_free(p);
         }
      }
   }
}

/**
 * Read a specified amount of data from a random offset in
 * a file.
 *
 * @param h         handle to file returned by CreateFile()
 * @param offset    offset in bytes from beginning of file
 * @param pbBuffer  pointer to byte array to store data
 * @param cbBuffer  length of data to read
 *
 * @exception Win32Error on any file error
 *
 * @return true if the amount of data has been completely read in
 * @return false on EOF
 */
static bool ReadBuffer(HANDLE h, REGOFF offset, void *pbBuffer, size_t cbBuffer) /* throws Win32Error */ {
   // go to specified position in file...
   //
   SetFilePointer(h, offset, NULL, FILE_BEGIN);

   BYTE *pb = (BYTE *) pbBuffer;
   size_t cb = cbBuffer;

   while (cb > 0) {
      DWORD dwRead;

      if (!ReadFile(h,
                    pb,
                    cb,
                    &dwRead,
                    NULL)) {
         throw Win32Error("ReadFile");
      }
      else if (dwRead == 0) {
         return false;
      }
      else {
         cb -= dwRead;
         pb += dwRead;
      }
   }

   return true;
}

/**
 * Retrieve the registry settings for the current version of
 * AOL, specifically the installation directory.
 *
 * @param pachData  pointer to array of character to store the
 *                  directory name
 * @param cchData   size (in characters) of pachData
 *
 * @exception on any registry error
 *
 * @return 0 if key not found, length of directory string if found
 */
size_t AolFavoritePlaces::GetDefaultDirectory(TCHAR *pachData, size_t cchData) {

   for (int i = 0; i < ELEMENTS(AOL_HKEYS); i++) {
      for (int j = 0; j < ELEMENTS(AOL_VALUES); j++) {
         RegKey key;

         if (key.open(AOL_HKEYS[i], AOL_VALUES[j].pszKeyName)) {
            size_t l = key.queryValue(AOL_VALUES[j].pszValueName, pachData, cchData);

            if (l > 0) {
               if (pachData[l - 1] == '\\') l--;

               return l + bufcopy("\\organize\\", pachData + l, cchData - l);
            }
         }
      }
   }

   return 0;
}
