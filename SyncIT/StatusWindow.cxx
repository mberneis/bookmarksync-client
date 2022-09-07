/*
 * SyncIT/StatusWindow.cxx
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
#include <windowsx.h>

#include "StatusWindow.h"
#include "About.h"
#include "resource.h"

#include "SyncLib/Errors.h"
#include "SyncLib/Log.h"

using namespace syncit;

enum {
   MYWM_APP = WM_APP,

   MYWM_NOTIFYICON
};

extern HINSTANCE ghResourceInstance;

extern HWND MyCreateDialogParam(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);

StatusWindow::StatusWindow(HINSTANCE hInstance) : m_popup() {

#ifndef NDEBUG
   strcpy(m_achStartTag, "StatusWindow");
   strcpy(m_achEndTag, "StatusWindow");
#endif /* NDEBUG */

   WNDCLASSEX  wndClass;

   wndClass.cbSize = sizeof(wndClass);
   wndClass.style = 0;
   wndClass.lpfnWndProc = DefDlgProc;
   wndClass.cbClsExtra = 0;
   wndClass.cbWndExtra = DLGWINDOWEXTRA;
   wndClass.hInstance = hInstance;
   wndClass.hIcon = LoadIcon(ghResourceInstance, MAKEINTRESOURCE(IDI_LIVE));
   wndClass.hIconSm = NULL;
   wndClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
   wndClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
   wndClass.lpszMenuName = NULL;
   wndClass.lpszClassName = TEXT("SyncItStatus");
   // class name must be reflected in the properties of the IDD_STATUS
   //   dialog box

   ::RegisterClassEx(&wndClass);

   m_hwnd = MyCreateDialogParam(hInstance,
                                MAKEINTRESOURCE(IDD_STATUS),
                                NULL,
                                (DLGPROC) StatusDlgProc,
                                (LPARAM) this);

   if (m_hwnd == NULL) {
      throw Win32Error("CreateDialog");
   }

   {
      HWND hwndButton = GetDlgItem(m_hwnd, ID_STATUS_DISMISS);

      if (hwndButton != NULL) {
         HCURSOR hcursor = LoadCursor(ghResourceInstance, MAKEINTRESOURCE(IDC_HAND));

         if (hcursor != NULL) {
            SetClassLong(hwndButton, GCL_HCURSOR, (LONG) hcursor);
         }
      }
   }

   m_pListener = NULL;

   m_hMenu = LoadMenu(ghResourceInstance, MAKEINTRESOURCE(IDM_POPUP));
   if (m_hMenu == NULL) {
      throw Win32Error("LoadMenu");
   }

   m_hPopupMenu = ::GetSubMenu(m_hMenu, 0);

   assert(m_hPopupMenu != NULL);

   if (::SetMenuDefaultItem(m_hPopupMenu,          // hMenu
                            ID_POPUP_SYNCNOW,      // pos/id
                            FALSE) == 0) {         // fByPosition
      throw Win32Error("SetMenuDefaultItem");
   }

   // initialize the tray icon data
   //
   m_nid.cbSize = sizeof(m_nid);
   m_nid.hWnd = m_hwnd;
   m_nid.uID = 0;
   m_nid.hIcon = NULL;
}

StatusWindow::~StatusWindow() {
   ::DestroyMenu(m_hMenu);

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */

}

#ifndef NDEBUG
bool StatusWindow::isValid() const {
   return strcmp(m_achStartTag, "StatusWindow") == 0 &&
          strcmp(m_achEndTag, "StatusWindow") == 0 &&
          m_popup.isValid();
}
#endif /* NDEBUG */

void StatusWindow::show() {

   assert(isValid());

   ::ShowWindow(m_hwnd, SW_SHOWNORMAL);
   ::SetForegroundWindow(m_hwnd);
   ::UpdateWindow(m_hwnd);
}

void StatusWindow::addIcon(int idi, int ids, ...) {
   va_list ap;
   va_start(ap, ids);
   notifyIcon(NIM_ADD, idi, ids, ap);
   va_end(ap);
}

void StatusWindow::modifyIcon(int idi, int ids, ...) {
   va_list ap;
   va_start(ap, ids);
   notifyIcon(NIM_MODIFY, idi, ids, ap);
   va_end(ap);
}

void StatusWindow::resetIcon() {
   m_nid.uFlags = 0;
   ::Shell_NotifyIcon(NIM_DELETE, &m_nid);

   m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
   m_nid.uCallbackMessage = MYWM_NOTIFYICON;

   ::Shell_NotifyIcon(NIM_ADD, &m_nid);
}

void StatusWindow::notifyIcon(DWORD dwMessage, int idi, int ids, va_list ap) {
   m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
   m_nid.uCallbackMessage = MYWM_NOTIFYICON;

   if (m_nid.hIcon != NULL) {
      ::DestroyIcon(m_nid.hIcon);
   }

   m_nid.hIcon = (HICON) LoadImage(ghResourceInstance,
                                   MAKEINTRESOURCE(idi),
                                   IMAGE_ICON,
                                   16, 16, 0);

   TCHAR achFormat[ELEMENTS(m_nid.szTip)];

   ::LoadString(ghResourceInstance, ids, achFormat, ELEMENTS(achFormat));

   ::FormatMessage(FORMAT_MESSAGE_FROM_STRING,  // flags
                   achFormat,                   // source
                   0,                           // message ID
                   0,                           // language
                   m_nid.szTip,                 // buffer
                   ELEMENTS(m_nid.szTip),       // cchBuffer
                   &ap);                        // arguments

   ::Shell_NotifyIcon(dwMessage, &m_nid);

   // set the window's icon to match
   //
   {
      HICON hIcon = ::LoadIcon(ghResourceInstance, MAKEINTRESOURCE(idi));

      ::SetClassLong(m_hwnd, GCL_HICON, (LONG) hIcon);

      ::DestroyIcon(hIcon);
   }
}

void StatusWindow::removeIcon() {
   if (m_nid.hIcon != NULL) {
      ::DestroyIcon(m_nid.hIcon);
      m_nid.hIcon = NULL;
   }

   m_nid.uFlags = 0;
   ::Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

void StatusWindow::setStatus(int ids, ...) {
   TCHAR achFormat[1024];
   TCHAR achBuffer[2048];

   va_list ap;

   va_start(ap, ids);

   int i;
   i = ::LoadString(ghResourceInstance,
                    ids, achFormat, ELEMENTS(achFormat));

   assert(0 <= i && i < ELEMENTS(achFormat));

   if (i > 0) {
      assert(achFormat[i] == TEXT('\0'));

      i = ::FormatMessage(FORMAT_MESSAGE_FROM_STRING,
                          achFormat,
                          0,     // dwMessage(ignored)
                          0,     // dwLangId(ignored)
                          achBuffer,
                          ELEMENTS(achBuffer),
                          &ap);

      assert(0 <= i && i < ELEMENTS(achBuffer));

      if (i > 0) {
         assert(achBuffer[i] == TEXT('\0'));

         Log("Status: %s\r\n", achBuffer);

         ::SetDlgItemText(m_hwnd, IDC_STATUS, achBuffer);
      }
   }

   va_end(ap);
}

BOOL APIENTRY StatusDlgProc(HWND hDialog, UINT wMsg, WPARAM wParam, LONG lParam) {
   StatusWindow *p = (StatusWindow *) ::GetWindowLong(hDialog, DWL_USER);

   switch (wMsg) {
      case WM_INITDIALOG:
         // WM_INITDIALOG
         // hwndFocus = (HWND) wParam; // handle of control to receive focus
         // lInitParam = lParam;       // initialization parameter

         assert(((StatusWindow *) lParam)->isValid());

         ::SetWindowLong(hDialog, DWL_USER, lParam);

         // The dialog box procedure should return TRUE to direct Windows to
         // set the keyboard focus to the control given by hwndFocus.
         // Otherwise, it should return FALSE to prevent Windows from setting
         // the default keyboard focus. 
         //
         return TRUE;

      case WM_SETTINGCHANGE:
         p->m_pListener->resetPopup();
         return TRUE;

      case WM_HELP:
         // WM_HELP
         // lphi = (LPHELPINFO) lParam;
         p->m_pListener->onHelp(hDialog, (const HELPINFO *) lParam, TEXT("status-window"));
         return TRUE;

      case MYWM_NOTIFYICON:
         switch (lParam) {
            case WM_RBUTTONDBLCLK:
            case WM_LBUTTONDBLCLK:
               p->m_pListener->onSyncNow();
               return TRUE;

            case WM_LBUTTONDOWN:
               p->showBookmarkPopup();
               return TRUE;

            case WM_RBUTTONDOWN:
               p->showStatusPopup();
               return TRUE;
         }

         return FALSE;

      case WM_CONTEXTMENU:
         // hwnd = (HWND) wParam;
         // xPos = LOWORD(lParam);
         // yPos = HIWORD(lParam); 
         //
         p->showStatusPopup();
         return TRUE;

      case WM_DESTROY:
         PostQuitMessage(0);
         p->m_hwnd = NULL;

         return TRUE;

      case WM_CLOSE:
         ::ShowWindow(p->m_hwnd, SW_HIDE);
         return TRUE;

      case WM_MEASUREITEM:
         // idCtl = (UINT) wParam;
         // lpmis = (MEASUREITEMSTRUCT *) lParam;
         //
         {
            HDC hdc = GetDC(hDialog);

            p->m_popup.measureItem((UINT) wParam,
                                   (MEASUREITEMSTRUCT *) lParam,
                                   hdc);

            ReleaseDC(hDialog, hdc);
         }
         // If an application processes this message, it should return TRUE. 
         return TRUE;

      case WM_DRAWITEM:
         // idCtl = (UINT) wParam;
         // lpdis = (DRAWITEMSTRUCT *) lParam;
         //
         p->m_popup.drawItem((UINT) wParam,
                             (DRAWITEMSTRUCT *) lParam);

         // If an application processes this message, it should return TRUE. 
         return TRUE;

      case WM_COMMAND:
         // If an application processes this message, it should return zero. 
         //
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         switch (LOWORD(wParam)) {
            case IDCANCEL:
            case ID_STATUS_DISMISS:
               ::ShowWindow(hDialog, SW_HIDE);
               break;

            case ID_POPUP_HELP:
               p->m_pListener->onHelp(hDialog, TEXT("default.htm"));
               break;

            case ID_POPUP_ABOUT:
               DisplayAboutBox(GetWindowInstance(hDialog));
               break;

            case ID_POPUP_SYNCNOW:        // from popup menu
               p->m_pListener->onSyncNow();
               break;

            case ID_POPUP_ORGANIZE:       // organize from popup menu
               p->m_pListener->onOrganize();
               break;

            case ID_POPUP_STATUS:
               p->show();
               break;

            case ID_POPUP_SETTINGS:
               p->m_pListener->onShowSettings(0);
               break;

            case ID_RESETSYNCIT:       // from external programs
               p->resetIcon();
               break;

            case ID_POPUP_EXIT:        // from popup menu
            case ID_SHUTDOWNSYNCIT:    // from external programs
               p->m_pListener->onShutdown();
               break;

            case ID_POPUP_AUTOSYNC:
               p->m_pListener->toggleAutoSync();
               break;

            case ID_POPUP_REFER:
               {
                  char achUrl[4096];
                  size_t offset;

                  lstrcpyA(achUrl, "refer.asp?email=");
                  offset = lstrlenA(achUrl);
                  GetDlgItemText(hDialog, IDC_EMAIL, achUrl + offset, sizeof(achUrl) - offset);
                  p->m_pListener->showDocument(achUrl);
               }
               break;

            case ID_HOMEPAGE:
               p->m_pListener->showDocument("main.asp");
               break;

            default:
               if (HIWORD(wParam) != 0) {
                  return FALSE;
               }

               p->m_popup.showMenuUrl(wParam);
         }
         return TRUE;

      default:
         return FALSE;
   }
}

void StatusWindow::showBookmarkPopup() {
   showPopup(m_popup.getMenu());
}

void StatusWindow::showStatusPopup() {
   showPopup(m_hPopupMenu);
}

void StatusWindow::setAutoSyncCheck(bool fChecked) {
   UINT uCheck = fChecked ? MF_CHECKED : MF_UNCHECKED;

   CheckMenuItem(m_hPopupMenu, ID_POPUP_AUTOSYNC, MF_BYCOMMAND | uCheck);
}

void StatusWindow::showPopup(HMENU hmenu) {

   POINT pt;

   if (GetCursorPos(&pt)) {
      // Pulled from MSDN "PRB: Menus for Notification Icons Don't Work
      // Correctly"
      // Article Q135788
      //
      // The gist is this: menus don't go away unless
      // the owner window is in the foreground and a null
      // message is posted. 
      //
      SetForegroundWindow(m_hwnd);
      TrackPopupMenuEx(hmenu,
                       TPM_HORIZONTAL,
                       pt.x,
                       pt.y,
                       m_hwnd,
                       NULL);

      PostMessage(m_hwnd, WM_NULL, 0, 0);
   }
}
