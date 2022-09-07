/*
 * SyncLib/BitmapFileImage.cxx
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
#include "BitmapFileImage.h"
#include "FileInputStream.h"
#include "text.h"
#include "util.h"

using namespace syncit;

BitmapFileImage::BitmapFileImage() {
   m_pbi = NULL;
   m_pb  = NULL;
   m_pszUrl = NULL;
}

/* virtual */
BitmapFileImage::~BitmapFileImage() {
   delete m_pbi;
   delete m_pb;
   delete m_pszUrl;
}

bool BitmapFileImage::load(const char *psz) /* throws IOError */ {
   FileInputStream f;

   return f.open(psz) && load(f);
}

/* virtual */
bool BitmapFileImage::load(InputStream &in) /* throws IOError */ {
   BITMAPFILEHEADER bfh;

   delete m_pbi;
   delete m_pb;

   in.read((char *) &bfh, sizeof(bfh));

   if (bfh.bfType != 0x4D42) {
      // first two bytes aren't 'BM'
      return false;
   }

   // we've read in sizeof(BITMAPFILEHEADER) so far, the offset
   // to the image data is bfh.bfOffBits, so the header size
   // must be bfh.bfOffBits-sizeof(BITMAPFILEHEADER)
   //
   else if (bfh.bfOffBits < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) {
      // not a valid bitmap file...
      return false;
   }
   else {
      size_t cbInfo = bfh.bfOffBits - sizeof(BITMAPFILEHEADER);
      size_t cbData = bfh.bfSize - bfh.bfOffBits;

      m_pbi = (BITMAPINFO *) NEW char[cbInfo];

      if (m_pbi == NULL) {
         return false;
      }
      else {
         m_pb = NEW unsigned char[cbData];

         if (m_pb == NULL) {
            delete m_pbi;
            m_pbi = NULL;
            return false;
         }
         else {
            in.read((char *) m_pbi, cbInfo);
            in.read((char *) m_pb, cbData);

            return true;
         }
      }
   }
}

/* virtual */
int BitmapFileImage::getWidth() const {
   return m_pbi->bmiHeader.biWidth;
}

/* virtual */
int BitmapFileImage::getHeight() const {
   return m_pbi->bmiHeader.biHeight;
}

/* virtual */
void BitmapFileImage::draw(HDC hdc, int x, int y) {
   SetDIBitsToDevice(hdc,                       // handle to destination device context
                     x,                         // destination x
                     y,                         // destination y
                     m_pbi->bmiHeader.biWidth,  // source rectangle width
                     m_pbi->bmiHeader.biHeight, // source rectangle height
                     0,                         // source rectangle x
                     0,                         // source rectangle y
                     0,                         // first scan line in array
                     m_pbi->bmiHeader.biHeight, // number of scan lines
                     m_pb,                      // array of source DIB bits
                     m_pbi,                     // source DIB info
                     DIB_RGB_COLORS);
}
