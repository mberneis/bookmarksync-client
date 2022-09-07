/*
 * SyncLib/HTML.h
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
#ifndef HTML_H
#define HTML_H

#include <string>

#include "XML.h"
#include "HttpRequest.h"

namespace syncit {

   using std::string;

   class HtmlParser : public XMLParser {

   public:
      HtmlParser() {
         m_fTitle = false;
      }

      virtual void xml(TokenType t, const tchar_t *psz, size_t cch,
                       bool fStart, bool fComplete);

      bool hasTitle() const {
         return m_title.size() > 0 && !m_fTitle;
      }

      const string &getTitle() const {
         return m_title;
      }

   private:
      bool m_fTitle;
      string m_title;
   };

   int GetHTMLTitle(HttpRequest *preq,
                    const URL &url,
                    char *pachTitle,
                    size_t cchTitle);
}

#endif /* HTML_H */
