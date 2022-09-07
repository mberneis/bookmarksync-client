/*
 * SyncLib/Character.cxx
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
#include "Character.h"
#include "Util.h"

using namespace syncit;

CharTypes Character::m_gData;

CharTypes::CharRange CharTypes::m_charData[] = {
#include "Unicode/chartable.c"
};

CharTypes::CharTypes() {
   int ch = 0, i;
   for (i = 0; i < ELEMENTS(m_charData) && ch < 256; i++) {
      while (ch < m_charData[i].first && ch < 256) {
         m_iso8859_1charData[ch] = Unknown;
         m_iso8859_1charUpper[ch] = ch;
         m_iso8859_1charLower[ch] = ch;
         ch++;
      }

      // ch == charData[i].first
      while (ch <= m_charData[i].last && ch < 256) {
         unsigned short offset = ch - m_charData[i].first;
         m_iso8859_1charData[ch] = m_charData[i].t;
         m_iso8859_1charLower[ch] = m_charData[i].lower + offset;
         m_iso8859_1charUpper[ch] = m_charData[i].upper + offset;
         ch++;
      }
   }

   while (ch < 256) {
      m_iso8859_1charData[ch] = Unknown;
      m_iso8859_1charUpper[ch] = ch;
      m_iso8859_1charLower[ch] = ch;
      ch++;
   }
}

int Character::digitValue(tchar_t ch) {
   if ('0' <= ch && ch <= '9') {
      return ch - '0';
   }
   else if ('a' <= ch && ch <= 'f') {
      return 10 + ch - 'a';
   }
   else if ('A' <= ch && ch <= 'F') {
      return 10 + ch - 'A';
   }
   else if (ch == ' ' || ch == '\t') {
      return -1;
   }
   else {
      return -2;
   }
}

#ifdef TEXT16

CharType CharTypes::getCharType0(wchar_t ch) const {
   const CharRange *p = getCharRange(ch);

   return p ? (CharType) p->t : Unknown;
}

wchar_t CharTypes::toUpper0(wchar_t ch) const {
   const CharRange *p = getCharRange(ch);

   if (p == NULL) {
      return ch;
   }
   else {
      unsigned short offset = ch - p->first;

      return p->upper + offset;
   }
}

wchar_t CharTypes::toLower0(wchar_t ch) const {
   const CharRange *p = getCharRange(ch);

   if (p == NULL) {
      return ch;
   }
   else {
      unsigned short offset = ch - p->first;

      return p->lower + offset;
   }
}

const CharTypes::CharRange *CharTypes::getCharRange(wchar_t ch) const {
   int left = 0, right = ELEMENTS(m_charData);

   while (left != right) {
      int middle = (left + right) / 2;

      if (ch < m_charData[middle].first) {
         right = middle;
      }
      else if (ch > m_charData[middle].last) {
         left = middle + 1;
      }
      else {
         /* charData[middle].first <= ch && ch <= charData[middle.last */
         return m_charData + middle;
      }
   }

   return NULL;
}
#endif /* TEXT16 */
