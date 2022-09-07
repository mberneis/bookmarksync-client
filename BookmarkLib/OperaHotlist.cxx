/*
 * BookmarkLib/OperaHotlist.cxx
 * Copyright(c) 1999, SyncIt.com  Inc.
 *
 * Author:            Terence Way
 * Last Modification: 30 Apr 1999
 *
 * Description:
 *    Defines ReadOperaHotlist and WriteOperaHotlist routines, which
 *    can read/write files in the Opera hotlist OPERA3.ADR file format.
 */
#pragma warning( disable : 4786 )

#include "SyncLib/DateTime.h"
#include "SyncLib/RegKey.h"

#include "BrowserBookmarks.h"

using namespace syncit;

// HKEY_LOCAL_MACHINE
//    Software
//       Microsoft
//          Windows
//             CurrentVersion
//                AppPaths
//                   Opera.exe
//                      Path={directory name}
//
static const char OPERA_KEY[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Opera.exe";

size_t OperaHotlist::GetDefaultFilename(TCHAR *pach, size_t cch) {
   RegKey key;
   size_t u;

   if (key.open(HKEY_LOCAL_MACHINE, OPERA_KEY)) {
      u = key.queryValue("Path", pach, cch);

      if (u > 0) {
         if (pach[u - 1] == '\\') {
            u--;
         }

         u += bufcopy("\\opera3.adr", pach + u, cch - u);
      }
   }
   else {
      if (cch > 0) {
         pach[0] = 0;
      }

      u = 0;
   }

   return u;
}
