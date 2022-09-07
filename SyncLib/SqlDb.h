/*
 * SyncLib/SqlDb.h
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
#ifndef SqlDb_H
#define SqlDb_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "Errors.h"

namespace syncit {

   class SqlGlobal {
   public:
      SqlGlobal();
      ~SqlGlobal();
   private:
      bool m_fInitialized;
   };

   /**
    * The SqlException class is for exceptions that are thrown
    * as a result of ODBC errors.
    */
   class SqlError : public BaseError {
   public:
      SqlError(const string &s) : m_s(s) {
      }

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

   private:
      string m_s;
   };

   class SqlHandle {
   protected:
      SqlHandle(SQLSMALLINT hType);
      SqlHandle(SQLSMALLINT hType, SqlHandle &parent);

      SQLRETURN trap(SQLRETURN r) const;

   public:
      virtual ~SqlHandle();

   protected:
      SQLSMALLINT m_HandleType;
      SQLHANDLE m_Handle;

      static SqlGlobal m_gData;

   private:
      SqlHandle(SqlHandle &rhs);
      SqlHandle &operator=(SqlHandle &rhs);
   };


   class SqlEnv : protected SqlHandle {
      friend class SqlConnection;

   public:
      SqlEnv();

      SQLRETURN setAttr(SQLINTEGER attribute, const char *pszValue);

      SQLRETURN setAttr(SQLINTEGER attribute, long lValue);
   };

   class SqlConnection : protected SqlHandle {
      friend class SqlStatement;

   public:
      SqlConnection(SqlEnv &env);
      ~SqlConnection();

      SQLHDBC getHandle() const {
         return m_Handle;
      }

      SQLRETURN setAttr(SQLINTEGER attribute, const char *pszValue);
      SQLRETURN setAttr(SQLINTEGER attribute, long lValue);

      SQLRETURN connect(const char *pszDsn, const char *pszUid, const char *pszAuthStr);
      SQLRETURN disconnect();

   private:
      bool m_fConnected;
   };

   class SqlStatement : protected SqlHandle {
   public:
      SqlStatement(SqlConnection &c);
      SQLRETURN execDirect(const char *pszSql);
      SQLRETURN bindCol(SQLUSMALLINT columnNumber, unsigned long *pul, SQLINTEGER *p);
      SQLRETURN bindCol(SQLUSMALLINT columnNumber, char *pach, size_t cch, SQLINTEGER *p);
      SQLRETURN fetch();

      SQLRETURN prepare(const char *pszSql);
      SQLRETURN bindParameter(SQLUSMALLINT columnNumber, unsigned short *pus, SQLINTEGER *p);
      SQLRETURN bindParameter(SQLUSMALLINT columnNumber, short *ps, SQLINTEGER *p);
      SQLRETURN bindParameter(SQLUSMALLINT columnNumber, unsigned long *pul, SQLINTEGER *p);
      SQLRETURN bindParameter(SQLUSMALLINT columnNumber, long *pl, SQLINTEGER *p);
      SQLRETURN bindParameter(SQLUSMALLINT columnNumber, char *pach, size_t cch, SQLINTEGER *p);

      SQLRETURN bindOutput(SQLUSMALLINT columnNumber, unsigned long *pul, SQLINTEGER *p);
      SQLRETURN bindOutput(SQLUSMALLINT columnNumber, char *pach, size_t cch, SQLINTEGER *p);
      SQLRETURN execute();

      SQLRETURN moreResults();

      SQLRETURN close();
   };

}

#endif /* SqlDb_H */
