/*
 * SyncLib/StringBuffer.h
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
 *    The StringBuffer class can efficiently append characters onto
 *    a buffer until the whole string is requested.
 */
#ifndef StringBuffer_H
#define StringBuffer_H

#include "util.h"
#include "text.h"

namespace syncit {
   class StringBuffer {
   public:
      /**
       * Initialize a stack- or user-allocated
       * StringBuffer structure.  Use StringBuffer_destruct
       * to release any memory.
       */
      StringBuffer(size_t max = 32) {
         m_len = 0;
         m_max = max;

         m_pach = (char *) u_malloc(m_max);
         assert(m_pach != NULL);
      }

      /**
       * Release any memory inside the StringBuffer.
       */
      ~StringBuffer() {
         u_free(m_pach);
      }

      void reset() {
         m_len = 0;
      }

      /**
       * Get a copy of the string maintained by the StringBuffer.
       * Use the util.h u_free() routine to free the storage returned
       * by this method.
       *
       * @param psb  a pointer to a valid StringBuffer
       * @return a null-terminated string allocated by u_malloc()
       */
      char *getString() const {
         return strcalloc(m_pach, m_len);
      }

      /**
       * Append a character to the end of the string maintained
       * by the StringBuffer.
       *
       * @param ch   a character to add to the end
       */
      void append(char ch) {
         if (m_len == m_max) {
            m_max = m_max * 3 / 2;

            m_pach = (char *) u_realloc(m_pach, m_max);
            assert(m_pach != NULL);
         }

         m_pach[m_len++] = ch;
      }

   private:
      char *m_pach;

      size_t m_max, m_len;

      // disable copy constructor and assignment
      //
      StringBuffer(StringBuffer &out);
      StringBuffer &operator=(StringBuffer &out);
   };
}

#endif /* StringBuffer_H */
