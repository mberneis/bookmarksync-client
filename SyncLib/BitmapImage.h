/*
 * SyncLib/BitmapImage.h
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
 * Creation:          8 Jan 1999
 * Last modification: 6 Oct 1999
 *
 * Description:
 *    A BitmapImage is an Image object that is loaded from the BITMAP resource of an
 *    executable.
 */
#ifndef BitmapImage_H
#define BitmapImage_H

#include "Image.h"

namespace syncit {

   class BitmapImage : public Image {

   public:
      /**
       * Create an BitmapImage by loading it from our image's resources.
       *
       * @param hInstance -- handle to application instance
       * @param pszResourceName -- resource name (or atom, use MAKEINTRESOURCE) to load
       *
       * @exception Win32Error on any load error
       */
      BitmapImage(HINSTANCE hInstance, LPCTSTR pszResourceName);

      virtual ~BitmapImage();

      virtual int getWidth() const;
      virtual int getHeight() const;

      virtual void draw(HDC hdc, int x, int y);

   private:
      HBITMAP  m_hBitmap;
      SIZE     m_size;
   };

}

#endif /* IconImage_H */
