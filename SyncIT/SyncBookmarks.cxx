/*
 * SyncIT/SyncBookmarks.cxx
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
#pragma warning( disable : 4786 )

#include "ProductVersion.h"

#include "SyncLib/PrintWriter.h"

#include "BookmarkLib/BookmarkEditor.h"
#include "BookmarkLib/BrowserBookmarks.h"

#include "Synchronizer.h"
#include "BuiltinImages.h"

static const TCHAR FILE_SUFFIX[] = TEXT(".xml");

extern HINSTANCE ghResourceInstance;

/**
 * 
 * B -- the backup set
 * C -- the current set
 * REQ -- the collected instructions to be sent to the server
 * 
 * operations:
 *    recover() : boolean
 *    init()
 *    fileChange(pb : Browser)
 *    response(AsyncResponse r, BookmarkFolder *pbf)
 */
bool Synchronizer::recover(RegKey &key) {

   TCHAR achFilename[1024];

   ///////////////////////////
   // Recover subscriptions...
   //
   BookmarkObject::Detach(m_pSubscriptions);
   m_pSubscriptions = NEW BookmarkModel();

   BookmarkContext csub(m_pSubscriptions, gpLoader);

   setImages(&csub);

   GetConfigFilename(m_pszEmail, ".sub.xml", achFilename, ELEMENTS(achFilename) - 1);
   XBELBookmarks::Read(achFilename, &csub);
   //
   // ...subscriptions
   ///////////////////

   assert(m_bookmarks == NULL);

   BookmarkModel *pbm = NEW BookmarkModel();
   BookmarkContext c(pbm, gpLoader);

   setImages(&c);

   GetConfigFilename(m_pszEmail, FILE_SUFFIX, achFilename, ELEMENTS(achFilename) - 1);

   if (!XBELBookmarks::Read(achFilename, &c)) {
      BookmarkObject::Detach(pbm);

      return false;
   }

   assert(m_bookmarks == NULL);
   m_bookmarks = pbm;
   setSeqNo(pbm->getSeqNo());

   // with backup
   // pre:  B != NULL
   //       C == NULL
   //
   assert(m_pC == NULL);
   m_pC = NEW BookmarkModel(*m_bookmarks);

   // FOR each browser
   //
   for (int i = 0; i < m_nRegisteredBrowsers; i++) {
      Browser *pb = m_apRegisteredBrowsers[i];

      if (pb->isInitialized()) {
         BookmarkModel *newpbm = new BookmarkModel;
         BookmarkContext bc(newpbm, gpLoader);

         setImages(&bc);

         // read browser bookmarks
         //
         if (pb->readBookmarks(&bc, true)) {
            if (pb->hasDirectoryChanged(key)) {
               pb->setBackup(newpbm);
      
               BookmarkMerger merge(m_pC);

               diff(newpbm, m_pC, &merge);
            }
            else {
               char achBackup[MAX_PATH];

               BookmarkModel *pmbck = new BookmarkModel;
               BookmarkContext cbck(pmbck, gpLoader);

               getBrowserBackupPath(pb, achBackup, ELEMENTS(achBackup));

               if (XBELBookmarks::Read(achBackup, &cbck)) {
                  pb->setBackup(pmbck);

                  // diff against B, updating C
                  //

                  // only called when state is State_START, so there
                  // is no other thread
                  //
                  assert(m_bookmarks != NULL);
                  assert(m_pC != NULL);

                  // edit the current set:
                  BookmarkEditor edit(m_pC);

                  diff(newpbm, pmbck, &edit);
                  delete newpbm;
               }
               else {
                  pb->setBackup(newpbm);

                  BookmarkMerger merge(m_pC);

                  diff(newpbm, m_pC, &merge);

                  delete pmbck;
               }
            }
         }
         else {
            delete newpbm;
         }
      }
   }

   resetPopup();

   // post: B == backup bookmarks
   //       C == current bookmarks
   return true;
}

void Synchronizer::setPopupMenuColumns(bool f) {
   m_fPopupMenuColumns = f;
   resetPopup();
}

void Synchronizer::showBookmarkPopup() {
   m_status.showBookmarkPopup();
}

const URL &Synchronizer::getRoot() const {
   return m_root;
}

void Synchronizer::resetPopup() {
   m_status.setPopupMenuColumnSize(m_fPopupMenuColumns ? GetSystemMetrics(SM_CYSCREEN) / GetSystemMetrics(SM_CYMENU) : -1);
   setPopup();
}

void Synchronizer::setPopup() {
   m_status.setPopupMenuBookmarks(m_pC);

   assert(m_pSubscriptions == NULL || m_pSubscriptions->isValid());
   m_status.setPopupMenuSubscriptions(m_pSubscriptions);
}

void Synchronizer::init() {
   // with no backup/connect from wizard

   // pre:  B == NULL
   //       C == NULL
   //
   // merge bookmarks
   //
   BookmarkModel *pOnDisk = NEW BookmarkModel();

   for (int i = 0; i < m_nRegisteredBrowsers; i++) {
      Browser *pb = m_apRegisteredBrowsers[i];

      if (pb->isInitialized()) {
         BookmarkModel *pbm = new BookmarkModel;
         BookmarkContext bc(pbm, gpLoader);
         BookmarkMerger merger(pOnDisk);

         // merge with existing bookmarks
         setImages(&bc);

         pb->readBookmarks(&bc, true);
         pb->setBackup(pbm);

         diff(pbm, pOnDisk, &merger);
      }
   }

   m_cs.enter();
   m_pC = pOnDisk;
   resetPopup();
   m_cs.leave();

   // post: C == current bookmarks
   //       B == NULL
   //
}

void Synchronizer::getBrowserBackupPath(const Browser *pb, char *pach, size_t cch) const {
   char achExtension[16];

   wsprintf(achExtension, ".%s.xml", pb->getShortName().c_str());

   GetConfigFilename(getEmail(), achExtension, pach, cch);
}

/**
 * @param pb  the browser that "noticed" the file change
 *
 * pre:  C != NULL
 */
void Synchronizer::fileChange(Browser *pb) {
   assert(m_pC != NULL);

   m_cs.enter();

   try {
      BookmarkModel *pOnDisk = new BookmarkModel;

      try {
         BookmarkContext bc(pOnDisk, gpLoader);

         setImages(&bc);

         // read browser bookmarks
         //
         if (pb->readBookmarks(&bc, false)) {
            BookmarkEditor editor(m_pC);

            if (diff(pOnDisk, pb->getBackup(), &editor) > 0) {
               m_status.setPopupMenuBookmarks(m_pC);

               pb->setBackup(pOnDisk);
               pOnDisk = NULL;

               onChange();
            }
         }
      } catch (...) {
         delete pOnDisk;

         throw;
      }

      delete pOnDisk;
   } catch (...) {
      m_cs.leave();

      throw;
   }

   m_cs.leave();
}

static unsigned long count(const BookmarkFolder *pbf) {
   BookmarkVector::const_iterator i = pbf->begin(), end = pbf->end();

   unsigned long ulResult = 0;

   while (i != end) {
      if ((*i)->isFolder()) {
         ulResult += count((const BookmarkFolder *) (*i));
      }
      else if ((*i)->isBookmark()) {
         ulResult++;
      }

      i++;
   }

   return ulResult;
}

static BOOL APIENTRY MergeDlgProc(HWND hDlg,
                                  UINT wMsg,
                                  WPARAM wParam,
                                  LONG lParam)
{
   lParam;  // unreferenced

   switch (wMsg) {
      /*
       * Set the focus to the OK button.
       * <p>
       * In response to a WM_INITDIALOG message, the dialog box
       * procedure should return FALSE (zero) if it calls the SetFocus
       * function to set the focus to one of the controls in the
       * dialog box. Otherwise, it should return TRUE (nonzero),
       * in which case the system sets the focus to the first control
       * in the dialog box that can be given the focus. 
       */
      case WM_INITDIALOG:
         {
            TCHAR achFormat[512], achLabel[1024];

            LoadString(ghResourceInstance,
                       IDS_BOOKMARK_COUNT,
                       achFormat,
                       ELEMENTS(achFormat));
            wsprintf(achLabel, achFormat, lParam);
            SetDlgItemText(hDlg, IDC_BOOKMARK_COUNT, achLabel);
            Button_SetCheck(GetDlgItem(hDlg, IDC_MERGE), BST_CHECKED);
         }
         return TRUE;

      /*
       * Look for an ESC or RETURN event.
       */
      case WM_COMMAND:
         // WM_COMMAND
         // wNotifyCode = HIWORD(wParam); // notification code 
         // wID = LOWORD(wParam);         // item, control, or accelerator identifier 
         // hwndCtl = (HWND) lParam;      // handle of control
         //
         switch (LOWORD(wParam)) {
            case ID_HOMEPAGE:
            case IDOK:
               ::EndDialog(hDlg,    // hDlg -- handle to dialog box
                           Button_GetCheck(GetDlgItem(hDlg, IDC_MERGE)) == BST_CHECKED ? 1 : 2);   // nResult -- value to return
               return TRUE;
         }
         break;
   }

   // default handler
   //
   return FALSE;
}

void Synchronizer::response(AsyncResponse r, BookmarkModel *p) {

   unsigned long ulCount = count(p);

   bool fMerge;

   if (ulCount == 0) {
      fMerge = m_state == State_START ||
               m_state == State_CFG ||
               m_state == State_INIT ||
               m_state == State_LOGIN;
   }
   else if (m_state == State_CFG || m_state == State_LOGIN) {
      /* login wizard is up */

      if (m_pszSavedEmail != NULL && tstrcmp(m_pszSavedEmail, m_pszEmail) == 0) {
         fMerge = true;
      }
      else {
         /*
          * DialogBoxParam returns -1, 0 on error,
          *                        1 on merge, 2 on don't merge
          * we should merge if -1, 0, 1
          */
         fMerge = DialogBoxParam(ghResourceInstance,
                                 MAKEINTRESOURCE(IDD_MERGE),
                                 m_wiz.isVisible() ? m_wiz.getWindow() : m_status.getWindow(),
                                 MergeDlgProc,
                                 ulCount) != 2;

         if (!fMerge) {
            m_bookmarks = NEW BookmarkModel(*m_pC);
         }
      }
   }
   else {
      fMerge = m_state == State_START ||
               m_state == State_CFG ||
               m_state == State_INIT ||
               m_state == State_LOGIN;
   }

   u_free0(m_pszSavedEmail);
   m_pszSavedEmail = NULL;

   m_cs.enter();

   switch (r) {
      case OK:
         if (fMerge) {
            // server is giving us a set of bookmarks
            //
            // we'll merge these new bookmarks with our
            // current set

            assert(m_pC != NULL);
            assert(m_bookmarks == NULL);
            m_bookmarks = NEW BookmarkModel(*m_pC);

            BookmarkMerger edit(m_pC);

            diff(p, m_bookmarks, &edit);

            // any new bookmarks have now been merged
            // into m_pC
         }
         else {
            // if (r == OK and m_state == State_update)
            assert(m_bookmarks != NULL);

            BookmarkEditor edit(m_pC);

            diff(p, m_bookmarks, &edit);
         }

         m_status.setPopupMenuBookmarks(m_pC);

         BookmarkObject::Detach(m_bookmarks);

         m_bookmarks = p;
         save();
         break;

      case NoChange:
         BookmarkObject::Detach(p);
         BookmarkObject::Detach(m_bookmarks);

         m_bookmarks = NEW BookmarkModel(*m_pC);
         save();
         break;

      default:
         BookmarkObject::Detach(p);
   }

   m_cs.leave();
}

void Synchronizer::save() {
   int i;
   TCHAR achPathname[MAX_PATH + 1];

   GetConfigFilename(m_pszEmail, FILE_SUFFIX, achPathname, ELEMENTS(achPathname) - 1);
   m_bookmarks->setSeqNo(m_seqno);
   XBELBookmarks::Write(m_bookmarks, achPathname);

   GetConfigFilename(m_pszEmail, ".sub.xml", achPathname, ELEMENTS(achPathname) - 1);
   XBELBookmarks::Write(m_pSubscriptions, achPathname);

   for (i = 0; i < m_nRegisteredBrowsers; i++) {
      Browser *pb = m_apRegisteredBrowsers[i];

      if (pb->isInitialized()) {
         char achBackup[MAX_PATH];

         getBrowserBackupPath(pb, achBackup, ELEMENTS(achBackup));

         pb->writeBookmarks(m_pC, achBackup);
      }
   }
}
