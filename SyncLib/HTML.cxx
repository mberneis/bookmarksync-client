/*
 * SyncLib/HTML.cxx
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
#include "HttpRequest.h"
#include "URL.h"

#include "FileOutputStream.h"
#include "HTML.h"

using namespace syncit;


/* virtual */
void HtmlParser::xml(TokenType t, const tchar_t *psz, size_t cch,
                     bool fStart, bool fComplete) {

   if (t == STARTING_TAG && tstricmp(psz, "title") == 0) {
      m_fTitle = true;
   }
   else if (t == END_TAG && tstricmp(psz, "title") == 0) {
      m_fTitle = false;
   }
   else if (t == STARTING_TAG &&
            (tstricmp(psz, "script") == 0 || tstricmp(psz, "style") == 0)) {
      setContentType(CONTENT_CDATA);
   }
   else if (t == CHAR_DATA && m_fTitle) {
      char *p = strchr(psz, '\n');

      while (p != NULL) {
         m_title.append(psz, p - psz);
         psz = p + 1;
         p = strchr(psz, '\n');
      }

      m_title += psz;
   }
}

/**
 * Retrieve an HTML document via HTTP, and parse it to get
 * its title.
 *
 * @param pszUrl     null-terminated URL of document to fetch
 * @param pachTitle  pointer to buffer to store title
 * @param cchTile    length, in characters, of pachTitle buffer
 *
 * @return an HTTP status code, an error code returned from
 *          HttpRequest::send, or 0 if the URL doesn't begin
 *          with http://
 */
int syncit::GetHTMLTitle(HttpRequest *preq,
                         const URL &url,
                         char *pachTitle,
                         size_t cchTitle) {

   int l = 0;
   int r = preq->send("GET", url);

   if (200 <= r && r <= 299) {
      if (preq->getHeader("content-type") == "text/html") {
         HtmlParser parser;

         try {
            parser.parse(*preq->getInputStream());
         } catch (...) {
            if (!parser.hasTitle()) {
               throw;
            }
         }

         l = parser.getTitle().copy(pachTitle, cchTitle - 1);
      }
   }

   if (l < cchTitle) {
      pachTitle[l] = 0;
   }

   return r;
}

int fetchdoc(HttpRequest *preq, const URL &url, const char *pszPath) {
   int r = preq->send("GET", url);
   if (200 <= r && r <= 299) {
      FileOutputStream f;

      f.create(pszPath);

      try {
         char buf[4096];

         InputStream *pin = preq->getInputStream();

         int s = pin->read(buf, sizeof(buf));
         while (s > 0) {
            f.write(buf, s);
            s = pin->read(buf, sizeof(buf));
         }

         f.close();
         f.commit();
      } catch (...) {
         f.close();

         DeleteFile(pszPath);

         throw;
      }
   }

   return r;
}
