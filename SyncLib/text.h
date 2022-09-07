/*
 * SyncLib/text.h
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
#ifndef Text_H
#define Text_H

#include <string>
#include <cwchar>

#ifndef ELEMENTS
# define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))
#endif

#ifdef WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
#endif

#define strcat lstrcatA
#define strcmp lstrcmpA
#define stricmp lstrcmpiA
#define strlen lstrlenA
#define strcpy lstrcpyA
#define strncpy lstrcpynA

namespace syncit {

   using namespace std;

/*
   inline void strcat(char *pach, const char *psz) { lstrcatA(pach, psz); }
   inline int strcmp(const char *psz1, const char *psz2) { return lstrcmpA(psz1, psz2); }
   inline int stricmp(const char *psz1, const char *psz2) { return lstrcmpiA(psz1, psz2); }
   inline size_t strlen(const char *psz) { return lstrlenA(psz); }
   inline char *strcpy(char *pach, const char *psz) { return lstrcpyA(pach, psz); }
   inline char *strncpy(char *pach, const char *psz, size_t cch) { return lstrcpynA(pach, psz, cch); }
*/

   char *stralloc(const char *psz);
   char *strrealloc(char *pszOld, const char *s);
   char *strrealloc(char *pszOld, const char *pszNew, size_t cchNew);
   char *strcalloc(const char *psz, size_t cch);

#ifdef TEXT16
# define T(quote) L##quote
# define tmain wmain

   typedef wchar_t tchar_t;
   typedef wstring tstring;

   inline size_t tstrlen(const wchar_t *psz) { return wcslen(psz); }
   inline int tstrcmp(const wchar_t *psz1, const wchar_t *psz2) { return wcscmp(psz1, psz2); }
   inline int tstrncmp(const wchar_t *psz1, const wchar_t *psz2, size_t cch) { return wcsncmp(psz1, psz2, cch); }
   inline unsigned long tstrtoul(const wchar_t *psz, wchar_t **pp, int r) { return wcstoul(psz, pp, r); }

   inline wchar_t *tstrcpy(wchar_t *pszDst, const wchar_t *pszSrc) { return wcscpy(pszDst, pszSrc); }
   inline wchar_t *tstrncpy(wchar_t *pszDst, const wchar_t *pszSrc, size_t cch) { return wcsncpy(pszDst, pszSrc, cch); }

   inline wchar_t *tstrchr(const wchar_t *psz, wchar_t ch) { return wcschr(psz, ch); }

   wchar_t *tstralloc(const wchar_t *psz);
   wchar_t *tstrrealloc(wchar_t *pszOld, const wchar_t *s);
   wchar_t *tstrcalloc(const wchar_t *psz, size_t cch);

#else /* !TEXT16 */
# define T(quote) quote
# define tmain main

   typedef char   tchar_t;
   typedef string tstring;

   inline size_t tstrlen(const char *psz) { return strlen(psz); }
   inline int tstrcmp(const char *psz1, const char *psz2) { return strcmp(psz1, psz2); }
   inline int tstrncmp(const char *psz1, const char *psz2, size_t cch) { return strncmp(psz1, psz2, cch); }
   inline unsigned long tstrtoul(const char *psz, char **pp, int r) { return strtoul(psz, pp, r); }

   inline char *tstrcpy(char *pszDst, const char *pszSrc) { return strcpy(pszDst, pszSrc); }
   inline char *tstrncpy(char *pszDst, const char *pszSrc, size_t cch) { return strncpy(pszDst, pszSrc, cch); }

   inline char *tstrchr(const char *psz, char ch) { return strchr(psz, ch); }

   inline tchar_t *tstralloc(const tchar_t *psz) { return stralloc(psz); }
   inline tchar_t *tstrrealloc(tchar_t *pszOld, const tchar_t *s) { return strrealloc(pszOld, s); }
   inline tchar_t *tstrcalloc(const tchar_t *psz, size_t cch) { return strcalloc(psz, cch); }

#endif /* TEXT16 */


   int tstricmp(const tchar_t *psz1, const tchar_t *psz2);
   inline bool EqualsIgnoreCase(const tchar_t *psz1, const tchar_t *psz2) {
      return tstricmp(psz1, psz2) == 0;
   }

   size_t bufcopy(const char *pszSrc, char *pachDst, size_t cchDst);
   size_t bufcopy(const char *pachSrc, size_t cchSrc,
                  char *pachDst, size_t cchDst);

   size_t Encode(const tchar_t *psz,
                 char chEscape,
                 const tchar_t *pchMap,
                 size_t cchMap,
                 tchar_t *pach, size_t cch);

   size_t Decode(const tchar_t *pszSrc, size_t cchSrc,
                 char chEscape,
                 const tchar_t *pchMap, size_t cchMap,
                 tchar_t *pachDst, size_t cchDst);

}

#endif /* Text_H */
