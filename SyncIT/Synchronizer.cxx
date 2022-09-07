/*
 * SyncIT/Synchronizer.cxx
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
 * Last Modification: 2 Jan 1999
 *
 * Description:
 *    The Synchronizer is the main "object" of the SyncIT agent.
 *    There is only one Synchronizer: it maintains the main event loop
 *    and the set of Browser that are read in.
 *
 *    There is a precise state machine that a Synchronizer must follow,
 *    it is documented right before the onStart() method.
 *
 *    Here is the basic logic:
 *       There are two threads: the main WinMain thread, always running,
 *       and an initial/update thread, which is sporadically started.
 *       The initial thread is used only once, when starting.  It does
 *       the initial log on, getting the set of bookmarks from the server.
 *       The update is started soon after, and is restarted 5 seconds after
 *       the latest file change on one of the browser bookmarks.
 *
 *       Synchronizer is started:
 *          all bookmarks from all browsers are read in, and merged.
 *          the merged set is written out to each browser.
 *          file change notification is set up, so we'll be notified the moment
 *          one of the browsers updates the bookmarks.
 *          the initial thread is created.
 *       Initial thread:
 *          set empty post request,
 *          get server's bookmarks as response.
 *          on error, retry every ten seconds.
 *          on success, get main thread to start update thread
 *
 *       Update thread:
 *          set post request, containing set of commands to update
 *          server's stuff with our stuff.
 *          get server's bookmarks as response.
 *          on error, retry in five minutes? or on next file change
 *          on success, get main thread to write new bookmarks and update
 *          popup menu.
 *
 *    When the main thread writes out the new bookmarks, the file change will
 *    be signalled, the new bookmarks will be read, and the update thread will
 *    start again.  This time, however, there is no difference between the onDisk
 *    and onServer set, so the update will exit without doing a post.
 */
#pragma warning( disable : 4786 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include "ProductVersion.h"
#include "SyncLib/Errors.h"
#include "SyncLib/util.h"
#include "SyncLib/Base64.h"

#include "MicrosoftBrowser.h"
#include "BuiltinImages.h"
#include "Synchronizer.h"
#include "About.h"
#include "ProxyDialog.h"
#include "resource.h"

#include "BookmarkLib/BookmarkEditor.h"
#include "BookmarkLib/BrowserBookmarks.h"

extern "C" {
   #define PROTOTYPES 1
   #include "MD5/global.h"
   #include "MD5/md5.h"
}

using namespace syncit;

static const TCHAR REG_EMAIL[]    = TEXT("LastEmail");
static const TCHAR REG_PASS[]     = TEXT("LastPassword");
static const TCHAR REG_MD5 []     = TEXT("LastMD5");
static const TCHAR REG_SYNCED[]   = TEXT("LastSynced");
static const TCHAR KEY_SYNCIT[]   = TEXT("Software\\SyncIT\\BookmarkSync");


static const long INITIAL_SEQNO = -1;

static DWORD WINAPI asyncThreadProc(LPVOID pParameter);

// variables which can be overridden by Registry values
//
const char URL_ROOT[] = "http://post.syncit.com";
const char URL_FILE[] = "client/syncit3.dll?";

static const DWORD TIME_INFINITE = MAXLONG;
static const DWORD TIME_RESET = 60 * 60 * 1000;    // 1 hour, time to reset last synced time
static const DWORD TIME_IDLE  = 45 * 60 * 1000;    // 45 minutes
static const DWORD TIME_RETRY =      10 * 1000;    // 10 seconds
static const DWORD TIME_SYNC  =       5 * 1000;    //  5 seconds


static char *md5alloc(const char *pszEmail, const char *pszPlainText);

Synchronizer::Synchronizer(HINSTANCE hInstance) : m_status(hInstance), m_wiz(hInstance), m_root(URL_ROOT), m_post(m_root, URL_FILE) {

#ifndef NDEBUG
   strcpy(m_achStartTag, TEXT("Synchronizer"));
   strcpy(m_achEndTag, TEXT("Synchronizer"));
#endif /* NDEBUG */

   char achBuf[512];

   LoadStringA(ghResourceInstance, IDS_REG_KEY, achBuf, ELEMENTS(achBuf));

   m_pszRegKey = stralloc(achBuf);

   m_wiz.addWizardListener(this);
   m_status.addStatusListener(this);
   m_proxy.addProxyListener(this);
   m_settings.addSettingsListener(this);
 //m_settings.init(m_status.getWindow(), this);

   // START:
   //    expire == infinite
   //    nHandles == 0
   //    nBrowsers == 0
   //    wizard == not-visible && cursor == arrow
   //    icon == null
   //

   m_dwExpire = ::GetTickCount() + TIME_RESET;
   m_state = State_START;
   m_nHandles = 0;
   m_nBrowsers = 0;

   m_nRegisteredBrowsers = 0;

   m_pszEmail = m_pszPWHash  = NULL;
   m_seqno = -1;
   m_pszSavedEmail = NULL;

   m_pC = m_bookmarks = NULL;
   m_pSubscriptions = NEW BookmarkModel();

   m_ulTimeIdle = TIME_IDLE;
   m_ulTimeSync = TIME_SYNC;
   m_ulTimeRetry = TIME_RETRY;
   m_fAuto = true;

   assert(isValid());
}

Synchronizer::~Synchronizer() {
   assert(isValid());

   u_free(m_pszRegKey);

   u_free0(m_pszEmail);
   u_free0(m_pszPWHash);
   u_free0(m_pszSavedEmail);

   m_status.setPopupMenuBookmarks(NULL);
   m_status.setPopupMenuSubscriptions(NULL);

   BookmarkObject::Detach(m_pC);
   BookmarkObject::Detach(m_bookmarks);
   BookmarkObject::Detach(m_pSubscriptions);

#ifndef NDEBUG
   m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */

}

#ifndef NDEBUG
bool Synchronizer::isValid() const {
   if (strcmp(m_achStartTag, "Synchronizer") != 0 ||
       strcmp(m_achEndTag, "Synchronizer") != 0 ||
       m_nRegisteredBrowsers > ELEMENTS(m_apRegisteredBrowsers))
      return false;

   switch (m_state) {
      case State_START:
         // START:
         //    expire == infinite
         //    nHandles == 0
         //    nBrowsers == 0
         //    wizard == not-visible && cursor == arrow
         //    icon == null
         //
         if (m_nHandles != 0 ||
             m_nBrowsers != 0 ||
             m_wiz.isVisible())
            return false;
         break;

      case State_CFG:
         // CFG:
         //    expire == infinite
         //    nHandles == 0
         //    nBrowsers == 0
         //    wizard == visible
         //    cursor == arrow
         //    icon == null
         //
         if (m_nHandles != 0 ||
             m_nBrowsers != 0 ||
             !m_wiz.isVisible())
            return false;
         break;

      case State_LOGIN:
         // LOGIN:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    wizard == visible
         //    cursor == hour
         //    icon == null
         //
         if (m_nHandles != m_nBrowsers + 1 ||
             m_nBrowsers == 0 ||
             !m_wiz.isVisible())
            return false;
         break;

      case State_INIT:
         // INIT:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         if (m_nHandles != m_nBrowsers + 1 ||
             m_nBrowsers == 0)
            return false;
         break;

      case State_RETRY:
         // RETRY:
         //    expire <= now + 10 seconds
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == stale
         //
         if (m_nHandles != m_nBrowsers ||
             m_nBrowsers == 0)
            return false;
         break;

      case State_IDLE:
         // IDLE:
         //    expire <= now + 30 minutes
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == live
         //
         if (m_nHandles != m_nBrowsers ||
             m_nBrowsers == 0)
            return false;
         break;

      case State_WAIT:
         // WAIT:
         //    expire <= now + 5 seconds
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == stale
         //
         if (m_nHandles != m_nBrowsers ||
             m_nBrowsers == 0)
            return false;
         break;

      case State_SYNC:
         // SYNC:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         if (m_nHandles != m_nBrowsers + 1 ||
             m_nBrowsers == 0)
            return false;
         break;

      case State_SYNCW:
         // SYNCW:
         //    expire <= now + 5 seconds
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //    
         if (m_nHandles != m_nBrowsers + 1 ||
             m_nBrowsers == 0)
            return false;
         break;

      case State_STOP:
         if (m_nHandles != 0 ||
             m_nBrowsers != 0)
            return false;
         break;

      default:
         return false;
   }

   return true;
}
#endif /* NDEBUG */

void Synchronizer::addBrowser(Browser *pb) {
   assert(isValid());
   assert(m_nRegisteredBrowsers < ELEMENTS(m_apRegisteredBrowsers));

   if (pb->initialize())
   {
       m_apRegisteredBrowsers[m_nRegisteredBrowsers++] = pb;
   }

   assert(isValid());
}

#ifndef NDEBUG
// debug checking to make sure certain operations
// are only performed in the main thread.
//
static DWORD gThreadId = -1;
#endif /* NDEBUG */

int Synchronizer::driver() /* throws Win32Error */ {
   bool fDone = false;

   MSG msg;

   assert(gThreadId == GetCurrentThreadId());

   while (!fDone) {
      switch (m_state) {
         case State_START:
         case State_CFG:
         case State_STOP:
            if (GetMessage(&msg, NULL, 0, 0)) {
               if (!m_wiz.isDialogMessage(&msg) &&
                   !m_settings.isDialogMessage(&msg)) {
                  TranslateMessage(&msg);
                  DispatchMessage(&msg);
               }
            }
            else {
               fDone = true;
            }
            break;

         default:
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
               if (WM_QUIT == msg.message) {
                  fDone = true;
               }
               else if (!m_wiz.isDialogMessage(&msg) &&
                        !m_settings.isDialogMessage(&msg)) {
                  TranslateMessage(&msg);
                  DispatchMessage(&msg);
               }
            }
            else {
               DWORD dw;
               long lTimeout = (long) (m_dwExpire - ::GetTickCount());

               assert(m_nHandles > 0);
               assert(isValid());
               dw = ::MsgWaitForMultipleObjects(m_nHandles,    // nCount
                                                m_aHandles,    // pahHandles
                                                FALSE,         // fWaitAll
                                                lTimeout < 0 ? 0 : lTimeout,  // dwMillis
                                                QS_ALLINPUT |
                                                  QS_POSTMESSAGE |
                                                  QS_TIMER);
               assert(m_nHandles > 0);
               assert(isValid());

               switch (dw) {
                  case WAIT_FAILED:
                     {
                        DWORD dwError = GetLastError();
                        stop();
                        throw Win32Error("MsgWaitForMultipleObjects", dwError);
                     }
                     break;

                  case WAIT_TIMEOUT:
                     onTimer();
                     break;

                  default:
                     {
                        int i = dw - WAIT_OBJECT_0;

                        if (m_nHandles > m_nBrowsers && i == m_nHandles - 1) {
                           DWORD dwExitCode;
                           
                           /* there's one more handle than the file changes,
                            * that's the thread */
                           if (!GetExitCodeThread(m_aHandles[i], &dwExitCode)) {
                              DWORD dwError = GetLastError();
                              stop();
                              throw Win32Error("GetExitCodeThread", dwError);
                           }

                           onThreadExit((AsyncResponse) dwExitCode);
                        }
                        else if (0 <= i && i < m_nHandles) {
                           // one of the files have changed
                           if (!FindNextChangeNotification(m_aHandles[i])) {
                              DWORD dwError = GetLastError();
                              stop();
                              throw FileError(FileError::Access, m_apBrowsers[i]->getDirectory().c_str(), Win32Error("FindNextChangeNotification", dwError));
                           }

                           fileChange(m_apBrowsers[i]);
                        }
                        else if (i == m_nHandles) {
                           // break, we'll get the message from
                           // the queue the next time around

                        }
                        else {
                           TCHAR ach[1024];
                           assert(m_nHandles > 0);
                           wsprintf(ach, "dw = %08X, m_nHandles = %d, m_nBrowsers = %d, state = %d", dw,
                                    m_nHandles, m_nBrowsers, m_state);
                           MessageBox(NULL, ach, NULL, 0);
                           assert(WAIT_OBJECT_0 <= dw && dw <= WAIT_OBJECT_0 + m_nHandles);
                        }
                     }
               }
            }
      }
   }

   switch (m_state) {
      case State_INIT:
      case State_SYNC:
      case State_SYNCW:
         // fall through

      case State_RETRY:
      case State_IDLE:
      case State_WAIT:
         m_status.removeIcon();
         break;
   }

   return msg.wParam;
}

void Synchronizer::start() /* throws Win32Error */ {

   char achBuf[256];

   assert(isValid());

#ifndef NDEBUG
   gThreadId = GetCurrentThreadId();
#endif /* NDEBUG */

   // HKEY_CURRENT_USER\\Software\\SyncIT\\BookmarkSync
   //
   RegKey key;

   key.create(HKEY_CURRENT_USER, m_pszRegKey);

   // key is valid
   getConfigVariables(key);

   // Get the last synced time
   // the last synced time is stored as a FILETIME or
   // unsigned __int64
   //
   m_dtLastSynced = key.queryDateTime(REG_SYNCED);

   if (m_dtLastSynced.isValid()) {
      TCHAR achDateTime[1024];

      m_dtLastSynced.format(achDateTime, ELEMENTS(achDateTime));

      m_status.setLastSynced(achDateTime);
   }

   assert(m_pszEmail == NULL);
   m_pszEmail = key.queryValue(REG_EMAIL, achBuf, sizeof(achBuf)) > 0 ? stralloc(achBuf) : NULL;

   assert(m_pszPWHash == NULL);

   if (key.queryValue(REG_PASS, achBuf, sizeof(achBuf)) > 0) {
      // convert old plaintext password into MD5 hash
      m_pszPWHash = md5alloc(m_pszEmail, achBuf);
   }
   else if (key.queryValue(REG_MD5, achBuf, sizeof(achBuf)) > 0) {
      m_pszPWHash = stralloc(achBuf);
   }
   else {
      m_pszPWHash = NULL;
   }

   // TTW016 new: we keep the directory where each different
   // browser stores its bookmarks in our registry.  Should
   // there be a change, we cannot "recover" but we must merge
   // all bookmarks again.
   //
   bool fDirectoryChanged = false;
   int i;

   if (m_pszEmail == NULL || m_pszPWHash == NULL) {
      for (i = 0; i < m_nRegisteredBrowsers; i++) {
         if (m_apRegisteredBrowsers[i]->hasDirectoryChanged(key))
            fDirectoryChanged = true;
      }

      onStartErr();
   }
   else {
      onStartOk(key);
   }

   assert(isValid());
}

void Synchronizer::getConfigVariables(RegKey &key) {
   // Pull out the Root registry variable, controls
   // where our web-site is...
   // defaults to "http://www.bookmarksync.com/"
   //
   char buf[4096];

   m_root = key.queryValue("Root", buf, sizeof(buf)) > 0 ? buf : URL_ROOT;
   m_post = URL(m_root, key.queryValue("File3", buf, sizeof(buf)) > 0 ? buf : URL_FILE);

   m_fAuto = key.queryDWORD("AutoSync", (DWORD) m_fAuto) ? true : false;

   m_status.setAutoSyncCheck(m_fAuto);

   m_ulTimeIdle = key.queryDWORD("IdleTime", TIME_IDLE);
   m_ulTimeSync = key.queryDWORD("SyncTime", TIME_SYNC);
   m_ulTimeRetry = key.queryDWORD("RetryTime", TIME_RETRY);

   m_fPopupMenuColumns = key.queryDWORD("PopupMenuColumns", FALSE) != FALSE;
}

void Synchronizer::toggleAutoSync() {
   RegKey key;

   m_fAuto = !m_fAuto;

   if (key.open(HKEY_CURRENT_USER, m_pszRegKey, true)) {
      key.setValue("AutoSync", m_fAuto ? TRUE : FALSE);
   }

   m_status.setAutoSyncCheck(m_fAuto);
}

// The Synchronizer State Machine
// ------------------------------
//
// States:
//    START -- the initial state.  No activity, and no way of
//             getting back to this state once left.
//
//    CFG --   configuration.  The wizard dialog box is up, there
//             is no activity.
//
//    LOGIN -- The wizard dialog box is up, an initial sync is
//             in the process of being sent up.
//
//    INIT --  an initial sync is in the process of being sent up,
//             but there is no wizard dialog box up.
//
//    RETRY -- the initial sync failed due to a temporary error.
//             the initial sync will be retried.
//
//    IDLE --  everything is synced up, waiting for a file change
//
//    WAIT --  a file has changed, waiting for more changes, or for
//             some timeout to start resyncing.
//
//    SYNC --  delta synchronization in progress.
//
//    STOP --  the final state.  No activity, and no way of leaving
//             this state.
//
//
// Events:
//    startErr -- no initial configuration on startup, must use login
//                wizard
//
//    startOk0 -- a valid initial configuration during startup, but directories
//                have changed, or can't find recovery file, must do initial sync
//
//    startOk1 -- a valid initial configuration, found recovery file, can
//                do differential sync
//
//    tempErr --  synchronizing failed due to a temporary error
//
//    permErr --  synchronizing failed due to a permanent error
//                (permanent in the sense of requiring user intervention
//                before retrying)
//
//    syncOk --   synchronizing succeeded
//
//    connect --  the wizard is at the point where starting the connection
//                makes sense
//
//    timer --    
//
//    stop --     the wizard dialog box cancel button (or close) was
//                pressed, or shutdown syncit was selected.
//
//
//
// There are also some variables that are modified by the state machine:
//    now -- the current time
//    expire -- the time till next timer expiration
//    nHandles -- number of handles waiting for
//    nBrowsers -- number of browsers, also the number of file changes
//    wizard -- the wizard dialog box (visible, not-visible)
//    cursor -- arrow | wait
//    icon -- system tray icon (null, live, stale)
//
// START:
//    expire == infinite
//    nHandles == 0
//    nBrowsers == 0
//    wizard == not-visible && cursor == arrow
//    icon == null
//
// CFG:
//    expire == infinite
//    nHandles == 0
//    nBrowsers == 0
//    wizard == visible
//    cursor == arrow
//    icon == null
//
// LOGIN:
//    expire == infinite
//    nHandles == nBrowsers + 1
//    nBrowsers > 0
//    wizard == visible
//    cursor == hour
//    icon == null
//
// INIT:
//    expire == infinite
//    nHandles == nBrowsers + 1
//    nBrowsers > 0
//    icon == stale
//
// RETRY:
//    expire <= now + 10 seconds
//    nHandles == nBrowsers
//    nBrowsers > 0
//    icon == stale
//
// IDLE:
//    expire <= now + 30 minutes
//    nHandles == nBrowsers
//    nBrowsers > 0
//    icon == live
//
// WAIT:
//    expire <= now + 5 seconds
//    nHandles == nBrowsers
//    nBrowsers > 0
//    icon == stale
//
// SYNC:
//    expire == infinite
//    nHandles == nBrowsers + 1
//    nBrowsers > 0
//    icon == stale
//
// SYNCW:
//    expire <= now + 5 seconds
//    nHandles == nBrowsers + 1
//    nBrowsers > 0
//    icon == stale
//    
// Here's the transition table:
//
//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
// startErr go CFG      X        X        X        X        X        X        X        X
// startOk0 go RETRY    X        X        X        X        X        X        X        X
// startOk1 go WAIT     X        X        X        X        X        X        X        X
//  tempErr    X        X     go CFG   go RETRY    X        X        X     go WAIT  go WAIT
//  permErr    X        X     go CFG   go CFG      X        X        X     go CFG   go CFG
//   syncOk    X        X     go SYNC  go SYNC     X        X        X     go IDLE  go WAIT
//  connect    X     go LOGIN    -     go LOGIN go LOGIN go LOGIN go LOGIN go LOGIN go LOGIN
//     back    X        -     go CFG   go CFG   go CFG   go CFG   go CFG   go CFG   go CFG
//  syncnow    X        -        -        -     go INIT  go SYNC  go SYNC  go SYNCW go SYNCW
//   change    X        X        -        -        -     go WAIT  go WAIT  go SYNCW go SYNCW
//    timer    X        X        X        X     go INIT  go SYNC  go SYNC     X     go SYNCW
//   cancel    X     go STOP  go STOP     -        -        -        -        -        -
// shutdown    X     go STOP  go STOP  go STOP  go STOP  go STOP  go STOP  go STOP  go STOP

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
// startErr go CFG      X        X        X        X        X        X        X        X
//
void Synchronizer::onStartErr() {
   assert(isValid() && m_state == State_START);

   // START:
   //    expire == infinite
   //    nHandles == 0
   //    nBrowsers == 0
   //    wizard == not-visible && cursor == arrow
   //    icon == null
   //
   assert(m_nHandles == 0);
   assert(m_nBrowsers == 0);
   assert(!m_wiz.isVisible());

   m_wiz.create(m_status.getWindow(), m_pszEmail);

   m_state = State_CFG;

   // CFG:
   //    expire == infinite
   //    nHandles == 0
   //    nBrowsers == 0
   //    wizard == visible
   //    cursor == arrow
   //    icon == null
   //

   assert(isValid() && m_state == State_CFG);
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
// startOk0 go RETRY    X        X        X        X        X        X        X        X
// startOk1 go WAIT     X        X        X        X        X        X        X        X
//
void Synchronizer::onStartOk(RegKey &key) /* throws Win32Error */ {
   assert(isValid() && m_state == State_START);

   // START:
   //    expire == infinite
   //    nHandles == 0
   //    nBrowsers == 0
   //    wizard == not-visible && cursor == arrow
   //    icon == null
   //
   assert(m_nHandles == 0);
   assert(m_nBrowsers == 0);
   assert(!m_wiz.isVisible());

   m_status.setEmail(m_pszEmail);
   m_settings.setEmail(m_pszEmail);

//
   assert(m_bookmarks == NULL);

   if (recover(key)) {
      m_dwExpire = ::GetTickCount() + (m_fAuto ? m_ulTimeSync : TIME_INFINITE);
      // WAIT:
      //    expire <= now + 5 seconds
      //    nHandles == nBrowsers
      //    nBrowsers > 0
      //    icon == stale
      //
      m_status.addIcon(IDI_STALE, IDS_TIP_UPLOAD);
      m_status.setState(IDS_STATE_DOWNLOAD);
      m_state = State_WAIT;
   }
   else {
      init();

      // use an old token initially
      m_seqno = INITIAL_SEQNO;

      // RETRY:
      //    expire <= now + 10 seconds
      //    nHandles == nBrowsers
      //    nBrowsers > 0
      //    icon == stale
      //
      m_dwExpire = ::GetTickCount() + (m_fAuto ? m_ulTimeRetry : TIME_INFINITE);
      m_status.addIcon(IDI_STALE, IDS_TIP_DOWNLOAD);
      m_status.setState(IDS_STATE_DOWNLOAD);

      m_state = State_RETRY;
   }

   // START:
   //    expire == infinite
   //    nHandles == 0
   //    nBrowsers == 0
   //    wizard == not-visible && cursor == arrow
   //    icon == null
   //
   assert(m_nHandles == 0);
   assert(m_nBrowsers == 0);
   for (int i = 0; i < m_nRegisteredBrowsers; i++) {
      Browser *pb = m_apRegisteredBrowsers[i];

      if (pb->isInitialized()) {
         HANDLE h = pb->FindFirstChangeNotification();

         if (h != INVALID_HANDLE_VALUE) {
            m_aHandles[m_nHandles++] = h;
            m_apBrowsers[m_nBrowsers++] = pb;
         }
      }
   }

   assert(m_nHandles == m_nBrowsers);

   if (m_nHandles == 0) {
      m_state = State_STOP;

      throw Win32Error("FindFirstChangeNotification");
   }

   assert(isValid());
}

void Synchronizer::onThreadExit(AsyncResponse exitCode) {
   assert(gThreadId == GetCurrentThreadId());

   switch (exitCode) {
      case Interrupted:
         break;

      case OK:
      case NoChange:
         onSyncOk(exitCode);
         break;

      case UnknownHost:
      case ConnectError:
      case SocketError:
      case HttpError:
      case Wait:
      case ProxyAuthenticate:
      case PrematureEOF:
      case ProtocolError:
      case ErrorMessage:
         onTempErr(exitCode);
         break;

      case UnknownEmail:
      case UnknownPassword:
         onPermErr(exitCode);
         break;
   }
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//  tempErr    X        X     go CFG   go RETRY    X        X        X     go WAIT  go WAIT
//
void Synchronizer::onTempErr(AsyncResponse error) {
   assert(isValid());

   assert(gThreadId == GetCurrentThreadId());

   switch (m_state) {
      case State_LOGIN:

         // LOGIN:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    wizard == visible
         //    cursor == hour
         //    icon == null
         //
         assert(m_nBrowsers > 0);
         assert(m_nHandles == m_nBrowsers + 1);
         assert(m_wiz.isVisible());

         CloseHandle(m_aHandles[--m_nHandles]);

         stop();

         m_state = State_CFG;
         m_wiz.bad(LoginWizard::Panel_Ready, m_status.getStatusWindow());

         // CFG:
         //    expire == infinite
         //    nHandles == 0
         //    nBrowsers == 0
         //    wizard == visible
         //    cursor == arrow
         //    icon == null
         //
         assert(m_state == State_CFG);
         break;

      case State_INIT:
         // INIT:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         assert(m_nBrowsers > 0);
         assert(m_nHandles == m_nBrowsers + 1);

         ::CloseHandle(m_aHandles[--m_nHandles]);

         if (!m_fAuto && error == HttpError) {
            m_dwExpire = ::GetTickCount() + TIME_INFINITE;
         }
         else {
            m_dwExpire = ::GetTickCount() + m_ulTimeRetry;
         }

         m_status.modifyIcon(IDI_STALE, IDS_TIP_TEMPERR);
         m_status.setState(IDS_STATE_TEMPERR);

         m_state = State_RETRY;

         // RETRY:
         //    expire <= now + 10 seconds
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == stale
         //

         assert(m_state == State_RETRY);
         break;

      case State_SYNC:
      case State_SYNCW:
         // SYNC | SYNCW
         //    expire == infinite | expire <= now + 5 seconds
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         assert(m_nBrowsers > 0);
         assert(m_nHandles == m_nBrowsers + 1);

         ::CloseHandle(m_aHandles[--m_nHandles]);

         m_dwExpire = ::GetTickCount() + m_ulTimeRetry;

         m_status.modifyIcon(IDI_STALE, IDS_TIP_TEMPERR);
         m_status.setState(IDS_STATE_TEMPERR);

         m_state = State_WAIT;

         // WAIT:
         //    expire <= now + 5 seconds
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == stale
         //
         assert(m_state == State_WAIT);
         break;

      default:
         assert(m_state == State_LOGIN ||
                m_state == State_INIT  ||
                m_state == State_SYNC  ||
                m_state == State_SYNCW);
   }

   assert(isValid());
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//  permErr    X        X     go CFG   go CFG      X        X        X     go CFG   go CFG
//
void Synchronizer::onPermErr(AsyncResponse error) {
   assert(gThreadId == GetCurrentThreadId());

   assert(isValid());

   switch (m_state) {
      case State_LOGIN:
         // LOGIN:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    wizard == visible
         //    cursor == hour
         //    icon == null
         //
         assert(m_nHandles == m_nBrowsers + 1);
         assert(m_nBrowsers > 0);
         break;

      case State_SYNCW:
         m_dwExpire = ::GetTickCount() + TIME_RESET;
         // fall through

      case State_SYNC:
      case State_INIT:
         // INIT:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         // SYNC | SYNCW:
         //    expire == infinite || expire < now + 5 seconds
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         assert(m_nHandles == m_nBrowsers + 1);
         assert(m_nBrowsers > 0);

         m_status.removeIcon();

         m_wiz.create(m_status.getWindow(), m_pszEmail);

         break;

      default:
         assert(m_state == State_LOGIN ||
                m_state == State_INIT  ||
                m_state == State_SYNC  ||
                m_state == State_SYNCW);
   }

   CloseHandle(m_aHandles[--m_nHandles]);

   stop();

   LoginWizard::Panels p;

   if (m_state == State_LOGIN) {
      if (error == UnknownEmail || error == UnknownPassword) {
         p = LoginWizard::Panel_Login;
      }
      else {
         p = LoginWizard::Panel_Ready;
      }
   }
   else if (error == UnknownEmail || error == UnknownPassword) {
      m_pszSavedEmail = strrealloc(m_pszSavedEmail, m_pszEmail);
      p = LoginWizard::Panel_Intro;
      m_status.setStatus(IDS_LOGIN_ERR);
   }
   else {
      p = LoginWizard::Panel_Ready;
   }

   m_state = State_CFG;
   m_wiz.bad(p, m_status.getStatusWindow());

   m_status.setState(IDS_STATE_TEMPERR);

   // CFG:
   //    expire == infinite
   //    nHandles == 0
   //    nBrowsers == 0
   //    wizard == visible
   //    cursor == arrow
   //    icon == null
   //
   assert(m_nHandles == 0);
   assert(m_nBrowsers == 0);
   assert(m_state == State_CFG);
   assert(isValid());
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//   syncOk    X        X     go SYNC  go SYNC     X        X        X     go IDLE  go WAIT
//
void Synchronizer::onSyncOk(AsyncResponse rcode) {

   assert(isValid());

   assert(m_nBrowsers > 0);
   assert(m_nHandles == m_nBrowsers + 1);
   ::CloseHandle(m_aHandles[--m_nHandles]);

   switch (m_state) {
      case State_LOGIN:
            // LOGIN:
            //    expire == infinite
            //    nHandles == nBrowsers + 1
            //    nBrowsers > 0
            //    wizard == visible
            //    cursor == hour
            //    icon == null
            //
      case State_INIT:
         // INIT:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         {
            RegKey key;

            if (m_state == State_LOGIN) {
               m_status.addIcon(IDI_STALE, IDS_TIP_UPLOAD);
            }
            else {
               m_status.modifyIcon(IDI_STALE, IDS_TIP_UPLOAD);
            }

            if (key.open(HKEY_CURRENT_USER, m_pszRegKey, true)) {
               key.setValue(REG_EMAIL, m_pszEmail);
               key.setValue(REG_MD5,   m_pszPWHash);
               key.deleteValue(REG_PASS);
            }

            spawn(State_SYNC);

            // SYNC:
            //    expire == infinite
            //    nHandles == nBrowsers + 1
            //    nBrowsers > 0
            //    icon == stale
            //
         }
         break;

      case State_SYNC:
      case State_SYNCW:
         {
            TCHAR achDateTime[1024];
            RegKey key;

            m_wiz.done();

            m_dtLastSynced.format(achDateTime, ELEMENTS(achDateTime));

            m_status.setLastSynced(achDateTime);

            m_status.setState(IDS_RUNNING);

            if (key.open(HKEY_CURRENT_USER, m_pszRegKey, true)) {
               key.setValue(REG_SYNCED, m_dtLastSynced);
            }

            if (m_state == State_SYNC) {
               // SYNC:
               //    expire == infinite
               //    nHandles == nBrowsers + 1
               //    nBrowsers > 0
               //    icon == stale
               //
               //m_dwExpire = ::GetTickCount() + m_ulTimeIdle;
               m_dwExpire = ::GetTickCount() + (m_fAuto ? m_ulTimeIdle : TIME_INFINITE);

               m_status.modifyIcon(IDI_LIVE, IDS_TIP_LAST_SYNCED, achDateTime);

               m_state = State_IDLE;

               // IDLE:
               //    expire <= now + 30 minutes
               //    nHandles == nBrowsers
               //    nBrowsers > 0
               //    icon == live
               //
            }
            else {
               // SYNCW:
               //    expire <= now + 5 seconds
               //    nHandles == nBrowsers + 1
               //    nBrowsers > 0
               //    icon == stale
               //
               m_state = State_WAIT;

               // WAIT:
               //    expire <= now + 5 seconds
               //    nHandles == nBrowsers
               //    nBrowsers > 0
               //    icon == stale
               //
            }
         }
         break;

      default:
         assert(m_state == State_LOGIN ||
                  m_state == State_INIT  ||
                  m_state == State_SYNC  ||
                  m_state == State_SYNCW);
   }

   m_status.setStatus(IDS_NO_ERROR);
   assert(isValid());
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//  connect    X     go LOGIN    -     go LOGIN go LOGIN go LOGIN go LOGIN go LOGIN go LOGIN
//
void Synchronizer::onConnect(LPCTSTR pszEmail, LPCTSTR pszPassword) {
   assert(gThreadId == GetCurrentThreadId());

   assert(isValid());

   switch (m_state) {
      case State_LOGIN:
         return;

      case State_INIT:
      case State_SYNC:
      case State_SYNCW:
      case State_RETRY:
      case State_IDLE:
      case State_WAIT:
         m_status.removeIcon();
         break;

      default:
         assert(m_state != State_START &&
                  m_state != State_STOP);
   }

   stop();
   m_state = State_CFG;

   m_pszEmail = strrealloc(m_pszEmail, pszEmail);
   m_pszPWHash = md5alloc(m_pszEmail, pszPassword);

   m_status.setEmail(m_pszEmail);
   m_settings.setEmail(m_pszEmail);
   m_status.setLastSynced("");
   m_status.setStatus("");

   // use an old token initially
   m_seqno = INITIAL_SEQNO;

   m_dtLastSynced.set_time_t(0);

   // CFG:
   //    expire == infinite
   //    nHandles == 0
   //    nBrowsers == 0
   //    wizard == visible
   //    cursor == arrow
   //    icon == null
   //
   init();
   spawn(State_LOGIN);

   // LOGIN:
   //    expire == infinite
   //    nHandles == nBrowsers + 1
   //    nBrowsers > 0
   //    wizard == visible
   //    cursor == hour
   //    icon == null
   //
   assert(isValid());
   assert(m_state == State_LOGIN);
}

void Synchronizer::onLogin(HWND hwnd) {
   if (m_wiz.isVisible()) {
      m_wiz.go(LoginWizard::Panel_Intro);
   }
   else {
      m_wiz.create(hwnd, m_pszEmail);
   }
}

void Synchronizer::onShowSettings(int i) {
   m_settings.init(m_status.getWindow(), i, m_pszEmail, m_fPopupMenuColumns);
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//     back    X        -     go CFG   go CFG   go CFG   go CFG   go CFG   go CFG   go CFG
//
/* virtual */
void Synchronizer::onBack() {
   assert(gThreadId == GetCurrentThreadId());

   assert(isValid());

   switch (m_state) {
      case State_LOGIN:
         // LOGIN:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    wizard == visible
         //    cursor == hour
         //    icon == null
         //
         assert(m_nHandles == m_nBrowsers + 1);
         assert(m_nBrowsers > 0);
         break;

      case State_SYNCW:
         m_dwExpire = ::GetTickCount() + TIME_RESET;
         // fall through

      case State_SYNC:
      case State_INIT:
         // INIT:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         // SYNC | SYNCW:
         //    expire == infinite || expire < now + 5 seconds
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //

      case State_RETRY:
      case State_IDLE:
      case State_WAIT:
         m_status.removeIcon();

         if (!m_wiz.isVisible()) {
            m_wiz.create(m_status.getWindow(), m_pszEmail);
         }

         break;

      default:
         assert(m_state != State_START);
   }

   stop();

   m_state = State_CFG;

   // CFG:
   //    expire == infinite
   //    nHandles == 0
   //    nBrowsers == 0
   //    wizard == visible
   //    cursor == arrow
   //    icon == null
   //
   assert(m_nHandles == 0);
   assert(m_nBrowsers == 0);
   assert(m_state == State_CFG);
   assert(isValid());
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//  syncnow    X        -        -        -     go INIT  go SYNC  go SYNC  go SYNCW go SYNCW
//
void Synchronizer::onSyncNow() {
   assert(m_state != State_START);

   assert(isValid());

   int i;

   for (i = 0; i < m_nBrowsers; i++) {
      fileChange(m_apBrowsers[i]);
   }

   switch (m_state) {
      case State_RETRY:
         // RETRY:
         //    expire <= now + 10 seconds
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == stale
         //
         spawn(State_INIT);

         // INIT:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         m_status.modifyIcon(IDI_STALE, IDS_TIP_RETRY);

         m_dwExpire += TIME_RESET;
         assert(m_state == State_INIT);
         break;

      case State_IDLE:
      case State_WAIT:

         // IDLE:
         //    expire <= now + 30 minutes
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == live
         //
         // WAIT:
         //    expire <= now + 5 seconds
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == stale
         //
         m_dwExpire += TIME_RESET;

         spawn(State_SYNC);

         // SYNC:
         //    expire == INFINITE
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         assert(m_state == State_SYNC);
         m_status.modifyIcon(IDI_STALE, IDS_TIP_SYNCING);
         break;

      case State_SYNC:
         // SYNC:
         //    expire == INFINITE
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         m_state = State_SYNCW;
         // fall through

      case State_SYNCW:
         // SYNCW:
         //    expire <= now + 5 seconds
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         m_dwExpire = ::GetTickCount() + m_ulTimeSync;
         break;


      default:
         assert(m_state == State_CFG || m_state == State_LOGIN || m_state == State_INIT);
   }

   assert(isValid());
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//   change    X        X        -        -        -     go WAIT  go WAIT  go SYNCW go SYNCW
//
void Synchronizer::onChange() {
   assert(isValid());

   switch (m_state) {
      case State_IDLE:
         // IDLE:
         //    expire <= now + 30 minutes
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == live
         //
         m_status.modifyIcon(IDI_STALE, IDS_TIP_SYNCING);

         m_state = State_WAIT;
         // fall through

      case State_WAIT:
         // WAIT:
         //    expire <= now + 5 seconds
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == stale
         //
         m_dwExpire = ::GetTickCount() + (m_fAuto ? m_ulTimeSync : TIME_INFINITE);
         break;

      case State_SYNC:
         // SYNC:
         //    expire == INFINITE
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         m_state = State_SYNCW;
         // fall through

      case State_SYNCW:
         // SYNCW:
         //    expire <= now + 5 seconds
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         m_dwExpire = ::GetTickCount() + (m_fAuto ? m_ulTimeSync : TIME_INFINITE);
         break;

      default:
         assert(m_state != State_START &&
                m_state != State_STOP  &&
                m_state != State_CFG);
   }

   assert(isValid());
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//    timer    X        X        X        X     go INIT  go SYNC  go SYNC     X     go SYNCW
//
void Synchronizer::onTimer() {
   assert(isValid());

   if (m_dtLastSynced.isValid()) {
      TCHAR achDateTime[1024];

      m_dtLastSynced.format(achDateTime, ELEMENTS(achDateTime));

      m_status.setLastSynced(achDateTime);
   }

   switch (m_state) {
      case State_RETRY:
         // RETRY:
         //    expire <= now + 10 seconds
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == stale
         //
         spawn(State_INIT);

         // INIT:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         m_status.modifyIcon(IDI_STALE, IDS_TIP_RETRY);

         m_dwExpire += TIME_RESET;
         assert(m_state == State_INIT);
         break;

      case State_IDLE:
      case State_WAIT:

         // IDLE:
         //    expire <= now + 30 minutes
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == live
         //
         // WAIT:
         //    expire <= now + 5 seconds
         //    nHandles == nBrowsers
         //    nBrowsers > 0
         //    icon == stale
         //
         m_dwExpire += TIME_RESET;

         spawn(State_SYNC);

         // SYNC:
         //    expire == INFINITE
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         assert(m_state == State_SYNC);
         m_status.modifyIcon(IDI_STALE, IDS_TIP_SYNCING);
         break;

      case State_SYNCW:
         // SYNCW:
         //    expire <= now + 5 seconds
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    icon == stale
         //
         m_dwExpire += m_ulTimeSync;
         break;

      default:
         assert(m_state != State_STOP);
         m_dwExpire += TIME_RESET;
         break;
   }

   assert(isValid());
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//   cancel    X     go STOP  go STOP     -        -        -        -        -        -
//
void Synchronizer::onCancel() {
   assert(isValid());

   assert(gThreadId == GetCurrentThreadId());

   switch (m_state) {
      case State_LOGIN:
      case State_CFG:
         break;

         // LOGIN:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    wizard == visible
         //    cursor == hour
         //    icon == null
         //
         assert(m_nHandles == m_nBrowsers + 1);
         assert(m_nBrowsers > 0);
         assert(m_wiz.isVisible());
         break;

      case State_STOP:
         break;

      default:
         assert(m_state != State_START);
         return;
   }

   stop();

   m_status.destroy();

   m_state = State_STOP;
}

//           START     CFG     LOGIN     INIT    RETRY     IDLE     WAIT     SYNC    SYNCW
//          -------- -------- -------- -------- -------- -------- -------- -------- --------
//     stop    X     go STOP  go STOP  go STOP  go STOP  go STOP  go STOP  go STOP  go STOP
//
void Synchronizer::onShutdown() {
   assert(isValid());

   assert(gThreadId == GetCurrentThreadId());

   switch (m_state) {
      case State_START:
         // START:
         //    expire == infinite
         //    nHandles == 0
         //    nBrowsers == 0
         //    wizard == not-visible && cursor == arrow
         //    icon == null
         //
         break;

      case State_LOGIN:
      case State_CFG:
         break;

         // LOGIN:
         //    expire == infinite
         //    nHandles == nBrowsers + 1
         //    nBrowsers > 0
         //    wizard == visible
         //    cursor == hour
         //    icon == null
         //
         assert(m_nHandles == m_nBrowsers + 1);
         assert(m_nBrowsers > 0);
         assert(m_wiz.isVisible());
         break;

      case State_INIT:
      case State_SYNC:
      case State_SYNCW:

      case State_RETRY:
      case State_IDLE:
      case State_WAIT:
         m_status.removeIcon();
         break;

      case State_STOP:
         break;
   }

   stop();

   m_status.destroy();

   m_state = State_STOP;
}

//  pre: m_state in { State_START, State_CFG, State_RETRY, State_IDLE, State_WAIT }
// post: m_state == state && m_nBrowsers > 0 && m_nHandles == m_nBrowsers + 1
//       ||
//       m_state == State_STOP && nHandles == 0 && nBrowsers == 0
//
void Synchronizer::spawn(Synchronizer::States state) /* throws Win32Error */ {
   int i;
   HANDLE h;
   DWORD dwThreadId;

   assert(gThreadId == GetCurrentThreadId());

   switch (m_state) {
      case State_START:
      case State_CFG:
         // START:
         //    expire == infinite
         //    nHandles == 0
         //    nBrowsers == 0
         //    wizard == not-visible && cursor == arrow
         //    icon == null
         //
         assert(m_nHandles == 0);
         assert(m_nBrowsers == 0);
         for (i = 0; i < m_nRegisteredBrowsers; i++) {
            Browser *pb = m_apRegisteredBrowsers[i];

            if (pb->isInitialized()) {
               HANDLE h = pb->FindFirstChangeNotification();

               if (h != INVALID_HANDLE_VALUE) {
                  m_aHandles[m_nHandles++] = h;
                  m_apBrowsers[m_nBrowsers++] = pb;
               }
            }
         }

         assert(m_nHandles == m_nBrowsers);

         if (m_nHandles == 0) {
            m_state = State_STOP;

            throw Win32Error("FindFirstChangeNotification");
         }

         // fall through

      case State_INIT:
      case State_LOGIN:

      case State_RETRY:
      case State_IDLE:
      case State_WAIT:
         m_state = state;
         h = CreateThread(NULL,              // lpSecurityAttributes
                          0,                 // dwStackSize
                          asyncThreadProc,   // pfnStartAddress
                          this,              // pvParameter
                          0,                 // dwCreationFlags
                          &dwThreadId);      // pdwThreadId

         if (h == NULL) {
            DWORD dwError = GetLastError();

            stop();

            m_state = State_STOP;

            throw Win32Error("CreateThread", dwError);
         }

         m_aHandles[m_nHandles++] = h;
         break;
   }
}

//  pre: 
// post: nHandles == nBrowsers && nBrowsers == 0
//
void Synchronizer::stop() {
   int i;

   assert(gThreadId == GetCurrentThreadId());

   if (m_nHandles == m_nBrowsers + 1) {
      m_nHandles--;

      TerminateThread(m_aHandles[m_nHandles], (DWORD) Interrupted);
      CloseHandle(m_aHandles[m_nHandles]);
   }

   m_status.setPopupMenuBookmarks(NULL);
   m_status.setPopupMenuSubscriptions(NULL);
   BookmarkObject::Detach(m_pC);
   BookmarkObject::Detach(m_bookmarks);
   BookmarkObject::Detach(m_pSubscriptions);

   m_pC = m_bookmarks = NULL;
   m_pSubscriptions = NEW BookmarkModel();

   for (i = 0; i < m_nHandles; i++) {
      ::FindCloseChangeNotification(m_aHandles[i]);
   }

   m_nHandles = 0;
   m_nBrowsers = 0;
}

typedef UINT (CALLBACK* LPFNORGFAV)(HWND, LPTSTR);

void Synchronizer::onOrganize() {
   LPTSTR pszDirectory = MicrosoftBrowser::GetFavoritesDirectory();
   bool fDone = false;

   HMODULE hMod = (HMODULE) LoadLibrary(TEXT("shdocvw.dll"));
   if (hMod != NULL){
      LPFNORGFAV lpfnDoOrganizeFavDlg = (LPFNORGFAV)GetProcAddress(hMod, 
                                                                   TEXT("DoOrganizeFavDlg"));

      if (lpfnDoOrganizeFavDlg != NULL) {
         lpfnDoOrganizeFavDlg(NULL, pszDirectory);
         fDone = true;
      }

      FreeLibrary(hMod);
   }

   if (!fDone) {
      ShellExecute(NULL,
                   TEXT("open"),
                   pszDirectory,
                   NULL,
                   pszDirectory,
                   SW_SHOWNORMAL);
   }

   u_free(pszDirectory);
}

void Synchronizer::showDocument(LPCTSTR pszDoc) 
{
   ShellExecuteA(NULL, "open", pszDoc, NULL, ".", 0);
}

struct HelpUrl {
   int id;
   const char *pszUrl;
};

static const HelpUrl gMenuUrls[] = {
   { ID_POPUP_SYNCNOW,           "popup-syncnow.html" },
   { ID_POPUP_AUTOSYNC,          "popup-autosync.html" },
   { ID_POPUP_STATUS,            "popup-status.html" },
   { ID_POPUP_ABOUT,             "popup-about.html" },
   { ID_POPUP_EXIT,              "popup-exit.html" },
   { ID_POPUP_HELP,              "popup-help.html" },
};

static const HelpUrl gCtrlUrls[] = {
   { ID_LOGIN,             "status-changeaccount.html" },
   { IDC_SYNCED,           "status-synced.html" },
   { IDC_EMAIL,            "status-email.html" },
   { IDC_STATUS,           "status-status.html" },
   { IDC_STATE,            "status-state.html" },
   { IDC_WIZ_EMAIL,        "wizard-email.html" },
   { IDC_NEXT,             "wizard-next.html" },
   { IDC_BACK,             "wizard-back.html" },
   { IDC_REGISTER,         "wizard-register.html" },
//   { IDC_SENDPASS,         "wizard-sendpass.html" },
   { IDC_WIZ_PASSWORD,     "wizard-password.html" },
   { ID_WIZ_CANCEL,        "wizard-cancel.html" },
   { ID_PROXY_CANCEL,      "proxy-cancel.html" },
   { ID_STATUS_DISMISS,    "status-dismiss.html" },
   { ID_PROXY_OK,          "proxy-ok.html" },
};

void Synchronizer::onHelp(HWND hwnd, const HELPINFO *phi, LPCTSTR pszSource) {
   int iCtrlId = phi->iCtrlId;

   if (phi->iContextType == HELPINFO_MENUITEM) {
      // if the menu ID is one of the auto-generated ones
      // from PopupMenu...
      //
      if (iCtrlId >= ID_SUBSCRIPTION) {
         onHelp(hwnd, "bookmarkmenu-subscription.html");
         return;
      }
      else if (iCtrlId >= ID_BOOKMARK) {
         onHelp(hwnd, "bookmarkmenu-bookmark.html");
         return;
      }
      else {
         int i;

         for (i = 0; i < ELEMENTS(gMenuUrls); i++) {
            if (gMenuUrls[i].id == iCtrlId) {
               onHelp(hwnd, gMenuUrls[i].pszUrl);
               return;
            }
         }
      }
   }
   else {
      int i;

      assert(phi->iContextType == HELPINFO_WINDOW);

      if (iCtrlId == -1 || iCtrlId > 1100)
         iCtrlId = GetWindowLong(hwnd, GWL_ID);

      if (iCtrlId == 0)
         iCtrlId = GetWindowLong(hwnd, GWL_USERDATA);

      for (i = 0; i < ELEMENTS(gCtrlUrls); i++) {
         if (gCtrlUrls[i].id == iCtrlId) {
            onHelp(hwnd, gCtrlUrls[i].pszUrl);
            return;
         }
      }
   }
}

void Synchronizer::onHelp(HWND hwnd, LPCTSTR psz) {
   TCHAR achBuffer[MAX_PATH];

   DWORD dw = GetRootDirectory(achBuffer, ELEMENTS(achBuffer));

   tstrcpy(achBuffer + dw, TEXT("help\\"));
   tstrcpy(achBuffer + dw + 5, psz);

   if ((int) ShellExecute(hwnd,           // hwnd
                          TEXT("open"),   // pszOperation
                          achBuffer,      // pszFile
                          NULL,           // pszParameters
                          TEXT("."),      // pszDirectory
                          0) <= 32) {     // nShowCmd
#ifndef NDEBUG
      // oops!  missed a help topic
      //
      MessageBox(hwnd, achBuffer, NULL, 0);
#endif /* NDEBUG */
   }
}

static DWORD WINAPI asyncThreadProc(LPVOID pParameter) {
   Synchronizer *p = (Synchronizer *) pParameter;

   return (DWORD) p->asyncRequest();
}

void Synchronizer::setImages(BookmarkContext *pc) {
   pc->setDefaultBookmarkImages(BookmarkItem::DEFAULT_IMAGE, &gpBuiltins->Bookmark);
   pc->setDefaultBookmarkImages(BookmarkItem::SELECTED_IMAGE, &gpBuiltins->Bookmark);

   pc->setDefaultFolderImages(BookmarkItem::OPEN_IMAGE, &gpBuiltins->BookmarkFolderOpen);
   pc->setDefaultFolderImages(BookmarkItem::CLOSED_IMAGE, &gpBuiltins->BookmarkFolderClosed);

   pc->setDefaultSubscriptionImages(BookmarkItem::OPEN_IMAGE, &gpBuiltins->SubscriptionFolderOpen);
   pc->setDefaultSubscriptionImages(BookmarkItem::CLOSED_IMAGE, &gpBuiltins->SubscriptionFolderClosed);
}

static char *md5alloc(const char *pszEmail, const char *pszPlainText) {
   MD5_CTX context;
   unsigned char digest[16];
   char digest64[25];

   MD5Init(&context);
   MD5Update(&context, (unsigned char *) pszEmail, lstrlenA(pszEmail));
   MD5Update(&context, (unsigned char *) pszPlainText, lstrlenA(pszPlainText));
   MD5Final(digest, &context);

   Base64Encode(digest, sizeof(digest), digest64);

   return stralloc(digest64);
}
