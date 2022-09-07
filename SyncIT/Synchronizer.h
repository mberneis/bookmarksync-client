/*
 * SyncIT/Synchronizer.h
 * Copyright(c) 1999, SyncIt, Inc.
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
 * Last Modification: 2 Jan 1999
 *
 * Description:
 *    The Synchronizer is the main "object" of the SyncIT agent.
 *    There is only one Synchronizer: it maintains the main event loop
 *    and the set of Browser that are read in.
 */
#ifndef Synchronizer_H
#define Synchronizer_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */
#include <windows.h>
#include <windowsx.h>

#include "Browser.h"
#include "PopupMenu.h"
#include "StatusWindow.h"
#include "SettingsWindow.h"
#include "LoginWizard.h"
#include "ProxyDialog.h"

#include "BookmarkLib/BookmarkModel.h"

#include "SyncLib/PostOutputStream.h"
#include "SyncLib/CriticalSection.h"
#include "SyncLib/DateTime.h"
#include "SyncLib/URL.h"

class Synchronizer : private WizardListener, StatusListener, ProxyListener, SettingsListener {

#ifndef NDEBUG
public:
   bool isValid() const;
private:
   char m_achStartTag[sizeof("Synchronizer")];
#endif /* NDEBUG */

public:
   Synchronizer(HINSTANCE hInstance);
   ~Synchronizer();

   void addBrowser(Browser *pb);

   void start() /* throws Win32Error */;
   int driver();

   // wizard events:
   //
   virtual void onCancel();
   virtual void onConnect(LPCTSTR pszEmail, LPCTSTR pszPassword);
   virtual void onBack();

   // status events:
   //
   void onSyncNow();
   void onOrganize();
   void onShutdown();
   void setPopup();
   void resetPopup();
   void onShowSettings(int i);

   // settings events:
   //
   void onLogin(HWND hwnd);
   void toggleAutoSync();
   void setPopupMenuColumns(bool f);
   void showBookmarkPopup();
   virtual const URL &getRoot() const;

   // shared events
   //
   virtual void onHelp(HWND hwnd, const HELPINFO *phi, LPCTSTR pszSource);
   virtual void onHelp(HWND hwnd, LPCTSTR psz);
   virtual void showDocument(LPCTSTR pszDoc);

   LPCTSTR getEmail() const {
      return m_pszEmail;
   }

   long getSeqNo() const {
      return m_seqno;
   }

   void setSeqNo(long l) {
      m_seqno = l;
   }

   bool showMenuUrl(int id);

   enum AsyncResponse {
      Interrupted = -1,    // sent by TerminateThread

      OK = 0,              // success, bookmarks have changed
      NoChange,            // success, no bookmark changes

      UnknownHost,         // temporary errors
      ConnectError,
      SocketError,
      HttpError,
      ProxyAuthenticate,
      Wait,
      PrematureEOF,
      ProtocolError,
      ErrorMessage,

      UnknownEmail,        // permanent errors
      UnknownPassword,

      ResponseRange
   };

   AsyncResponse asyncRequest();

   /**
    * Submit the contents of the PostOutputStream object to
    * www.bookmarksync.com, through a proxy if necessary,
    * and store the resulting bookmarks into <i>m_pOnServer</i>
    */
   AsyncResponse submit(BookmarkContext *pc, PostOutputStream *preq);

   AsyncResponse submit0(BookmarkContext *pc,
                         PostOutputStream *preq);

protected:
   void stop();
   void synced();

   bool recover(RegKey &key);
   void init();
   void fileChange(Browser *pb);
   void response(AsyncResponse r, BookmarkModel *pbm);

   void setImages(BookmarkContext *pc);

private:
   enum States {
      State_START,
      State_CFG,
      State_LOGIN,
      State_INIT,
      State_RETRY,
      State_IDLE,
      State_WAIT,
      State_SYNC,
      State_SYNCW,
      State_STOP
   } m_state;

   DWORD m_dwExpire;

   LoginWizard m_wiz;
   StatusWindow m_status;
   SettingsWindow m_settings;
   ProxyDialog m_proxy;

   void onStartErr();
   void onStartOk(RegKey &key);
   void onThreadExit(AsyncResponse exitCode);
   void onTempErr(AsyncResponse error);
   void onPermErr(AsyncResponse error);
   void onSyncOk(AsyncResponse error);
   void onChange();
   void onTimer();
 //void onShutdown();

   void setIcon(DWORD dwMessage, int idi, LPCTSTR pszTip);
   void removeIcon();
   void spawn(States state);

   enum {
      MAX_BROWSERS = 6
   };

   Browser* m_apRegisteredBrowsers[MAX_BROWSERS];
   int      m_nRegisteredBrowsers;

   Browser* m_apBrowsers[MAX_BROWSERS];
   int      m_nBrowsers;

   HANDLE   m_aHandles[MAX_BROWSERS + 1];
   int      m_nHandles;

   void getBrowserBackupPath(const Browser *pb, char *pach, size_t cch) const;

   DateTime m_dtLastSynced;   // time of last sync

   // The critical section protects variables that are shared between
   // the two threads...
   //
   CriticalSection m_cs;

   BookmarkModel *m_pC;   // C -- current
   BookmarkModel *m_bookmarks;   // B -- backup

   BookmarkModel *m_pSubscriptions;

   struct Subscription {
      unsigned long m_id;
      long m_seqno;
   };

   void save();

   ///////////////////
   // Configuration...
   //
   LPTSTR m_pszRegKey;

   LPTSTR m_pszEmail;
   LPTSTR m_pszPWHash;

   long m_seqno;              // touched only by sync thread, or by main
                              //   thread when sync thread isn't running

   LPTSTR m_pszSavedEmail;    // NULL except when a bad password status is
                              //   returned by the server, to check whether
                              //   the email address is being changed during
                              //   a password change
   LPTSTR m_pszAuthorization;

   URL m_root;                // http://www.bookmarksync.com
   URL m_post;                // http://www.bookmarksync.com/client/syncit2.dll?

   unsigned long m_ulTimeIdle;
   unsigned long m_ulTimeSync;
   unsigned long m_ulTimeRetry;

   bool m_fAuto;

   bool m_fPopupMenuColumns;
   // ...configuration
   ///////////////////

   void getConfigVariables(RegKey &key);

   AsyncResponse parseResponse(BufferedInputStream *pin,
                               LPCTSTR pszHost,
                               BookmarkContext *pc);

   // disable copy constructor and assignment
   //
   Synchronizer(Synchronizer &out);
   Synchronizer &operator=(Synchronizer &out);

#ifndef NDEBUG
private:
   char m_achEndTag[sizeof("Synchronizer")];
#endif /* NDEBUG */
};

#endif /* Synchronizer_H */
