/*
 * BookmarkLib/BookmarkModel.cxx
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
#pragma warning( disable : 4786 )

#include <algorithm>

#include "BookmarkModel.h"

using namespace syncit;

BookmarkObject::BookmarkObject() {
   m_ulRefCount = 1;
}

/* virtual */
BookmarkObject::~BookmarkObject() {
   assert(m_ulRefCount <= 1);
}

/* virtual */
bool BookmarkObject::isBookmark() const {
   return false;
}

/* virtual */
bool BookmarkObject::isFolder() const {
   return false;
}

/* virtual */
bool BookmarkObject::isAlias() const {
   return false;
}

/* virtual */
bool BookmarkObject::isSeparator() const {
   return false;
}

void BookmarkObject::Detach(BookmarkObject *p) {
   if (p != NULL && p->detach()) {
      delete p;
   }
}

/* virtual */
bool BookmarkObject::equals(const BookmarkObject *p) const {
   return false;
}

/* virtual */
bool BookmarkSeparator::isSeparator() const {
   return true;
}

BookmarkAlias::BookmarkAlias(const BookmarkAlias &rhs) {
   m_id = tstralloc(rhs.m_id);
}

BookmarkAlias::BookmarkAlias(const tchar_t *pszId) {
   m_id = tstralloc(pszId);
}

BookmarkAlias::~BookmarkAlias() {
   u_free0(m_id);
}

BookmarkAlias &BookmarkAlias::operator=(const BookmarkAlias &rhs) {
   m_id = tstrrealloc(m_id, rhs.m_id);
   return *this;
}

/* virtual */
bool BookmarkAlias::isAlias() const {
   return true;
}

BookmarkItem::BookmarkItem(const tchar_t *pszName) {
   m_pszName = tstralloc(pszName);
   m_pszId = NULL;
   m_pszDescription = NULL;
   m_addTime = 0L;
   m_images[OPEN_IMAGE] = Image::Blank.attach();
   m_images[CLOSED_IMAGE] = Image::Blank.attach();
}

BookmarkItem::BookmarkItem(const BookmarkItem &rhs) {
   m_pszName = tstralloc(rhs.m_pszName);
   m_pszId   = tstralloc(rhs.m_pszId);
   m_pszDescription = tstralloc(rhs.m_pszDescription);

   m_addTime = rhs.m_addTime;

   m_images[0] = rhs.m_images[0]->attach();
   m_images[1] = rhs.m_images[1]->attach();
}

/* virtual */
BookmarkItem::~BookmarkItem() {
   u_free0(m_pszName);
   u_free0(m_pszId);
   u_free0(m_pszDescription);

   Image::Detach(m_images[0]);
   Image::Detach(m_images[1]);
}

BookmarkItem &BookmarkItem::operator=(const BookmarkItem &rhs) {
   m_pszName = tstrrealloc(m_pszName, rhs.m_pszName);
   m_pszId = tstrrealloc(m_pszId, rhs.m_pszId);
   m_pszDescription = tstrrealloc(m_pszDescription, rhs.m_pszDescription);
   m_addTime = rhs.m_addTime;
   setImages((BookmarkItem::ImageType) 0, rhs.m_images[0]);
   setImages((BookmarkItem::ImageType) 1, rhs.m_images[1]);

   return *this;
}

/* virtual */
bool BookmarkSubscription::isSubscription() const {
   return true;
}

BookmarkModel::BookmarkModel() {
   m_pNewItemHeader = m_pMenuHeader = NULL;
}

BookmarkModel::BookmarkModel(const BookmarkModel &rhs) : BookmarkSubscription(rhs) {
   m_pNewItemHeader = m_pMenuHeader = NULL;
   rebuild(this);
}

BookmarkModel &BookmarkModel::operator=(const BookmarkModel &rhs) {
   BookmarkSubscription::operator=(rhs);

   m_pNewItemHeader = m_pMenuHeader = NULL;
   rebuild(this);
   return *this;
}

void BookmarkModel::rebuild(BookmarkFolder *pbf) {
   BookmarkVector::const_iterator i = pbf->begin(), end = pbf->end();

   while (i != end) {
      if ((*i)->isBookmark()) {
         Bookmark *pb = (Bookmark *) (*i);

         if (pb->hasId()) {
            defineAliasId(pb->getId(), pb);
         }
      }
      else if ((*i)->isFolder()) {
         BookmarkFolder *pf = (BookmarkFolder *) (*i);

         if (pf->hasId()) {
            defineAliasId(pf->getId(), pf);
         }

         rebuild(pf);
      }

      i++;
   }
}

int syncit::CompareBookmarkFolders(const void *p1, const void *p2) {
   const BookmarkFolder **pd1 = (const BookmarkFolder **) p1;
   const BookmarkFolder **pd2 = (const BookmarkFolder **) p2;

   return tstricmp((*pd1)->m_pszName, (*pd2)->m_pszName);
}

/* virtual */
void BookmarkDifferences::addFolder(const BookmarkFolder *pNew) {
   add0(pNew);

   pushFolder(pNew);

   BookmarkVector::const_iterator i = pNew->begin(), end = pNew->end();

   while (i != end) {
      if ((*i)->isBookmark()) {
         addBookmark((const Bookmark *) (*i));
      }
      else if ((*i)->isFolder()) {
         addFolder((const BookmarkFolder *) (*i));
      }

      i++;
   }

   popFolder();
}

/* virtual */
void BookmarkDifferences::add0(const BookmarkFolder *pOld) {
   pOld;    // unreferenced
}

/* virtual */
void BookmarkDifferences::delFolder(const BookmarkFolder *pOld) {
   pushFolder(pOld);

   BookmarkVector::const_iterator i = pOld->begin(), end = pOld->end();

   while (i != end) {
      if ((*i)->isBookmark()) {
         delBookmark((const Bookmark *) (*i));
      }
      else if ((*i)->isFolder()) {
         delFolder((const BookmarkFolder *) (*i));
      }

      i++;
   }

   popFolder();

   del0(pOld);
}

/* virtual */
void BookmarkDifferences::del0(const BookmarkFolder *pOld) {
   pOld;    // unreferenced
}

void syncit::ExtractFromFolder(const BookmarkFolder *pbf,
                               vector<const Bookmark *> &vb,
                               vector<const BookmarkFolder *> &vf) {
   BookmarkVector::const_iterator i = pbf->begin(), end = pbf->end();

   while (i != end) {
      const BookmarkObject *pbn = *i++;

      if (pbn->isBookmark()) {
         const Bookmark *p = (const Bookmark *) pbn;
         if (p->getName()[0] != '.') {
            vb.push_back(p);
         }
      }
      else if (pbn->isFolder()) {
         const BookmarkFolder *p = (const BookmarkFolder *) pbn;

         if (p->getName()[0] != '.') {
            vf.push_back((const BookmarkFolder *) pbn);
         }
      }
   }
}

/**
 * @param vb1  a sorted vector of Bookmarks (the *new* bookmarks)
 * @param vb2  a sorted vector of Bookmarks (the *old* bookmarks)
 * @param pdiff  a listener object for edit commands necessary to bring vb2 similar to vb1
 */
static
int diffBookmarks(const vector<const Bookmark *> &vb1,
                  const vector<const Bookmark *> &vb2,
                  BookmarkDifferences *pdiff) {
   int r = 0;
   vector<const Bookmark *>::const_iterator ib1 = vb1.begin(), endb1 = vb1.end(),
                                            ib2 = vb2.begin(), endb2 = vb2.end();

   while (ib1 != endb1 && ib2 != endb2) {
      int c = BookmarkCompare(*ib1, *ib2);

      if (c == 0) {
         ib1++;
         ib2++;
      }
      else if (c < 0) {
         pdiff->addBookmark(*ib1++);
         r++;
      }
      else {
         pdiff->delBookmark(*ib2++);
         r++;
      }
   }

   while (ib1 != endb1) {
      pdiff->addBookmark(*ib1++);
      r++;
   }

   while (ib2 != endb2) {
      pdiff->delBookmark(*ib2++);
      r++;
   }

   return r;
}

static
int diffFolders(const vector<const BookmarkFolder *> &vf1,
                const vector<const BookmarkFolder *> &vf2,
                BookmarkDifferences *pdiff) {
   int r = 0;
   vector<const BookmarkFolder *>::const_iterator if1 = vf1.begin(), endf1 = vf1.end(),
                                                  if2 = vf2.begin(), endf2 = vf2.end();

   while (if1 != endf1 && if2 != endf2) {
      const BookmarkFolder *pf1 = *if1;
      const BookmarkFolder *pf2 = *if2;
      int c = tstricmp(pf1->getName(), pf2->getName());

      if (c == 0) {
         vector<const Bookmark *> b1, b2;
         vector<const BookmarkFolder *> f1, f2;

         do {
            ExtractFromFolder(*if1, b1, f1);

            if1++;
         } while (if1 != endf1 && EqualsIgnoreCase((*if1)->getName(), pf1->getName()));

         // if1 == endf1 || if1->title != pf1->title

         do {
            ExtractFromFolder(*if2, b2, f2);

            if2++;
         } while (if2 != endf2 && EqualsIgnoreCase((*if2)->getName(), pf2->getName()));

         // diff Bookmarks
         //
         sort(b1.begin(), b1.end(), BookmarkLess);
         sort(b2.begin(), b2.end(), BookmarkLess);

         pdiff->pushFolder(pf1);

         r += diffBookmarks(b1, b2, pdiff);

         // diff Folders
         //
         sort(f1.begin(), f1.end(), BookmarkFolderLess);
         sort(f2.begin(), f2.end(), BookmarkFolderLess);

         r += diffFolders(f1, f2, pdiff);
         pdiff->popFolder();
      }
      else if (c < 0) {
         pdiff->addFolder(pf1);
         if1++;
         r++;
      }
      else {
         pdiff->delFolder(pf2);
         if2++;
         r++;
      }
   }

   while (if1 != endf1) {
      pdiff->addFolder(*if1++);
      r++;
   }

   while (if2 != endf2) {
      pdiff->delFolder(*if2++);
      r++;
   }

   return r;
}

int syncit::diff(const BookmarkFolder *pbf1,
                 const BookmarkFolder *pbf2,
                 BookmarkDifferences *pdiff) {

   vector<const Bookmark *> b1, b2;
   vector<const BookmarkFolder *> f1, f2;

   ExtractFromFolder(pbf1, b1, f1);
   ExtractFromFolder(pbf2, b2, f2);

   // diff Bookmarks
   //
   sort(b1.begin(), b1.end(), BookmarkLess);
   sort(b2.begin(), b2.end(), BookmarkLess);

   int r = diffBookmarks(b1, b2, pdiff);

   // diff Folders
   //
   sort(f1.begin(), f1.end(), BookmarkFolderLess);
   sort(f2.begin(), f2.end(), BookmarkFolderLess);

   r += diffFolders(f1, f2, pdiff);

   //pdiff->commit();

   return r;
}
