/* $Id: route.h,v 1.3 2005/03/15 18:33:18 jirka Exp $ */
/*
 * Routing AAA module.
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
 * Routing AAA module.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.3 $ $Date: 2005/03/15 18:33:18 $
 */

#ifndef ROUTE_H
#define ROUTE_H

#include "utils.h"
#include "module.h"
#include "rap-types.h"


/** Initialize routing AAA module.
 * @sa module_initialize
 */
extern int route_initialize(struct module *module);


/** Check client's access permissions according to its IP and listener ID.
 *
 * @param module
 *      aaa/routing module description structure.
 *
 * @param client
 *      client's IP address.
 *
 * @param listener
 *      listener ID as obtained from management/session.
 *
 * @return
 *      access permissions.
 */
extern enum raphdr_access route_acl_check(struct module *module,
                                          const IN_ADDR *client,
                                          int listener);


#endif

