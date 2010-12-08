/* $Id: pthr.h,v 1.6 2005/03/24 20:17:21 jirka Exp $ */
/*
 * Wrapper functions and macros for POSIX thread library.
 * Copyright (C) 2003 Jiri Denemark
 *
 * This file is part of RUM2.
 *
 * RUM2 is free software; you can redistribute it and/or modify
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
 */

/** @file
 * Wrapper functions and macros for POSIX thread library.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.6 $ $Date: 2005/03/24 20:17:21 $
 */

#ifndef PTHR_H
#define PTHR_H

#if HAVE_PTHREAD_H
# include <pthread.h>
#endif

#if RUM_DEBUG
# if STDC_HEADERS
#  include <stdio.h>
# endif
# if HAVE_UNISTD_H
#  include <unistd.h>
# endif
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  if HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   include <time.h>
#  endif
# endif
# if HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
# endif
# if HAVE_SIGNAL_H
#  include <signal.h>
# endif

extern int pthread_mutex_timedlock(pthread_mutex_t * mutex,
                                   struct timespec *abstime);
#endif /* #if RUM_DEBUG */

/** Internal locking macro with deadlock detection for debugging. */
#if RUM_DEBUG
# define internal_lock(mutex) \
    {   struct timespec tmout = { time(NULL) + 10, 0 }; \
        struct rlimit rlim = { -1, -1 };                \
        if (pthread_mutex_timedlock(mutex, &tmout)) {   \
            fprintf(stderr, "Possible deadlock!\n"      \
                            "  thread id: %p\n"         \
                            "  PID:       %ld\n"        \
                            "  mutex:     %s\n"         \
                            "  source:    %s\n"         \
                            "  line:      %d\n",        \
                    (void *) pthread_self(),            \
                    (long) getpid(),                    \
                    #mutex, __FILE__, __LINE__);        \
            setrlimit(RLIMIT_CORE, &rlim);              \
            kill(0, SIGQUIT);                           \
        }                                               \
    }
#else
# define internal_lock(mutex)   pthread_mutex_lock(mutex)
#endif


/** Lock the mutex ensuring it si unlocked even if the thread is cancelled.
 * Matching pairs of lock() and unlock() must occur in the same function, at
 * the same level of block nesting. The expansion of lock() macro introduces
 * an open brace '{' with the matching closing brace '}' being introduced by
 * the expansion of the matching unlock().
 *
 * @param mutex
 *      pointer to the mutex being locked.
 *
 * @return
 *      nothing.
 */
#define lock(mutex) \
    {   int pthr_oldtype;                                               \
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &pthr_oldtype);  \
        pthread_cleanup_push(pthr_mutex_unlock, (void *) (mutex));      \
        internal_lock(mutex);                                           \
        pthread_setcanceltype(pthr_oldtype, NULL);


/** Unlock the mutex previously locked by lock().
 * For details about how to use this see lock() macro description.
 *
 * @param mutex
 *      pointer to the mutex being unlocked.
 *
 * @return
 *      nothing.
 */
#define unlock(mutex)   \
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &pthr_oldtype);  \
        pthread_cleanup_pop(1);                                         \
        pthread_setcanceltype(pthr_oldtype, NULL);                      \
    }


/** Writer's locking macro for readers-writers synchronization problem.
 * Matching pairs of writer_lock() and writer_unlock() must occur in the same
 * function, at the same level of block nesting. The expansion of
 * writer_lock() macro introduces an open brace '{' with the matching closing
 * brace '}' being introduced by the expansion of the matching writer_unlock().
 *
 * @param rw_mutex
 *      mutex used for keeping all other entities (readers and writers) out of
 *      critical section once a writer steps into it.
 *
 * @param r_mutex
 *      mutex associated with @c r_cond condtition and @c r_count counter.
 *
 * @param r_count
 *      number of readers in critical section.
 *
 * @param r_cond
 *      condition for signalling the last reader is leaving critical section.
 *
 * @return
 *      nothing.
 */
#define writer_lock(rw_mutex, r_mutex, r_count, r_cond) \
    lock(rw_mutex);                             \
    lock(r_mutex);                              \
    while (0 != *(r_count))                     \
        pthread_cond_wait(r_cond, r_mutex);     \
    unlock(r_mutex);


/** Writer's unlocking macro for readers-writers synchronization problem.
 * Matching pairs of writer_lock() and writer_unlock() must occur in the same
 * function, at the same level of block nesting. The expansion of
 * writer_lock() macro introduces an open brace '{' with the matching closing
 * brace '}' being introduced by the expansion of the matching writer_unlock().
 *
 * @see writer_lock()
 *
 * @param rw_mutex
 *      mutex used for keeping all other entities (readers and writers) out of
 *      critical section once a writer steps into it.
 *
 * @return
 *      nothing.
 */
#define writer_unlock(rw_mutex) \
    unlock(rw_mutex);


/** Reader's locking macro for readers-writers synchronization problem.
 * @see writer_lock()
 *
 * @param rw_mutex
 *      mutex used for keeping all other entities (readers and writers) out of
 *      critical section once a writer steps into it.
 *
 * @param r_mutex
 *      mutex associated with @c r_cond condtition and @c r_count counter.
 *
 * @param r_count
 *      number of readers in critical section.
 *
 * @return
 *      nothing.
 */
#define reader_lock(rw_mutex, r_mutex, r_count) \
    lock(rw_mutex);                             \
    lock(r_mutex);                              \
    (*(r_count))++;                             \
    unlock(r_mutex);                            \
    unlock(rw_mutex);


/** Reader's unlocking macro for readers-writers synchronization problem.
 * @see writer_lock()
 *
 * @param r_mutex
 *      mutex associated with @c r_cond condtition and @c r_count counter.
 *
 * @param r_count
 *      number of readers in critical section.
 *
 * @param r_cond
 *      condition for signalling the last reader is leaving critical section.
 *
 * @return
 *      nothing.
 */
#define reader_unlock(r_mutex, r_count, r_cond) \
    lock(r_mutex);                              \
    if (0 == --*(r_count))                      \
        pthread_cond_signal(r_cond);            \
    unlock(r_mutex);


/** pthread_mutex_unlock() wrapper with (void *) argument.
 * It is intended to be an unlocking routine for pthread_cleanup_push().
 *
 * @param mutex
 *      pointer to the mutex being unlocked typecasted to (void *).
 *
 * @return
 *      nothing.
 */
extern void pthr_mutex_unlock(void *mutex);


#endif

