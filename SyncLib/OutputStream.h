/*
 * SyncLib/OutputStream.h
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
 *    OutputStream is an abstract base class patterned after
 *    the java.io interface.  The basic idea is that the level
 *    of file/socket IO is far less complex than that provided
 *    by the C run-time library; so our executable size is smaller;
 *    also it is *very hard* to merge socket IO with file IO in
 *    the standard C library.  This package makes it easy.
 *
 * See also:
 *    BufferedOutputStream.h  -- an implementation that buffers another
 *    BufferedOutputStream.cxx   OutputStream
 *
 *    Socket.h                -- an implementation that reads from a
 *    Socket.cxx                 WinSock socket.
 *
 *    InputStream.h           -- corresponding class for reading
 */
#ifndef OutputStream_H
#define OutputStream_H

namespace syncit {
   class OutputStream {

   protected:
      OutputStream() {
      }

   public:
      virtual ~OutputStream() {
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
      virtual void write(const char *pbBuffer, size_t cbBuffer) /* throws Error */ = 0;

      /**
       * Flush any cached/buffered data to the stream.
       */
      virtual void flush() /* throws Error */ = 0;

      /**
       * Close the stream.
       */
      virtual void close() = 0;

   private:
      // disable copy constructor and assignment
      //
      OutputStream(OutputStream &out);
      OutputStream &operator=(OutputStream &out);
   };
}

#endif /* OutputStream_H */
