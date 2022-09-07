/*
 * SyncIT/SettingsWindow.cxx
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
#pragma warning( disable : 4786 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
#include <vector>

#include "SyncLib/Log.h"
#include "SyncLib/HttpRequest.h"
#include "SyncLib/ConnectSettings.h"
#include "SyncLib/RegKey.h"

#include "About.h"
#include "SettingsWindow.h"
#include "resource.h"

static const char KEY_SYNCIT[] = "Software\\SyncIT\\BookmarkSync";

using namespace syncit;

static BOOL CALLBACK AccountDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
//atic BOOL CALLBACK BrowserDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK ProxiesDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DisplayDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void SetProxyDialog(HWND hwndDlg, ConnectSetting fcs);
static ConnectSetting GetProxyDialog(HWND hwndDlg, char *pach, size_t cch);
static void SetProxyDialog(HWND hwndDlg, BOOL fAddrPort, BOOL fConfigUrl);
static void TestProxy(HWND hwndResult, ConnectSetting f, const char *psz, const URL &root);

static struct {
   int idd;
   DLGPROC pfn;
} PROPERTY_SHEETS[] = {
   { IDD_ACCOUNT,          AccountDlgProc },
// { IDD_BROWSERS,         BrowserDlgProc },
   { IDD_PROXY_SETTINGS,   ProxiesDlgProc },
   { IDD_DISPLAY,          DisplayDlgProc }
};

// automatic settings: default      0
// Custom HTTP                      1
// Custom SOCKS                     2
// Windows Internet API             3
// for each browser
//    current settings
//
static ConnectSetting gsettings[8];
static int gindex;

extern HINSTANCE ghResourceInstance;

static void push(HWND hwndComboBox,
                 int ids,
                 ConnectSetting f) {
   char achBuffer[4096];

   LoadString(ghResourceInstance, ids, achBuffer, sizeof(achBuffer));

   int i = ComboBox_AddString(hwndComboBox, achBuffer);
   assert(i == gindex);
   gsettings[gindex++] = f;
}

void SettingsWindow::init(HWND hwndParent,
                          int nStartPage,
                          LPCTSTR pszEmail,
                          bool fPopupMenuColumns) {
   if (m_hwnd != NULL) {
      setEmail(pszEmail);
      ShowWindow(m_hwnd, SW_NORMAL);
   }
   else {
      int i;
      PROPSHEETHEADER psh;
      unsigned char apsp[PROPSHEETPAGE_V1_SIZE * ELEMENTS(PROPERTY_SHEETS)];

      m_pszEmail = pszEmail;
      m_fPopupMenuColumns = m_fSavedPopupMenuColumns = fPopupMenuColumns;

      //psh.dwSize = sizeof(psh);
      psh.dwSize = PROPSHEETHEADER_V1_SIZE;
      psh.dwFlags = PSH_PROPSHEETPAGE | PSH_MODELESS;
      psh.hwndParent = hwndParent;
      psh.hInstance = ghResourceInstance;
      psh.hIcon = NULL;
      psh.pszCaption = MAKEINTRESOURCE(IDS_SETTINGS_CAPTION);
      psh.nPages = ELEMENTS(PROPERTY_SHEETS);
      psh.nStartPage = nStartPage;
      psh.ppsp = (PROPSHEETPAGE *) apsp;
      psh.pfnCallback = NULL;

      for (i = 0; i < ELEMENTS(PROPERTY_SHEETS); i++) {
         PROPSHEETPAGE *ppsp = (PROPSHEETPAGE *) (apsp + i * PROPSHEETPAGE_V1_SIZE);
         //ppsp->dwSize = sizeof(apsp[i]);
         ppsp->dwSize = PROPSHEETPAGE_V1_SIZE;
         ppsp->dwFlags = PSP_DEFAULT;
         ppsp->hInstance = ghResourceInstance;
         ppsp->pszTemplate = MAKEINTRESOURCE(PROPERTY_SHEETS[i].idd);
         ppsp->hIcon = NULL;
         ppsp->pszTitle = NULL;
         ppsp->pfnDlgProc = PROPERTY_SHEETS[i].pfn;
         ppsp->lParam = (LONG) this;
      }

      gindex = 0;
      m_hwnd = (HWND) PropertySheet(&psh);
   }
}

bool SettingsWindow::isDialogMessage(MSG *pmsg) {
   bool result = (m_hwnd != NULL && IsDialogMessage(m_hwnd, pmsg));

   if (PropSheet_GetCurrentPageHwnd(m_hwnd) == NULL) {
      DestroyWindow(m_hwnd);
      m_hwnd = NULL;
   }

   return result;
}

void SettingsWindow::setEmail(LPCTSTR pszEmail) {
   if (m_hwnd) {
      HWND hwndDlg = PropSheet_GetCurrentPageHwnd(m_hwnd);
      HWND hwndCtl = GetDlgItem(hwndDlg, IDC_EMAIL);

      if (hwndCtl != NULL) {
         SetWindowText(hwndCtl, pszEmail);
      }
   }
}

static BOOL CALLBACK AccountDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   SettingsWindow *pw = (SettingsWindow *) GetWindowLong(hwndDlg, DWL_USER);

   switch (uMsg) {
      case WM_INITDIALOG:
         // WM_INITDIALOG 
         // hwndFocus = (HWND) wParam; // handle of control to receive focus 
         // lInitParam = lParam;       // initialization parameter, for property sheets:
         // PSP = (PROPSHEETPAGE*) lParam
         //
         {
            PROPSHEETPAGE *ppsp = (PROPSHEETPAGE *) lParam;

            ::SetWindowLong(hwndDlg, DWL_USER, (LONG) ppsp->lParam);
            pw = (SettingsWindow *) ppsp->lParam;

            SetDlgItemText(hwndDlg, IDC_EMAIL, pw->getEmail());
            SetDlgItemText(hwndDlg, IDC_PROFILES, pw->getRegistrationUrl().c_str());
         }
         // The dialog box procedure should return TRUE to direct Windows to
         // set the keyboard focus to the control given by hwndFocus.
         // Otherwise, it should return FALSE to prevent Windows from setting
         // the default keyboard focus. 
         //
         return TRUE;

      case WM_COMMAND:
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         switch (LOWORD(wParam)) {
            case IDC_PROFILES:
               pw->showDocument(pw->getRegistrationUrl().c_str());
               return TRUE;

            case ID_LOGIN:
               pw->onLogin(hwndDlg);
               return TRUE;
         }

         return FALSE;

      /*
      case WM_HELP:
         // WM_HELP
         // lphi = (LPHELPINFO) lParam;
         return TRUE;
      */

      case WM_DRAWITEM:
         DrawLinkButton(hwndDlg, (DRAWITEMSTRUCT *) lParam);
         return TRUE;

      case WM_NOTIFY:
         // WM_NOTIFY
         // idCtrl = (int) wParam;
         // pnmh = (LPNMHDR) lParam; 
         //
         {
            int idCtrl = (int) GetWindowLong(hwndDlg, DWL_USER);
            const NMHDR *phdr = (const NMHDR *) lParam;

            DWORD dwResult = 0;

            switch (phdr->code) {
               case PSN_KILLACTIVE:
                  // The page should set DWL_MSGRESULT to FALSE when it is
                  // okay to lose the activation.
                  //
                  //switch (idCtrl) {
                  //}
                  break;

               case PSN_APPLY:
                  break;

               case PSN_SETACTIVE:
                  {
                     // Returns zero to accept the activation or -1 to activate
                     // the next or previous page (depending on whether the user
                     // chose the Next or Back button). To set the activation to
                     // a particular page, return the resource identifier of the
                     // page. 
                     //

                     // To set the return value, the dialog box procedure for the
                     // page must use the SetWindowLong function with the
                     // DWL_MSGRESULT value, and the dialog box procedure must
                     // return TRUE. 
                     //
                     break;
                  }

               default:
                  return FALSE;
            }

            SetWindowLong(hwndDlg, DWL_MSGRESULT, dwResult);

            return TRUE;
         }

      default:
         return FALSE;
   }
}

/*
static BOOL CALLBACK BrowserDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch (uMsg) {
      case WM_INITDIALOG:
         // WM_INITDIALOG 
         // hwndFocus = (HWND) wParam; // handle of control to receive focus 
         // lInitParam = lParam;       // initialization parameter, for property sheets:
         // PSP = (PROPSHEETPAGE*) lParam
         //
         {
            PROPSHEETPAGE *ppsp = (PROPSHEETPAGE *) lParam;

            ::SetWindowLong(hwndDlg, DWL_USER, (LONG) ppsp->lParam);
         }
         // The dialog box procedure should return TRUE to direct Windows to
         // set the keyboard focus to the control given by hwndFocus.
         // Otherwise, it should return FALSE to prevent Windows from setting
         // the default keyboard focus. 
         //
         return TRUE;

      case WM_COMMAND:
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         //switch (LOWORD(wParam)) {
            //case ID_LOGIN:
         //}

         return FALSE;

      case WM_NOTIFY:
         // WM_NOTIFY
         // idCtrl = (int) wParam;
         // pnmh = (LPNMHDR) lParam; 
         //
         {
            int idCtrl = (int) GetWindowLong(hwndDlg, DWL_USER);
            const NMHDR *phdr = (const NMHDR *) lParam;

            DWORD dwResult = 0;

            switch (phdr->code) {
               case PSN_KILLACTIVE:
                  // The page should set DWL_MSGRESULT to FALSE when it is
                  // okay to lose the activation.
                  //
                  //switch (idCtrl) {
                  //}

                  break;

               case PSN_SETACTIVE:
                  {
                     HWND hwndParent = GetParent(hwndDlg);

                     // Returns zero to accept the activation or -1 to activate
                     // the next or previous page (depending on whether the user
                     // chose the Next or Back button). To set the activation to
                     // a particular page, return the resource identifier of the
                     // page. 
                     //

                     // To set the return value, the dialog box procedure for the
                     // page must use the SetWindowLong function with the
                     // DWL_MSGRESULT value, and the dialog box procedure must
                     // return TRUE. 
                     //
                     break;
                  }

               default:
                  return FALSE;
            }

            SetWindowLong(hwndDlg, DWL_MSGRESULT, dwResult);

            return TRUE;
         }

      default:
         return FALSE;
   }
}
*/

static BOOL CALLBACK ProxiesDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch (uMsg) {
      case WM_INITDIALOG:
         // WM_INITDIALOG 
         // hwndFocus = (HWND) wParam; // handle of control to receive focus 
         // lInitParam = lParam;       // initialization parameter, for property sheets:
         // PSP = (PROPSHEETPAGE*) lParam
         //
         {
            PROPSHEETPAGE *ppsp = (PROPSHEETPAGE *) lParam;

            ::SetWindowLong(hwndDlg, DWL_USER, (LONG) ppsp->lParam);

            HWND hwndComboBox = GetDlgItem(hwndDlg, IDC_PROXY_CONFIG);

            push(hwndComboBox, IDS_PROXY_AUTO,    ConnectAutomatic);
            push(hwndComboBox, IDS_PROXY_DIRECT,  ConnectDirect);
            push(hwndComboBox, IDS_PROXY_HTTP,    ConnectHttp);
            push(hwndComboBox, IDS_PROXY_SOCKS,   ConnectSocks);
            push(hwndComboBox, IDS_PROXY_WININET, ConnectWinInet);
            push(hwndComboBox, IDS_PROXY_IE,      ConnectExplorer);
            push(hwndComboBox, IDS_PROXY_NS,      ConnectNetscape);

            ConnectSetting f = GetCurrentConnectSetting();
            ComboBox_SetCurSel(hwndComboBox, f);
            SetProxyDialog(hwndDlg, f);
         }
         // The dialog box procedure should return TRUE to direct Windows to
         // set the keyboard focus to the control given by hwndFocus.
         // Otherwise, it should return FALSE to prevent Windows from setting
         // the default keyboard focus. 
         //
         return TRUE;

      case WM_COMMAND:
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         if (HIWORD(wParam) == EN_CHANGE) {
            PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            return TRUE;
         }

         else if (HIWORD(wParam) == CBN_SELCHANGE) {
            // CBN_SELCHANGE:
            // idComboBox = (int) LOWORD(wParam);
            // hwndComboBox = (HWND) lParam;
            HWND hwndCtl = (HWND) lParam;

            SetProxyDialog(hwndDlg, gsettings[ComboBox_GetCurSel(hwndCtl)]);
            SetDlgItemText(hwndDlg, IDC_TEST_RESULTS, "");

            PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            return TRUE;
         }

         else if (LOWORD(wParam) == IDC_PROXY_TEST) {
            char ach[4096], achTesting[128];
            LoadString(ghResourceInstance, IDS_TESTING, achTesting, ELEMENTS(achTesting));
            SettingsWindow *pw = (SettingsWindow *) GetWindowLong(hwndDlg, DWL_USER);

            ConnectSetting f = GetProxyDialog(hwndDlg, ach, sizeof(ach));
            HWND hwndTestResults = GetDlgItem(hwndDlg, IDC_TEST_RESULTS);
            SetWindowText(hwndTestResults, achTesting);
            TestProxy(hwndTestResults, f, ach, pw->getRoot());
            return TRUE;
         }

         else {
            return FALSE;
         }

      /*
      case WM_HELP:
         // WM_HELP
         // lphi = (LPHELPINFO) lParam;
         return TRUE;
      */

      case WM_NOTIFY:
         // WM_NOTIFY
         // idCtrl = (int) wParam;
         // pnmh = (LPNMHDR) lParam; 
         //
         {
            int idCtrl = (int) GetWindowLong(hwndDlg, DWL_USER);
            const NMHDR *phdr = (const NMHDR *) lParam;

            DWORD dwResult = 0;

            switch (phdr->code) {
               case PSN_KILLACTIVE:
                  break;

               case PSN_APPLY:
                  {
                     char achBuf[4096];

                     ConnectSetting f = GetProxyDialog(hwndDlg, achBuf, sizeof(achBuf));

                     SetConnectSettings(f, achBuf);
                  }
                  break;

               case PSN_QUERYCANCEL:
                  break;

               case PSN_SETACTIVE:
                  // Returns zero to accept the activation or -1 to activate
                  // the next or previous page (depending on whether the user
                  // chose the Next or Back button). To set the activation to
                  // a particular page, return the resource identifier of the
                  // page. 
                  //

                  // To set the return value, the dialog box procedure for the
                  // page must use the SetWindowLong function with the
                  // DWL_MSGRESULT value, and the dialog box procedure must
                  // return TRUE. 
                  //
                  break;

               default:
                  return FALSE;
            }

            SetWindowLong(hwndDlg, DWL_MSGRESULT, dwResult);

            return TRUE;
         }

      default:
         return FALSE;
   }
}

static void SetProxyDialog(HWND hwndDlg, ConnectSetting fcs) {
   char achBuf[4096];
   ConnectResult f = GetConnectSettings(fcs, achBuf, sizeof(achBuf));
   ConnectResult r = ConnectResult(f & ConnectResultMask);
   BOOL readonly = (f & ConnectEditable) != ConnectEditable;

   switch (r) {
      case ConnectDisabled:
         SetProxyDialog(hwndDlg, FALSE, FALSE);
         break;

      case ConnectEnabled:
         SetProxyDialog(hwndDlg, TRUE, FALSE);
         Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PROXY_ADDR), readonly);
         Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PROXY_PORT), readonly);
         SetDlgItemText(hwndDlg, IDC_PROXY_ADDR, "");
         SetDlgItemText(hwndDlg, IDC_PROXY_PORT, "");
         break;

      case ConnectHttpUrl:
         {
            URL url(achBuf, URL::HTTP);

            SetDlgItemText(hwndDlg, IDC_PROXY_ADDR, url.getHostSz());
            SetDlgItemText(hwndDlg, IDC_PROXY_PORT, url.getPortSz());
         }
         SetProxyDialog(hwndDlg, TRUE, FALSE);
         Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PROXY_ADDR), readonly);
         Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PROXY_PORT), readonly);
         break;

      case ConnectSocksUrl:
         SetProxyDialog(hwndDlg, TRUE, FALSE);
         Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PROXY_ADDR), readonly);
         Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PROXY_PORT), readonly);

         {
            URL url(achBuf, URL::SOCKS);

            SetDlgItemText(hwndDlg, IDC_PROXY_ADDR, url.getHostSz());
            SetDlgItemText(hwndDlg, IDC_PROXY_PORT, url.getPortSz("1080"));
         }
         break;

      case ConnectConfigUrl:
         SetProxyDialog(hwndDlg, FALSE, TRUE);
         Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_CONFIG_URL), readonly);
         SetDlgItemText(hwndDlg, IDC_CONFIG_URL, achBuf);
         break;
   }
}

static ConnectSetting GetProxyDialog(HWND hwndDlg, char *pach, size_t cch) {
   if (IsWindowVisible(GetDlgItem(hwndDlg, IDC_PROXY_ADDR))) {
      UINT i = GetDlgItemText(hwndDlg, IDC_PROXY_ADDR, pach, cch - 6);

      pach[i++] = ':';

      GetDlgItemText(hwndDlg, IDC_PROXY_PORT, pach + i, cch - i);
   }
   else {
      GetDlgItemText(hwndDlg, IDC_CONFIG_URL, pach, cch);
   }

   return gsettings[ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_PROXY_CONFIG))];
}

static void SetProxyDialog(HWND hwndDlg, BOOL fAddrPort, BOOL fConfigUrl) {
   DWORD dwAddrPort = fAddrPort ? SW_SHOW : SW_HIDE;
   DWORD dwConfigUrl = fConfigUrl ? SW_SHOW : SW_HIDE;

   ShowWindow(GetDlgItem(hwndDlg, IDC_ADDR_LBL), dwAddrPort);
   ShowWindow(GetDlgItem(hwndDlg, IDC_PROXY_ADDR), dwAddrPort);
   ShowWindow(GetDlgItem(hwndDlg, IDC_PORT_LBL), dwAddrPort);
   ShowWindow(GetDlgItem(hwndDlg, IDC_PROXY_PORT), dwAddrPort);
   ShowWindow(GetDlgItem(hwndDlg, IDC_CURL_LBL), dwConfigUrl);
   ShowWindow(GetDlgItem(hwndDlg, IDC_CONFIG_URL), dwConfigUrl);
}

static void TestProxy(HWND hwndResult, ConnectSetting f, const char *psz,
					  const URL &root) {
   HttpRequest *preq = NULL;
   
   try {
      preq = NewHttpRequest(f, psz);

      if (preq == NULL) {
         char achInvalidSettings[128];
         LoadString(ghResourceInstance, IDS_INVALID_SETTINGS, achInvalidSettings, ELEMENTS(achInvalidSettings));
         SetWindowText(hwndResult, achInvalidSettings);
      }
      else {
         preq->send("HEAD", root);
         SetWindowText(hwndResult, preq->getResponse());
      }
   } catch (BaseError &e) {
      char ach[1024];

      e.format(ach, sizeof(ach));

      SetWindowText(hwndResult, ach);
   }

   delete preq;
}

static BOOL CALLBACK DisplayDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   SettingsWindow *pw = (SettingsWindow *) GetWindowLong(hwndDlg, DWL_USER);

   switch (uMsg) {
      case WM_INITDIALOG:
         // WM_INITDIALOG 
         // hwndFocus = (HWND) wParam; // handle of control to receive focus 
         // lInitParam = lParam;       // initialization parameter, for property sheets:
         // PSP = (PROPSHEETPAGE*) lParam
         //
         {
            PROPSHEETPAGE *ppsp = (PROPSHEETPAGE *) lParam;

            ::SetWindowLong(hwndDlg, DWL_USER, (LONG) ppsp->lParam);
            pw = (SettingsWindow *) ppsp->lParam;

            CheckDlgButton(hwndDlg, IDC_USE_COLUMNS, pw->getPopupMenuColumns() ? BST_CHECKED : BST_UNCHECKED);
         }
         // The dialog box procedure should return TRUE to direct Windows to
         // set the keyboard focus to the control given by hwndFocus.
         // Otherwise, it should return FALSE to prevent Windows from setting
         // the default keyboard focus. 
         //
         return TRUE;

      case WM_COMMAND:
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         switch (LOWORD(wParam)) {
            case IDC_TEST_ICON:
               if (HIWORD(wParam) == STN_CLICKED) {
                  pw->showBookmarkPopup();
                  return TRUE;
               }
               break;

            case IDC_USE_COLUMNS:
               if (HIWORD(wParam) == BN_CLICKED) {
                  pw->setPopupMenuColumns(IsDlgButtonChecked(hwndDlg, IDC_USE_COLUMNS) == BST_CHECKED);
                  PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                  return TRUE;
               }
               break;
         }
         return FALSE;

      /*
      case WM_HELP:
         // WM_HELP
         // lphi = (LPHELPINFO) lParam;
         return TRUE;
      */

      case WM_NOTIFY:
         // WM_NOTIFY
         // idCtrl = (int) wParam;
         // pnmh = (LPNMHDR) lParam; 
         //
         {
            int idCtrl = (int) GetWindowLong(hwndDlg, DWL_USER);
            const NMHDR *phdr = (const NMHDR *) lParam;

            DWORD dwResult = 0;

            switch (phdr->code) {
               case PSN_SETACTIVE:
                  // Returns zero to accept the activation or -1 to activate
                  // the next or previous page (depending on whether the user
                  // chose the Next or Back button). To set the activation to
                  // a particular page, return the resource identifier of the
                  // page. 
                  //

                  // To set the return value, the dialog box procedure for the
                  // page must use the SetWindowLong function with the
                  // DWL_MSGRESULT value, and the dialog box procedure must
                  // return TRUE. 
                  //
                  break;

               case PSN_APPLY:
                  pw->applyPopupMenuColumns(IsDlgButtonChecked(hwndDlg, IDC_USE_COLUMNS) == BST_CHECKED);

                  {
                     RegKey key;

                     if (key.open(HKEY_CURRENT_USER, "Software\\SyncIT\\BookmarkSync", true)) {
                        key.setValue("PopupMenuColumns", pw->getPopupMenuColumns());
                     }
                  }
                  break;

               case PSN_QUERYCANCEL:
                  pw->resetPopupMenuColumns();
                  break;

               case PSN_KILLACTIVE:
                  break;

               default:
                  return FALSE;
            }

            SetWindowLong(hwndDlg, DWL_MSGRESULT, dwResult);

            return TRUE;
         }

      default:
         return FALSE;
   }
}


//------------------------------------------------------------------------------
tstring SettingsWindow::getRegistrationUrl() const
{
    std::vector<char> buf(1024);
    RegKey key;
    if (key.open(HKEY_CURRENT_USER, KEY_SYNCIT)) 
    {
        size_t len = key.queryValue("Root", &buf[0], buf.size());
        if (len > 0)
        {
            tstring url(&buf[0], len);
            if (url[url.length() - 1] != '/')
            {
                url.append("/");
            }

            len = key.queryValue("RegistrationUrl", &buf[0], buf.size());
            if (len > 0)
            {
                if (buf[0] == '/')
                {
                    url.append(&buf[1], len - 1);
                }
                else
                {
                    url.append(&buf[0], len);
                }

                return url;
            }
        }
    }

    return "http://syncit.com/common/login.asp";
}