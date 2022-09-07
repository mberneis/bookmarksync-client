/*
 * SyncLib/FileOutputStream.h
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
 *    we just use the Win32 file routines, and have a buffered writer
 *    to do the character output.
 */
#ifndef FileOutputStream_H
#define FileOutputStream_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include <string>

#include "OutputStream.h"
#include "Errors.h"

#include "util.h"

namespace syncit {

   using namespace std;
   using std::basic_string;

   class Win32OutputStream : public OutputStream {

   public:
      Win32OutputStream(HANDLE h, LPCTSTR pszName);

      virtual ~Win32OutputStream();

      bool isOpen() const;

      /**
       * Write the contents of the buffer to the file/socket.
       * <p>
       * The entire contents are written.
       *
       * @param pbBuffer start of buffer to write
       * @param cbBuffer length of buffer in bytes to write
       */
      virtual void write(const char *pbBuffer, size_t cbBuffer) /* throws Error */;

      /**
       * Flush any cached/buffered data to the stream.
       */
      virtual void flush();

      /**
       * Close the stream.
       */
      virtual void close();

   protected:
      HANDLE m_h;

      basic_string<TCHAR> m_szCur;

   private:
      // Disable copy constructor and assignment
      Win32OutputStream(Win32OutputStream &rhs);
      Win32OutputStream &operator=(Win32OutputStream &rhs);
   };

   class FileOutputStream : public Win32OutputStream {
   public:
      FileOutputStream();

      virtual ~FileOutputStream();

      /**
       * Create/open a new file for writing.
       * The new file is created with a file name suffixed with '~'
       * Use the commit method to move the file to its correct filename.
       *
       * @param psz  pointer to null-terminated string -- the filename to open
       *
       * @require !isOpen()
       * @ensure result == isOpen()
       */
      void create(LPCTSTR pszFilename) /* throws Error */;

      /**
       * Create/open a new file for writing.  The file position is placed at the
       * start of the file regardless of whether the file existed before or not.
       * Creates the file directly, without the '~' hokey-pokey that the create() method
       * uses.
       *
       * @param psz  pointer to null-terminated string -- the filename to open
       * @return true if the file was created, false if the file existed before
       *
       * @require !isOpen()
       * @ensure result == isOpen()
       */
      bool create0(LPCTSTR pszFilename) /* throws Error */;

      /**
       * Open/append a new file for writing.  The file position is placed at the
       * end of the file.
       *
       * @param psz  pointer to null-terminated string -- the filename to open
       * @return true if the file was created, false if the file existed before
       *
       * @require !isOpen()
       * @ensure result == isOpen()
       */
      bool open(LPCTSTR pszFilename) /* throws Error */;

      /**
       * Open an existing file for writing.  The file position is placed at the
       * beginning of the file.  The contents of the file haven't yet been erased.
       */
      bool open0(LPCTSTR pszFilename) /* throws Error */;

      /**
       * Commit the file.  The output stream is really writing
       * to a file named <psz>~ (tilde at the end).  When the file
       * is closed, the application can commit the changes by deleting
       * the original and renaming the ~ to the original.
       */
      virtual void commit() /* throws Error */;

   protected:
      bool createFile(DWORD dwCreation);

   private:
      basic_string<TCHAR> m_szNew;
   };

}

#endif /* FileOutputStream_H */
