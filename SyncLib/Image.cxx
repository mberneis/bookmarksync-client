/*
 * SyncLib/Image.cxx
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
 * Last Modification: 4 Feb 1999
 *
 * Description:
 *    This module defines the Image abstract base class.
 *
 * See also:
 *    SyncLib/Image.h      -- declarations
 *    SyncLib/BitmapImage.h
 *    SyncLib/BitmapImage.cxx
 *    SyncLib/IconImage.h
 *    SyncLib/IconImage.cxx
 */
#include "Image.h"
#include "util.h"
#include "HttpRequest.h"

#include "BitmapFileImage.h"
#include "IconImage.h"

using namespace syncit;
using std::map;

Image Image::Blank;

void Image::Detach(Image *p) {
   if (p->detach()) {
      delete p;
   }
}
/* virtual */
void Image::draw(HDC hdc,
                 int x,
                 int y) {
}

/* virtual */
int Image::getWidth() const {
   return 0;
}

/* virtual */
int Image::getHeight() const {
   return 0;
}

Image *ImageLoader::load(const string &url, const URL *pbase) {
   Map::const_iterator i = m_map.find(url);
   Image *p = NULL;

   if (i == m_map.end()) {
      char achPath[MAX_PATH];

      size_t lPath = UrlToFilename(url.c_str(), achPath, sizeof(achPath), true);

      if (pbase != NULL) {
         DownloadDocument(URL(*pbase, url), achPath);
      }

      if (stricmp(achPath + lPath - 4, ".bmp") == 0) {
         BitmapFileImage *pp = NEW BitmapFileImage;

         try {
            if (pp->load(achPath)) {
               pp->setUrl(url.c_str());

               p = pp;
            }
            else {
               delete pp;
            }
         } catch (...) {
            delete pp;
         }
      }
      else if (stricmp(achPath + lPath - 4, ".ico") == 0) {
         IconImage *pp = NEW IconImage();

         try {
            if (pp->load(m_hInstance, achPath, 0)) {
               pp->setUrl(url.c_str());

               p = pp;
            }
            else {
               delete pp;
            }
         } catch (...) {
            delete pp;
         }
      }

      m_map[url] = p;
   }
   else {
      p = (*i).second;
   }

   return p ? p->attach() : NULL;
}
