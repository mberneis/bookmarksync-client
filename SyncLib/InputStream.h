/*
 * SyncLib/InputStream.h
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
 *    InputStream is an abstract base class patterned after
 *    the java.io interface.  The basic idea is that the level
 *    of file/socket IO is far less complex than that provided
 *    by the C run-time library; so our executable size is smaller;
 *    also it is *very hard* to merge socket IO with file IO in
 *    the standard C library.  This package makes it easy.
 *
 * See also:
 *    BufferedInputStream.h   -- an implementation that buffers another
 *    BufferedInputStream.cxx    InputStream
 *
 *    FileInputStream.h       -- an implementation that reads from a Win32
 *    FileInputStream.cxx        file handle
 *
 *    Socket.h                -- an implementation that reads from a
 *    Socket.cxx                 WinSock socket.
 *
 *    OutputStream.h          -- corresponding class for writing
 */
#ifndef InputStream_H
#define InputStream_H

#include <stddef.h>        // declare size_t

#include "Errors.h"

namespace syncit {

   /**
    * An InputStream is an interface (abstract base class) that can
    * read from a generic file/socket.
    *
    * @see BufferedInputStream
    * @see FileInputStream
    * @see Socket
    */
   class InputStream {

   protected:
      InputStream() {
      }

   public:
      virtual ~InputStream() {
      }

      /**
       * Read into a buffer
       *
       * @param pbBuffer pointer to byte buffer
       * @param cbBuffer size of buffer pointed to by pbBuffer
       * @return the number of bytes read, 1..cbBuffer or 0 on EOF
       */
      virtual size_t read(char *pbBuffer, size_t cbBuffer) /* throws IOError */
            = 0;

      /**
       * Close the stream.
       */
      virtual void close() = 0;

   private:
      // disable copy constructor and assignment
      //
      InputStream(InputStream &out);
      InputStream &operator=(InputStream &out);
   };
}

#endif /* InputStream_H */
