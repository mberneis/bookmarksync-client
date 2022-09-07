/*
 * SyncLib/Socket.h
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
#ifndef Socket_H
#define Socket_H

#include <cassert>

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>
#include <winsock.h>

#include "InputStream.h"
#include "OutputStream.h"
#include "SocketError.h"

#include "util.h"

namespace syncit {

   ///////////////////////
   // Table of Contents...
   //
   class SocketInit;
   class SocketInputStream;
   class SocketOutputStream;
   class Socket;
   // ...table of contents
   ///////////////////////

   class SocketInit {

   public:
      SocketInit();
      ~SocketInit();

      WSADATA wsadata;
   };

   class SocketInputStream : public InputStream {

   public:
      SocketInputStream(Socket *p) {
         m_p = p;
      }

      virtual ~SocketInputStream() {
      }

      size_t read(char *pbBuffer, size_t cbBuffer) /* throws Error */;

      void close();

   private:
      Socket *m_p;

      // disable copy constructor and assignment
      SocketInputStream(SocketInputStream &rhs);
      SocketInputStream &operator=(SocketInputStream &rhs);
   };

   class SocketOutputStream : public OutputStream {

   public:
      SocketOutputStream(Socket *p) {
         m_p = p;
      }

      virtual ~SocketOutputStream() {
      }

      void write(const char *pb, size_t cb) /* throws Error */;

      /**
       * Flush any data in the write buffer.
       *
       * @return true if operation succeeded, false otherwise
       *
       * @require isConnected
       */
      void flush() {
      }

      void close();

   private:
      Socket *m_p;

      // disable copy constructor and assignment
      SocketOutputStream(SocketOutputStream &rhs);
      SocketOutputStream &operator=(SocketOutputStream &rhs);
   };

   /**
    * A Socket is a WinSock socket in combination
    * with a read/write buffer.
    */
   class Socket {

   public:
      /**
       * Initialize a Socket structure.
       *
       * @ensure !isConnected
       */
      Socket();

      virtual ~Socket();

      static SocketInit init;

      /**
       * @return true if connected, false otherwise
       */
      bool isConnected() const;

      /**
       * Connect the socket connection to a remote host
       *
       * @param pszHostname  a DNS hostname to connect to
       * @param port         the TCP port number to connect to
       * @return true on success, false otherwise
       *
       * @require !isConnected
       * @ensure  result == false || isConnected
       */
      virtual void connect(const char *pszHostname, unsigned short port);

      InputStream *getInputStream() {
         return m_in;
      }

      OutputStream *getOutputStream() {
         return m_out;
      }

      /**
       * Flush and close the socket connection.
       *
       * @require isConnected
       * @ensure !isConnected
       */
      void close();

      int read(char *pb, size_t cb) const;

      void write(const char *pb, size_t cb) const;

      const string &getHostname() const {
         return m_hostname;
      }

   private:

      int m_s;
      string m_hostname;

      SocketInputStream *m_in;
      SocketOutputStream *m_out;

      // disable copy constructor and assignment
      //
      Socket(Socket &out);
      Socket &operator=(Socket &out);

   };

}

#endif /* Socket_H */
