/*
 * SyncLib/NetscapeInfo.cxx
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
 * Last Modification: 30 Apr 1999
 *
 * Description:
 *    Get Netscape v4.5-specific information.  This requires parsing
 *    files in the NSREG.DAT binary file format.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma warning( disable : 4786 )

#include "util.h"
#include "text.h"
#include "RegKey.h"
#include "NetscapeInfo.h"

namespace syncit {

#define NETSCAPE_KEY "Software\\Netscape\\Netscape Navigator"

#define MAGIC_NUMBER    0x76644441L
#define MAJOR_VERSION   1          /* major version for incompatible changes */
#define MINOR_VERSION   2          /* minor ver for new (compatible) features */

typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef          __int32  int32;

typedef int32 REGOFF;   /* offset into registry file */

typedef struct _hdr {
   uint32  magic;      /* must equal MAGIC_NUMBER */
   uint16  verMajor;   /* major version number */
   uint16  verMinor;   /* minor version number */
   REGOFF  avail;      /* next available offset */
   REGOFF  root;       /* root object */
} REGHDR;

typedef struct _desc {
   REGOFF  location;   /* this object's offset (for verification) */
   REGOFF  name;       /* name string */
   uint16  namelen;    /* length of name string (including terminator) */
   uint16  type;       /* node type (key, or entry style) */
   REGOFF  left;       /* next object at this level (0 if none) */
   REGOFF  down;       /* KEY: first subkey        VALUE: 0 */
   REGOFF  value;      /* KEY: first entry object  VALUE: value string */
   uint32  valuelen;   /* KEY: 0  VALUE: length of value data */
#if 0 /* ignore, not necessary for correct operation */
   uint32  valuebuf;   /* KEY: 0  VALUE: length available */
   REGOFF  parent;     /* the node on the immediate level above */
#endif

} REGDESC;

static bool FindValue(HANDLE hFile,
                      REGOFF offsetToKey,
                      const char * const *pszKeyNames,
                      int cKeyNames,
                      char *pach, size_t cch);

static HANDLE OpenRandomReadFile(const char *pszFilename);

static bool Read(HANDLE h, REGOFF offset, void *pb, size_t cb);

static const char *gpszLastNetscapeUser[] = {
   "/", "Common", "Netscape", "ProfileManager", "LastNetscapeUser"
};

bool syncit::GetNetscapeV40Info(char *pachProfileDirectory, size_t cchProfileDirectory) {
   bool result = false;

   RegKey key;

   if (key.open(HKEY_LOCAL_MACHINE, NETSCAPE_KEY "\\Users")) {
      char achCurrentUser[128];

      if (key.queryValue("CurrentUser", achCurrentUser, sizeof(achCurrentUser)) > 0) {
         RegKey keyUser;

         if (keyUser.open(key, achCurrentUser) &&
             keyUser.queryValue("DirRoot", pachProfileDirectory, cchProfileDirectory)) {
            result = true;
         }
      }
   }

   return result;
}

bool syncit::GetNetscapeV20Info(char *pachBookmarkFile, size_t cchBookmarkFile) {
   RegKey key;

   return key.open(HKEY_CURRENT_USER, NETSCAPE_KEY "\\Bookmark List") &&
          key.queryValue("File Location", pachBookmarkFile, cchBookmarkFile) > 0;
}

bool syncit::GetNetscapeV45Info(char *pachProfileDirectory, size_t cchProfileDirectory) {
   HANDLE h;

   char achDirectory[MAX_PATH];
   DWORD dw = GetWindowsDirectoryA(achDirectory, sizeof(achDirectory) - sizeof("\\nsreg.dat"));

   bool fDone = false;

   lstrcpyA(achDirectory + dw, "\\nsreg.dat");

   h = OpenRandomReadFile(achDirectory);

   if (h != INVALID_HANDLE_VALUE) {

      REGHDR hdr;

      if (Read(h, 0, &hdr, sizeof(hdr))) {
         if (hdr.magic == MAGIC_NUMBER && hdr.verMajor == MAJOR_VERSION) {
            char achLastUser[512];

            if (FindValue(h, hdr.root, gpszLastNetscapeUser, 5,
                achLastUser, sizeof(achLastUser))) {
               char *apsz[4];

               apsz[0] = "/";
               apsz[1] = "Users";
               apsz[2] = achLastUser;
               apsz[3] = "ProfileLocation";

               fDone = FindValue(h, hdr.root, apsz, 4, pachProfileDirectory, cchProfileDirectory);
            }
         }
      }

      CloseHandle(h);
   }

   return fDone;
}

//------------------------------------------------------------------------------
bool GetMozillaInfo(const char* subpath, char *pachProfileDirectory, size_t cchProfileDirectory) 
{
   HANDLE h;

   typedef HRESULT (STDAPICALLTYPE *PFN_SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPSTR);
   typedef BOOL (STDAPICALLTYPE *PFN_SHGETSPECIALFOLDERPATH)(HWND, LPSTR, int, BOOL);

   bool fDone = false;

   PFN_SHGETFOLDERPATH pfnSHGetFolderPath = NULL;
   PFN_SHGETSPECIALFOLDERPATH pfnSHGetSpecialFolderPath = NULL;

   // try loading SHGetFolderPath from SHFolder.DLL
   HMODULE hModule = LoadLibrary("SHFolder");
   if (hModule)
   {
       pfnSHGetFolderPath = reinterpret_cast<PFN_SHGETFOLDERPATH>(GetProcAddress(hModule, "SHGetFolderPathA"));
   }

   // try loading from Shell32.DLL
   if (!pfnSHGetFolderPath)
   {
       if (hModule)
       {
           FreeLibrary(hModule);
       }

        hModule = LoadLibrary("Shell32");
        if (hModule)
        {
            pfnSHGetFolderPath = reinterpret_cast<PFN_SHGETFOLDERPATH>(GetProcAddress(hModule, "SHGetFolderPathA"));
        }

        if (!pfnSHGetFolderPath)
        {
            // try locating SHGetSpecialFolderPath
            pfnSHGetSpecialFolderPath = reinterpret_cast<PFN_SHGETSPECIALFOLDERPATH>(GetProcAddress(hModule, "SHGetSpecialFolderPathA"));
        }
   }


   TCHAR achDirectory[MAX_PATH];
   if (   pfnSHGetFolderPath && SUCCEEDED((*pfnSHGetFolderPath)(NULL, 0x001a /* = CSIDL_APPDATA */ , NULL, 0, achDirectory))
       || pfnSHGetSpecialFolderPath && (*pfnSHGetSpecialFolderPath)(NULL, achDirectory, 0x001a /* = CSIDL_APPDATA */, FALSE))
   {
       if (achDirectory[lstrlen(achDirectory) - 1] != '\\')
       {
           lstrcatA(achDirectory, "\\");
       }

       if (subpath)
       {
           lstrcatA(achDirectory, subpath);
       }
       lstrcat(achDirectory, "registry.dat");

       h = OpenRandomReadFile(achDirectory);

       if (h != INVALID_HANDLE_VALUE) {

           REGHDR hdr;

           if (Read(h, 0, &hdr, sizeof(hdr))) {
               if (hdr.magic == MAGIC_NUMBER && hdr.verMajor == MAJOR_VERSION) {
                   char achLastUser[512];

                   static const char *gpszCurrentUser[] = {
                       "/", "Common", "Profiles", "CurrentProfile"
                   };

                   if (FindValue(h, hdr.root, gpszCurrentUser, 4, achLastUser, sizeof(achLastUser))) {
                       char *apsz[5];

                       apsz[0] = "/";
                       apsz[1] = "Common";
                       apsz[2] = "Profiles";
                       apsz[3] = achLastUser;
                       apsz[4] = "directory";

                       fDone = FindValue(h, hdr.root, apsz, 5, pachProfileDirectory, cchProfileDirectory);
                   }
               }
           }

           CloseHandle(h);
       }
   }

   if (hModule)
   {
       FreeLibrary(hModule);
   }

   return fDone;
}

static bool FindValue(HANDLE hFile,
                      REGOFF offsetToKey,
                      const char * const *pszKeyNames,
                      int cKeyNames,
                      char *pach, size_t cch) {
   REGDESC desc;

   bool fDone = false;

   do {
      if (Read(hFile, offsetToKey, &desc, sizeof(desc))) {
         if (desc.location != offsetToKey) {
            offsetToKey = 0;
         }
         else {
            char *pn = NEW char[desc.namelen];

            if (pn == NULL) {
               /* Out of memory */
               offsetToKey = 0;
            }
            else {
               if (!Read(hFile, desc.name, pn, desc.namelen)) {
                  /* Can't read name */
                  offsetToKey = 0;
               }
               else if (lstrcmpA(pn, *pszKeyNames) != 0) {
                  offsetToKey = desc.left;
               }
               else if (cKeyNames == 1) {
                  /* read value */
                  if (desc.valuelen > 0 && Read(hFile, desc.value, pach, min(desc.valuelen, cch))) {
                     if (desc.valuelen < cch) {
                        pach[desc.valuelen] = '\0';
                        fDone = true;
                     }
                  }

                  offsetToKey = 0;
               }
               else if (cKeyNames == 2 && desc.valuelen == 0) {
                  offsetToKey = desc.value;
                  pszKeyNames++;
                  cKeyNames--;
               }
               else {
                  offsetToKey = desc.down;
                  pszKeyNames++;
                  cKeyNames--;
               }

               delete[] pn;
            }
         }
      }
      else {
         offsetToKey = 0;
      }
   } while (offsetToKey != 0);

   return fDone;
}

static HANDLE OpenRandomReadFile(const char *pszFilename) {
   return ::CreateFileA(pszFilename,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_RANDOM_ACCESS,
                        NULL);
}

static bool Read(HANDLE h, REGOFF offset, void *pb, size_t cb) {
   DWORD dwRead;

   SetFilePointer(h, offset, NULL, FILE_BEGIN);

   if (ReadFile(h,
                pb,
                cb,
                &dwRead,
                NULL)) {
      return dwRead == cb;
   }
   else {
      return false;
   }
}

} // namespace syncit

