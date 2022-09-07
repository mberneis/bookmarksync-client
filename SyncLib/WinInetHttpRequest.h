/*
 * SyncLib/WinInetHttpRequest.h
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
#ifndef WinInetHttpRequest_H
#define WInInetHttpRequest_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>
#include <wininet.h>

#include "HttpRequest.h"
#include "Errors.h"

namespace syncit {

   typedef
   HINTERNET
   (WINAPI
    *PfnInternetOpenA)(
       IN LPCSTR lpszAgent,
       IN DWORD dwAccessType,
       IN LPCSTR lpszProxy OPTIONAL,
       IN LPCSTR lpszProxyBypass OPTIONAL,
       IN DWORD dwFlags
       );

   typedef
   HINTERNET
   (WINAPI
   *PfnInternetConnectA)(
       IN HINTERNET hInternet,
       IN LPCSTR lpszServerName,
       IN INTERNET_PORT nServerPort,
       IN LPCSTR lpszUserName OPTIONAL,
       IN LPCSTR lpszPassword OPTIONAL,
       IN DWORD dwService,
       IN DWORD dwFlags,
       IN DWORD dwContext
       );

   typedef
   BOOL
   (WINAPI * 
   PfnInternetCloseHandle)(
       IN HINTERNET hInternet
       );

   typedef
   HINTERNET
   (WINAPI * 
   PfnHttpOpenRequestA)(
       IN HINTERNET hConnect,
       IN LPCSTR lpszVerb,
       IN LPCSTR lpszObjectName,
       IN LPCSTR lpszVersion,
       IN LPCSTR lpszReferrer OPTIONAL,
       IN LPCSTR FAR * lplpszAcceptTypes OPTIONAL,
       IN DWORD dwFlags,
       IN DWORD dwContext
       );

   typedef
   BOOL
   (WINAPI * 
   PfnHttpSendRequestA)(
       IN HINTERNET hRequest,
       IN LPCSTR lpszHeaders OPTIONAL,
       IN DWORD dwHeadersLength,
       IN LPVOID lpOptional OPTIONAL,
       IN DWORD dwOptionalLength
       );

   typedef
   BOOL
   (WINAPI * 
   PfnHttpSendRequestExA)(
       IN HINTERNET hRequest,
       IN LPINTERNET_BUFFERS lpBuffersIn,
       OUT LPINTERNET_BUFFERS lpBuffersOut,
       IN DWORD dwFlags,
       IN DWORD dwContext
   );

   typedef
   BOOL
   (WINAPI * 
   PfnHttpQueryInfoA)(
       IN HINTERNET hRequest,
       IN DWORD dwInfoLevel,
       IN OUT LPVOID lpBuffer OPTIONAL,
       IN OUT LPDWORD lpdwBufferLength,
       IN OUT LPDWORD lpdwIndex OPTIONAL
       );

   typedef
   BOOL
   (WINAPI * 
   PfnInternetReadFile)(
       IN HINTERNET hFile,
       IN LPVOID lpBuffer,
       IN DWORD dwNumberOfBytesToRead,
       OUT LPDWORD lpdwNumberOfBytesRead
       );

   typedef
   BOOL
   (WINAPI * 
   PfnInternetWriteFile)(
       IN HINTERNET hFile,
       IN LPCVOID lpBuffer,
       IN DWORD dwNumberOfBytesToWrite,
       OUT LPDWORD lpdwNumberOfBytesWritten
       );

   typedef
   BOOL
   (WINAPI * 
   PfnHttpEndRequest)(
       IN HINTERNET hRequest,
       OUT LPINTERNET_BUFFERS lpBuffersOut,
       IN DWORD dwFlags,
       IN DWORD dwContext
       );

   //typedef
   //BOOL
   //(WINAPI *
   // PfnInternetCheckConnection)(
   //   IN LPCTSTR pszUrl,
   //   IN DWORD dwFlags,
   //   IN DWORD dwReserved
   //   );

   class WinInet {

   public:
      WinInet();

      virtual ~WinInet() {
         if (isLoaded()) {
            ::FreeLibrary(m_dll);
         }
      }

      bool isLoaded() const {
         return m_dll != NULL;
      }

      HINTERNET
      internetOpen(
          IN LPCSTR lpszAgent,
          IN DWORD dwAccessType,
          IN LPCSTR lpszProxy OPTIONAL,
          IN LPCSTR lpszProxyBypass OPTIONAL,
          IN DWORD dwFlags
          ) const {
         return m_open(lpszAgent, dwAccessType, lpszProxy, lpszProxyBypass, dwFlags);
      }

      HINTERNET
      internetConnect(
          IN HINTERNET hInternet,
          IN LPCSTR lpszServerName,
          IN INTERNET_PORT nServerPort,
          IN LPCSTR lpszUserName OPTIONAL,
          IN LPCSTR lpszPassword OPTIONAL,
          IN DWORD dwService,
          IN DWORD dwFlags,
          IN DWORD dwContext
          ) const {
         return m_connect(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword,
                          dwService, dwFlags, dwContext);
      }

      BOOL
      internetCloseHandle(
          IN HINTERNET hInternet
          ) const {
         return m_close(hInternet);
      }

      HINTERNET
      httpOpenRequest(
          IN HINTERNET hConnect,
          IN LPCSTR lpszVerb,
          IN LPCSTR lpszObjectName,
          IN LPCSTR lpszVersion,
          IN LPCSTR lpszReferrer OPTIONAL,
          IN LPCSTR FAR * lplpszAcceptTypes OPTIONAL,
          IN DWORD dwFlags,
          IN DWORD dwContext
          ) const {
         return m_request(hConnect, lpszVerb, lpszObjectName, lpszVersion,
                          lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
      }

      BOOL
      httpSendRequest(
          IN HINTERNET hRequest,
          IN LPCSTR lpszHeaders OPTIONAL,
          IN DWORD dwHeadersLength,
          IN LPVOID lpOptional OPTIONAL,
          IN DWORD dwOptionalLength
          ) const {
         return m_send(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
      }

      BOOL
      httpSendRequestEx(
          IN HINTERNET hRequest,
          IN LPINTERNET_BUFFERS lpBuffersIn,
          OUT LPINTERNET_BUFFERS lpBuffersOut,
          IN DWORD dwFlags,
          IN DWORD dwContext
          ) const {
         return m_sendex(hRequest, lpBuffersIn, lpBuffersOut, dwFlags, dwContext);
      }

      BOOL
      httpQueryInfo(
          IN HINTERNET hRequest,
          IN DWORD dwInfoLevel,
          IN OUT LPVOID lpBuffer OPTIONAL,
          IN OUT LPDWORD lpdwBufferLength,
          IN OUT LPDWORD lpdwIndex OPTIONAL
          ) const {
         return m_httpQuery(hRequest, dwInfoLevel, lpBuffer, lpdwBufferLength, lpdwIndex);
      }

      BOOL
      internetReadFile(
          IN HINTERNET hFile,
          IN LPVOID lpBuffer,
          IN DWORD dwNumberOfBytesToRead,
          OUT LPDWORD lpdwNumberOfBytesRead
          ) const {
         return m_read(hFile, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead);
      }

      BOOL
      internetWriteFile(
          IN HINTERNET hFile,
          IN LPCVOID lpBuffer,
          IN DWORD dwNumberOfBytesToWrite,
          OUT LPDWORD lpdwNumberOfBytesWritten
          ) const {
         return m_write(hFile, lpBuffer, dwNumberOfBytesToWrite, lpdwNumberOfBytesWritten);
      }

      BOOL
      httpEndRequest(
          IN HINTERNET hRequest,
          OUT LPINTERNET_BUFFERS lpBuffersOut,
          IN DWORD dwFlags,
          IN DWORD dwContext) const {
         return m_endReq(hRequest, lpBuffersOut, dwFlags, dwContext);
      }

      //BOOL
      //internetCheckConnection(
      //   IN LPCTSTR pszUrl,
      //   IN DWORD dwFlags,
      //   IN DWORD dwReserved) const {
      //   return m_check(pszUrl, dwFlags, dwReserved);
      //}

   private:
      HINSTANCE                  m_dll;

      PfnInternetOpenA           m_open;
      PfnInternetConnectA        m_connect;
      PfnHttpOpenRequestA        m_request;
      PfnHttpSendRequestA        m_send;
      PfnHttpQueryInfoA          m_httpQuery;
      PfnInternetReadFile        m_read;
      PfnInternetCloseHandle     m_close;

      // optional
      PfnHttpSendRequestExA      m_sendex;
      PfnInternetWriteFile       m_write;
      PfnHttpEndRequest          m_endReq;
      //PfnInternetCheckConnection m_check;
   };

   class WinInetHttpRequest : public HttpRequest {
      friend class WinInetInputStream;

   public:
      WinInetHttpRequest(WinInet *parent);
      ~WinInetHttpRequest();

      virtual void setAuthentication(const char *pszUsername,
                                     const char *pszPassword);

      virtual int send(const char *pszVerb,
                       const URL &url,
                       ByteArrayOutputStream *data,
                       const char *pszMimeType = NULL);

      virtual int send(const char *pszVerb,
                       const URL &url,
                       unsigned __int64 size,
                       InputStream *pin,
                       const char *pszMimeType = NULL,
                       HttpProgressListener *pListener = NULL);

      virtual void close();

      virtual bool isConnected() const;

   private:
      void connect(const URL &url);

      string getHeader(int i) const;

      WinInet *m_pdll;

      HINTERNET m_hInternetSession;
      HINTERNET m_hHttpSession;
      HINTERNET m_hHttpRequest;

      WinInetInputStream *m_pWinIn;

      char *m_pszUsername;
      char *m_pszPassword;
   };
}

#endif /* WinInetHttpRequest_H */
