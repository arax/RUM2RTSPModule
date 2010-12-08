/* $Id: sender.h,v 1.2 2005/03/15 20:16:42 jirka Exp $ */
/*
 * Sender module.
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
 * Sender module.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.2 $ $Date: 2005/03/15 20:16:42 $
 */

#ifndef SENDER_H
#define SENDER_H

#include "module.h"


/** Initialize sender module.
 * @sa module_initialize
 */
extern int sender_initialize(struct module *module);


#endif

