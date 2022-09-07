/*
 * BookmarkLib/BookmarkEditor.h
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
#ifndef BookmarkEditor_H
#define BookmarkEditor_H

#include "BookmarkModel.h"

using namespace syncit;

class BookmarkMerger : public BookmarkDifferences {

#ifndef NDEBUG
public:
   bool isValid() const;
private:
   char m_achStartTag[sizeof("BookmarkMerger")];
#endif /* NDEBUG */

public:
   BookmarkMerger(BookmarkModel *pbm);

   virtual ~BookmarkMerger();

   /**
    * Add the <i>pNew</i> bookmark to the old bookmark folder
    * to make it equal to the new bookmark folder
    */
   virtual void addBookmark(const Bookmark *pbNew);

   /**
    * Add the new folder...
    */
   virtual void addFolder(const BookmarkFolder *pfNew);

   /**
    * Remove the <i>pOld</i> bookmark from the old bookmark folder
    * to make it equal to the new bookmark folder
    */
   virtual void delBookmark(const Bookmark *pOld);

   virtual void pushFolder(const BookmarkFolder *pbf);
   virtual void popFolder();

protected:
   BookmarkFolder *getSubFolder();

   BookmarkModel *m_pbm;

   BookmarkPath m_stack;

private:
   // disable copy constructor and assignment
   //
   BookmarkMerger(BookmarkMerger &rhs);
   BookmarkMerger &operator=(BookmarkMerger &rhs);

#ifndef NDEBUG
private:
   char m_achEndTag[sizeof("BookmarkMerger")];
#endif /* NDEBUG */

};

class BookmarkEditor : public BookmarkMerger {

public:
   BookmarkEditor(BookmarkModel *pbm) : BookmarkMerger(pbm) {
   }

   virtual ~BookmarkEditor();

   virtual void delBookmark(const Bookmark *pOld);
   virtual void del0(const BookmarkFolder *pOld);

private:
   // disable copy constructor and assignment
   //
   BookmarkEditor(BookmarkEditor &rhs);
   BookmarkEditor &operator=(BookmarkEditor &rhs);

};

#endif /* BookmarkEditor_H */
