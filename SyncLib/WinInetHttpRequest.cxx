/*
 * SyncLib/WinInetHttpRequest.cxx
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
 * Last Modification: 5 Jan 1999
 *
 * Description:
 *    Dynamically loads the WININET.DLL, so our main image can
 *    load and run even if WININET.DLL isn't present.
 *
 *    We need 8 routines from WININET.DLL:
 *    1. InternetOpen
 *    2. InternetConnect
 *    3. InternetOpenRequest
 *    4. HttpOpenRequest
 *    5. HttpSendRequest
 *    6. HttpQueryInfo
 *    7. InternetCloseHandle
 *
 *    The problem is that not all Win95 installations have the WININET.DLL,
 *    which is included in IE 3.0, and is updated by later versions.  The
 *    DLL is 500K, and including it in the SyncIT distribution would increase
 *    its size dramatically, and also subject us to versioning problems.
 *
 *    Much easier to dynamically load it (LoadLibrary, FindProcAddress, etc)
 *    and call it from a separate class: this stub class.
 *
 * Change History
 *    5 Jan 1998     TW
 *       Added InternetCheckConnection to try to
 *       disable autodial
 */
#include "WinInetHttpRequest.h"

#include "../ProductVersion.h"

namespace syncit {

   static const char MIME_TYPE[] = "application/x-www-form-urlencoded";

   class WinInetInputStream : public InputStream {

   public:
      WinInetInputStream(WinInet *p,
                         HINTERNET hHttpRequest) {
         m_p = p;
         m_hHttpRequest = hHttpRequest;
      }

      virtual ~WinInetInputStream() {
      }

      /**
       * Read into a buffer
       *
       * @param pbBuffer pointer to byte buffer
       * @param cbBuffer size of buffer pointed to by pbBuffer
       * @return the number of bytes read, 1..cbBuffer or 0 on EOF
       */
      virtual size_t read(char *pbBuffer, size_t cbBuffer) /* throws IOError */ {
         DWORD dwBytesRead;

         if (!m_p->internetReadFile(m_hHttpRequest,    // hFile -- previously returned from HttpOpenRequest
                                    pbBuffer,          // lpBuffer
                                    cbBuffer,          // cbBuffer
                                    &dwBytesRead)) {   // lpNumberOfBytesRead
            throw Win32Error("InternetReadFile");
         }

         return dwBytesRead;
      }

      /**
       * Close the stream.
       */
      virtual void close() {
         m_p->internetCloseHandle(m_hHttpRequest);
         m_hHttpRequest = NULL;
      }

   private:
      WinInet *m_p;

      HINTERNET m_hHttpRequest;

      // disable copy constructor and assignment
      //
      WinInetInputStream(WinInetInputStream &out);
      WinInetInputStream &operator=(WinInetInputStream &out);
   };

}

using namespace syncit;

WinInet::WinInet() {
   m_dll = ::LoadLibrary(TEXT("WININET"));

   if (m_dll != NULL) {
      m_open      = (PfnInternetOpenA)       ::GetProcAddress(m_dll, "InternetOpenA");
      m_connect   = (PfnInternetConnectA)    ::GetProcAddress(m_dll, "InternetConnectA");
      m_request   = (PfnHttpOpenRequestA)    ::GetProcAddress(m_dll, "HttpOpenRequestA");
      m_send      = (PfnHttpSendRequestA)    ::GetProcAddress(m_dll, "HttpSendRequestA");
      m_httpQuery = (PfnHttpQueryInfoA)      ::GetProcAddress(m_dll, "HttpQueryInfoA");
      m_read      = (PfnInternetReadFile)    ::GetProcAddress(m_dll, "InternetReadFile");
      m_close     = (PfnInternetCloseHandle) ::GetProcAddress(m_dll, "InternetCloseHandle");

      m_sendex    = (PfnHttpSendRequestExA)  ::GetProcAddress(m_dll, "HttpSendRequestExA");
      m_write     = (PfnInternetWriteFile)   ::GetProcAddress(m_dll, "InternetWriteFile");
      m_endReq    = (PfnHttpEndRequest)      ::GetProcAddress(m_dll, "HttpEndRequest");

    //m_check     = (PfnInternetCheckConnection) ::GetProcAddress(m_dll, "InternetCheckConnectionA");

      if (m_open == NULL || m_connect == NULL || m_request == NULL || m_send == NULL || m_httpQuery == NULL ||
          m_read == NULL || m_close == NULL) {
         ::FreeLibrary(m_dll);
         m_dll = NULL;
      }

   }
}

WinInetHttpRequest::WinInetHttpRequest(WinInet *pdll) {
   m_pdll = pdll;

   m_hInternetSession = NULL;
   m_hHttpSession = NULL;
   m_hHttpRequest = NULL;
   m_pszUsername = m_pszPassword = NULL;
   m_pWinIn = NULL;
}

WinInetHttpRequest::~WinInetHttpRequest() {
   delete m_pWinIn;

   u_free0(m_pszUsername);
   u_free0(m_pszPassword);

   close();
}

/* virtual */
void WinInetHttpRequest::setAuthentication(const char *pszUsername,
                               const char *pszPassword) {
   m_pszUsername = strrealloc(m_pszUsername, pszUsername);
   m_pszPassword = strrealloc(m_pszPassword, pszPassword);
}

/* virtual */
void WinInetHttpRequest::close() {
   if (m_hHttpRequest != NULL) m_pdll->internetCloseHandle(m_hHttpRequest);
   if (m_hHttpSession != NULL) m_pdll->internetCloseHandle(m_hHttpSession);
   if (m_hInternetSession != NULL) m_pdll->internetCloseHandle(m_hInternetSession);

   m_hInternetSession = NULL;
   m_hHttpSession = NULL;
   m_hHttpRequest = NULL;
}

/* virtual */
bool WinInetHttpRequest::isConnected() const {
   return m_hHttpRequest != NULL;
}

/* virtual */
void WinInetHttpRequest::connect(const URL &url) {

   char achHost[256];

   url.getHost(achHost, sizeof(achHost));

   m_hInternetSession = m_pdll->internetOpen("SyncIT/" SYNCIT_V_STR,         // pszAgent
                                             INTERNET_OPEN_TYPE_PRECONFIG,   // dwAccessType
                                             NULL,                           // pszProxyName
                                             NULL,                           // pszProxyBypass
                                             0);                             // dwFlags

   if (m_hInternetSession == NULL) {
      throw NetError(NetError::Connect, achHost, Win32Error("InternetOpen"));
   }

   m_hHttpSession = m_pdll->internetConnect(m_hInternetSession,      // hInternetSession -- returned by InternetOpen
                                            achHost,                 // pszServerName
                                            url.getPort(),           // nServicePort -- HTTP port 80
                                            m_pszUsername,           //pszUsername,
                                            m_pszPassword,           //pszPassword,
                                            INTERNET_SERVICE_HTTP,   // dwService
                                            0,                       // dwFlags
                                            0);                      // dwContext

   if (m_hHttpSession == NULL) {
      throw NetError(NetError::Connect, achHost, Win32Error("InternetConnect"));
   }
}

string WinInetHttpRequest::getHeader(int i) const {
   char  achBuffer[4096];
   DWORD dwBytesRead = sizeof(achBuffer);

   if (m_pdll->httpQueryInfo(m_hHttpRequest,
                             i,
                             achBuffer,
                             &dwBytesRead,
                             NULL)) {
      return string(achBuffer, dwBytesRead);
   }
   else {
      return "";
   }

}

/* virtual */
int WinInetHttpRequest::send(const char *pszVerb,
                             const URL &url,
                             ByteArrayOutputStream *data,
                             const char *pszMimeType) {
   // hInternetSession is open
   // hHttpSession is open

   connect(url);

   m_hHttpRequest = m_pdll->httpOpenRequest(m_hHttpSession,             // hHttpSession -- returned by InternetConnect
                                            pszVerb,                    // pszVerb -- "GET" "POST" or "PUT"
                                            url.getFileSz(),            // pszObjectName
                                            "HTTP/1.0",                 // pszVersion
                                            NULL,                       // pszReferrer
                                            NULL,                       // pszAcceptTypes
                                            INTERNET_FLAG_RELOAD
                                            | INTERNET_FLAG_DONT_CACHE
                                            | INTERNET_FLAG_NO_CACHE_WRITE, // dwFlags
                                            0);                         // dwContext

   if (m_hHttpRequest == NULL) {
      throw Win32Error("HttpOpenRequest");
   }

   // hInternetSession is open
   // hHttpSession is open
   // hHttpRequest is open

   DWORD cbData = data == NULL ? 0 : data->getContentLength();
   char *pbData;

   if (data == NULL) {
      pbData = NULL;
   }
   else {
      pbData = NEW char[cbData];
      data->copyInto(pbData, cbData);
   }

   char achHeaders[32768];

   if (pszMimeType == NULL)
      pszMimeType = MIME_TYPE;

   wsprintfA(achHeaders, "Content-type: %s\r\n", pszMimeType);

   if (!m_pdll->httpSendRequest(m_hHttpRequest,         // hHttpRequest -- returned by HttpOpenRequest
                                achHeaders,             // pszHeaders OPTIONAL
                                -1,                     // dwHeadersLength (-1 for ASCIZ)
                                pbData,                 // pOptional -- contents of POST or PUT
                                cbData)) {              // dwOptionalLength
      Win32Error e("HttpSendRequest");
      delete[] pbData;
      throw e;
   }
   else {
      delete[] pbData;
   }

   DWORD dwResult, dwBytesRead;
   
   dwBytesRead = sizeof(dwResult);
   if (!m_pdll->httpQueryInfo(m_hHttpRequest,
                              HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE,
                              (void *) &dwResult,
                              &dwBytesRead,
                              NULL)) {
      throw Win32Error("HttpQueryInfo");
   }

   m_response = getHeader(HTTP_QUERY_STATUS_TEXT);

   dwBytesRead = sizeof(achHeaders);
   if (m_pdll->httpQueryInfo(m_hHttpRequest, HTTP_QUERY_RAW_HEADERS,
                             achHeaders, 
                             &dwBytesRead,
                             NULL)) {

      char *p = achHeaders;

      while (*p) {
         putHeader(p);

         p += strlen(p) + 1;
      }
   }

   delete m_pWinIn;
   delete m_pIn;
   m_pWinIn = NEW WinInetInputStream(m_pdll, m_hHttpRequest);
   m_pIn = NEW BufferedInputStream(m_pWinIn);

   return dwResult;
}

/* virtual */
int WinInetHttpRequest::send(const char *pszVerb,
                             const URL &url,
                             unsigned __int64 size,
                             InputStream *pin,
                             const char *pszMimeType,
                             HttpProgressListener *pListener) {
   // hInternetSession is open
   // hHttpSession is open

   connect(url);

   m_hHttpRequest = m_pdll->httpOpenRequest(m_hHttpSession,             // hHttpSession -- returned by InternetConnect
                                            pszVerb,                    // pszVerb -- "GET" "POST" or "PUT"
                                            url.getFileSz(),            // pszObjectName
                                            "HTTP/1.0",                 // pszVersion
                                            NULL,                       // pszReferrer
                                            NULL,                       // pszAcceptTypes
                                            INTERNET_FLAG_RELOAD
                                            | INTERNET_FLAG_DONT_CACHE
                                            | INTERNET_FLAG_NO_CACHE_WRITE, // dwFlags
                                            0);                         // dwContext

   if (m_hHttpRequest == NULL) {
      throw Win32Error("HttpOpenRequest");
   }

   // hInternetSession is open
   // hHttpSession is open
   // hHttpRequest is open

   char achHeaders[32768];
   DWORD dwLength;

   if (pszMimeType == NULL)
	  pszMimeType = MIME_TYPE;

   dwLength = wsprintfA(achHeaders, "Content-type: %s\r\n", pszMimeType);

   INTERNET_BUFFERS buf;
   memset(&buf, 0, sizeof(buf));
   buf.dwStructSize = sizeof(buf);
 //buf.Next = NULL;
   buf.lpcszHeader = achHeaders;
   buf.dwHeadersLength = buf.dwHeadersTotal = dwLength;
 //buf.lpvData = NULL;
 //buf.dwBufferLength = 0;
   buf.dwBufferTotal = size;
 //buf.dwOffsetHigh = buf.dwOffsetLow = 0;

   if (!m_pdll->httpSendRequestEx(m_hHttpRequest,         // hHttpRequest -- returned by HttpOpenRequest
                                  &buf,
                                  NULL,
                                  0,
                                  0)) {
      throw Win32Error("HttpSendRequest");
   }

   unsigned __int64 cb = 0;
   while (size > 4096) {
      char ach[4096];
      DWORD dwWritten;

      int r = pin->read(ach, sizeof(ach));
      m_pdll->internetWriteFile(m_hHttpRequest, ach, r, &dwWritten);

      cb   += dwWritten;
      pListener->progress(cb);

      size -= dwWritten;
   }

   // size <= 4096
   while (size > 0) {
      char ach[4096];
      DWORD dwWritten;

      int r = pin->read(ach, (size_t) size);
      m_pdll->internetWriteFile(m_hHttpRequest, ach, r, &dwWritten);

      cb   += dwWritten;
      pListener->progress(cb);

      size -= dwWritten;
   }

   DWORD dwResult, dwBytesRead;
   
   dwBytesRead = sizeof(dwResult);
   if (!m_pdll->httpQueryInfo(m_hHttpRequest,
                              HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE,
                              (void *) &dwResult,
                              &dwBytesRead,
                              NULL)) {
      throw Win32Error("HttpQueryInfo");
   }

   m_response = getHeader(HTTP_QUERY_STATUS_TEXT);

   dwBytesRead = sizeof(achHeaders);
   if (m_pdll->httpQueryInfo(m_hHttpRequest, HTTP_QUERY_RAW_HEADERS,
                             achHeaders, 
                             &dwBytesRead,
                             NULL)) {

      char *p = achHeaders;

      while (*p) {
         putHeader(p);

         p += strlen(p) + 1;
      }
   }

   delete m_pWinIn;
   delete m_pIn;
   m_pWinIn = NEW WinInetInputStream(m_pdll, m_hHttpRequest);
   m_pIn = NEW BufferedInputStream(m_pWinIn);

   return dwResult;
}
