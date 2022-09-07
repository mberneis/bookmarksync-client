/*
 * SyncIT/MenuBookmarks.h
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
 * This class declares the MenuBookmarks class,
 * which keeps track of a set of bookmarks used
 * in the top-level task-bar popup menu of the
 * SyncIT agent.
 *
 * The idea here is that the user's bookmarks and
 * subscriptions (and possibly other sets) need to
 * be displayed on the popup menu, yet need to be
 * edited separately.
 *
 * So this class keeps track of a single set of bookmarks,
 * and can insert them into the top-level popup menu.
 * The PopupMenu class then instantiates of these MenuBookmarks
 * classes, one for bookmarks and one for subscriptions.
 */
#ifndef MenuBookmarks_H
#define MenuBookmarks_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */
#include <windows.h>

#include "BookmarkLib/BookmarkModel.h"

#include "MenuCursor.h"

using namespace syncit;

class MenuBookmarks {

#ifndef NDEBUG
public:
   bool isValid() const;
private:
   char m_achStartTag[sizeof("MenuBookmarks")];
#endif /* NDEBUG */

public:
   MenuBookmarks();
   virtual ~MenuBookmarks();

   void setRootId(int id) {
      m_iRootId = m_iNextId = id;
   }

   void setRoot(MenuCursor *pcursor,
                const BookmarkFolder *pbf,
                UINT wFolderId);

   void getRoot(MenuCursor *pcursor, UINT wFolderId);

   bool containsId(int id) const;
   bool executeId(int id) const;

protected:
   void populateMenu(MenuCursor &cursor, const BookmarkFolder *pbf, UINT wFolderId);
   int addBookmark(const Bookmark *pb);

private:
   BookmarkFolder *m_pbfRoot;

   size_t m_iNumBookmarks;
   size_t m_iMaxBookmarks;
   const Bookmark **m_papbBookmarks;

   int m_iRootId;
   int m_iNextId;

   HMENU *m_pahMenus;
   size_t m_iMenus;

#ifndef NDEBUG
private:
   char m_achEndTag[sizeof("MenuBookmarks")];
#endif /* NDEBUG */
};

#endif /* MenuBookmarks_H */
