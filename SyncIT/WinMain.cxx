/*
 * SyncIT/WinMain.cxx
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
 *    This module contains the WinMain function: the entry
 *    point for this Windows application.
 */
#pragma warning( disable : 4786 )

#include "MozillaBrowser.h"
#include "MicrosoftBrowser.h"
#include "NetscapeBrowser.h"

#include "SyncLib\Socket.h"
#include "SyncLib\Errors.h"
#include "SyncLib\Log.h"

#include "Synchronizer.h"
#include "About.h"
#include "resource.h"
#include "BuiltinImages.h"

#include "..\ProductVersion.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace syncit;

#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

BuiltinImages *syncit::gpBuiltins;
ImageLoader   *syncit::gpLoader;

static void Localize(HINSTANCE hInstance);
static HINSTANCE FindResourceFile(HINSTANCE hDefaultInstance,
                                  const char **papszNames, int nNames,
                                  const LANGID *paLangIDs, int nLangIDs);
static HINSTANCE LoadResourceFile(const char *pszName, LANGID langid);

HINSTANCE ghResourceInstance;

typedef void (*PFN)();

static void loadextension(const char *pszExtension) {
   HMODULE hdll = (HMODULE) ::LoadLibraryA(pszExtension);

   if (hdll != NULL) {
      PFN pfn = (PFN) ::GetProcAddress(hdll, "StartExtension");

      if (pfn != NULL) {
         try {
            (*pfn)();
         } catch (...) {
         }
      }
      else {
         ::FreeLibrary(hdll);
      }
   }
}

int WINAPI WinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance,
                   LPSTR lpszLine,
                   int nShow)
{
   int         result;
   HANDLE      hMutex;

   Localize(hInstance);

   bool retry;
   do {
      int error = IDS_CANNOT_START;

      retry = false;

      try {
         char achBuf[1024];

         LoadString(ghResourceInstance, IDS_APP_UUID, achBuf, ELEMENTS(achBuf));

         // hPrevInstance should always be NULL in Win95/98/NT applications
         //
         assert(hPrevInstance == NULL);

         // Attempt to get an exclusive "lock" on the SyncIt application,
         // so only one copy is running at a time.
         //
         hMutex = CreateMutex(NULL,   // < security attributes
                              FALSE,  // < bInitialOwner       v pszName
                              achBuf);
      
         if (hMutex == NULL) {
            throw Win32Error("CreateMutex");
         }

         if (GetLastError() == ERROR_ALREADY_EXISTS) {
            // if an existing copy is already running,
            // reset that running copy and exit silently
            //
            LoadString(ghResourceInstance, IDS_STATUS_CAPTION, achBuf, ELEMENTS(achBuf));

            HWND hwnd = FindWindow(TEXT("SyncItStatus"), achBuf);

            if (hwnd != NULL) {
               if (lstrcmpA(lpszLine, "stop") == 0) {
                  SendMessage(hwnd, WM_COMMAND, 40011, 0);
                  Sleep(2 * 1000L);
                  SendMessage(hwnd, WM_CLOSE, 0, 0);
                  Sleep(2 * 1000L);
                  SendMessage(hwnd, WM_QUIT, 0, 0);
                  Sleep(2 * 1000L);
               }
               else {
                  SendMessage(hwnd, WM_COMMAND, ID_RESETSYNCIT, 0);
               }
            }
            result = 1;
         }
         else if (lstrcmpA(lpszLine, "stop") != 0) {
            LogInitialize(TEXT("SyncIT"));

            Log("SyncIT version %s starting\r\n", SYNCIT_V_STR);

            MicrosoftBrowser ms;
            MozillaBrowser mozilla("Mozilla",          "mozilla", "Mozilla");
            MozillaBrowser phoenix("Mozilla Firebird", "phoenix", "Phoenix");
            MozillaBrowser ns6    ("Netscape6",        "ns6"    , "Mozilla");
            NetscapeBrowser ns;

          //OperaBrowser opera;

            gpBuiltins = NEW BuiltinImages(ghResourceInstance);
            gpLoader = NEW ImageLoader(hInstance);

            Synchronizer sync(hInstance);

            Log("WSAStartup: %s version = %d.%d, %s\r\n",
                Socket::init.wsadata.szDescription,
                LOBYTE(Socket::init.wsadata.wVersion),
                HIBYTE(Socket::init.wsadata.wVersion),
                Socket::init.wsadata.szSystemStatus);

            sync.addBrowser(&ms);
            sync.addBrowser(&mozilla);
            sync.addBrowser(&phoenix);
            sync.addBrowser(&ns6);
            sync.addBrowser(&ns);
          //sync.addBrowser(&opera);

            sync.start();

            error = IDS_STOPPING_WIN32;
            result = sync.driver();

            Log("SyncIT exiting code %d\r\n", result);
         }
      } catch (BaseError &e) {
         TCHAR achFormat[4096], achBuffer[4096];
         TCHAR achError[512];

         e.format(achError, ELEMENTS(achError));

         LPCTSTR apszArgs[1];

         ::LoadString(ghResourceInstance,
                      error,
                      achFormat,
                      ELEMENTS(achFormat));

         apszArgs[0] = achError;

         ::FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                              FORMAT_MESSAGE_ARGUMENT_ARRAY,
                         achFormat,
                         0,
                         0,
                         achBuffer,
                         ELEMENTS(achBuffer),
                         (va_list *) apszArgs);

         retry = ::MessageBox(NULL,
                              achBuffer,
                              NULL,
                              MB_RETRYCANCEL | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL) == IDRETRY;
      }

      delete gpBuiltins;
      delete gpLoader;

      ::CloseHandle(hMutex);
   } while (retry);

   return result;
}

static const char *RESOURCE_DLLS[]  = { "csrc", "bmrc" };

static void Localize(HINSTANCE hInstance) {
   LANGID langids[1];

   langids[0] = GetUserDefaultLangID();

   ghResourceInstance = FindResourceFile(hInstance, RESOURCE_DLLS, ELEMENTS(RESOURCE_DLLS), langids, 1);
}

static HINSTANCE FindResourceFile(HINSTANCE hDefaultInstance,
                                  const char **papszNames, int nNames,
                                  const LANGID *paLangIDs, int nLangIDs) {
   for (int i = 0; i < nNames; i++) {
      for (int j = 0; j < nLangIDs; j++) {
         HINSTANCE hInstance;
         
         if ((hInstance = LoadResourceFile(papszNames[i], paLangIDs[j])) != NULL) {
            return hInstance;
         }
         
         if ((hInstance = LoadResourceFile(papszNames[i], PRIMARYLANGID(paLangIDs[j]))) != NULL) {
            return hInstance;
         }
      }
   }

   return hDefaultInstance;
}

static HINSTANCE LoadResourceFile(const char *pszName, LANGID langid) {
   char achBuf[1024];
   wsprintfA(achBuf, "%s%04x.DLL", pszName, langid);
   return LoadLibrary(achBuf);
}

HWND MyCreateDialogParam(HINSTANCE hInstance,
                         LPCTSTR pszTemplateName,
                         HWND hwndOwner,
                         DLGPROC dlgProc,
                         LPARAM lParam) {
   HRSRC hrsrc = FindResource(ghResourceInstance, pszTemplateName, RT_DIALOG);
   HGLOBAL hglobal = LoadResource(ghResourceInstance, hrsrc);

   return CreateDialogIndirectParam(hInstance,
                                    (LPCDLGTEMPLATE) LockResource(hglobal),
                                    hwndOwner,
                                    dlgProc,
                                    lParam);
}
