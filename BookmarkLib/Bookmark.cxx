/*
 * BookmarkLib/Bookmark.cxx
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
 *    This file defines methods for the Bookmark class.  A Bookmark object
 *    describes one bookmark in the user's browser bookmark file.
 *
 * See also:
 *    BookmarkModel.h      -- contains the declarations for the class
 *                            defined here.
 *    BookmarkFolder.cxx
 *
 *    util.h               -- memory and string management
 */
#include <stdlib.h>

#include "BookmarkModel.h"

#include "SyncLib/util.h"
#include "SyncLib/text.h"

using namespace syncit;

Bookmark::Bookmark() : BookmarkItem(NULL) {
#ifndef NDEBUG
   strcpy(m_achStartTag, "Bookmark");
   strcpy(m_achEndTag, "Bookmark");
#endif /* NDEBUG */

   m_modified = 0L;
   m_visited = 0L;

   assert(isValid());
}

Bookmark::Bookmark(const Bookmark &rhs) : BookmarkItem(rhs), m_href(rhs.m_href) {
#ifndef NDEBUG
   strcpy(m_achStartTag, "Bookmark");
   strcpy(m_achEndTag, "Bookmark");
#endif /* NDEBUG */

   m_modified = rhs.m_modified;
   m_visited = rhs.m_visited;

   assert(isValid());
}

Bookmark::~Bookmark() {
   assert(isValid());

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */
}

Bookmark &Bookmark::operator=(const Bookmark &rhs) {
   BookmarkItem::operator=(rhs);

   m_href = rhs.m_href;
   m_modified = rhs.m_modified;
   m_visited = rhs.m_visited;

   return *this;
}

/* virtual */
bool Bookmark::isBookmark() const {
   return true;
}

#ifndef NDEBUG
bool Bookmark::isValid() const {
   return strcmp(m_achStartTag, "Bookmark") == 0 &&
          strcmp(m_achEndTag, "Bookmark") == 0;
}
#endif /* NDEBUG */

/**
 * Two bookmarks are equal if the names are equal,
 * the URLs are equal, and the modify times have the
 * same value to the resolution of one second.
 */
bool Bookmark::equals(const BookmarkObject *pbo) const {
   if (pbo->isBookmark()) {
      const Bookmark *p = (const Bookmark *) pbo;

      assert(isValid());
      assert(p->isValid());

      if (this == p)
         return true;
      else if (m_href != p->m_href) {
         return false;
      }
      else if (m_pszName == NULL && p->m_pszName == NULL) {
         return true;
      }
      else if (m_pszName == NULL || p->m_pszName == NULL) {
         return false;
      }
      else {
         return tstricmp(m_pszName, p->m_pszName) == 0;
      }
   }
   else {
      return false;
   }
}

/**
 * The url property is the location that the
 * bookmark points to.
 * <p>
 * A dynamically allocated copy of the parameter is taken,
 * so the string may be modified or freed at any time after
 * setHref() is called.
 *
 * @param psz  the new value of the url property
 * @see #getHref()
 */
void Bookmark::setHref(const char *psz) {
   m_href = Href::Intern(psz);
}
