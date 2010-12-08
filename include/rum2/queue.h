/* $Id: queue.h,v 1.19 2005/05/09 22:05:08 jirka Exp $ */
/*
 * Generic thread-safe queue (FIFO).
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
 * Generic thread-safe queue (FIFO).
 *
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.19 $ $Date: 2005/05/09 22:05:08 $
 */

#ifndef QUEUE_H
#define QUEUE_H

#if HAVE_PTHREAD_H
# include <pthread.h>
#endif
#if HAVE_SEMAPHORE_H
# include <semaphore.h>
#endif

#include "limits.h"
#include "error.h"

/** Queue group description structure.
 * It is used for waiting for data in any of the queues belonging to the
 * given group. Wait functions can also be interrupted by some event on
 * a given file descriptors.
 *
 * Single queue group can be used only by single thread.
 */
struct queue_group {
    /** Pointer to the previous group in the list. */
    struct queue_group *prev;
    /** Pointer to the next group in the list. */
    struct queue_group *next;
    /** Number of queues in this group. */
    int count;
    /** Array of queues belonging to this group. */
    struct queue **queues;
    /** Number of file descriptors. */
    int fd_count;
    /** File descriptors. */
    int *fds;
    /** POSIX thread's condition variable. */
    pthread_cond_t cond;
    /** Nonzero when a condition was signalled since the last check. */
    int signal;
    /** Mutex for the condition. */
    pthread_mutex_t mutex;
};


/** Type of queue. */
enum queue_type {
    /** Data queue.
     * If length of data queue exceeds length limit items starts to be dropped
     * from the queue. This queue is technically implemented as an array of
     * queue_item which is allocated at once when the queue is created. All
     * items are linked to form a unidirectional circle. No queue_item has
     * to be requested when a new data packet arrives.
     *
     * @warning The queue relies on all items being (struct meta *) for packet
     * profiling. */
    QT_DATA,
    /** RAP message queue.
     * Items are never dropped from queues of this type. Items are stored in
     * a single connected list of queue_item and for each new item a new
     * queue_item is requested from memory management.
     */
    QT_MESSAGE
};


/** Queue flush callback function.
 * The function should typically free the memory accupied by 'item'.
 *
 * @param item
 *      pointer to the item being flushed from a queue.
 *
 * @return
 *      nothing.
 */
typedef void (*queue_flush_cb)(void *item);


/** Common parts of queue description structure.
 * This must always be the first item of any queue structure.
 */
struct queue {
    /** Previous queue in a queue list. */
    struct queue *prev;
    /** Next queue in the list. */
    struct queue *next;
    /** Type of the queue. */
    enum queue_type type;
    /** POSIX semaphore which counts a length of the queue.
     * The counter within this semaphore is always set to the length of the
     * queue plus one. **/
    sem_t length;
    /** Total number of received items. */
    unsigned long total;
    /** Dropped packets. */
    unsigned long dropped;
    /** Queue flush callback function. */
    queue_flush_cb flush_cb;
    /** Number of groups this queue belongs to.
     * Each queue usually belongs to single group only. */
    int group_count;
    /** Array of groups this queue belongs to. */
    struct queue_group **groups;
    /** Optional queue description. */
    char desc[RUM_QUEUE_DESC];
    /** Head of the queue (this item will be removed by queue_pop()). */
    struct queue_item *head;
    /** Tail of the queue (pointer to the queue_item where a new item
     * will be placed). */
    struct queue_item *tail;
    /** Mutex for pop operation on a queue. */
    pthread_mutex_t pop_mutex;
    /** Mutex for push on a queue. */
    pthread_mutex_t push_mutex;
};


/** Message queue. */
struct queue_message {
    /** Common fields of queue structure. */
    struct queue common;
};


/** Data queue. */
struct queue_data {
    /** Common fields of queue structure. */
    struct queue common;
    /** Maximum length of data queue. */
    int max;
    /** Array of @c max queue items. */
    struct queue_item *items;
};


/** Queue item structure. */
struct queue_item {
    /** Next item in the queue (NULL if this is the last item). */
    struct queue_item *next;
    /** Pointer to the user supplied data. */
    void *data;
};


/** Initialize queue management system.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int queue_initialize(EC);


/** Set parameters for dropping policy.
 *
 * @param max_th
 *      maximum size of each queue.
 *
 * @return
 *      nothing.
 */
extern void queue_params(int max_th);


/** Generate queue management statistics.
 *
 * @param content
 *      pointer to the RAP message content typecasted to (void *).
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int queue_stat(EC, void *content);


/** Create a new queue.
 *
 * @param type
 *      requested serialization policy.
 *
 * @param queue
 *      place where to store the pointer to the queue. On error the pointer
 *      is reset to NULL.
 *
 * @param callback
 *      is called for each item that is flushed from the queue.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int queue_new(EC, enum queue_type type,
                     struct queue **queue,
                     queue_flush_cb callback);


/** Set description of the queue.
 * Up to RUM_QUEUE_DESC - 1 characters will be copied from a string created
 * according to desc and further arguments to queue->desc.
 *
 * @param queue
 *      pointer to the queue.
 *
 * @param desc
 *      format string for description of the queue.
 *
 * @param ...
 *      arguments for format string.
 *
 * @return
 *      nothing.
 */
extern void queue_desc(struct queue *queue, const char *desc, ...);


/** Flush a queue and free all of its internal structures.
 *
 * @param queue
 *      place where the pointer to the queue to be destroyed is stored. The
 *      pointer is set to NULL after the queue is freed.
 *
 * @return
 *      nothing.
 */
extern void queue_free(struct queue **queue);


/** Flush a queue.
 *
 * @param queue
 *      a queue to be flushed.
 *
 * @return
 *      nothing.
 */
extern void queue_flush(struct queue *queue);


/** Append a new item to a queue.
 *
 * @param queue
 *      where to append.
 *
 * @param item
 *      item to be appended.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
#define queue_push(errctx, queue, item)                     \
    (((queue)->type == QT_MESSAGE)                          \
        ? queue_push_message((errctx), (queue), (item))     \
        : queue_push_data((errctx), (queue), (item)))

/** Append a new item to a message queue.
 * @sa queue_push
 */
extern int queue_push_message(EC, struct queue *queue, void *item);

/** Append a new item to a data queue.
 * @sa queue_push
 */
extern int queue_push_data(EC, struct queue *queue, void *item);


/** Signal queue group.
 * Argument @c grp is used more than once within this macro!
 *
 * @param grp
 *      pointer to the group to be signaled.
 *
 * @return
 *      nothing.
 */
#define queue_group_signal(grp) {               \
            lock(&(grp)->mutex);                \
            (grp)->signal = 1;                  \
            pthread_cond_signal(&(grp)->cond);  \
            unlock(&(grp)->mutex);              \
        }


/** Return the oldest item without removing it from the queue.
 * This function cannot be used when more than one thread is accessing the
 * same queue.
 *
 * @param queue
 *      the queue.
 *
 * @param item
 *      place where the pointer to the oldest item will be stored.
 *      When there is no item in the queue, NULL pointer will be stored and
 *      nonzero value will be returned. In this context "item" means data
 *      stored within queue_item.
 *
 * @return
 *      zero on success, nonzero on empty queue.
 */
extern int queue_top(struct queue *queue, void **item);


/** Remove the oldest item from a queue.
 *
 * @param queue
 *      the queue an item will be removed from.
 *
 * @param item
 *      place where the pointer to the removed item will be stored.
 *      When there is no item in the queue, NULL pointer will be stored and
 *      nonzero value will be returned. In this context "item" means data
 *      stored within queue_item.
 *
 * @return
 *      zero on success, nonzero on empty queue.
 */
#define queue_pop(queue, item)                  \
    (((queue)->type == QT_MESSAGE)              \
        ? queue_pop_message((queue), (item))    \
        : queue_pop_data((queue), (item)))

/** Remove the oldest item from a message queue.
 * @sa queue_pop
 */
extern int queue_pop_message(struct queue *queue, void **item);

/** Remove the oldest item from a data queue.
 * @sa queue_pop
 */
extern int queue_pop_data(struct queue *queue, void **item);


/** Register a new queue group.
 * The group is freed automatically when any of the queues belonging to the
 * group is freed.
 *
 * @param count
 *      number of queues in the group.
 *
 * @param ...
 *      list of queues (of type struct queue *). Exactly @c count queues have
 *      to be passed on, otherwise strange things can happen.
 *
 * @return
 *      pointer to the queue group or NULL on error.
 */
extern struct queue_group *queue_group_reg(EC, int count, ...);


/** Add file descriptor to a given queue group.
 * When some IO event occurs on that fd, queue group will be signaled. Caller
 * MUST ensure all of the file descriptors even from different queue groups
 * are unique.
 *
 * @param group
 *      queue group.
 *
 * @param fd
 *      file descriptor on which events can signal the group.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int queue_group_io_add(EC, struct queue_group *group, int fd);


/** Remove file descriptor from queue group.
 *
 * @param group
 *      queue group.
 *
 * @param fd
 *      file descriptor to be removed.
 *
 * @return
 *      nothing.
 */
extern void queue_group_io_remove(struct queue_group *group, int fd);


/** Wait for data in any of the queues belonging to the group.
 * This function is allowed (on some conditions) to return even if there are
 * no items in any queue.
 *
 * @param group
 *      which queue group to wait on.
 *
 * @return
 *      nothing.
 */
extern void queue_group_wait(struct queue_group *group);


/** Wait for data in any of the queues belonging to the group.
 * This function is allowed (on some conditions) to return even if there are
 * no items in any queue.
 *
 * @param group
 *      which queue group to wait on.
 *
 * @param sec
 *      how many seconds to wait at most.
 *
 * @return
 *      nothing.
 */
extern void queue_group_tmwait(struct queue_group *group, time_t sec);


#endif

