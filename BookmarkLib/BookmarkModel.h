/*
 * BookmarkLib/BookmarkModel.h
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
 * Last Modification: 1 Sep 1999
 *
 * Description:
 *    This file declares the Bookmark class.  A Bookmark object
 *    describes one bookmark in the user's browser bookmark file.
 *
 * See also:
 *    Bookmark.cxx         -- contains the definitions for the class
 *                            declared here.
 *    BookmarkFolder.cxx
 */
#pragma warning( disable : 4786 )

#ifndef BookmarkModel_H
#define BookmarkModel_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include <cassert>

#include <vector>
#include <stack>
#include <map>
#include <set>

#include "SyncLib/util.h"
#include "SyncLib/DateTime.h"
#include "SyncLib/text.h"
#include "SyncLib/Image.h"

namespace syncit {

   using std::vector;

   ///////////////////////
   // Table of Contents...
   //
   class Href;
   class BookmarkObject;
   class    BookmarkSeparator;
   class    BookmarkAlias;
   class    BookmarkItem;
   class       Bookmark;
   class       BookmarkFolder;
   class          BookmarkSubscription;
   class             BookmarkModel;
   class BookmarkDifferences;
   class BookmarkSink;
   class BookmarkContext;
   //
   // ...table of contents
   ///////////////////////

   /**
    * A Href is a smart pointer to a web URL.  The idea is this: an Href is nothing
    * more than a pointer to a compressed, reference-counted URL.
    */
   class Href {

      friend class Compare;

      struct Data {
         unsigned short flags;
         char ach[2];
      };

      class Compare {
      public:
         bool operator()(const Data *p1, const Data *p2) const {
            return Href::Compare0(p1, p2) < 0;
         }
      };

      enum {
         PROTOCOL_MASK  = 0xE000,   // 1110 0000
         WWW_MASK       = 0x1000,   // 0001 0000   host prefixed with "www."
         DOMAIN_MASK    = 0x0C00,   // 0000 1100
         INTERNED_MASK  = 0x0200,   // 0000 0010
         REFCOUNT_MASK  = 0x01FF    // 0000 0001 1111 1111
      };

      Href(Data *p) : m_p(Attach(p)) {
      }

   public:
      typedef set<Data *, Compare> Set;

   private:
      class Table {
      public:
#ifndef NDEBUG
         ~Table();
#endif /* NDEBUG */

      public:
         Set m_set;
      };

   public:
      // Create an empty Href.
      //
      // post: !valid()
      //
      Href() {
         m_p = NULL;
      }

      // Create an Href based on the string
      //
      // post: valid()
      //       refcount() == 1
      //       
      Href(const char *psz);

      Href(const Href &rhs) {
         m_p = Attach(rhs.m_p);
      }

      ~Href() {
         release();
      }

      Href &operator=(const Href &rhs) {
         release();
         m_p = Attach(rhs.m_p);
         return *this;
      }

      bool operator==(const Href &rhs) const {
         return m_p == rhs.m_p;
      }

      bool operator!=(const Href &rhs) const {
         return m_p != rhs.m_p;
      }

      enum Protocol {
         AOL            = 0x0000,   // 000         "aol:"
         FTP            = 0x2000,   // 001         "ftp://"
         HTTP           = 0x4000,   // 010         "http://"
         HTTPS          = 0x6000,   // 011         "https://"
         MAILTO         = 0x8000,   // 100         "mailto:"
         SOCKS          = 0xA000,   // 101         "socks:"
         FILE           = 0xC000,   // 110         "file:"
         OTHER_PROTOCOL = 0xE000,   // 111         anything else
      };

      enum Domain {
         COM            = 0x0000,   //      00     ".com"
         NET            = 0x0400,   //      01     ".net"
         ORG            = 0x0800,   //      10     ".org"
         OTHER_DOMAIN   = 0x0C00,   //      11     anything else
      };

      static Href Intern(const char *psz);

      size_t getHost(char *pach, size_t cch) const {
         return getHost0(pach, cch, lstrlenA(m_p->ach));
      }

      const char *getPath() const {
         return m_p->ach + lstrlenA(m_p->ach) + 1;
      }

      size_t format(char *pach, size_t cch) const;

      static const Set &GetSet() {
         return m_gtable.m_set;
      }

      int getWWW() const {
         return m_p->flags & WWW_MASK;
      }

      Protocol getProtocol() const {
         return Protocol(m_p->flags & PROTOCOL_MASK);
      }

      Domain getDomain() const {
         return Domain(m_p->flags & DOMAIN_MASK);
      }

      static int Compare(const Href &lhs, const Href &rhs) {
         return Compare0(lhs.m_p, rhs.m_p);
      }

      static int Compare0(const Data *pd1, const Data *pd2);


   private:
      static Data *Attach(Data *p);

      // Release the link to the Href Data.  Decrements the refcount,
      // deletes the storage if zero.  Removes from interned set if
      // interned.
      //
      void release();

      size_t getHost0(char *pach, size_t cch, size_t l) const;

      static size_t init(Data *p, const char *psz);

      unsigned refcount() const {
         return m_p->flags & REFCOUNT_MASK;
      }

   private:
      static Table m_gtable;

      Data *m_p;  // refcount > 0
   };

   class BookmarkObject {
   protected:
      BookmarkObject();
      BookmarkObject &operator=(const BookmarkObject &rhs) {
         return *this;
      }

   public:
      virtual ~BookmarkObject();

      virtual bool isBookmark() const;
      virtual bool isFolder()   const;
      virtual bool isAlias()    const;
      virtual bool isSeparator() const;

      virtual bool equals(const BookmarkObject *p) const;

      const BookmarkObject *attach() const {
         return (const BookmarkObject *) ((BookmarkObject *) this)->attach();
      }

      BookmarkObject *attach() {
         m_ulRefCount++;
         return this;
      }

      bool detach() {
         return --m_ulRefCount == 0;
      }

      static void Detach(BookmarkObject *p);

   private:
      unsigned long m_ulRefCount;

      // disable copy constructor
      BookmarkObject(BookmarkObject &rhs);
   };

   class BookmarkSeparator : public BookmarkObject {
   public:
      BookmarkSeparator() {}

      virtual bool isSeparator() const;
   };

   class BookmarkAlias : public BookmarkObject {

   public:
      BookmarkAlias(const tchar_t *pszId);
      BookmarkAlias(const BookmarkAlias &rhs);
      ~BookmarkAlias();

      BookmarkAlias &operator=(const BookmarkAlias &rhs);

      virtual bool isAlias() const;

      ////////////////
      // Attributes...
      //
      const tchar_t *getId() const {
         return m_id;
      }
      // ...attributes
      ////////////////

   private:
      tchar_t *m_id;
   };

   class BookmarkItem : public BookmarkObject {
   protected:
      BookmarkItem(const tchar_t *pszName);
      BookmarkItem(const BookmarkItem &rhs);

      BookmarkItem &operator=(const BookmarkItem &rhs);

   public:
      virtual ~BookmarkItem();

   ////////////////
   // Properties...
   //
      ///////////////////////
      // The name property...
      //
      bool hasName() const {
         return m_pszName != NULL;
      }

      /**
       * The name property is the user-selected name or
       * title tag of a bookmark.
       * <p>
       * This name is not hierarchical (it doesn't contain
       * any information about the parent) even if bookmarks
       * are arranged hierarchically.
       * <p>
       * The pointer returned by getName() is freed by the destructor.
       *
       * @return the current value of the name property
       */
      const tchar_t *getName() const {
         return m_pszName;
      }

      void setName(const tchar_t *pszName) {
         m_pszName = tstrrealloc(m_pszName, pszName);
      }
      //
      // ...the name property
      ///////////////////////

      /////////////////////
      // The ALIASID property...
      //
      bool hasId() const {
         return m_pszId != NULL;
      }

      const tchar_t *getId() const {
         return m_pszId;
      }

      void setId(const tchar_t *psz) {
         m_pszId = tstrrealloc(m_pszId, psz);
      }
      // ... the id property
      //////////////////////

      //////////////////////////////
      // The description property...
      //
      bool hasDescription() const {
         return m_pszDescription != NULL;
      }

      const tchar_t *getDescription() const {
         return m_pszDescription;
      }

      void setDescription(const tchar_t *psz) {
         m_pszDescription = tstrrealloc(m_pszDescription, psz);
      }
      // ...the description property
      //////////////////////////////

      //////////////////////////
      // The addTime property...
      //
      const DateTime &getAdded() const {
         return m_addTime;
      }

      void setAdded(const DateTime &dt) {
         m_addTime = dt;
      }
      // ...the addTime property
      //////////////////////////

      /////////////////////////
      // The images property...
      //
      enum ImageType {
         DEFAULT_IMAGE = 0,

         FOLDED_IMAGE = DEFAULT_IMAGE,
         CLOSED_IMAGE = DEFAULT_IMAGE,

         SELECTED_IMAGE,
         OPEN_IMAGE = SELECTED_IMAGE,

         NUM_IMAGE_TYPES
      };

      Image *getImages(ImageType t) const {
         return m_images[t];
      }

      void setImages(ImageType t, Image *p) {
         Image::Detach(m_images[t]);
         m_images[t] = p->attach();
      }

      // ...the images property
      /////////////////////////

   protected:
      tchar_t *m_pszName;
      tchar_t *m_pszId;
      tchar_t *m_pszDescription;
      DateTime m_addTime;
      Image *m_images[NUM_IMAGE_TYPES];
   };

   /**
    * A Bookmark describes one bookmark in the user's browser
    * bookmark file.  The object maintains three dynamically
    * managed string properties:
    * <pre>
    * Property name     Type     Description
    * -------------     ----     -----------
    * name              char*    The user-selected name or
    *                            title tag of a bookmark.
    * url               char*    The URL of the bookmark.
    * modified          UNT64    The modification time in Win32
    *                            FILETIME format.
    *                            which is the UTC/GMT time of the
    *                            last modification to this bookmark.
    * </pre>
    * Use get...() or set...(v) methods to access these properties.
    */
   class Bookmark : public BookmarkItem {

#ifndef NDEBUG
   public:
      bool isValid() const;
   private:
      char m_achStartTag[sizeof("Bookmark")];

#endif /* NDEBUG */

      friend class BookmarkFolder;

   public:
      Bookmark();
      Bookmark(const Bookmark &rhs);

      Bookmark &operator=(const Bookmark &rhs);

      ~Bookmark();

      virtual bool isBookmark() const;

      virtual bool equals(const BookmarkObject *p) const;

   ////////////////
   // Properties...
   //
      Image *getDefaultImage() const { return m_images[DEFAULT_IMAGE]; }
      void setDefaultImage(Image *p) { setImages(DEFAULT_IMAGE, p); }

      Image *getSelectedImage() const { return m_images[SELECTED_IMAGE]; }
      void setSelectedImage(Image *p) { setImages(SELECTED_IMAGE, p);  }


      //////////////////////
      // The url property...
      //
      /**
       * The url property is the location that the
       * bookmark points to.
       * <p>
       * The pointer returned by getHref() is liable to be
       * freed by setHref() or the destructor.
       *
       * @return the current value of the url property
       * @see #setHref
       */
      const Href &getHref() const {
         return m_href;
      }

      /**
       * The url property is the location that the
       * bookmark points to.
       * <p>
       * A dynamically allocated copy of the parameter is taken,
       * so the string may be modified or freed at any time after
       * setHref() is called.
       *
       * @param psz  the new value of the url property
       * @see #getHref()
       */
      void setHref(const char *psz);
      //
      // ...the url property
      //////////////////////

      ///////////////////////////
      // The modified property...
      //
      /**
       * The modified property is the GMT time of the
       * last modification of the bookmark, as recorded either
       * by the filesystem or in the bookmark file itself.
       * <p>
       * @return the current value of the modified property
       * @see #setModified
       */
      const DateTime &getModified() const {
         return m_modified;
      }

      /**
       * The modified property is the GMT time of the
       * last modification of the bookmark, as recorded either
       * by the filesystem or in the bookmark file itself.
       *
       * @param psz  the new value of the modified property
       * @see #getModified()
       */
      void setModified(const DateTime &dt) {
         m_modified = dt;
      }
      //
      // ...the modified property
      ///////////////////////////

      //////////////////////////
      // The visited property...
      //
      /**
       * The visited property is the GMT time.
       * <p>
       * @return the current value of the visited property
       * @see #setVisited
       */
      const DateTime &getVisited() const {
         return m_visited;
      }

      /**
       * The visited property is the GMT time.
       *
       * @param psz  the new value of the visited property
       * @see #getVisited()
       */
      void setVisited(const DateTime &dt) {
         m_visited = dt;
      }
      //
      // ...the visited property
      //////////////////////////

   // ...properties
   ////////////////

   private:
      Href     m_href;

      DateTime m_modified, m_visited;

#ifndef NDEBUG
      char m_achEndTag[sizeof("Bookmark")];
#endif /* NDEBUG */

   };

   typedef vector<BookmarkObject *> BookmarkVector;
   typedef vector<const BookmarkFolder *> BookmarkPath;

   /**
    * A BookmarkFolder is an object that tracks all Bookmarks and
    * recursively all BookmarkFolders within one folder on the user's
    * bookmark file.
    * <p>
    * The BookmarkFolder has one property:
    * <pre>
    * Property name     Type     Description
    * -------------     ----     -----------
    * name              char*    The user-selected name of
    *                            the bookmark folder
    * </pre>
    * <p>
    * The BookmarkFolder also has two arrays: one of Bookmarks,
    * one of BookmarkFolders.
    */
   class BookmarkFolder : public BookmarkItem {

#ifndef NDEBUG
   public:
      bool isValid() const;

   private:
      char m_achStartTag[sizeof("BookmarkFolder")];
#endif /* NDEBUG */

      /*
       * Use CompareBookmarkFolders to sort or search through arrays
       * of pointers to BookmarkFolder objects.  The sorting or searching
       * is done on the name property.  The url and modified
       * properties of the Bookmarks are ignored, as are subdirectories.
       * <p>
       * Returns a value less than 0 if pv1's name is
       *    less than pv2's;
       * returns 0 if pv1 and pv2 have identical names;
       * otherwise returns a value greater than 0.
       *
       * @param pv1  pointer to pointer to BookmarkFolder (const BookmarkFolder**)
       * @param pv2  pointer to pointer to BookmarkFolder (const BookmarkFolder**)
       *
       * @see BookmarkFolder::sort() in BookmarkFolder.cxx for an
       *       example.
       */
      friend int CompareBookmarkFolders(const void *pv1, const void *pv2);

   public:
      BookmarkFolder();
      BookmarkFolder(const BookmarkFolder &rhs);

      virtual ~BookmarkFolder();

      BookmarkFolder &operator=(const BookmarkFolder &rhs);

      virtual bool isFolder() const;
      virtual bool isSubscription() const;

      bool sameElements(const BookmarkFolder *p) const {
         BookmarkVector::const_iterator i = m_elements.begin();
         BookmarkVector::const_iterator j = p->m_elements.begin();
         BookmarkVector::const_iterator end = m_elements.end();

         if (end - i != p->m_elements.size()) {
            return false;
         }
         else {
            while (i != end) {
               if ((*i)->equals(*j)) {
                  i++;
                  j++;
               }
               else {
                  return false;
               }
            }

            return true;
         }
      }

      virtual bool equals(const BookmarkObject *p) const;

   ////////////////
   // Properties...
   //
      Image *getOpenImage() const { return m_images[OPEN_IMAGE]; }
      void setOpenImage(Image *p) { setImages(OPEN_IMAGE, p); }

      Image *getClosedImage() const { return m_images[CLOSED_IMAGE]; }
      void setClosedImage(Image *p) { setImages(CLOSED_IMAGE, p); }

      //////////////
      // Elements...
      //
      const BookmarkVector &elements() const {
         return m_elements;
      }

      BookmarkVector::const_iterator begin() const { 
         return m_elements.begin();
      }

      BookmarkVector::const_iterator end() const {
         return m_elements.end();
      }

      void add(BookmarkObject *p) {
         m_elements.push_back(p);
      }
      // ...elements
      //////////////

      /////////////////////////
      // The folded property...
      //
      bool isFolded() const {
         return m_fFolded;
      }

      void setFolded(bool f) {
         m_fFolded = f;
      }
      // ...the folded property
      /////////////////////////

   //
   // ...properties
   ////////////////

      Bookmark *findBookmark(LPCTSTR pszName) const;
      BookmarkFolder *findBookmarkFolder(LPCTSTR pszName) const;

      /**
       * Removes a bookmark from the folder.
       */
      Bookmark *removeBookmark(BookmarkPath::const_iterator i, BookmarkPath::const_iterator e, const Bookmark *pb);

      BookmarkFolder *removeFolder(BookmarkPath::const_iterator i, BookmarkPath::const_iterator e, const BookmarkFolder *pf);

      void clear() {
         detachall();
         m_elements.clear();
         m_fFolded = true;
      }

   private:
      void detachall();
      void copy(const BookmarkFolder &rhs);

      bool m_fFolded;
      BookmarkVector m_elements;

#ifndef NDEBUG
   private:
      char m_achEndTag[sizeof("BookmarkFolder")];
#endif /* NDEBUG */

   };

   class BookmarkSubscription : public BookmarkFolder {

   public:
      BookmarkSubscription() : BookmarkFolder() {
         m_seqno = 0;
      }

      BookmarkSubscription(const BookmarkSubscription &rhs) : BookmarkFolder(rhs) {
         m_seqno = rhs.m_seqno;
      }

      virtual ~BookmarkSubscription() {
      }

      BookmarkSubscription &operator=(const BookmarkSubscription &rhs) {
         BookmarkFolder::operator=(rhs);

         m_seqno = rhs.m_seqno;
         return *this;
      }

      virtual bool isSubscription() const;

      long getSeqNo() const {
         return m_seqno;
      }

      void setSeqNo(long l) {
         m_seqno = l;
      }

      void clear() {
         BookmarkFolder::clear();
         m_seqno = 0;
      }

   private:
      long m_seqno;
   };

   class BookmarkModel : public BookmarkSubscription {

      friend class BookmarkContext;

   public:
      BookmarkModel();
      BookmarkModel(const BookmarkModel &rhs);

      BookmarkModel &operator=(const BookmarkModel &rhs);

      bool isNewItemHeader(const BookmarkFolder *p) const {
         return p == m_pNewItemHeader;
      }

      bool isMenuHeader(const BookmarkFolder *p) const {
         return p == m_pMenuHeader;
      }

      const BookmarkItem *findId(const tchar_t *s) const {
         map<tstring, BookmarkItem *>::const_iterator i = m_aliases.find(s);

         if (i == m_aliases.end())
            return NULL;
         else
            return (*i).second;
      }

      void removeAliasId(const tchar_t *s) {
         map<tstring, BookmarkItem *>::iterator i = m_aliases.find(s);

         if (i != m_aliases.end()) {
            m_aliases.erase(i);
         }
      }

      void defineAliasId(const tchar_t *s, BookmarkItem *p) {
         m_aliases[s] = p;
      }

      void clear() {
         BookmarkSubscription::clear();
         m_aliases.clear();
         m_pNewItemHeader = NULL;
         m_pMenuHeader = NULL;
      }

   private:
      void rebuild(BookmarkFolder *pbf);

      map<tstring, BookmarkItem *> m_aliases;

      BookmarkFolder *m_pNewItemHeader;
      BookmarkFolder *m_pMenuHeader;
   };

   /**
    * BookmarkDifferences is an interface (abstract base class) that is
    * called for each difference between one BookmarkFolder and another
    *
    * @see BookmarkFolder::diff
    */
   class BookmarkDifferences {
   protected:
      BookmarkDifferences() {
      }

   public:
      virtual ~BookmarkDifferences() {
      }

      /**
       * Add the <i>pNew</i> bookmark to the old bookmark folder
       * to make it equal to the new bookmark folder
       */
      virtual void addBookmark(const Bookmark *pNew) = 0;

      /**
       * Remove the <i>pOld</i> bookmark from the old bookmark folder
       * to make it equal to the new bookmark folder
       */
      virtual void delBookmark(const Bookmark *pOld) = 0;

      virtual void pushFolder(const BookmarkFolder *pbf) = 0;
      virtual void addFolder(const BookmarkFolder *pNew);
      virtual void add0(const BookmarkFolder *pNew);

      virtual void delFolder(const BookmarkFolder *pOld);
      virtual void del0(const BookmarkFolder *pOld);
      virtual void popFolder() = 0;

   private:
      // disable copy constructor and assignment
      //
      BookmarkDifferences(BookmarkDifferences &rhs);
      BookmarkDifferences &operator=(BookmarkDifferences &rhs);
   };

   class BookmarkSink {
   protected:
      BookmarkSink() {
      }

   public:
      virtual ~BookmarkSink() {
      }

      virtual void progress() {}

   ///////////////////
   // BookmarkItems...
   //
      virtual void setName(const tchar_t *pszTitle) = 0;
      virtual void setDescription(const tchar_t *pszDesc) {}
      virtual void setAdded(const DateTime &dt) {}
      virtual void setId(const tchar_t *pszId) {}
      virtual void setImages(BookmarkItem::ImageType f, Image *p) {}
      virtual void setImages(BookmarkItem::ImageType f, const char *pszUrl) {}

      ///////////////
      // bookmarks...
      //
      virtual void startBookmark() = 0;
      virtual void setBookmarkVisited(const DateTime &dt) {}
      virtual void setBookmarkModified(const DateTime &dt) {}
      virtual void setBookmarkHref(const char *pszHref) = 0;
      virtual void endBookmark() = 0;
      //
      // ...bookmarks
      ///////////////

      /////////////
      // folders...
      //
      virtual void startFolder() = 0;
      virtual void setFolderFolded(bool folded) {}
      virtual void setMenuHeader() {}
      virtual void setNewItemHeader() {}

      virtual void pushFolder() = 0;
      virtual void popFolder() = 0;

      virtual void endFolder() = 0;
      //
      // ...folders
      /////////////

   // ...BookmarkItems
   ///////////////////

      virtual void newSeparator() {}
      virtual void newAlias(const tchar_t *pszId) {}

      ///////////////////
      // Subscriptions...
      //
      virtual void startSubscription() = 0;
      virtual void setSubscriptionSeqNo(long l) {
      }

      virtual void endSubscription() = 0;
      //
      // ...subscriptions
      ///////////////////

      virtual void undoCurrent() = 0;
   };

   /**
    * The BookmarkContext class is the only thing allowed to edit Bookmark
    * objects.  It can maintain a stack of contexts to allow the easy creation
    * from recursively-defined bookmarks, like the Netscape bookmark format,
    * XBEL, or directories and files.
    * <p>
    * There are several ways to create the four major bookmark object types
    * (bookmark, folder, alias, and separator):
    * 1. By context.  The current parent folder is kept track of by the
    *    BookmarkContext object, and a simple method creates a new object
    *    and adds it to the current parent.  See newAlias(const tchar_t *), and newSeparator()
    * 3. By copy.  A copy of the passed-in object is created, along with a copy
    *    of any parents if necessary.
    */
   class BookmarkContext : public BookmarkSink {

   public:
      BookmarkContext(BookmarkModel *p, ImageLoader *pLoader);

      virtual ~BookmarkContext();

      void undoCurrent();

      /**
       * Create a new bookmark and set it as the current item
       */
      void startBookmark();

      bool getBookmark(const tstring &path, const tstring &href);

      /** Set the current bookmark's visited property */
      void setBookmarkVisited(const DateTime &dt);

      /** Set the current bookmark's modified property */
      void setBookmarkModified(const DateTime &dt);

      /** Set the current bookmark's href property */
      void setBookmarkHref(const char *s);

      /**
       * Commit changes to current bookmark, and add it to current folder
       */
      void endBookmark();

      void startBookmark(const tchar_t *pszPathName, tchar_t chEscape, const tchar_t *pachMap, size_t cchMap, const char *pszUrl);
      void endBookmark(const tchar_t *pszPathName);

      bool delBookmark(const Bookmark *pCopy);

      /**
       * Create a new folder and set it as the current item
       */
      void startFolder();

      void startSubscription();

      void setNewItemHeader();
      void setMenuHeader();
      void setFolderFolded(bool f);
      void pushFolder();
      void popFolder();

      /**
       * Commit changes to folder as current item, and add it to current folder
       */
      void endFolder();

      const tchar_t *startFolder(const tchar_t *pszPathName, tchar_t chEscape,
                                 const tchar_t *pachMap, size_t cchMap);
      void endFolder(const tchar_t *pszPathName);

      void endSubscription();

      bool delFolder(const BookmarkFolder *pCopy);

      void newSeparator();

      void newAlias(const tchar_t *s);

      void setSubscriptionSeqNo(long l);

      /** Set the current bookmark's added property */
      void setAdded(const DateTime &dt);
      void setDescription(const tchar_t *s);
      void setName(const tchar_t *s);
      void setId(const tchar_t *s);
      void setImages(BookmarkItem::ImageType f, Image *p);
      void setImages(BookmarkItem::ImageType f, const char *pszUrl);

      void setDefaultFolderImages(BookmarkItem::ImageType f, Image *p);
      void setDefaultBookmarkImages(BookmarkItem::ImageType f, Image *p);
      void setDefaultSubscriptionImages(BookmarkItem::ImageType f, Image *p);

      void setModelSeqNo(unsigned long ul) {
         m_pModel->setSeqNo(ul);
      }

   private:
      BookmarkModel *m_pModel;
      ImageLoader *m_pLoader;

      BookmarkObject *m_pCurrentItem;
      BookmarkFolder *m_pCurrentFolder;

      stack<BookmarkFolder *> m_stack;

      Image *m_apFolderImages[BookmarkItem::NUM_IMAGE_TYPES];
      Image *m_apBookmarkImages[BookmarkItem::NUM_IMAGE_TYPES];
      Image *m_apSubscriptionImages[BookmarkItem::NUM_IMAGE_TYPES];
   };

   inline
   int BookmarkFolderCompare(const BookmarkFolder *p1,
                             const BookmarkFolder *p2) {
      if (p1 == p2) {
         return 0;
      }
      else {
         int c = tstricmp(p1->getName(), p2->getName());

      //   if (c == 0) {
      //      c = p1->getId().compare(p2->getId());
      //   }

         return c;
      }
   }

   inline
   bool BookmarkFolderLess(const BookmarkFolder *p1,
                           const BookmarkFolder *p2) {
      return BookmarkFolderCompare(p1, p2) < 0;
   }

   inline
   int BookmarkCompare(const Bookmark *p1,
                       const Bookmark *p2) {
      if (p1 == p2) {
         return 0;
      }
      else {
         int c = tstricmp(p1->getName(), p2->getName());

         if (c == 0) {
            c = Href::Compare(p1->getHref(), p2->getHref());
         }

         return c;
      }
   }

   inline
   bool BookmarkLess(const Bookmark *p1,
                     const Bookmark *p2) {
      return BookmarkCompare(p1, p2) < 0;
   }

   void ExtractFromFolder(const BookmarkFolder *pbf,
                          vector<const Bookmark *> &vb,
                          vector<const BookmarkFolder *> &vf);

   int diff(const BookmarkFolder *pf1,
            const BookmarkFolder *pf2,
            BookmarkDifferences *pdiff);

}

#endif /* BookmarkModel_H */
