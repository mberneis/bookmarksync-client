/*
 * SyncLib/CriticalSection.h
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
 * Last Modification: 8 Jan 1999
 *
 * Description:
 *    A simple class wrapper around the WIndows NT/95
 *    CRITICAL_SECTION structure.
 */
#ifndef CriticalSection_H
#define CriticalSection_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include "util.h"

namespace syncit {

   /**
    * The CriticalSection wraps an NT critical section or
    * mutex up in a class.
    */
   class CriticalSection {

   public:
      CriticalSection() {
         InitializeCriticalSection(&m_cs);
      }

      virtual ~CriticalSection() {
         DeleteCriticalSection(&m_cs);
      }

      void enter() {
         EnterCriticalSection(&m_cs);
      }

      void leave() {
         LeaveCriticalSection(&m_cs);
      }

   private:
      CRITICAL_SECTION m_cs;

      // disable copy, assignment
      //
      CriticalSection(CriticalSection &rhs);
      CriticalSection &operator=(CriticalSection &rhs);
   };

}

#endif /* CriticalSection_H */
