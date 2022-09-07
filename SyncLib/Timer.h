/**
 * SyncLib/Timer.h
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
 * Example #1:
 * A series of timers that have the same delta time.  If you have a set of
 * actions that all expire after, say, 5 seconds, then all timer objects can
 * go on a linear list.  Each object goes onto the end of the list (because
 * it expires after all objects on the list.  One thread can put items onto
 * a list, and another thread can pop them off the list, and you don't need
 * a signal object to notify when the list changes, as long as the expiration
 * thread (the thread popping TimerEntry objects off the list) waits for
 * the timeout value when there are no entries on the list.
 *
 * Declare the timer queue:
 *    TimerQueue tq;
 *    static const unsigned long EXPIRATION = 5000; // 5 seconds
 *
 * To put a Signalable object s on the timer queue:
 *    tq.push_back(TimerEntry(::GetTickCount() + EXPIRATION, s));
 *
 * To expire all due timers:
 *    while (!tq.empty() && tq.front().getDeltaTime(::GetTickCount())) {
 *       tq.front().signal();
 *       tq.pop_front();
 *    }
 *    // tq.empty() || tq.front().getDeltatime() > 0
 *
 * To get the time until the next expiration:
 *    unsigned long ulExpiration = tq.empty() ? EXPIRATION : tq.front().getDeltaTime(::GetTickCount());
 */
#ifndef Timer_H
#define Timer_H

#include <queue>

namespace syncit {

   using std::queue;
   using std::priority_queue;

   /**
    * The Signalable class is an abstract base class (an interface) for objects
    * to be placed on the TimerQueue for eventual expiration.
    */
   class Signalable {

   protected:
      Signalable() {}

   public:
      virtual ~Signalable() {}

      virtual void signal() = 0;

   };

   /**
    * A TimerEntry records an expiration time and associates it with
    * a Signalable object.  Use any of the STL container classes to
    * store TimerEntry objects to build a timer queue.  Two good
    * choices are queue<TimerEntry> and priority_queue<TimerEntry>
    */
   class TimerEntry {

   public:
      /**
       * Create a TimerEntry given an absolute expiration time and
       * an interface to an object to signal when the timer expires.
       *
       * @param ulExpiration   absolute expiration time
       * @param ps             pointer to Signalable interface
       */
      TimerEntry(unsigned long ulExpiration,
                 Signalable *ps) {
         m_ulExpiration = ulExpiration;
         m_ps = ps;
      }

      /**
       * Compares two TimerEntry objects to see which one is later.
       * This should be used by priority queue and heap algorithms,
       * where earlier TimerEntry objects have greater priority.
       * The comparison takes into account rollover of expiration times.
       * <p>
       * Some examples:
       * <pre>
       * lhs.ulExpiration      < or >     rhs.ulExpiration
       * ----------------     --------    ----------------
       * 0                    >           1000
       * 1000                 <           0
       * 0xFFFFFF00           >           1000
       * </pre>
       */
      bool operator<(const TimerEntry &rhs) const {
         long d = long(m_ulExpiration - rhs.m_ulExpiration);

         return d > 0;
      }

      /**
       * Returns the delta time before this TimerEntry expires.
       * If the delta time has already expired, this method returns 0.
       * The delta takes into account rollover of expiration times.
       * <p>
       * Some examples:
       * <pre>
       * ulExpiration       ulCurrent           deltaTime
       * ------------      -----------          ---------
       * 5000              1000                 4000
       * 3000              8000                 0
       * 0x1000            0xFFFFF000           0x2000
       * </pre>
       */
      unsigned long getDeltaTime(unsigned long ulCurrent) const {
         long d = long(m_ulExpiration - ulCurrent);

         return d < 0 ? 0 : (unsigned long) d;
      }

      /**
       * Signal the associated Signalable object
       */
      void signal() {
         m_ps->signal();
      }

      /**
       * Retrieve the associated Signalable object
       */
      Signalable *getSignalable() const {
         return m_ps;
      }

   private:
      unsigned long m_ulExpiration;
      Signalable *m_ps;
   };

   typedef queue<TimerEntry> TimerQueue;
   typedef priority_queue<TimerEntry> TimerPriorityQueue;
}

#endif /* Timer_H */
