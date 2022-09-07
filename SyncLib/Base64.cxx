/*
 * SyncLib/Base64.cxx
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
#include "Base64.h"

static const char Base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void syncit::Base64Encode(const unsigned char *pabIn,
                          size_t cbIn, 
                          char *pachOut) {
   const unsigned char *pabInMax = pabIn + cbIn;

   while (pabIn < pabInMax) {
      unsigned long ul;

      ul  = *pabIn++ << 16;

      if (pabIn < pabInMax) {
         ul |= *pabIn++ << 8;

         if (pabIn < pabInMax) {
            ul |= *pabIn++;
         }
      }

      *pachOut++ = Base64[(ul >> 18) & 0x3F];
      *pachOut++ = Base64[(ul >> 12) & 0x3F];
      *pachOut++ = Base64[(ul >>  6) & 0x3F];
      *pachOut++ = Base64[(ul      ) & 0x3F];
   }

   switch (cbIn % 3) {
      case 0:
         break;

      case 1:
         pachOut[-2] = '=';
         // fallthrough

      case 2:
         pachOut[-1] = '=';
         break;
   }

   *pachOut = '\0';
}
