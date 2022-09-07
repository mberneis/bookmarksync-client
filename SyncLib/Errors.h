/*
 * SyncLib/Errors.h
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
 *    A set of classes to be used by C++ exception handling.
 */
#ifndef ERRORS_H
#define ERRORS_H

#include <string>

#include "SyncITMsg.h"

namespace syncit {
   using std::string;

   class BaseError;
   class    FunctionError;
   class       CError;
   class       Win32Error;
// class       SocketError;   /* defined in SocketError.h   */
// class       SqlError;      /* defined in SqlError.h      */
   class    BaseEvent;
   class       IOError;
   class          FileError;
   class          NetError;
   class             UnknownHostError;
   class       RegError;
   class          RegValueError;
   class NestedError;

   class BaseError {
   protected:
      BaseError() {}

   public:
      virtual ~BaseError();

      virtual size_t format(char *pach, size_t cch) = 0;
      virtual BaseError *newclone() const = 0;
   };

   class NestedError {
   public:
      NestedError(const BaseError &nested) {
         m_nested = nested.newclone();
      }

      NestedError(const NestedError &rhs) {
         m_nested = rhs.m_nested->newclone();
      }

      NestedError &operator=(const NestedError &rhs) {
         delete m_nested;
         m_nested = rhs.m_nested->newclone();
      }

      ~NestedError() {
         delete m_nested;
      }

      size_t format(char *pach, size_t cch) {
         return m_nested->format(pach, cch);
      }

   private:
      BaseError *m_nested;
   };

   class FunctionError : public BaseError {
   protected:
      FunctionError(const string &function) : m_function(function) {}

   public:
      const string &getFunction() const {
         return m_function;
      }

   private:
      string m_function;
   };

   class CError : public FunctionError {
   public:
      CError(const string &function);
      CError(const string &function, int error) : FunctionError(function), m_errno(error) {}

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

      int getError() const {
         return m_errno;
      }

   private:
      int m_errno;
   };

   class Win32Error : public FunctionError {
   public:
      Win32Error(const string &function);
      Win32Error(const string &function,
                 unsigned long ulError) : FunctionError(function), m_ulError(ulError) {}

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

      unsigned long getError() const {
         return m_ulError;
      }

   private:
      unsigned long m_ulError;
   };

   class BaseEvent : public BaseError {
   public:
      BaseEvent(unsigned long ulEventCode) : m_ulEventCode(ulEventCode) {}

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

      unsigned long getEventCode() const {
         return m_ulEventCode;
      }

   protected:
      size_t format0(char *pach, size_t cch, ...);
      size_t vformat(char *pach, size_t cch, va_list ap);

   private:
      unsigned long m_ulEventCode;
   };

   class HttpError : public BaseEvent {
   public:
      HttpError(unsigned code, const string &expl) : BaseEvent(SYNCIT_HTTP_ERR), m_code(code), m_expl(expl)  {
      }

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

      unsigned getCode() const { return m_code; }
      const string &getExplanation() const { return m_expl; }

   private:
      unsigned m_code;
      string   m_expl;
   };

   class HttpAuthenticationError : public HttpError {
   public:
      HttpAuthenticationError(unsigned code, const string &expl, const string &domain) : HttpError(code, expl), m_domain(domain) {
      }

      const string &getDomain() const {
         return m_domain;
      }

      virtual BaseError *newclone();

   private:
      string m_domain;
   };

   class ServerError : public BaseEvent {
   public:
      ServerError(const string &expl) : BaseEvent(SYNCIT_SERVER_ERR), m_expl(expl) {
      }

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

      const string &getExplanation() const { return m_expl; }

   private:
      string m_expl;
   };

   class ProtocolError : public BaseEvent {
   public:
      ProtocolError(const string &expl) : BaseEvent(SYNCIT_PROTOCOL_ERR), m_expl(expl) {
      }

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

      const string &getExplanation() const { return m_expl; }

   private:
      string m_expl;
   };

   class IOError : public BaseEvent {
   protected:
      IOError(unsigned long ulEventCode) : BaseEvent(ulEventCode) {}
   };

   class FileError : public IOError {

   public:
      enum EventCode {
         Access = SYNCIT_FILE_ERR,
         Create = SYNCIT_CREATE_ERR,
         Open   = SYNCIT_OPEN_ERR,
         Read   = SYNCIT_READ_ERR,
         Write  = SYNCIT_WRITE_ERR,
         Close  = SYNCIT_CLOSE_ERR,
      };

      FileError(EventCode f, const string &filename, const BaseError &nested);

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

   private:
      string m_filename;
      NestedError m_nested;
   };

   class NetError : public IOError {
   public:
      enum EventCode {
         Connect = SYNCIT_NETCONNECT_ERR,
         Read    = SYNCIT_NETREAD_ERR,
         Write   = SYNCIT_NETWRITE_ERR,
         Close   = SYNCIT_NETCLOSE_ERR,
         Name    = SYNCIT_HOSTNAME_ERR
      };

      NetError(EventCode f, const string &hostname, const BaseError &nested) : IOError(f), m_hostname(hostname), m_nested(nested) {
      }

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

      const string &getHostname() const {
         return m_hostname;
      }

   private:
      string m_hostname;
      NestedError m_nested;
   };

   class UnknownHostError : public NetError {
   public:
      UnknownHostError(const string &hostname, const BaseError &nested) : NetError(Name, hostname, nested) {}

   };

   class RegError : public BaseEvent {

   public:
      enum EventCode {
         Access = SYNCIT_REGKEY_ERR,
         Create = SYNCIT_CREATEKEY_ERR,
         Open   = SYNCIT_OPENKEY_ERR,
         Query  = SYNCIT_REGQUERY_ERR
      };

      RegError(EventCode f, const string &keyname, const BaseError &nested);

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

   protected:
      string m_keyname;
      NestedError m_nested;
   };

   class RegValueError : public RegError {
   public:
      RegValueError(EventCode f, const string &valuename, const BaseError &nested);
      RegValueError(EventCode f, const string &keyname, const string &valuename, const BaseError &nested);

      virtual size_t format(char *pach, size_t cch);
      virtual BaseError *newclone() const;

   private:
      string m_valuename;
   };

   bool Ignore(BaseError &e);
}

#endif /* ERRORS_H */
