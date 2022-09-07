/*
 * SyncLib/URL.cxx
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
 *    Pick apart a URL (Uniform Resource Locator)
 */
#include <cstdlib>

#include "URL.h"
#include "util.h"

using namespace syncit;

const char URL::HTTP[] = "http:";
const char URL::SOCKS[] = "socks:";

void URL::combine(const URL &root, const char *pszUrl, size_t cchUrl, const char *pszProtocol) {
   size_t cchProtocol = lstrlenA(pszProtocol);

   m_pszProtocol = pszProtocol;

   if (strnicmp(pszUrl, pszProtocol, cchProtocol) == 0) {
      pszUrl += cchProtocol;
      cchUrl -= cchProtocol;
   }

   if (strncmp(pszUrl, "//", 2) == 0) {
      init0(pszUrl + 2, cchUrl - 2);
   }
   else if (pszUrl[0] == '/') {
      // pull host, port, use pszUrl for rest
      char achUrl[1024];
      unsigned short usPort = root.getPort(0);
      size_t cch;

      if (usPort == 0) {
         cch = wsprintfA(achUrl, "%s%s", root.getHostSz(), pszUrl);
      }
      else {
         cch = wsprintfA(achUrl, "%s:%d%s", root.getHostSz(), usPort, pszUrl);
      }

      init0(achUrl, cch);
   }
   else {
      // pull host, port, use pszUrl for rest
      char achUrl[1024];
      unsigned short usPort = root.getPort(0);

      if (usPort == 0) {
         wsprintfA(achUrl, "%s/%s", root.getHostSz(), root.getFileSz());
      }
      else {
         wsprintfA(achUrl, "%s:%d/%s", root.getHostSz(), usPort, root.getFileSz());
      }

      char *p = strrchr(achUrl, '/') + 1;
      size_t i = p - achUrl;

      init0(achUrl, i + bufcopy(pszUrl, p, 1024 - i));
   }
}

/**
 * @param pszUrl        pointer to null-terminated string
 * @param cchUrl        length of string referened by pszUrl
 * @param pszProtocol   default protocol to assume
 *
 * @require pszUrl[cchUrl] == 0
 */
void URL::init(const char *pszUrl, size_t cchUrl, const char *pszProtocol) {
   m_pszProtocol = pszProtocol;

   size_t cchProtocol = lstrlenA(pszProtocol);

   // skip protocol specification
   if (strnicmp(pszUrl, pszProtocol, cchProtocol) == 0) {
      pszUrl += cchProtocol;
      cchUrl -= cchProtocol;
   }

   // skip "//"
   if (pszUrl[0] == '/' && pszUrl[1] == '/') {
      pszUrl += 2;
      cchUrl -= 2;
   }

   init0(pszUrl, cchUrl);
}

void URL::init0(const char *pszUrl, size_t cchUrl) {
   m_len = cchUrl;
   m_pach = NEW char[m_len + 1];
   u_memcpy(m_pach, pszUrl, m_len + 1);

   char *p = m_pach;
   // p, m_pach points to start of host
   while (*p && *p != '/' && *p != ':')
      p++;

   // p points to end of host
   m_hostlen = p - m_pach;

   if (*p == ':') {
      *p++ = 0;
      m_port = p - m_pach;
      while (*p && *p != '/')
         p++;
   }
   else {
      m_port = 0;
   }

   // *p == 0 || *p == '/'
   if (*p == '/') {
      *p++ = 0;
   }

   m_file = p - m_pach;

   while (*p && *p != '#')
      p++;

   m_filelen = p - m_pach - m_file;

   if (*p == '#') {
      *p++ = 0;
   }

   m_hash = p - m_pach;
}

URL::URL(const URL &rhs) {
   m_pach = NEW char[rhs.m_len + 1];
   copy(rhs);
}

void URL::copy(const URL &rhs) {
   m_hostlen = rhs.m_hostlen;
   m_port = rhs.m_port;
   m_file = rhs.m_file;
   m_filelen = rhs.m_filelen;
   m_hash = rhs.m_hash;
   m_len  = rhs.m_len;
   u_memcpy(m_pach, rhs.m_pach, m_len + 1);

   m_pszProtocol = rhs.m_pszProtocol;
}

URL &URL::operator=(const URL &rhs) {
   if (rhs.m_len > m_len) {
      delete[] m_pach;
      m_pach = NEW char[rhs.m_len + 1];
   }

   copy(rhs);
   return *this;
}

unsigned short URL::getPort(unsigned short usDefault) const {
   if (m_port == 0) {
      return usDefault;
   }
   else {
      return (unsigned short) strtoul(m_pach + m_port, NULL, 10);
   }
}

size_t URL::format(char *pach, size_t cch) {
   size_t i = bufcopy(m_pszProtocol, pach, cch);

   if (i < cch - 2) {
      pach[i++] = '/';
      pach[i++] = '/';

      i += bufcopy(m_pach, m_hostlen, pach + i, cch + i);

      if (m_port != 0 && i < cch) {
         pach[i++] = ':';
         i += bufcopy(m_pach + m_port, pach + i, cch - i);
      }

      if (i < cch) {
         pach[i++] = '/';
         i += bufcopy(m_pach + m_file, m_filelen, pach + i, cch - i);

         if (m_hash != m_len && i < cch) {
            pach[i++] = '#';
            i += bufcopy(m_pach + m_hash, pach + i, cch - i);
         }
      }
   }

   return i;
}
