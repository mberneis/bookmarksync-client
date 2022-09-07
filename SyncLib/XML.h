/*
 * SyncLib/XML.h
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
#ifndef XML_H
#define XML_H

#include "text.h"
#include "util.h"

#include "Reader.h"
#include "Errors.h"
#include "PrintWriter.h"

namespace syncit {

class ParseError : public BaseError {
public:
   ParseError(const char *psz) : m_reason(psz) {
   }

   virtual size_t format(char *pach, size_t cch) {
      size_t n = m_reason.copy(pach, cch - 1);
      pach[n] = 0;
      return n;
   }

#if _MSC_VER < 1310
   virtual BaseError *newclone() const {
#else
   virtual ParseError *newclone() const {
#endif
        return NEW ParseError(*this);
    }

private:
   string m_reason;
};

class XMLParser {

public:
   bool parse(Reader &r);

   /*
    * alphabet:
    *    STARTING_TAG:  read the '<' and the tag name
    *    EMPTY_TAG:     handled all attributes, and read the closing '/>'
    *    START_TAG:     handled all attributes, and read the closing '>'
    *    END_TAG:       handled all text, and read the '</' tag name '>'
    *
    * STARTING_TAG -> (EMPTY_TAG -> STOP)
    *                 |
    *                 (START_TAG -> END_TAG -> STOP)
    * Called when an XML/HTML tag has been read.  The t parameter
    * indicates the state of the tag.
    * Take two examples:
    * 1. The XML "<p class=sample>text</p>"
    *    This causes xml to be called three times:
    *    1. xml(STARTING_TAG, L"p")  when "<p" has been read
    *    2. xml(START_TAG, L"p") when the closing '>' has been read
    *    3. xml(END_TAG, L"p") when "</p>" has been read
    * 2. The XML "<separator/>"
    *    This causes xml to be called twice:
    *    1. xml(STARTING_TAG, L"separator") when "<separator" has been read
    *    2. xml(EMPTY_TAG, L"separator") when the closing "/>" has been read
    *
    * @param t  one of STARTING_TAG, EMPTY_TAG, START_TAG, END_TAG
    * @param pszTagName  null-terminated string name of tag
    */

protected:
   enum TokenType {
      DECL,             /* <!                         */
      DECL_REST,        /* <!...>                     */

      XML_DECL,         /* <?xml                      */
      XML_DECL_NAME,    /* <?xml name=                */
      XML_DECL_VALUE,   /* <?xml name='value'         */

      PI,               /* <?Processing Instruction>  */
      COMMENT,          /* <!-- comment -->           */
      CHAR_DATA,        /* <tag>text between</tag>    */
      STARTING_TAG,     /* <tag                       */
      EMPTY_TAG,        /* <tag/>         psz == NULL */
      START_TAG,        /* <tag>          psz == NULL */
      END_TAG,          /* </tag>                     */
      ATTRIBUTE_NAME,   /* <tag attr...               */
      ATTRIBUTE_VALUE,  /* <tag attr='value'       OR
                           <tag attr      psz == NULL */
      ENTITY_REF,       /* &ref; can interrupt others */
      CDATA,            /* <![CDATA]...  ]]>          */

      SERVER_SCRIPT
   };

   virtual void xml(TokenType t, const tchar_t *psz, size_t cch,
                    bool fStart, bool fComplete) = 0;

   enum ContentType {
      CONTENT_XML,
      CONTENT_CDATA,
      CONTENT_LINE
   };

   void setContentType(ContentType t) {
      m_fContentType = t;
   }

   virtual int rdError(Reader &r, int ch, int chExpected);

protected:
   int rdDocument(Reader &r, int ch);

   /**
    * Read a name (production #5 in the XML spec).
    * Names are defined as:
    * <pre>
    * // names and tokens
    *  [4]  NameChar ::=  Letter | Digit | '.' | '-' | '_' | ':' | CombiningChar | Extender 
    *  [5]  Name ::=  (Letter | '_' | ':') (NameChar)* 
    *  [7]  Nmtoken ::=  (NameChar)+ 
    * </pre>
    *
    * @param r          reference(&) to Reader to parse
    * @param ch         Unicode character -- next character to read
    * @param t          TokenType of xml buffer
    *
    * @return the first character that didn't match the name, or -1 on EOF
    *
    * @exception SyntaxError if no name at all
    */
   int rdName(Reader &r, int ch, TokenType t);
   int rdName0(Reader &r, int ch, TokenType t);

   /**
    * Read an attribute value (production #10 in the XML spec).
    * Attribute values are defines as:
    * <pre>
    * [10]  AttValue ::=  '"' ([^<&"] | Reference)* '"'  
    *                  |  "'" ([^<&'] | Reference)* "'" 
    * </pre>
    *
    * @param r          reference(&) to Reader to parse
    * @param ch         Unicode character -- next character to read
    *
    * @return the first character that didn't match the name, or -1 on EOF
    */
   int rdAttValue(Reader &r, int ch, TokenType t);

   /**
    * Read an XML character reference (production #67 in XML spec)
    *
    * If it's a character reference (&#32;, &#x20;), the character is appended
    * to the xml buffer.  If it's a name reference (&quot;) then the xml
    * buffer is flushed, and xml(ENTITY_REF) is called with the name.
    *
    * @param r          reference(&) to Reader to parse
    * @param ch         Unicode character -- next character to read
    * @param t          TokenType of xml buffer
    *
    * @return the next character character after ';'
    * @exception ParseError if not terminated by ';'
    */
   int rdReference(Reader &r, int ch, TokenType t);

   int rdRest(Reader &r, int ch, TokenType t);

   /**
    * Read whitespace (production #2 in the XML spec)
    * Whitespace is defined as:
    * <pre>
    * // whitepace
    *  [3]  S ::=  (#x20 | #x9 | #xD | #xA)+ 
    *
    *    #x20 == ' '
    *    #x9  == '\t'
    *    #xD  == '\r'
    *    #xA  == '\n'
    * </pre>
    *
    * @param r          reference(&) to Reader to parse
    * @param ch         Unicode character -- next character to read
    *
    * @ensure result \notin { ' '  '\t'  '\r'  '\n' }
    *
    * @return the first character that didn't match the name, or -1 on EOF
    */
   int rdSpace(Reader &r, int ch);

   /**
    * Read an XML/HTML comment (Production #15 in the XML spec)
    * A comment is defined as:
    * <pre>
    * // comments
    * [15]  Comment ::=  '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->' 
    * </pre>
    *
    * @param r          reference(&) to Reader to parse
    * @param ch         Unicode character -- next character to read
    */
   int rdComment(Reader &r, int ch);

   /**
    * Read a stream of characters until a substring exactly
    * matching the specified pattern is found.
    * <p>
    * Uses the KMP (Knuth, Morris, and Pratt) algorithm, as the
    * Boyer-Moore algorithm is unwieldy for unicode.
    *
    * @param r          reference(&) to Reader to parse
    * @param ch         Unicode character -- next character to read
    * @param pszPattern null-terminated string of pattern to match
    * @param t          ContentType of xml callback for data read
    *
    * @return the character read after the pattern, or -1 on EOF
    */
   int rdUntil(Reader &r, int ch, const tchar_t *pszPattern,
               TokenType t);

   int rdUntil(Reader &r, int ch, tchar_t chPattern, TokenType t);

   /**
    * Read a string.  Must exactly match string.
    *
    * @param r          reference(&) to Reader to parse
    * @param ch         Unicode character -- next character to read
    * @param pszExpected pointer to null-terminated Unicode string -- must match input stream r
    *
    * @return next character read after pszExpected has been matched
    *
    * @exception ParseError if input stream doesn't match pszExpected
    */
   int rdString(Reader &r, int ch, const tchar_t *pszExpected);

   /**
    * Read a character.  Must match expected character.
    *
    * @param r          reference(&) to Reader to parse
    * @param ch         Unicode character -- next character to read
    * @param expected   Unicode character -- must match next character
    *
    * @return next character read
    *
    * @exception ParseError if ch != expected
    */
   int rdChar(Reader &r, int ch, tchar_t expected);

private:
   void appendBuf(TokenType t, int ch);
   void flushBuf(TokenType t, bool fComplete);
   void flushBuf0(TokenType t, bool fComplete);

   tchar_t m_achContent[4096];
   int     m_iContent;

   bool    m_fStart;

   ContentType m_fContentType;
};

void XMLWriteAttribute(syncit::PrintWriter &w, const tchar_t *pszAttrName, const tchar_t *value);

#ifdef TEXT16
void XMLWriteAttribute(syncit::PrintWriter &w, const tchar_t *pszAttrName, const char *value);
#endif

void XMLWriteContents(syncit::PrintWriter &w, const tchar_t *psz);

}

#endif /* XML_H */
