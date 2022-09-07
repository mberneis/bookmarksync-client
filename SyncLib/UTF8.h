/*
 * SyncLib/UTF8.h
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
#ifndef UTF8_H
#define UTF8_H

#include <cwchar>

namespace syncit {

   /**
    * Encode a UTF16 (wchar_t) string into a UTF8 (char) string.
    *
    * @param psz  pointer to null-terminated UTF16 wide char Unicode string
    * @param pach  pointer to buffer to contain UTF8
    * @param cch   length, in bytes or characters of buffer pach
    *
    * @return length of encoded text, not including null
    */
   size_t utf8enc(const wchar_t *psz, char *pach, size_t cch);

   /**
    * Encode a single UTF16 (wchar_t) character into a UTF8 (char) string.
    * The string must be at least 3 characters long.
    *
    * @param ch  UTF16 wide char Unicode character to encode
    * @param pach  pointer to buffer to contain UTF8
    * @param cch  length, in bytes or characters of buffer pach
    *
    * @return length of encoded text
    * @require cch >= 3
    */
   size_t utf8enc(wchar_t ch, char *pach, size_t cch);

   /*
    * Encode a single UTF32 (int) character into a UTF8 (char) string.
    * The string must be at least 6 characters long.
    *
    * @param ul  UTF32 long Unicode character to encode
    * @param pach  pointer to buffer to contain UTF8
    * @param cch  length, in bytes or characters of buffer pach
    *
    * @return length of encoded text
    * @require cch >= 6
    */
   size_t utf8enc(unsigned long ul, char *pach, size_t cch);

   /**
    * Decode a UTF8 (char) string into UTF16 wide character Unicode
    *
    * @param psz  pointer to null-terminated UTF8 character string to decode
    * @param pach  pointer to wide char buffer to contain Unicode UTF16 text
    * @param cch   size, in 16-bit characters, of buffer pach
    *
    * @return number of characters successfully decoded
    */
   size_t utf8dec(const char *psz, wchar_t *pach, size_t cch);

   /**
    * Decode a single Unicode character from a UTF8 (char) string
    *
    * @param psz  pointer to start of UTF8 encoded character
    * @param ppsz  pointer to pointer past the single character
    *
    * @return the Unicode character processed
    */
   int utf8dec(const char *psz, const char **ppsz);
}

#endif /* UTF8_H */
