/*
 * SyncLib/CsvParser.h
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
 *    This module parses CSV (Comma-Separated Value) files, as output by,
 *    say, Microsoft Excel.
 *
 * Revision History:
 * CsvParser.h,v
 * Revision 1.3  2003/10/24 14:12:35  dgehriger
 * * committing 1.5 development branch to trunk
 *
 * Revision 1.2.2.2  2003/10/23 10:25:18  dgehriger
 * * Added support for Netscape 6 & 7
 * * Code refactoring of the Browser code
 * * Split browser code into several source files (one file / class)
 *
 */
#ifndef CsvParser_H
#define CsvParser_H

#include <string>
#include <vector>

#include "BufferedInputStream.h"
#include "Reader.h"

namespace syncit {

   using std::string;
   using std::vector;

   /**
    * Read one line in the file, returning a Vector of the String elements.
    * The Vector may contain null elements, signifying blank spaces in the
    * input.
    *
    * @param in   a BufferedInputStream to read from
    * @param v    a vector of string values to append the results into
    * @return the number of values in the line, -1 on EOF at start of line
    */
   int CsvParser(Reader *in,
                 vector<string> &v);

   /**
    * Read one line in the file, returning a Vector of the String elements.
    * The Vector may contain null elements, signifying blank spaces in the
    * input.
    *
    * @param in   a BufferedInputStream to read from
    * @param papsz a pointer to a blank array of strings, the values are
    *                copied into this array
    * @param npsz  the number of elements in papsz
    * @return the number of values in the line, may be larger than npsz,
    *          -1 on EOF at start of line
    *
    * All values may be freed by the u_free() macro, or use the
    * CsvRelease procedure to release all.
    */
   int CsvParser(BufferedInputStream *in,
                 char **papsz,
                 int npsz);

   /**
    * Free the storage allocated by CsvParser
    *
    * @param papsz a pointer to an array of strings, the same array
    *              passed to CsvParser.
    * @param npsz  the number of elements in papsz, the same value
    *              passed to CsvParser
    * @param r     the result from CsvParser, the number of CSV values
    *              parsed.
    */
   void CsvRelease(char **papsz,
                   int npsz,
                   int r);
}

#endif /* CsvParser_H */
