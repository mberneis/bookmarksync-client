/*
 * SyncIT/ProxyDialog.cxx
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
 *    Brings up an authentication screen for proxies,
 *    should one get in the way...
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "ProductVersion.h"
#include "SyncLib/RegKey.h"
#include "SyncLib/Util.h"

#include "resource.h"
#include "ProxyDialog.h"

static const TCHAR PROXY_USER[] = TEXT("ProxyUser");
static const TCHAR PROXY_PASS[] = TEXT("ProxyPass");

using namespace syncit;

extern HINSTANCE ghResourceInstance;

ProxyDialog::ProxyDialog() {
#ifndef NDEBUG
   strcpy(m_achStartTag, "ProxyDialog");
   strcpy(m_achEndTag, "ProxyDialog");
#endif /* NDEBUG */

   // m_fSave is false, unless both PROXY_USER and PROXY_PASS
   // is specified, then it is true.
   //
   m_fSave = false;

   RegKey key;

   if (key.open(HKEY_CURRENT_USER, APP_REG_KEY) &&
       key.queryValue(PROXY_USER, m_achUsername, sizeof(m_achUsername)) > 0 &&
       key.queryValue(PROXY_PASS, m_achPassword, sizeof(m_achPassword)) > 0) {
      m_fSave = true;
   }
   else {
      m_achUsername[0] = m_achPassword[0] = TEXT('\0');
   }

   m_pListener = NULL;
}

ProxyDialog::~ProxyDialog() {

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */
}

#ifndef NDEBUG
bool ProxyDialog::isValid() const {
   return strcmp(m_achStartTag, "ProxyDialog") == 0 &&
          strcmp(m_achEndTag, "ProxyDialog") == 0;
}
#endif /* NDEBUG */

bool ProxyDialog::show(HWND hwndParent,
                       LPCTSTR pszServer,
                       LPCTSTR pszRealm) {
   m_pszServer = pszServer == NULL ? "" : pszServer;
   m_pszRealm  = pszRealm  == NULL ? "" : pszRealm;

   int r = DialogBoxParam(ghResourceInstance,
                          MAKEINTRESOURCE(IDD_PROXY),
                          hwndParent,
                          (DLGPROC) ProxyDialogProc,
                          (LONG) this);

   m_pszServer = m_pszRealm = NULL;

   switch (r) {
      case -1:
      case FALSE:
         return false;

      case TRUE:
         if (m_fSave) {
            RegKey key;

            if (key.open(HKEY_CURRENT_USER, APP_REG_KEY, true)) {
               // if both username and password are blank
               //
               if (m_achUsername[0] == TEXT('\0') && m_achPassword[0] == TEXT('\0')) {
                  key.deleteValue(PROXY_USER);
                  key.deleteValue(PROXY_PASS);
               }
               else {
                  key.setValue(PROXY_USER, m_achUsername);
                  key.setValue(PROXY_PASS, m_achPassword);
               }
            }
         }
         return true;

      default:
         return false;
   }
}

BOOL CALLBACK ProxyDialogProc(HWND hwndDialog,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam) {
   ProxyDialog *p = (ProxyDialog *) GetWindowLong(hwndDialog, DWL_USER);

   switch (uMsg) {
      /*
       * Set the focus to the OK button.
       * <p>
       * In response to a WM_INITDIALOG message, the dialog box
       * procedure should return FALSE (zero) if it calls the SetFocus
       * function to set the focus to one of the controls in the
       * dialog box. Otherwise, it should return TRUE (nonzero),
       * in which case the system sets the focus to the first control
       * in the dialog box that can be given the focus. 
       */
      case WM_INITDIALOG:
            // WM_INITDIALOG
            // hwndFocus = (HWND) wParam; // handle of control to receive focus 
            // lInitParam = lParam;       // initialization parameter
         {
            p = (ProxyDialog *) lParam;

            assert(p->isValid());

            SetWindowLong(hwndDialog, DWL_USER, lParam);

            SetDlgItemText(hwndDialog, IDC_USERNAME, p->m_achUsername);
            SetDlgItemText(hwndDialog, IDC_SERVER, p->m_pszServer);
            SetDlgItemText(hwndDialog, IDC_REALM, p->m_pszRealm);

            if (p->m_fSave) {
               SetDlgItemText(hwndDialog, IDC_PASSWORD, p->m_achPassword);

               CheckDlgButton(hwndDialog, IDC_SAVE, BST_CHECKED);
            }
            else {
               CheckDlgButton(hwndDialog, IDC_SAVE, BST_UNCHECKED);
            }

            CheckDlgButton(hwndDialog, IDC_ENABLE_SYNC, p->m_fAuto ? BST_CHECKED : BST_UNCHECKED);
         }
         return TRUE;

      case WM_HELP:
         // WM_HELP
         // lphi = (LPHELPINFO) lParam; 
         p->m_pListener->onHelp(hwndDialog, (HELPINFO *) lParam, TEXT("proxy-dialog"));
         return TRUE;

      case WM_COMMAND:
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         switch (LOWORD(wParam)) {
            case IDOK:
            case ID_PROXY_OK:
               assert(p->isValid());

               GetDlgItemText(hwndDialog,
                              IDC_USERNAME,
                              p->m_achUsername,
                              ELEMENTS(p->m_achUsername));

               GetDlgItemText(hwndDialog,
                              IDC_PASSWORD,
                              p->m_achPassword,
                              ELEMENTS(p->m_achPassword));

               p->m_fSave = IsDlgButtonChecked(hwndDialog, IDC_SAVE) == BST_CHECKED;
               p->m_fAuto = IsDlgButtonChecked(hwndDialog, IDC_ENABLE_SYNC) == BST_CHECKED;

               EndDialog(hwndDialog,   // hDlg -- handle to dialog box
                         TRUE);        // nResult -- value to return
               return TRUE;

            case IDCANCEL:
            case ID_PROXY_CANCEL:
               EndDialog(hwndDialog,   // hDlg -- handle to dialog box
                         FALSE);       // nResult -- value to return
               return TRUE;

            default:
               return FALSE;
         }

      default:
         return FALSE;
   }
}
