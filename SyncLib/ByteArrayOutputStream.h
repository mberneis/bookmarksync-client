/*
 * SyncLib/ByteArrayOutputStream.h
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
 *    The ByteArrayOutputStream class defines an object that
 *    can maintain a rather large in-memory binary file
 */
#ifndef ByteArrayOutputStream_H
#define ByteArrayOutputStream_H

#include "OutputStream.h"

namespace syncit {

   /**
    * A ByteArrayOutputStream buffers up data to be sent
    * to a web server via an HTTP POST method.
    * <p>
    * The following headers are required:
    * <pre>
    * Content-type: application/x-www-form-urlencoded
    * Content-length: {the value of the contentLength property}
    * </pre>
    * <p>
    * Valid URL characters (alphanumeric, *, ., _, -) are passed
    * through, all others are encoded by their hex value like this:
    *   %XX
    * So '\n' maps to %0A, '\r' maps to %0D.
    */
   class ByteArrayOutputStream : public OutputStream {

   public:
      /**
       * Constructs a new ByteArrayOutputStream out of an uninitialized structure
       */
      ByteArrayOutputStream();

      /**
       * Release any internal storage used by the byte array.
       */
      virtual ~ByteArrayOutputStream();

      void reset();

      //////////////////
      // OutputStream...
      //
      virtual void write(const char *pbBuffer, size_t cbBuffer);

      virtual void flush();

      virtual void close();
      //
      // ...OutputStream
      //////////////////

      void write(char ch);

      /**
       * The getContentLength method returns the total number of bytes
       * contained in the buffer.
       *
       * @return the number of bytes
       */
      unsigned long getContentLength() const;

      /**
       * The writeTo method sends the entire contents of the
       * ByteArrayOutputStream to the output stream.
       *
       * @param out an output stream
       *
       * @return true if the write operation(s) succeeded, false on error
       */
      void writeTo(OutputStream *out) const /* throws Exception */;

      void copyInto(char *pb, size_t cb) const;

   private:
      size_t m_iBuffer;

      char **m_papbBuffers;
      int m_curBuffer, m_maxBuffers;

      char m_achBuffer[4096];

      // disable copy constructor and assignment
      //
      ByteArrayOutputStream(ByteArrayOutputStream &out);
      ByteArrayOutputStream &operator=(ByteArrayOutputStream &out);

   };

}

#endif /* ByteArrayOutputStream_H */
