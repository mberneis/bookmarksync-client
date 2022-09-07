/*
 * SyncLib/BitmapFileImage.h
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
 * Last Modification: 6 Oct 1999
 *
 * Description:
 *    A BitmapFileImage is an Image loaded from a .BMP file.
 */
#ifndef BitmapFileImage_H
#define BitmapFileImage_H

#include "text.h"
#include "Image.h"
#include "InputStream.h"

namespace syncit {

   class BitmapFileImage : public Image {

   public:
      BitmapFileImage();
      virtual ~BitmapFileImage();

      virtual void draw(HDC hdc,
                        int x,
                        int y);

      virtual int getWidth() const;
      virtual int getHeight() const;

      virtual bool load(const char *pszFilename) /* throws IOError */;
      virtual bool load(InputStream &in) /* throws IOError */;

      virtual const char *getUrl() const {
         return m_pszUrl;
      }

      void setUrl(const char *pszUrl) {
         m_pszUrl = strrealloc(m_pszUrl, pszUrl);
      }

   private:
      // pointer to BITMAPINFO, and colors
      BITMAPINFO *m_pbi;

      // pointer to image data
      unsigned char *m_pb;

      char *m_pszUrl;
   };

}

#endif /* BitmapFileImage_H */
