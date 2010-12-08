/* $Id: mod-static.h,v 1.2 2005/03/20 21:23:29 jirka Exp $ */
/*
 * List of statically linked modules.
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
 * List of statically linked modules.
 * @author Jiri Denemark
 * @date 2005
 * @version $Revision: 1.2 $ $Date: 2005/03/20 21:23:29 $
 */

#ifndef MOD_STATIC_H
#define MOD_STATIC_H

#include "module.h"
#include "rap-types.h"


/** Find statically linked module according to its identifier.
 *
 * @param mclass
 *      module's class.
 *
 * @param name
 *      module's name.
 *
 * @return
 *      pointer to the module's initialize() function or NULL when there is no
 *      statically linked module matching the given identifier.
 */
extern module_initialize mod_static(enum module_class mclass,
                                    const char *name);


/** Generate a list of available static modules of a given class.
 *
 * @param cls
 *      module class a caller is interested in.
 *
 * @param cont
 *      pointer to a content of a RAP response.
 *
 * @param mod
 *      pointer to a temporary module description structure.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int mod_static_avail(EC, enum module_class cls,
                            struct rap_content *cont,
                            struct module *mod);

#endif

