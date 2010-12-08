/* $Id: event.h,v 1.2 2006/04/09 12:46:12 jirka Exp $ */
/*
 * Event types.
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
 * Event types and helper functions.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.2 $ $Date: 2006/04/09 12:46:12 $
 */

#ifndef EVENT_H
#define EVENT_H


/** RUM events. */
enum event {
    /** Event which is sent to any msg-interface module when a new module is
     * registered as a log source.
     *
     * Pointer to a description structure of the source module is passed as
     * an argument (struct module *).
     */
    EVENT_LOG_SOURCE_NEW,

    /** This event is sent to all msg-interface modules which are registered
     * for receiving log messages from some module which is about to be
     * removed.
     *
     * It takes (struct module *) as an argument which is a pointer to a
     * description structure of a module being removed.
     */
    EVENT_LOG_SOURCE_REMOVED,

    /** Event which occures when any change is made on a list of clients of a
     * particular session. This event is sent to all listener modules which
     * form the session of which client list changes.
     *
     * The event takes (struct client_list *) as an argument pointing to the
     * affected list of clients. The pointer is valid only during the time of
     * processing this event. The event callback must not call any session
     * related function when processing this event to avoid possible deadlock.
     */
    EVENT_CLIENT_LIST
};


#endif

