/*
 * SyncIT/PopupMenu.h
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
 *    The SyncIT taskbar icon has a popup menu
 *    which contains bookmarks.  These menu items are dynamic,
 *    so they don't have command constants associated with them.
 *    Although there is a way of storing per-application data
 *    with a menu item, there doesn't seem to be a way to extract
 *    that information when the menu WM_COMMAND is invoked.  Duh.
 *    This class solves this problem.  It relies on defined
 *    constants in resource.h ID_BOOKMARK and ID_SUBSCRIPTION
 *    which are large enough that no other commands will ever
 *    overwrite it.  It allocates IDs starting from this point,
 *    storing the bookmark in an array thus associating the ID
 *    with the bookmark.
 *
 * See also:
 *    PopupMenu.cxx     -- defines the class declared in this module
 *    MenuCursor.h      -- formats menu items into rows and columns
 *    MenuCursor.cxx
 *    MenuBookmarks.h   -- keeps track of a single set of bookmarks,
 *    MenuBookmarks.cxx    used for both subscriptions and bookmarks
 *    StatusWindow.h    -- uses the PopupMenu to create menu items
 *    StatusWindow.cxx     in the popup menu
 *    WinMain.cxx       -- uses the PopupMenu to invoke commands
 *                         when the menu item is selected
 */
#ifndef PopupMenu_H
#define PopupMenu_H

#include "SyncLib/util.h"

#include "BookmarkLib/BookmarkModel.h"

#include "MenuBookmarks.h"
#include "MenuCursor.h"

#include "resource.h"

class PopupMenu {

#ifndef NDEBUG
public:
   bool isValid() const;
private:
   char m_achStartTag[sizeof("PopupMenu")];
#endif /* NDEBUG */

public:
   PopupMenu() /* throws Win32Error */;

   virtual ~PopupMenu();

   enum PopupSet {
      Popup_BOOKMARKS,
      Popup_SUBSCRIPTIONS,

      NUM_POPUP_SETS
   };

   /**
    * Create a popup menu populated by the contents of the folder.
    * First come the bookmark folders, each recursively a popup menu;
    * then a separator; then the bookmarks themselves.  Each bookmark
    * menu item has an command ID of ID_BOOKMARK plus a bit...
    */
   void setBookmarks(PopupSet s, const BookmarkFolder *pbfBookmarks);

   /**
    * Helpers for the window proc...
    */
   bool measureItem(UINT idCtl, MEASUREITEMSTRUCT *pmis, HDC hdc);
   bool drawItem(UINT idCtl, DRAWITEMSTRUCT *pmis);

   bool showMenuUrl(int id);

   HMENU getMenu() const {
      return m_hPopupMenu;
   }

   int getColumnSize() const {
      return m_nColumnSize;
   }

   void setColumnSize(int i) {
      m_nColumnSize = i;
   }

private:
   MenuBookmarks  m_sets[NUM_POPUP_SETS];
   Bookmark       m_bookmarksync;

   HMENU m_hPopupMenu;  // submenu, the real popup

   int m_nColumnSize;

   COLORREF m_crSelText, m_crSelBkgnd;
   HFONT m_hFont;
   int m_cx;

#ifndef NDEBUG
private:
   char m_achEndTag[sizeof("PopupMenu")];
#endif /* NDEBUG */

};

#endif /* PopupMenu_H */
