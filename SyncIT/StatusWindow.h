/*
 * SyncIT/StatusWindow.h
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
 * Last Modification: 8 Jan 1999
 *
 * Description:
 *    The main status window, also used as a sink for
 *    popup menu events.
 */
#ifndef StatusWindow_H
#define StatusWindow_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <stdarg.h>
#include <windows.h>
#include <shellapi.h>

#include "PopupMenu.h"

#include "resource.h"

extern HINSTANCE ghResourceInstance;

class StatusListener {
protected:
   StatusListener() {}

public:
   virtual ~StatusListener() {}

   virtual void onShutdown() = 0;

   virtual void onHelp(HWND hwnd, const HELPINFO *phi, LPCTSTR pszSource) = 0;
   virtual void onHelp(HWND hwnd, LPCTSTR psz) = 0;

   /**
    * Point a web browser at the rooted URL.
    * The root defaults to "http://www.bookmarksync.com/" but can
    * be changed by registry entries...  So for instance:
    * showDocument("main.asp") would work.
    */
   virtual void showDocument(LPCTSTR pszUrl) = 0;

   virtual void onSyncNow() = 0;

   virtual void onOrganize() = 0;

   virtual void toggleAutoSync() = 0;

   virtual void resetPopup() = 0;

   virtual void onShowSettings(int i) = 0;

private:
   StatusListener(StatusListener &rhs);
   StatusListener &operator=(StatusListener &rhs);
};

class StatusWindow {

#ifndef NDEBUG
public:
   bool isValid() const;
private:
   char m_achStartTag[sizeof("StatusWindow")];
#endif /* NDEBUG */

   friend BOOL APIENTRY StatusDlgProc(HWND hDialog,
                                      UINT wMsg,
                                      WPARAM wParam,
                                      LONG lParam);

public:
   StatusWindow(HINSTANCE hInstance);

   ~StatusWindow();

   void show();

   void addStatusListener(StatusListener *p) {
      assert(m_pListener == NULL);
      m_pListener = p;
   }

   void setEmail(LPCTSTR psz) {
      ::SetDlgItemText(m_hwnd, IDC_EMAIL, psz);
   }

   void setLastSynced(LPCTSTR psz) {
      ::SetDlgItemText(m_hwnd, IDC_SYNCED, psz);
   }

   void setStatus(int ids, ...);

   void setStatus(LPCTSTR psz) {
      ::SetDlgItemText(m_hwnd, IDC_STATUS, psz);
   }

   HWND getStatusWindow() const {
      return ::GetDlgItem(m_hwnd, IDC_STATUS);
   }

   void setState(int ids) {
      TCHAR achBuffer[4096];

      LoadString(ghResourceInstance,
                 ids,
                 achBuffer,
                 ELEMENTS(achBuffer));

      setState(achBuffer);
   }

   void setState(LPCTSTR psz) {
      ::SetDlgItemText(m_hwnd, IDC_STATE, psz);
   }

   int getPopupMenuColumnSize() const {
      return m_popup.getColumnSize();
   }

   void setPopupMenuColumnSize(int i) {
      m_popup.setColumnSize(i);
   }

   void setPopupMenuBookmarks(BookmarkFolder *pbfBookmarks) {
      m_popup.setBookmarks(PopupMenu::Popup_BOOKMARKS, pbfBookmarks);
   }

   void setPopupMenuSubscriptions(BookmarkFolder *pbfBookmarks) {
      m_popup.setBookmarks(PopupMenu::Popup_SUBSCRIPTIONS, pbfBookmarks);
   }

   void addIcon(int idi, int ids, ...);
   void modifyIcon(int idi, int ids, ...);

   void removeIcon();

   void destroy() {
      ::DestroyWindow(m_hwnd);
   }

   HWND getWindow() const {
      return m_hwnd;
   }

   HINSTANCE getInstance() const {
      return GetWindowInstance(m_hwnd);
   }

   void setAutoSyncCheck(bool fChecked);

   void showBookmarkPopup();
   void showStatusPopup();

protected:
   void resetIcon();
   void notifyIcon(DWORD dwMessage, int idi, int ids, va_list ap);

   void showPopup(HMENU hmenu);

private:
   HWND m_hwnd;

   // The tray icon
   //
   NOTIFYICONDATA m_nid;

   // The status popup (defined by resource IDM_POPUP)
   //
   HMENU m_hMenu;          // defined by resource
   HMENU m_hPopupMenu;     // item 0 (the real popup)

   // The bookmark popups (defined on the fly)
   //
   PopupMenu m_popup;

   StatusListener *m_pListener;

   // disable copy constructor and assignment
   //
   StatusWindow(const StatusWindow &rhs);
   StatusWindow &operator=(const StatusWindow &rhs);

#ifndef NDEBUG
private:
   char m_achEndTag[sizeof("StatusWindow")];
#endif /* NDEBUG */

};

#endif /* StatusWindow_H */
