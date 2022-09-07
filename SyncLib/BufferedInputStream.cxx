/*
 * SyncLib/BufferedInputStream.cxx
 * Copyright (C) 2003  SyncIT.com, Inc.
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
 * Description: BookmarkSync client software for Windows
 * Author:      Terence Way
 * Created:     October 1998
 * Modified:    September 2003 by Terence Way
 * E-mail:      mailto:tway@syncit.com
 * Web site:    http://www.syncit.com
 */
#include <cassert>

#include "BufferedInputStream.h"
#include "util.h"

using namespace syncit;

BufferedInputStream::BufferedInputStream(InputStream *in) {

   m_in = in;
   m_cb = m_i = 0;
}

/* virtual */
size_t BufferedInputStream::read(char *pbBuffer, size_t cbBuffer) {
   size_t rem = m_cb - m_i;

   /* First, copy the saved buffer into the user's buffer */
   if (cbBuffer <= rem) {
      u_memcpy(pbBuffer, m_ab + m_i, cbBuffer);
      m_i += cbBuffer;

      return cbBuffer;
   }
   else {
      u_memcpy(pbBuffer, m_ab + m_i, rem);

      m_cb = 0;
      m_i = 0;

      /* now fill the remainder directly from the input stream */
      return m_in->read(pbBuffer + rem,
                        cbBuffer - rem) + rem;
   }
}

/* virtual */
void BufferedInputStream::close() /* throws IOError */ {
   m_in->close();
}

int BufferedInputStream::readx() /* throws IOError */ {
   if (fill()) {
      return m_ab[m_i++];
   }
   else {
      return -1;
   }
}

bool BufferedInputStream::fill() /* throws IOError */ {
   assert(m_i == m_cb);

   m_i = 0;
   m_cb = m_in->read((char *) m_ab, sizeof(m_ab));

   return m_cb > 0;
}

bool BufferedInputStream::readLine(char *pach, size_t cch) /* throws Error */ {

   char *p = pach;
   int ch = read();

   assert(cch > 1);

   // leave one left for null termination
   cch--;

   if (ch == -1) {
      return false;
   }
   else {
      while (cch > 0 && ch != -1 && ch != '\n') {
         *p++ = (char) ch;
         cch--;
         ch = read();
      }

      if (p > pach && p[-1] == '\r')
         p--;

      // cch == 0 || ch == -1 || ch == '\n'
      *p++ = '\0';

      return true;
   }
}

