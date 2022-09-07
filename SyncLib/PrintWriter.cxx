/*
 * SyncLib/PrintWriter.cxx
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
 *    A PrintWriter formats character data to an
 *    OutputStream.  Patterned after java.io.PrintWriter.
 */
#include "PrintWriter.h"

#include <cwchar>
#include "util.h"

using namespace syncit;

PrintWriter::PrintWriter(OutputStream *pout) {
   m_out = pout;
}

/* virtual */
void PrintWriter::write(const char *pbBuffer, size_t cbBuffer) {
   m_out->write(pbBuffer, cbBuffer);
}

/* virtual */
void PrintWriter::flush() {
   m_out->flush();
}

void PrintWriter::print(const tchar_t *psz) {
   assert(psz != NULL);

   while (*psz) {
      write(*psz++);
   }
}

void PrintWriter::print(const tstring &sz) {
   tstring::const_iterator i = sz.begin(), end = sz.end();

   while (i != end) {
      write(*i++);
   }
}

#ifdef TEXT16
void PrintWriter::print(const string &sz) {
   string::const_iterator i = sz.begin(), end = sz.end();

   while (i != end) {
      print((tchar_t) *i++);
   }
}
#endif /* TEXT16 */

/* virtual */
void PrintWriter::close() {
   m_out->close();
}

void PrintWriter::print(unsigned long ul) {
   printf("%lu", ul);
}

void PrintWriter::print(long l) {
   printf("%ld", l);
}

void PrintWriter::print(unsigned __int64 u64) {
   char buf[32];
   int i = 32;

   do {
      buf[--i] = '0' + u64 % 10;
      u64 /= 10;
   } while (u64 != 0);

   write(buf + i, 32 - i);
}

void PrintWriter::printf(const char *pszFormat, ...) {
   va_list ap;

   va_start(ap, pszFormat);
   vprintf(pszFormat, ap);
   va_end(ap);
}

void PrintWriter::vprintf(const char *pszFormat, va_list ap) {
   char achBuffer[1024];

   int i = wvsprintf(achBuffer, pszFormat, ap);

   write(achBuffer, i);
}

static const tchar_t digits[] = T("0123456789ABCDEF");

void PrintWriter::printHex(unsigned char b) {
   write(digits[b >> 4]);
   write(digits[b & 0x0F]);
}
