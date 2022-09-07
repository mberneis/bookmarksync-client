/*
 * SyncLib/Reader.h
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
#ifndef Reader_H
#define Reader_H

namespace syncit {

   /**
    * The Reader class is the abstract base class for all
    * charater-based data input streams.  Compare with InputStream,
    * which is intended for 8-bit binary data streams.
    */
   class Reader {

   protected:
      Reader() {}

   public:
      virtual ~Reader() {}

      /**
       * Read a character from the input stream.
       *
       * Return -1 on EOF, otherwise returns the unsigned character
       * value just read.
       *
       * @return -1 on EOF, 0..255 if ISO-Latin-1, 0..65535 if Unicode
       */
      virtual int read() /* throws IOException */ = 0;

      /**
       * Closes the reader
       */
      virtual void close() /* throws IOException */ = 0;
   };

}

#endif /* Reader */
