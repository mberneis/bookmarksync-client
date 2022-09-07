/*
 * SyncLib/Log.cxx
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
#include <new>

#include "Log.h"
#include "CriticalSection.h"
#include "Util.h"

#ifndef NLOG

using namespace syncit;

static HANDLE gh;

static unsigned __int64 lastMemoryId = 0;
static char gbLockId[sizeof(CriticalSection)];
static CriticalSection * gLockId;

void LogV(const char *pszFormat, va_list ap);

void syncit::LogInitialize(LPCSTR pszProgramName) /* throws Win32Exception */ {
   SYSTEMTIME st;
   char achLogFilename[MAX_PATH];

   GetSystemTime(&st);

   DWORD dw = GetRootDirectory(achLogFilename, ELEMENTS(achLogFilename));

   wsprintfA(achLogFilename + dw, "%s-%04d%02d%02d-%02d%02d%02d.log",
             pszProgramName,
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

   gh = CreateFile(achLogFilename,
                   GENERIC_WRITE,
                   FILE_SHARE_READ,
                   NULL,
                   CREATE_ALWAYS,
                   FILE_FLAG_SEQUENTIAL_SCAN,
                   NULL);
}

void syncit::LogLock() {
   if (gLockId == NULL) {
      gLockId = new(gbLockId) CriticalSection;
   }

   gLockId->enter();
}

void syncit::LogUnlock() {
   gLockId->leave();
}

void syncit::Log(const char *pszFormat, ...) {
   va_list ap;
   va_start(ap, pszFormat);
   LogV(pszFormat, ap);
   va_end(ap);
}

void LogV(const char *pszFormat, va_list ap) {
   char ach[1024];
   DWORD dwWritten;
   int i = wvsprintf(ach, pszFormat,ap);

   if (gh == NULL) {
      gh = GetStdHandle(STD_ERROR_HANDLE);
   }

   WriteFile(gh, ach, i, &dwWritten, NULL);
}

HANDLE syncit::GetLogHandle() {
   return gh;
}

#endif /* NLOG */
