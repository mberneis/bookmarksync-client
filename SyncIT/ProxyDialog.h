/*
 * SyncIT/ProxyDialog.h
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
 * Last Modification: 8 Jan 1999
 *
 * Description:
 *    Brings up an authentication screen for proxies,
 *    should one get in the way...
 */
#ifndef ProxyDialog_H
#define ProxyDialog_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include <cassert>

class ProxyListener {
protected:
   ProxyListener() {}

public:
   virtual ~ProxyListener() {}

   virtual void onHelp(HWND hwnd, const HELPINFO *phi, LPCTSTR pszSource) = 0;
   virtual void onHelp(HWND hwnd, LPCTSTR psz) = 0;

private:
   ProxyListener(const ProxyListener &rhs);
   ProxyListener &operator=(const ProxyListener &rhs);
};

class ProxyDialog {

#ifndef NDEBUG
public:
   bool isValid() const;
private:
   char m_achStartTag[sizeof("ProxyDialog")];
#endif /* NDEBUG */

   friend BOOL CALLBACK ProxyDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
   ProxyDialog();
   ~ProxyDialog();

   bool show(HWND hwndParent,
             LPCTSTR pszServer,
             LPCTSTR pszRealm);

   LPCTSTR getUsername() const {
      return m_achUsername[0] == TEXT('\0') ? NULL : m_achUsername;
   }

   LPCTSTR getPassword() const {
      return m_achPassword[0] == TEXT('\0') ? NULL : m_achPassword;
   }

   bool isAutoSyncEnabled() const {
      return m_fAuto;
   }

   void setAutoSyncEnabled(bool f) {
      m_fAuto = f;
   }

   void addProxyListener(ProxyListener *p) {
      assert(m_pListener == NULL);
      m_pListener = p;
   }

private:
   ProxyListener *m_pListener;

   TCHAR m_achUsername[256];
   TCHAR m_achPassword[256];

   LPCTSTR m_pszServer, m_pszRealm;

   bool m_fSave, m_fAuto;

   // disable copy constructor and assignment
   ProxyDialog(const ProxyDialog &rhs);
   ProxyDialog &operator=(const ProxyDialog &rhs);

#ifndef NDEBUG
private:
   char m_achEndTag[sizeof("ProxyDialog")];
#endif /* NDEBUG */

};

#endif /* ProxyDialog_H */
