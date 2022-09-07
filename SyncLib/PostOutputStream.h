/*
 * SyncLib/PostOutputStream.h
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
 * Last Modification: 2 Jan 1999
 *
 * Description:
 *    The PostOutputStream class defines an object that
 *    can maintain a rather large in-memory text file
 *    to be submitted as an HTTP POST request.  Characters
 *    are encoded as per the application/x-www-form-urlencoded
 *    MIME-type.
 */
#ifndef PostOutputStream_H
#define PostOutputStream_H

#include "OutputStream.h"
#include "ByteArrayOutputStream.h"

namespace syncit {

   /**
    * A PostOutputStream buffers up data to be sent
    * to a web server via an HTTP POST method.
    * <p>
    * The following headers are required:
    * <pre>
    * Content-type: application/x-www-form-urlencoded
    * Content-length: {the value of the contentLength property}
    * </pre>
    * <p>
    * Valid URL characters (alphanumeric, *, ., _, -) are passed
    * through, all others are encoded by their hex value like this:
    *   %XX
    * So '\n' maps to %0A, '\r' maps to %0D.
    */
   class PostOutputStream : public ByteArrayOutputStream {

   public:
      /**
       * Constructs a new PostOutputStream out of an uninitialized structure
       */
      PostOutputStream() {}

      /**
       * Release any internal storage used by the post request.
       */
      virtual ~PostOutputStream() {}

      /**
       * The appendVariable method adds a new {tag} '=' {value} pair,
       * separating it from the previous pair by a '&' if necessary.
       *
       * @param pszName the name of the variable
       */
      void nextVariable(const char *pszName);

      //////////////////
      // OutputStream...
      //
      virtual void write(const char *pbBuffer, size_t cbBuffer);

      //virtual void flush();

      //virtual void close();
      //
      // ...OutputStream
      //////////////////

   private:
      // disable copy constructor and assignment
      //
      PostOutputStream(PostOutputStream &out);
      PostOutputStream &operator=(PostOutputStream &out);
   };

}

#endif /* PostOutputStream_H */
