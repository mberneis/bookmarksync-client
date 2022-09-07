/*
 * BookmarkLib/XBELInput.cxx
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
 * Last Modification: 30 Apr 1999
 *
 * Description:
 *    Reads/write bookmark sets using the XBEL (XML Bookmark Exchange Language)
 *    file format.
 */
#pragma warning( disable : 4786 )

#include "BrowserBookmarks.h"

#include "SyncLib/BinarySearch.h"
#include "SyncLib/BufferedInputStream.h"
#include "SyncLib/FileInputStream.h"
#include "SyncLib/XML.h"
#include "SyncLib/BitmapFileImage.h"

using namespace syncit;

class XBELBookmarkParser : public XMLParser {

public:
   XBELBookmarkParser(BookmarkSink *pc) {
      m_pc = pc;
      m_fTagType = UnknownTag;
      m_fAttributeType = UnknownAttribute;
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

            case START_TAG:
               startTag();
               break;

            case END_TAG:
               endTag(identifyTag(psz));
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
   enum TagType {
      UnknownTag,

      XBEL,
      BOOKMARK,
      FOLDER,
      SEPARATOR,
      ALIAS,
      SUBSCRIPTION,

      DESC,
      TITLE
   };

   void startingTag(TagType t) {
      if (t != UnknownTag) {
         switch (t) {
            case BOOKMARK:
               endPrev();
               m_pc->startBookmark();
               break;

            case FOLDER:
               endPrev();
               m_pc->startFolder();
               break;

            case SUBSCRIPTION:
               endPrev();
               m_pc->startSubscription();
               break;

            case SEPARATOR:
               endPrev();
               m_pc->newSeparator();
               break;

            case XBEL:
               endPrev();
               break;

            case TITLE:
            case DESC:
               m_text.resize(0);
               break;
         }

         m_fTagType = t;
      }
   }

   void startTag() {
      if (m_fTagType == FOLDER || m_fTagType == XBEL || m_fTagType == SUBSCRIPTION) {
         m_pc->pushFolder();
         m_level++;
      }
   }

   void endTag(TagType t) {
      switch (t) {
         case TITLE:
            m_pc->setName(m_text.c_str());
            break;

         case DESC:
            m_pc->setDescription(m_text.c_str());
            break;

         case FOLDER:
            m_pc->popFolder();
            m_pc->endFolder();
            m_level--;
            break;

         case SUBSCRIPTION:
            m_pc->popFolder();
            m_pc->endSubscription();
            m_level--;
            break;

         case BOOKMARK:
            m_pc->endBookmark();
            break;

         case XBEL:
            m_pc->popFolder();
            m_level--;
            break;
      }
   }


   enum AttributeType {
      UnknownAttribute,

      HREF,
      ADDED,
      VISITED,
      MODIFIED,

      ID,
      REF,

      NEWITEMHEADER,
      MENUHEADER,
      FOLDED,

      SEQNO,
      OPENIMG,
      CLOSEDIMG
   };

   void attributeName(AttributeType t) {
      m_fAttributeType = t;
   }

   void attributeValue(const tchar_t *psz) {

      switch (m_fAttributeType) {
         case ADDED:
         case VISITED:
         case MODIFIED:
            {
               DateTime dt;

               dt.parseW3C(psz);

               switch (m_fAttributeType) {
                  case ADDED:
                     if (m_fTagType == BOOKMARK || m_fTagType == FOLDER || m_fTagType == XBEL || m_fTagType == SUBSCRIPTION) {
                        m_pc->setAdded(dt);
                     }
                     break;

                  case VISITED:
                     if (m_fTagType == BOOKMARK) {
                        m_pc->setBookmarkVisited(dt);
                     }
                     break;

                  case MODIFIED:
                     if (m_fTagType == BOOKMARK) {
                        m_pc->setBookmarkModified(dt);
                     }
                     break;

               }
            }
            break;

         case HREF:
            if (m_fTagType == BOOKMARK) {
               m_pc->setBookmarkHref(psz);
            }
            break;

         case ID:
            if (m_fTagType == BOOKMARK || m_fTagType == FOLDER || m_fTagType == SUBSCRIPTION) {
               m_pc->setId(psz);
            }
            break;

         case REF:
            if (m_fTagType == ALIAS) {
               m_pc->newAlias(psz);
            }
            break;

         case FOLDED:
            m_pc->setFolderFolded(psz == NULL || tstrcmp(psz, T("yes")) == 0);
            break;

         case MENUHEADER:
            if (m_fTagType == FOLDER || m_fTagType == XBEL || m_fTagType == SUBSCRIPTION) {
               m_pc->setMenuHeader();
            }
            break;

         case NEWITEMHEADER:
            if (m_fTagType == FOLDER || m_fTagType == XBEL || m_fTagType == SUBSCRIPTION) {
               m_pc->setNewItemHeader();
            }
            break;

         case SEQNO:
            if (m_fTagType == SUBSCRIPTION || m_fTagType == XBEL) {
               m_pc->setSubscriptionSeqNo(tstrtoul(psz, NULL, 10));
            }
            break;

         case OPENIMG:
         case CLOSEDIMG:
            if (m_fTagType == FOLDER || m_fTagType == XBEL || m_fTagType == SUBSCRIPTION || m_fTagType == BOOKMARK) {
               m_pc->setImages(m_fAttributeType == OPENIMG ? BookmarkItem::OPEN_IMAGE : BookmarkItem::CLOSED_IMAGE, psz);
            }
            break;
      }
   }

   void charData(const tchar_t *psz, size_t cch) {
      if (m_fTagType == DESC || m_fTagType == TITLE) {
         m_text.append(psz, psz + cch);      // start, end
      }
   }

   TagType identifyTag(const tchar_t *psz);
   AttributeType identifyAttribute(const tchar_t *psz);

   void endPrev() {
      m_text.resize(0);
   }

   static Token m_gaTags[];
   static Token m_gaAttributes[];

private:
   TagType m_fTagType;
   AttributeType m_fAttributeType;

   tstring m_text;

   BookmarkSink *m_pc;

   int m_level;
};

Token XBELBookmarkParser::m_gaTags[] = {
   // alphabetical order
   { T("alias"),        ALIAS },
   { T("bookmark"),     BOOKMARK },
   { T("desc"),         DESC },
   { T("folder"),       FOLDER },
   { T("separator"),    SEPARATOR },
   { T("subscription"), SUBSCRIPTION },
   { T("title"),        TITLE },
   { T("xbel"),         XBEL }
};

XBELBookmarkParser::TagType XBELBookmarkParser::identifyTag(const tchar_t *psz) {
   return (TagType) BinarySearch(psz, m_gaTags, ELEMENTS(m_gaTags), UnknownTag);
}

Token XBELBookmarkParser::m_gaAttributes[] = {
   // alphabetical order
   { T("added"),        ADDED },
   { T("closedimg"),    CLOSEDIMG },
   { T("folded"),       FOLDED },
   { T("href"),         HREF },
   { T("id"),           ID },
   { T("menuheader"),   MENUHEADER },
   { T("modified"),     MODIFIED },
   { T("newitemheader"),NEWITEMHEADER },
   { T("openimg"),      OPENIMG },
   { T("ref"),          REF },
   { T("seqno"),        SEQNO },
   { T("visited"),      VISITED }
};

XBELBookmarkParser::AttributeType XBELBookmarkParser::identifyAttribute(const tchar_t *psz) {
   return (AttributeType) BinarySearch(psz, m_gaAttributes, ELEMENTS(m_gaAttributes), UnknownAttribute);
}

bool XBELBookmarks::Read(LPCTSTR pszFilename, BookmarkSink *pbs) {
   FileInputStream f;

   if (f.open(pszFilename)) {
      BufferedInputStream b(&f);
      bool r = Read(b, pbs);
      b.close();
      return r;
   }
   else {
      return false;
   }
}

bool XBELBookmarks::Read(Reader &r, BookmarkSink *pc) {
   XBELBookmarkParser x(pc);

   if (x.parse(r)) {
      return x.isFinished();
   }
   else {
      return false;
   }
}
