/*
 * SyncIT/LoginWizard.h
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
#ifndef LoginWizard_H
#define LoginWizard_H

class WizardListener {
protected:
   WizardListener() {}

public:
   virtual ~WizardListener() {}

   virtual void onCancel() = 0;
   virtual void onConnect(LPCTSTR pszEmail, LPCTSTR pszPassword) = 0;
   virtual void onBack() = 0;

   virtual void onHelp(HWND hwnd, const HELPINFO *phi, LPCTSTR pszSource) = 0;
   virtual void onHelp(HWND hwnd, LPCTSTR psz) = 0;

   /**
    * Point a web browser at the rooted URL.
    * The root defaults to "http://www.bookmarksync.com/" but can
    * be changed by registry entries...  So for instance:
    * showDocument("main.asp") would work.
    */
   virtual void showDocument(LPCTSTR pszUrl) = 0;

   virtual void onShowSettings(int i) = 0;

private:
   WizardListener(const WizardListener &rhs);
   WizardListener &operator=(const WizardListener &rhs);
};

/**
 * The Wizard here is the 5 panel wizard that greets users that
 * haven't logged into bookmarksync on this computer before.
 * The five panels are dialog resources in SyncIT.rc, declared in resource.h:
 * 1. IDD_INTRO   -- introductory text
 * 2. IDD_EMAIL   -- 
 */
class LoginWizard {

#ifndef NDEBUG
public:
   bool isValid() const;
private:
   char m_achStartTag[sizeof("LoginWizard")];
#endif /* NDEBUG */

   friend BOOL CALLBACK WizardDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
   friend BOOL CALLBACK PanelDlgProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

public:
   LoginWizard(HINSTANCE hInstance);

   virtual ~LoginWizard();

   enum Panels {
      Panel_First = 0,
      Panel_Intro = Panel_First,
      Panel_Login,
    //Panel_Browsers,
      Panel_Ready,
      Panel_Connect,
      Panel_Last = Panel_Connect,

      NUM_PANELS
   };

   void create(HWND hWndParent, LPCTSTR pszEmail);

   bool isDialogMessage(MSG *msg);

   // inputs from the synchronizer:
   //
   /**
    * checkpoint -- called either while login wizard is not active,
    *               or when IDD_CONNECT is shown, to cycle through
    *               the activities.
    * pass in one of
    *    IDC_WIZ_CONNECTING
    *    IDC_WIZ_DOWNLOADING or
    *    IDC_WIZ_UPLOADING
    *
    * @param id  control ID of one of the static texts
    *            in IDD_CONNECT
    */
   void checkpoint(int id);

   /**
    * Go to a specified page and set the IDC_MSG control on that
    * page to the specified text.
    *
    * @param idd  one of IDD_INTRO, IDD_EMAIL, IDD_PASSWORD, or IDD_READY
    * @param ids  one of the resource string IDs
    *
    * @pre: isVisible()
    * @post: isVisible()
    */
   void bad(Panels p, HWND hwnd);
   void bad(Panels p, LPCTSTR psz);

   void go(Panels p);

   void done();

   ////////////////
   // properties...
   //
   void addWizardListener(WizardListener *p) {
      assert(m_pListener == NULL);
      m_pListener = p;
   }

   LPCTSTR getEmail() const { return m_pszEmail; }
   void setEmail(LPCTSTR psz) { m_pszEmail = strrealloc(m_pszEmail, psz); }

   // get the URL to register a new user
   tstring getRegistrationUrl() const;

   bool isVisible() const {
      return m_hwnd != NULL;
   }

   HWND getWindow() const {
      return m_hwnd;
   }

protected:
   void next() {
      if (m_iCurPanel < Panel_Last) {
         go((Panels) (m_iCurPanel + 1));
      }
      else {
         finish();
      }
   }

   void back() {
      go((Panels) (m_iCurPanel - 1));
   }

   void cancel();
   void finish();

   /**
    * Reset all pages except the current one
    * to their initial state.
    */
   void wipe();

   void enable(HWND hwnd, int id, BOOL fState);

private:
   // disable copy constructor and assignment operator
   //
   LoginWizard(const LoginWizard &rhs);
   LoginWizard &operator=(const LoginWizard &rhs);

   WizardListener *m_pListener;

   // handle to wizard window, the container
   HWND m_hwnd;

   // five panels...
   // the idea here is that panels are windows
   // that can be created or destroyed as needed.
   // if m_ahwndPanels[i] is NULL, then the window
   // needs to be created when viewed.
   //
   HWND m_ahwndPanels[NUM_PANELS];

   Panels m_iCurPanel;

   LPTSTR m_pszEmail, m_pszPassword;

   int m_state;

#ifndef NDEBUG
private:
   char m_achEndTag[sizeof("LoginWizard")];
#endif /* NDEBUG */

};

#endif /* LoginWizard_H */
