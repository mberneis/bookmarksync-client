/*
 * SyncLib/XML.cxx
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

#include "XML.h"
#include "Character.h"
#include "Util.h"
#include "PrintWriter.h"

using namespace syncit;

/**
 * Pulled from http://www.w3.org/REC/TR-CSS:
 * <pre>
 *  [1]  document ::=  prolog element Misc* 
 *
 * // any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.
 *  [2]  Char ::=   #x9 | #xA | #xD | [#x20-#xD7FF]
 *                | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
 *                  
 *
 * // whitepace
 *  [3]  S ::=  (#x20 | #x9 | #xD | #xA)+ 
 *
 * // names and tokens
 *  [4]  NameChar ::=  Letter | Digit | '.' | '-' | '_' | ':' | CombiningChar | Extender 
 *  [5]  Name ::=  (Letter | '_' | ':') (NameChar)* 
 *  [6]  Names ::=  Name (S Name)* 
 *  [7]  Nmtoken ::=  (NameChar)+ 
 *  [8]  Nmtokens ::=  Nmtoken (S Nmtoken)* 
 *
 * // literals
 *  [9]  EntityValue ::=  '"' ([^%&"] | PEReference | Reference)* '"'  
 *                     |  "'" ([^%&'] | PEReference | Reference)* "'" 
 * [10]  AttValue ::=  '"' ([^<&"] | Reference)* '"'  
 *                  |  "'" ([^<&'] | Reference)* "'" 
 * [11]  SystemLiteral ::=  ('"' [^"]* '"') | ("'" [^']* "'")  
 * [12]  PubidLiteral ::=  '"' PubidChar* '"' | "'" (PubidChar - "'")* "'" 
 * [13]  PubidChar ::=  #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%] 
 *
 * // character data
 * [14]  CharData ::=  [^<&]* - ([^<&]* ']]>' [^<&]*) 
 *
 * // comments
 * [15]  Comment ::=  '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->' 
 *
 * // processing instructions
 * [16]  PI ::=  '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>' 
 * [17]  PITarget ::=  Name - (('X' | 'x') ('M' | 'm') ('L' | 'l')) 
 *
 * // CDATA sections
 * [18]  CDSect ::=  CDStart CData CDEnd 
 * [19]  CDStart ::=  '<![CDATA[' 
 * [20]  CData ::=  (Char* - (Char* ']]>' Char*))  
 * [21]  CDEnd ::=  ']]>' 
 *
 * // Prolog 
 * [22]  prolog ::=  XMLDecl? Misc* (doctypedecl Misc*)? 
 * [23]  XMLDecl ::=  '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>' 
 * [24]  VersionInfo ::=  S 'version' Eq (' VersionNum ' | " VersionNum ") 
 * [25]  Eq ::=  S? '=' S? 
 * [26]  VersionNum ::=  ([a-zA-Z0-9_.:] | '-')+ 
 * [27]  Misc ::=  Comment | PI |  S 
 * 
 * // Document Type Definition 
 * [28]  doctypedecl ::=  '<!DOCTYPE' S Name (S ExternalID)? S? ('[' (markupdecl | PEReference | S)* ']' S?)? '>' [  VC: Root Element Type ] 
 * [29]  markupdecl ::=  elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment  [  VC: Proper Declaration/PE Nesting ] 
 *     [  WFC: PEs in Internal Subset ] 
 * 
 * 
 * // External Subset 
 * [30]  extSubset ::=  TextDecl? extSubsetDecl 
 * [31]  extSubsetDecl ::=  ( markupdecl | conditionalSect | PEReference | S )* 
 * 
 * // Standalone Document Declaration 
 * [32]  SDDecl ::=  S 'standalone' Eq (("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no') '"'))  [  VC: Standalone Document Declaration ] 
 * 
 * // Language Identification 
 * [33]  LanguageID ::=  Langcode ('-' Subcode)* 
 * [34]  Langcode ::=  ISO639Code |  IanaCode |  UserCode 
 * [35]  ISO639Code ::=  ([a-z] | [A-Z]) ([a-z] | [A-Z]) 
 * [36]  IanaCode ::=  ('i' | 'I') '-' ([a-z] | [A-Z])+ 
 * [37]  UserCode ::=  ('x' | 'X') '-' ([a-z] | [A-Z])+ 
 * [38]  Subcode ::=  ([a-z] | [A-Z])+ 
 * 
 * // Element 
 * [39]  element ::=  EmptyElemTag 
 *    | STag content ETag [  WFC: Element Type Match ] 
 *     [  VC: Element Valid ] 
 * 
 * // Start-tag 
 * [40]  STag ::=  '<' Name (S Attribute)* S? '>' [  WFC: Unique Att Spec ] 
 * [41]  Attribute ::=  Name Eq AttValue [  VC: Attribute Value Type ] 
 *     [  WFC: No External Entity References ] 
 *     [  WFC: No < in Attribute Values ] 
 * 
 * // End-tag 
 * [42]  ETag ::=  '</' Name S? '>' 
 * 
 * // Content of Elements 
 * [43]  content ::=  (element | CharData | Reference | CDSect | PI | Comment)* 
 * 
 * // Tags for Empty Elements 
 * [44]  EmptyElemTag ::=  '<' Name (S Attribute)* S? '/>' [  WFC: Unique Att Spec ] 
 * 
 * // Element Type Declaration 
 * [45]  elementdecl ::=  '<!ELEMENT' S Name S contentspec S? '>' [  VC: Unique Element Type Declaration ] 
 * [46]  contentspec ::=  'EMPTY' | 'ANY' | Mixed | children  
 * 
 * // Element-content Models 
 * [47]  children ::=  (choice | seq) ('?' | '*' | '+')? 
 * [48]  cp ::=  (Name | choice | seq) ('?' | '*' | '+')? 
 * [49]  choice ::=  '(' S? cp ( S? '|' S? cp )* S? ')' [  VC: Proper Group/PE Nesting ] 
 * [50]  seq ::=  '(' S? cp ( S? ',' S? cp )* S? ')' [  VC: Proper Group/PE Nesting ] 
 * 
 * // Mixed-content Declaration 
 * [51]  Mixed ::=  '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*'  
 *    | '(' S? '#PCDATA' S? ')'  [  VC: Proper Group/PE Nesting ] 
 *     [  VC: No Duplicate Types ] 
 * 
 * // Attribute-list Declaration 
 * [52]  AttlistDecl ::=  '<!ATTLIST' S Name AttDef* S? '>' 
 * [53]  AttDef ::=  S Name S AttType S DefaultDecl 
 * 
 * // Attribute Types 
 * [54]  AttType ::=  StringType | TokenizedType | EnumeratedType  
 * [55]  StringType ::=  'CDATA' 
 * [56]  TokenizedType ::=  'ID' [  VC: ID ] 
 *     [  VC: One ID per Element Type ] 
 *     [  VC: ID Attribute Default ] 
 *    | 'IDREF' [  VC: IDREF ] 
 *    | 'IDREFS' [  VC: IDREF ] 
 *    | 'ENTITY' [  VC: Entity Name ] 
 *    | 'ENTITIES' [  VC: Entity Name ] 
 *    | 'NMTOKEN' [  VC: Name Token ] 
 *    | 'NMTOKENS' [  VC: Name Token ] 
 * 
 * // Enumerated Attribute Types 
 * [57]  EnumeratedType ::=  NotationType | Enumeration  
 * [58]  NotationType ::=  'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')'  [  VC: Notation Attributes ] 
 * [59]  Enumeration ::=  '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')' [  VC: Enumeration ] 
 * 
 * // Attribute Defaults 
 * [60]  DefaultDecl ::=  '#REQUIRED' | '#IMPLIED'  
 *    | (('#FIXED' S)? AttValue) [  VC: Required Attribute ] 
 *     [  VC: Attribute Default Legal ] 
 *     [  WFC: No < in Attribute Values ] 
 *     [  VC: Fixed Attribute Default ] 
 * 
 * // Conditional Section 
 * [61]  conditionalSect ::=  includeSect | ignoreSect  
 * [62]  includeSect ::=  '<![' S? 'INCLUDE' S? '[' extSubsetDecl ']]>'  
 * [63]  ignoreSect ::=  '<![' S? 'IGNORE' S? '[' ignoreSectContents* ']]>' 
 * [64]  ignoreSectContents ::=  Ignore ('<![' ignoreSectContents ']]>' Ignore)* 
 * [65]  Ignore ::=  Char* - (Char* ('<![' | ']]>') Char*)  
 * 
 * // Character Reference 
 * [66]  CharRef ::=  '&#' [0-9]+ ';'  
 *    | '&#x' [0-9a-fA-F]+ ';' [  WFC: Legal Character ] 
 * 
 * // Entity Reference 
 * [67]  Reference ::=  EntityRef | CharRef 
 * [68]  EntityRef ::=  '&' Name ';' [  WFC: Entity Declared ] 
 *     [  VC: Entity Declared ] 
 *     [  WFC: Parsed Entity ] 
 *     [  WFC: No Recursion ] 
 * [69]  PEReference ::=  '%' Name ';' [  VC: Entity Declared ] 
 *     [  WFC: No Recursion ] 
 *     [  WFC: In DTD ] 
 * 
 * // Entity Declaration 
 * [70]  EntityDecl ::=  GEDecl | PEDecl 
 * [71]  GEDecl ::=  '<!ENTITY' S Name S EntityDef S? '>' 
 * [72]  PEDecl ::=  '<!ENTITY' S '%' S Name S PEDef S? '>' 
 * [73]  EntityDef ::=  EntityValue | (ExternalID NDataDecl?) 
 * [74]  PEDef ::=  EntityValue | ExternalID 
 * 
 * // External Entity Declaration 
 * [75]  ExternalID ::=  'SYSTEM' S SystemLiteral 
 *    | 'PUBLIC' S PubidLiteral S SystemLiteral  
 * [76]  NDataDecl ::=  S 'NDATA' S Name [  VC: Notation Declared ] 
 * 
 * // Text Declaration 
 * [77]  TextDecl ::=  '<?xml' VersionInfo? EncodingDecl S? '?>' 
 * 
 * // Well-Formed External Parsed Entity 
 * [78]  extParsedEnt ::=  TextDecl? content 
 * [79]  extPE ::=  TextDecl? extSubsetDecl 
 * 
 * // Encoding Declaration 
 * [80]  EncodingDecl ::=  S 'encoding' Eq ('"' EncName '"' |  "'" EncName "'" )  
 * [81]  EncName ::=  [A-Za-z] ([A-Za-z0-9._] | '-')* //  Encoding name contains only Latin characters
 * 
 * // Notation Declarations 
 * [82]  NotationDecl ::=  '<!NOTATION' S Name S (ExternalID |  PublicID) S? '>' 
 * [83]  PublicID ::=  'PUBLIC' S PubidLiteral  
 * </pre>
 * 
 * <H2><A NAME='sec-notation'>6. Notation</a></h2>
 * 
 * <P>The formal grammar of XML is given in this specification using a simple
 * Extended Backus-Naur Form (EBNF) notation.  Each rule in the grammar defines
 * one symbol, in the form
 * </p><table cellpadding='5' border='1' bgcolor='#80ffff' width='100%'><tr><td><code><font>symbol&nbsp;::=&nbsp;expression</font></code></td></tr></table><p></P>
 * <P>Symbols are written with an initial capital letter if they are
 * defined by a regular expression, or with an initial lower case letter 
 * otherwise.
 * Literal strings are quoted.
 * 
 * </P>
 * 
 * <P>Within the expression on the right-hand side of a rule, the following
 * expressions are used to match strings of one or more characters:
 * <DL>
 * 
 * <DT><B><CODE>#xN</CODE></B></DT>
 * <DD>where <CODE>N</CODE> is a hexadecimal integer, the
 * expression matches the character in ISO/IEC 10646 whose canonical
 * (UCS-4) 
 * code value, when interpreted as an unsigned binary number, has
 * the value indicated.  The number of leading zeros in the
 * <CODE>#xN</CODE> form is insignificant; the number of leading
 * zeros in the corresponding code value 
 * is governed by the character
 * encoding in use and is not significant for XML.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>[a-zA-Z]</CODE>, <CODE>[#xN-#xN]</CODE></B></DT>
 * <DD>matches any <A href='#dt-character'>character</A> 
 * with a value in the range(s) indicated (inclusive).</DD>
 * 
 * 
 * 
 * <DT><B><CODE>[^a-z]</CODE>, <CODE>[^#xN-#xN]</CODE></B></DT>
 * <DD>matches any <A href='#dt-character'>character</A> 
 * with a value <EM>outside</EM> the
 * range indicated.</DD>
 * 

 * 
 * <DT><B><CODE>[^abc]</CODE>, <CODE>[^#xN#xN#xN]</CODE></B></DT>
 * <DD>matches any <A href='#dt-character'>character</A>
 * with a value not among the characters given.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>"string"</CODE></B></DT>
 * <DD>matches a literal string <A href='#dt-match'>matching</A>
 * that given inside the double quotes.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>'string'</CODE></B></DT>
 * <DD>matches a literal string <A href='#dt-match'>matching</A>
 * that given inside the single quotes.</DD>
 * 
 * 
 * </DL>
 * 
 * These symbols may be combined to match more complex patterns as follows,
 * where <CODE>A</CODE> and <CODE>B</CODE> represent simple expressions:
 * <DL>
 * 
 * <DT><B>(<CODE>expression</CODE>)</B></DT>
 * <DD><CODE>expression</CODE> is treated as a unit 
 * and may be combined as described in this list.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>A?</CODE></B></DT>
 * <DD>matches <CODE>A</CODE> or nothing; optional <CODE>A</CODE>.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>A B</CODE></B></DT>
 * <DD>matches <CODE>A</CODE> followed by <CODE>B</CODE>.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>A | B</CODE></B></DT>
 * <DD>matches <CODE>A</CODE> or <CODE>B</CODE> but not both.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>A - B</CODE></B></DT>
 * <DD>matches any string that matches <CODE>A</CODE> but does not match
 * <CODE>B</CODE>.
 * </DD>
 * 
 * 
 * 
 * <DT><B><CODE>A+</CODE></B></DT>
 * <DD>matches one or more occurrences of <CODE>A</CODE>.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>A*</CODE></B></DT>
 * <DD>matches zero or more occurrences of <CODE>A</CODE>.</DD>
 * 
 * 
 * 
 * </DL>
 * 
 * Other notations used in the productions are:
 * <DL>
 * 
 * <DT><B><CODE>/* ... * /</CODE></B></DT>
 * <DD>comment.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>[ wfc: ... ]</CODE></B></DT>
 * <DD>well-formedness constraint; this identifies by name a 
 * constraint on 
 * <A href='#dt-wellformed'>well-formed</A> documents
 * associated with a production.</DD>
 * 
 * 
 * 
 * <DT><B><CODE>[ vc: ... ]</CODE></B></DT>
 * <DD>validity constraint; this identifies by name a constraint on
 * <A href='#dt-valid'>valid</A> documents associated with
 * a production.</DD>
 * 
 * 
 * </DL>
 */


/*
 * All rd...() methods follow the 'push character passing' style, where
 * the next character to read is passed as a parameter, and is returned
 * as the result code.
 *
 * @return true iff the document has been completely parsed to EOF
 */
bool XMLParser::parse(Reader &r) {
   int ch = r.read();

   m_fStart = true;
   m_iContent = 0;
   m_fContentType = CONTENT_XML;

   ch = rdDocument(r, ch);

   return ch == -1;
}

void XMLParser::flushBuf0(TokenType t, bool fComplete) {
   m_achContent[m_iContent] = 0;
   xml(t, m_achContent, m_iContent, m_fStart, fComplete);
   m_fStart = fComplete;
   m_iContent = 0;
}

void XMLParser::flushBuf(TokenType t, bool fComplete) {
   if (m_iContent > 0 || fComplete) {
      flushBuf0(t, fComplete);
   }
}

void XMLParser::appendBuf(TokenType t, int ch) {
   if (m_iContent == ELEMENTS(m_achContent) - 1) {
      flushBuf0(t, false);
   }

   if (ch != '\r') {
      m_achContent[m_iContent++] = ch;
   }
}

int XMLParser::rdDocument(Reader &r, int ch) {
   while (ch != -1) {
      // read: '<'
      //
      if (ch == '<') {
         flushBuf(CHAR_DATA, true);

         ch = r.read();

         if (ch == '!') {
            // read: '<!'
            //    '<!--' comment
            //    '<!DOCTYPE'  document type declaration
            //    '<!ELEMENT'  element type declaration
            //    '<!ATTLIST'  attribute-list declaration
            //    '<!ENTITY'   entity declaration
            //    '<!NOTATION' notation declaration
            //
            //    '<![CDATA['
            
            ch = r.read();

            switch (ch) {
               case '-':
                  ch = rdChar(r, r.read(), T('-'));
                  ch = rdUntil(r, ch, T("--"), COMMENT);
                  ch = rdSpace(r, ch);
                  ch = rdChar(r, ch, T('>'));
                  break;

               case '[':
                  /*0[18]  CDSect ::=  '<![CDATA[' (Char* - (Char* ']]>' Char*)) ']]>' */
                  ch = rdString(r, ch, T("[CDATA]"));
                  ch = rdUntil(r, ch, T("]]>"), CDATA);
                  break;

               default:
                  /*
                   * [28]  doctypedecl ::=  '<!DOCTYPE' S Name (S ExternalID)? S? ('[' (markupdecl | PEReference | S)* ']' S?)? '>' [  VC: Root Element Type ] 
                   * [45]  elementdecl ::=  '<!ELEMENT' S Name S contentspec S? '>' [  VC: Unique Element Type Declaration ] 
                   * [52]  AttlistDecl ::=  '<!ATTLIST' S Name AttDef* S? '>' 
                   * [71]  GEDecl ::=  '<!ENTITY' S Name S EntityDef S? '>' 
                   * [72]  PEDecl ::=  '<!ENTITY' S '%' S Name S PEDef S? '>' 
                   * [82]  NotationDecl ::=  '<!NOTATION' S Name S (ExternalID |  PublicID) S? '>' 
                   */
                  ch = rdName(r, ch, DECL);
                  ch = rdRest(r, ch, DECL_REST);
                  // ch == '>'
                  ch = rdChar(r, ch, T('>'));
                  break;
            }
         }

         else if (ch == '?') {
            // read: '<?'
            //   PI (processing instructions)
            // | XMLDecl (prolog)
            // | TextDecl
            ch = rdName0(r, r.read(), PI);
            if (m_fStart && tstricmp(m_achContent, T("xml")) == 0) {
               flushBuf(XML_DECL, true);

               ch = rdSpace(r, ch);

               while (ch != '?' && ch != -1) {
                  ch = rdName(r, ch, XML_DECL_NAME);
                  ch = rdSpace(r, ch);
                  ch = rdChar(r, ch, T('='));
                  ch = rdSpace(r, ch);
                  ch = rdAttValue(r, ch, XML_DECL_VALUE);
                  ch = rdSpace(r, ch);
               }

               ch = rdString(r, ch, T("?>"));
            }
            else {
               flushBuf(PI, true);

               ch = rdSpace(r, ch);
               ch = rdUntil(r, ch, T("?>"), PI);
            }
         }

         else if (ch == '/') {
            // read: '</'
            //    end tag
            //
            // [42] ETag ::= '</' Name S? '>'
            ch = rdName(r, r.read(), END_TAG);
            ch = rdSpace(r, ch);
            ch = rdChar(r, ch, T('>'));
         }

         else if (ch == '%') {
            ch = rdUntil(r, ch, T("%>"), SERVER_SCRIPT);
         }

         else {
            /*
             * // Start-tag 
             * [40]  STag ::=  '<' Name (S Attribute)* S? '>' [  WFC: Unique Att Spec ] 
             * [41]  Attribute ::=  Name Eq AttValue [  VC: Attribute Value Type ] 
             *     [  WFC: No External Entity References ] 
             *     [  WFC: No < in Attribute Values ] 
             * 
             * // Content of Elements 
             * [43]  content ::=  (element | CharData | Reference | CDSect | PI | Comment)* 
             * 
             * // Tags for Empty Elements 
             * [44]  EmptyElemTag ::=  '<' Name (S Attribute)* S? '/>' [  WFC: Unique Att Spec ] 
             */
            ch = rdName(r, ch, STARTING_TAG);
            ch = rdSpace(r, ch);

            while (ch != T('>') && ch != T('/') && ch != -1) {
               ch = rdName(r, ch, ATTRIBUTE_NAME);
               ch = rdSpace(r, ch);

               /* [25]  Eq ::=  S? '=' S? */

               if (ch == T('=')) {
                  ch = rdSpace(r, r.read());

                  /*
                   * [10]  AttValue ::=  '"' ([^<&"] | Reference)* '"'  
                   *                  |  "'" ([^<&'] | Reference)* "'" 
                   */
                  ch = rdAttValue(r, ch, ATTRIBUTE_VALUE);
                  ch = rdSpace(r, ch);
               }
               else {
                  xml(ATTRIBUTE_VALUE, NULL, 0, true, true);
               }
            }

            if (ch == '/') {
               ch = rdChar(r, r.read(), '>'); 
               xml(EMPTY_TAG, NULL, 0, true, true);
            }
            else if (ch == '>') {
               xml(START_TAG, NULL, 0, true, true);
               ch = r.read();

               if (m_fContentType == CONTENT_CDATA) {
                  ch = rdUntil(r, ch, T("</"), CHAR_DATA);
                  ch = rdName(r, ch, END_TAG);
                  ch = rdSpace(r, ch);
                  ch = rdChar(r, ch, T('>'));
                  m_fContentType = CONTENT_XML;
               }
               else if (m_fContentType == CONTENT_LINE) {
                  ch = rdUntil(r, ch, T('\n'), CHAR_DATA);
                  xml(END_TAG, NULL, 0, true, true);
                  m_fContentType = CONTENT_XML;
               }
            }
         }
      }
      else if (ch == '&') {
         ch = rdReference(r, ch, CHAR_DATA);
      }

      else {
         appendBuf(CHAR_DATA, ch);
         ch = r.read();
      }
   }

   flushBuf(CHAR_DATA, true);

   return ch;
}

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
 * @exception ParseError if no name at all
 */
int XMLParser::rdName(Reader &r, int ch, TokenType t) {
   ch = rdName0(r, ch, t);
   flushBuf(t, true);
   return ch;
}

inline bool isXmlNameChar(tchar_t ch) {
   switch (ch) {
      case T('.'):
      case T('-'):
      case T('_'):
      case T(':'):
         return true;

      default:
         CharType ct = Character::getCharType(ch);

         return (ct & XmlNameChar) != 0;
   }
}

int XMLParser::rdName0(Reader &r, int ch, TokenType t) {
   while (ch != -1 && !Character::isXmlLetter(ch) && ch != T('_') && ch != T(':')) {
      // ch != -1
      // !Character::isXmlLetter(ch)
      // ch != '_'
      // ch != ':'

      ch = r.read();
    //throw ParseError("Syntax error: expecting name");
   }

   if (ch != -1) {
      do {
         appendBuf(t, ch);
         ch = r.read();
      } while (ch != -1 && isXmlNameChar(ch));
   }

   m_achContent[m_iContent] = 0;

   return ch;
}

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
int XMLParser::rdAttValue(Reader &r, int ch, TokenType t) {
   int termch;
   bool quoted;

   if (ch == T('"') || ch == T('\'')) {
      termch = ch;
      ch = r.read();
      quoted = true;
   }
   else {
      termch = (t == XML_DECL_VALUE) ? T('?') : T('>');
      quoted = false;
   }

   if (m_fContentType == CONTENT_LINE) {
      // netscape hack
      // always double quoted, 
      while (ch != termch && (quoted || !Character::isSpace(ch)) && ch != -1) {
         if (ch == T('%')) {
            ch = r.read();

            if (ch == '2') {
               ch = r.read();

               if (ch == '2') {
                  appendBuf(t, '"');
               }
               else {
                  appendBuf(t, '%');
                  appendBuf(t, '2');
                  appendBuf(t, ch);
               }
            }
            else {
               appendBuf(t, '%');
               appendBuf(t, ch);
            }
         }
         else {
            appendBuf(t, ch);
         }

         ch = r.read();
      }
   }
   else {
      while (ch != termch && (quoted || !Character::isSpace(ch)) && ch != -1) {
         if (ch == T('&')) {
            ch = rdReference(r, ch, t);
         }
         else {
            appendBuf(t, ch);
            ch = r.read();
         }
      }
   }

   flushBuf(t, true);

   return quoted ? r.read() : ch;
}

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
int XMLParser::rdReference(Reader &r, int ch, TokenType t) {
   assert(ch == T('&'));

   tchar_t ach[66];
   int i = 0;

   /*
    *0[67]  Reference ::=  EntityRef | CharRef 
    *0[68]  EntityRef ::=  '&' Name ';' [  WFC: Entity Declared ] 
    *     [  VC: Entity Declared ] 
    *     [  WFC: Parsed Entity ] 
    *     [  WFC: No Recursion ] 
    *0[66]  CharRef ::=  '&#' [0-9]+ ';'  
    *    | '&#x' [0-9a-fA-F]+ ';' [  WFC: Legal Character ] 
    */
   do {
      ach[i++] = ch;
      ch = r.read();
   } while (i < 64 && ch != -1 && (ch == '#' || isXmlNameChar(ch)));

   if (ch == T(';')) {
      ach[i++] = ch;
      ach[i] = 0;

      ch = r.read();

      if (ach[1] == T('#')) {
         unsigned long ul;
         const tchar_t *pch1;
         tchar_t *pch2;
         int r;

         if (ach[2] == T('x') || ach[2] == T('X')) {
            pch1 = ach + 3;
            r = 16;
         }
         else {
            pch1 = ach + 2;
            r = 10;
         }

         ul = tstrtoul(pch1, &pch2, r);
         if (pch2 > pch1 && *pch2 == T(';') && 0 <= ul && ul < 65536) {
            appendBuf(t, (int) ul);
            return ch;
         }
      }
      else if (tstrcmp(ach, T("&lt;")) == 0) {
         appendBuf(t, T('<'));
         return ch;
      }
      else if (tstrcmp(ach, T("&gt;")) == 0) {
         appendBuf(t, T('>'));
         return ch;
      }
      else if (tstrcmp(ach, T("&amp;")) == 0) {
         appendBuf(t, T('&'));
         return ch;
      }
      else if (tstrcmp(ach, T("&apos;")) == 0) {
         appendBuf(t, T('\''));
         return ch;
      }
      else if (tstrcmp(ach, T("&quot;")) == 0) {
         appendBuf(t, T('"'));
         return ch;
      }
      else {
         flushBuf(t, false);

         for (int j = 1; j < i - 1; j++) {
            appendBuf(ENTITY_REF, ach[j]);
         }

         flushBuf(ENTITY_REF, true);
         return ch;
      }
   }

   for (int j = 0; j < i; j++) {
      appendBuf(t, ach[j]);
   }

   return ch;
}

int XMLParser::rdRest(Reader &r, int ch, TokenType t) {
   while (ch != T('>') && ch != -1) {
      appendBuf(t, ch);

      if (ch == '\'' || ch == '"') {
         int termch = ch;

         do {
            ch = r.read();
            appendBuf(t, ch);
         } while (ch != termch && ch != -1);
      }

      ch = r.read();
   }

   return ch;
}

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
int XMLParser::rdSpace(Reader &r, int ch) {
   while (Character::isSpace(ch)) {
      ch = r.read();
   }

   return ch;
}

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
int XMLParser::rdUntil(Reader &r, int ch, const tchar_t *pszPattern,
                       TokenType t) {
   int cchPattern = tstrlen(pszPattern);
   int* next = NEW int[cchPattern];

   assert(cchPattern > 1);

   next[0] = -1;
   next[1] = 0;

   for (int i = 3; i <= cchPattern; i++) {
      int j = next[i - 2] + 1;
      while (pszPattern[i - 2] != pszPattern[j - 1] && j > 0) {
         j = next[j - 1] + 1;
      }

      next[i - 1] = j;
   }

   // find the substring
   //
   {
      int j = 0;

      while (j < cchPattern && ch != -1) {
         if (pszPattern[j] == ch) {
            j++;

            ch = r.read();
         }
         else {
            for (int i = 0; i < j; i++) {
               appendBuf(t, pszPattern[i]);
            }

            j = next[j];
            if (j == -1) {
               j = 0;

               appendBuf(t, ch);
               ch = r.read();
            }
         }
      }
   }

   delete[] next;

   flushBuf(t, true);

   return ch;
}

int XMLParser::rdUntil(Reader &r, int ch, tchar_t chPattern, TokenType t) {
   while (ch != -1 && ch != chPattern) {
      appendBuf(t, ch);
      ch = r.read();
   }

   flushBuf(t, true);

   return ch;
}

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
int XMLParser::rdString(Reader &r, int ch, const tchar_t *pszExpected) {
   int iExpected = 0;
   int chExpected = pszExpected[iExpected++];

   while (ch == chExpected) {
      ch = r.read();
      chExpected = pszExpected[iExpected++];
   }

   if (chExpected) {
      throw ParseError("Syntax error");
   }

   return ch;
}

/**
 * Read a character.  Must match expected character.
 *
 * @param r          reference(&) to Reader to parse
 * @param ch         Unicode character -- next character to read
 * @param chExpected Unicode character -- must match next character
 *
 * @return next character read
 *
 * @exception ParseError if ch != chExpected
 */
int XMLParser::rdChar(Reader &r, int ch, tchar_t chExpected) {
   if (ch != chExpected) {
      rdError(r, ch, chExpected);
   }

   return r.read();
}

/* virtual */
int XMLParser::rdError(Reader &r, int ch, int chExpected) {
   if (chExpected == '>') {
      do {
         ch = r.read();
      } while (ch != -1 && ch != chExpected);

      // ch == -1 || ch == chExpected
   }
   else {
      throw ParseError("syntax error");
   }

   return ch;
}

void syncit::XMLWriteAttribute(PrintWriter &w, const char *pszAttrName, const tchar_t *value) {
   w.write(' ');
   w.print(pszAttrName);

   if (value != NULL) {
      w.write('=');

      // if there is no double quote (") in the value, then use it as delimiter,
      // otherwise use single quote (')
      //
      tchar_t delim = (tstrchr(value, T('"')) == NULL) ? T('"') : T('\'');

      w.write(delim);

      tchar_t ch = *value++;

      while (ch) {
         if (ch == delim) {
            w.print(ch == T('"') ? T("&quot;") : T("&apos;"));
         }
         else if (ch == '&') {
            w.print(T("&amp;"));
         }
         else {
            w.write(ch);
         }

         ch = *value++;
      }

      w.write(delim);
   }
}

#ifdef TEXT16
void syncit::XMLWriteAttribute(PrintWriter &w, const char *pszAttrName, const char *value) {
   if (value.size() > 0) {
      w.print(T(' '));
      w.print(pszAttrName);
      w.print(T('='));

      // if there is no double quote (") in the value, then use it as delimiter,
      // otherwise use single quote (')
      //
      char delim = (strchr(value, '"') == NULL) ? '"' : '\'';

      char ch = *value++;

      w.print(delim);

      while (ch) {
         if (ch == delim) {
            w.print(ch == '"' ? T("&quot;") : T("&apos;"));
         }
         else if (ch == '&') {
            w.print(T("&amp;"));
         }
         else {
            w.print(ch);
         }

         ch = *value++;
      }

      w.print(delim);
   }
}
#endif

void syncit::XMLWriteContents(PrintWriter &w, const tchar_t *psz) {
   tchar_t ch = *psz++;

   while (ch) {
      switch (ch) {
         case '<':
            w.print(T("&lt;"));
            break;

         case '&':
            w.print(T("&amp;"));
            break;

         case '\n':
            w.write(T('\r'));
            // fall through

         default:
            w.write(ch);
            break;
      }

      ch = *psz++;
   }
}
