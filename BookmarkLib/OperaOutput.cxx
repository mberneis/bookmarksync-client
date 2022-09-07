/*
 * BookmarkLib/OperaHotlist.cxx
 * Copyright(c) 1999, SyncIt.com  Inc.
 *
 * Author:            Terence Way
 * Last Modification: 30 Apr 1999
 *
 * Description:
 *    Defines ReadOperaHotlist and WriteOperaHotlist routines, which
 *    can read/write files in the Opera hotlist OPERA3.ADR file format.
 */
#pragma warning( disable : 4786 )

#include "SyncLib/DateTime.h"
#include "SyncLib/BufferedOutputStream.h"
#include "SyncLib/FileOutputStream.h"
#include "SyncLib/PrintWriter.h"
#include "SyncLib/RegKey.h"
#include "SyncLib/UTF8.h"

#include "BrowserBookmarks.h"

using namespace syncit;

static void WriteFolder(const BookmarkFolder *pbf, PrintWriter &w);
static void WriteFolderElements(const BookmarkFolder *pbf, PrintWriter &w);
static void WriteBookmark(const Bookmark *pb, PrintWriter &w);

/**
 * Write a BookmarkModel to a file in Opera Hotlist format.  See
 * ReadOperaHotlist for details of the Hotlist file format.
 * <p>
 * @param pb    pointer to BookmarkModel to write
 * @param pszFilename  filename to write to.
 * @exception IOException on any IO error
 */
void OperaHotlist::Write(const BookmarkModel *pb, LPCTSTR pszFilename) {
   FileOutputStream f;
   BufferedOutputStream b(&f);
   PrintWriter w(&b);

   f.create(pszFilename);
   w.print("Opera Hotlist version 2.0\r\n\r\n");

   WriteFolderElements(pb, w);

   w.close();
   f.commit();
}

/**
 * Write the contents of a BookmarkModel to an OutputStream in the
 * Opera Hotlist file format.  See ReadOperaHotlist for more details
 * of the Opera file format.
 *
 * @param pb    pointer to BookmarkModel to write
 * @param out   pointer to OutputStream to write to
 * @exception IOException on any write error
 */
void OperaHotlist::Write(const BookmarkModel *pb, PrintWriter &w) {
   WriteFolderElements(pb, w);
}

static void WriteFolder(const BookmarkFolder *pbf, PrintWriter &w) {
   w.print("#FOLDER\r\n\tNAME=");
   w.print(pbf->getName());
   w.print("\r\n\tCREATED=");
   w.print(pbf->getAdded().get_time_t());
   w.print("\r\n\tVISITED=0\r\n\tORDER=-1\r\n");

   if (!pbf->isFolded()) {
      w.print("\tEXPANDED=YES\r\n");
   }

   if (pbf->hasDescription()) {
      w.print("\tDESCRIPTION=");
      w.print(pbf->getDescription());
      w.print("\r\n");
   }

   w.print("\r\n");

   WriteFolderElements(pbf, w);
}

static void WriteFolderElements(const BookmarkFolder *pbf, PrintWriter &w) {
   BookmarkVector::const_iterator i = pbf->begin(), end = pbf->end();

   while (i != end) {
      const BookmarkObject *pbo = *i++;

      if (pbo->isFolder()) {
         WriteFolder((const BookmarkFolder *) pbo, w);
      }
      else if (pbo->isBookmark()) {
         WriteBookmark((const Bookmark *) pbo, w);
      }
   }

   w.print("-\r\n");
}

/**
 * Write a Bookmark in the Opera Hotlist format.
 * Writes out the following text:
 * <pre>
 * #URL
 * \tNAME={name}
 * \tURL={href}
 * \tCREATED={added}
 * \tVISITED={visited}
 * \tORDER=-1
 * \tDESCRIPTION={description}
 *
 * </pre>
 */
static void WriteBookmark(const Bookmark *pb, PrintWriter &w) {
   char ach[4096];
   size_t cch = pb->getHref().format(ach, sizeof(ach));

   w.print("#URL\r\n\tNAME=");
   w.print(pb->getName());
   w.print("\r\n\tURL=");
   w.write(ach, cch);
   w.print("\r\n\tCREATED=");
   w.print(pb->getAdded().get_time_t());
   w.print("\r\n\tVISITED=");
   w.print(pb->getVisited().get_time_t());
   w.print("\r\n\tORDER=-1\r\n");

   if (pb->hasDescription()) {
      w.print("\tDESCRIPTION=");
      w.print(pb->getDescription());
      w.print("\r\n");
   }

   w.print("\r\n");
}
