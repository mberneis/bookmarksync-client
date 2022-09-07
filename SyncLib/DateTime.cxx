/*
 * SyncLib/DateTime.cxx
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
#include <cassert>
#include <cwchar>

#include "DateTime.h"
#include "Character.h"

using namespace syncit;

#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

static const longlong TIME_INTERVAL_SECOND = 10000000;
static const longlong TIME_INTERVAL_MILLIS = TIME_INTERVAL_SECOND / 1000;
static const longlong TIME_INTERVAL_MINUTE = 60 * TIME_INTERVAL_SECOND;
static const longlong TIME_INTERVAL_HOUR   = 60 * TIME_INTERVAL_MINUTE;
static const longlong TIME_INTERVAL_DAY    = 24 * TIME_INTERVAL_HOUR;

const DeltaTime DeltaTime::SECOND(TIME_INTERVAL_SECOND);
const DeltaTime DeltaTime::MINUTE(TIME_INTERVAL_MINUTE);
const DeltaTime DeltaTime::HOUR(TIME_INTERVAL_HOUR);
const DeltaTime DeltaTime::DAY(TIME_INTERVAL_DAY);


/**
 * The Windows C run-time library time_t type measures
 * seconds elapsed since midnight (00:00:00) January 1, 1970,
 * co-ordinated universal time (UTC)
 *
 * The Win32 FILETIME structure is the number of 100 nanosecond
 * intervals since January 1, 1601.
 *
 * The short program listed here shows how this value was calculated:
 * <pre>
 * SYSTEMTIME st;
 * FILETIME ft;
 * 
 * st.wYear = 1970;
 * st.wMonth = 1;
 * st.wDay = 1;
 * st.wHour = 0;
 * st.wMinute = 0;
 * st.wSecond = 0;
 * st.wMilliseconds = 0;
 * 
 * SystemTimeToFileTime(&st, &ft);
 * 
 * printf("static const ulonglong ROOT_FILETIME = 0x%08lX%08lX;\n",
 *        ft.dwHighDateTime, ft.dwLowDateTime);
 * </pre>
 */
static const ulonglong ROOT_FILETIME = 0x019DB1DED53E8000L;

static const char DAYS_OF_WEEK[][4] = {
   "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char MONTHS[][4] = {
   "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

const struct {
   char ach[4];
   int  hours;
} TIMEZONES[] = {
   { "UT",   0 }, { "GMT",  0 },
   { "EST", -5 }, { "EDT", -4 },
   { "CST", -6 }, { "CDT", -5 },
   { "MST", -7 }, { "MDT", -6 },
   { "PST", -8 }, { "PDT", -7 },
   { "Z",    0 }, { "A",   -1 },
   { "M",  -12 }, { "N",   +1 },
   { "Y",  +12 }
};

DateTime::DateTime(time_t t) {
   set_time_t(t);
}

time_t DateTime::get_time_t() const {
   return (time_t) (m_u64 / TIME_INTERVAL_SECOND);
}

void DateTime::set_time_t(time_t t) {
   m_u64 = (ulonglong) (t * TIME_INTERVAL_SECOND);
}

#ifdef _WIN32
FILETIME DateTime::getFileTime() const {
   union {
      ulonglong u64;
      FILETIME  ft;
   } u;

   if (m_u64 == 0) {
      u.u64 = 0;
   }
   else {
      u.u64 = m_u64 + ROOT_FILETIME;
   }

   return u.ft;
}

void DateTime::setFileTime(const FILETIME &ft) {
   union {
      ulonglong u64;
      FILETIME  ft;
   } u;

   u.ft = ft;

   if (u.u64 == 0) {
      m_u64 = 0;
   }
   else {
      m_u64 = u.u64 - ROOT_FILETIME;
   }
}

#endif /* _WIN32 */

bool DateTime::preformat(SYSTEMTIME *pst, unsigned long *pulTenNanos) const {
   if (m_u64 != 0) {

      FILETIME ft = getFileTime();
      if (FileTimeToSystemTime(&ft, pst)) {
         unsigned long ul = (unsigned long) (m_u64 % TIME_INTERVAL_SECOND);

         assert(ul / TIME_INTERVAL_MILLIS == pst->wMilliseconds);

         *pulTenNanos = ul;

         return true;
      }
   }

   return false;
}

size_t DateTime::postformat(const char *pszResult, size_t cchResult, tchar_t *pach, size_t cch) const {
   if (cch > 0) {
      /* leave room for null terminator */
      cch--;

      if (cchResult < cch) {
         cch = cchResult;
      }

      for (size_t i = 0; i < cch; i++) {
         pach[i] = pszResult[i];
      }

      pach[i] = 0;

      return i;
   }
   else {
      return 0;
   }
}

size_t DateTime::formatW3C(tchar_t *pach, size_t cch) const {
   SYSTEMTIME st;
   unsigned long ulTenNanos;

   if (preformat(&st, &ulTenNanos)) {
      char achBuffer[1024];
      size_t r = wsprintfA(achBuffer,
                           ulTenNanos == 0 ? "%d-%d-%dT%d:%02d:%02dZ" : "%d-%d-%dT%d:%02d:%02d.%07dZ",
                           st.wYear, st.wMonth, st.wDay,
                           st.wHour, st.wMinute, st.wSecond, ulTenNanos);

      return postformat(achBuffer, r, pach, cch);
   }
   else {
      return 0;
   }
}

class DateTimeParser {
   enum TokenType {
      End = 0,
      Error = -1,
      Number = -2,
      String = -3
   };

public:
   DateTimeParser(const char *psz) : m_psz(psz) {
   }

   bool parseW3C(FILETIME *pft, long *pminutes, unsigned long *pulTenNanos) {
      SYSTEMTIME st;
      if (nextNumber(&st.wYear) &&
          next() == '-' &&
          nextNumber(&st.wMonth) &&
          next() == '-' &&
          nextNumber(&st.wDay) &&
          next() == String && (m_ach[0] == 't' || m_ach[0] == 'T') && m_ach[1] == 0 &&
          nextNumber(&st.wHour) &&
          next() == ':' &&
          nextNumber(&st.wMinute)) {
         int t = next();

         if (t == ':') {
            if (!nextNumber(&st.wSecond)) { return false; }
            t = next();

            if (t == '.') {
               if (next() != Number) { return false; }

               int n = m_ndigits;
               unsigned long ul = m_ul;

               while (n > 7) {
                  ul /= 10;
                  n--;
               }

               while (n < 7) {
                  ul *= 10;
                  n++;
               }

               st.wMilliseconds = (unsigned short) (ul / 10000);
               *pulTenNanos = ul % 10000;

               t = next();
            }
            else {
               st.wMilliseconds = 0;
               *pulTenNanos = 0;
            }
         }

         else {
            st.wSecond = 0;
            st.wMilliseconds = 0;
            *pulTenNanos = 0;
         }

         if (t == '+' || t == '-') { 
            unsigned short usHour, usMin;

            if (nextNumber(&usHour) &&
                next() == ':' &&
                nextNumber(&usMin)) {
               *pminutes = (t == '+') ? usHour * 60 + usMin : usHour * -60 - usMin;
            }
            else {
               return false;
            }
         }
         else {
            *pminutes = 0;
         }

         return SystemTimeToFileTime(&st, pft) != FALSE;
      }
      else {
         return false;
      }
   }

   bool parse822(FILETIME *pft, long *pminutes) {
      SYSTEMTIME st;
      int t = next();

      if (t == String) {
         if (next() != ',') {
            return false;
         }

         t = next();
      }

      if (t != Number) { return false; }
      st.wDay = (unsigned short) m_ul;

      if (nextMonth(&st.wMonth) &&
          nextNumber(&st.wYear) &&
          nextNumber(&st.wHour) &&
          next() == ':' &&
          nextNumber(&st.wMinute)) {

         if (st.wYear < 90) {
            st.wYear += 2000;
         }
         else if (st.wYear < 100) {
            st.wYear += 1900;
         }

         int t = next();
         if (t == ':') {
            if (!nextNumber(&st.wSecond)) return false;
            t = next();
         }
         else {
            st.wSecond = 0;
         }

         st.wMilliseconds = 0;

         if (t == String) {
            // timezone
            for (int i = 0; i < ELEMENTS(TIMEZONES) && stricmp(m_ach, TIMEZONES[i].ach) != 0; i++)
               ;

            if (i == ELEMENTS(TIMEZONES)) return false;

            *pminutes = TIMEZONES[i].hours * 60;
         }
         else if (t == '+' || t == '-') {
            if (next() != Number) return false;

            if (m_ndigits <= 2) {
               *pminutes = (t == '+') ? m_ul * 60 : m_ul * -60;
            }
            else if (m_ndigits == 4) {
               long l = (m_ul / 100) * 60 + m_ul % 100;

               if (t == '-') l = 0 - l;

               *pminutes = l;
            }
            else {
               return false;
            }
         }
         else {
            *pminutes = 0;
         }

         return SystemTimeToFileTime(&st, pft) != FALSE;
      }
      else {
         return false;
      }
   }

private:
   bool nextMonth(unsigned short *pus) {
      if (next() == String) {
         for (int i = 1; i < ELEMENTS(MONTHS) && stricmp(m_ach, MONTHS[i]) != 0; i++)
            ;

         if (i == ELEMENTS(MONTHS)) { return false; }

         *pus = i;
         return true;
      }
      else {
         return false;
      }
   }

   bool nextNumber(unsigned short *pus) {
      if (next() == Number) {
         *pus = (unsigned short) m_ul;
         return true;
      }
      else {
         return false;
      }
   }

   int next() {
      const char *psz = m_psz;

      while (Character::isSpace(*psz)) {
         psz++;
      }

      // not space
      char ch = *psz;

      int r;

      if ('0' <= ch && ch <= '9') {
         const char *p = psz;
         m_ul = strtoul(p, (char **) &psz, 10);
         m_ndigits = psz - p;
         r = Number;
      }
      else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')) {
         int i = 0;

         do {
            m_ach[i++] = ch;
            ch = *++psz;
         } while (i < 3 && (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')));

         m_ach[i] = 0;

         while (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')) {
            ch = *++psz;
         }

         r = String;
      }
      else {
         r = ch;
         psz++;
      }

      m_psz = psz;
      return r;
   }

private:
   const char *m_psz;

   char m_ach[4];
   int m_ndigits;
   unsigned long m_ul;
};

/**
 * Format into RFC-822 acceptable format:
 *   Tue, 01 Jun 1999 20:32:03 GMT
 */
size_t DateTime::format822(char *pach, size_t cch) const {
   SYSTEMTIME st;
   unsigned long ulTenNanos;

   if (preformat(&st, &ulTenNanos)) {
      char achBuffer[1024];
      size_t r = wsprintfA(achBuffer,
                           "%s, %02d %s %d %02d:%02d:%02d GMT",
                           DAYS_OF_WEEK[st.wDayOfWeek],
                           st.wDay, MONTHS[st.wMonth], st.wYear,
                           st.wHour, st.wMinute, st.wSecond);

      return postformat(achBuffer, r, pach, cch);
   }
   else {
      return 0;
   }
}

bool DateTime::parseW3C(const tchar_t *psz) {
   DateTimeParser p(psz);
   FILETIME ft;
   long lTimezone;
   unsigned long ulTenNanos;

   if (p.parseW3C(&ft, &lTimezone, &ulTenNanos)) {
      setFileTime(ft);
      m_u64 = m_u64 - lTimezone * TIME_INTERVAL_MINUTE + ulTenNanos;
      return true;
   }
   else {
      return false;
   }
}

bool DateTime::parse822(const tchar_t *psz) {
   DateTimeParser p(psz);
   FILETIME ft;
   long lTimezone;

   if (p.parse822(&ft, &lTimezone)) {
      setFileTime(ft);
      m_u64 -= lTimezone * TIME_INTERVAL_MINUTE;
      return true;
   }
   else {
      return false;
   }
}

static DateTime::FormatFlags getFlags(const FILETIME &ftLocal, const SYSTEMTIME &stLocal) {
   SYSTEMTIME stLocalNow;
   FILETIME ftLocalNow;

   // Step 1.  Get current local date/time
   //
   GetLocalTime(&stLocalNow);
   SystemTimeToFileTime(&stLocalNow, &ftLocalNow);

   // Three possibilities:
   //       same date as today, don't show date, show seconds
   //       within the week, only show week day name, don't show seconds
   //       show full date, don't show seconds
   //
   if (stLocal.wYear  == stLocalNow.wYear &&
       stLocal.wMonth == stLocalNow.wMonth &&
       stLocal.wDay   == stLocalNow.wDay) {
      return DateTime::FormatToday;
   }

   // if within the week
   //
   else if (*(ulonglong *) &ftLocal > *(ulonglong *) &ftLocalNow -
                                       7 * TIME_INTERVAL_DAY) {
      return DateTime::FormatThisWeek;
   }
   else {
      return DateTime::FormatFull;
   }
}

/**
 * Format a UTC (Coordinated Universal Time) stored in a FILETIME structure
 * in the locale-specific date/time format.
 */
size_t DateTime::format(tchar_t *pach, size_t cch, FormatFlags f) const {

   size_t r;

   if (m_u64 != 0 && cch > 0) {

#ifdef TEXT16
      char achBuffer[1024];
      char *pachBuffer = achBuffer;
      size_t cchBuffer = sizeof(achBuffer);
#else
      char *pachBuffer = pach;
      size_t cchBuffer = cch;
#endif

      // Step 1.  Convert FILETIME into local FILETIME
      //
      FILETIME ftLocal, ft = getFileTime();
      SYSTEMTIME stLocal;

      FileTimeToLocalFileTime(&ft, &ftLocal);
      FileTimeToSystemTime(&ftLocal, &stLocal);

      int i;

      bool ftz = (f & FormatTimeZone) == FormatTimeZone;
      if ((f & FormatMask) == FormatDefault) {
         f = getFlags(ftLocal, stLocal);
      }

      // Three possibilities:
      //       same date as today, don't show date, show seconds
      //       within the week, only show week day name, don't show seconds
      //       show full date, don't show seconds
      //
      if (f == FormatToday) {
         r = 0;
      }
      else {
         const char *pszDateFormat;

         // if before a week
         //
         if (f == FormatThisWeek) {
            pszDateFormat = "ddd";
         }
         else {
            pszDateFormat = NULL;
         }

         r = GetDateFormat(LOCALE_USER_DEFAULT, // locale
                           0,                   // dwFlags
                           &stLocal,            // pDate
                           pszDateFormat,       // pszFormat
                           pachBuffer,          // pachDateBuffer
                           cchBuffer);          // cchDateBuffer

         pachBuffer[r - 1] = ' ';
      }

      i = GetTimeFormat(LOCALE_USER_DEFAULT, // locale
                        0,                   // dwFlags
                        &stLocal,            // pDate
                        NULL,                // pszFormat
                        pachBuffer + r,      // pachDateBuffer
                        cchBuffer - r);      // cchDateBuffer

      r += i;

      if (ftz) {
         TIME_ZONE_INFORMATION tz;
         DWORD dwTimeZoneId = GetTimeZoneInformation(&tz);

         pachBuffer[r - 1] = ' ';

         wchar_t *pwsz;

         switch (dwTimeZoneId) {
            case TIME_ZONE_ID_STANDARD:
               pwsz = tz.StandardName;
               break;

            case TIME_ZONE_ID_DAYLIGHT:
               pwsz = tz.DaylightName;
               break;

            default:
               pwsz = L"";
         }

         while (*pwsz && r < cchBuffer) {
            pachBuffer[r++] = (TCHAR) *pwsz++;
         }
      }

#ifdef TEXT16
      r = MultiByteToWideChar(CP_ACP,
                              0,
                              achBuffer,
                              o,
                              pach,
                              cch);

#else

#endif
   }
   else {
      r = 0;
   }

   if (r < cch)
      pach[r] = 0;

   return r;
}

DateTime DateTime::now() {
   FILETIME ft;

   GetSystemTimeAsFileTime(&ft);

   return DateTime(ft);
}


longlong DeltaTime::getMilliseconds() const {
   return m_i64 / TIME_INTERVAL_MILLIS;
}

longlong DeltaTime::getSeconds() const {
   return m_i64 / TIME_INTERVAL_SECOND;
}
