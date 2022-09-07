/*
 * SyncLib/BufferedInputStream.h
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
 *    A java class library -inspired utility class.
 *    This acts as an input stream, putting a 4K buffer
 *    between the user of this class and another InputStream.
 */
#ifndef BufferedInputStream_H
#define BufferedInputStream_H

#include <cassert>

#include "InputStream.h"
#include "Reader.h"

namespace syncit {

   class BufferedInputStream : public InputStream, public Reader {

   public:
      BufferedInputStream(InputStream *in);

      virtual ~BufferedInputStream() {
         // input stream pointed to may not be valid...
      }

      int read() {
         int result = m_i == m_cb ? readx() : m_ab[m_i++];

         assert(-1 <= result && result < 256);

         return result;
      }

      /**
       * Put back a character just read.  Only guaranteed to
       * work once, right after a character has been read by read()
       */
      void putback(char ch) {
         assert(m_i > 0);
         m_i--;
         assert(ch == (char) m_ab[m_i]);
      }

      virtual size_t read(char *pbBuffer, size_t cbBuffer);

      bool readLine(char *pachBuffer, size_t cchBuffer);

      virtual void close();

   protected:
      int readx();
      bool fill();

   private:
      InputStream *m_in;

      unsigned char m_ab[4096];
      size_t m_i, m_cb;

      // Disable copy constructor and assignment
      BufferedInputStream(BufferedInputStream &rhs);
      BufferedInputStream &operator=(BufferedInputStream &rhs);
   };

}

#endif /* BufferedInputStream_H */
