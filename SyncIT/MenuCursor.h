/*
 * SyncIT/MenuCursor.h
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
 */
#ifndef MenuCursor_H
#define MenuCursor_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */
#include <windows.h>

/**
 * A MenuCursor wraps a handle to a popup menu
 * and keeps track of the current position (current row and
 * column) as items are inserted into it.
 *
 * The first item is added to the top (position 0) and further
 * items are added after the last item inserted.  There is a
 * maximum number of items per column, after this is exceeded,
 * a menu break (no bar) is inserted to create a new column.
 *
 * Separators can be added at any time, these are horizontal
 * separators, or if occuring at the beginning or end of a column,
 * a menu bar break.
 */
class MenuCursor {

#ifndef NDEBUG
private:
   char m_achStartTag[sizeof("MenuCursor")];
public:
   bool isValid() const;
#endif /* NDEBUG */

public:
   MenuCursor(HMENU hmenu, int nColumnSize);
   virtual ~MenuCursor();

   /**
    * Insert an owner-draw submenu into this menu.
    *
    * @param hSubMenu  -- handle to submenu to insert
    * @param pszString -- string label of menu item
    * @param pv        -- item data of menu item, used
    *                     during owner-draw operations.
    */
   bool insertSubMenu(LPCTSTR pszString, void *pv, UINT wID, HMENU hSubMenu);

   /**
    * Insert an owner-draw menu item into this menu.
    *
    * @param wID       -- the command ID of the menu item
    * @param pszString -- string label of menu item
    * @param pv        -- item data of menu item, used
    *                     during owner-draw operations.
    */
   bool insertItem(LPCTSTR pszString, void *pv, UINT wID);

   /**
    * Insert either a vertical or horizontal separator
    */
   bool insertSeparator();

   int getColumnSize() const {
      return m_nColumnSize;
   }

protected:
   /**
    * Insert a menu item, either a submenu or a single menu item.
    *
    * @param pszString  -- string label of menu item
    * @param pv        -- item data of menu item, used
    *                     during owner-draw operations.
    * @param fMask     -- a combination of the MIIM_ constants, like MIIM_ID or
    *                     MIIM_SUBMENU
    * @param WID       -- the control ID of the menu item
    * @param hSubMenu  -- ignored unless fMask contains MIIM_SUBMENU
    */
   bool insert(LPCTSTR pszString, void *pv, UINT fMask, UINT wID, HMENU hSubMenu);

   /**
    * Insert a menu item based on the MENUITEMINFO structure
    */
   bool insert(const MENUITEMINFO *pmii);

private:
   HMENU m_hmenu;    // handle to current popup menu
   UINT  m_uPos;     // current position, starts at 0 and is
                     // incremented on every insert(...)

   int   m_nColumnSize;       // maximum rows per column
   int   m_row, m_col;        // current row, column

   bool  m_fSubMenuOnColumn;        // is there a submenu on current column
   bool  m_fNeedVerticalSeparator;  // true if insertSeparator() occurred while
                                    // m_row == m_nColumnSize

#ifndef NDEBUG
private:
   char m_achEndTag[sizeof("MenuCursor")];
#endif /* NDEBUG */

};

#endif /* MenuCursor_H */
