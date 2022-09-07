/*
 * SyncIT/About.h
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
 *   Calls up the SyncIT agent's about box.  The about box
 *   is a simple modeless dialog box.  It is modeless because
 *   the SyncIT agent's main loop should never be stopped to
 *   handle window loops as a lot of timeout and file change
 *   processing occurs in the main loop.
 *
 * See also:
 *    About.cxx         -- contains the definitions of the method(s)
 *                         declared here
 *    WinMain.cxx       -- calls up the dialog box on command
 *    StatusWindow.cxx  -- calls DisplayAboutBox.cxx
 */
#ifndef About_H
#define About_H

/*
 * Display the about box, loading it initially if required.
 */
void DisplayAboutBox(HINSTANCE hInstance);
void DrawLinkButton(HWND hDlg, DRAWITEMSTRUCT *pdis);

#endif /* About_H */
