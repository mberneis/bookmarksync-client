/*
 * BookmarkLib/XBELOutput.cxx
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
 *    Reads/write bookmark sets using the XBEL (XML Bookmark Exchange Language)
 *    file format.
 */
#pragma warning( disable : 4786 )

#include "BrowserBookmarks.h"

#include "SyncLib/BufferedOutputStream.h"
#include "SyncLib/FileOutputStream.h"

#include "SyncLib/XML.h"
#include "SyncLib/BitmapFileImage.h"

using namespace syncit;

static void PrintFolder(PrintWriter &w, const BookmarkModel *p, const BookmarkFolder *pbf, int tab);
static void PrintBookmark(PrintWriter &w, const Bookmark *pb, int tab);
static void PrintItemAttributes(PrintWriter &w, const BookmarkItem *pbi);
static void PrintItemElements(PrintWriter &w, const BookmarkItem *pbi, int tab);
static void PrintFolderElements(PrintWriter &w, const BookmarkModel *p, const BookmarkVector &v, int tab);
static void stab(PrintWriter &w, int tab);

/* virtual */
void XBELBookmarks::Write(const BookmarkModel *p, LPCTSTR pszFilename) {
   FileOutputStream f;
   BufferedOutputStream b(&f);
   PrintWriter w(&b);

   f.create(pszFilename);
   Write(p, w);
   w.close();
   f.commit();
}

void XBELBookmarks::Write(const BookmarkModel *p, PrintWriter &w) {
   w.print(T("<?xml version=\"1.0\" encoding=\"cp"));
   w.print((unsigned long) GetACP());
   w.print(T("\" ?>\r\n")
           T("<!DOCTYPE XBEL PUBLIC\r\n")
           T("     \"+//IDN python.org//DTD XML Bookmark Exchange Language 1.0//EN//XML\"\r\n")
           T("     \"http://www.python.org/topics/xml/dtds/xbel-1.0.dtd\">\r\n")
           T("<!-- This is an automatically generated file.\r\n")
           T("     It will be read and overwritten.\r\n")
           T("     Do Not Edit! -->\r\n")
           T("<xbel version=\"1.0\" seqno=\""));

   w.print(p->getSeqNo());
   w.write(T('"'));

   PrintFolder(w, p, p, 1);

   w.print(T("</xbel>\r\n"));
}

static void PrintFolder(PrintWriter &w, const BookmarkModel *p, const BookmarkFolder *pbf, int tab) {
   if (p != NULL && p->isMenuHeader(pbf)) {
      w.print(T(" menuheader"));
   }
   if (p != NULL && p->isNewItemHeader(pbf)) {
      w.print(T(" newitemheader"));
   }

   w.print(T(" folded=\""));
   w.print(pbf->isFolded() ? T("yes") : T("no"));
   w.write(T('"'));

   PrintItemAttributes(w, pbf);

   w.print(T(">\r\n"));

   PrintItemElements(w, pbf, tab);
   PrintFolderElements(w, p, pbf->elements(), tab);
}

static void PrintFolderElements(PrintWriter &w, const BookmarkModel *p, const BookmarkVector &v, int tab) {
   BookmarkVector::const_iterator i = v.begin(), max = v.end();

   while (i != max) {
      const BookmarkObject *pb = (*i);

      if (pb->isBookmark()) {
         PrintBookmark(w, (const Bookmark *) pb, tab);
      }
      else if (pb->isFolder()) {
         const BookmarkFolder *pbf = (const BookmarkFolder *) pb;

         stab(w, tab);
         if (pbf->isSubscription()) {
            const BookmarkSubscription *pbs = (const BookmarkSubscription *) pbf;

            w.print(T("<subscription seqno=\""));
            w.print(pbs->getSeqNo());
            w.write(T('"'));
         }
         else {
            w.print(T("<folder"));
         }

         PrintFolder(w, p, pbf, tab + 1);
         stab(w, tab);

         if (pbf->isSubscription()) {
            w.print(T("</subscription>\r\n"));
         }
         else {
            w.print(T("</folder>\r\n"));
         }
      }
      else if (pb->isSeparator()) {
         stab(w, tab);
         w.print(T("<separator/>\r\n"));
      }
      else if (pb->isAlias()) {
         const BookmarkAlias *pa = (const BookmarkAlias *) pb;

         stab(w, tab);
         w.print(T("<alias"));
         XMLWriteAttribute(w, T("ref"), pa->getId());
         w.print(T("/>\r\n"));
      }

      i++;
   }
}

static void PrintBookmark(PrintWriter &w, const Bookmark *pb, int tab) {
   char achBuffer[4096];
   size_t cch = pb->getHref().format(achBuffer, sizeof(achBuffer));

   stab(w, tab);

   w.print(T("<bookmark"));

   XMLWriteAttribute(w, T("href"), achBuffer);

   PrintItemAttributes(w, pb);

   if (pb->getVisited().formatW3C(achBuffer, ELEMENTS(achBuffer))) { 
      w.print(T(" visited=\""));
      w.print(achBuffer);
      w.write(T('"'));
   }

   if (pb->getModified().formatW3C(achBuffer, ELEMENTS(achBuffer))) { 
      w.print(T(" modified=\""));
      w.print(achBuffer);
      w.write(T('"'));
   }

   w.print(T(">\r\n"));

   PrintItemElements(w, pb, tab + 1);

   stab(w, tab);
   w.print(T("</bookmark>\r\n"));
}

static void PrintItemAttributes(PrintWriter &w, const BookmarkItem *pbi) {
   tchar_t achDateTime[sizeof("YYYY-MM-DDThh:mm:ss.nnnnnnnZ")];

   if (pbi->hasId()) {
      XMLWriteAttribute(w, T("id"), pbi->getId());
   }

   if (pbi->getAdded().formatW3C(achDateTime, ELEMENTS(achDateTime))) {
      w.print(T(" added=\""));
      w.print(achDateTime);
      w.write(T('"'));
   }

   const char *pszUrl;

   if ((pszUrl = pbi->getImages(BookmarkItem::OPEN_IMAGE)->getUrl()) != NULL) {
      w.print(T(" openimg=\""));
      w.print(pszUrl);
      w.write(T('"'));
   }

   if ((pszUrl = pbi->getImages(BookmarkItem::CLOSED_IMAGE)->getUrl()) != NULL) {
      w.print(T(" closedimg=\""));
      w.print(pszUrl);
      w.write(T('"'));
   }
}

static void PrintItemElements(PrintWriter &w, const BookmarkItem *pbi, int tab) {
   if (pbi->hasName()) {
      stab(w, tab);
      w.print(T("<title>"));
      XMLWriteContents(w, pbi->getName());
      w.print(T("</title>\r\n"));
   }

   if (pbi->hasDescription()) {
      stab(w, tab);
      w.print(T("<desc>"));
      XMLWriteContents(w, pbi->getDescription());
      w.print(T("</desc>\r\n"));
   }
}

static void stab(PrintWriter &w, int tab) {
   for (int i = 0; i < tab; i++) {
      w.print(T("  "));
   }
}
