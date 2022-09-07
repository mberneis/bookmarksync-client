/*
 * SyncLib/FileOutputStream.cxx
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
 *    to do the character output.
 *
 * See also:
 *    BufferedOutputStream.h     -- layer on top of a FileOutputStream to
 *    BufferedOutputStream.cxx      buffer, read by character or by line
 */
#include <cassert>

#include "FileOutputStream.h"
#include "Errors.h"
#include "util.h"

using namespace syncit;
using namespace std;

Win32OutputStream::Win32OutputStream(HANDLE h, LPCTSTR pszName) : m_h(h), m_szCur(pszName) {
}

/* virtual */
Win32OutputStream::~Win32OutputStream() {
}

FileOutputStream::FileOutputStream() : Win32OutputStream(INVALID_HANDLE_VALUE, TEXT("")) {
}

/* virtual */
FileOutputStream::~FileOutputStream() {
   if (isOpen()) close();
}

void FileOutputStream::create(LPCTSTR psz) {
   m_szNew = psz;
   m_szCur = m_szNew + '~';

   createFile(CREATE_ALWAYS);
}

bool FileOutputStream::create0(LPCTSTR psz) {
   m_szCur = psz;

   return createFile(CREATE_ALWAYS);
}

/**
 * Open/append an existing file for writing.
 * @return true if the file was created, false if the file existed before
 */
bool FileOutputStream::open(LPCTSTR pszFilename) {
   bool result = open0(pszFilename);

   // set to append...
   SetFilePointer(m_h,        // hFile
                  0,          // lDistanceToMove
                  NULL,       // plDistanceToMoveHigh
                  FILE_END);  // dwMoveMethod

   return result;
}

/**
 * Open/append an existing file for writing.
 * @return true if the file was created, false if the file existed before
 */
bool FileOutputStream::open0(LPCTSTR pszFilename) {
   m_szCur = pszFilename;

   return createFile(OPEN_ALWAYS);
}

/**
 * @param dwCreation  one of CREATE_ALWAYS or OPEN_ALWAYS
 * @return true if created, false if it already exists
 * @exception FileError if file cannot be opened/created
 */
bool FileOutputStream::createFile(DWORD dwCreation) {
   assert(!isOpen());

   m_h = ::CreateFile(m_szCur.c_str(),             // pszFileName
                      GENERIC_WRITE,               // dwDesiredAccess
                      FILE_SHARE_READ,             // dwShareMode
                      NULL,                        // pSecurityAttributes
                      dwCreation,                  // dwCreation
                      FILE_FLAG_SEQUENTIAL_SCAN,   // dwFlagsAndAttributes
                      NULL);                       // hTemplate;

   DWORD dwError = ::GetLastError();

   if (m_h == INVALID_HANDLE_VALUE) {
      throw FileError(FileError::Create, m_szCur, Win32Error("CreateFile", dwError));
   }

   assert(isOpen());

   return dwError != ERROR_ALREADY_EXISTS;
}

bool Win32OutputStream::isOpen() const {
   return m_h != INVALID_HANDLE_VALUE;
}

/**
 * Write the contents of the buffer to the file/socket.
 * <p>
 * The entire contents are written.
 *
 * @param pbBuffer start of buffer to write
 * @param cbBuffer length of buffer in bytes to write
 * @return true if successful, false on error
 */
/* virtual */
void Win32OutputStream::write(const char *pbBuffer, size_t cbBuffer) {
   DWORD dwWritten;

   assert(isOpen());

   do {
      if (!::WriteFile(m_h,         // hFile
                       pbBuffer,    // pbBuffer
                       cbBuffer,    // cbBuffer
                       &dwWritten,  // pcbNumberOfBytesWritten
                       NULL)) {
         throw FileError(FileError::Write, m_szCur, Win32Error("WriteFile"));
      }

      pbBuffer += dwWritten;
      cbBuffer -= dwWritten;
   } while (cbBuffer != 0);
}

/**
 * Flush any cached/buffered data to the stream.
 *
 * @return true if successful, false on error
 */
/* virtual */
void Win32OutputStream::flush() /* throws Exception */ {
   assert(isOpen());
}

/**
 * Close the stream.
 */
/* virtual */
void Win32OutputStream::close() /* throws Exception */ {
   assert(isOpen());

   HANDLE h = m_h;
   m_h = INVALID_HANDLE_VALUE;

   if (!::CloseHandle(h)) {
      throw FileError(FileError::Close, m_szCur, Win32Error("CloseHandle"));
   }

   assert(!isOpen());
}

void FileOutputStream::commit() /* throws Exception */ {
   assert(!isOpen());

   const char *pszNew = m_szNew.c_str();
   const char *pszCur = m_szCur.c_str();

   if (!DeleteFile(pszNew)) {
      DWORD dwError = GetLastError();

      if (dwError != ERROR_FILE_NOT_FOUND) {
         throw FileError(FileError::Access, m_szNew, Win32Error("DeleteFile"));
      }
   }

   if (!MoveFile(pszCur, pszNew)) {
      throw Win32Error("MoveFile");
   }
}
