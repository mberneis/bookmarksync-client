/*
 * BookmarkLib/NetscapeInput.cxx
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
#include "NetscapeBookmarks.h"

#include "SyncLib/Character.h"
#include "SyncLib/BinarySearch.h"
#include "SyncLib/BufferedInputStream.h"
#include "SyncLib/FileInputStream.h"
#include "SyncLib/XML.h"
#include "SyncLib/UTF8.h"
#include <vector>
#include <string>
#include <tchar.h>

using namespace syncit;

class NetscapeBookmarkParser : public XMLParser {

public:
   NetscapeBookmarkParser(BookmarkSink *pc) {
      m_pc = pc;
      m_fTagType = UnknownTag;
      m_fAttributeType = UnknownAttribute;
      m_fObjectType = UnknownObject;
      m_level = 0;
   }

   bool isFinished() const {
      return m_level == 0;
   }

protected:
   virtual void xml(TokenType t, const tchar_t *psz, size_t cch,
                    bool fStart, bool fComplete) {
      if (t == CHAR_DATA) {
         charData(psz, cch);
      }
      else if (fStart && fComplete) {
         switch (t) {
            case STARTING_TAG:
               startingTag(identifyTag(psz));
               break;

            case END_TAG:
               endTag(psz == NULL ? NullTag : identifyTag(psz));
               break;

            case ATTRIBUTE_NAME:
               attributeName(identifyAttribute(psz));
               break;

            case ATTRIBUTE_VALUE:
               attributeValue(psz);
               break;
         }
      }

   }

private:
   enum ObjectType {
      UnknownObject,
      BookmarkType,
      FolderType,
      SeparatorType,
      AliasType
   };

   enum TagType {
      UnknownTag,
      NullTag,

      HR,
      BR,
      A,
      DL,
      DT,
      DD,
      H1,
      H3,
      TITLE,
      META
   };

   void startingTag(TagType t) {
      switch (t) {
         case UnknownTag:
            return;

         case DL:
            endPrev();
            m_pc->pushFolder();
            m_level++;
            break;

         case BR:
            m_text += L'\n';
            break;

         case A:
            endPrev();
            m_pc->startBookmark();
            m_fObjectType = BookmarkType;
            setContentType(CONTENT_LINE);
            break;

         case H1:
            endPrev();
            m_fObjectType = FolderType;
            setContentType(CONTENT_LINE);
            break;

         case H3:
            endPrev();
            m_fObjectType = FolderType;
            m_pc->startFolder();
            m_pc->setFolderFolded(false);
            setContentType(CONTENT_LINE);
            break;

         case TITLE:
            setContentType(CONTENT_CDATA);
            break;

         case DT:
            endPrev();
            break;

         case HR:
            endPrev();
            m_fObjectType = SeparatorType;
            m_pc->newSeparator();
            break;

         case DD:
            m_text.resize(0);
            break;

         case META:
            endPrev();
            break;
      }

      m_fTagType = t;
   }

   void endTag(TagType t) {
      if (t == DL) {
         endPrev();
         if (m_level > 0) {
            m_level--;
         }

         if (m_level > 0) {
            m_pc->popFolder();
            m_pc->endFolder();
         }
         m_fTagType = UnknownTag;
      }
      else if (m_fTagType == A || m_fTagType == H1 || m_fTagType == H3) {
         const char *ps = m_text.c_str();
         const char *pe = ps + m_text.length() - 1;

         while (pe > ps && isspace(*pe)) {
            pe--;
         }

         if (strnicmp(pe - 3, "</a>", 4) == 0) {
            m_text.resize(pe - ps - 3);
         }
         else if (strnicmp(pe - 4, "</h1>", 5) == 0 || strnicmp(pe - 4, "</h3>", 5) == 0) {
            m_text.resize(pe - ps - 4);
         }



         if (m_fObjectType != AliasType) m_pc->setName(m_text.c_str());
      }
   }

   enum AttributeType {
      UnknownAttribute,

      HREF,
      ADD_DATE,
      LAST_VISIT,
      LAST_MODIFIED,

      ALIASID,
      ALIASOF,

      NEWITEMHEADER,
      MENUHEADER,
      FOLDED
   };

   void attributeName(AttributeType t) {
      m_fAttributeType = t;
   }

   void attributeValue(const tchar_t *psz) {

      switch (m_fAttributeType) {
         case ADD_DATE:
         case LAST_VISIT:
         case LAST_MODIFIED:
            {
               DateTime dt;

               dt.set_time_t((time_t) tstrtoul(psz, NULL, 10));

               switch (m_fAttributeType) {
                  case ADD_DATE:
                     if (m_fTagType == A) {
                        if (m_fObjectType != AliasType) m_pc->setAdded(dt);
                     }
                     else if (m_fTagType == H3 || m_fTagType == H1) {
                        m_pc->setAdded(dt);
                     }
                     break;

                  case LAST_VISIT:
                     if (m_fTagType == A) {
                        if (m_fObjectType != AliasType) m_pc->setBookmarkVisited(dt);
                     }
                     break;

                  case LAST_MODIFIED:
                     if (m_fTagType == A) {
                        if (m_fObjectType != AliasType) m_pc->setBookmarkModified(dt);
                     }
                     break;

               }
            }
            break;

         case HREF:
            if (m_fTagType == A) {
               char achHref[4096];    // utf-8 encodes up to max 3 bytes
   #ifdef TEXT16

               utf8enc(psz, achHref, sizeof(achHref));

   #else
               int i = 0, j = 0;
               for (;;) {
                  char ch = psz[i++];
                  achHref[j++] = ch;

                  if (ch == 0) {
                     break;
                  }
                  else if (ch == '&' && strnicmp(psz + i, "amp;", 4) == 0) {
                     i += 4;
                  }
               }
   #endif

               m_pc->setBookmarkHref(achHref);
            }
            break;

         case ALIASID:
            if (m_fTagType == A) {
               m_pc->setId(psz);
            }
            break;

         case ALIASOF:
            m_fObjectType = AliasType;
            m_pc->undoCurrent();
            m_pc->newAlias(psz);
            break;

         case FOLDED:
            if (m_fTagType == H1 || m_fTagType == H3) {
               m_pc->setFolderFolded(true);
            }
            break;

         case MENUHEADER:
            if (m_fTagType == H1 || m_fTagType == H3) {
               m_pc->setMenuHeader();
            }
            break;

         case NEWITEMHEADER:
            if (m_fTagType == H1 || m_fTagType == H3) {
               m_pc->setNewItemHeader();
            }
            break;
      }
   }


   void charData(const tchar_t *psz, size_t cch) {
      switch (m_fTagType) {
         case A:     // anchor: this text is the name(title) of a bookmark
         case H1:    // heading: this text is the folder name(title) of the entire bookmark set
         case H3:    // heading: this text is the folder name(title) of a new folder

         case DD:    // description: this text is description of the last bookmark/folder
         case BR:    // line break:  this text is description of the last bookmark/folder
            {
               tstring cdata;
               for (const tchar_t *p = psz, *max = psz + cch; p < max; p++) {
                  tchar_t ch = *p;
                  if (ch != '\n') {
                     cdata += ch;
                  }
               }

               size_t pos;
               while ((pos = cdata.find("&amp;")) != tstring::npos)
               {
                   cdata.replace(pos, 5, "&");
               }
               while ((pos = cdata.find("&lt;")) != tstring::npos)
               {
                   cdata.replace(pos, 4, "<");
               }
               while ((pos = cdata.find("&gt;")) != tstring::npos)
               {
                   cdata.replace(pos, 4, ">");
               }
               while ((pos = cdata.find("&quot;")) != tstring::npos)
               {
                   cdata.replace(pos, 6, "\"");
               }

               m_text.append(cdata);
            }
            break;
      }
   }


   TagType identifyTag(const tchar_t *psz);
   AttributeType identifyAttribute(const tchar_t *psz);

   void endPrev() {
      if (m_fObjectType != AliasType && (m_fTagType == DD || m_fTagType == BR)) {
         tchar_t *p = (tchar_t *) m_text.c_str();
         int i = m_text.size() - 1;

         while (i >= 0 && (p[i] == ' ' || p[i] == '\t')) {
            p[i] = 0;
            i--;
         }

         // i < 0 || (p[i] != ' ' && p[i] != '\t')
         m_pc->setDescription(p);
      }

      m_text.resize(0);

      if (m_fObjectType == BookmarkType) {
         m_pc->endBookmark();
      }

      m_fObjectType = UnknownObject;
   }


   static Token m_gaTags[];
   static Token m_gaAttributes[];

private:
   TagType m_fTagType;
   AttributeType m_fAttributeType;
   ObjectType m_fObjectType;

   tstring m_text;

   int m_level;

   BookmarkSink  *m_pc;
};

bool NetscapeBookmarks::Read(LPCTSTR pszFilename, BookmarkSink *pbs) {
   FileInputStream f;

   if (f.open(pszFilename)) {
      BufferedInputStream b(&f);

      return Read(b, pbs);
   }
   else {
      return false;
   }
}

bool NetscapeBookmarks::Read(Reader &in, BookmarkSink *pbs) {
   NetscapeBookmarkParser x(pbs);

   if (x.parse(in)) {
      return x.isFinished();
   }
   else {
      return false;
   }
}

Token NetscapeBookmarkParser::m_gaTags[] = {
   { T("A"),     A },
   { T("BR"),    BR },
   { T("DD"),    DD },
   { T("DL"),    DL },
   { T("DT"),    DT },
   { T("H1"),    H1 },
   { T("H3"),    H3 },
   { T("HR"),    HR },
   { T("META"),  META },
   { T("TITLE"), TITLE }
};

Token NetscapeBookmarkParser::m_gaAttributes[] = {
   { T("ADD_DATE"),       ADD_DATE },
   { T("ALIASID"),        ALIASID },
   { T("ALIASOF"),        ALIASOF },
   { T("FOLDED"),         FOLDED },
   { T("HREF"),           HREF },
   { T("LAST_MODIFIED"),  LAST_MODIFIED },
   { T("LAST_VISIT"),     LAST_VISIT },
   { T("MENUHEADER"),     MENUHEADER },
   { T("NEWITEMHEADER"),  NEWITEMHEADER }
};

NetscapeBookmarkParser::TagType NetscapeBookmarkParser::identifyTag(const tchar_t *psz) {
   tchar_t achUpper[6];

   for (int i = 0; i < 5 && psz[i]; i++) {
      achUpper[i] = Character::toUpper(psz[i]);
   }

   achUpper[i] = L'\0';

   return (TagType) BinarySearch(achUpper, m_gaTags, ELEMENTS(m_gaTags), UnknownTag);
}

NetscapeBookmarkParser::AttributeType NetscapeBookmarkParser::identifyAttribute(const tchar_t *psz) {
   tchar_t achUpper[14];

   for (int i = 0; i < 13 && psz[i]; i++) {
      achUpper[i] = Character::toUpper(psz[i]);
   }

   achUpper[i] = L'\0';

   return (AttributeType) BinarySearch(achUpper, m_gaAttributes, ELEMENTS(m_gaAttributes), UnknownAttribute);
}
