/*
 * SyncIT/PopupMenu.cxx
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
 *    This class solves this problem.  It relies on a defined
 *    constant in resource.h ID_BOOKMARK which is large enough
 *    so no other commands will ever overwrite it.  It allocates
 *    IDs starting from this point, storing the bookmark in an
 *    array thus associating the ID with the bookmark.
 *
 * See also:
 *    PopupMenu.h       -- declares the class defined in this module
 *    Synchronizer.h    -- uses the PopupMenu to create menu items
 *    Synchronizer.cxx     in the popup menu
 *    WinMain.cxx       -- uses the PopupMenu to invoke commands
 *                         when the menu item is selected
 *
 * Change history:
 * 9-Dec-1998  Terence Way
 *    Large numbers of bookmarks prevent the popup menu from being shown:
 *    a temporary fix is to size the popup menus proportional to the size
 *    of the screen, then more than 20 bookmarks can be shown per column.
 *    Another change truncates long bookmarks to 300 pixels: then most
 *    screens can have three columns.  The big change, to come later, will
 *    be to have an HTML-like, scrolling display of bookmarks, with no
 *    limit...
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "PopupMenu.h"

#include "SyncLib/Errors.h"

PopupMenu::PopupMenu() {
#ifndef NDEBUG
   strcpy(m_achStartTag, "PopupMenu");
   strcpy(m_achEndTag, "PopupMenu");
#endif /* NDEBUG */

   m_sets[Popup_BOOKMARKS].setRootId(ID_BOOKMARK);
   m_sets[Popup_SUBSCRIPTIONS].setRootId(ID_SUBSCRIPTION);

   m_nColumnSize = GetSystemMetrics(SM_CYSCREEN) / GetSystemMetrics(SM_CYMENU);

   m_crSelText = GetSysColor(COLOR_HIGHLIGHTTEXT);
   m_crSelBkgnd = GetSysColor(COLOR_HIGHLIGHT);

   m_hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);

   m_cx = GetSystemMetrics(SM_CYEDGE);

   m_hPopupMenu = ::CreatePopupMenu();

   assert(m_hPopupMenu != NULL);

   m_bookmarksync.setName("BookmarkSync Web Site");
   m_bookmarksync.setHref("http://www.bookmarksync.com/main.asp");

   assert(isValid());
}

PopupMenu::~PopupMenu() {
   assert(isValid());

   ::DestroyMenu(m_hPopupMenu);

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */

}

#ifndef NDEBUG
/* virtual */
bool PopupMenu::isValid() const {
   return strcmp(m_achStartTag, TEXT("PopupMenu")) == 0 &&
          strcmp(m_achEndTag, TEXT("PopupMenu")) == 0 &&
          m_sets[Popup_BOOKMARKS].isValid() && m_sets[Popup_SUBSCRIPTIONS].isValid();
}
#endif /* NDEBUG */

void PopupMenu::setBookmarks(PopupSet s, const BookmarkFolder *pbf) {
   int num = ::GetMenuItemCount(m_hPopupMenu), i;

   m_crSelText = GetSysColor(COLOR_HIGHLIGHTTEXT);
   m_crSelBkgnd = GetSysColor(COLOR_HIGHLIGHT);

   m_hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);

   m_cx = GetSystemMetrics(SM_CYEDGE);
   while (num-- > 0) {
      ::RemoveMenu(m_hPopupMenu, 0, MF_BYPOSITION);
      // NO! ::DeleteMenu(m_hPopupMenu, 0, MF_BYPOSITION);
   }

   MenuCursor cursor(m_hPopupMenu, m_nColumnSize);

   for (i = 0; i < NUM_POPUP_SETS; i++) {
      if (i == s) {
         m_sets[i].setRoot(&cursor, pbf, i);
      }
      else {
         m_sets[i].getRoot(&cursor, i);
      }
   }

   cursor.insertItem(m_bookmarksync.getName(), &m_bookmarksync, ID_HOMEPAGE);
}

bool PopupMenu::measureItem(UINT idCtl, MEASUREITEMSTRUCT *pmis, HDC hdc) {
   if (pmis->CtlType == ODT_MENU && pmis->itemData != NULL) {
      HFONT hOldFont = (HFONT) SelectObject(hdc, m_hFont);

      const BookmarkItem *pbi = (const BookmarkItem *) pmis->itemData;

      LPCTSTR pszName = pbi->getName();
      SIZE size;
      int width = 300, height = 16;

      int m;

      m = pbi->getImages(BookmarkItem::OPEN_IMAGE)->getHeight();
      if (m > height) height = m;
      width = pbi->getImages(BookmarkItem::OPEN_IMAGE)->getWidth();

      m = pbi->getImages(BookmarkItem::CLOSED_IMAGE)->getHeight();
      if (m > height) height = m;
      m = pbi->getImages(BookmarkItem::CLOSED_IMAGE)->getWidth();
      if (m > width)  width  = m;

      ::GetTextExtentPoint32(hdc,
                             pszName,
                             tstrlen(pszName),
                             &size);

      pmis->itemWidth  = min(300, max(width, m_cx + 16 + m_cx + size.cx));
      pmis->itemHeight = max(height, size.cy);

      ::SelectObject(hdc, hOldFont);

      return true;
   }
   else {
      return false;
   }
}

bool PopupMenu::drawItem(UINT idCtl, DRAWITEMSTRUCT *pdis) {
   if (pdis->CtlType == ODT_MENU && pdis->itemData != NULL) {
      COLORREF crText;            // text color of unselected item      
      COLORREF crBkgnd;           // background color unselected item   
      HFONT hOldFont = (HFONT) SelectObject(pdis->hDC, m_hFont);

      const BookmarkItem *pbi = (const BookmarkItem *) pdis->itemData;
      LPCTSTR pszName = pbi->getName();
      bool fSelected = (pdis->itemState & ODS_SELECTED) == ODS_SELECTED;

      Image *pImage = pbi->getImages(fSelected ? BookmarkItem::SELECTED_IMAGE : BookmarkItem::DEFAULT_IMAGE);

      if (fSelected) {
         crText = SetTextColor(pdis->hDC, m_crSelText);
         crBkgnd = SetBkColor(pdis->hDC, m_crSelBkgnd);

         FillRect(pdis->hDC, &pdis->rcItem, (HBRUSH) (COLOR_HIGHLIGHT + 1));
      }
      else {
         FillRect(pdis->hDC, &pdis->rcItem, (HBRUSH) (COLOR_MENU + 1));
      }

      RECT rcText = pdis->rcItem;

      rcText.left += m_cx + 16 + m_cx;

      if (pImage->getWidth() > 16) {
         pImage->draw(pdis->hDC, rcText.left, rcText.top);
      }
      else {
         DrawTextEx(pdis->hDC,
                    (char *) pszName,
                    -1,
                    &rcText,
                    DT_END_ELLIPSIS | DT_NOPREFIX,
                    NULL);

         pImage->draw(pdis->hDC,
                      m_cx + pdis->rcItem.left,
                      pdis->rcItem.top);
      }

      if (fSelected) {
         SetTextColor(pdis->hDC, crText);
         SetBkColor(pdis->hDC, crBkgnd);
      }

      SelectObject(pdis->hDC, hOldFont);

      return true;
   }
   else {
      return false;
   }
}

bool PopupMenu::showMenuUrl(int id) {
   if (m_sets[0].containsId(id)) return m_sets[0].executeId(id);
   if (m_sets[1].containsId(id)) return m_sets[1].executeId(id);
   return false;
}
