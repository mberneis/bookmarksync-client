/*
 * SyncLib/DateTime.h
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
 * Last Modification: 6 Oct 1999
 *
 * Description:
 *    This module declares the DateTime class, a wrapper around a 64-bit
 *    high precision absolute time value.
 */
#ifndef DateTime_H
#define DateTime_H

#include <ctime>

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include "text.h"

namespace syncit {

   typedef __int64 longlong;
   typedef unsigned __int64 ulonglong;

   /**
    * A DeltaTime is the difference between two absolute times.  It is represented
    * as a signed 64-bit quantity.  The abstract model shies away from explicitly
    * talking about the precision, other than milliseconds must be represented.
    * The concrete representation is 100 nanosecond intervals (10e-7 of a second).
    *
    * The operations on a DeltaTime:
    *    _dt_ + _dt_ = _dt_
    *    _dt_ - _dt_ = _dt_
    *
    *    _dt_ += _dt_
    *    _dt_ -= _dt_
    *
    *    _dt_  > _dt_
    *    _dt_ >= _dt_
    *    _dt_ == _dt_
    *    _dt_ != _dt_
    *    _dt_  < _dt_
    *    _dt_ <= _dt_
    */
   class DeltaTime {
      friend class DateTime;

   protected:
      DeltaTime(longlong i64) : m_i64(i64) {
      }

   public:
      DeltaTime() {
         m_i64 = 0;
      }

      DeltaTime(long seconds);

      static const DeltaTime SECOND;
      static const DeltaTime MINUTE;
      static const DeltaTime HOUR;
      static const DeltaTime DAY;

      longlong getMilliseconds() const;
      longlong getSeconds() const;

      DeltaTime plus(const DeltaTime &rhs) const {
         return DeltaTime(m_i64 + rhs.m_i64);
      }

      DeltaTime minus(const DeltaTime &rhs) const {
         return DeltaTime(m_i64 - rhs.m_i64);
      }

      DeltaTime &operator+=(const DeltaTime &rhs) {
         m_i64 += rhs.m_i64;
         return *this;
      }

      DeltaTime &operator-=(const DeltaTime &rhs) {
         m_i64 -= rhs.m_i64;
         return *this;
      }

      bool operator==(const DeltaTime &rhs) const {
         return m_i64 == rhs.m_i64;
      }

      bool operator!=(const DeltaTime &rhs) const {
         return m_i64 != rhs.m_i64;
      }

      bool operator<=(const DeltaTime &rhs) const {
         return m_i64 <= rhs.m_i64;
      }

      bool operator>=(const DeltaTime &rhs) const {
         return m_i64 >= rhs.m_i64;
      }

      bool operator<(const DeltaTime &rhs) const {
         return m_i64 < rhs.m_i64;
      }

      bool operator>(const DeltaTime &rhs) {
         return m_i64 > rhs.m_i64;
      }

   private:
      longlong m_i64;
   };

   /**
    * High-precision date/time value.
    *
    * The abstract model shies away from specifying either the resolution
    * (just like DeltaTime) or the base (the first representable date/time)
    *
    * The concrete model is the number of 100-nanosecond intervals since
    * Midnight, Jan 1, 1970.  This is the base time for the C run-time
    * library's time_t type, and is the same resolution as the Win32 FILETIME.
    * The value 0 is used as a marker for an invalid/unspecified date/time.
    */
   class DateTime {

   protected:
      DateTime(ulonglong t) : m_u64(t) {
      }

   public:
      /**
       * Create a date/time with no value (the earliest value
       * known)
       */
      DateTime() {
         m_u64 = 0;
      }

      /**
       * Create a date/time with a specified time_t value
       */
      DateTime(time_t t);

#ifdef _WIN32
      DateTime(const FILETIME &ft) {
         setFileTime(ft);
      }
#endif /* WIN32 */

      bool isValid() const {
         return m_u64 != 0;
      }

      /**
       * @return number of seconds since midnight, Jan 1, 1970; or 0
       *          if invalid/default time
       */
      time_t get_time_t() const;

      /**
       * Set the time value based on a Unix/CRTL time value.
       *
       * @param t  number of seconds since midnight, Jan 1, 1970
       */
      void set_time_t(time_t t);

      /**
       * Formats the date/time in W3C-recommended format,
       * 'yyyy-mm-ddThh:mm:ss[.nnnnnnn]Z'
       * <p>
       * If the date/time is rounded to the second, then no decimal
       * notation is used, otherwise the time is formatted to 7 decimal
       * places.
       * <p>
       * A note on i18n: the characters returned are pure iso646 (7-bit ASCII)
       *
       * @param pach   ptr to buffer to store formatted date/time
       * @param cch    length in characters of buffer
       *
       * @return 0 on error, number of characters copied if < cch, overflow if >= cch
       */
      size_t formatW3C(tchar_t *pach, size_t cch) const;

      /**
       * Set the date/time value by parsing a string.
       * Currently, only values that have been generated by formatW3C, or are
       * in the format:
       *     n+ '-' n+ '-' n+ 'T' n+ ':' n+ ':' n+ [ '.' n* ] 'Z'
       * can be parsed.
       *
       * @param psz  string in W3C date/time format to be parsed
       * @return true if successfully parsed and set, false if the date/time value hasn't changed
       */
      bool parseW3C(const tchar_t *psz);

      /**
       * Format into RFC-822 acceptable format:
       *   Tue, 01 Jun 1999 20:32:03 GMT
       */
      size_t format822(char *pach, size_t cch) const;

      bool parse822(const tchar_t *psz);

      enum FormatFlags {
         FormatDefault,    // 0

         FormatToday,      // 1
         FormatThisWeek,   // 2
         FormatFull,       // 3
         FormatMask = 0x07,

         FormatTimeZone = 0x08
      };

      /**
       * Formats the date/time into a format acceptable in the user's current
       * locale.
       *
       * Note on i18n: if TEXT16 is defined, then tchar_t is wchar_t, and the
       * result is in Unicode.  If not defined, then the result is encoded in the
       * current ANSI code page.
       */
      size_t format(tchar_t *pach, size_t cch, FormatFlags f = FormatDefault) const;

#ifdef _WIN32
      /**
       * Return the date/time in Win32 FILETIME format.
       *
       * If not valid (isValid() returns false), then the filetime is zero-filled.
       */
      FILETIME getFileTime() const;

      /**
       * Set the date/time based on the Win32 FILETIME format.
       */
      void setFileTime(const FILETIME &ft);
#endif /* _WIN32 */

      bool operator==(const DateTime &rhs) const {
         return m_u64 == rhs.m_u64;
      }

      bool operator!=(const DateTime &rhs) const {
         return m_u64 != rhs.m_u64;
      }

      bool operator<=(const DateTime &rhs) const {
         return m_u64 <= rhs.m_u64;
      }

      bool operator>=(const DateTime &rhs) const {
         return m_u64 >= rhs.m_u64;
      }

      bool operator<(const DateTime &rhs) const {
         return m_u64 < rhs.m_u64;
      }

      bool operator>(const DateTime &rhs) {
         return m_u64 > rhs.m_u64;
      }

      DateTime plus(const DeltaTime &dt) const {
         return DateTime(m_u64 + dt.m_i64);
      }

      DeltaTime minus(const DateTime &dt) const {
         return DeltaTime((longlong) (m_u64 - dt.m_u64));
      }

      DateTime minus(const DeltaTime &dt) const {
         return DateTime(m_u64 - dt.m_i64);
      }

      DateTime &operator+=(const DeltaTime &dt) {
         m_u64 += dt.m_i64;
         return *this;
      }

      DateTime &operator-=(const DeltaTime &dt) {
         m_u64 -= dt.m_i64;
         return *this;
      }

      static DateTime now();

   protected:
      bool preformat(SYSTEMTIME *pst, unsigned long *pulTenNanos) const;
      size_t postformat(const char *pszSrc, size_t cchSrc, tchar_t *pach, size_t cch) const;

   private:
      ulonglong m_u64;
   };

   inline DeltaTime operator+(const DeltaTime &lhs, const DeltaTime &rhs) {
      return lhs.plus(rhs);
   }

   inline DeltaTime operator-(const DeltaTime &lhs, const DeltaTime &rhs) {
      return lhs.minus(rhs);
   }

   inline DateTime operator+(const DateTime &lhs, const DeltaTime &rhs) {
      return lhs.plus(rhs);
   }

   inline DeltaTime operator-(const DateTime &lhs, const DateTime &rhs) {
      return lhs.minus(rhs);
   }

   inline DateTime operator-(const DateTime &lhs, const DeltaTime &rhs) {
      return lhs.minus(rhs);
   }
}

#endif /* DateTime_H */
