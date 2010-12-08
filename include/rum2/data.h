/* $Id: data.h,v 1.20 2006/12/17 22:11:56 jirka Exp $ */
/*
 * Reflector data structure and related functions.
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
 * Reflector data structure and related functions.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.20 $ $Date: 2006/12/17 22:11:56 $
 */

#ifndef DATA_H
#define DATA_H

#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_PTHREAD_H
# include <pthread.h>
#endif

#include "limits.h"
#include "utils.h"
#include "error.h"
#include "module.h"
#include "clients.h"
#include "session.h"
#if RUM_PROFILE
# include "profile.h"
#endif


/** Reflector data structure. */
struct data {
    /** Reference counter. */
    int ref_count;
    /** Mutex for accessing reference counter. */
    pthread_mutex_t rcmutex;

    /** Identifier of the session the packet belongs to. */
    int session;

    /** Port the data was received on. */
    uint16_t port;
    /** Name of the listener module which received the data. */
    char name[RUM_LISTENER_LEN];

    /** Sender who sent the data. */
    ADDR_TYPE source;

    /** Data buffer. */
    void *buffer;
    /** Number of bytes stored in the buffer. */
    long size;
};


/** Processor path.
 * The content of this structure must never change. The structure can only be
 * freed after it is created and filled in. Memory for the structure is
 * obtained using mem_new() and reference counters are used. */
struct path {
    /** Pointers to description structures of modules in the list. */
    struct module *mod[RUM_PATH_LEN];
    /** Array of module names.
     * The names pointed to by this array are actually stored one by one at
     * the end of this structure within the same memory block. */
    char *names[RUM_PATH_LEN];
};


/** Metadata (client list and paths) for data structure.
 * Each metadata structure must be referenced only once. If another reference
 * is needed, a copy of metadata structure must be made. */
struct meta {
#if RUM_PROFILE
    /** Data for packet profiling. */
    struct profile_packet profile;
#endif
    /** Pointer to data description structure. */
    struct data *data;
    /** Array of clients which the data are to be sent to.
     * In fact, the data will be sent to particular client if and only if
     * there is a @c 1 set for that client in @c mask. */
    struct client *client;
    /** Number of items in @c client array. */
    int count;
    /** Index of the next node in the path. */
    int next_node;
    /** Bit mask which selects valid clients from @c client array.
     * The mask is stored at the end of this structure within the same memory
     * block. */
    unsigned long *mask;
    /** Path for each client in the @c client array.
     * Paths are stored after client mask. */
    struct path **path;
};


/** Number of bits in an item of meta::mask array. */
#define MMASK_BITS  (sizeof(unsigned long) * 8)


/** Initialize new data structure.
 * The main purpose of this function is to take care of reference counter.
 *
 * @param data
 *      data structure to be initialized.
 *
 * @param session
 *      session identifier.
 *
 * @return
 *      nothing.
 */
extern void data_init(struct data *data,
                      int session);


/** Free all the memory occupied by data structure (including buffer).
 * In fact this function decrements data::ref_count. The memory will be
 * freed iff data::ref_count drops to zero.
 *
 * @param data
 *      data structure to be freed typecasted to (void *).
 *
 * @return
 *      nothing.
 */
extern void data_free(void *data);


/** Increment reference counter of data structure.
 *
 * @param data
 *      data structure to be copied.
 *
 * @return
 *      nothing.
 */
extern void data_ref(struct data *data);


/** Function used by data_writable() for copying data buffer.
 * This function must allocate new buffer using mem_new() function, copy
 * (or even transform) data from the original buffer to the new one, and set
 * data::buffer and data::size appropriately. This function must NOT touch
 * any other fields of data structure.
 *
 * @warning
 *      Both @c orig and @c copy argument can point to the same structure
 *      when in-place copy (trasformation) is to be done (this does not
 *      mean that the copy must be stored in the same buffer as the original
 *      data were stored). This function must be able to deal with this
 *      situation. Possibly by modifying fields of @c copy structure just
 *      before successful end of this function.
 *
 * @param orig
 *      data structure which has to be copied.
 *
 * @param copy
 *      data structure where the copy has to be stored. This structure must
 *      remain unchanged when this function fails.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
typedef int (*data_copy_fn)(EC, const struct data *orig, struct data *copy);


/** Make data structure writable.
 * If there is only one reference to this data structure, this function does
 * nothing. In other cases it performs deep copy of the structure, resets
 * reference counter within the new structure, and decrements number of
 * references to the original data structure. That is one of the references
 * to the data structure is replaced by a writable copy.
 *
 * @param data
 *      the original data structure.
 *
 * @param copy
 *      this function is used to copy data buffer. If the function pointer is
 *      NULL default copying is done (i.e. an exact copy of the original
 *      buffer is created).
 *
 * @return
 *      pointer to the data structure that can be safely changed or NULL
 *      on error.
 */
extern struct data *data_writable(EC, struct data *data, data_copy_fn copy);


/** Create new mostly empty metadata structure for a given data packet.
 * This function is used to create a structure where profiling information
 * can be stored before a real metadata structure is created.
 *
 * @warning All fields except meta::profile and meta::data should be
 * considered uninitialized and as such should never be accessed.
 *
 * @warning This function should never be called from any module but
 * listener-class one.
 *
 * If the reflector is compiled without support for packet profiling, this
 * function becomes a macro which just returns a pointer to the original data
 * structure.
 *
 * @param data
 *      data structure to be pointed to by meta::data. Reference to data
 *      is NOT incremented.
 *
 * @return
 *      pointer to a new "meta" structure or to a data structure passed on as
 *      the first argument. The behaviour depends on RUM_PROFILE macro.
 */
#if RUM_PROFILE
extern void *meta_new_simple(EC, struct data *data);
#else
# define meta_new_simple(errctx, data)    ((void *) (data))
#endif


/** Create new metadata structure for a given data and client list.
 *
 * @param data
 *      data structure to be pointed to by meta::data. Reference to data
 *      is NOT incremented.
 *
 * @param clients
 *      client list. Reference to it is NOT automatically incremented.
 *
 * @param count
 *      number of clients in the list. The number determines the amount of
 *      memory needed for metadata structure.
 *
 * @return
 *      pointer to the new metadata structure or NULL on failure.
 */
extern struct meta *meta_new(EC, struct data *data,
                             struct client *clients,
                             int count);


/** Make a copy of metadata structure.
 *
 * @param meta
 *      original metadata structure.
 *
 * @return
 *      pointer to the copied structure, NULL on error.
 */
extern struct meta *meta_copy(EC, const struct meta *meta);


/** Mark all clients as (in)valid.
 *
 * @param meta
 *      metadata structure.
 *
 * @param valid
 *      if zero, all clients are marked as invalid. Otherwise they are marked
 *      as valid.
 *
 * @return
 *      nothing.
 */
extern void meta_mask_all(struct meta *meta, int valid);


/** Helper macro for meta_mask_* macros. */
#define MASK_BYTE(i)    ((i) / MMASK_BITS)

/** Helper macro for meta_mask_* macros. */
#define MASK_BIT(i)     ((i) % MMASK_BITS)


/** Get (in)valid bit value for a specified client.
 *
 * @param meta
 *      pointer to a metadata structure.
 *
 * @param client
 *      index of the client in meta::client array.
 *
 * @return
 *      zero when the client is invalid; nonzero when it is valid.
 */
#define meta_mask_get(meta, client)                                         \
    (((client) < 0 || (client) >= ((meta)->count))                          \
        ? 0                                                                 \
        : (((meta)->mask[MASK_BYTE(client)] & (1 << MASK_BIT(client))) != 0))


/** Set (in)vlaid bit for a specified client.
 *
 * @param meta
 *      pointer to a metadata structure.
 *
 * @param client
 *      index of the client in meta::client array.
 *
 * @param valid
 *      if zero, the client is marked as invalid, otherwise it is marked as
 *      valid.
 *
 * @return
 *      nothing.
 */
#define meta_mask_set(meta, client, valid)                                  \
    (void)                                                                  \
    (((client) >= 0 && (client) < (meta)->count)                             \
     && ((valid)                                                            \
         ? ((meta)->mask[MASK_BYTE(client)] |= (1 << MASK_BIT(client)))     \
         : ((meta)->mask[MASK_BYTE(client)] &= ~(1 << MASK_BIT(client)))))


/** Free all the memory occupied by metadata structure as well as data in it.
 * This function calls data_free() to free the data pointed to from metadata
 * structure. Client list meta::client is freed using mem_free().
 *
 * @param meta
 *      metadata structure to be freed typecasted to (void *).
 *
 * @return
 *      nothing.
 */
extern void meta_free(void *meta);


#endif

