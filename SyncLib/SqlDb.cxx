/*
 * SyncLib/SqlDb.cxx
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
#include <sqlext.h>

#include "SqlDb.h"

using namespace syncit;

SqlGlobal::SqlGlobal() {
 //SQLSetEnvAttr(NULL, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER) SQL_CP_OFF, SQL_IS_INTEGER);
 //SQLSetEnvAttr(NULL, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER) SQL_CP_ONE_PER_HENV, SQL_IS_INTEGER);
   SQLSetEnvAttr(NULL, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER) SQL_CP_ONE_PER_DRIVER, SQL_IS_INTEGER);
}

SqlGlobal::~SqlGlobal() {
}

SqlGlobal SqlHandle::m_gData;

/* virtual */
size_t SqlError::format(char *pach, size_t cch) {
   size_t r = m_s.copy(pach, cch);
   if (r < cch) {
      pach[r] = 0;
   }
   return r;
}

BaseError *SqlError::newclone() const {
   return new SqlError(*this);
}

SqlHandle::SqlHandle(SQLSMALLINT hType) {
   trap(SQLAllocHandle(m_HandleType = hType,
                       SQL_NULL_HANDLE,
                       &m_Handle));
}

SqlHandle::SqlHandle(SQLSMALLINT hType, SqlHandle &parent) {
   parent.trap(SQLAllocHandle(m_HandleType = hType,
                              parent.m_Handle,
                              &m_Handle));
}

SQLRETURN SqlHandle::trap(SQLRETURN r) const {
   if (r == SQL_ERROR) {
      SQLRETURN r1;
      int i = 0;
      char achState[6], achText[1024];
      SQLSMALLINT cchText;
      SQLINTEGER nativeError;

      string expl;

      r1 = SQLGetDiagRec(m_HandleType, m_Handle, ++i,
                         (SQLTCHAR *) achState, &nativeError,
                         (SQLTCHAR *) achText, sizeof(achText),
                         &cchText);
      while (r1 == SQL_SUCCESS || r1 == SQL_SUCCESS_WITH_INFO) {
         expl.append(achState);
         expl.append(": ");
         expl.append(achText);
         expl.append("\n");

         r1 = SQLGetDiagRec(m_HandleType, m_Handle, ++i,
                            (SQLTCHAR *) achState, &nativeError,
                            (SQLTCHAR *) achText, sizeof(achText),
                            &cchText);
      }

      throw SqlError(expl);
   }
   else if (r == SQL_INVALID_HANDLE) {
      throw SqlError("invalid handle");
   }

   return r;
}

/* virtual */
SqlHandle::~SqlHandle() {
   trap(SQLFreeHandle(m_HandleType, m_Handle));
}

SqlEnv::SqlEnv() : SqlHandle(SQL_HANDLE_ENV) {
   setAttr(SQL_ATTR_ODBC_VERSION, SQL_OV_ODBC3);
}

SQLRETURN SqlEnv::setAttr(SQLINTEGER attribute, const char *pszValue) {
   return trap(SQLSetEnvAttr(m_Handle, attribute, (void *) pszValue, strlen(pszValue)));
}

SQLRETURN SqlEnv::setAttr(SQLINTEGER attribute, long lValue) {
   return trap(SQLSetEnvAttr(m_Handle, attribute, (void *) lValue, SQL_IS_INTEGER));
}

SqlConnection::SqlConnection(SqlEnv &env) : SqlHandle(SQL_HANDLE_DBC, env) {
   m_fConnected = false;
}

SqlConnection::~SqlConnection() {
   if (m_fConnected) {
      disconnect();
   }
}

SQLRETURN SqlConnection::setAttr(SQLINTEGER attribute, const char *pszValue) {
   return trap(SQLSetConnectAttr(m_Handle, attribute, (void *) pszValue, strlen(pszValue)));
}

SQLRETURN SqlConnection::setAttr(SQLINTEGER attribute, long lValue) {
   return trap(SQLSetConnectAttr(m_Handle, attribute, (void *) lValue, SQL_IS_INTEGER));
}

SQLRETURN SqlConnection::connect(const char *pszDsn, const char *pszUid, const char *pszAuthStr) {
   SQLRETURN r = trap(SQLConnect(m_Handle, (SQLTCHAR *) pszDsn, strlen(pszDsn),
                                 (SQLTCHAR *) pszUid, strlen(pszUid),
                                 (SQLTCHAR *) pszAuthStr, strlen(pszAuthStr)));

   m_fConnected = true;
   return r;
}

SQLRETURN SqlConnection::disconnect() {
   SQLRETURN r = trap(SQLDisconnect(m_Handle));
   m_fConnected = false;
   return r;
}

SqlStatement::SqlStatement(SqlConnection &c) : SqlHandle(SQL_HANDLE_STMT, c) {
}

SQLRETURN SqlStatement::execDirect(const char *pszSql) {
   return trap(SQLExecDirect(m_Handle, (SQLTCHAR *) pszSql, strlen(pszSql)));
}

SQLRETURN SqlStatement::bindCol(SQLUSMALLINT columnNumber, unsigned long *pul, SQLINTEGER *p) {
   return trap(SQLBindCol(m_Handle, columnNumber, SQL_C_ULONG, (void *) pul, sizeof(unsigned long), p));
}

SQLRETURN SqlStatement::bindCol(SQLUSMALLINT columnNumber, char *pach, size_t cch, SQLINTEGER *p) {
   return trap(SQLBindCol(m_Handle, columnNumber, SQL_C_CHAR, (void *) pach, cch, p));
}

SQLRETURN SqlStatement::fetch() {
   return trap(SQLFetch(m_Handle));
}

SQLRETURN SqlStatement::prepare(const char *pszSql) {
   return trap(SQLPrepare(m_Handle, (SQLTCHAR *) pszSql, strlen(pszSql)));
}

SQLRETURN SqlStatement::bindParameter(SQLUSMALLINT columnNumber, unsigned short *pus, SQLINTEGER *p) {
   return trap(SQLBindParameter(m_Handle, columnNumber, SQL_PARAM_INPUT,
                                SQL_C_USHORT, SQL_SMALLINT, 0, 0, pus, sizeof(unsigned short), p));
}

SQLRETURN SqlStatement::bindParameter(SQLUSMALLINT columnNumber, short *ps, SQLINTEGER *p) {
   return trap(SQLBindParameter(m_Handle, columnNumber, SQL_PARAM_INPUT,
                                SQL_C_SSHORT, SQL_SMALLINT, 0, 0, ps, sizeof(short), p));
}

SQLRETURN SqlStatement::bindParameter(SQLUSMALLINT columnNumber, unsigned long *pul, SQLINTEGER *p) {
   return trap(SQLBindParameter(m_Handle, columnNumber, SQL_PARAM_INPUT,
                                SQL_C_ULONG, SQL_INTEGER, 0, 0, pul, sizeof(unsigned long), p));
}

SQLRETURN SqlStatement::bindOutput(SQLUSMALLINT columnNumber, unsigned long *pul, SQLINTEGER *p) {
   return trap(SQLBindParameter(m_Handle, columnNumber, SQL_PARAM_OUTPUT,
                                SQL_C_ULONG, SQL_INTEGER, 0, 0, pul, sizeof(unsigned long), p));
}

SQLRETURN SqlStatement::bindParameter(SQLUSMALLINT columnNumber, long *pl, SQLINTEGER *p) {
   return trap(SQLBindParameter(m_Handle, columnNumber, SQL_PARAM_INPUT,
                                SQL_C_SLONG, SQL_INTEGER, 0, 0, pl, sizeof(long), p));
}

SQLRETURN SqlStatement::bindParameter(SQLUSMALLINT columnNumber, char *pach, size_t cch, SQLINTEGER *p) {
   return trap(SQLBindParameter(m_Handle, columnNumber, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR, cch, 0, pach, cch, p));
}

SQLRETURN SqlStatement::bindOutput(SQLUSMALLINT columnNumber, char *pach, size_t cch, SQLINTEGER *p) {
   return trap(SQLBindParameter(m_Handle, columnNumber, SQL_PARAM_OUTPUT,
                                SQL_C_CHAR, SQL_CHAR, cch, 0, pach, cch, p));
}

SQLRETURN SqlStatement::execute() {
   return trap(SQLExecute(m_Handle));
}

SQLRETURN SqlStatement::moreResults() {
   return trap(SQLMoreResults(m_Handle));
}

SQLRETURN SqlStatement::close() {
   return trap(SQLCloseCursor(m_Handle));
}
