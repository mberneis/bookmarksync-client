/*
 * SyncLib/SocksSocket.cxx
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
#include "SocksSocket.h"
#include "SocketError.h"
#include "URL.h"
#include "text.h"

using namespace syncit;

SocksSocket::SocksSocket(const URL &url) : m_url(url) {
}

/* virtual */
void SocksSocket::connect(const char *pszHostname, unsigned short port) {
   connectv4(pszHostname, port);
}

void SocksSocket::connectv4(const char *pszHostname, unsigned short port) {
   char achHost[256];
   char achRequest[512];

   m_url.getHost(achHost, sizeof(achHost));
   Socket::connect(achHost, m_url.getPort(1080));

   achRequest[0] = 4;   // version 4 of SOCKS
   achRequest[1] = 1;   // CONNECT request
   *((unsigned short *) (achRequest + 2)) = htons(port);
   *((unsigned long  *) (achRequest + 4)) = inet_addr(pszHostname);

   int i = min(m_url.getFileLen(), sizeof(achRequest) - 9);
   u_memcpy(achRequest + 8, m_url.getFileSz(), i);
   achRequest[8 + i] = 0;
   write(achRequest, i + 9);

   char achResponse[8];

   if (readFully(achResponse, sizeof(achResponse)) != sizeof(achResponse)) {
      // could not read a full response
      throw SocksError(SocksError::PrematureEOF);
   }

   if (achResponse[0] != 0) {
      throw SocksError(SocksError::UnknownVersion);
   }

   if (achResponse[1] != 90) {
      throw SocksError((SocksError::ErrorCode) achResponse[1]);
   }
}

size_t SocksSocket::readFully(char *pab, size_t cb) const {
   size_t offset = 0;

   while (offset < cb) {
      int cbRead = read(pab + offset, cb - offset);

      if (cbRead == -1) {
         throw SocketError("recv");
      }
      else if (cbRead == 0) {
         return offset;
      }
      else {
         offset += cbRead;
      }
   }

   return offset;
}

/* virtual */
size_t SocksError::format(char *pach, size_t cch) {
   const char *psz;
   char ach[1024];

   switch (m_errcode) {
      case PrematureEOF:
         psz = "premature EOF on SOCKS connection";
         break;

      case UnknownVersion:
         psz = "unknown SOCKS response version";
         break;

      case 91: // request rejected or failed
         psz = "SOCKS request rejected or failed";
         break;

      case 92: // request rejected becasue SOCKS server cannot connect to
               // identd on the client
         psz = "SOCKS request rejected because server cannot connect to identd on the client";
         break;

      case 93: // request rejected because the client program and identd
               // report different user-ids
         psz = "SOCKS request rejected because the client program and identd report different user-ids";
         break;

      default:
         wsprintfA(ach, "unknown SOCKS response code %d", m_errcode);
         psz = ach;
         break;
   }

   return bufcopy(psz, pach, cch);
}

/* virtual */
BaseError *SocksError::newclone() const {
   return NEW SocksError(*this);
}
