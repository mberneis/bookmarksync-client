/*
 * SyncLib/ConnectSettings.cxx
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
#include <wininet.h>
#include <winsock.h>

#include <cassert>

#include "ConnectSettings.h"
#include "FileInputStream.h"
#include "BufferedInputStream.h"
#include "NetscapeInfo.h"
#include "text.h"
#include "RegKey.h"

using namespace syncit;

typedef
BOOL
(WINAPI * 
PfnInternetQueryOptionA)(
    IN HINTERNET hInternet,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );

static void GetCustomSetting(const char *pszValueName,
                             char *pach,
                             size_t cch);

static void SetCustomSetting(RegKey &key,
                             const char *pszValueName,
                             const char *pszValue);

static ConnectResult GetIESettings(char *pach,
                                   size_t cch);

static ConnectResult GetNSSettings(char *pach,
                                   size_t cch);

static ConnectResult ReadNetscapePrefs(const char *pszFilename,
                                       char *pach, size_t cch);

static ConnectResult FindProxySetting(const char *pszSpec,
                                      char *pach, size_t cch);

static bool FindProxySetting(const char *pszSpec,
                             const char *pszType,
                             char *pach,
                             size_t cch);

static const char INTERNET_SETTINGS[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
static const char KEY_SYNCIT[] = "Software\\SyncIT";
static const char PROXY_SETTINGS[] = "ProxySettings";
static const char HTTP_PROXY[] = "HttpProxy";
static const char SOCKS_PROXY[] = "SocksProxy";

ConnectSetting syncit::GetCurrentConnectSetting() {
   char achBuf[128];

   GetCustomSetting(PROXY_SETTINGS, achBuf, sizeof(achBuf));

   switch (achBuf[0]) {
      case 'd':   // direct
      case 'D':
         return ConnectDirect;

      case 'h':   // http
      case 'H':
         return ConnectHttp;

      case 's':   // socks
      case 'S':
         return ConnectSocks;

      case 'w':   // wininet
      case 'W':
         return ConnectWinInet;

      case 'm':   // MS, microsoft
      case 'M':
      case 'i':   // IE, internet
      case 'I':
      case 'e':   // explorer
      case 'E':
         return ConnectExplorer;

      case 'n':   // netscape
      case 'N':
      case 'c':   // communicator
      case 'C':
         return ConnectNetscape;

      default:    // automatic, anything else
         return ConnectAutomatic;
   }
}

ConnectResult syncit::GetConnectSettings(ConnectSetting f,
                                         char *pach,
                                         size_t cch) {
   int editMask = ConnectEditable;
   ConnectResult r;

   switch (f) {
      case ConnectAutomatic:
         r = GetIESettings(pach, cch);
         if (r == ConnectDisabled || r == ConnectEnabled) {
            r = GetNSSettings(pach, cch);
            if (r == ConnectDisabled) r = ConnectEnabled;
         }
         break;

      case ConnectDirect:
         r = ConnectEnabled;
         break;

      case ConnectHttp:
         GetCustomSetting(HTTP_PROXY, pach, cch);
         r = (ConnectResult) (ConnectEditable | ConnectHttpUrl);
         break;

      case ConnectSocks:
         GetCustomSetting(SOCKS_PROXY, pach, cch);
         r = (ConnectResult) (ConnectEditable | ConnectSocksUrl);
         break;

      case ConnectWinInet:
      case ConnectExplorer:
         r = GetIESettings(pach, cch);
         break;

      case ConnectNetscape:
         r = GetNSSettings(pach, cch);
         break;

      default:
         r = ConnectDisabled;
         break;
   }

   return r;
}

void syncit::SetConnectSettings(ConnectSetting f,
                                const char *psz) {
   RegKey key;

   key.create(HKEY_CURRENT_USER, KEY_SYNCIT);

   switch (f) {
      case ConnectAutomatic:
         SetCustomSetting(key, PROXY_SETTINGS, "Automatic");
         break;

      case ConnectDirect:
         SetCustomSetting(key, PROXY_SETTINGS, "Direct");
         break;

      case ConnectHttp:
         SetCustomSetting(key, HTTP_PROXY, psz);
         SetCustomSetting(key, PROXY_SETTINGS, HTTP_PROXY);
         break;

      case ConnectSocks:
         SetCustomSetting(key, SOCKS_PROXY, psz);
         SetCustomSetting(key, PROXY_SETTINGS, SOCKS_PROXY);
         break;

      case ConnectWinInet:
         SetCustomSetting(key, PROXY_SETTINGS, "WinInet");
         break;

      case ConnectExplorer:
         SetCustomSetting(key, PROXY_SETTINGS, "MSIE");
         break;

      case ConnectNetscape:
         SetCustomSetting(key, PROXY_SETTINGS, "Netscape");
         break;

      default:
         assert(f == ConnectAutomatic ||
                f == ConnectDirect ||
                f == ConnectHttp ||
                f == ConnectSocks ||
                f == ConnectWinInet ||
                f == ConnectExplorer ||
                f == ConnectNetscape);
   }
}

static void GetCustomSetting(const char *pszValueName,
                             char *pach,
                             size_t cch) {

   RegKey key;

   if (key.open(HKEY_CURRENT_USER, KEY_SYNCIT)) {
      size_t r = key.queryValue(pszValueName, pach, cch);

      if (r < cch) pach[r] = 0;
   }
   else {
      pach[0] = 0;
   }
}

static void SetCustomSetting(RegKey &key,
                             const char *pszValueName,
                             const char *pszValue) {
   if (pszValue != NULL && pszValue[0] != 0) {
      key.setValue(pszValueName, pszValue);
   }
   else {
      key.deleteValue(pszValueName);
   }
}

static ConnectResult GetIESettings(char *pach,
                                   size_t cch) {
   HMODULE hdll = (HMODULE) LoadLibrary(TEXT("WININET"));

   ConnectResult r = ConnectDisabled;

   RegKey key;

   if (key.open(HKEY_CURRENT_USER, INTERNET_SETTINGS)) {
      if (key.queryValue("AutoConfigURL", pach, cch) > 0) {
         r = ConnectConfigUrl;
      }
   }

   if (r == ConnectDisabled && hdll != NULL) {
      PfnInternetQueryOptionA pfn = (PfnInternetQueryOptionA) ::GetProcAddress(hdll, "InternetQueryOptionA");

      if (pfn != NULL) {
         union {
            INTERNET_PROXY_INFO ipi;
            char ach[1024];
         } u;
         DWORD dwcb = sizeof(u);

         if (pfn(NULL,
                 INTERNET_OPTION_PROXY,
                 &u.ipi,
                 &dwcb)) {
            if (u.ipi.lpszProxy == NULL) {
               r = ConnectEnabled;
            }
            else {
               r = FindProxySetting(u.ipi.lpszProxy, pach, cch);
            }
         }
      }
   }

   if (r == ConnectDisabled && key.isOpen()) {
      char achProxy[4096];

      if (key.queryDWORD("ProxyEnable", FALSE) && key.queryValue("ProxyServer", achProxy, sizeof(achProxy)) > 0) {
         r = FindProxySetting(achProxy, pach, cch);
      }
   }

   return r;
}

static ConnectResult GetNSSettings(char *pach, size_t cch) {

   ConnectResult r = ConnectDisabled;

   char achProfileDirectory[MAX_PATH];

   if (GetNetscapeV45Info(achProfileDirectory, sizeof(achProfileDirectory)) ||
       GetNetscapeV40Info(achProfileDirectory, sizeof(achProfileDirectory))) {
      size_t l = lstrlenA(achProfileDirectory);

      if (achProfileDirectory[l - 1] == '\\') {
         l--;
      }

      lstrcpy(achProfileDirectory + l, "\\prefs.js");

      return ReadNetscapePrefs(achProfileDirectory, pach, cch);
   }

   // Netscape V2 info
   //
   RegKey key;

   if (key.open(HKEY_CURRENT_USER, "Software\\Netscape\\Netscape Navigator\\Proxy Information")) {
      r = ConnectEnabled;

      if (key.queryDWORD("Proxy Type", TRUE) != FALSE) {
         size_t i = key.queryValue("HTTP_Proxy", pach, cch);

         if (i > 0) {
            DWORD dwPort = key.queryDWORD("HTTP_ProxyPort", 0);

            if (dwPort) {
               wsprintfA(pach + i, ":%d", dwPort);
            }
            else {
               pach[i] = 0;
            }

            r = ConnectHttpUrl;
         }
      }
   }

   return r;
}

static const char JS_PROXY_PREFIX[] = "user_pref(\"network.proxy.";
static const size_t JS_PROXY_PREFIX_LEN = sizeof(JS_PROXY_PREFIX) - 1;

static const char JS_HTTP[] = "http\", \"";
static const size_t JS_HTTP_LEN = sizeof(JS_HTTP) - 1;

static const char JS_HTTP_PORT[] = "http_port\", ";
static const size_t JS_HTTP_PORT_LEN = sizeof(JS_HTTP_PORT) - 1;

static const char JS_TYPE[] = "type\", ";
static const size_t JS_TYPE_LEN = sizeof(JS_TYPE) - 1;

/**
 * If we can't open the file, return ConnectDisabled.
 * If we can open the file, but it doesn't have any proxy information,
 * or if network.proxy.type is 0, then return ConnectEnabled.
 * If it has proxy information, then format it as a URL into pach, and return
 *    ConnectHttpUrl
 */
static ConnectResult ReadNetscapePrefs(const char *pszFilename,
                                       char *pach, size_t cch) {
   FileInputStream f;
   if (f.open(pszFilename)) {
      size_t i = 0;
      unsigned long ulPort = 80;
      unsigned long ulType = 0;

      try {
         BufferedInputStream in(&f);
         char achLine[4096];

         while (in.readLine(achLine, sizeof(achLine))) {
            if (memcmp(achLine, JS_PROXY_PREFIX, JS_PROXY_PREFIX_LEN) == 0) {
               char *p = achLine + JS_PROXY_PREFIX_LEN;

               if (memcmp(p, JS_HTTP, JS_HTTP_LEN) == 0) {
                  char *pszHost = p + JS_HTTP_LEN;
                  char *pszEnd  = strchr(pszHost, '"');

                  if (pszEnd != NULL) {
                     i = bufcopy(pszHost, pszEnd - pszHost, pach, cch);
                  }
               }
               else if (memcmp(p, JS_HTTP_PORT, JS_HTTP_PORT_LEN) == 0) {
                  ulPort = strtoul(p + JS_HTTP_PORT_LEN, NULL, 10);
               }
               else if (memcmp(p, JS_TYPE, JS_TYPE_LEN) == 0) {
                  ulType = strtoul(p + JS_TYPE_LEN, NULL, 10);
               }
            }
         }
      } catch (...) {
         f.close();
         throw;
      }

      f.close();

      if (0 < i && i < cch - 10 && ulType == 1) {
         wsprintf(pach + i, ":%d", ulPort);

         return ConnectHttpUrl;
      }
      else {
         return ConnectEnabled;
      }
   }
   else {
      return ConnectDisabled;
   }
}

static ConnectResult FindProxySetting(const char *pszSpec,
                                      char *pach, size_t cch) {
   if (FindProxySetting(pszSpec, "http=", pach, cch)) {
      return ConnectHttpUrl;
   }
   else if (FindProxySetting(pszSpec, "socks=", pach, cch)) {
      return ConnectSocksUrl;
   }
   else {
      bufcopy(pszSpec, pach, cch);
      return ConnectHttpUrl;
   }
}

static bool FindProxySetting(const char *pszSpec,
                             const char *pszType,
                             char *pach,
                             size_t cch) {
   char *p1 = strstr(pszSpec, pszType);

   if (p1 != NULL) {
      char *p2 = p1 + lstrlenA(pszType);
      char *p3 = strchr(p2, ' ');

      if (p3 == NULL) p3 = strchr(p2, ';');

      if (p3 == NULL) {
         bufcopy(p2, pach, cch);
      }
      else {
         bufcopy(p2, p3 - p2, pach, cch);
      }

      return true;
   }
   else {
      return false;
   }
}

typedef BOOL (WINAPI * PfnInternetGetConnectedState)(OUT LPDWORD pdwFlags, IN DWORD dwReserved);

static class WinInetDll {
public:
   WinInetDll() {
      m_hmodule = ::LoadLibraryA("WININET");

      if (m_hmodule != NULL) {
         m_pfn = (PfnInternetGetConnectedState) ::GetProcAddress(m_hmodule, "InternetGetConnectedState");
      }
      else {
         m_pfn = NULL;
      }
   }

   ~WinInetDll() {
      ::FreeLibrary(m_hmodule);
   }

   HMODULE m_hmodule;
   PfnInternetGetConnectedState m_pfn;
} gwininet;

/**
 * Return true if we're directly connected to the Internet, or if we have
 * autodial enabled AND we're connected to some network.  Return false if
 * autodial is enabled but we're not connected.
 */
bool syncit::IsDialInActive() {
   RegKey key;

   bool result = true;

   if (gwininet.m_pfn != NULL) {
      DWORD dwFlags;

      return (*gwininet.m_pfn)(&dwFlags, 0) ? true : false;
   }

   if (key.open(HKEY_CURRENT_USER, INTERNET_SETTINGS)) {
      if (key.queryDWORD("EnableAutodial", FALSE)) {
         // autodial enabled, not connected 
         char achHostName[256];

         result = false;

         if (gethostname(achHostName, sizeof(achHostName)) == 0) {
            struct hostent *he = gethostbyname(achHostName);

            if (he != NULL) {
               int i = 0;

               char *addr = he->h_addr_list[i];
               while (!result && addr != NULL) {
                  if (addr[0] != 0 && addr[0] != 127) {
                     /* address not in 0 or localhost net, must be connected */
                     result = true;
                  }

                  addr = he->h_addr_list[++i];
               }

               /* result == true || addr == NULL */
               /* if addr == NULL then result == false */
            }
         }
      }
   }

   return result;
}
