/*
 * BookmarkLib/BookmarkFolder.cxx
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
 *    This file defines methods for the BookmarkFolder class.
 *    A BookmarkFolder object describes a folder or directory
 *    hierarchy of bookmarks in the user's browser bookmark file.
 */
#pragma warning( disable : 4786 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdlib>

#include "BookmarkModel.h"

#include "SyncLib\util.h"
#include "SyncLib\text.h"
#include "SyncLib\Log.h"

using namespace syncit;

BookmarkFolder::BookmarkFolder() : BookmarkItem("") {
#ifndef NDEBUG
   strcpy(m_achStartTag, TEXT("BookmarkFolder"));
   strcpy(m_achEndTag, TEXT("BookmarkFolder"));
#endif /* NDEBUG */

   m_fFolded = true;
}

BookmarkFolder::BookmarkFolder(const BookmarkFolder &rhs) : BookmarkItem(rhs) {
#ifndef NDEBUG
   strcpy(m_achStartTag, TEXT("BookmarkFolder"));
   strcpy(m_achEndTag, TEXT("BookmarkFolder"));
#endif /* NDEBUG */

   copy(rhs);
}

BookmarkFolder::~BookmarkFolder() {
   assert(isValid());

   detachall();

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */

}

BookmarkFolder &BookmarkFolder::operator=(const BookmarkFolder &rhs) {
   BookmarkItem::operator=(rhs);

   clear();
   copy(rhs);

   return *this;
}

void BookmarkFolder::copy(const BookmarkFolder &rhs) {
   BookmarkVector::const_iterator i = rhs.begin(), end = rhs.end();

   while (i != end) {
      BookmarkObject *pold = *i++, *pnew;

      if (pold->isBookmark()) {
         pnew = pold->attach();
      }
      else if (pold->isFolder()) {
         pnew = NEW BookmarkFolder(*(const BookmarkFolder *) pold);
      }
      else if (pold->isAlias()) {
         pnew = pold->attach();
      }
      else if (pold->isSeparator()) {
         pnew = pold->attach();
      }
      else {
         assert(pold->isFolder() || pold->isBookmark() || pold->isAlias() || pold->isSeparator());
      }

      add(pnew);
   }

   m_fFolded = rhs.m_fFolded;
}

void BookmarkFolder::detachall() {
   BookmarkVector::iterator i = m_elements.begin(), end = m_elements.end();
   while (i != end) {
      Detach(*i++);
   }
}

/* virtual */
bool BookmarkFolder::isFolder() const {
   return true;
}

/* virtual */
bool BookmarkFolder::isSubscription() const {
   return false;
}

/* virtual */
bool BookmarkFolder::equals(const BookmarkObject *pbo) const {
   if (pbo->isFolder()) {
      const BookmarkFolder *p = (const BookmarkFolder *) pbo;

      assert(p->isValid());

      if (this == p) {
         return true;
      }
      else if (tstricmp(m_pszName, p->m_pszName) != 0 ||
               m_elements.size() != p->m_elements.size()) {
         return false;
      }
      else {
         return sameElements(p);
      }
   }
   else {
      return false;
   }
}

#ifndef NDEBUG

bool BookmarkFolder::isValid() const {
/*
   int i;

   for (i = 0; i < m_numBookmarks; i++) {
      assert(m_pBookmarks[i]->isValid());
      if (!m_pBookmarks[i]->isValid())
         return false;
   }

   for (i = 0; i < m_numFolders; i++) {
      assert(m_pFolders[i]->isValid());
      if (!m_pFolders[i]->isValid())
         return false;
   }
*/
   return strcmp(m_achStartTag, "BookmarkFolder") == 0 &&
          strcmp(m_achEndTag, "BookmarkFolder") == 0 &&
          m_pszName != NULL;
}

#endif /* NDEBUG */

Bookmark *BookmarkFolder::findBookmark(LPCTSTR pszName) const {
   assert(isValid());
   assert(pszName != NULL);

   BookmarkVector::const_iterator i = m_elements.begin(), end = m_elements.end();
   // just a bookmark name, find bookmark with same title, href

   while (i != end) {
      if ((*i)->isBookmark()) {
         Bookmark *pb = (Bookmark *) (*i);

         if (EqualsIgnoreCase(pb->getName(), pszName)) {
            return pb;
         }
      }

      i++;
   }

   return NULL;
}

Bookmark *BookmarkFolder::removeBookmark(BookmarkPath::const_iterator i, BookmarkPath::const_iterator e, const Bookmark *pb) {
   BookmarkVector::iterator bi = m_elements.begin(), be = m_elements.end();

   if (i == e) {
      while (bi != be) {
         if ((*bi)->isBookmark()) {
            Bookmark *pbMatch = (Bookmark *) (*bi);

            if (EqualsIgnoreCase(pb->getName(), pbMatch->getName()) &&
                pb->getHref() == pbMatch->getHref()) {
               m_elements.erase(bi);
               return pbMatch;
            }
         }

         bi++;
      }
   }
   else {
      const BookmarkFolder *pbfMatch = (*i);

      while (bi != be) {
         if ((*bi)->isFolder()) {
            BookmarkFolder *pbfSub = (BookmarkFolder *) (*bi);

            if (EqualsIgnoreCase(pbfMatch->getName(), pbfSub->getName())) {
               Bookmark *pbResult = pbfSub->removeBookmark(i + 1, e, pb);

               if (pbResult != NULL) {
                  return pbResult;
               }
            }
         }

         bi++;
      }
   }

   return NULL;
}

BookmarkFolder *BookmarkFolder::findBookmarkFolder(LPCTSTR pszName) const {
   assert(isValid());
   assert(pszName != NULL);

   BookmarkVector::const_iterator i = m_elements.begin(), end = m_elements.end();
   // just a bookmark name, find bookmark with same title, href

   while (i != end) {
      if ((*i)->isFolder()) {
         BookmarkFolder *pb = (BookmarkFolder *) (*i);

         if (EqualsIgnoreCase(pb->getName(), pszName)) {
            return pb;
         }
      }

      i++;
   }

   return NULL;
}

BookmarkFolder *BookmarkFolder::removeFolder(BookmarkPath::const_iterator i, BookmarkPath::const_iterator e, const BookmarkFolder *pf) {
   BookmarkVector::iterator bi = m_elements.begin(), be = m_elements.end();

   if (i == e) {
      while (bi != be) {
         if ((*bi)->isFolder()) {
            BookmarkFolder *pfMatch = (BookmarkFolder *) (*bi);

            if (EqualsIgnoreCase(pf->getName(), pfMatch->getName())) {
               BookmarkVector::const_iterator ei = pfMatch->begin(), ee = pfMatch->end();
               bool empty = true;

               while (ei != ee && empty == true) {
                  if ((*ei)->isBookmark() || (*ei)->isFolder()) {
                     empty = false;
                  }

                  ei++;
               }

               if (empty) {
                  m_elements.erase(bi);
                  return pfMatch;
               }
            }
         }

         bi++;
      }
   }
   else {
      const BookmarkFolder *pbfMatch = (*i);

      while (bi != be) {
         if ((*bi)->isFolder()) {
            BookmarkFolder *pbfSub = (BookmarkFolder *) (*bi);

            if (EqualsIgnoreCase(pbfMatch->getName(), pbfSub->getName())) {
               BookmarkFolder *pfResult = pbfSub->removeFolder(i + 1, e, pf);

               if (pfResult != NULL) {
                  return pfResult;
               }
            }
         }

         bi++;
      }
   }

   return NULL;
}
