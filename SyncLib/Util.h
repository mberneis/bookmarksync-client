/*
 * SyncLib/util.h
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
 *    The util.h module contains some string and unicode string helper
 *    macros.
 */
#ifndef UTIL_H
#define UTIL_H

#pragma warning( disable : 4786 )

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include <string>

#include <cstdarg>
#include <cstdlib>
#include <cstring>

namespace syncit {

#define REDIRECT_NEW

/*
 * This header is divided into four sections:
 * Section 1.  Utility macros
 *    ELEMENTS(array)
 *    assert(expr)
 * Section 2.  Unicode support
 *    u_atoul
 * Section 3.  Win32 utility functions
 *    Combine
 *    GetRootDirectory
 *    TruncateFile
 *    SetWindowText
 * Section 4.  Safe memory allocation
 *    u_malloc
 *    u_calloc
 *    u_realloc
 *    u_free
 *    u_free0
 *    new
 *    delete
 */

/*******************************
 * Section 1.  Utility macros...
 */

/**
 * ELEMENTS(array) returns the number of items in a static array.
 * For instance:
 *    char ach1[10];
 *    assert(sizeof(ach1) == 10 && ELEMENTS(ach1) == 10);
 *    wchar_t ach2[10];
 *    assert(sizeof(ach2) == 20 && ELEMENTS(ach2) == 10);
 */
#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

   inline void u_memset(void *pv, unsigned char b, size_t cb) {
      FillMemory(pv, cb, b);
   }

   inline void u_memcpy(void *pvdst, const void *pvsrc, size_t cb) {
      CopyMemory(pvdst, pvsrc, cb);
   }

/*
 * ...section 1.  Utility macros
 *******************************/

/********************************
 * Section 2.  Unicode support...
 */

   unsigned long u_atoul(LPCTSTR psz, unsigned long ulDefault = 0);

/*
 * ...section 2.  Unicode support
 ********************************/

/****************************************
 * Section 3.  Win32 utility functions...
 */

   LPTSTR Combine(LPCTSTR pszDirectory,
                  LPCTSTR pszFilename);

   bool Combine(LPCTSTR pszDirectory,
                LPCTSTR pszFilename,
                LPTSTR pachBuffer,
                size_t cchBuffer);

   DWORD GetRootDirectory(TCHAR *pachBuffer, size_t cchBuffer);

   void GetConfigFilename(LPCTSTR pszName, LPCTSTR pszExtension, TCHAR *pachBuffer, size_t cchBuffer);

   DWORD TruncateFile(TCHAR *pachBuffer);

   bool MkDir(LPCTSTR pszPath);

   size_t UrlToFilename(const char *pszUrl, char *pachPath, size_t cchPath, bool fCreate);
/*
 * ...section 3.  Win32 utility functions
 ****************************************/

/***************************************
 * Section 4.  Safe memory allocation...
 */
#ifdef NDEBUG
// not debugging -- u_...alloc routines that map to Win32 API
# define u_malloc(cb)    ((void*)LocalAlloc(LMEM_FIXED,(cb)))
# define u_realloc(p,cb) ((p)==NULL?u_malloc(cb):(void*)LocalReAlloc((HLOCAL)(p),(cb),LMEM_MOVEABLE))
# define u_calloc(n,cb)  ((void*)LocalAlloc(LPTR,(n)*(cb)))

# define u_free(p)       (LocalFree((HLOCAL)(p)))

#else
// debugging -- u_...alloc routines that tracemem
# define u_malloc(cb) u_mallocX((cb),__FILE__,__LINE__)
# define u_realloc(p,cb) u_reallocX((p),(cb),__FILE__,__LINE__)
# define u_calloc(n,cb) u_callocX((n),(cb),__FILE__,__LINE__)

   void *u_mallocX(size_t cb, const LPCTSTR pszSourceFile, unsigned long ulSourceLine);
   void *u_reallocX(void *p, size_t cb, const LPCTSTR pszSourceFile, unsigned long ulSourceLine);
   void *u_callocX(size_t n, size_t cb, const LPCTSTR pszSourceFile, unsigned long ulSourceLine);

   void u_free(void *p);

   bool u_isValid(void *p);

#endif /* NDEBUG */

   inline void u_free0(void *p) {
      if (p != NULL) u_free(p);
   }

   const char *skipws(const char *p);
}

#ifdef REDIRECT_NEW

# ifdef NDEBUG
#  define NEW new

inline void *operator new(size_t cb) { return u_malloc(cb); }

# else /* !NDEBUG */

#  define NEW new(__FILE__, __LINE__)
inline void *operator new(size_t cb, const char *pszSourceFile, unsigned long ulSourceLine) {
   return syncit::u_mallocX(cb, pszSourceFile, ulSourceLine);
}

inline void *operator new(size_t cb) {
   return syncit::u_mallocX(cb, __FILE__, __LINE__);
}

inline void operator delete(void *pv, const char *pszSourceFile, unsigned long ulSourceLine) {
   syncit::u_free0(pv);
}

# endif /* NDEBUG */

inline void operator delete(void *p) {
   syncit::u_free0(p);
}

#else /* REDIRECT_NEW */
# define NEW new

#endif /* REDIRECT_NEW */

/*
 * ...section 4.  Safe memory allocation
 ***************************************/


#endif /* UTIL_H */
