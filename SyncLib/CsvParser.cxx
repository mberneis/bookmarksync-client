/*
 * SyncLib/CsvParser.cxx
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
#pragma warning( disable : 4786 )

#include <string>

#include "CsvParser.h"
#include "util.h"
#include "StringBuffer.h"

using namespace syncit;

/**
 * A CsvParser parses Comma-Separated-Value (CSV) files.
 * The syntax of a CSV file is:
 * <pre>
 *    file ::=    line*
 *
 *    line ::=    '\n'
 *              | values
 *
 *    values ::=  value '\n'
 *              | value ',' values
 *
 *    value ::=   '"' string_char* '"'
 *              | char*
 *
 *    string_char ::=    <any character other than '"' and '\n'>
 *                     | '"' '"'
 *
 *    char ::=    <any character other than '\n' '"' and ','>
 * </pre>
 * <p>
 * For example:
 * <pre>
 *  this,is,a,test
 *  "A string,", "Followed by a ""quoted string"""
 *  may,also,,contain blanks.
 * </pre>
 * <p>
 * This results in:
 * <table border=1>
 *  <tr><td>this<td>is<td>a<td>test
 *  <tr><td>A string,<td>Followed by a "quoted string"
 *  <tr><td>may<td>also<td><td>contain blanks.
 * </table>
 *
 * @author <a href=mailto:tway@tway.demon.co.uk>Terence Way</a>
 * @author dgehriger
 * @version $Version: $
 */
enum CsvState {
   START,

   TOKEN,
   COMMA,
   STRING,
   QUOTED
};

// Design notes on state machine:
//    Events:
//       -1             EOF end-of-file
//       '\r'           CR  carriage return
//       '\n'           LF  newline
//       ','            ',' comma
//       ' ' and '\t'   WS  whitespacespace
//       '"'            QT  quote
//       anything else  CH  character
//
//    States:
//       TOKEN
//       COMMA
//       STRING
//       QUOTED
//
//    Transitions:
//
//             INITIAL     TOKEN       COMMA       STRING      QUOTED
//       ---   ----------- ----------- ----------- ----------- ------------
//       EOF   return null return v    return v    error       return v
//       LF    return v    return v    return v       -        return v
//       CR       -          -            -           -           -
//       WS       -          -            -           -           -
//       ','   goto COMMA  goto COMMA     -           -        goto COMMA
//       QT    goto STRING goto QUOTED goto STRING goto QUOTED goto STRING
//       CH    goto TOKEN    -         goto TOKEN     -           -
//


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
 * All values may be freed by the u_free() macro
 */
int syncit::CsvParser(BufferedInputStream *in,
                      char **papsz,
                      int npsz) {
   int i = 0, r = -1;
   StringBuffer sval;

   CsvState state = START;
   bool done = false;

   int ch = in->read();

   while (ch == '\r' || ch == ' ' || ch == '\t') {
      ch = in->read();
   }

   // pre: v == null  && ch != '\r'  && ch != ' ' && ch != '\t'
   // post: done || (v != null && state != START)
   switch (ch) {
      case -1:
         // eof at start of line
         r = -1;
         done = true;
         break;

      case '\n':
         // empty line, return empty vector
         r = 0;
         done = true;
         break;

      case ',':
         state = COMMA;
         if (i < npsz) papsz[i] = NULL;
         i++;
         break;

      case '"':
         state = STRING;
         break;

      default:
         state = TOKEN;
         sval.append((char) ch);
         break;
   }

   while (!done) {
      ch = in->read();

      switch(state) {
         // pre: v != null && sval != null && ch != '"'
         // post: done || (v != null && sval != null)
         case TOKEN:
            switch (ch) {
               case -1:
               case '\n':
                  done = true;
                  if (i < npsz) papsz[i] = sval.getString();
                  i++;
                  r = i;
                  break;

               case '\r':
                  break;

               case ',':
                  state = COMMA;
                  if (i < npsz) papsz[i] = sval.getString();
                  i++;
                  sval.reset();
                  break;

               case '"':
                  state = STRING;
                  break;

               default:
                  sval.append((char) ch);
                  break;
            }
            break;

         // pre: v != null
         case COMMA:
            switch (ch) {
               case -1:
               case '\n':
                  done = true;
                  if (i < npsz) papsz[i] = NULL;
                  i++;
                  r = i;
                  break;

               case '\r':
                  break;

               case ',':
                  if (i < npsz) papsz[i] = NULL;
                  i++;
                  break;

               case '"':
                  state = STRING;
                  sval.reset();
                  break;

               case ' ':
               case '\t':
                  /* ignore */
                  break;

               default:
                  state = TOKEN;
                  sval.reset();
                  sval.append((char) ch);
                  break;
            }
            break;

         // pre: ch != -1 && sval != null
         //
         case STRING:
            switch (ch) {
               case -1:
               case '\n':
                  done = true;
                  if (i < npsz) papsz[i] = NULL;
                  i++;
                  r = i;
                  break;

               case '\r':
                  break;

               case '"':
                  state = QUOTED;
                  break;

               default:
                  sval.append((char) ch);
                  break;
            }
            break;

         // pre: ch in { EOF, '\n', '\r', ',', '"' } && sval != null && v != null
         // post: done || sval != null
         case QUOTED:
            switch (ch) {
               case -1:
               case '\n':
                  done = true;
                  if (i < npsz) papsz[i] = sval.getString();
                  i++;
                  r = i;
                  break;

               case '\r':
                  break;

               case ',':
                  state = COMMA;
                  if (i < npsz) papsz[i] = sval.getString();
                  i++;
                  sval.reset();
                  break;

               case '"':
                  state = STRING;
                  sval.append((char) ch);
                  break;

               default:
                  state = TOKEN;
                  sval.append((char) ch);
                  break;
            }
            break;
      }
   }

   return r;
}

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
 void syncit::CsvRelease(char **papsz,
                         int npsz,
                         int r) {
   int i;

   for (i = 0; i < npsz && i < r; i++) {
      u_free0(papsz[i]);
   }
}
// Design notes on state machine:
//    Events:
//       -1             EOF end-of-file
//       '\r'           CR  carriage return
//       '\n'           LF  newline
//       ','            ',' comma
//       ' ' and '\t'   WS  whitespacespace
//       '"'            QT  quote
//       anything else  CH  character
//
//    States:
//       TOKEN
//       COMMA
//       STRING
//       QUOTED
//
//    Transitions:
//
//             INITIAL     TOKEN       COMMA       STRING      QUOTED
//       ---   ----------- ----------- ----------- ----------- ------------
//       EOF   return null return v    return v    error       return v
//       LF    return v    return v    return v       -        return v
//       CR       -          -            -           -           -
//       WS       -          -            -           -           -
//       ','   goto COMMA  goto COMMA     -           -        goto COMMA
//       QT    goto STRING goto QUOTED goto STRING goto QUOTED goto STRING
//       CH    goto TOKEN    -         goto TOKEN     -           -
//


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
 * All values may be freed by the u_free() macro
 */
int syncit::CsvParser(Reader *in,
                      vector<string> &v) {
   int r = 0;
   string sval;

   CsvState state = START;
   bool done = false;

   int ch = in->read();

   while (ch == '\r' || ch == ' ' || ch == '\t') {
      ch = in->read();
   }

   // pre: v == null  && ch != '\r'  && ch != ' ' && ch != '\t'
   // post: done || (v != null && state != START)
   switch (ch) {
      case -1:
         // eof at start of line
         r = -1;
         // fall through

      case '\n':
         // empty line, return empty vector
         done = true;
         break;

      case ',':
         state = COMMA;
         v.push_back(sval);
         break;

      case '"':
         state = STRING;
         break;

      default:
         state = TOKEN;
         sval += (char) ch;
         break;
   }

   while (!done) {
      ch = in->read();

      switch(state) {
         // pre: v != null && sval != null && ch != '"'
         // post: done || (v != null && sval != null)
         case TOKEN:
            switch (ch) {
               case -1:
                  r = -1;
                  // fall through

               case '\n':
                  done = true;
                  v.push_back(sval);
                  break;

               case '\r':
                  break;

               case ',':
                  state = COMMA;
                  v.push_back(sval);
                  sval.resize(0);
                  break;

               case '"':
                  state = STRING;
                  break;

               default:
                  sval += (char) ch;
                  break;
            }
            break;

         // pre: v != null
         case COMMA:
            switch (ch) {
               case -1:
                  r = -1;
                  // fall through

               case '\n':
                  done = true;
                  v.push_back(sval);
                  break;

               case '\r':
                  break;

               case ',':
                  v.push_back(sval);
                  break;

               case '"':
                  state = STRING;
                  sval.resize(0);
                  break;

               case ' ':
               case '\t':
                  /* ignore */
                  break;

               default:
                  state = TOKEN;
                  sval.resize(0);
                  sval += (char) ch;
                  break;
            }
            break;

         // pre: ch != -1 && sval != null
         //
         case STRING:
            switch (ch) {
               case -1:
                  r = -1;
                  // fall through

               case '\n':
                  done = true;
                  v.push_back(sval);
                  break;

               case '\r':
                  break;

               case '"':
                  state = QUOTED;
                  break;

               default:
                  sval += (char) ch;
                  break;
            }
            break;

         // pre: ch in { EOF, '\n', '\r', ',', '"' } && sval != null && v != null
         // post: done || sval != null
         case QUOTED:
            switch (ch) {
               case -1:
                  r = -1;
                  // fall through

               case '\n':
                  done = true;
                  v.push_back(sval);
                  break;

               case '\r':
                  break;

               case ',':
                  state = COMMA;
                  v.push_back(sval);
                  sval.resize(0);
                  break;

               case '"':
                  state = STRING;
                  sval += (char) ch;
                  break;

               default:
                  state = TOKEN;
                  sval += (char) ch;
                  break;
            }
            break;
      }
   }

   return r;
}
