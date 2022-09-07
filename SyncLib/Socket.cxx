/*
 * SyncLib/Socket.cxx
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
 *    The Socket class defines an object that
 *    maintains Winsock TCP connections to a remote server.
 */
#include <cassert>

#include <winsock.h>

#include "Socket.h"

#include "util.h"

using namespace syncit;

SocketInit::SocketInit() {
   int wsaError = ::WSAStartup(MAKEWORD(1, 1), &wsadata);

   if (wsaError != 0) {
      throw SocketError("WSAStartup", wsaError);
   }
}

SocketInit::~SocketInit() {
   ::WSACleanup();
}

size_t SocketInputStream::read(char *pbBuffer, size_t cbBuffer) {
   int n = m_p->read(pbBuffer, cbBuffer);

   if (n == SOCKET_ERROR) {
      throw NetError(NetError::Read, m_p->getHostname(), SocketError("recv"));
   }

   return (size_t) n;
}

void SocketInputStream::close() {
   m_p->close();
}

void SocketOutputStream::write(const char *pb, size_t cb) {
   m_p->write(pb, cb);
}

void SocketOutputStream::close() {
   m_p->close();
}

SocketInit Socket::init;

Socket::Socket() {
   m_s = -1;

   m_in = NEW SocketInputStream(this);
   m_out = NEW SocketOutputStream(this);
}

Socket::~Socket() {
   if (m_s != -1) {
      ::closesocket(m_s);
      m_s = -1;
   }

   delete m_in;
   delete m_out;
}

bool Socket::isConnected() const {
   return m_s != -1;
}

/* virtual */
void Socket::connect(const char *pszHostname, unsigned short port) {

   struct in_addr addrs[16];
   int num_addrs = 0;

   assert(!isConnected());

   // Step 1...
   // Resolve the string hostname into a set of IP addresses
   // This can be done either by number (inet_addr returns something
   // other than INADDR_NONE), or by name (gethostbyname returns
   // something other than NULL).  If neither works, throw an
   // UnknownHostError
   //
   {
      unsigned long ulAddress = inet_addr(pszHostname);

      if (ulAddress == INADDR_NONE) {
         struct hostent *phe = ::gethostbyname(pszHostname);

         if (phe == NULL) {
            throw UnknownHostError(pszHostname, SocketError("gethostbyname"));
         }

         while (num_addrs < ELEMENTS(addrs) &&
                phe->h_addr_list[num_addrs] != NULL) {
            addrs[num_addrs] = *(struct in_addr *) phe->h_addr_list[num_addrs];
            num_addrs++;
         }

         // num_addrs == ELEMENTS(addrs) || phe->... == NULL
      }
      else {
         addrs[num_addrs++].s_addr = ulAddress;
      }
   }
   // addrs[0..num_addrs] is now filled with IP addresses

   // Step 2.
   // Create the socket and initialize the sockaddr_in
   // structure.  If the socket cannot be created, throw
   // a SocketError.
   //
   m_s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

   if (m_s == -1) {
      throw SocketError("socket");
   }

   struct sockaddr_in sin;
   sin.sin_family = AF_INET;
   sin.sin_port = htons(port);

   // Step 3.
   // Try to connect to each IP address in turn.
   //
   bool done = false;
   int iError = 0, i = 0;

   while (!done && i < num_addrs) {
      sin.sin_addr = addrs[i];

      if (::connect(m_s, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
         iError = ::WSAGetLastError();
      }
      else {
         done = true;
      }

      i++;
   }

   if (!done) {
      ::closesocket(m_s);
      m_s = -1;
      throw NetError(NetError::Connect, pszHostname, SocketError("connect", iError));
   }

   assert(isConnected());
}

int Socket::read(char *pb, size_t cb) const {
   return ::recv(m_s, pb, cb, 0);
}

void Socket::write(const char *pb, size_t cb) const {
   assert(isConnected());

   // loop invariant: ok, cb > 0
   // loop variant: !ok || cb is always decreasing
   //
   while (cb > 0) {
      int r = ::send(m_s, pb, cb, 0);

      if (r == 0) {
         throw NetError(NetError::Write, m_hostname, SocketError("send"));
      }

      pb += r;
      cb -= r;
   }
}

void Socket::close() {
   int s = m_s;

   assert(isConnected());

   m_s = -1;

   if (::closesocket(s) == -1) {
      throw NetError(NetError::Close, m_hostname, SocketError("closesocket"));
   }
}
