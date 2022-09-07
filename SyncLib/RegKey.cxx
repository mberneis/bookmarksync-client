/*
 * SyncLib/RegKey.cxx
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
#include "RegKey.h"
#include "Errors.h"

using namespace syncit;

bool RegKey::open(HKEY hkey,
                  const char *pszSubKey,
                  bool fWritable) {
   LONG l = RegOpenKeyExA(hkey,
                          pszSubKey,
                          0,
                          fWritable ? KEY_SET_VALUE | KEY_QUERY_VALUE : KEY_QUERY_VALUE,
                          &m_hkey);

   if (l != ERROR_SUCCESS && l != ERROR_FILE_NOT_FOUND) {
      throw RegError(RegError::Open, pszSubKey, Win32Error("RegOpenKeyEx", l));
   }

   return l == ERROR_SUCCESS;
}

bool RegKey::create(HKEY hkey, const char *pszSubKey) {
   DWORD dwDisposition;
   LONG l = RegCreateKeyExA(hkey,
                            pszSubKey,
                            0,
                            NULL,
                            0,
                            KEY_QUERY_VALUE | KEY_SET_VALUE,
                            NULL,
                            &m_hkey,
                            &dwDisposition);

   if (l != ERROR_SUCCESS) {
      throw RegError(RegError::Create, pszSubKey, Win32Error("RegCreateKey", l));
   }

   return dwDisposition == REG_CREATED_NEW_KEY;
}

void RegKey::setValue(const char *pszValueName, const DateTime &dt) {
   FILETIME ft = dt.getFileTime();

   setValue(pszValueName, REG_BINARY, &ft, sizeof(ft));
}

void RegKey::setValue(const char *pszValueName,
                      DWORD dwType, const void *pv, size_t cb) {
   LONG l = RegSetValueExA(m_hkey,
                           pszValueName,
                           0,
                           dwType,
                           (const BYTE *) pv,
                           cb);

   if (l != ERROR_SUCCESS) {
      throw RegError(RegError::Query, pszValueName, Win32Error("RegSetValueEx", l));
   }
}

size_t RegKey::queryValue(const char *pszValueName, DWORD *pdwType, void *pv, size_t cb) const {
   DWORD dwcb = cb;
   LONG l = RegQueryValueExA(m_hkey,
                             pszValueName,
                             NULL,
                             pdwType,
                             (BYTE *) pv,
                             &dwcb);

   if (l == ERROR_FILE_NOT_FOUND) {
      return 0;
   }
   else if (l != ERROR_SUCCESS && l != ERROR_MORE_DATA) {
      throw RegValueError(RegError::Query, pszValueName, Win32Error("RegQueryValueEx", l));
   }

   return dwcb;
}

DWORD RegKey::queryDWORD(const char *pszValueName, DWORD dwDefault) const {
   union {
      DWORD dwValue;
      char achValue[64];
   } u;
   DWORD dwType;

   size_t cb = queryValue(pszValueName, &dwType, &u, sizeof(u));

   if ((dwType == REG_DWORD || dwType == REG_BINARY) && cb == sizeof(DWORD)) {
      return u.dwValue;
   }
   else if (dwType == REG_SZ) {
      char *p;
      DWORD dw = strtoul(u.achValue, &p, 0);
      return *p == 0 && p > u.achValue ? dw : dwDefault;
   }
   else {
      return dwDefault;
   }
}

DateTime RegKey::queryDateTime(const char *pszValueName) const {
   FILETIME ft;
   DWORD dwType;

   if (queryValue(pszValueName, &dwType, &ft, sizeof(ft)) == sizeof(ft) &&
       dwType == REG_BINARY) {
      return DateTime(ft);
   }
   else {
      return DateTime();
   }
}

bool RegKey::deleteValue(const char *pszValueName) {
   LONG l = RegDeleteValueA(m_hkey, pszValueName);

   if (l == ERROR_SUCCESS) {
      return true;
   }
   else if (l == ERROR_FILE_NOT_FOUND || l == ERROR_PATH_NOT_FOUND) {
      return false;
   }
   else {
      throw RegValueError(RegError::Access, pszValueName, Win32Error("RegDeleteValue", l));
   }
}

void RegKey::close() {
   LONG l = RegCloseKey(m_hkey);

   m_hkey = 0;

   if (l != ERROR_SUCCESS) {
      throw Win32Error("RegCloseKey", l);
   }
}
