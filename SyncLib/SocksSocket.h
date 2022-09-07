/*
 * SyncLib/SocksSocket.h
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
#ifndef SocksSocket_H
#define SocksSocket_H

#include "Socket.h"
#include "Errors.h"
#include "URL.h"

namespace syncit {
   class SocksSocket : public Socket {

   public:
      /**
       * @param url  a string of the form [socks:][//]{hostname}[:{port}]
       *             if port is not present, defaults to 1080
       */
      SocksSocket(const URL &url);

      virtual void connect(const char *pszHostname, unsigned short port);
      void connectv4(const char *pszHostname, unsigned short port);
      void connectv5(const char *pszHostname, unsigned short port);

   private:
      size_t readFully(char *pb, size_t cb) const;

   private:
      URL m_url;
   };

   class SocksError : public BaseError {
   public:
      enum ErrorCode {
         PrematureEOF = -2,
         UnknownVersion = -1,

         OK = 90
      };

      SocksError(ErrorCode errcode) : m_errcode(errcode) {
      }

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

      int getErrorCode() const {
         return m_errcode;
      }

   private:
      int m_errcode;
   };
}

#endif /* SocksSocket_H */
