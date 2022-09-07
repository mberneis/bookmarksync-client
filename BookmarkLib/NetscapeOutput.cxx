/*
 * BookmarkLib/NetscapeOutput.cxx
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
#include "NetscapeBookmarks.h"

#include "SyncLib/BufferedOutputStream.h"
#include "SyncLib/FileOutputStream.h"
#include "SyncLib/XML.h"

using namespace syncit;

static void PrintFolder(PrintWriter &w, const BookmarkModel *p, const BookmarkFolder *pbf, int tab);
static void PrintBookmark(PrintWriter &w, const Bookmark *pb, int tab, bool fAlias);
static void PrintDate(PrintWriter &w, const char *pszAttribute, const DateTime &dt);
static void WriteHtml(PrintWriter &w, const tchar_t *psz);
static void WriteHref(PrintWriter &w, const Href &href);
static void stab(PrintWriter &w, int tab);

void NetscapeBookmarks::Write(const BookmarkModel *p, LPCTSTR pszFilename) {
   FileOutputStream f;
   BufferedOutputStream b(&f);
   PrintWriter w(&b);

   f.create(pszFilename);
   w.print(T("<!DOCTYPE NETSCAPE-Bookmark-file-1>\r\n")
           T("<!-- This is an automatically generated file.  It will be read and overwritten.  Do Not Edit! -->\r\n")
           T("<TITLE>"));
   if (p->hasName()) {
      w.print(p->getName());
   }
   w.print(T("</TITLE>\r\n"));
   PrintFolder(w, p, p, 0);
   w.close();
   f.commit();
}

void NetscapeBookmarks::Write(const BookmarkModel *p, PrintWriter &w) {
   PrintFolder(w, p, p, 0);
}

static void PrintFolder(PrintWriter &w, const BookmarkModel *p, const BookmarkFolder *pbf, int tab) {
   stab(w, tab);

   if (tab == 0) {
      w.print(T("<H1>"));
      if (pbf->hasName()) {
         w.print(pbf->getName());
      }
      w.print(T("</H1>\r\n"));
   }
   else {
      w.print(T("<DT><H3"));

      if (p->isMenuHeader(pbf)) {
         w.print(T(" MENUHEADER"));
      }
      if (p->isNewItemHeader(pbf)) {
         w.print(T(" NEWITEMHEADER"));
      }
      if (pbf->isFolded()) {
         w.print(T(" FOLDED"));
      }

      PrintDate(w, "ADD_DATE", pbf->getAdded());
      w.print(T(">"));
      WriteHtml(w, pbf->getName());
      w.print(T("</H3>\r\n"));
   }

   if (pbf->hasDescription()) {
      w.print(T("<DD>"));
      WriteHtml(w, pbf->getDescription());
      w.print(T("\r\n"));
   }

   stab(w, tab);

   w.print(T("<DL><p>\r\n"));

   BookmarkVector::const_iterator i = pbf->begin(), max = pbf->end();

   while (i != max) {
      const BookmarkObject *pb = (*i);

      if (pb->isBookmark()) {
         PrintBookmark(w, (const Bookmark *) pb, tab + 1, false);
      }
      else if (pb->isFolder()) {
         PrintFolder(w, p, (const BookmarkFolder *) pb, tab + 1);
      }
      else if (pb->isSeparator()) {
         stab(w, tab + 1);
         w.print(T("<HR>\r\n"));
      }
      else if (pb->isAlias()) {
         const BookmarkAlias *pa = (const BookmarkAlias *) pb;
         const BookmarkItem  *pbmk  = p->findId(pa->getId());

         if (pbmk == NULL) {
         }
         else if (pbmk->isBookmark()) {
            PrintBookmark(w, (const Bookmark *) pbmk, tab + 1, true);
         }
      }

      i++;
   }

   stab(w, tab);

   w.print(T("</DL><p>\r\n"));
}

static void PrintBookmark(PrintWriter &w, const Bookmark *pb, int tab, bool fAlias) {
   stab(w, tab);

   w.print(T("<DT><A"));

   WriteHref(w, pb->getHref());

   if (pb->hasId()) {
      XMLWriteAttribute(w, fAlias ? T("ALIASOF") : T("ALIASID"), pb->getId());
   }

   PrintDate(w, "ADD_DATE", pb->getAdded());
   PrintDate(w, "LAST_VISIT", pb->getVisited());
   PrintDate(w, "LAST_MODIFIED", pb->getModified());
   w.print(T(">"));

   if (pb->hasName()) {
      WriteHtml(w, pb->getName());
   }

   w.print(T("</A>\r\n"));

   if (pb->hasDescription()) {
      w.print(T("<DD>"));
      WriteHtml(w, pb->getDescription());
      w.print(T("\r\n"));
   }
}

static void WriteHref(PrintWriter &w, const Href &href) {
   char ach[4096], *p = ach;
   char *e = p + href.format(ach, sizeof(ach));

   w.print(T(" HREF=\""));

   while (p != e) {
      char ch = *p++;

      if (ch == '"') {
         w.print(T("%22"));
      }
      else {
         w.write(ch);
      }
   }

   w.write('"');
}

static void PrintDate(PrintWriter &w, const char *pszAttribute, const DateTime &dt) {
   if (dt.isValid()) {
      w.write(' ');
      w.print(pszAttribute);
      w.print("=\"");
      w.print((unsigned long) dt.get_time_t());
      w.write('"');
   }
}

static void WriteHtml(PrintWriter &w, const tchar_t *psz) {
   tchar_t ch = *psz++;

   while (ch != 0) {
      switch (ch) {
         case '<':
            w.print(T("&lt;"));
            break;

         case '&':
            w.print(T("&amp;"));
            break;

         case '\n':
            w.print(T("<BR>\r"));
            // fall through

         default:
            w.write(ch);
            break;
      }

      ch = *psz++;
   }
}

static void stab(PrintWriter &w, int tab) {
   for (int i = 0; i < tab; i++) {
      w.print(T("    "));
   }
}
