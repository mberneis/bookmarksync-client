/*
 * BookmarkLib/NetscapeBookmarks.h
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
#ifndef NetscapeBookmarks_H
#define NetscapeBookmarks_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include "BookmarkModel.h"
#include "BrowserBookmarks.h"

#include "SyncLib/PrintWriter.h"

namespace syncit {

   class NetscapeBookmarks : public BrowserBookmarks {
   public:
      static  bool Read(LPCTSTR pszFilename, BookmarkSink *pbs);
      static  bool Read(Reader &in, BookmarkSink *pbs);
      static  void Write(const BookmarkModel *p, LPCTSTR pszFilename);
      static  void Write(const BookmarkModel *p, PrintWriter &w);

      static size_t GetDefaultFilename(TCHAR *pach, size_t cch);
   };

}

#endif /* NetscapeBookmarks_H */
