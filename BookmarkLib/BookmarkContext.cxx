/*
 * BookmarkLib/BookmarkContext.cxx
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
 * Last Modification: 30 Apr 1999
 *
 * Description:
 *    A BookmarkContext object is the only thing allowed
 *    to edit Bookmark model objects (Bookmark, BookmarkFolder, etc)
 *    See the declarations in BookmarkContext.h
 *
 *    A BookmarkContext object keeps track of the following information:
 *       the current item --  the last thing created, so it can be edited
 *                            by the BookmarkContext
 *       the current folder - where newly created bookmark things get added
 *       the current model -- the root of the bookmark tree
 *
 *    A BookmarkContext also has a default set of images that get
 *    associated with bookmark items.
 */
#pragma warning( disable : 4786 )

#include <algorithm>

#include "BookmarkModel.h"

#include "SyncLib/util.h"

using namespace syncit;

/**
 * Construct a new BookmarkContext that will edit the specified BookmarkModel.
 * The context's current item and current folder will both be set to the
 * root of the bookmark model.  The blank image (0x0 icon) will be used as the
 * default image icon for bookmarks and folders.
 *
 * @param p  a BookmarkModel to edit.
 */
BookmarkContext::BookmarkContext(BookmarkModel *p, ImageLoader *pLoader) {
   m_pModel = p;
   m_pLoader = pLoader;

   m_pCurrentItem = p;
   m_pCurrentFolder = NULL;

   m_apFolderImages[0] = Image::Blank.attach();
   m_apFolderImages[1] = Image::Blank.attach();
   m_apBookmarkImages[0] = Image::Blank.attach();
   m_apBookmarkImages[1] = Image::Blank.attach();
   m_apSubscriptionImages[0] = Image::Blank.attach();
   m_apSubscriptionImages[1] = Image::Blank.attach();
}

/**
 * Destroy a BookmarkContext
 */
BookmarkContext::~BookmarkContext() {
   Image::Detach(m_apFolderImages[0]);
   Image::Detach(m_apFolderImages[1]);
   Image::Detach(m_apBookmarkImages[0]);
   Image::Detach(m_apBookmarkImages[1]);
   Image::Detach(m_apSubscriptionImages[0]);
   Image::Detach(m_apSubscriptionImages[1]);

   if (m_pCurrentItem != m_pModel) {
      BookmarkObject::Detach(m_pCurrentItem);
   }
}

/**
 * Create a new, blank bookmark and add it to the current folder
 */
void BookmarkContext::startBookmark() {
   Bookmark *p = NEW Bookmark();

   p->setImages((BookmarkItem::ImageType) 0, m_apBookmarkImages[0]);
   p->setImages((BookmarkItem::ImageType) 1, m_apBookmarkImages[1]);

   m_pCurrentItem = p;
}

void BookmarkContext::endBookmark() {
   assert(m_pCurrentItem->isBookmark());
   m_pCurrentFolder->add(m_pCurrentItem);
   m_pCurrentItem = NULL;
}

/**
 * Assumes the current item is a bookmark, and sets its visited date/time property.
 */
void BookmarkContext::setBookmarkVisited(const DateTime &dt) {
   assert(m_pCurrentItem->isBookmark());

   Bookmark *pb = (Bookmark *) m_pCurrentItem;

   assert(pb != NULL);

   pb->setVisited(dt);
}

/**
 * Assumes the current item is a bookmark, and sets its modified date/time property.
 */
void BookmarkContext::setBookmarkModified(const DateTime &dt) {
   assert(m_pCurrentItem->isBookmark());

   Bookmark *pb = (Bookmark *) m_pCurrentItem;

   assert(pb != NULL);

   pb->setModified(dt);
}

/**
 * Assumes the current item is a bookmark, and sets its href link property.
 */
void BookmarkContext::setBookmarkHref(const char *s) {
   assert(m_pCurrentItem->isBookmark());

   Bookmark *pb = (Bookmark *) m_pCurrentItem;

   assert(pb != NULL);

   pb->setHref(s);
}

/**
 * Create a new, blank folder and add it to the current folder
 */
void BookmarkContext::startFolder() {
   BookmarkFolder *p = NEW BookmarkFolder();

   p->setImages((BookmarkItem::ImageType) 0, m_apFolderImages[0]);
   p->setImages((BookmarkItem::ImageType) 1, m_apFolderImages[1]);

   m_pCurrentItem = p;
}

void BookmarkContext::startSubscription() {
   BookmarkSubscription *p = NEW BookmarkSubscription();

   p->setImages((BookmarkItem::ImageType) 0, m_apSubscriptionImages[0]);
   p->setImages((BookmarkItem::ImageType) 1, m_apSubscriptionImages[1]);

   m_pCurrentItem = p;
}

void BookmarkContext::endFolder() {
   assert(m_pCurrentItem->isFolder());
   m_pCurrentFolder->add(m_pCurrentItem);
   m_pCurrentItem = NULL;
}

void BookmarkContext::endSubscription() {
   assert(m_pCurrentItem->isFolder());
   m_pCurrentFolder->add(m_pCurrentItem);
   m_pCurrentItem = NULL;
}

void BookmarkContext::undoCurrent() {
   BookmarkObject::Detach(m_pCurrentItem);

   m_pCurrentItem = NULL;
}

/**
 * Netscape has a newitemheader attribute for bookmark folders.  Only
 * one folder can be the current newitemheader.
 */
void BookmarkContext::setNewItemHeader() {
   assert(m_pCurrentItem->isFolder());

   BookmarkFolder *pbf = (BookmarkFolder *) m_pCurrentItem;

   assert(pbf != NULL);

   m_pModel->m_pNewItemHeader = pbf;
}

/**
 * Netscape has a menuheader attribute for bookmark folders.  Only
 * one folder can be the current menuheader.
 */
void BookmarkContext::setMenuHeader() {
   assert(m_pCurrentItem->isFolder());

   BookmarkFolder *pbf = (BookmarkFolder *) m_pCurrentItem;

   assert(pbf != NULL);

   m_pModel->m_pMenuHeader = pbf;
}

/**
 * Assumes the current item is a folder, and sets its folded (open/closed) property.
 */
void BookmarkContext::setFolderFolded(bool f) {
   assert(m_pCurrentItem->isFolder());

   BookmarkFolder *pbf = (BookmarkFolder *) m_pCurrentItem;

   assert(pbf != NULL);

   pbf->setFolded(f);
}

/**
 * Assumes the current item is a folder, and sets it as the current folder.
 */
void BookmarkContext::pushFolder() {
   assert(m_pCurrentItem->isFolder());

   BookmarkFolder *pf = (BookmarkFolder *) m_pCurrentItem;

   m_stack.push(m_pCurrentFolder);
   m_pCurrentFolder = pf;
}

/**
 * The new current bookmark folder is the old one's parent.
 */
void BookmarkContext::popFolder() {
   m_pCurrentItem = m_pCurrentFolder;
   m_pCurrentFolder = m_stack.top();
   m_stack.pop();
}

/**
 * Creates a new separator and adds it to the current folder
 */
void BookmarkContext::newSeparator() {
   m_pCurrentFolder->add(NEW BookmarkSeparator());
}

/**
 * Creates a new bookmark alias and adds it to the current folder
 */
void BookmarkContext::newAlias(const tchar_t *pszId) {
   m_pCurrentFolder->add(NEW BookmarkAlias(pszId));
}

/**
 * Assumes the current item is a bookmark or a folder, and sets its added date/time property.
 */
void BookmarkContext::setAdded(const DateTime &dt) {
   assert(m_pCurrentItem->isBookmark() || m_pCurrentItem->isFolder());

   BookmarkItem *pb = (BookmarkItem *) m_pCurrentItem;

   assert(pb != NULL);

   pb->setAdded(dt);
}

void BookmarkContext::setSubscriptionSeqNo(long l) {
   assert(m_pCurrentItem->isFolder());

   BookmarkFolder *pbf = (BookmarkFolder *) m_pCurrentItem;

   assert(pbf->isSubscription());

   BookmarkSubscription *pbs = (BookmarkSubscription *) pbf;

   pbs->setSeqNo(l);
}

/**
 * Assumes the current item is a bookmark or a folder, and sets its title/name.
 */
void BookmarkContext::setName(const tchar_t *s) {
   assert(m_pCurrentItem->isFolder() || m_pCurrentItem->isBookmark());

   BookmarkItem *pb = (BookmarkItem *) m_pCurrentItem;

   assert(pb != NULL);

   pb->setName(s);
}

void BookmarkContext::setId(const tchar_t *psz) {
   assert(m_pCurrentItem->isFolder() || m_pCurrentItem->isBookmark());

   BookmarkItem *pb = (BookmarkItem *) m_pCurrentItem;

   assert(pb != NULL);

   pb->setId(psz);
   m_pModel->defineAliasId(psz, pb);
}

void BookmarkContext::setDescription(const tchar_t *psz) {
   assert(m_pCurrentItem->isFolder() || m_pCurrentItem->isBookmark());

   BookmarkItem *pb = (BookmarkItem *) m_pCurrentItem;

   assert(pb != NULL);

   pb->setDescription(psz);
}

void BookmarkContext::setImages(BookmarkItem::ImageType f, Image *p) {
   assert(m_pCurrentItem->isFolder() || m_pCurrentItem->isBookmark());

   BookmarkItem *pb = (BookmarkItem *) m_pCurrentItem;

   assert(pb != NULL);

   pb->setImages(f, p);
}

void BookmarkContext::setImages(BookmarkItem::ImageType f, const char *pszUrl) {
   Image *p = m_pLoader->load(pszUrl);

   if (p != NULL) {
      setImages(f, p);
      Image::Detach(p);
   }
}

void BookmarkContext::setDefaultFolderImages(BookmarkItem::ImageType f, Image *p) {
   Image::Detach(m_apFolderImages[f]);
   m_apFolderImages[f] = p->attach();
}

void BookmarkContext::setDefaultBookmarkImages(BookmarkItem::ImageType f, Image *p) {
   Image::Detach(m_apBookmarkImages[f]);
   m_apBookmarkImages[f] = p->attach();
}

void BookmarkContext::setDefaultSubscriptionImages(BookmarkItem::ImageType f, Image *p) {
   Image::Detach(m_apSubscriptionImages[f]);
   m_apSubscriptionImages[f] = p->attach();
}

/**
 * @param pbf  pointer to Bookmark
 * @param pszPathName pointer to hierarchical name of bookmark
 * @param 
 */
void BookmarkContext::startBookmark(const tchar_t *pszPathName,
                                    tchar_t chEscape, const tchar_t *pachMap, size_t cchMap, const char *pszUrl) {
   tchar_t achElement[MAX_PATH];

   const tchar_t *pszNext = startFolder(pszPathName, chEscape, pachMap, cchMap);

   BookmarkFolder *pbf = (BookmarkFolder *) m_pCurrentItem;

   Decode(pszNext, tstrlen(pszNext),
          chEscape, pachMap, cchMap,
          achElement, ELEMENTS(achElement));

   pushFolder();
   startBookmark();
   setName(achElement);
   setBookmarkHref(pszUrl);
}

void BookmarkContext::endBookmark(const tchar_t *pszPath) {
   endBookmark();
   popFolder();
   endFolder(pszPath);
}

const tchar_t *BookmarkContext::startFolder(const tchar_t *pszPath,
                                            tchar_t chEscape, const tchar_t *pachMap, size_t cchMap) {

   tchar_t achElement[MAX_PATH];

   BookmarkFolder *pbf = m_pCurrentFolder;

   // invariant: first character of pszName is the delimiter...
   tchar_t delim = *pszPath++;

   tchar_t *pszRest = tstrchr(pszPath, delim);

   while (pszRest != NULL) {
      assert(pszRest[0] == delim);

      Decode(pszPath, pszRest - pszPath,
             chEscape, pachMap, cchMap,
             achElement, ELEMENTS(achElement));

      // find or create sub-bookmark
      BookmarkFolder *pbfNew = pbf->findBookmarkFolder(achElement);

      if (pbfNew == NULL) {
         pbfNew = NEW BookmarkFolder();
         pbfNew->setName(achElement);

         pbfNew->setImages((BookmarkItem::ImageType) 0, m_apFolderImages[0]);
         pbfNew->setImages((BookmarkItem::ImageType) 1, m_apFolderImages[1]);

         pbf->add(pbfNew);
      }

      pbf = pbfNew;

      pszPath = pszRest + 1;
      pszRest = tstrchr(pszPath, delim);
   }

   m_pCurrentItem = pbf;

   return pszPath;
}

void BookmarkContext::endFolder(const tchar_t *pszPath) {
   assert(m_pCurrentItem->isFolder());
   m_pCurrentItem = NULL;
}
