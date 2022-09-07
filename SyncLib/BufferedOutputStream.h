/*
 * SyncLib/BufferedOutputStream.h
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
 *    This acts as an output stream, putting a 4K buffer
 *    between the user of this class and another OutputStream.
 */
#ifndef BufferedOutputStream_H
#define BufferedOutputStream_H

#include <cassert>

#include "OutputStream.h"

namespace syncit {

   class BufferedOutputStream : public OutputStream {

   public:
      BufferedOutputStream(OutputStream *pout);

      virtual ~BufferedOutputStream() {
      }

      void write(int ch) {
         assert(-128 <= ch && ch < 256);

         m_ab[m_i++] = (char) ch;
         if (m_i == sizeof(m_ab))
            flush();
      }

      virtual void write(const char *pbBuffer, size_t cbBuffer);
      virtual void flush();
      virtual void close();

   private:
      OutputStream *m_out;

      char m_ab[4096];
      size_t m_i;

      // Disable copy constructor and assignment
      BufferedOutputStream(BufferedOutputStream &rhs);
      BufferedOutputStream &operator=(BufferedOutputStream &rhs);

   };

}

#endif /* BufferedOutputStream_H */
