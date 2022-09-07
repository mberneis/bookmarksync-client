/*
 * SyncLib/ISAPIInputStream.cxx
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
#include "util.h"
#include "ISAPIInputStream.h"
#include "Errors.h"

using namespace syncit;

/**
 * Read into a buffer
 *
 * @param pbBuffer pointer to byte buffer
 * @param cbBuffer size of buffer pointed to by pbBuffer
 * @return the number of bytes read, 1..cbBuffer or 0 on EOF
 */
/* virtual */
size_t ISAPIInputStream::read(char *pbBuffer, size_t cbBuffer) { /* throws IOError */
   size_t r;

   // IF we're still within the cbAvailable range...
   //
   if (m_dwOffset < m_pecb->cbAvailable) {

      // then copy from the available range
      //
      r = m_pecb->cbAvailable - m_dwOffset;

      if (r > cbBuffer) {
         r = cbBuffer;
      }

      u_memcpy(pbBuffer, m_pecb->lpbData + m_dwOffset, r);
   }

   // ELSE if there's more available for ReadClient
   //
   else if (m_dwOffset < m_pecb->cbTotalBytes) {

      // then call the ReadClient callback to get it
      //
      DWORD dwRead = m_pecb->cbTotalBytes - m_dwOffset;

      if (dwRead > cbBuffer) {
         dwRead = cbBuffer;
      }

      if (!m_pecb->ReadClient(m_pecb->ConnID, pbBuffer, &dwRead)) {
         throw Win32Error("ISAPI ReadClient");
      }

      r = dwRead;
   }
   else {
      r = 0;
   }

   m_dwOffset += r;

   return r;
}

/**
 * Close the stream.
 */
/* virtual */
void ISAPIInputStream::close() {
}
