/*
 * SyncLib/RegKey.h
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
#ifndef RegKey_H
#define RegKey_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include "DateTime.h"

namespace syncit {

   class RegKey {
   public:
      RegKey(HKEY hkey = 0) : m_hkey(hkey) {}

      ~RegKey() {
         if (isOpen()) close();
      }

      bool isOpen() const {
         return m_hkey != 0;
      }

      HKEY getHandle() const {
         return m_hkey;
      }

      bool open(const RegKey &root,
                const char *pszSubKey,
                bool fWritable = false) {
         return open(root.m_hkey, pszSubKey, fWritable);
      }

      bool open(HKEY hkey, const char *pszSubKey, bool fWritable = false);

      bool create(const RegKey &root, const char *pszSubKey) {
         return create(root.m_hkey, pszSubKey);
      }

      bool create(HKEY hkey, const char *pszSubKey);

      void setValue(const char *pszValueName, const char *pszValue) {
         setValue(pszValueName, REG_SZ, pszValue, lstrlenA(pszValue) + 1);
      }

      void setValue(const char *pszValueName, DWORD dwValue) {
         setValue(pszValueName, REG_DWORD, &dwValue, sizeof(dwValue));
      }

      void setValue(const char *pszValueName, const void *pv, size_t cb) {
         setValue(pszValueName, REG_BINARY, pv, cb);
      }

      void setValue(const char *pszValueName, const DateTime &dt);

      void setValue(const char *pszValueName,
                    DWORD dwType, const void *pv, size_t cb);

      DWORD queryValue(const char *pszValueName, DWORD dwDefault) const {
         DWORD dwValue, dwType;

         if (queryValue(pszValueName, &dwType, &dwValue, sizeof(dwValue)) == sizeof(dwValue) &&
             dwType == REG_DWORD) {
            return dwValue;
         }
         else {
            return dwDefault;
         }
      }

      size_t queryValue(const char *pszValueName, char *pach, size_t cch) const {
         DWORD dwType;

         size_t l = queryValue(pszValueName, &dwType, pach, cch);

         return (dwType == REG_SZ && l > 0) ? l - 1 : 0;
      }

      DWORD queryDWORD(const char *pszValueName, DWORD dwDefault) const;

      DateTime queryDateTime(const char *pszValueName) const;

      size_t queryValue(const char *pszValueName, DWORD *pdwType, void *pv, size_t cb) const;

      bool deleteValue(const char *pszValueName);

      void close();

   private:
      HKEY m_hkey;

      RegKey(RegKey &rhs);
      RegKey &operator=(RegKey &rhs);
   };

}

#endif /* RegKey_H */
