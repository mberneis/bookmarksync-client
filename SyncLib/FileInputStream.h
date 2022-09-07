/*
 * SyncLib/FileInputStream.h
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
 */
#ifndef FileInputStream_H
#define FileInputStream_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include <string>
#include <cwchar>

#include "InputStream.h"
#include "util.h"

namespace syncit {

   using std::basic_string;

   class Win32InputStream : public InputStream {

   public:
      Win32InputStream(HANDLE h);

      virtual ~Win32InputStream();

      bool isOpen() const;

      /**
       * Read from the file directly into the buffer
       *
       * @param pbBuffer pointer to start of buffer to read into
       * @param cbBuffer size in bytes of buffer
       * @return a value 1..cbBuffer indicating how many bytes have been
       *          read, or 0 on EOF
       */
      virtual size_t read(char *pbBuffer, size_t cbBuffer) /* throws Error */;

      /**
       * Close the opened file.
       *
       * @require isOpen()
       * @ensure  !isOpen()
       */
      virtual void close() /* throws Error */;

      HANDLE getHandle() {
         return m_h;
      }

   protected:
      HANDLE m_h;

   private:
      // Disable copy constructor and assignment
      Win32InputStream(Win32InputStream &rhs);
      Win32InputStream &operator=(Win32InputStream &rhs);
   };

   class FileInputStream : public Win32InputStream {

   public:
      FileInputStream();
      virtual ~FileInputStream();

      /**
       * Open the named file for reading.
       *
       * @param psz  the filename to open
       * @return true on success, false if file/path not found
       * @exception Error on any other open error
       *
       * @require !isOpen()
       * @ensure result ? isOpen() : !isOpen()
       */
      bool open(LPCTSTR pszFilename) /* throws Error */;

   private:

      // Disable copy constructor and assignment
      FileInputStream(FileInputStream &rhs);
      FileInputStream &operator=(FileInputStream &rhs);
   };

   extern Win32InputStream StandardInput;

}

#endif /* FileInputStream_H */
