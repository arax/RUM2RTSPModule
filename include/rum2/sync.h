/* $Id: sync.h,v 1.1 2005/01/30 17:30:03 jirka Exp $ */
/*
 * Support for synchronous requests (Sync: on).
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
 * Support for synchronous requests (Sync: on).
 * @author Jiri Denemark
 * @date 2005
 * @version $Revision: 1.1 $ $Date: 2005/01/30 17:30:03 $
 */

#ifndef SYNC_H
#define SYNC_H

#include "error.h"
#include "rap-types.h"


/** Mark the beginning of synchronous or asynchronous request.
 * This function must be called before the request is pushed on input message
 * queue of management/master module. This function will block untill the 
 * previous synchronous request is finished. Thus in any time, exactly one of
 * the following can happen:
 *      -# no request is being processed
 *      -# exactly one synchronous request is being processed
 *      -# any number os asynchronous requests are being processed
 *
 * @param sync
 *      type of the request.
 *
 * @param iface_name
 *      name of a module which received the request.
 *
 * @return
 *      nothing.
 */
extern void sync_request(enum raphdr_sync sync, const char *iface_name);


/** Mark the end of synchronous or asynchronous request.
 * This function must be called after pushing the response on input message
 * queue of a particular msg-interface module.
 *
 * @param sync
 *      type of the request.
 *
 * @param iface_name
 *      name of a module which received the request.
 *
 * @return
 *      nothing.
 */
extern void sync_response(enum raphdr_sync sync, const char *iface_name);


/** Enable or disable request-in-request (RIR) mode.
 * RIR mode is a mode in which one specific module (that one specified as an
 * argument to this function) can send requests during processing of another
 * request which has enabled RIR. Only synchronous request can enable RIR. All
 * RIR requests are treated as synchronous regardless of Sync header.
 *
 * Though RIR can be disabled using this function, one should not do that, as
 * RIR is disabled automatically at the end of synchronous request during
 * which RIR was enabled.
 *
 * @param iface_name
 *      name of a msg-interface class module which can send RIR requests.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int sync_request_in_request(EC, const char *iface_name);


#endif

