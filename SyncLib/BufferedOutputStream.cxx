/*
 * SyncLib/BufferedOutputStream.cxx
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
#include "BufferedOutputStream.h"

#include "util.h"

using namespace syncit;

BufferedOutputStream::BufferedOutputStream(OutputStream *pout) {
   m_out = pout;
   m_i  = 0;
}

/* virtual */
void BufferedOutputStream::write(const char *pbBuffer,
                                 size_t cbBuffer) /* throws Exception */ {
   size_t rem = sizeof(m_ab) - m_i;

   if (cbBuffer < rem) {
      u_memcpy(m_ab + m_i, pbBuffer, cbBuffer);
      m_i += cbBuffer;
   }
   else if (m_i == 0) {
      // nothing in buffer, and cbBuffer >= rem
      m_out->write(pbBuffer, cbBuffer);
   }
   else if (cbBuffer == rem) {
      u_memcpy(m_ab + m_i, pbBuffer, rem);
      m_i = sizeof(m_ab);
      flush();
   }
   else {
      flush();
      m_out->write(pbBuffer, cbBuffer);
   }
}

/* virtual */
void BufferedOutputStream::flush() /* throws IOException */ {
   if (m_i > 0) {
      m_out->write(m_ab, m_i);

      m_i = 0;
   }

   m_out->flush();
}

/* virtual */
void BufferedOutputStream::close() /* throws Exception */ {
   flush();
   m_out->close();
}
