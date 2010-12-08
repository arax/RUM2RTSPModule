/* $Id: processor.h,v 1.5 2005/03/15 16:49:19 jirka Exp $ */
/*
 * Processor master module.
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
 * Processor master module.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.5 $ $Date: 2005/03/15 16:49:19 $
 */

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "utils.h"
#include "module.h"
#include "data.h"


/** Initialize processor module.
 * @sa module_initialize
 */
extern int processor_initialize(struct module *module);


/** Compute path for given source and destination address and listener module.
 * The function automatically increments reference counter of that path.
 *
 * @param mod
 *      processor/master module.
 *
 * @param from
 *      source IP address.
 *
 * @param to
 *      destination IP address.
 *
 * @param listener
 *      identifier of listener module.
 *
 * @return
 *      pointer to the computed path. When the path is empty, i.e. data are to
 *      be passed on directly to sender/master, NULL is returned.
 */
extern struct path *processor_path(struct module *mod,
                                   const IN_ADDR *from,
                                   const IN_ADDR *to,
                                   int listener);


/** Pass on data to the next processor module in path.
 *
 * @param mod   
 *      processor/master module.
 *
 * @param meta
 *      metadata of the data to be passed on.
 *
 * @return
 *      nothing.
 */
extern void processor_path_pass(struct module *mod, struct meta *meta);

#endif

