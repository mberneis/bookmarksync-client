/*
 * SyncLib/Image.h
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
 *    This module declares the Image abstract base class.
 *
 * See also:
 *    SyncLib/Image.cxx    -- source code definitions
 *    SyncLib/BitmapImage.h
 *    SyncLib/BitmapImage.cxx
 *    SyncLib/IconImage.h
 *    SyncLib/IconImage.cxx
 */
#pragma warning( disable : 4786 )

#ifndef Image_H
#define Image_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include <string>
#include <map>

#include "URL.h"

namespace syncit {

   using std::string;
   using std::map;

   /**
    * The Image class is the virtual base class (abstract base class) for all
    * images to be drawn by the SyncIT client.  Initial images will be icons,
    * bitmaps, and PNG files; but newer types (jpegs, etc.) should be easy to
    * add.
    *
    * Images are reference counted, and are "cached" in an in-memory table so
    * retrieving the same image for a new bookmark will take no further
    * resources.
    *
    * The primary method for an Image is draw(), which will do whatever it takes
    * to draw the image on a windows HDC.  The width and height properties are
    * also available through the getWidth() and getHeight() methods.
    */
   class Image {

   public:
      Image() {
         m_ulRefCount = 1;
      }

      virtual ~Image() {
      }

      Image *attach() {
         m_ulRefCount++;
         return this;
      }

      bool detach() {
         return --m_ulRefCount == 0;
      }

      static void Detach(Image *p);

      virtual void draw(HDC hdc,
                        int x,
                        int y);

      virtual int getWidth() const;
      virtual int getHeight() const;

      virtual const char *getUrl() const {
         return NULL;
      }

      static Image Blank;

   private:
      unsigned long m_ulRefCount;

      // disable copy constructor and assignment operator
      //
      Image(Image &rhs);
      Image &operator=(Image &rhs);
   };

   class ImageLoader {
   public:
      ImageLoader(HINSTANCE hInstance) : m_hInstance(hInstance) {
      }

      Image *load(const string &url, const URL *pbase = NULL);

   private:
      HINSTANCE m_hInstance;

      typedef map<string, Image *> Map;

      Map m_map;

      // disable copy constructor, assignment
      ImageLoader(ImageLoader &rhs);
      ImageLoader &operator=(ImageLoader &rhs);

   };

}

#endif /* Image_H */
