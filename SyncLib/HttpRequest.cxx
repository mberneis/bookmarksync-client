/*
 * SyncLib/HttpRequest.cxx
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
 *    The base class for any HttpRequest, whether WININET.DLL
 *    or raw.
 */
#include <cstdlib>

#include "HttpRequest.h"

#include "BufferedInputStream.h"
#include "BufferedOutputStream.h"
#include "FileOutputStream.h"
#include "ConnectSettings.h"
#include "PrintWriter.h"
#include "SocksSocket.h"
#include "log.h"
#include "Util.h"
#include "URL.h"
#include "WinInetHttpRequest.h"
#include "Base64.h"

#include "../ProductVersion.h"

using namespace syncit;

WinInet gwininet;

class DirectHttpRequest : public HttpRequest {

public:
   // only should be called by the HttpRequestFactory class,
   // or by derived classes
   //
   DirectHttpRequest(Socket *p) : m_pSocket(p) {
   }

   virtual ~DirectHttpRequest() {
      delete m_pSocket;
   }

   virtual void setAuthentication(const char *pszUsername,
                                  const char *pszPassword) {
      if (pszUsername != NULL && pszPassword != NULL) {
         char achBinary[1024];
         char achBase64[1024 * 4 / 3];

         wsprintfA(achBinary, "%s:%s", pszUsername, pszPassword);
         Base64Encode((const unsigned char *) achBinary, strlen(achBinary), achBase64);

         m_authorization = achBase64;
      }
      else {
         m_authorization = "";
      }
   }

   void printHeaders(PrintWriter *pw,
                     const char *pszVerb,
                     const URL &url) {
      char achHost[256];
      size_t cchHost = url.getHost(achHost, sizeof(achHost));

      pw->print(pszVerb);
      pw->write(' ');
      printResource(pw, url);
      pw->print(" HTTP/1.0\r\n");

      pw->print("Host: ");
      pw->write(achHost, cchHost);
      pw->print("\r\n");

      if (m_authorization.size() > 0) {
         string authenticate = getWWWAuthenticate();

         if (authenticate.size() == 0) {
            pw->print("Proxy-");
         }

         pw->print("Authorization: Basic ");
         pw->print(m_authorization);
         pw->print("\r\n");
      }

      pw->print("User-Agent: SyncIT/" SYNCIT_V_STR "\r\n");
   }

   void printData(PrintWriter *pw, ByteArrayOutputStream *pdata, const char *pszMimeType) {
      if (pdata != NULL) {
         pw->print("Content-type: ");
         pw->print(pszMimeType == NULL ? "application/x-www-form-urlencoded" : pszMimeType);
         pw->print("\r\nContent-length: ");
         pw->print(pdata->getContentLength());
         pw->print("\r\n\r\n");

         pdata->writeTo(pw);
      }
      else {
         pw->print("\r\n");
      }

      pw->flush();
      // ...request sent
      //////////////////
   }

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
            ByteArrayOutputStream *pdata,          // data to send in "POST" or "PUT request
            const char *pszMimeType) {             // MIME type (defaults to application/x-www-form-urlencoded)
      connect(url);

      //////////////////
      // Send request...
      //
      BufferedOutputStream bout(m_pSocket->getOutputStream());
      PrintWriter out(&bout);

      printHeaders(&out, pszVerb, url);
      printData(&out, pdata, pszMimeType);

#ifndef NLOG
      {
         Win32OutputStream o(GetLogHandle(), "log");
         BufferedOutputStream b(&o);
         PrintWriter w(&b);
         LogLock();
         Log("Sending request:\r\n");
         printHeaders(&w, pszVerb, url);
         printData(&w, pdata, pszMimeType);
         w.print("\r\n");
         w.flush();
         LogUnlock();
      }
#endif /* NLOG */

      return readResponse();
   }

   int readResponse() {
      unsigned long ulResult;

      ///////////////////
      // Read response...
      //
      m_response = "";

      char achBuffer[4096];

      delete m_pIn;
      m_pIn = NEW BufferedInputStream(m_pSocket->getInputStream());

      m_pIn->readLine(achBuffer, sizeof(achBuffer));

      const char *p = achBuffer;

      // parse HTTP/1.x y ...
      if (memcmp(p, "HTTP/1.", 7) == 0) {
         p += 7;

         strtoul(p, (char **) &p, 10);

         ulResult = strtoul(skipws(p), (char **) &p, 10);

         p = skipws(p);

         m_response = p;

         // go through header lines, looking for authentication
         // headers
         //
         while (m_pIn->readLine(achBuffer, sizeof(achBuffer)) && achBuffer[0] != '\0') {
            putHeader(achBuffer);
         }
      }
      else {
         ulResult = -1;
      }

      return ulResult;
   }

   virtual
   int send(const char *pszVerb,
            const URL &url,
            unsigned __int64 size,
            InputStream *pin,
            const char *pszMimeType,
            HttpProgressListener *pListener) {     // pointer to progress listener
      connect(url);

      //////////////////
      // Send request...
      //
      BufferedOutputStream bout(m_pSocket->getOutputStream());
      PrintWriter out(&bout);

      printHeaders(&out, pszVerb, url);
      out.print("Content-type: ");
      out.print(pszMimeType == NULL ? "application/x-www-form-urlencoded" : pszMimeType);
      out.print("\r\nContent-length: ");
      out.print(size);
      out.print("\r\n\r\n");

      unsigned __int64 cb = 0;
      while (size > 4096) {
         char ach[4096];

         int r = pin->read(ach, sizeof(ach));
         bout.write(ach, r);

         cb   += r;
         pListener->progress(cb);

         size -= r;
      }

      // size <= 4096
      while (size > 0) {
         char ach[4096];

         int r = pin->read(ach, (size_t) size);
         bout.write(ach, r);

         cb   += r;
         pListener->progress(cb);

         size -= r;
      }

      // size' == 0
      // cb    == size

      out.flush();

      return readResponse();
   }

   virtual void close() {
      m_pSocket->close();
   }

   virtual bool isConnected() const {
      return m_pSocket->isConnected();
   }

protected:
   virtual void connect(const URL &url) {
      char achHost[256];
      url.getHost(achHost, sizeof(achHost));
      m_pSocket->connect(achHost, url.getPort());
   }

   virtual void printResource(PrintWriter *pw, const URL &url) {
      pw->write('/');
      pw->write(url.getFileSz(), url.getFileLen());
   }

protected:
   Socket *m_pSocket;

   string m_authorization;       // Authorization: request header
};

class ProxyHttpRequest : public DirectHttpRequest {

public:
   ProxyHttpRequest(Socket *p, const URL &proxyUrl) : DirectHttpRequest(p), m_url(proxyUrl) {
   }

   virtual ~ProxyHttpRequest() {
   }

protected:
   virtual void connect(const URL &url) {
      char achHost[256];
      m_url.getHost(achHost, sizeof(achHost));
      m_pSocket->connect(achHost, m_url.getPort());
   }

   virtual void printResource(PrintWriter *pw, const URL &url) {
      pw->print("http://");
      pw->write(url.getHostSz(), url.getHostLen());
      pw->write(':');
      pw->print((unsigned long) url.getPort());
      pw->write('/');
      pw->write(url.getFileSz(), url.getFileLen());
   }

   URL m_url;
};

/* virtual */
HttpRequest::~HttpRequest() {
   delete m_pIn;
}

void HttpRequest::putHeader(const char *psz) {
   char achHeader[256];
   const char *p = strchr(psz, ':');

   if (p != NULL) {
      for (int i = 0; i < 256 && psz != p; i++) {
         achHeader[i] = tolower(*psz++);
      }

      // i == 256 || psz == p
      p = skipws(p + 1);

      putHeader(string(achHeader, i), p);
   }
}

void HttpRequest::putHeader(const string &name, const string &value) {
   m_headers[name] = value;
}

BufferedInputStream *HttpRequest::getInputStream() {
   return m_pIn;
}

static HttpRequest *MakeHttpRequest(ConnectSetting f) {
   char achBuf[4096];

   if (f == ConnectAutomatic && !IsDialInActive()) {
      throw BaseEvent(SYNCIT_NOT_DIALED_IN);
   }

   if (f == ConnectDirect) {
      return NEW DirectHttpRequest(NEW Socket());
   }
   else if (f == ConnectWinInet && gwininet.isLoaded()) {
      return NEW WinInetHttpRequest(&gwininet);
   }
   else {
      ConnectResult  r = GetConnectSettings(f, achBuf, sizeof(achBuf));

      switch (r & ConnectResultMask) {
         case ConnectEnabled:
            return NEW DirectHttpRequest(NEW Socket());

         case ConnectHttpUrl:
            return NEW ProxyHttpRequest(NEW Socket(), URL(achBuf));

         case ConnectSocksUrl:
            return NEW DirectHttpRequest(NEW SocksSocket(URL(achBuf, "socks:")));

         case ConnectConfigUrl:
            if (gwininet.isLoaded()) {
               return NEW WinInetHttpRequest(&gwininet);
            }
            break;
      }

      return NULL;
   }
}

HttpRequest *syncit::NewHttpRequest() {
   ConnectSetting f = GetCurrentConnectSetting();

   return MakeHttpRequest(f);
}

HttpRequest *syncit::NewHttpRequest(ConnectSetting f,
                                    const char *psz) {
   switch (f) {
      case ConnectHttp:
         return NEW ProxyHttpRequest(NEW Socket(), URL(psz));

      case ConnectSocks:
         return NEW DirectHttpRequest(NEW SocksSocket(URL(psz, "socks:")));
   }

   return MakeHttpRequest(f);
}

void syncit::DownloadDocument(const URL &url, const char *pszPath) {
   try {
      FileOutputStream f;

      if (f.open0(pszPath)) {
         HttpRequest *preq = NULL;

         try {
            preq = NewHttpRequest();

            if (preq->send("GET", url) == 200) {
               InputStream *pin = preq->getInputStream();
               char achBuf[4096];

               size_t r = pin->read(achBuf, sizeof(achBuf));
               while (r > 0) {
                  f.write(achBuf, r);
                  r = pin->read(achBuf, sizeof(achBuf));
               }
            }
         } catch (...) {
         }

         delete preq;
      }

      f.close();
   } catch (BaseError &) {
   }
}
