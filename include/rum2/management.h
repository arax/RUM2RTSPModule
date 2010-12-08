/* $Id: management.h,v 1.1 2003/12/30 16:46:40 jirka Exp $ */
/*
 * Management master module.
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
 * Management master module.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.1 $ $Date: 2003/12/30 16:46:40 $
 */

#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include "module.h"


/** Initialize management/master module.
 * Initialize module parameters structure and module interface.
 *
 * @param module
 *      pointer to the module structure which has to be initialized.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int management_initialize(struct module *module);


#endif

