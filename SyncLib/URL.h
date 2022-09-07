/*
 * SyncLib/URL.h
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
#ifndef URL_H
#define URL_H

#include "text.h"

namespace syncit {

   using std::string;

   class URL {
   public:
      static const char HTTP[];
      static const char SOCKS[];

      URL(const URL &root, const char *pszUrl, const char *pszProtocol = HTTP) {
         combine(root, pszUrl, strlen(pszUrl), pszProtocol);
      }

      URL(const URL &root, const string &url, const char *pszProtocol = HTTP) {
         combine(root, url.c_str(), url.size(), pszProtocol);
      }

      URL(const char *pszUrl, const char *pszProtocol = HTTP) {
         init(pszUrl, strlen(pszUrl), pszProtocol);
      }

      URL(const string &rhs, const char *pszProtocol = HTTP) {
         init(rhs.c_str(), rhs.size(), pszProtocol);
      }

      URL(const URL &rhs);
      URL &operator=(const URL &rhs);

      ~URL() {
         delete[] m_pach;
      }

      const char *getHostSz() const { return m_pach; }
      size_t getHostLen() const { return m_hostlen; }

      unsigned short getPort(unsigned short usDefault = 80) const;
      const char *getPortSz(const char *pszDefault = "80") const { return m_port == 0 ? pszDefault : m_pach + m_port; }

      const char *getFileSz() const { return m_pach + m_file; }
      size_t getFileLen() const { return m_filelen; }

      const char *getAnchorSz() const { return m_pach + m_hash; }
      size_t getAnchorLen() const { return m_len - m_hash; }

      size_t getHost(char *pach, size_t cch) const {
         return bufcopy(getHostSz(), getHostLen(), pach, cch);
      }

      string getFile()   const { return string(getFileSz(),   getFileLen());   }
      string getAnchor() const { return string(getAnchorSz(), getAnchorLen()); }

      size_t format(char *pach, size_t cch);

   private:
      void combine(const URL &root, const char *pszUrl, size_t cchUrl, const char *pszProtocol);
      void init(const char *psz, size_t cch, const char *pszProtocol);
      void init0(const char *psz, size_t cch);
      void copy(const URL &rhs);

      char *m_pach;
      unsigned short         m_hostlen;
      unsigned short m_port;              // offset to start of port, == 0 if not spec'ed
      unsigned short m_file, m_filelen;   // offset to start, length, of file WITHOUT '/'
      unsigned short m_hash;  // offset to start of internal link WITHOUT '#'
      unsigned short m_len;   // offset to null, end of URL

      const char *m_pszProtocol;
   };
}

#endif /* URL_H */
