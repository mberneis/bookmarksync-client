/*
 * SyncIT/MenuBookmarks.cxx
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
 */
#pragma warning( disable : 4786 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <algorithm>

#include "MenuBookmarks.h"

#include "SyncLib/util.h"

MenuBookmarks::MenuBookmarks() {

#ifndef NDEBUG
   strcpy(m_achStartTag, "MenuBookmarks");
   strcpy(m_achEndTag, "MenuBookmarks");
#endif /* NDEBUG */

   m_iNumBookmarks = 0;
   m_iMaxBookmarks = 64;

   m_papbBookmarks = (const Bookmark **) u_malloc(m_iMaxBookmarks * sizeof(const Bookmark *));
   m_iRootId = m_iNextId = 0;
   m_pahMenus = NULL;

   m_pbfRoot = NULL;

   assert(isValid());
}

/* virtual */
MenuBookmarks::~MenuBookmarks() {
   assert(isValid());

   BookmarkObject::Detach(m_pbfRoot);

   u_free0(m_pahMenus);
   u_free(m_papbBookmarks);

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */
}

#ifndef NDEBUG
bool MenuBookmarks::isValid() const {
   if (m_pbfRoot != NULL) {

      assert(m_pahMenus != NULL);
      if (m_pahMenus == NULL) return false;

      for (size_t i = 0; i < m_iMenus; i++) {
         assert(IsMenu(m_pahMenus[i]));
         if (!IsMenu(m_pahMenus[i])) return false;
      }
   }
   else {
      assert(m_pahMenus == NULL);
      if (m_pahMenus != NULL) return false;
   }

   for (size_t n = 0; n < m_iNumBookmarks; n++) {
      assert(m_papbBookmarks[n]->isValid());
      if (!m_papbBookmarks[n]->isValid())
         return false;
   }

   assert(strcmp(m_achStartTag, "MenuBookmarks") == 0);
   assert(strcmp(m_achEndTag, "MenuBookmarks") == 0);
   assert(m_iMaxBookmarks > 2);
   assert(0 <= m_iNumBookmarks && m_iNumBookmarks <= m_iMaxBookmarks);
   assert(m_iNextId == (int) m_iNumBookmarks + m_iRootId);

   return strcmp(m_achStartTag, "MenuBookmarks") == 0 &&
          strcmp(m_achEndTag, "MenuBookmarks") == 0 &&
          m_iMaxBookmarks > 2 &&
          0 <= m_iNumBookmarks && m_iNumBookmarks <= m_iMaxBookmarks &&
          m_iNextId == (int) m_iNumBookmarks + m_iRootId;
}
#endif /* NDEBUG */

void MenuBookmarks::setRoot(MenuCursor *pcursor,
                            const BookmarkFolder *pbf,
                            UINT wFolderId) {
   BookmarkFolder *pbfCopy = pbf == NULL ? NULL : NEW BookmarkFolder(*pbf);

   assert(isValid());

   // clean up old menus
   //
   if (m_pahMenus != NULL) {
      assert(m_pbfRoot != NULL);
      for (size_t i = 0; i < m_iMenus; i++) {
         if (m_pahMenus[i] != NULL) {
            DestroyMenu(m_pahMenus[i]);
         }
      }

      u_free(m_pahMenus);
   }

   m_iNumBookmarks = 0;
   m_iNextId = m_iRootId;

   if (pbfCopy != NULL) {
      vector<const Bookmark *> vb;
      vector<const BookmarkFolder *> vf;

      ExtractFromFolder(pbfCopy, vb, vf);

      vector<const Bookmark *>::iterator bi = vb.begin(), be = vb.end();
      vector<const BookmarkFolder *>::iterator fi = vf.begin(), fe = vf.end();

      sort(bi, be, BookmarkLess);
      sort(fi, fe, BookmarkFolderLess);

      int j = 0, k = 0;

      m_pahMenus = (HMENU *) u_calloc(fe - fi, sizeof(HMENU));

      while (fi != fe) {
         const BookmarkFolder *pbfSub = (*fi++), *pbfNext;
         const char *pszName = pbfSub->getName();

         HMENU hsubmenu = ::CreatePopupMenu();
         MenuCursor cursor(hsubmenu, pcursor->getColumnSize());

         populateMenu(cursor, pbfSub, wFolderId);

         while (fi != fe &&
                strcmp((pbfNext = (*fi))->getName(), pszName) == 0) {
            populateMenu(cursor, pbfNext, wFolderId);
            fi++;
         }

         pcursor->insertSubMenu(pszName, (void *) pbfSub,
                                wFolderId, hsubmenu);

         m_pahMenus[j++] = hsubmenu;
      }

      m_iMenus = j;

      if (j > 0) {
         pcursor->insertSeparator();
      }

      while (bi != be) {
         const Bookmark *pb = (*bi++);

         pcursor->insertItem(pb->getName(), (void *) pb, addBookmark(pb));
         k++;
      }

      if (k > 0) {
         pcursor->insertSeparator();
      }
   }
   else {
      m_pahMenus = NULL;
   }

   BookmarkObject::Detach(m_pbfRoot);

   m_pbfRoot = pbfCopy;
}

void MenuBookmarks::getRoot(MenuCursor *pcursor, UINT wFolderId) {
   assert(isValid());

   if (m_pbfRoot != NULL) {
      vector<const Bookmark *> vb;
      vector<const BookmarkFolder *> vf;

      ExtractFromFolder(m_pbfRoot, vb, vf);

      vector<const Bookmark *>::iterator bi = vb.begin(), be = vb.end();
      vector<const BookmarkFolder *>::iterator fi = vf.begin(), fe = vf.end();

      sort(bi, be, BookmarkLess);
      sort(fi, fe, BookmarkFolderLess);

      int j = 0, k = 0;

      while (fi != fe) {
         const BookmarkFolder *pbf = (*fi++), *pbfNext;
         const char *pszName = pbf->getName();

         while (fi != fe &&
                (*fi)->isFolder() &&
                strcmp((pbfNext = (const BookmarkFolder *) (*fi))->getName(), pszName) == 0) {
            fi++;
         }

         pcursor->insertSubMenu(pszName, (void *) pbf, wFolderId, m_pahMenus[j++]);
      }

      if (j > 0) {
         pcursor->insertSeparator();
      }

      int iRootStart = m_iNumBookmarks - vb.size();

      k = 0;

      while (bi != be) {
         const Bookmark *pb = (*bi++);

         assert(m_papbBookmarks[iRootStart + k] == pb);
         pcursor->insertItem(pb->getName(), (void *) pb, m_iRootId + iRootStart + k);
         k++;
      }

      if (k > 0) {
         pcursor->insertSeparator();
      }
   }
}

bool MenuBookmarks::containsId(int id) const {
   return m_iRootId <= id && id < m_iNextId;
}

bool MenuBookmarks::executeId(int id) const {
   char achRoot[MAX_PATH];

   GetRootDirectory(achRoot, ELEMENTS(achRoot));

   if (m_iRootId <= id && id < m_iNextId) {
      char achUrl[4096];
      const Bookmark *pb = m_papbBookmarks[id - m_iRootId];
      const char *pszImageUrl = pb->getImages(BookmarkItem::DEFAULT_IMAGE)->getUrl();

      pb->getHref().format(achUrl, sizeof(achUrl));

      if (pszImageUrl == NULL || strchr(pszImageUrl, '$') == NULL) {
         ShellExecute(NULL,         // hwnd parent
                      TEXT("open"), // pszOperation
                      achUrl,       // pszFile
                      NULL,         // pszParameters, should be NULL if pszFile is a document file
                      achRoot,      // pszDefaultDirectory
                      0);           // nShowCmd, should be 0 if pszFile is a document file
      }
      else {
         ShellExecute(NULL,         // hwnd parent
                      TEXT("open"), // pszOperation
                      "GGVIEW.EXE", // pszFile
                      achUrl,       // pszParameters, should be NULL if pszFile is a document file
                      achRoot,      // pszDefaultDirectory
                      0);           // nShowCmd, should be 0 if pszFile is a document file
      }

      return true;
   }
   else {
      return false;
   }
}

void MenuBookmarks::populateMenu(MenuCursor &cursor, const BookmarkFolder *pbf, UINT wFolderId) {
   vector<const Bookmark *> vb;
   vector<const BookmarkFolder *> vf;

   ExtractFromFolder(pbf, vb, vf);

   vector<const Bookmark *>::iterator bi = vb.begin(), be = vb.end();
   vector<const BookmarkFolder *>::iterator fi = vf.begin(), fe = vf.end();

   sort(bi, be, BookmarkLess);
   sort(fi, fe, BookmarkFolderLess);

   int j = 0, k = 0;

   while (fi != fe) {
      const BookmarkFolder *pbfSub = (*fi++);

      HMENU hsubmenu = ::CreatePopupMenu();
      MenuCursor subcursor(hsubmenu, cursor.getColumnSize());

      populateMenu(subcursor, pbfSub, wFolderId);

      cursor.insertSubMenu(pbfSub->getName(), (void *) pbfSub,
                           wFolderId, hsubmenu);
      j++;
   }

   if (j > 0 && bi != be)
      cursor.insertSeparator();

   while (bi != be) {
      const Bookmark *pb = (*bi++);

      cursor.insertItem(pb->getName(), (void *) pb, addBookmark(pb));
   }
}

int MenuBookmarks::addBookmark(const Bookmark *pb) {
   if (m_iNumBookmarks == m_iMaxBookmarks) {
      m_iMaxBookmarks = (m_iMaxBookmarks * 3) / 2;

      m_papbBookmarks = (const Bookmark **) u_realloc(m_papbBookmarks,
                                                      m_iMaxBookmarks * sizeof(const Bookmark *));
   }

   m_papbBookmarks[m_iNumBookmarks++] = pb;
   return m_iNextId++;
}
