/*
 * SyncLib/IconImage.h
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
#ifndef IconImage_H
#define IconImage_H

#include "Image.h"
#include "text.h"

namespace syncit {

   class IconImage : public Image {

   public:
      IconImage() {
         m_hIcon = NULL;
         m_pszUrl = NULL;
         m_size.cx = m_size.cy = 0;
      }

      /**
       * Create an IconImage by loading it from our image's resources.
       *
       * @param hInstance -- handle to application instance
       * @param pszResourceName -- resource name (or atom, use MAKEINTRESOURCE) to load
       *
       * @exception Win32Error on any load error
       */
      IconImage(HINSTANCE hInstance, LPCTSTR pszResourceName);

      /**
       * Create an IconImage by loading it from our image's resources.
       *
       * @param hInstance -- handle to application instance
       * @param pszResourceName -- resource name (or atom, use MAKEINTRESOURCE) to load
       * @param w  -- width of icon to load, searches for correct image in icon image list
       * @param h  -- height of icon to load, searches for correct image in icon image list
       *
       * @exception Win32Error on any load error
       */
      IconImage(HINSTANCE hInstance, LPCTSTR pszResourceName, int w, int h);

      /**
       * Create an IconImage from loading from an executable
       *
       * @param hInstance -- handle to application instance
       * @param pszImageName -- executable file name
       * @param iconIndex -- icon index, use 0 for default
       *
       * @exception Win32Error on any load error
       */
      IconImage(HINSTANCE hInstance, LPCTSTR pszImageName, int iconIndex);

      virtual ~IconImage();

      virtual int getWidth() const;
      virtual int getHeight() const;

      virtual void draw(HDC hdc, int x, int y);

      virtual const char *getUrl() const {
         return m_pszUrl;
      }

      void setUrl(const char *psz) {
         m_pszUrl = strrealloc(m_pszUrl, psz);
      }

      HICON getHandle() const {
         return m_hIcon;
      }

      bool load(HINSTANCE hInstance,
                const char *pszFile,
                int iconIndex = 0);

   private:
      HICON m_hIcon;
      SIZE  m_size;
      char *m_pszUrl;
   };

}

#endif /* IconImage_H */
