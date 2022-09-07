/*
 * SyncIT/SyncProtocol.cxx
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
#include "Synchronizer.h"
#include "BuiltinImages.h"

#include "SyncLib/HttpRequest.h"
#include "SyncLib/PrintWriter.h"
#include "SyncLib/Socket.h"
#include "SyncLib/SocketError.h"
#include "SyncLib/URL.h"
#include "SyncLib/Log.h"
#include "SyncLib/CsvParser.h"
#include "SyncLib/BitmapFileImage.h"
#include "SyncLib/FileInputStream.h"
#include "SyncLib/FileOutputStream.h"

#include "BookmarkLib/BookmarkEditor.h" // for useful startBookmark(...) function

#include "ProductVersion.h"

using namespace syncit;

static int readFolder(BookmarkContext *pc,
                      BufferedInputStream *in,
                      const URL &base);

static int skip(BufferedInputStream *in);
static void SetImages(BookmarkContext *pc, const URL &url, const char *pszOpen, const char *pszClosed);

static char gachMap[] = { '\\', '_' };

class PostBookmarks : public BookmarkDifferences {


#ifndef NDEBUG
public:
   bool isValid() const {
      return strcmp(m_achStartTag, "PostBookmarks") == 0 &&
             strcmp(m_achEndTag, "PostBookmarks") == 0;
   }
private:
   char m_achStartTag[sizeof("PostBookmarks")];
#endif /* NDEBUG */

public:
   enum Command {
      ADD = 'A',
      DEL = 'D',

      MKDIR = 'M',
      RMDIR = 'R'
   };

   PostBookmarks(PrintWriter *p) {
#ifndef NDEBUG
      strcpy(m_achStartTag, "PostBookmarks");
      strcpy(m_achEndTag, "PostBookmarks");
#endif /* NDEBUG */

      m_achPath[0] = 0;
      m_iPath = 0;

      m_p = p;

      assert(isValid());
   }

   ~PostBookmarks() {
      assert(isValid());

#ifndef NDEBUG
      m_achStartTag[0] = m_achEndTag[0] = '\0';
#endif /* NDEBUG */
   }

   virtual void pushFolder(const BookmarkFolder *pbf) {
      m_achPath[m_iPath++] = '\\';
      m_iPath += Encode(pbf->getName(), '%', gachMap, ELEMENTS(gachMap), m_achPath + m_iPath, ELEMENTS(m_achPath) - m_iPath);
   }

   virtual void popFolder() {
      int i;

      for (i = m_iPath - 1; i > 0 && m_achPath[i] != '\\'; --i)
         ;

      // i == 0 || m_achPath[i] == '\\'
      m_iPath = i;
      m_achPath[i] = 0;
   }

   virtual void addBookmark(const Bookmark *pNew) {
      appendBookmark(ADD, pNew);
   }

   virtual void delBookmark(const Bookmark *pOld) {
      appendBookmark(DEL, pOld);
   }

   void appendBookmark(Command c, const Bookmark *pb) {
      char ach[4096];
      size_t cch = pb->getHref().format(ach, sizeof(ach));

      m_p->write((char) c);
      m_p->write(',');
      m_p->write('"');
      writePath(pb);

      m_p->print("\",\"");
      quote(ach, cch);
      m_p->print("\"\r\n");
   }

   void add0(const BookmarkFolder *pNew) {
      appendFolder(MKDIR, pNew);
   }

   void del0(const BookmarkFolder *pOld) {
      appendFolder(RMDIR, pOld);
   }

   void appendFolder(Command c, const BookmarkFolder *pbf) {
      m_p->write((char) c);
      m_p->write(',');
      m_p->write('"');
      writePath(pbf);
      m_p->print("\\\"\r\n");
   }

   void writePath(const BookmarkItem *pbi) {
      tchar_t achElement[512];
      size_t cch;

      assert(lstrlenA(m_achPath) == m_iPath);
      quote(m_achPath, m_iPath);
      m_p->write('\\');
      cch = Encode(pbi->getName(), '%', gachMap, ELEMENTS(gachMap), achElement, ELEMENTS(achElement));
      quote(achElement, cch);
   }

   void quote(const char *psz, size_t cch) {
      const char *e = psz + cch;
      while (psz != e) {
         char ch = *psz++;

         if (ch == '"') {
            m_p->write('"');
         }
         
         m_p->write(ch);
      }
   }

private:
   char m_achPath[1024];
   int  m_iPath;

   PrintWriter *m_p;

   // disable copy constructor and assignment operator
   PostBookmarks(const PostBookmarks &rhs);
   PostBookmarks &operator=(const PostBookmarks &rhs);

#ifndef NDEBUG
private:
   char m_achEndTag[sizeof("PostBookmarks")];
#endif /* NDEBUG */

};

/**
 * Informs the server of the set of changes necessary to turn
 * m_pOnServer (the current set of bookmarks on the server) into <i>m_pOnDisk</i>
 * <p>
 * The server will then send down its set of bookmarks, which we'll
 * store.
 */
Synchronizer::AsyncResponse Synchronizer::asyncRequest() {
   AsyncResponse result;
   BookmarkModel *pbm = NEW BookmarkModel();
   BookmarkContext bc(pbm, gpLoader);
   PostOutputStream req;

   setImages(&bc);
   bc.pushFolder();

   // EMail
   // Pass
   // Version
   // Token
   // Contents (can be empty)
   //
   PrintWriter p(&req);

   req.nextVariable("Email");
   p.print(getEmail());

   req.nextVariable("md5");
   p.print(m_pszPWHash);

   req.nextVariable("Version");
   p.print(SYNCIT_V_STR);

   req.nextVariable("Token");
   p.print(getSeqNo());

   BookmarkVector::const_iterator i = m_pSubscriptions->begin(), end = m_pSubscriptions->end();

   while (i != end) {
      if ((*i)->isFolder()) {
         const BookmarkFolder *pbf = (const BookmarkFolder *) (*i);

         if (pbf->isSubscription()) {
            const BookmarkSubscription *pbs = (const BookmarkSubscription *) pbf;

            char achVariable[1024];
            wsprintfA(achVariable, "token%s", pbs->getId());
            req.nextVariable(achVariable);
            p.print(pbs->getSeqNo());
         }
      }

      i++;
   }

   req.nextVariable("CharSet");
   p.print("cp");
   p.print((unsigned long) (AreFileApisANSI() ? GetACP() : GetOEMCP()));

   req.nextVariable("Contents");

   switch (m_state) {
      case State_INIT:
      case State_LOGIN:
         m_wiz.checkpoint(IDC_WIZ_CONNECTING);
         result = submit(&bc, &req);

         break;

      case State_SYNC:
      case State_SYNCW:
         m_wiz.checkpoint(IDC_WIZ_UPLOADING);
         {
            DateTime dt = DateTime::now();

            PostBookmarks post(&p);

            m_cs.enter();
            diff(m_pC, m_bookmarks, &post);
            m_cs.leave();

            result = submit(&bc, &req);

            if (result == OK || result == NoChange) {
               m_dtLastSynced = dt;
            }
         }
         break;

      default:
         assert(m_state == State_INIT ||
                m_state == State_LOGIN ||
                m_state == State_SYNCW ||
                m_state == State_SYNC);
   }

   response(result, pbm);

   return result;
}

Synchronizer::AsyncResponse Synchronizer::submit(BookmarkContext *pc,
                                                 PostOutputStream *preq) {
   AsyncResponse r;
   
   do {
      r = submit0(pc, preq);
   } while (r == ProxyAuthenticate || r == Wait);

   return r;
}

// m_pOnDisk is the set of bookmark folders currently on disk
// it is never NULL.
//
// m_pOnServer is the set of bookmark folders at the time we
// last sent up a change.
// It may be equal to m_pOnDisk.

Synchronizer::AsyncResponse Synchronizer::submit0(BookmarkContext *pc, PostOutputStream *pdata) {
   AsyncResponse result = ConnectError;

   HttpRequest *preq = NULL;

   try {
      int r;

      preq = NewHttpRequest();
      result = SocketError;

      preq->setAuthentication(m_proxy.getUsername(), m_proxy.getPassword());

      m_wiz.checkpoint(IDC_WIZ_DOWNLOADING);

      r = preq->send("POST", m_post, pdata);

      if (r / 100 == 2) {
         result = parseResponse(preq->getInputStream(), m_root.getHostSz(), pc);
      }
      else if (r == 401 || r == 407) {
         m_proxy.setAutoSyncEnabled(m_fAuto);

         const char *psz = preq->getProxyAuthenticate().c_str();
         if (psz == NULL || *psz == 0)
            psz = preq->getWWWAuthenticate().c_str();

         if (m_proxy.show(m_status.getWindow(),
                          m_root.getHostSz(),
                          psz)) {
            m_fAuto = m_proxy.isAutoSyncEnabled();
            m_status.setAutoSyncCheck(m_fAuto);
            result = ProxyAuthenticate;
         }
         else {
            result = HttpError;
            m_status.setStatus(IDS_HTTP_RESPONSE, m_root.getHostSz(), preq->getResponse());
         }
      }
      else {
         result = HttpError;
         //m_status.setState();
         m_status.setStatus(IDS_HTTP_RESPONSE, m_root.getHostSz(), preq->getResponse());
       //m_wiz.bad(LoginWizard::Panel_Ready, IDS_HTTP_RESPONSE, achHost, achBuffer);
      }
   }
   catch (UnknownHostError &e) {
      TCHAR achError[1024];

      result = UnknownHost;
      e.format(achError, ELEMENTS(achError));
      m_status.setStatus(IDS_HOSTNAME_ERR, m_root.getHostSz(), achError);
    //m_wiz.bad(LoginWizard::Panel_Ready, IDS_HOSTNAME_ERR, achHost, achError);
   }
   catch (BaseError &e) {
      TCHAR achError[1024];

      e.format(achError, ELEMENTS(achError));
      m_status.setStatus(result == ConnectError ? IDS_CONNECT_ERR : IDS_SOCKET_ERR, m_root.getHostSz(), achError);
    //m_wiz.bad(LoginWizard::Panel_Ready, result == ConnectError ? IDS_CONNECT_ERR : IDS_SOCKET_ERR, achHost, achError);
   }

   delete preq;

   return result;
}

#define MakeVer(major,minor) (((major)<<16)|(minor))

static unsigned long ParseVer(const char *psz) {
   char *p;

   unsigned short us1 = (unsigned short) strtoul(psz, &p, 10), us2;
   if (*p == '.') {
      us2 = (unsigned short) strtoul(p + 1, NULL, 10);
   }
   else {
      us2 = 0;
   }

   return MakeVer(us1, us2);
}

#ifndef NLOG
static void DumpCsv(const char * const*papsz, int cpsz) {
   for (int i = 0; i < cpsz; i++) {
      if (i > 0) {
         Log(",");
      }

      if (papsz[i] != NULL) {
         Log("\"%s\"", papsz[i]);
      }
   }

   Log("\r\n");
}
#endif /* NLOG */

inline bool IsDriveUnmapped(DWORD dwMap, char ch) {
   return (dwMap & (1 << (ch - 'A'))) == 0;
}

static char GetNextDrive(char ch) {
   DWORD dwMap = GetLogicalDrives();

   if ('a' <= ch && ch <= 'z') {
      ch = (ch - 'a') + 'A';
   }

   if ('A' <= ch && ch <= 'Z' && IsDriveUnmapped(dwMap, ch)) return ch;

   for (ch = 'S'; ch <= 'Z'; ch++) {
      if (IsDriveUnmapped(dwMap, ch)) return ch;
   }

   for (ch = 'R'; ch > 'C'; ch--) {
      if (IsDriveUnmapped(dwMap, ch)) return ch;
   }

   return 0;
}

class NetworkDrive {

public:
   NetworkDrive(const char *const *apsz, int npsz) {
      m_nr.dwType = RESOURCETYPE_DISK;
      m_nr.lpRemoteName = tstralloc(apsz[0]);

      m_achBuf[0] = GetNextDrive(npsz > 1 ? apsz[1][0] : '*');
      m_achBuf[1] = ':';
      m_achBuf[2] = 0;
      m_nr.lpLocalName = m_achBuf;

      m_nr.lpProvider = NULL;

      m_pszUsername = npsz > 2 ? tstralloc(apsz[2]) : NULL;
      m_pszPassword = npsz > 3 ? tstralloc(apsz[3]) : NULL;
   }

   ~NetworkDrive() {
      u_free0(m_pszUsername);
      u_free0(m_pszPassword);
   }

   bool matches(const char *pszRemote) const {
      return stricmp(pszRemote, m_nr.lpRemoteName) == 0;
   }

   bool matches(const char **apsz, int npsz) const {
      if (!matches(apsz[0])) return false;
      if (!matches(m_pszUsername, apsz, npsz, 2)) return false;
      if (!matches(m_pszPassword, apsz, npsz, 3)) return false;

      return true;
   }

   static bool matches(char *pszLocal, const char **apsz, int npsz, int n) {
      // if both are null
      //
      if (pszLocal == NULL && n >= npsz) {
         return true;
      }

      // if one is null
      //
      if (pszLocal == NULL) {
         return false;
      }

      if (n >= npsz) {
         return false;
      }

      return stricmp(pszLocal, apsz[n]) == 0;
   }

private:
   NETRESOURCE m_nr;

   char *m_pszUsername;
   char *m_pszPassword;

   char m_achBuf[4];
};

Synchronizer::AsyncResponse Synchronizer::parseResponse(BufferedInputStream *in,
                                                        LPCTSTR pszHost,
                                                        BookmarkContext *pc) /* throws SocketError */ {
   AsyncResponse result = NoChange;
   BookmarkModel *pSubscriptions = NEW BookmarkModel();
   BookmarkContext bc(pSubscriptions, gpLoader);

   char *apszCommand[6];
   long seqno = -1;

   bool done = false;

   bc.pushFolder();
   setImages(&bc);

   LogLock();

   Log("Reading response:\r\n");

   int ch = in->read();

   while (!done) {

      if (ch == '*') {
         // *C,nlines,date,...
         ch = in->read();

         // ,nlines,date,...
         int i = CsvParser(in, apszCommand, ELEMENTS(apszCommand));

#ifndef NLOG
         Log("*%c", ch);
         DumpCsv(apszCommand, i);
#endif /* NLOG */

         if (i >= 0) {
            switch (ch) {
               case 'n':
               case 'N':
                  // *N    UnKnown Username
                  //
                  result = UnknownEmail;
                  m_status.setStatus(IDS_UNKNOWN_EMAIL, m_pszEmail);

                  // PLACEHOLDER:
                  //    state::= 'Login failure'
                  //   status::= '
                  ch = skip(in);
                  break;

               case 'p':
               case 'P':
                  // *P    Unknown Password
                  //
                  result = UnknownPassword;
                  m_status.setStatus(IDS_UNKNOWN_PASS);
                  ch = skip(in);
                  break;

               case 't':
               case 'T':
                  // *T    new Token
                  //
                  ch = skip(in);
                  if (i >= 2) {
                     seqno = strtol(apszCommand[1], NULL, 0);
                  }
                  break;

               case 'm':
               case 'M':
                  // *M    display Message in popup
                  // fall through, same as Update

               case 'u':
               case 'U':
                  // *U    Show document in URL (first parameter)
                  //
                  if (i > 0) {
                     try {
                        RegKey key;

                        if (key.open(HKEY_CURRENT_USER, APP_REG_KEY, true)) {
                           DateTime dt = key.queryDateTime(apszCommand[1]), dtNow = DateTime::now();

                           // show the document if:
                           // 1. the timing is right:
                           //       time pulled from the registry is invalid (value didn't exist)
                           //    OR
                           //       time pulled from registry is less than now
                           // 2. The versio is right:
                           //       no version specified
                           //    OR
                           //       version specified is greater than ours
                           ///
                           if ((!dt.isValid() || dt < dtNow) 
                               &&
                               (i == 2 || ParseVer(apszCommand[2]) > MakeVer(SYNCIT_V_MAJOR, SYNCIT_V_MINOR)))
                           {
                              showDocument(apszCommand[1]);

                              key.setValue(apszCommand[1], dtNow + DeltaTime::DAY);
                           }
                        }
                     } catch (BaseError &) {
                     }
                  }

                  ch = skip(in);
                  break;

               case 'w':
               case 'W':
                  // *W    Wait xxx seconds (see above to avoid deadlocks)
                  result = Wait;
                  ch = skip(in);
                  break;

               case 'b':
               case 'B':
                  // *B    All Bookmarks on File
                  //
                  ch = readFolder(pc, in, m_root);

                  result = OK;
                  break;

               case 'q':
               case 'Q':
                  // *Q  unchanged subscription
                  if (i > 3) {
                     unsigned long ulToken = u_atoul(apszCommand[3]);
                     BookmarkSubscription *pbs = (BookmarkSubscription *) m_pSubscriptions->findId(apszCommand[1]);

                     if (pbs != NULL) {
                        pbs = NEW BookmarkSubscription(*pbs);

                        assert(ulToken == pbs->getSeqNo());
                        pSubscriptions->add(pbs);
                        pSubscriptions->defineAliasId(apszCommand[1], pbs);
                     }
                  }
                  ch = skip(in);
                  break;

               case 'r':
               case 'R':
                  // *R   Read subscRiptions
                  //
                  if (i > 3) {
                     char *pszPath = apszCommand[2];
                     char *pszSub = strchr(pszPath, '\\');

                     if (pszSub != NULL) {
                        *pszSub++ = 0;
                     }

                     Image *open = NULL, *closed = NULL;

                     if (i > 4) {
                        if ((open = gpLoader->load(apszCommand[4], &m_root)) != NULL) {
                           closed = open->attach();
                        }
                     }

                     bc.startSubscription();
                     bc.setName(pszPath);
                     bc.setSubscriptionSeqNo(strtol(apszCommand[3], NULL, 0));
                     bc.setId(apszCommand[1]);

                     if (i > 4) {
                        SetImages(&bc, m_root, apszCommand[4], i > 5 ? apszCommand[5] : NULL);
                     }

                     bc.pushFolder();

                     if (pszSub != NULL) {
                        bc.startFolder();
                        bc.setImages(BookmarkItem::OPEN_IMAGE, &gpBuiltins->SubscriptionFolderOpen);
                        bc.setImages(BookmarkItem::CLOSED_IMAGE, &gpBuiltins->SubscriptionFolderClosed);
                        bc.setName(pszSub);

                        if (i > 4) {
                           SetImages(&bc, m_root, apszCommand[4], i > 5 ? apszCommand[5] : NULL);
                        }

                        bc.pushFolder();
                     }

                     ch = readFolder(&bc, in, m_root);

                     if (pszSub != NULL) {
                        bc.popFolder();
                        bc.endFolder();
                     }

                     bc.popFolder();
                     bc.endSubscription();
                  }
                  else {
                     ch = skip(in);
                  }
                  break;

               case 's':
               case 'S':
                  // *S = Set Variable
                  // So the server can set some resources, i.e. Download URL or whatever, just
                  // some control
                  //
                  // Format is "*S,varname,value" which sets HKEY_CURRENT_USER\Software\SyncIT\BookmarkSync\varname to value
                  //        or "*S,varname" which removes HKEY_CURRENT_USER\Software\SyncIT\BookmarkSync\varname
                  //
                  try {
                     RegKey key;

                     if (key.open(HKEY_CURRENT_USER, m_pszRegKey, true)) {
                        if (i == 3) {
                           key.setValue(apszCommand[1], apszCommand[2]);
                        }
                        else if (i == 2) {
                           key.deleteValue(apszCommand[1]);
                        }

                        getConfigVariables(key);
                     }
                  } catch (BaseError &) {
                  }

                  ch = skip(in);
                  break;

               case 'v':
               case 'V':
                  // *V = Current Version of Software in following line
                  // This way the client can decide if to upgrade
                  // *V,url,m.n    Install EXE in URL (first parameter) if version (second parameter) greater than ours
                  //
                  if (i > 0) {
                     try {
                        RegKey key;

                        if (key.open(HKEY_CURRENT_USER, APP_REG_KEY, true)) {
                           DateTime dt = key.queryDateTime(apszCommand[1]), dtNow = DateTime::now();

                           // show the document if:
                           // 1. the timing is right:
                           //       time pulled from the registry is invalid (value didn't exist)
                           //    OR
                           //       time pulled from registry is less than now
                           // 2. The versio is right:
                           //       no version specified
                           //    OR
                           //       version specified is greater than ours
                           ///
                           if ((!dt.isValid() || dt < dtNow) 
                               &&
                               (i == 2 || ParseVer(apszCommand[2]) > MakeVer(SYNCIT_V_MAJOR, SYNCIT_V_MINOR)))
                           {
                              char achPath[MAX_PATH];

                              size_t lPath = UrlToFilename(apszCommand[1], achPath, sizeof(achPath), true);
                              DownloadDocument(URL(m_root, apszCommand[1]), achPath);
                              ::ShellExecute(NULL, TEXT("open"), achPath, NULL, TEXT("."), SW_NORMAL);

                              key.setValue(apszCommand[1], dtNow + DeltaTime::DAY);
                           }
                        }
                     } catch (BaseError &) {
                     }
                  }

                  ch = skip(in);
                  break;

               case 'e':
               case 'E':
                  // *E = Error Message
                  //
                  result = ErrorMessage;

                  ch = in->read();

                  if (ch != '*') {
                     char achLine[512];

                     in->readLine(achLine, ELEMENTS(achLine));
                     ch = skip(in);
                     m_status.setStatus(IDS_SERVER_ERROR, achLine);
                   //m_wiz.bad(LoginWizard::Panel_Ready, IDS_SERVER_ERROR, achLine);
                  }
                  break;

               case 'c':
               case 'C':
                  // *C = Copy File
                  if (i > 1) {
                     char achFilename[MAX_PATH];
                     UrlToFilename(apszCommand[1], achFilename, sizeof(achFilename), true);

                     DownloadDocument(URL(m_root, apszCommand[1]), achFilename);
                  }

                  ch = skip(in);
                  break;

               case 'z':
               case 'Z':
                  // *Z = End of file
                  //
                  done = true;

                  m_cs.enter();
                  m_status.setPopupMenuSubscriptions(pSubscriptions);

                  BookmarkObject::Detach(m_pSubscriptions);
                  m_pSubscriptions = pSubscriptions;
                  pSubscriptions = NULL;
                  m_cs.leave();

                  if (seqno != -1) {
                     setSeqNo(seqno);
                  }
                  break;

               default:
                  ch = skip(in);
            }
         }

         CsvRelease(apszCommand, ELEMENTS(apszCommand), i);
      }
      else if (ch == -1) {
         if (result == OK || result == NoChange) {
            result = PrematureEOF;
            m_status.setStatus(IDS_BAD_RESPONSE, pszHost, TEXT("<EOF>"));
         }

         done = true;
      }
      else {
         TCHAR achLine[4096];

         achLine[0] = ch;
         int i = in->read(achLine + 1, ELEMENTS(achLine) - 2);
         achLine[i + 1] = '\0';

         // protocol error
         result = ProtocolError;
         done = true;
         m_status.setStatus(IDS_BAD_RESPONSE, pszHost, achLine);
      }
   }

   LogUnlock();

   BookmarkObject::Detach(pSubscriptions);

   return result;
}

/**
 * Read a bookmark folder from the server.  The format is
 * one bookmark per line, in CSV format
 * {id},{name},{url},{extra}...
 * For example:
 * 4253,"\Work\SyncIT\Admin","http://www.bookmarksync.com/Admin/","28 Nov 1998 12:23:42.352"
 *
 * @param in   an InputStream reading from the server socket
 * @param p    a bookmark folder to read into
 * @return ch  the last character read
 */
static int readFolder(BookmarkContext *pc, BufferedInputStream *in, const URL &url) {
   char *apszBookmark[4];

   // 0 = name
   // 1 = URL (optional, if not present, its a folder)
   // 2 = open image (optional, if not present, use 
   // 3 = closed image

   int ch = in->read();

   if (ch != '*' && ch != -1) {
      int m;
      DateTime now = DateTime::now();

      do {
         in->putback(ch);
         m = CsvParser(in, apszBookmark, ELEMENTS(apszBookmark));

#ifndef NLOG
         DumpCsv(apszBookmark, m);
#endif /* NLOG */

         if (m > 0) {
            if (m > 1 && apszBookmark[1] != NULL) {
               pc->startBookmark(apszBookmark[0], '%', gachMap, ELEMENTS(gachMap), apszBookmark[1]);

               if (m > 2) {
                  SetImages(pc, url, apszBookmark[2], m > 3 ? apszBookmark[3] : NULL);
               }

               pc->endBookmark(apszBookmark[0]);
            }
            else {
               pc->startFolder(apszBookmark[0], '%', gachMap, ELEMENTS(gachMap));

               if (m > 2) {
                  SetImages(pc, url, apszBookmark[2], m > 3 ? apszBookmark[3] : NULL);
               }

               pc->endFolder(apszBookmark[0]);
            }
         }

         CsvRelease(apszBookmark, ELEMENTS(apszBookmark), m);
         ch = in->read();
      } while (m >= 0 && ch != '*' && ch != -1);
   }

   return ch;
}

/**
 * Skip all lines not beginning with '*'
 *
 * @param in  a BufferedInputStream to read from
 * @return last character read, either '*' or -1 (for EOF)
 */
static int skip(BufferedInputStream *in) {
   int ch = in->read();

   Log("\r\n");

   // line doesn't begin with '*'
   //
   if (ch != '*') {
      while (ch != -1) {
         Log("%c", (char) ch);

         if (ch == '\n') {
            ch = in->read();

            if (ch == '*')
               return ch;
         }
         else {
            ch = in->read();
         }
      }
   }

   return ch;
}

static void SetImages(BookmarkContext *pc, const URL &url, const char *pszOpen, const char *pszClosed) {
   Image *pOpen   = pszOpen   == NULL ? NULL : gpLoader->load(pszOpen,   &url);
   Image *pClosed = pszClosed == NULL ? NULL : gpLoader->load(pszClosed, &url);

   if (pOpen != NULL || pClosed != NULL) {
      if (pOpen != NULL)   { pc->setImages(BookmarkItem::OPEN_IMAGE, pOpen); }

      pc->setImages(BookmarkItem::CLOSED_IMAGE, pClosed != NULL ? pClosed : pOpen);

      if (pOpen   != NULL) { Image::Detach(pOpen);   }
      if (pClosed != NULL) { Image::Detach(pClosed); }
   }
}
