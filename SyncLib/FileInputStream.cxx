/*
 * SyncLib/FileInputStream.cxx
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
 *    Rather than importing the whole stdio package (fopen, gets, etc)
 *    we just use the Win32 file routines, and have a buffered reader
 *    to do the character input.
 *
 * See also:
 *    BufferedInputStream.h      -- layer on top of a FileInputStream to
 *    BufferedInputStream.cxx       buffer, read by character or by line
 */
#include <cassert>

#include "FileInputStream.h"
#include "Errors.h"

using namespace syncit;

Win32InputStream::Win32InputStream(HANDLE h) : m_h(h) {
}

/* virtual */
Win32InputStream::~Win32InputStream() {
}

bool Win32InputStream::isOpen() const {
   return m_h != INVALID_HANDLE_VALUE;
}

FileInputStream::FileInputStream() : Win32InputStream(INVALID_HANDLE_VALUE) {
}

/* virtual */
FileInputStream::~FileInputStream() {
   if (isOpen()) close();
}

bool FileInputStream::open(LPCTSTR psz) /* throws Exception */ {
   int retries = 3;
   unsigned long ulTimeout = 0;

   assert(!isOpen());

   do {
      m_h = ::CreateFile(psz,                      // pszFileName
                         GENERIC_READ,             // dwDesiredAccess
                         FILE_SHARE_READ,          // dwShareMode
                         NULL,                     // pSecurityAttributes
                         OPEN_EXISTING,            // dwCreation
                         FILE_FLAG_SEQUENTIAL_SCAN,// dwFlagsAndAttributes
                         NULL);                    // hTemplate

      if (m_h == INVALID_HANDLE_VALUE) {
         DWORD dwError = ::GetLastError();

         if (dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_PATH_NOT_FOUND) {
            return false;
         }
         else if (retries == 0 || (dwError != ERROR_SHARING_VIOLATION && dwError != ERROR_LOCK_VIOLATION)) {
            throw FileError(FileError::Open, psz, Win32Error("CreateFile", dwError));
         }
         else {
            retries--;
            ::Sleep(ulTimeout);
            ulTimeout = ulTimeout * 2 + 250;
         }
      }
   } while (m_h == INVALID_HANDLE_VALUE);

   assert(isOpen());

   return true;
}

/* virtual */
size_t Win32InputStream::read(char *pbBuffer,
                              size_t cbBuffer) /* throws Exception */ {
   DWORD dwRead;

   assert(isOpen());

   if (!::ReadFile(m_h,       // hFile
                   pbBuffer,  // pbBuffer
                   cbBuffer,  // cbBuffer
                   &dwRead,   // pcbNumberOfBytesRead
                   NULL)) {   // pOverlapped
      DWORD dwError = ::GetLastError();

      if (dwError != ERROR_BROKEN_PIPE) {
         throw Win32Error("ReadFile", dwError);
      }

      dwRead = 0;
   }

   return dwRead;
}

void Win32InputStream::close() /* throws Exception */ {
   assert(isOpen());

   HANDLE h = m_h;
   m_h = INVALID_HANDLE_VALUE;

   if (!::CloseHandle(h)) {
      throw Win32Error("CloseHandle");
   }

   assert(!isOpen());
}
