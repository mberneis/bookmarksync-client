/*
 * SyncLib/ByteArrayOutputStream.cxx
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
 *    can maintain a rather large (0..100K bytes) in-memory
 *    data file.
 */

#include <cassert>
#include <cstdlib>
#include <cctype>

#include "ByteArrayOutputStream.h"

#include "util.h"

using namespace syncit;

ByteArrayOutputStream::ByteArrayOutputStream() {
   m_iBuffer = m_curBuffer = 0;

   m_maxBuffers = 16;

   m_papbBuffers = (char **) u_calloc(m_maxBuffers, sizeof(char *));
   m_papbBuffers[0] = m_achBuffer;
}

ByteArrayOutputStream::~ByteArrayOutputStream() {
   reset();

   u_free(m_papbBuffers);
}

void ByteArrayOutputStream::reset() {
   int i;

   for (i = 1; i <= m_curBuffer; i++) {
      u_free(m_papbBuffers[i]);
   }

   m_curBuffer = m_iBuffer = 0;

   assert(getContentLength() == 0);
}

void ByteArrayOutputStream::write(const char *pbBuffer, size_t cbBuffer) {
   for (size_t i = 0; i < cbBuffer; i++) {
      write(pbBuffer[i]);
   }
}

void ByteArrayOutputStream::write(char c) {

#ifndef NDEBUG
   unsigned long ul = getContentLength();
#endif /* NDEBUG */

   if (m_iBuffer == sizeof(m_achBuffer)) {
      if (++m_curBuffer == m_maxBuffers) {
         m_maxBuffers = (m_maxBuffers * 3) / 2;
         m_papbBuffers = (char **) u_realloc(m_papbBuffers,
                                             m_maxBuffers * sizeof(char *));
      }

      // m_curBuffer < m_maxBuffers

      m_papbBuffers[m_curBuffer] = (char *) u_malloc(sizeof(m_achBuffer));
      m_iBuffer = 0;
   }

   m_papbBuffers[m_curBuffer][m_iBuffer++] = c;

   assert(getContentLength() == ul + 1);
}

void ByteArrayOutputStream::flush() {
}

void ByteArrayOutputStream::close() {
}

unsigned long ByteArrayOutputStream::getContentLength() const {
   return m_curBuffer * sizeof(m_achBuffer) + m_iBuffer;
}

void ByteArrayOutputStream::writeTo(OutputStream *out) const {
   int i;

   for (i = 0; i < m_curBuffer; i++) {
      out->write(m_papbBuffers[i], sizeof(m_achBuffer));
   }

   if (m_iBuffer > 0) {
      out->write(m_papbBuffers[i], m_iBuffer);
   }
}

void ByteArrayOutputStream::copyInto(char *pb, size_t cb) const {
   int i = 0;

   while (i < m_curBuffer && cb >= sizeof(m_achBuffer)) {
      u_memcpy(pb, m_papbBuffers[i], sizeof(m_achBuffer));
      pb += sizeof(m_achBuffer);
      cb -= sizeof(m_achBuffer);

      i++;
   }

   if (m_iBuffer > 0 && cb >= m_iBuffer) {
      u_memcpy(pb, m_papbBuffers[i], m_iBuffer);
   }

}
