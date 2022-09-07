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
#include "SyncLib/BufferedInputStream.h"
#include "SyncLib/FileInputStream.h"
#include "SyncLib/RegKey.h"
#include "SyncLib/UTF8.h"

#include "BrowserBookmarks.h"

using namespace syncit;

/**
 * Read an Opera browser Hotlist file into a BookmarkSink.
 * <p>
 * Opera Hotlist files are formatted as text files, with one name/value
 * pair per line.  A line consisting of the text '#URL' marks the beginning
 * of a bookmark's name/value attribute pairs; a line consisting of the text
 * '#FOLDER' marks the beginning of a bookmark folder.  Name/value pairs
 * are prefixed by a tab, and may be one of the following:
 * <pre>
 *       URL=
 *       DESCRIPTION=
 *       NAME=
 *       CREATED=
 *       VISITED=
 *       EXPANDED=
 *       ORDER=
 * </pre>
 * <p>
 * A line consisting of the text '- ends the contents of a folder.
 *
 * @param pszFilename   file name to read
 * @param pbs           pointer to BookmarkSink where bookmark data
 *                      will be written
 * @return true if a complete bookmark file has been read, false
 *          if not
 * @exception IOException on any read error
 */
bool OperaHotlist::Read(LPCTSTR pszFilename, BookmarkSink *pbs) {
   FileInputStream f;

   if (f.open(pszFilename)) {
      BufferedInputStream in(&f);

      enum {
         TUnknown,
         TFolder,
         TBookmark
      } f = TUnknown;

      bool fExpanded = false;
      int  level = 0;

      tchar_t achLine[8192];

      pbs->pushFolder();

      while (in.readLine(achLine, ELEMENTS(achLine))) {
         if (achLine[0] == '#') {
            if (tstrcmp(achLine, T("#URL")) == 0) {
               pbs->startBookmark();
               f = TBookmark;
            }
            else if (tstrcmp(achLine, T("#FOLDER")) == 0) {
               pbs->startFolder();
               f = TFolder;
               fExpanded = false;
            }
         }
         else if (achLine[0] == '-') {
            if (level > 0) {
               pbs->popFolder();
               pbs->endFolder();
               level--;
            }

            f = TUnknown;
         }
         else {
            tchar_t *psz = achLine;

            while (isspace(*psz)) {
               psz++;
            }

            if (tstrncmp(psz, T("NAME="), 5) == 0) {
               pbs->setName(psz + 5);
            }
            else if (tstrncmp(psz, T("CREATED="), 8) == 0) {
               DateTime dt;

               dt.set_time_t(tstrtoul(psz + 8, NULL, 0));

               pbs->setAdded(dt);
            }
            else if (tstrncmp(psz, T("VISITED="), 8) == 0) {
               if (f == TBookmark) {
                  DateTime dt;

                  dt.set_time_t(tstrtoul(psz + 8, NULL, 0));

                  pbs->setBookmarkVisited(dt);
               }
            }
            else if (tstrncmp(psz, T("EXPANDED="), 9) == 0) {
               if (f == TFolder) {
                  if (tstrcmp(psz + 9, T("YES")) == 0) {
                     fExpanded = true;
                  }
                  else if (tstrcmp(psz + 9, T("NO")) == 0) {
                     fExpanded = false;
                  }
               }
            }
            else if (tstrncmp(psz, T("DESCRIPTION="), 12) == 0) {
               pbs->setDescription(psz + 12);
            }
            else if (tstrncmp(psz, T("URL="), 4) == 0) {
               if (f == TBookmark) {
#ifdef TEXT16
                  char achHref[4096];
                  utf8enc(psz + 4, achHref, sizeof(achHref));
                  pbs->setBookmarkHref(achHref);
#else
                  pbs->setBookmarkHref(psz + 4);
#endif /* TEXT16 */
               }
            }
            else if (*psz == 0) {
               if (f == TFolder) {
                  pbs->setFolderFolded(!fExpanded);
                  pbs->pushFolder();
                  level++;
               }
               else if (f == TBookmark) {
                  pbs->endBookmark();
               }
            }
         }
      }

      in.close();

      return level == 0;
   }
   else {
      return false;
   }
}
