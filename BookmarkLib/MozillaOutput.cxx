/*
* BookmarkLib/MozillaBookmarks.cxx
* Copyright (C) 2003  SyncIT.com, Inc.
* Copyright (c) 2003, Daniel Gehriger <gehriger@linkcad.com>
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
* Description:         BookmarkSync client software for Windows
* Author:              Terence Way
*                      Daniel Gehriger
* Created:             21 October 2003
* Last Modification:   21 October 2003
* E-mail:              mailto:tway@syncit.com
* Web site:            http://www.syncit.com
*/
#include "MozillaBookmarks.h"

#include "SyncLib/BufferedOutputStream.h"
#include "SyncLib/FileOutputStream.h"
#include "SyncLib/XML.h"
#include "SyncLib/UTF8.h"

namespace syncit {

//------------------------------------------------------------------------------
void MozillaBookmarks::Write(const BookmarkModel *p, LPCTSTR pszFilename) 
{
    FileOutputStream f;
    BufferedOutputStream b(&f);
    PrintWriter w(&b);

    f.create(pszFilename);
    w.print(T("<!DOCTYPE NETSCAPE-Bookmark-file-1>\r\n")
            T("<!-- This is an automatically generated file.\r\n")
            T("     It will be read and overwritten.\r\n")
            T("     DO NOT EDIT! -->\r\n")
            T("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\r\n")
            T("<TITLE>"));
    
    if (p->hasName()) 
    {
        w.print(p->getName());
    }

    w.print(T("</TITLE>\r\n"));
    PrintFolder(w, p, p, 0);
    w.close();
    f.commit();
}

//------------------------------------------------------------------------------
void MozillaBookmarks::Write(const BookmarkModel *p, PrintWriter &w) 
{
   PrintFolder(w, p, p, 0);
}

//------------------------------------------------------------------------------
void MozillaBookmarks::PrintFolder(PrintWriter &w, const BookmarkModel *p, const BookmarkFolder *pbf, int tab) 
{
    stab(w, tab);

    if (tab == 0) 
    {
        w.print(T("<H1>"));
        if (pbf->hasName()) 
        {
            w.print(pbf->getName());
        }
        w.print(T("</H1>\r\n\r\n"));
    }
    else 
    {
        w.print(T("<DT><H3"));

        if (pbf->hasId()) 
        {
            if (tstrcmp(pbf->getId(), "NC:PersonalToolbarFolder") == 0)
            {
                XMLWriteAttribute(w, T("PERSONAL_TOOLBAR_FOLDER"), T("true"));
            }

            XMLWriteAttribute(w, T("ID"), pbf->getId());
        }

        w.print(T(">"));
        WriteHtml(w, pbf->getName());
        w.print(T("</H3>\r\n"));
    }

    if (pbf->hasDescription())
    {
        w.print(T("<DD>"));
        WriteHtml(w, pbf->getDescription());
        w.print(T("\r\n"));
    }

    stab(w, tab);

    w.print(T("<DL><p>\r\n"));

    BookmarkVector::const_iterator i = pbf->begin(), max = pbf->end();

    while (i != max) 
    {
        const BookmarkObject *pb = (*i);

        if (pb->isBookmark()) 
        {
            PrintBookmark(w, static_cast<const Bookmark*>(pb), tab + 1);
        }
        else if (pb->isFolder())
        {
            PrintFolder(w, p, static_cast<const BookmarkFolder*>(pb), tab + 1);
        }
        else if (pb->isSeparator()) 
        {
            stab(w, tab + 1);
            w.print(T("<HR>\r\n"));
        }
        else if (pb->isAlias()) 
        {
            const BookmarkAlias* pa = static_cast<const BookmarkAlias*>(pb);
            const BookmarkItem* pbmk = p->findId(pa->getId());

            if (pbmk && pbmk->isBookmark())
            {
                PrintBookmark(w, static_cast<const Bookmark*>(pbmk), tab + 1);
            }
        }

        i++;
    }

    stab(w, tab);

    w.print(T("</DL><p>\r\n"));
}

//------------------------------------------------------------------------------
void MozillaBookmarks::PrintBookmark(PrintWriter &w, const Bookmark *pb, int tab) 
{
    stab(w, tab);

    w.print(T("<DT><A"));

    WriteHref(w, pb->getHref());

    if (pb->hasId())
    {
        XMLWriteAttribute(w, T("ID"), pb->getId());
    }

    w.print(T(">"));

    if (pb->hasName()) 
    {
        WriteHtml(w, pb->getName());
    }

    w.print(T("</A>\r\n"));

    if (pb->hasDescription()) 
    {
        w.print(T("<DD>"));
        WriteHtml(w, pb->getDescription());
        w.print(T("\r\n"));
    }
}

//------------------------------------------------------------------------------
void MozillaBookmarks::WriteHref(PrintWriter &w, const Href &href) 
{
   char ach[4096], *p = ach;
   char *e = p + href.format(ach, sizeof(ach));

   w.print(T(" HREF=\""));

   while (p != e) 
   {
      char ch = *p++;

      if (ch == '"')
      {
         w.print(T("%22"));
      }
      else 
      {
         w.write(ch);
      }
   }

   w.write('"');
}

//------------------------------------------------------------------------------
void MozillaBookmarks::WriteHtml(PrintWriter &w, const tchar_t *psz) 
{
    char ach[3];    // utf-8 encodes up to max 3 bytes
    size_t len;
    tchar_t ch = *psz++;

    while (ch != 0)
    {
        switch (ch) 
        {
         case '<':  w.print(T("&lt;"));     break;
         case '>':  w.print(T("&gt;"));     break;
         case '"':  w.print(T("&quot;"));   break;
         case '&':  w.print(T("&amp;"));    break;
         case '\n': w.print(T("<BR>\r"));   // fall through
         default:
             len = utf8enc(static_cast<wchar_t>(static_cast<unsigned char>(ch)), ach, sizeof(ach));
             w.write(ach, len);
             break;
        }

        ch = *psz++;
    }
}

//------------------------------------------------------------------------------
void MozillaBookmarks::stab(PrintWriter &w, int tab)
{
    for (int i = 0; i < tab; i++) 
    {
        w.print(T("    "));
    }
}

} // namespace syncit