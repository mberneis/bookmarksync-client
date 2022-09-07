/*
 * BookmarkLib/BookmarkEditor.cxx
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

#include "BookmarkEditor.h"

#include "SyncLib/FileOutputStream.h"
#include "SyncLib/BufferedOutputStream.h"
#include "SyncLib/PrintWriter.h"
#include "SyncLib/DateTime.h"
#include "SyncLib/Util.h"
#include "SyncLib/Image.h"

using namespace syncit;

BookmarkFolder *FindFolder(BookmarkFolder *pbfRoot,
                           BookmarkPath::const_iterator si, BookmarkPath::const_iterator se) {
   if (si == se) {
      return pbfRoot;
   }
   else {
      const BookmarkFolder *pbfMatch = (*si);

      BookmarkVector::const_iterator bi = pbfRoot->begin(), be = pbfRoot->end();

      while (bi != be) {
         if ((*bi)->isFolder()) {
            BookmarkFolder *pbfSub = (BookmarkFolder *) (*bi);

            if (EqualsIgnoreCase(pbfMatch->getName(), pbfSub->getName())) {
               BookmarkFolder *pbfResult = FindFolder(pbfSub, si + 1, se);

               if (pbfResult != NULL) {
                  return pbfResult;
               }
            }
         }

         bi++;
      }

      return NULL;
   }
}

BookmarkMerger::BookmarkMerger(BookmarkModel *pbm) {

#ifndef NDEBUG
   strcpy(m_achStartTag, "BookmarkMerger");
   strcpy(m_achEndTag, "BookmarkMerger");
#endif /* NDEBUG */

   m_pbm = pbm;

   assert(isValid());
}

/* virtual */
BookmarkMerger::~BookmarkMerger() {
   assert(isValid());

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */
}

/* virtual */
BookmarkEditor::~BookmarkEditor() {
}

#ifndef NDEBUG
bool BookmarkMerger::isValid() const {
   return strcmp(m_achStartTag, "BookmarkMerger") == 0 &&
          strcmp(m_achEndTag, "BookmarkMerger") == 0 &&
          m_pbm->isValid();
}
#endif /* NDEBUG */

/* virtual */
void BookmarkMerger::pushFolder(const BookmarkFolder *pbf) {
   m_stack.push_back(pbf);
}

/* virtual */
void BookmarkMerger::popFolder() {
   m_stack.pop_back();
}

/**
 * Add the new folder...
 */
/* virtual */
void BookmarkMerger::addFolder(const BookmarkFolder *pfNew) {
   BookmarkFolder *pbf = FindFolder(m_pbm, m_stack.begin(), m_stack.end());

   assert(pbf != NULL);

   pbf->add(NEW BookmarkFolder(*pfNew));
}

/* virtual */
void BookmarkMerger::addBookmark(const Bookmark *pNew) {
   BookmarkFolder *pbf = FindFolder(m_pbm, m_stack.begin(), m_stack.end());

   assert(pbf != NULL);

   pbf->add((BookmarkObject *) pNew->attach());
}

/* virtual */
void BookmarkMerger::delBookmark(const Bookmark *pOld) {
}

/* virtual */
void BookmarkEditor::delBookmark(const Bookmark *pOld) {
   Bookmark *pb = m_pbm->removeBookmark(m_stack.begin(), m_stack.end(), pOld);

   if (pb != NULL) {
      if (pb->hasId()) { m_pbm->removeAliasId(pb->getId()); }

      BookmarkObject::Detach(pb);
   }
}

/* virtual */
void BookmarkEditor::del0(const BookmarkFolder *pOld) {
   BookmarkFolder *pbf = m_pbm->removeFolder(m_stack.begin(), m_stack.end(), pOld);

   if (pbf != NULL) {
      if (pbf->hasId()) { m_pbm->removeAliasId(pbf->getId()); }

      BookmarkObject::Detach(pbf);
   }
}
