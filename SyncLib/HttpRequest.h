/*
 * SyncLib/HttpRequest.h
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
 *    This acts as an input stream, putting a 4K buffer
 *    between the user of this class and another InputStream.
 */
#pragma warning( disable : 4786 )

#ifndef HttpRequest_H
#define HttpRequest_H

#include <string>
#include <map>

#include "BufferedInputStream.h"
#include "ByteArrayOutputStream.h"
#include "ConnectSettings.h"
#include "PrintWriter.h"
#include "Socket.h"
#include "URL.h"

namespace syncit {

   using std::map;

   class HttpProgressListener {
   protected:
      HttpProgressListener() {
      }

   public:
      virtual ~HttpProgressListener() {
      }

      virtual void progress(unsigned __int64 cbBytes) = 0;
   };

   class HttpRequest {

   protected:
      // abstract base class
      //
      HttpRequest() {
         m_pIn = NULL;
      }

   public:
      virtual ~HttpRequest();

      virtual void setAuthentication(const char *pszUsername,
                                     const char *pszPassword) = 0;

      /**
       * The send method issues the HTTP request.
       * The socket is connected, data is sent, and the response
       * is parsed.  The data of the response is left so the InputStream
       * returned by the getInputStream() method (below) can read it.
       *
       * @return an HTTP response code 100-599 if the request was *issued* successfully,
       *         or 0 for premature EOF, -1 for protocol error (no HTTP)
       *
       * @exception Win32Exception or SocketException on any read/write failure
       */
      virtual
      int send(const char *pszVerb,                   // required, "GET", "POST" etc.
               const URL &url,                        // complete URL
               ByteArrayOutputStream *data = NULL,    // data to send in "POST" or "PUT" request
               const char *pszMimeType = NULL) = 0;   // MIME type (defaults to application/x-www-form-urlencoded)

      virtual
      int send(const char *pszVerb,
               const URL &url,
               unsigned __int64 size,
               InputStream *pin,
               const char *pszMimeType = NULL,
               HttpProgressListener *pListener = NULL) = 0;   // MIME type (defaults to application/x-www-form-urlencoded)

      virtual void close() = 0;

      virtual bool isConnected() const = 0;

      /**
       * Get the differentiating text output from the send request.
       * May be NULL.
       */
      const char *getResponse() const {
         return m_response.c_str();
      }

      const string &getHeader(const string &s) const {
         static string blank = "";
         map<string, string>::const_iterator i = m_headers.find(s);

         if (i != m_headers.end()) {
            return i->second;
         }
         else {
            return blank;
         }
      }

      const string &getProxyAuthenticate() const {
         return getHeader("proxy-authenticate");
      }

      const string &getWWWAuthenticate() const {
         return getHeader("www-authenticate");
      }

      BufferedInputStream *getInputStream();

   protected:
      void putHeader(const char *psz);
      void putHeader(const string &name, const string &value);

      string m_response;

      map<string, string> m_headers;

      BufferedInputStream *m_pIn;

   private:
      // disable copy constructor and assignment
      HttpRequest(HttpRequest &rhs);
      HttpRequest &operator=(HttpRequest &rhs);
   };

   HttpRequest *NewHttpRequest();
   HttpRequest *NewHttpRequest(ConnectSetting f,
                               const char *psz);

   /* utility routine to copy a web document */
   void DownloadDocument(const URL &url, const char *pszPath);
}

#endif /* HttpRequest */
