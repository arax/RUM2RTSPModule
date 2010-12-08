/* $Id: profile.h,v 1.2 2005/05/06 16:56:16 jirka Exp $ */
/*
 * Packets profiling.
 * Copyright (C) 2005 Jiri Denemark
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
 * Packets profiling.
 * @author Jiri Denemark
 * @date 2005
 * @version $Revision: 1.2 $ $Date: 2005/05/06 16:56:16 $
 */

#ifndef PROFILE_H
#define PROFILE_H
# if RUM_PROFILE

# if HAVE_STDINT_H
#  include <stdint.h>
# endif

#include "utils.h"


/** Template for a profile's file name. */
#define PROFILE         "rum2.profile.%d"


/** Sampling frequency (in Hz).
 * @sa profile_write().
 */
#define SAMPLING_FRQ    2


/** Data structure for packet profiling. */
struct profile_packet {
    /** When a packet was received. */
    uint64_t received;
    /** When a packet was started to being processed by sender module. */
    uint64_t pre_send;
    /** Number of ticks a packet was waiting in queues for. */
    uint64_t in_queue;
    /** When a packet was pushed into a current queue. */
    uint64_t enqueued;
};


/** Compute cpu ticks difference.
 *
 * @param new_t
 *      a more recent number of cpu ticks.
 *
 * @param old_t
 *      a former number of cpu ticks.
 *
 * @return
 *      number of ticks between @c new_t and @c old_t.
 */
#define tick_diff(new_t, old_t) \
    ((new_t > old_t) ? new_t - old_t : (uint64_t) (-1) - old_t + new_t)


/** Get current number of cpu ticks.
 *
 * @param ticks
 *      (uint64_t) variable where cpu ticks are to be stored
 *      (note @c ticks is not a pointer).
 *
 * @return
 *      nothing.
 */
#define get_ticks(ticks) \
    { asm volatile("rdtsc":"=A"(ticks)); }


/** Initialize reflector profiling code.
 *
 * @return
 *      nothing.
 */
extern void profile_init(void);


/** Initialize packet's profiling data.
 * This macro is intended to be called just after a packet is received.
 *
 * @param packet
 *      pointer to a profile_packet structure.
 *
 * @return
 *      nothing.
 */
#define profile_recv(packet) {      \
    get_ticks((packet)->received);  \
    (packet)->pre_send = 0;         \
    (packet)->in_queue = 0;         \
}


/** Call this function when a packet is pushed into a queue.
 *
 * @param packet
 *      pointer to a profile_packet structure.
 *
 * @return
 *      nothing.
 */
#define profile_push(packet)        \
    get_ticks((packet)->enqueued)


/** Call this function when a packet is popped from a queue.
 *
 * @param packet
 *      pointer to a profile_packet structure.
 *
 * @return
 *      nothing.
 */
#define profile_pop(packet) {                                   \
    register uint64_t now;                                      \
    get_ticks(now);                                             \
    (packet)->in_queue += tick_diff(now, (packet)->enqueued);   \
}


/** Call this macro before starting to send it.
 *
 * @param packet
 *      pointer to a profile_packet structure.
 *
 * @return
 *      nothing.
 */
#define profile_send_start(packet)  \
    get_ticks((packet)->pre_send)


/** Write packet's profiling data into a file.
 * This function is supposed to be called just after sending of a packet is
 * finished.
 *
 * Profiling information is stored at SAMPLING_FRQ at most. All other packets
 * are ignored.
 *
 * @param source
 *      source IP address.
 *
 * @param port
 *      port the packet was received on.
 *
 * @param
 *      size of the packet.
 *
 * @param packet
 *      pointer to a packet's profiling data.
 *
 * @return
 *      nothing.
 */
extern void profile_write(ADDR_TYPE source,
                          uint16_t port,
                          long size,
                          struct profile_packet *packet);


# endif
#endif

