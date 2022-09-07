/*
 * SyncLib/UTF8.cxx
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
#include <cstring>

#include "UTF8.h"
#include "util.h"

#define fill6(i) ((char) ((int) ((i & 0x3F) | 0x80)))

size_t syncit::utf8enc(const wchar_t *psz, char *pach, size_t cch) {
   char *p = pach;
   wchar_t ch = *psz++;

   while (ch != 0 && cch >= 3) {
      size_t i = utf8enc(ch, p, cch);
      p += i;
      cch -= i;
      ch = *psz++;
   }

   while (ch != 0) {
      char ach[3];
      size_t i = utf8enc(ch, ach, 3);

      if (i > cch) {
         if (cch > 0) {
            u_memcpy(p, ach, cch);
         }

         return p - pach + cch;
      }

      u_memcpy(p, ach, i);
      p += i;
      cch -= i;
      ch = *psz++;
   }

   if (cch > 0) {
      *p = 0;
   }

   return p - pach;
}

size_t syncit::utf8enc(wchar_t ch, char *pach, size_t cch) {
   assert(cch >= 3);

   if (ch < 0x80) {
      pach[0] = (char) ch;
      return 1;
   }
   else if (ch < 0x800) {
      pach[1] = fill6(ch);
      ch >>= 6;
      pach[0] = 0xC0 | (ch & 0x1F);
      return 2;
   }
   else {
      pach[2] = fill6(ch);
      ch >>= 6;
      pach[1] = fill6(ch);
      ch >>= 6;
      pach[0] = 0xE0 | (ch & 0x0F);
      return 3;
   }
}

size_t syncit::utf8enc(unsigned long ul, char *pach, size_t cch) {
   assert(cch >= 6);

   if (ul < 0x00010000) {
      return utf8enc((wchar_t) ul, pach, cch);
   }
   else if (ul < 0x00200000) {
      pach[3] = fill6(ul);
      ul >>= 6;
      pach[2] = fill6(ul);
      ul >>= 6;
      pach[1] = fill6(ul);
      ul >>= 6;

      pach[0] = (char) (0xF0 | (ul & 0x07));
      return 4;
   }
   else if (ul < 0x04000000) {
      pach[4] = fill6(ul);
      ul >>= 6;
      pach[3] = fill6(ul);
      ul >>= 6;
      pach[2] = fill6(ul);
      ul >>= 6;
      pach[1] = fill6(ul);			// 100?
      ul >>= 6;

      pach[0] = (char) (0xF8 | (ul & 0x03));
      return 5;
   }
   else {
      pach[5] = fill6(ul);
      ul >>= 6;
      pach[4] = fill6(ul);
      ul >>= 6;
      pach[3] = fill6(ul);
      ul >>= 6;
      pach[2] = fill6(ul);
      ul >>= 6;
      pach[1] = fill6(ul);
      ul >>= 6;

      pach[0] = (char) (0xFC | (ul & 0x01));
      return 6;
   }
}

size_t syncit::utf8dec(const char *psz, wchar_t *pach, size_t cch) {
   wchar_t *p = pach;

   if (cch > 0) {
      int i;

      do {
         i = utf8dec(psz, &psz);

         *p++ = (wchar_t) i;
         cch--;
      } while (i != 0 && cch > 0 && *psz != '\0');
   }

   return p - pach;
}

int syncit::utf8dec(const char *psz, const char **ppsz) {
   const unsigned char *p = (const unsigned char *) psz;
   int r;
   unsigned char b = *p++;

   if (b < 0x80) {
      r = b;
   }
   else {
      const unsigned char *e;

      if (b < 0xE0) {
         r = b & 0x1F;
         e = p + 1;
      }
      else if (b < 0xF0) {
         r = b & 0x0F;
         e = p + 2;
      }
      else if (b < 0xF8) {
         r = b & 0x07;
         e = p + 3;
      }
      else if (b < 0xFC) {
         r = b & 0x03;
         e = p + 4;
      }
      else if (b < 0xFE) {
         r = b & 0x01;
         e = p + 5;
      }
      else {
         // error: first char is either FE or FF, bad
         if (ppsz != NULL) *ppsz = psz;
         return -1;
      }

      while (e != p) {
         b = *p;

         if ((b & 0xC0) != 0x80) {
            if (ppsz != NULL) *ppsz = (const char *) p;
            return -1;
         }
         else {
            r = (r << 6) | (b & 0x3F);
            p++;
         }
      }
   }

   if (ppsz != NULL) *ppsz = (const char *) p;

   return r;
}
