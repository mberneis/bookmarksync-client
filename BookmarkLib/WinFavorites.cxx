/*
 * BookmarkLib/WinFavorites.cxx
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
#include "BrowserBookmarks.h"
#include "WinFavorites.h"

#include "SyncLib/RegKey.h"
#include "SyncLib/Errors.h"

using namespace syncit;

static const char FAVORITES[] = "Favorites\\";
static const char MicrosoftFolders[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";

const char WindowsFavorites::m_gszSuffix[] = ".url";

const char WindowsFavorites::m_gszInternetShortcut[] = "[InternetShortcut]";

const tchar_t WindowsFavorites::m_gachMap[20] = {
   '"', '\'',
   '*', '+',
   '/', '-',
   '\\', '_',
   ':', ';',
   '<', '{',
   '>', '}',
   '?', '~',
   '|', '!',
   '%', '%'
};

size_t WindowsFavorites::GetDefaultDirectory(TCHAR *pach, size_t cch) {
   RegKey key;
   size_t u;

   // Look for Internet Explorer Favorites...
   //
   if (!key.open(HKEY_CURRENT_USER, MicrosoftFolders) ||
       (u = key.queryValue("Favorites", pach, cch)) == 0) {

      u = GetWindowsDirectory(pach,    // LPTSTR lpBuffer
                              cch);    // UINT   uSize

      if (u == 0) {
         throw Win32Error("GetWindowsDirectory");
      }

      if (pach[u - 1] != '\\') {
         pach[u++] = '\\';
         pach[u] = 0;
      }

      u += bufcopy(FAVORITES, pach + u, cch - u);
   }

   return u;
}
