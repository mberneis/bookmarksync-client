/*
 * SyncIT/About.cxx
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
 *   Calls up the SyncIT agent's about box.  The about box
 *   is a simple modeless dialog box.  It is modeless because
 *   the SyncIT agent's main loop should never be stopped to
 *   handle window loops as a lot of timeout and file change
 *   processing occurs in the main loop.
 *
 * See also:
 *    About.h     -- contains the declarations of the method(s)
 *                   defined here
 *    WinMain.cxx -- calls up the dialog box on command
 *    SyncIT.rc   -- contains the template for the about box
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

#include "About.h"
#include "resource.h"

#include "ProductVersion.h"

// DialogProc forward declaration...
//
static BOOL APIENTRY AboutDlgProc(HWND hDlg,
                                  UINT wMsg,
                                  WPARAM wParam,
                                  LONG lParam);

static HWND ghDialog = NULL;

extern HINSTANCE ghResourceInstance;

extern HWND MyCreateDialogParam(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);

/*
 * Display the about box, loading it initially if required.
 * If the dialog is already being shown, it is moved to the
 * foreground.  If it is iconized, it is deiconized.  If it
 * isn't being shown at all, it is loaded and shown normally.
 *
 * @param hInstance
 *
 * @see AboutDlgProc          (later in this module)
 * @see IDD_ABOUT_BOX         (defined in SyncIT.rc)
 * @see CreateDialog          (Win32 routine)
 * @see ShowWindow            (Win32 routine)
 * @see SetForegroundWindow   (Win32 routine)
 */
void DisplayAboutBox(HINSTANCE hInstance) {

   if (ghDialog == NULL) {
      ghDialog = MyCreateDialogParam(ghResourceInstance,               // hInstance
                                     MAKEINTRESOURCE(IDD_ABOUT_BOX),   // pTemplate
                                     NULL,                             // hWndParent
                                     (DLGPROC) AboutDlgProc,           // pDialogFunction
                                     0);
   }

   ::ShowWindow(ghDialog,           // hWnd -- handle to window
                SW_SHOWNORMAL);     // nCmdShow

   ::SetForegroundWindow(ghDialog); // hwnd
}

/**
 * About box dialog procedure.
 * <p>
 * This is the main dialog box routine for the IDD_ABOUT_BOX
 * template.
 * <p>
 * Except in response to the WM_INITDIALOG message,
 * the dialog box procedure should return TRUE (nonzero) if it
 * processes the message, and FALSE (zero) if it does not.
 */
static BOOL APIENTRY AboutDlgProc(HWND hDlg,
                                  UINT wMsg,
                                  WPARAM wParam,
                                  LONG lParam)
{
   lParam;  // unreferenced

   switch (wMsg) {
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
         SetDlgItemText(hDlg, IDC_VERSION, "v" SYNCIT_V_STR " - " __DATE__);
         return TRUE;

      case WM_DRAWITEM:
         DrawLinkButton(hDlg, (DRAWITEMSTRUCT *) lParam);
         return TRUE;

      /*
       * Look for an ESC or RETURN event.
       */
      case WM_COMMAND:
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         switch (LOWORD(wParam)) {
            case ID_HOMEPAGE:
            case ID_HOMEPAGE2:
               {
                  TCHAR achUrl[512];

                  GetWindowText((HWND) lParam, achUrl, 512);
                  ShellExecute(hDlg,
                               TEXT("open"),
                               achUrl,
                               NULL,
                               TEXT("."),
                               0);
               }
               return TRUE;

            case IDOK:
            case IDCANCEL:
               ::EndDialog(hDlg,    // hDlg -- handle to dialog box
                           TRUE);   // nResult -- value to return
               ghDialog = NULL;
               return TRUE;
         }
         break;
   }

   // default handler
   //
   return FALSE;
}

void DrawLinkButton(HWND hDlg, DRAWITEMSTRUCT *pdis) {
   TCHAR achUrl[512];

   DWORD dw = GetDlgItemText(hDlg, pdis->CtlID, achUrl, 512);

   COLORREF oldc = SetTextColor(pdis->hDC, RGB(0, 0, 255));
   RECT rc = pdis->rcItem;

   if (pdis->itemState & ODS_SELECTED) { // if selected 
      rc.top++;
      rc.left++;
   }
   else {
      rc.right++;
      rc.bottom++;
   }

   FillRect(pdis->hDC, &rc, (HBRUSH) (COLOR_BTNFACE + 1));

   DrawText(pdis->hDC,
            achUrl,
            dw,
            &rc,
            DT_CENTER);

   SetTextColor(pdis->hDC, oldc);
}
