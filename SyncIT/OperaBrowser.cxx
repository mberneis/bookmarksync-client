/*
 * SyncIT/OperaBrowser.cxx
 * Copyright(c) 1999, SyncIt.com  Inc.
 *
 * Author:            Terence Way
 * Last Modification: 2 Jan 1999
 *
 * Description:
 *    This module contains the definitions to methods
 *    for the OperaBrowser class, a subclass
 *    of the Browser class.
 *
 *    The three basic things this module defines:
 *    1. Get the location of the user's bookmark file from
 *       the registry;
 *    2. FindFirstChangeNotification, to open a handle
 *       that is signalled when changes to the directory
 *       that contains the BOOKMARK.HTM are made;
 *    3. Read the bookmark file into a BookmarkFolder
 *       object.
 */
#include <cstdlib>

#include "SyncLib/util.h"
#include "SyncLib/FileInputStream.h"
#include "SyncLib/BufferedInputStream.h"
#include "SyncLib/Errors.h"

#include "BookmarkLib/BookmarkModel.h"
#include "BookmarkLib/BookmarkEditor.h"
#include "BookmarkLib/BrowserBookmarks.h"

#include "OperaBrowser.h"
#include "resource.h"
#include "BuiltinImages.h"

namespace syncit
{

//------------------------------------------------------------------------------
OperaBrowser::OperaBrowser() : 
    Browser(TEXT("opera")) 
{
   m_pszDirectory = NULL;
   m_pszBookmarks = NULL;
}

//------------------------------------------------------------------------------
OperaBrowser::~OperaBrowser() 
{
   u_free0(m_pszBookmarks);
   u_free0(m_pszDirectory);
}

//------------------------------------------------------------------------------
HANDLE OperaBrowser::FindFirstChangeNotification() 
{
   HANDLE h;

   h = ::FindFirstChangeNotification(m_pszDirectory,
                                     FALSE,                         // fSubDir
                                     FILE_NOTIFY_CHANGE_LAST_WRITE);// dwFlags

   return h;
}

//------------------------------------------------------------------------------
bool OperaBrowser::readBookmarks(BookmarkSink *pbs, bool fForce) 
{
   return OperaHotlist::Read(m_pszBookmarks, pbs);
}

//------------------------------------------------------------------------------
bool OperaBrowser::initialize() /* throws Error */ 
{

   bool result = false;
   char achBuf[MAX_PATH];

   if (OperaHotlist::GetDefaultFilename(achBuf, ELEMENTS(achBuf)) > 0) {
      m_pszBookmarks = stralloc(achBuf);

      int i = tstrlen(m_pszBookmarks) - 1;
      while (i > 0 && m_pszBookmarks[i] != TEXT('\\'))
         i--;

      // i == 0 || m_pszBookmarks[i] == '\\'
      //
      m_pszDirectory = tstrcalloc(m_pszBookmarks, i);
      result = true;
   }

   if (result) {
      TCHAR achBackup[1024];
      size_t cch = tstrlen(m_pszBookmarks);

      tstrcpy(achBackup, m_pszBookmarks);

      // create a safe copy (don't overwrite the previously saved version)
      // just in case
      //
      tstrcpy(achBackup + cch - 4, TEXT(".sav"));
      CopyFile(m_pszBookmarks, achBackup, TRUE);

      // create a safe copy (overwrite the previously saved version)
      // if this fails, then there prolly isn't a bookmark file
      //
      tstrcpy(achBackup + cch - 4, TEXT(".bck"));

      if (!CopyFile(m_pszBookmarks, achBackup, FALSE)) {
         result = false;
         u_free(m_pszDirectory);
         m_pszDirectory = NULL;

         u_free(m_pszBookmarks);
         m_pszBookmarks = NULL;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
tstring OperaBrowser::getDirectory() const 
{
   return m_pszDirectory;
}

//------------------------------------------------------------------------------
void OperaBrowser::writeBookmarks(const BookmarkModel *pbfNew,
                                  const char *pszBackupFilename)
{
   BookmarkModel opera;
   BookmarkContext bc(&opera, gpLoader);

   if (OperaHotlist::Read(m_pszBookmarks, &bc)) {
      BookmarkEditor e(&opera);
      int i = diff(pbfNew, m_bookmarks, &e);

      if (i > 0) {
         OperaHotlist::Write(&opera, m_pszBookmarks);
      }

      XBELBookmarks::Write(&opera, pszBackupFilename);
      m_bookmarks = NEW BookmarkModel(opera);
   }
   else {
      OperaHotlist::Write(pbfNew, m_pszBookmarks);

      XBELBookmarks::Write(pbfNew, pszBackupFilename);
      m_bookmarks = NEW BookmarkModel(*pbfNew);
   }
}

} // namespace syncit
