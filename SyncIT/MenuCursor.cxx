/*
 * SyncIT/MenuCursor.cxx
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
 *    This module keeps track of menu items in a popup menu,
 *    put horizontal or vertical separators in where necessary.
 *
 *    The positioning rules:
 *    1. No more than 20 items per column
 *    2. A separator (either vertical or horizontal) must be
 *       between the folders and the bookmarks (naturally,
 *       only if there are bookmarks *and* folders)
 *    3. A separator (either vertical or horizontal) must be
 */
#include <cassert>

#include "MenuCursor.h"

#include "SyncLib/util.h"
#include "SyncLib/text.h"

using namespace syncit;

MenuCursor::MenuCursor(HMENU hmenu, int nColumnSize) {

#ifndef NDEBUG
   strcpy(m_achStartTag, "MenuCursor");
   strcpy(m_achEndTag, "MenuCursor");
#endif /* NDEBUG */

   m_hmenu = hmenu;
   m_uPos = 0;
   m_row  = 0;
   m_col  = 0;

   m_fSubMenuOnColumn = false;
   m_fNeedVerticalSeparator = false;
   m_nColumnSize = nColumnSize;

   assert(isValid());
}

MenuCursor::~MenuCursor() {

   assert(isValid());

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */

}

#ifndef NDEBUG
bool MenuCursor::isValid() const {
   return strcmp(m_achStartTag, "MenuCursor") == 0 &&
          strcmp(m_achEndTag, "MenuCursor") == 0 &&
          0 <= m_row && (m_nColumnSize == -1 || m_row <= m_nColumnSize) &&
          IsMenu(m_hmenu);
}
#endif /* NDEBUG */

/**
 * Insert an owner draw submenu into this menu.
 *
 * @param hSubMenu  -- handle to submenu to insert
 * @param pszString -- string label of menu item
 * @param pv        -- item data of menu item
 */
bool MenuCursor::insertSubMenu(LPCTSTR pszString, void *pv, UINT wID, HMENU hSubMenu) {
   assert(isValid());

   m_fSubMenuOnColumn = true;

   return insert(pszString, pv, MIIM_SUBMENU | MIIM_ID, wID, hSubMenu);
}

/**
 * Insert a horizontal menu separator, separating two
 * items on the same column.
 */
bool MenuCursor::insertSeparator() {
   assert(isValid());

   if (m_row == 0) {
      // don't need a separator...
      //
      return true;
   }
   else if (m_row == m_nColumnSize) {
      m_fNeedVerticalSeparator = true;

      return true;
   }
   else {
      MENUITEMINFO mii;

      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE;
      mii.fType = MFT_SEPARATOR;

      return insert(&mii);
   }
}

/**
 * Insert an owner-draw menu item into this menu.
 *
 * @param wID       -- the command ID of the menu item
 * @param pszString -- string label of menu item
 * @param pv        -- item data of menu item
 */
bool MenuCursor::insertItem(LPCTSTR pszString, void *pv, UINT wID) {
   assert(isValid());

   return insert(pszString, pv, MIIM_ID, wID, NULL);
}

bool MenuCursor::insert(LPCTSTR pszString, void *pv, UINT fMask, UINT wID, HMENU hSubMenu) {
   MENUITEMINFO mii;

   assert(pszString != NULL);
   assert((fMask & MIIM_SUBMENU) == 0 || IsMenu(hSubMenu));

   mii.cbSize = sizeof(mii);
   mii.fMask = fMask | MIIM_DATA | MIIM_STATE | MIIM_TYPE;
   mii.fType = MFT_OWNERDRAW | MFT_STRING;
   mii.fState = MFS_ENABLED;
   mii.wID = wID;
   mii.hSubMenu = hSubMenu;

   mii.dwItemData = (DWORD) pv;
   mii.dwTypeData = (LPTSTR) pszString;
   mii.cch = tstrlen(pszString);

   if (m_fNeedVerticalSeparator) {
      mii.fType |= MFT_MENUBARBREAK;
      m_col++;
      m_row = 0;

      m_fSubMenuOnColumn = (fMask & MIIM_SUBMENU) == MIIM_SUBMENU;
      m_fNeedVerticalSeparator = false;
   }
   else if (m_row == m_nColumnSize) {
      mii.fType |= MFT_MENUBREAK;
      m_col++;
      m_row = 0;

      m_fSubMenuOnColumn = (fMask & MIIM_SUBMENU) == MIIM_SUBMENU;
   }

   // 0 <= m_row && m_row < m_nColumnSize
   m_row++;
   // 0 < m_row && m_row <= m_nColumnSize

   return insert(&mii);
}

bool MenuCursor::insert(const MENUITEMINFO *pmii) {
   assert(isValid());

   return InsertMenuItem(m_hmenu,         // hMenu -- menu into which new item is inserted
                         m_uPos++,        // uItem -- by pos or by ID
                         TRUE,            // fByPosition
                         pmii) != FALSE;  // pMenuItemInfo
}
