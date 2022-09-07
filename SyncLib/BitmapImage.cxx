/*
 * SyncLib/BitmapImage.cxx
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
#include "BitmapImage.h"
#include "Errors.h"

using namespace syncit;

/* virtual */
BitmapImage::~BitmapImage() {
   ::DeleteObject(m_hBitmap);
}

BitmapImage::BitmapImage(HINSTANCE hInstance, LPCTSTR pszResourceName) {
   m_hBitmap = LoadBitmap(hInstance, pszResourceName);

   if (m_hBitmap == NULL) {
      throw Win32Error("LoadBitmap");
   }

   ::GetBitmapDimensionEx(m_hBitmap, &m_size);
}

/* virtual */
int BitmapImage::getWidth() const {
   return m_size.cx;
}

/* virtual */
int BitmapImage::getHeight() const {
   return m_size.cy;
}

void BitmapImage::draw(HDC hdc, int x, int y) {
   ::BitBlt(hdc,           // hdcDest
            x, y,          // nXDest, nYDest
            m_size.cx,     // nWidth
            m_size.cy,     // nHeight
            (HDC) m_hBitmap,// hdcSource
            0, 0,          // nXSource, nYSource
            SRCCOPY);
}
