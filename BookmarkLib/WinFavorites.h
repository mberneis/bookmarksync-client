/*
 * BookmarkLib/WinFavorites.h
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
#ifndef WindowsFavorites_H
#define WindowsFavorites_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include "BookmarkModel.h"

namespace syncit {

   class WinFavoritesOutput : public BookmarkSink, public BookmarkDifferences {

   public:
      WinFavoritesOutput(LPCTSTR pszDirectory, HWND hwndProgress = NULL);

      //////////////////
      // BookmarkSink...
      //
      virtual void progress();
      virtual void setName(const tchar_t *pszTitle);
      virtual void startBookmark();
      virtual void setBookmarkModified(const DateTime &dt);
      virtual void setBookmarkHref(const char *pszHref);
      virtual void endBookmark();
      virtual void startFolder();
      virtual void startSubscription();
      virtual void pushFolder();
      virtual void popFolder();
      virtual void endFolder();
      virtual void endSubscription();
      virtual void undoCurrent();
      // ...BookmarkSink
      //////////////////

      /////////////////////////
      // BookmarkDifferences...
      //
      virtual void addBookmark(const Bookmark *p);
      virtual void delBookmark(const Bookmark *p);

      virtual void pushFolder(const BookmarkFolder *pbf);
    //virtual void popFolder();

      virtual void add0(const BookmarkFolder *p);
      virtual void delFolder(const BookmarkFolder *p);

      virtual void commit();
      // ...BookmarkDifferences
      /////////////////////////

   private:
      void writeBookmark(const char *pszUrl, size_t cchUrl, const DateTime &dt);
      void del();

      void pushPath(const tchar_t *pszTitle);

      HWND m_hwndProgress;

      char m_achPath[MAX_PATH];
      int  m_i;

      string  m_href;
      DateTime m_dtModified, m_dtInvalid;
   };

}

#endif /* WindowsFavorites_H */
