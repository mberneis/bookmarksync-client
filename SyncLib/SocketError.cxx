/*
 * SyncLib/SocketError.cxx
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>

#include "SocketError.h"
#include "util.h"
#include "text.h"

using namespace syncit;

SocketError::SocketError(const string &function) : FunctionError(function) {
   m_error = WSAGetLastError();
}

BaseError *SocketError::newclone() const {
   return NEW SocketError(*this);
}

#define case_WSA0(n)    case WSA##n: psz = #n; break
#define case_WSA1(n,s)  case WSA##n: psz = s; break

/* virtual */
size_t SocketError::format(char *pach, size_t cch) {
   char achBuf[64];
   const char *psz;

   switch (m_error) {
      case_WSA0(EALREADY);
      case_WSA0(EMSGSIZE);
      case_WSA0(ENOPROTOOPT);
      case_WSA0(EPROTONOSUPPORT);
      case_WSA0(ESOCKTNOSUPPORT);
      case_WSA0(EPFNOSUPPORT);
      case_WSA0(EAFNOSUPPORT);
      case_WSA0(ENOBUFS);
      case_WSA0(ETOOMANYREFS);
      case_WSA0(ELOOP);
      case_WSA0(EPROCLIM);
      case_WSA0(EUSERS);
      case_WSA0(EDQUOT);
      case_WSA0(ESTALE);
      case_WSA0(EREMOTE);
      case_WSA0(EDISCON);
      case_WSA0(SYSNOTREADY);
      case_WSA0(VERNOTSUPPORTED);

      case_WSA1(EACCES,          "Permission denied");
      case_WSA1(EFAULT,          "Bad memory address");
      case_WSA1(EMFILE,          "Too many open files");
      case_WSA1(ENOTEMPTY,       "Directory not empty");
      case_WSA1(EINTR,           "Interrupted system call");
      case_WSA1(EBADF,           "Bad file number");
      case_WSA1(EINVAL,          "Invalid argument");
      case_WSA1(EWOULDBLOCK,     "Operation would block");
      case_WSA1(EINPROGRESS,     "Operation in progress");
      case_WSA1(ENOTSOCK,        "Not a socket");
      case_WSA1(EDESTADDRREQ,    "Destination address required");
      case_WSA1(EPROTOTYPE,      "Protocol is wrong type for socket");
      case_WSA1(EOPNOTSUPP,      "Operation not supported on socket");
      case_WSA1(EADDRINUSE,      "Address already in use");
      case_WSA1(EADDRNOTAVAIL,   "Address is not available");
      case_WSA1(ENETDOWN,        "Network is down");
      case_WSA1(ENETUNREACH,     "ICMP network unreachable");
      case_WSA1(ENETRESET,       "Network was reset");
      case_WSA1(ECONNABORTED,    "Software caused connection abort");
      case_WSA1(ECONNRESET,      "Connection reset by peer");
      case_WSA1(EISCONN,         "already connected");
      case_WSA1(ENOTCONN,        "Socket is not connected");
      case_WSA1(ESHUTDOWN,       "Socket has been shut down");
      case_WSA1(ETIMEDOUT,       "Operation timed out");
      case_WSA1(ECONNREFUSED,    "Connection refused");
      case_WSA1(ENAMETOOLONG,    "Host name is too long");
      case_WSA1(EHOSTDOWN,       "Host is down");
      case_WSA1(EHOSTUNREACH,    "Host is unreachable");
      case_WSA1(NOTINITIALISED,  "WinSock hasn't been initialized");

      case_WSA1(HOST_NOT_FOUND,  "Host not found");
      case_WSA1(TRY_AGAIN,       "Non-Authoritative, Host not found, or SERVERFAIL");
      case_WSA1(NO_RECOVERY,     "Non recoverable errors, FORMERR, REFUSED, NOTIMP");
      case_WSA1(NO_DATA,         "Valid name, no data record of requested type");

      default:
         wsprintfA(achBuf, "socket error %d", m_error);
         psz = achBuf;
   }

   return bufcopy(psz, pach, cch);
}
