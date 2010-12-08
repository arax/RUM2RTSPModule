/* $Id: clients.h,v 1.4 2007/01/23 13:35:18 jirka Exp $ */
/*
 * Client list for session management.
 * Copyright (C) 2004 Jiri Denemark
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
 * Client list for session management.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.4 $ $Date: 2007/01/23 13:35:18 $
 */

#ifndef CLIENTS_H
#define CLIENTS_H

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include "utils.h"


/** Client description structure. */
struct client {
    /** Client's IP address. */
    IN_ADDR ip;
    /** Timestamp of last packet received from this client.
     * Measured in seconds since the Epoch. */
    time_t last;
    /** Nonzero if the client was added manually and with no timeout.
     * Therefore, it shell never be automatically removed from the list. */
    int permanent;
    /** ID of the listener which added the client to the list. */
    int listener;
};


/** List of clients. */
struct client_list {
    /** Number of clients in the list. */
    int count;
    /** Array of clients.
     * The array is allocated using memory management module. */
    struct client *list;
};


/** Initialize client list.
 *
 * @param clients
 *      where to place pointer to the client list.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int clients_init(EC, struct client_list **clients);


/** Free all the memory occupied by client list.
 *
 * @param clients
 *      pointer to the client list passed on by reference.
 *
 * @return
 *      nothing.
 */
extern void clients_free(struct client_list **clients);


/** Add client into client list.
 * This call is equivalent to clients_add_tm() with timeout set to 0.
 *
 * @param clients
 *      pointer to the client list.
 *
 * @param client
 *      IP address of the client.
 *
 * @param session
 *      session ID.
 *
 * @param listener
 *      ID of the listener which added the client to the list.
 *
 * @return
 *      nonzero if the client list has changed, nonzero otherwise.
 */
#define clients_add(clients, client, session, listener)   \
    clients_add_tm((clients), (client), 0, (session), (listener))


/** Add client into client list and set a given timeout it.
 *
 * @param clients
 *      pointer to the client list.
 *
 * @param client
 *      IP address of the client.
 *
 * @param timeout
 *      the client can be automatically removed from the list only after
 *      the given period of time since it was added there. A value of -1
 *      means the client will never be removed automatically.
 *
 * @param session
 *      session ID.
 *
 * @param listener
 *      ID of the listener which added the client to the list.
 *
 * @return
 *      nonzero if the client list has changed, nonzero otherwise.
 */
extern int clients_add_tm(struct client_list *clients,
                          const IN_ADDR *client,
                          long timeout,
                          int session,
                          int listener);


/** Remove stale clients.
 * Session mutex is supposed to be already locked.
 *
 * @param clients
 *      pointer to the client list.
 *
 * @param last
 *      clients having their last-sent time older than or equal to this time
 *      will be removed.
 *
 * @param session
 *      session ID.
 *
 * @return
 *      nonzero if the client list has changed, nonzero otherwise.
 */
extern int clients_remove_stale(struct client_list *clients,
                                time_t last,
                                int session);


/** Remove a particular client.
 * Session mutex is supposed to be already locked.
 *
 * @param clients
 *      pointer to the client list.
 *
 * @param client
 *      IP address of the client.
 *
 * @param mask
 *      mask to apply to IP addresses before trying to match them.
 *
 * @param session
 *      session ID.
 *
 * @return
 *      nonzero if the client list has changed, nonzero otherwise.
 */
extern int clients_remove(struct client_list *clients,
                          const IN_ADDR *client,
                          int mask,
                          int session);


/** Copy all the clients from one list into another.
 * Both source session and dest. session mutexes are supposed to be already
 * locked.
 *
 * @param dst
 *      target client list.
 *
 * @param src
 *      source client list.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int clients_copy(EC, struct client_list *dst,
                        struct client_list *src);


#endif

