/* $Id: iowait.h,v 1.1 2004/04/11 15:04:54 jirka Exp $ */
/*
 * IO events signaling support for queue groups.
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
 * IO events signaling support for queue groups.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.1 $ $Date: 2004/04/11 15:04:54 $
 */

#ifndef IOWAIT_H
#define IOWAIT_H

#if HAVE_PTHREAD_H
# include "pthread.h"
#endif

#include "error.h"
#include "queue.h"


/** Register file descriptor to signal queue group.
 *
 * @param fd
 *      file descriptor on which to wait for events.
 *
 * @param group
 *      group to be signaled when IO event occurs on fd.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int iowait_reg(EC, int fd, struct queue_group *group);


/** Unregister file descriptor.
 *
 * @param fd
 *      file descriptor to be unregistered.
 *
 * @return
 *      nothing.
 */
extern void iowait_unreg(int fd);


/** Start iowait thread.
 *
 * @param thid
 *      where to store POSIX thread identifier of the started thread.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int iowait_thread(EC, pthread_t *thid);

#endif

