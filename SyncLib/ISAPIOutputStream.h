/*
 * SyncLib/ISAPIOutputStream.h
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
#ifndef ISAPIOutputStream_H
#define ISAPIOutputStream_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>
#include <httpext.h>

#include "SyncLib/OutputStream.h"

namespace syncit {

   class ISAPIOutputStream : public OutputStream {
   public:
      ISAPIOutputStream(EXTENSION_CONTROL_BLOCK *pecb) {
         m_pecb = pecb;
      }

      /**
       * Write the contents of the buffer to the file/socket.
       * <p>
       * The entire contents are written.
       *
       * @param pbBuffer start of buffer to write
       * @param cbBuffer length of buffer in bytes to write
       * @return true if successful, false on error
       */
      virtual void write(const char *pbBuffer, size_t cbBuffer); /* throws IOError */

      /**
       * Flush any cached/buffered data to the stream.
       */
      virtual void flush(); /* throws IOError */

      /**
       * Close the stream.
       */
      virtual void close();

   private:
      EXTENSION_CONTROL_BLOCK *m_pecb;
   };

}

#endif /* ISAPIOutputStream_H */
