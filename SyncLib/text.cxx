/*
 * SyncLib/text.cxx
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Character.h"
#include "util.h"
#include "text.h"

using namespace syncit;

int syncit::tstricmp(const tchar_t *psz1, const tchar_t *psz2) {
   tchar_t ch1 = Character::toUpper(*psz1++);
   tchar_t ch2 = Character::toUpper(*psz2++);

   while (ch1 && ch1 == ch2) {
      ch1 = Character::toUpper(*psz1++);
      ch2 = Character::toUpper(*psz2++);
   }

   return (int) ch1 - (int) ch2;
}

/*
bool syncit::EqualsIgnoreCase(const tchar_t *psz1, const tchar_t *psz2) {
   tchar_t ch1 = Character::toUpper(*psz1++);
   tchar_t ch2 = Character::toUpper(*psz2++);

   while (ch1 && ch1 == ch2) {
      ch1 = Character::toUpper(*psz1++);
      ch2 = Character::toUpper(*psz2++);
   }

   // ch1 == 0 || ch1 != ch2
   return ch1 == ch2;
}*/


char *syncit::stralloc(const char *psz) {
   if (psz == NULL) {
      return NULL;
   }
   else {
      char *pszResult = (char *) u_malloc(lstrlenA(psz) + 1);
      lstrcpyA(pszResult, psz);
      return pszResult;
   }
}

char *syncit::strrealloc(char *psz, const char *sz) {
   if (sz == NULL) {
      u_free0(psz);
      return NULL;
   }
   else {
      return strrealloc(psz, sz, lstrlenA(sz));
   }
}

char *syncit::strrealloc(char *pszOld, const char *pszNew, size_t cchNew) {
   if (pszNew == NULL) {
      u_free0(pszOld);
      return NULL;
   }
   else {
      char *pszResult = (char *) u_realloc(pszOld, cchNew + 1);
      pszResult[cchNew] = 0;
      u_memcpy(pszResult, pszNew, cchNew);
      return pszResult;
   }
}

char *syncit::strcalloc(const char *psz, size_t cch) {
   char *pszResult = (char *) u_malloc(cch + 1);
   u_memcpy(pszResult, psz, cch);
   pszResult[cch] = 0;
   return pszResult;
}

size_t syncit::bufcopy(const char *psz,
                       char *pach,
                       size_t cch) {
   return bufcopy(psz, lstrlenA(psz), pach, cch);
}

size_t syncit::bufcopy(const char *pachSrc,
                       size_t cchSrc,
                       char *pachDst,
                       size_t cchDst) {
   if (cchSrc < cchDst) {
      u_memcpy(pachDst, pachSrc, cchSrc);
      pachDst[cchSrc] = 0;
      return cchSrc;
   }
   else {
      u_memcpy(pachDst, pachSrc, cchDst);
      return cchDst;
   }
}

size_t syncit::Encode(const tchar_t *psz,
                      tchar_t chEscape,
                      const tchar_t *pchMap,
                      size_t cchMap,
                      tchar_t *pach, size_t cch) {
   tchar_t *p = pach, *pe = pach + cch - 1;

   while (p < pe) {
      tchar_t ch = *psz++;

      for (size_t i = 0; i < cchMap; i += 2) {
         if (ch == pchMap[i]) {
            *p++ = chEscape;
            *p++ = pchMap[i + 1];
            break;
         }
      }

      if (i >= cchMap) {
         if (ch == 0) {
            *p = 0;
            return p - pach;
         }
         else {
            *p++ = ch;
         }
      }
   }

   return p - pach;
}

size_t syncit::Decode(const tchar_t *pszSrc,
                      size_t cchSrc,
                      tchar_t chEscape,
                      const tchar_t *pchMap,
                      size_t  cchMap,
                      tchar_t *pachDst,
                      size_t cchDst) {
   const tchar_t *psrc = pszSrc,  *psrcend = pszSrc + cchSrc;
   tchar_t       *pdst = pachDst, *pdstend = pachDst + cchDst;

   while (pdst != pdstend && psrc != psrcend) {
      tchar_t ch = *psrc++;

      if (ch == chEscape) {
         ch = *psrc++;

         if (psrc > psrcend || ch == 0) {
            *pdst = 0;
            return pdst - pachDst;
         }

         for (size_t i = 0; i < cchMap; i += 2) {
            if (pchMap[i + 1] == ch) {
               ch = pchMap[i];
               break;
            }
         }
      }

      *pdst++ = ch;
   }

   if (pdst < pdstend) *pdst = 0;

   return pdst - pachDst;
}

