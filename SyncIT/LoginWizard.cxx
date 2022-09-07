/*
 * SyncIT/LoginWizard.cxx
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
 *    This module displays the 5 step wizard that
 *    logs a user into BookmarkSync.com for the first
 *    time.
 */
#pragma warning( disable : 4786 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "SyncLib/Util.h"

#include "resource.h"
#include "About.h"
#include "Synchronizer.h"

static const char KEY_SYNCIT[] = "Software\\SyncIT\\BookmarkSync";

extern HINSTANCE ghResourceInstance;

static HCURSOR ghCurWait = ::LoadCursor(NULL, IDC_WAIT);
static HCURSOR ghCurArrow = ::LoadCursor(NULL, IDC_ARROW);

/**
 * This maps the Panels enum constants to 
 * resource IDs.
 * PANELS[Panel_Intro] == IDD_INTRO
 * PANELS[Panel_Email] == IDD_EMAIL
 * ...
 * PANELS[Panel_Connect] == IDD_CONNECT
 */
static const int PANELS[] = {
   IDD_REG_INTRO, IDD_REG_LOGIN, IDD_REG_READY, IDD_REG_CONNECT
};

LoginWizard::LoginWizard(HINSTANCE hInstance) {

#ifndef NDEBUG
   strcpy(m_achStartTag, "LoginWizard");
   strcpy(m_achEndTag, "LoginWizard");
#endif /* NDEBUG */

   assert(PANELS[Panel_Intro] == IDD_REG_INTRO);
   assert(PANELS[Panel_Login] == IDD_REG_LOGIN);
 //assert(PANELS[Panel_Browsers]  == IDD_REG_BROWSERS);
   assert(PANELS[Panel_Ready] == IDD_REG_READY);
   assert(PANELS[Panel_Connect] == IDD_REG_CONNECT);

   WNDCLASSEX  wndClass;

   wndClass.cbSize = sizeof(wndClass);
   wndClass.style = 0;
   wndClass.lpfnWndProc = DefDlgProc;
   wndClass.cbClsExtra = 0;
   wndClass.cbWndExtra = DLGWINDOWEXTRA;
   wndClass.hInstance = ghResourceInstance;
   wndClass.hIcon = NULL;
   wndClass.hIconSm = NULL;
   wndClass.hCursor = ghCurArrow;
   wndClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
   wndClass.lpszMenuName = NULL;
   wndClass.lpszClassName = TEXT("SyncItWizard");
   // class name must be reflected in the properties of the 
   // IDD_INTRO, IDD_EMAIL, IDD_PASSWORD, IDD_READY, and IDD_CONNECT
   //   dialog boxes

   RegisterClassEx(&wndClass);

   m_pListener = NULL;

   m_hwnd = NULL;
   m_pszEmail = m_pszPassword = NULL;
   m_state = 0;
   m_iCurPanel = Panel_First;
   ZeroMemory((void *) m_ahwndPanels, sizeof(m_ahwndPanels));

   assert(isValid());
}

/* virtual */
LoginWizard::~LoginWizard() {
   assert(isValid());

   u_free0(m_pszEmail);
   u_free0(m_pszPassword);

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */

}

#ifndef NDEBUG
bool LoginWizard::isValid() const {
   return strcmp(m_achStartTag, "LoginWizard") == 0 &&
          strcmp(m_achEndTag, "LoginWizard") == 0;
}
#endif /* NDEBUG */

extern HWND MyCreateDialogParam(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);

void LoginWizard::create(HWND hwndOwner, LPCTSTR pszEmail) {
   HINSTANCE hInstance = GetWindowInstance(hwndOwner);

   m_pszEmail = strrealloc(m_pszEmail, pszEmail);
   m_pszPassword = strrealloc(m_pszPassword, "");

   m_hwnd = MyCreateDialogParam(ghResourceInstance,
                                MAKEINTRESOURCE(IDD_WIZARD),
                                hwndOwner,
                                (DLGPROC) WizardDlgProc,
                                (LPARAM) this);

   m_ahwndPanels[0] = MyCreateDialogParam(ghResourceInstance,
                                          MAKEINTRESOURCE(PANELS[0]),
                                          m_hwnd,
                                          (DLGPROC) PanelDlgProc,
                                          (LPARAM) this);

   m_iCurPanel = Panel_First;

   ShowWindow(m_ahwndPanels[0], SW_SHOWNORMAL);
   SetWindowText(GetDlgItem(m_ahwndPanels[0], IDC_REGISTER), getRegistrationUrl().c_str());

   ShowWindow(m_hwnd, SW_SHOWNORMAL);
   SetForegroundWindow(m_hwnd);
}

BOOL CALLBACK WizardDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam) {
   LoginWizard *plw = (LoginWizard *) ::GetWindowLong(hDlg, DWL_USER);

   switch (wMsg) {
      case WM_INITDIALOG:
         // WM_INITDIALOG 
         // hwndFocus = (HWND) wParam; // handle of control to receive focus 
         // lInitParam = lParam;       // initialization parameter
         //
         {
            assert(((LoginWizard *) lParam)->isValid());

            ::SetWindowLong(hDlg, DWL_USER, (LONG) lParam);
         }
         // The dialog box procedure should return TRUE to direct Windows to
         // set the keyboard focus to the control given by hwndFocus.
         // Otherwise, it should return FALSE to prevent Windows from setting
         // the default keyboard focus. 
         //
         return TRUE;

      case WM_HELP:
         // WM_HELP
         // lphi = (LPHELPINFO) lParam; 
         plw->m_pListener->onHelp(hDlg, (HELPINFO *) lParam, TEXT("login-wizard"));
         return TRUE;

      case WM_COMMAND:
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         switch (LOWORD(wParam)) {
            case IDCANCEL:
            case ID_WIZ_CANCEL:
               plw->cancel();
               break;

            case IDC_NEXT:
               plw->next();
               break;

            case IDC_BACK:
               plw->back();
               break;

            default:
               return FALSE;
         }
         return TRUE;

      default:
         return FALSE;
   }
}

void LoginWizard::cancel() {
   if (m_iCurPanel == Panel_Connect) {
      back();
   }
   else {
      // order is important...
      // the listener's cancel will probably destroy our parent,
      // so we must destroy first.
      //
      ::DestroyWindow(m_hwnd);

      // the listener may assert that it is in the right state
      // State_CFG insists on the wizard being visible... so
      // we indulge it.
      //
      m_pListener->onCancel();

      // NOW we become !isVisible()
      //
      m_hwnd = NULL;
      m_iCurPanel = Panel_First;
      ZeroMemory((void *) m_ahwndPanels, sizeof(m_ahwndPanels));
   }
}

void LoginWizard::finish() {

   // like cancel but without the notification
   //
   ::DestroyWindow(m_hwnd);

   m_hwnd = NULL;
   m_iCurPanel = Panel_First;
   ZeroMemory((void *) m_ahwndPanels, sizeof(m_ahwndPanels));
}

void LoginWizard::checkpoint(int id) {
   if (m_hwnd != NULL && m_iCurPanel == Panel_Connect) {
      enable(m_ahwndPanels[m_iCurPanel], id, TRUE);
   }
}

void LoginWizard::enable(HWND hwnd, int id, BOOL fState) {
   HWND hwndControl = ::GetDlgItem(hwnd, id);

   if (hwndControl != NULL) {
      ::EnableWindow(hwndControl, fState);
   }
}

static int ENABLES[] = {
   IDC_WIZ_CONNECTING,
   IDC_WIZ_DOWNLOADING,
   IDC_WIZ_UPLOADING
};

void LoginWizard::done() {
   if (m_hwnd != NULL && m_iCurPanel == Panel_Connect) {
      HWND hwndCurrentPage = m_ahwndPanels[m_iCurPanel];

      for (int i = 0; i < ELEMENTS(ENABLES); i++) {
         enable(hwndCurrentPage, ENABLES[i], TRUE);
      }

      ::ShowWindow(::GetDlgItem(hwndCurrentPage, IDC_DONET), SW_SHOWNORMAL);
      ::ShowWindow(::GetDlgItem(hwndCurrentPage, IDC_DONEI), SW_SHOWNORMAL);
      enable(m_hwnd, IDC_BACK, FALSE);
      enable(m_hwnd, IDC_NEXT, TRUE);
      enable(m_hwnd, ID_WIZ_CANCEL, FALSE);

     ::SetClassLong(hwndCurrentPage, GCL_HCURSOR, (LONG) ghCurArrow);
   }
}

bool LoginWizard::isDialogMessage(MSG *pmsg) {
   return m_hwnd != NULL && IsDialogMessage(m_hwnd, pmsg);
}

void LoginWizard::bad(Panels p, HWND hwnd) {
   TCHAR achBuffer[4096];

   GetWindowText(hwnd, achBuffer, ELEMENTS(achBuffer));
   bad(p, achBuffer);
}

void LoginWizard::bad(Panels p, LPCTSTR psz) {
   m_state = 0;

   if (m_hwnd != NULL) {
      go(p);

      wipe();

      HWND hwndCurrentPage = m_ahwndPanels[m_iCurPanel];
      HWND hwndMsg = ::GetDlgItem(hwndCurrentPage, IDC_MSG);

      ::SetClassLong(hwndCurrentPage, GCL_HCURSOR, (LONG) ghCurArrow);

      if (hwndMsg != NULL) {
         ::ShowWindow(::GetDlgItem(hwndCurrentPage, IDC_MSG0), SW_HIDE);
         ::SetWindowText(hwndMsg, psz);
         ::ShowWindow(hwndMsg, SW_SHOWNORMAL);
      }

      if (p == Panel_Login) {
         ::SendDlgItemMessage(hwndCurrentPage, IDC_WIZ_EMAIL, EM_SETSEL, 0, -1);
         ::SetDlgItemText(hwndCurrentPage, IDC_WIZ_PASSWORD, TEXT(""));
      }
   }
}

void LoginWizard::wipe() {
   // wipe out everything BUT p
   //
   int i;

   for (i = 0; i < NUM_PANELS; i++) {
      if (i != m_iCurPanel && m_ahwndPanels[i] != NULL) {
         DestroyWindow(m_ahwndPanels[i]);
         m_ahwndPanels[i] = NULL;
      }
   }
}

static void setTextAndFocus(HWND hwnd, int idc, LPCTSTR pszText) {
   HWND hwndItem = ::GetDlgItem(hwnd, idc);

   if (hwndItem != NULL) {
      SetWindowText(hwndItem, pszText);
      SetFocus(hwndItem);
      SendMessage(hwndItem, EM_SETSEL, 0, -1);
   }
}

/**
 * 1. Hide current window
 * 2. Retrieve current window's value (or do things specific to previous window)
 * 3. Set new window (create if necessary)
 * 4. The header bar (page n of 5)
 * 5. The back button (enabled on pages 2..5, disabled on page 1)
 * 6. The next button (set to "&Next>" on pages 1..4, set to "&Finish" on page 5)
 * 7. Set information specific to new current window
 */
void LoginWizard::go(Panels p) {
   if (Panel_First <= p && p <= Panel_Last && p != m_iCurPanel) {
      HWND hwndCurrentPage = m_ahwndPanels[m_iCurPanel];

      // Step 1 -- Hide current window
      //
      ::ShowWindow(hwndCurrentPage, SW_HIDE);

      // Step 2 -- Retrieve current window's value
      //
      switch (m_iCurPanel) {
         case Panel_Login:
            {
               TCHAR achBuffer[1024];

               ::GetWindowText(GetDlgItem(hwndCurrentPage, IDC_WIZ_EMAIL),
                               achBuffer,
                               ELEMENTS(achBuffer));

               setEmail(achBuffer);

               ::GetWindowText(GetDlgItem(hwndCurrentPage, IDC_WIZ_PASSWORD),
                               achBuffer,
                               ELEMENTS(achBuffer));

               m_pszPassword = strrealloc(m_pszPassword, achBuffer);
            }
            break;

         case Panel_Ready:
            break;

         case Panel_Connect:
            ::SetClassLong(hwndCurrentPage, GCL_HCURSOR, (LONG) ghCurArrow);

            if (p == Panel_Ready) {
               m_pListener->onBack();
            }

            wipe();

            // we always destroy the last page, since it needs to
            // "start from scratch" on each connect...
            ::DestroyWindow(hwndCurrentPage);
            m_ahwndPanels[m_iCurPanel] = NULL;
            break;
      }

      // Step 3 -- Set new window, create if necessary
      //
      hwndCurrentPage = m_ahwndPanels[p];

      if (hwndCurrentPage == NULL) {
         hwndCurrentPage = MyCreateDialogParam(GetWindowInstance(m_hwnd),
                                               MAKEINTRESOURCE(PANELS[p]),
                                               m_hwnd,
                                               (DLGPROC) PanelDlgProc,
                                               (LPARAM) this);

         m_ahwndPanels[p] = hwndCurrentPage;
      }

      // hwndCurrentPage != NULL
      //
      m_iCurPanel = p;

      // Step 4 -- the header bar
      //
      TCHAR achHeaderBuf[1024];

      GetWindowText(hwndCurrentPage, achHeaderBuf, ELEMENTS(achHeaderBuf));
      SetWindowText(m_hwnd, achHeaderBuf);

      // Step 5 -- the back button
      //
      enable(m_hwnd, IDC_BACK, m_iCurPanel > 0);

      // Step 6 -- the next/finish button
      //
      {
         char achBuf[128];

         HWND hwndNext = GetDlgItem(m_hwnd, IDC_NEXT);
         bool fLast = m_iCurPanel == ELEMENTS(PANELS) - 1;

         LoadString(ghResourceInstance, fLast ? IDS_FINISH : IDS_NEXT, achBuf, ELEMENTS(achBuf));
         ::SetWindowText(hwndNext, achBuf);

         ::EnableWindow(hwndNext, !fLast);
      }

      // Step 7 -- set information specific to new window
      //
      switch (p) 
      {
      case Panel_Login:
          SetWindowText(GetDlgItem(hwndCurrentPage, IDC_REGISTER), getRegistrationUrl().c_str());
          setTextAndFocus(hwndCurrentPage, IDC_WIZ_EMAIL, getEmail());
          break;

      case Panel_Connect:
          ::SetClassLong(hwndCurrentPage, GCL_HCURSOR, (LONG) ghCurWait);
          m_pListener->onConnect(m_pszEmail, m_pszPassword);
          m_state = 1;
          break;
      }

      ::ShowWindow(hwndCurrentPage, SW_SHOWNORMAL);
   }
}

extern bool browse(HWND hwndDlg, int idc, const char *lpstrFilter);

BOOL APIENTRY PanelDlgProc(HWND hDlg,
                           UINT wMsg,
                           WPARAM wParam,
                           LPARAM lParam) {
   LoginWizard *plw = (LoginWizard *) GetWindowLong(hDlg, DWL_USER);

   switch (wMsg) {
      case WM_INITDIALOG:
         // WM_INITDIALOG 
         // hwndFocus = (HWND) wParam; // handle of control to receive focus 
         // lInitParam = lParam;       // initialization parameter
         //
         {
            HWND hwndField;

            assert(((LoginWizard *) lParam)->isValid());

            ::SetWindowLong(hDlg, DWL_USER, (LONG) lParam);

            // The dialog box procedure should return TRUE to direct Windows to
            // set the keyboard focus to the control given by hwndFocus.
            // Otherwise, it should return FALSE to prevent Windows from setting
            // the default keyboard focus. 
            //
            if ((hwndField = ::GetDlgItem(hDlg, IDC_WIZ_EMAIL)) != NULL) {
               SetFocus(hwndField);
               return FALSE;
            }
//            else if (::GetDlgItem(hDlg, IDC_NS) != NULL) {
//               setupBrowsers(hDlg);
//               return TRUE;
//            }
            else {
               return TRUE;
            }
         }

      case WM_DRAWITEM:
         DrawLinkButton(hDlg, (DRAWITEMSTRUCT *) lParam);
         return TRUE;

      case WM_COMMAND:
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         switch (LOWORD(wParam)) {
            case IDC_REGISTER:
               plw->m_pListener->showDocument(plw->getRegistrationUrl().c_str());
               return TRUE;

            case IDC_CONNECT_SETTINGS:
               plw->m_pListener->onShowSettings(1);
               return TRUE;
/*
            case IDC_NS_CHANGE:
               if (browse(hDlg, IDC_NS_DIR, "Netscape Bookmarks (*.HTM)\0*.HTM;*.HTML\0")) {
                  ::CheckDlgButton(hDlg, IDC_NS, BST_CHECKED);
               }
               return TRUE;

            case IDC_OP_CHANGE:
               if (browse(hDlg, IDC_OP_DIR, "Opera Hotlist (*.ADR)\0*.ADR\0")) {
                  ::CheckDlgButton(hDlg, IDC_OP, BST_CHECKED);
               }
               return TRUE;
*/
            default:
               return FALSE;
         }

      default:
         return FALSE;
   }
}

//------------------------------------------------------------------------------
tstring LoginWizard::getRegistrationUrl() const
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
