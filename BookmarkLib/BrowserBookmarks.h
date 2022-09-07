/*
 * BookmarkLib/BrowserBookmarks.h
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
#ifndef BrowserBookmarks_H
#define BrowserBookmarks_H

#include "BookmarkModel.h"

#include "SyncLib/PrintWriter.h"
#include "SyncLib/InputStream.h"
#include "SyncLib/Reader.h"

namespace syncit {

   class BrowserBookmarks {
   };

   class XBELBookmarks : public BrowserBookmarks {
   public:
      static  bool Read(LPCTSTR pszFilename, BookmarkSink *pbs);
      static  bool Read(Reader &r, BookmarkSink *pbs);

      static  void Write(const BookmarkModel *pm, PrintWriter &w);
      static  void Write(const BookmarkModel *pm, LPCTSTR pszFilename);
   };

   class OperaHotlist : public BrowserBookmarks {
   public:
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
      static  bool Read(Reader &in, BookmarkSink *pbs);
      static  bool Read(LPCTSTR pszFilename, BookmarkSink *pbs);

      /**
       * Write a BookmarkModel to a file in Opera Hotlist format.  See
       * ReadOperaHotlist for details of the Hotlist file format.
       * <p>
       * @param pb    pointer to BookmarkModel to write
       * @param pszFilename  filename to write to.
       * @exception IOException on any IO error
       */
      static  void Write(const BookmarkModel *pb, LPCTSTR pszFilename);

      /**
       * Write the contents of a BookmarkModel to an OutputStream in the
       * Opera Hotlist file format.  See ReadOperaHotlist for more details
       * of the Opera file format.
       *
       * @param pb    pointer to BookmarkModel to write
       * @param out   pointer to OutputStream to write to
       * @exception IOException on any write error
       */
      static  void Write(const BookmarkModel *pb, PrintWriter &w);

      static size_t GetDefaultFilename(TCHAR *pach, size_t cch);
   };

   class WindowsFavorites : public BrowserBookmarks {
   public:
      static bool Read(LPCTSTR pszDirectory, BookmarkSink *pbs);
      static void Write(const BookmarkModel *pb, LPCTSTR pszFilename);

      static size_t GetDefaultDirectory(TCHAR *pach, size_t cch);

      static const char m_gachMap[20];
      static const char m_gszSuffix[];
      static const char m_gszInternetShortcut[];
   };

   class AolFavoritePlaces : public BrowserBookmarks {
   public:
      static  bool Read(LPCTSTR pszFilename, BookmarkSink *pbs);

      /**
       * Retrieve the registry settings for the current version of
       * AOL, specifically the installation directory.
       *
       * The result will be terminated by a '\\' backslash separator character.
       *
       * @param pachData  pointer to array of character to store the
       *                  directory name
       * @param cchData   size (in characters) of pachData
       *
       * @exception on any registry error
       *
       * @return 0 if key not found, length of directory string if found
       */
      static size_t GetDefaultDirectory(TCHAR *pachData, size_t cchData);

      /**
       * Given a profile filename, get its profile name.  Return true if
       * it's an AOL profile db and we can retrieve the profile name, false otherwise
       *
       * @param pszProfileFilename  filename of AOL db
       * @param pach   buffer to store profile name
       * @param cch    size of buffer
       *
       * @return number of records in the db, 0 if not a profile or no profile name
       */
      static unsigned GetProfileName(LPCTSTR pszProfileFilename,
                                     char *pach, size_t cch);


   };
}

#endif /* BrowserBookmarks_H */
