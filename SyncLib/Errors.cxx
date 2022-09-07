/*
 * SyncLib/Errors.cxx
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>

#include <cerrno>

#include "Errors.h"
#include "util.h"
#include "text.h"

using namespace syncit;

/* virtual */
BaseError::~BaseError() {
}

CError::CError(const string &function) : FunctionError(function), m_errno(errno) {
}

/* virtual */
size_t CError::format(char *pach, size_t cch) {
   return bufcopy(strerror(m_errno), pach, cch);
}

/* virtual */
BaseError *CError::newclone() const {
   return NEW CError(*this);
}

Win32Error::Win32Error(const string &function) : FunctionError(function) {
   m_ulError = ::GetLastError();
}

/* virtual */
size_t Win32Error::format(char *pach, size_t cch) {
   return FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        m_ulError,
                        0,
                        pach,
                        cch,
                        NULL);
}

/* virtual */
BaseError *Win32Error::newclone() const {
   return NEW Win32Error(*this);
}

/* virtual */
size_t BaseEvent::format(char *pach, size_t cch) {
   return format0(pach, cch);
}

/* virtual */
BaseError *BaseEvent::newclone() const {
   return NEW BaseEvent(*this);
}

size_t BaseEvent::vformat(char *pach, size_t cch, va_list ap) {
   return FormatMessage(80 | FORMAT_MESSAGE_FROM_HMODULE,
                        NULL,
                        m_ulEventCode,
                        0,
                        pach,
                        cch,
                        &ap);
}

size_t BaseEvent::format0(char *pach, size_t cch, ...) {
   va_list ap;

   va_start(ap, cch);
   size_t r = vformat(pach, cch, ap);
   va_end(ap);

   return r;
}

/* virtual */
size_t HttpError::format(char *pach, size_t cch) {
   char achCode[11];

   wsprintfA(achCode, "%u", m_code);
   return format0(pach, cch, achCode, m_expl.c_str());
}

/* virtual */
BaseError *HttpError::newclone() const {
   return NEW HttpError(*this);
}

/* virtual */
BaseError *HttpAuthenticationError::newclone() {
   return NEW HttpAuthenticationError(*this);
}

/* virtual */
size_t ServerError::format(char *pach, size_t cch) {
   return format0(pach, cch, m_expl.c_str());
}

/* virtual */
BaseError *ServerError::newclone() const {
   return NEW ServerError(*this);
}

/* virtual */
size_t ProtocolError::format(char *pach, size_t cch) {
   return format0(pach, cch, m_expl.c_str());
}

/* virtual */
BaseError *ProtocolError::newclone() const {
   return NEW ProtocolError(*this);
}

FileError::FileError(EventCode f, const string &filename, const BaseError &nested) : IOError(f), m_filename(filename), m_nested(nested) {
}

/* virtual */
size_t FileError::format(char *pach, size_t cch) {
   size_t r = format0(pach, cch, m_filename.c_str());

   if (r + 3 < cch) {
      pach[r++] = '\r';
      pach[r++] = '\n';
      r += m_nested.format(pach + r, cch - r);
   }

   return r;
}

/* virtual */
BaseError *FileError::newclone() const {
   return NEW FileError(*this);
}

/* virtual */
size_t NetError::format(char *pach, size_t cch) {
   size_t r = format0(pach, cch, m_hostname.c_str());

   if (r + 3 < cch) {
      pach[r++] = '\r';
      pach[r++] = '\n';
      r += m_nested.format(pach + r, cch - r);
   }

   return r;
}
/* virtual */
BaseError *NetError::newclone() const {
   return NEW NetError(*this);
}

RegError::RegError(EventCode f,
                   const string &keyname,
                   const BaseError &nested) : BaseEvent(f), m_keyname(keyname), m_nested(nested) {
}

/* virtual */
size_t RegError::format(char *pach, size_t cch) {
   size_t r = format0(pach, cch, m_keyname.c_str());

   if (r + 3 < cch) {
      pach[r++] = '\r';
      pach[r++] = '\n';
      r += m_nested.format(pach + r, cch - r);
   }

   return r;
}

/* virtual */
BaseError *RegError::newclone() const {
   return NEW RegError(*this);
}

RegValueError::RegValueError(EventCode f,
                             const string &valuename,
                             const BaseError &nested) : RegError(f, "?", nested), m_valuename(valuename) {
}

RegValueError::RegValueError(EventCode f,
                             const string &keyname,
                             const string &valuename,
                             const BaseError &nested) : RegError(f, keyname, nested), m_valuename(valuename) {
}

/* virtual */
size_t RegValueError::format(char *pach, size_t cch) {
   size_t r = format0(pach, cch, m_keyname.c_str(), m_valuename.c_str());

   if (r + 3 < cch) {
      pach[r++] = '\r';
      pach[r++] = '\n';
      r += m_nested.format(pach + r, cch - r);
   }

   return r;
}

/* virtual */
BaseError *RegValueError::newclone() const {
   return NEW RegValueError(*this);
}
