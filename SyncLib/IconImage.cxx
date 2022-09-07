/*
 * SyncLib/IconImage.cxx
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
#include "IconImage.h"
#include "Errors.h"

#include <shellapi.h>

using namespace syncit;

/* virtual */
IconImage::~IconImage() {
   ::DestroyIcon(m_hIcon);

   delete m_pszUrl;
}

IconImage::IconImage(HINSTANCE hInstance, LPCTSTR pszResourceName) {
   m_size.cx = m_size.cy = 0;
   m_pszUrl = NULL;
   m_hIcon = LoadIcon(hInstance, pszResourceName);

   if (m_hIcon == NULL) {
      throw Win32Error("LoadIcon");
   }
}

IconImage::IconImage(HINSTANCE hInstance, LPCTSTR pszResourceName, int w, int h) {
   m_size.cx = m_size.cy = 0;
   m_pszUrl = NULL;
   m_hIcon = (HICON) LoadImage(hInstance,
                               pszResourceName,
                               IMAGE_ICON,
                               w, h,
                               0);

   if (m_hIcon == NULL) {
      throw Win32Error("LoadImage");
   }
}

/* virtual */
int IconImage::getWidth() const {
   if (m_size.cx == 0) {
      ICONINFO ii;

      ::GetIconInfo(m_hIcon,
                    &ii);

      ::GetBitmapDimensionEx(ii.hbmColor, &((IconImage *) this)->m_size);

      ::DeleteObject(ii.hbmColor);
      ::DeleteObject(ii.hbmMask);
   }

   return m_size.cx;
}

/* virtual */
int IconImage::getHeight() const {
   if (m_size.cy == 0) {
      ICONINFO ii;

      ::GetIconInfo(m_hIcon,
                    &ii);

      ::GetBitmapDimensionEx(ii.hbmColor, &((IconImage *) this)->m_size);

      ::DeleteObject(ii.hbmColor);
      ::DeleteObject(ii.hbmMask);
   }

   return m_size.cy;
}

void IconImage::draw(HDC hdc, int x, int y) {
   ::DrawIconEx(hdc,       // HDC
                x, y,      // xLeft, yTop
                m_hIcon,   // hIcon
                0, 0,      // cxWidth, cyHeight
                0,         // istepIfAniCur
                NULL,      // hbrFlickerFreeDraw
                DI_NORMAL);// diFlags
}

bool IconImage::load(HINSTANCE hInstance, LPCTSTR pszImageName, int iconIndex) {

#if 0
   m_hIcon = ::ExtractIcon(hInstance, pszImageName, iconIndex);

   if (m_hIcon == (HICON) 1) {
      throw Win32Error("ExtractIcon", ERROR_BAD_FORMAT);
   }
   else if (m_hIcon == NULL) {
      throw Win32Error("ExtractIcon");
   }

   return true;

#else

   return ::ExtractIconEx(pszImageName, iconIndex, NULL, &m_hIcon, 1) == 1;
#endif
}
