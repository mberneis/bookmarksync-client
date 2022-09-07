/*
 * BookmarkLib/Href.cxx
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
#pragma warning (disable : 4786)

#include <cassert>

#include "BookmarkModel.h"

using namespace syncit;

Href::Table Href::m_gtable;

#ifndef NDEBUG
Href::Table::~Table() {
   Set::iterator i = m_set.begin(), e = m_set.end();

   while (i != e) {
      delete *i++;
   }
}
#endif /* NDEBUG */

Href::Data *Href::Attach(Href::Data *p) {
   p->flags++;
   return p;
}

void Href::release() {
   if (m_p != NULL) {
      assert(refcount() != 0);
      m_p->flags--;

      if (refcount() == 0) {
         if (m_p->flags & INTERNED_MASK) {
            Set::iterator i = m_gtable.m_set.find(m_p);

            assert(i != m_gtable.m_set.end());
            m_gtable.m_set.erase(i);
         }

         delete[] (char *) m_p;
      }
   }
}

Href Href::Intern(const char *psz) {
   // construct a temporary Href for set lookup
   union {
      char ab[4096];
      Data d;
   } u;

   size_t cb = init(&u.d, psz);

   Set::const_iterator i = m_gtable.m_set.find(&u.d);

   if (i == m_gtable.m_set.end()) {
      Data *pnew = (Data *) NEW char[cb];
      memcpy(pnew, &u.d, cb);

      pnew->flags = (pnew->flags + 1) | INTERNED_MASK;

      m_gtable.m_set.insert(pnew);

      return Href(pnew);
   }
   else {
      return Href(*i);
   }
}

size_t Href::getHost0(char *pach, size_t cch, size_t l) const {
   char *p = pach;
   size_t i;

   if (getWWW()) {
      i = bufcopy("www.", 4, p, cch);
      p += i;
      cch -= i;
   }

   i = bufcopy(m_p->ach, l, p, cch);
   p += i;
   cch -= i;

   switch (getDomain()) {
      case COM:      i = bufcopy(".com", 4, p, cch); break;
      case ORG:      i = bufcopy(".org", 4, p, cch); break;
      case NET:      i = bufcopy(".net", 4, p, cch); break;

      case OTHER_DOMAIN:
      default:
         i = 0;
   }

   return (p - pach) + i;
}

size_t Href::format(char *pach, size_t cch) const {
   char *p = pach;
   size_t i;

   switch (getProtocol()) {
      case AOL:      i = bufcopy("aol:", 4, p, cch); break;
      case FTP:      i = bufcopy("ftp://", 6, p, cch); break;
      case HTTP:     i = bufcopy("http://", 7, p, cch); break;
      case HTTPS:    i = bufcopy("https://", 8, p, cch); break;
      case MAILTO:   i = bufcopy("mailto:", 7, p, cch); break;
      case SOCKS:    i = bufcopy("socks:", 6, p, cch); break;
      case FILE:     i = bufcopy("file:", 5, p, cch); break;

      case OTHER_PROTOCOL:
      default:
         i = 0;
   }

   p += i;
   cch -= i;

   size_t l = lstrlenA(m_p->ach);
   i = getHost0(p, cch, l);
   p += i;
   cch -= i;

   i = bufcopy(m_p->ach + l + 1, p, cch);
   return (p - pach) + i;
}

int Href::Compare0(const Data *pd1, const Data *pd2) {
   int c = (pd1->flags & ~(REFCOUNT_MASK | INTERNED_MASK)) - (pd2->flags & ~(REFCOUNT_MASK | INTERNED_MASK));

   if (c == 0) {
      c = lstrcmpA(pd1->ach, pd2->ach);

      if (c == 0) {
         c = lstrcmpA(pd1->ach + lstrlenA(pd1->ach) + 1,
                      pd2->ach + lstrlenA(pd2->ach) + 1);
      }
   }

   return c;
}

static bool strequal(const char *psz1, const char *psz2, size_t n) {
   for (size_t i = 0; i < n; i++) {
      if (tolower(*psz1) != *psz2++) {
         return false;
      }

      psz1++;
   }

   return true;
}

size_t Href::init(Data *pd, const char *psz) {
   unsigned short flags;

   if (strequal(psz, "http://", 7)) {
      flags = HTTP;
      psz += 7;
   }
   else if (strequal(psz, "https://", 8)) {
      flags = HTTPS;
      psz += 8;
   }
   else if (strequal(psz, "ftp://", 6)) {
      flags = FTP;
      psz += 6;
   }
   else if (strequal(psz, "aol:", 4)) {
      flags = AOL;
      psz += 4;
   }
   else if (strequal(psz, "file:", 5)) {
      flags = FILE;
      psz += 5;
   }
   else if (strequal(psz, "mailto:", 7)) {
      flags = MAILTO;
      psz += 7;
   }
   else if (strequal(psz, "socks:", 6)) {
      flags = SOCKS;
      psz += 6;
   }
   else {
      flags = OTHER_PROTOCOL;
   }

   if (strequal(psz, "www.", 4)) {
      flags |= WWW_MASK;
      psz += 4;
   }

   const char *p;
   size_t l;

   for (p = psz; *p && *p != '/' && *p != ':'; p++)
      ;

   l = p - psz;
   // *p == 0 || *p == '/' || *p == ':'

   if (p - psz >= 4) {
      if (strequal(p - 4, ".com", 4)) {
         flags |= COM;
         l -= 4;
      }
      else if (strequal(p - 4, ".net", 4)) {
         flags |= NET;
         l -= 4;
      }
      else if (strequal(p - 4, ".org", 4)) {
         flags |= ORG;
         l -= 4;
      }
      else {
         flags |= OTHER_DOMAIN;
      }
   }
   else {
      flags |= OTHER_DOMAIN;
   }

   pd->flags = flags;
   memcpy(pd->ach, psz, l);
   pd->ach[l++] = 0;

   lstrcpyA(pd->ach + l, p);

   return sizeof(Data) + l + lstrlenA(pd->ach + l) - 1;
}
