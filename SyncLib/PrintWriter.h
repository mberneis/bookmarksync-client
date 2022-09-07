/*
 * SyncLib/PrintWriter.h
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
#ifndef PrintWriter_H
#define PrintWriter_H

#include <cassert>
#include <cstdarg>

#include "text.h"
#include "OutputStream.h"

namespace syncit {

   using namespace std;

   class PrintWriter : public OutputStream {

   public:
      PrintWriter(OutputStream *pout);

      virtual ~PrintWriter() {
      }

      virtual void write(const char *pbBuffer, size_t cbBuffer);
      virtual void flush();
      virtual void close();

      void print(const tchar_t *psz);
      void print(const tstring &sz);
   #ifdef TEXT16
      void print(const string  &sz);
   #endif

      void printf(const char *pszFormat, ...);
      void vprintf(const char *pszFormat, va_list ap);

      void write(tchar_t ch) {
         char c = (char) ch;
         write(&c, 1);
      }

      void print(unsigned long ul);
      void print(long l);

      void print(unsigned __int64 u64);

      void printHex(unsigned char b);

   protected:
      OutputStream *m_out;

   private:
      // Disable copy constructor and assignment
      PrintWriter(PrintWriter &rhs);
      PrintWriter &operator=(PrintWriter &rhs);
   };

}

#endif /* PrintWriter_H */
