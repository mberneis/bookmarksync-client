/*
 * SyncLib/BinarySearch.cxx
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
#include <cassert>

#include "text.h"
#include "BinarySearch.h"

/**
 * Binary search across a sorted array, looking
 * for an integer token value given a token string.
 *
 * @param psz   null-terminated Unicode string to identify
 * @param pa    pointer to sorted array of Token structures
 * @param c     number of Token structures pointed to by pa
 * @param def   return value if psz isn't found in pa[0, c)
 *
 * @require tstrlen(psz) < 256  and  pa[0, c) is sorted
 * @return 
 */
int syncit::BinarySearch(const char *psz, const Token *pa, size_t c, int def) {

#ifndef NDEBUG
   for (size_t i = 1; i < c; i++) {
      assert(strcmp(pa[i].psz, pa[i - 1].psz) > 0);
   }
#endif /* NDEBUG */

   int left = 0, right = c;

   while (left != right) {
      int middle = (left + right) / 2;
      int cmp = strcmp(psz, pa[middle].psz);

      if (cmp < 0) {
         right = middle;
      }
      else if (cmp > 0) {
         left = middle + 1;
      }
      else {
         return pa[middle].i;
      }
   }

   return def;
}
