/*
 * SyncLib/Character.h
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
 *    A java class library -inspired utility class.
 *    This defines the character attributes of Unicode characters.
 */
#ifndef Character_H
#define Character_H

#include "text.h"

namespace syncit {

   enum CharType {
      Unknown,

      /* Normative */
      Mn, /* = Mark, Non-Spacing */
      Mc, /* = Mark, Spacing Combining */
      Me, /* = Mark, Enclosing */

      Nd, /* = Number, Decimal Digit */
      Nl, /* = Number, Letter */
      No, /* = Number, Other */

      Zs, /* = Separator, Space */
      Zl, /* = Separator, Line */
      Zp, /* = Separator, Paragraph */

      Cc, /* = Other, Control */
      Cf, /* = Other, Format */
      Cs, /* = Other, Surrogate */
      Co, /* = Other, Private Use */
      Cn, /* = Other, Not Assigned */

      /* Informative */
      Lu, /* = Letter, Uppercase */
      Ll, /* = Letter, Lowercase */
      Lt, /* = Letter, Titlecase */
      Lm, /* = Letter, Modifier */
      Lo, /* = Letter, Other */

      Pc, /* = Punctuation, Connector */
      Pd, /* = Punctuation, Dash */
      Ps, /* = Punctuation, Open */
      Pe, /* = Punctuation, Close */
      Pi, /* = Punctuation, Initial quote (may behave like Ps or Pe depending on usage) */
      Pf, /* = Punctuation, Final quote (may behave like Ps or Pe depending on usage) */
      Po, /* = Punctuation, Other */

      Sm, /* = Symbol, Math */
      Sc, /* = Symbol, Currency */
      Sk, /* = Symbol, Modifier */
      So, /* = Symbol, Other */

      TypeMask          = 0x1F,  /* 0001-1111 */

      XmlNameChar       = 0xE0,  /* 1110-0000 */
      XmlLetter         = 0x20,  /* 0010-0000 */
      XmlDigit          = 0x40,  /* 0100-0000 */
      XmlExtender       = 0x60,  /* 0110-0000 */
      XmlCombiningChar  = 0x80   /* 1000-0000 */
   };

   class CharTypes {

   public:
      CharTypes();

      bool isXmlDigit(tchar_t ch) const {
         return (getCharType(ch) & XmlDigit) == XmlDigit;
      }

      bool isXmlLetter(tchar_t ch) const {
         return (getCharType(ch) & XmlLetter) == XmlLetter;
      }

#ifdef TEXT16
      inline CharType getCharType(wchar_t ch) const {
         return (CharType) (ch < 256 ? m_iso8859_1charData[ch] : getCharType0(ch));
      }

      inline wchar_t toUpper(wchar_t ch) const {
         return (ch < 256) ? (wchar_t) m_iso8859_1charUpper[ch] : toUpper0(ch);
      }

      inline wchar_t toLower(wchar_t ch) const {
         return (ch < 256) ? (wchar_t) m_iso8859_1charLower[ch] : toLower0(ch);
      }

      CharType getCharType0(wchar_t ch) const;
      wchar_t toUpper0(wchar_t ch) const;
      wchar_t toLower0(wchar_t ch) const;

#else /* TEXT16 */
      inline CharType getCharType(char ch) const {
         return (CharType) m_iso8859_1charData[(unsigned char) ch];
      }

      inline char toUpper(char ch) const {
         return (char) m_iso8859_1charUpper[(unsigned char) ch];
      }

      inline char toLower(char ch) const {
         return (char) m_iso8859_1charLower[(unsigned char) ch];
      }
#endif /* TEXT16 */

   private:
      unsigned char m_iso8859_1charData[256];
      unsigned char m_iso8859_1charUpper[256];
      unsigned char m_iso8859_1charLower[256];

      static struct CharRange {
         wchar_t first; /* first character in range */
         wchar_t last;  /* last character in range */
         wchar_t upper; /*   upper case equivalent */
         wchar_t lower; /*   lower case equivalent */
         wchar_t title; /*   title case equivalent */
         unsigned char  t;
      } m_charData[];

#ifdef TEXT16
      const CharRange *getCharRange(wchar_t ch) const;
#endif /* TEXT16 */
   };

   class Character {
   public:
      enum {
         CR = T('\r'),
         LF = T('\n'),
         SP = T(' '),
         HT = T('\t'),

         NBSP = 0x00A0
      };

      static bool isXmlDigit (tchar_t ch) { return m_gData.isXmlDigit(ch); }
      static bool isXmlLetter(tchar_t ch) { return m_gData.isXmlLetter(ch); }

      static CharType getCharType(tchar_t ch) { return m_gData.getCharType(ch); }
      static tchar_t  toUpper    (tchar_t ch) { return m_gData.toUpper(ch);     }
      static tchar_t  toLower    (tchar_t ch) { return m_gData.toLower(ch);     }

      /**
       * Parse a single character for its numeric value
       * @return 0..9 if ch in range of '0'..'9'
       * @return 10..15 if ch in range of 'a'..'f' or 'A'..'F'
       * @return negative value (-1 if space, -2 if not)
       */
      static int digitValue(tchar_t ch);

      /**
       * @param ch  character to test
       * @return true iff ch in { space, tab, newline, carriage-return }
       */
      static bool isSpace(tchar_t ch) {
         return ch == SP || ch == HT || ch == LF || ch == CR;
      }

   private:
      static CharTypes m_gData;
   };

}

#endif /* Character_H */
