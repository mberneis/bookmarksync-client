/*
 * SyncLib/util.cxx
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
 *    The util.cxx module contains some string and unicode string helper
 *    functions.
 */
// #define TRACEMEM
#include <cstdlib>
#include <new>

#include <windowsx.h>

#include "Log.h"
#include "util.h"

#define TRACEMEM

using namespace syncit;

static DWORD TruncateFile(TCHAR *pachBuffer,
                          DWORD cchBuffer);

unsigned long syncit::u_atoul(LPCTSTR psz, unsigned long ulDefault) {
   unsigned long ulValue = 0;
   TCHAR ch = *psz++;

   do {
      if (TEXT('0') <= ch && ch <= TEXT('9')) {
         ulValue = ulValue * 10 + (ch - TEXT('0'));
      }
      else {
         return ulDefault;
      }
      ch = *psz++;
   } while (ch);

   return ulValue;
}

LPTSTR syncit::Combine(LPCTSTR pszDirectory, LPCTSTR pszFilename) {
   size_t cchDirectory = tstrlen(pszDirectory);
   size_t cchFilename  = tstrlen(pszFilename);

   TCHAR *pszResult = (TCHAR *) u_calloc(cchDirectory + cchFilename + 2, sizeof(TCHAR));

   tstrcpy(pszResult, pszDirectory);

   if (cchDirectory > 0 && pszDirectory[cchDirectory - 1] != TEXT('\\') &&
       pszFilename[0] != TEXT('\\'))
      pszResult[cchDirectory++] = TEXT('\\');

   tstrcpy(pszResult + cchDirectory, pszFilename);

   return pszResult;
}

bool syncit::Combine(LPCTSTR pszDirectory, LPCTSTR pszFilename, LPTSTR pachBuffer, size_t cchBuffer) {
   size_t cchDirectory = tstrlen(pszDirectory);
   size_t cchFilename  = tstrlen(pszFilename);

   if (cchDirectory < cchBuffer) {
      tstrcpy(pachBuffer, pszDirectory);

      if (cchDirectory > 0 && pszDirectory[cchDirectory - 1] != TEXT('\\') &&
          pszFilename[0] != TEXT('\\'))
         pachBuffer[cchDirectory++] = TEXT('\\');

      if (cchDirectory + cchFilename <= cchBuffer) {
         tstrcpy(pachBuffer + cchDirectory, pszFilename);
         return true;
      }
      else
         return false;
   }
   else
      return false;
}

DWORD syncit::GetRootDirectory(TCHAR *pachBuffer, size_t cchBuffer) {
   DWORD dw = GetModuleFileName(NULL, pachBuffer, cchBuffer);

   return ::TruncateFile(pachBuffer, dw);
}

void syncit::GetConfigFilename(LPCTSTR pszName, LPCTSTR pszExtension, TCHAR *pachBuffer, size_t cchBuffer) {

   size_t cchName = tstrlen(pszName);
   size_t cchFilename = cchName + tstrlen(pszExtension);

   assert(cchBuffer > cchFilename + 1);

   DWORD dw = GetRootDirectory(pachBuffer, cchBuffer - cchFilename - 1);

   tstrcpy(pachBuffer + dw, pszName);
   tstrcpy(pachBuffer + dw + cchName, pszExtension);
}

DWORD syncit::TruncateFile(TCHAR *pachBuffer) {
   return ::TruncateFile(pachBuffer, tstrlen(pachBuffer));
}

static DWORD TruncateFile(TCHAR *pachBuffer,
                          DWORD cchBuffer) {
   DWORD dw = cchBuffer;

   if (dw > 0) {
      assert(pachBuffer[dw] == TEXT('\0'));
      // pachBuffer[dw] == TEXT('\0')

      dw--;
      while (0 <= (int) dw && pachBuffer[dw] != TEXT('\\'))
         dw--;

      // -1 == (int) dw || pachBuffer[dw] == TEXT('\\')
      //
      pachBuffer[++dw] = TEXT('\0');
   }

   return dw;
}

#ifndef NDEBUG

#include "CriticalSection.h"

static unsigned __int64 lastMemoryId = 0;
static char gbLockId[sizeof(CriticalSection)];
static CriticalSection * gLockId;


struct MemoryTag {
   enum {
      START,
      END
   }                 m_fType;

   char              m_achTag[4];
   size_t            m_cb;
   unsigned __int64  m_id;
};

static MemoryTag *Allocate(size_t cb, LPCTSTR pszSourceFile, unsigned long ulSourceLine) {
   MemoryTag *pmtStart = (MemoryTag *) LocalAlloc(LMEM_FIXED, cb + 2 * sizeof(MemoryTag));

   MemoryTag *pmtEnd = (MemoryTag *) (((char *) (pmtStart + 1)) + cb);

   u_memset(pmtStart->m_achTag, 'A', sizeof(pmtStart->m_achTag));
   u_memset(pmtEnd->m_achTag,   'A', sizeof(pmtEnd->m_achTag));

   pmtStart->m_fType = MemoryTag::START;
   pmtEnd->m_fType = MemoryTag::END;

   pmtStart->m_cb = cb;
   pmtEnd->m_cb = cb;

   if (gLockId == NULL) {
      gLockId = new(gbLockId) CriticalSection;
   }

   gLockId->enter();
   ++lastMemoryId;
   pmtStart->m_id = lastMemoryId;
   pmtEnd->m_id = lastMemoryId;
   gLockId->leave();

#ifdef TRACEMEM
   Log("mem,%08lX%08lX,a,%d,%s,%lu\r\n",
       (unsigned long) (pmtStart->m_id >> 32),
       (unsigned long) (pmtStart->m_id & 0x00FFFFFFFF),
       cb, pszSourceFile, ulSourceLine);
#endif

   return pmtStart;
}

void *syncit::u_mallocX(size_t cb, LPCTSTR pszSourceFile, unsigned long ulSourceLine) {
   MemoryTag *pmt = Allocate(cb, pszSourceFile, ulSourceLine);

   void *pvResult = (void *) (pmt + 1);

   u_memset(pvResult, 'A', cb);

   return pvResult;
}

void *syncit::u_reallocX(void *p, size_t cb, LPCTSTR pszSourceFile, unsigned long ulSourceLine) {
   MemoryTag *pmtStart = ((MemoryTag *) p) - 1;

   void *pvResult = syncit::u_mallocX(cb, pszSourceFile, ulSourceLine);

   if (p != NULL) {
      u_memcpy(pvResult, p, cb > pmtStart->m_cb ? pmtStart->m_cb : cb);

      u_free(p);
   }

   return pvResult;
}

void *syncit::u_callocX(size_t n, size_t cb, LPCTSTR pszSourceFile, unsigned long ulSourceLine) {
   MemoryTag *pmt = Allocate(n * cb, pszSourceFile, ulSourceLine);

   void *pvResult = (void *) (pmt + 1);

   u_memset(pvResult, 0, n * cb);

   return pvResult;
}

void syncit::u_free(void *p) {
   assert(u_isValid(p));

   MemoryTag *pmtStart = ((MemoryTag *) p) - 1;

#ifdef TRACEMEM
   Log("mem,%08lX%08lX,d,%d\r\n",
       (unsigned long) (pmtStart->m_id >> 32),
       (unsigned long) (pmtStart->m_id & 0x00FFFFFFFF),
       pmtStart->m_cb);
#endif

   u_memset(pmtStart, 'D', pmtStart->m_cb + 2 * sizeof(MemoryTag));

   LocalFree((HLOCAL) pmtStart);
}

bool syncit::u_isValid(void *p) {
   MemoryTag *pmtStart = ((MemoryTag *) p) - 1;

   if (p == NULL ||
       pmtStart->m_achTag[0] != 'A' ||
       pmtStart->m_achTag[1] != 'A' ||
       pmtStart->m_achTag[2] != 'A' ||
       pmtStart->m_achTag[3] != 'A' ||
       pmtStart->m_fType != MemoryTag::START)
      return false;

   MemoryTag *pmtEnd = (MemoryTag *) ((char *) p + pmtStart->m_cb);

   if (pmtEnd->m_achTag[0] != 'A' ||
       pmtEnd->m_achTag[1] != 'A' ||
       pmtEnd->m_achTag[2] != 'A' ||
       pmtEnd->m_achTag[3] != 'A' ||
       pmtEnd->m_fType != MemoryTag::END ||
       pmtEnd->m_id != pmtStart->m_id)
      return false;

   return true;
}

#endif /* NDEBUG */

const char *syncit::skipws(const char *p) {
   while (isspace(*p))
      p++;
   return p;
}

bool syncit::MkDir(const TCHAR *pszPath) {
   TCHAR ach[MAX_PATH];

   size_t cch = bufcopy(pszPath, ach, ELEMENTS(ach)), i = cch;
   int r = CreateDirectory(ach, NULL);

   while (!r && GetLastError() == ERROR_PATH_NOT_FOUND) {
      do {
         if (i == 0) {
            return false;
         }

         i--;
      } while (ach[i] != '\\');

      ach[i] = 0;
      r = CreateDirectory(ach, NULL);
   }

   while (r && i < cch) {
      ach[i] = '\\';

      do {
         i++;
      } while (ach[i] != 0);

      r = CreateDirectory(ach, NULL);
   }

   return r ? true : false;
}

size_t syncit::UrlToFilename(const char *pszUrl, char *pachPath, size_t cchPath, bool fCreate) {
   size_t i, idir = 0;

   GetConfigFilename(pszUrl, "", pachPath, cchPath);

   for (i = 0; i < cchPath && pachPath[i]; i++) {
      if (pachPath[i] == '/') {
         pachPath[i] = '\\';
         idir = i;
      }
      else if (pachPath[i] == '\\') {
         idir = i;
      }
   }

   if (fCreate && idir > 0) {
      pachPath[idir] = 0;
      MkDir(pachPath);
      pachPath[idir] = '\\';
   }

   return i;
}
