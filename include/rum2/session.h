/* $Id: session.h,v 1.7 2005/03/15 18:56:10 jirka Exp $ */
/*
 * Session management module.
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
 * Session management module.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.7 $ $Date: 2005/03/15 18:56:10 $
 */

#ifndef SESSION_H
#define SESSION_H

#include "utils.h"
#include "module.h"


/** Initialize session management module.
 * @sa module_initialize
 */
extern int session_initialize(struct module *module);


/** Register listener module to session management.
 *
 * @param listener
 *      listener module identifier.
 *
 * @param sock
 *      socket to be used for sending data received by the listener.
 *
 * @return
 *      listener ID on success, -1 otherwise.
 */
extern int session_listener(EC, const struct module_id *listener, int sock);


/** Get name of listener module.
 *
 * @param listener
 *      listener ID which has to be converted to name of listener module.
 *
 * @return
 *      name of particular listener module or NULL when no such module exists.
 */
extern const char *session_listener_name(int listener);


/** Add client to session.
 */
extern void session_client(int listener, const IN_ADDR *client);


/** Get session's clients list.
 * This function returns a copy of client list. The list must be freed using
 * mem_free() function when it is no longer needed.
 *
 * @param listener
 *      listener ID which determines the session.
 *
 * @param count
 *      where to place number of clients.
 *
 * @return
 *      pointer to the first client in array.
 */
extern struct client *session_client_list(int listener, int *count);


/** Get socket for sending data received by a given listener.
 *
 * @param listener
 *      ID of the listener which has received the data.
 *
 * @return
 *      socket identifier.
 */
extern int session_socket(int listener);


/** Increment session's received data counter.
 *
 * @param listener
 *      listener ID which determines the session.
 *
 * @param bytes
 *      number of bytes received.
 *
 * @return
 *      nothing.
 */
extern void session_inc_recv(int listener, unsigned long bytes);


/** Increment session's sent data counter.
 *
 * @param listener
 *      listener ID which determines the session.
 *
 * @param bytes
 *      number of bytes sent.
 *
 * @return
 *      nothing.
 */
extern void session_inc_sent(int listener, unsigned long bytes);


#endif

