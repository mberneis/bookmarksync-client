/*
 * SyncIT/SettingsWindow.h
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
#ifndef SettingsWindow_H
#define SettingsWindow_H

#include "SyncLib/URL.h"

using namespace syncit;

class SettingsListener {
protected:
   SettingsListener() {}

public:
   virtual ~SettingsListener() {}

   virtual void onLogin(HWND hwnd) = 0;

   virtual void onHelp(HWND hwnd, const HELPINFO *phi, LPCTSTR pszSource) = 0;
   virtual void onHelp(HWND hwnd, LPCTSTR psz) = 0;

   /**
    * Point a web browser at the rooted URL.
    * The root defaults to "http://www.bookmarksync.com/" but can
    * be changed by registry entries...  So for instance:
    * showDocument("main.asp") would work.
    */
   virtual void showDocument(LPCTSTR pszUrl) = 0;

   virtual const URL &getRoot() const = 0;

   virtual void toggleAutoSync() = 0;

   virtual void setPopupMenuColumns(bool f) = 0;
   virtual void showBookmarkPopup() = 0;

private:
   SettingsListener(SettingsListener &rhs);
   SettingsListener &operator=(SettingsListener &rhs);
};

class SettingsWindow {

public:
   SettingsWindow() {
      m_hwnd = NULL;
      m_pl   = NULL;
   }

   void addSettingsListener(SettingsListener *pl) {
      m_pl = pl;
   }

   void init(HWND hwndParent,
             int nStartPage,
             LPCTSTR pszEmail,
             bool fPopupMenuColumns);

   LPCTSTR getEmail() const {
      return m_pszEmail;
   }

   void setEmail(LPCTSTR pszEmail);

   bool isDialogMessage(MSG *msg);

   void onLogin(HWND hwnd) {
      m_pl->onLogin(hwnd);
   }

   void showDocument(LPCTSTR pszUrl) {
      m_pl->showDocument(pszUrl);
   }

   void showBookmarkPopup() {
      m_pl->showBookmarkPopup();
   }

   void setPopupMenuColumns(bool f) {
      m_fPopupMenuColumns = f;
      m_pl->setPopupMenuColumns(f);
   }

   void applyPopupMenuColumns(bool f) {
      m_fPopupMenuColumns = m_fSavedPopupMenuColumns = f;
   }

   void resetPopupMenuColumns() {
      m_pl->setPopupMenuColumns(m_fPopupMenuColumns = m_fSavedPopupMenuColumns);
   }

   const URL &getRoot() {
	  return m_pl->getRoot();
   }

   bool getPopupMenuColumns() const {
      return m_fPopupMenuColumns;
   }

   // get the URL to register a new user
   tstring getRegistrationUrl() const;

private:
   HWND m_hwnd;
   SettingsListener *m_pl;
   LPCTSTR m_pszEmail;

   bool m_fPopupMenuColumns, m_fSavedPopupMenuColumns;
};


#endif /* SettingsWindow_H */
