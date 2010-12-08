/* $Id: modparam.h,v 1.8 2006/03/17 22:04:16 jirka Exp $ */
/*
 * Module startup parameters.
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
 * Module startup parameters.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.8 $ $Date: 2006/03/17 22:04:16 $
 */

#ifndef MODPARAM_H
#define MODPARAM_H

#include "error.h"
#include "module.h"


/** Get value of module parameter.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @param name
 *      parameter name.
 *
 * @return
 *       parameter value. When this function returns NULL, it can also be
 *       an error => check module::errctx.
 */
extern char *modparam_get(struct module *module, const char *name);


/** Set value of module parameter.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @param name
 *      parameter name.
 *
 * @param value
 *      parameter value. The function creates a copy of the value.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int modparam_set(struct module *module,
                        const char *name, 
                        const char *value);


/** Unset module parameter (reset it to tis default value).
 * Current parameter value (if any) will be free()d.
 * 
 * @param module
 *      pointer to the module description structure.
 *
 * @param name
 *      parameter name.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int modparam_unset(struct module *module, const char *name);


/** Initialize startup parameters.
 * This function allocates a memory for startup parameters, makes a copy of
 * them from @c params and sets module_param::next pointers. The function
 * also checks description field and returns an error if it is incorrect.
 *
 * Usual call to this function looks like the following:
 * <code>
 *  modparam_init(mod, params, sizeof(params) / sizeof(struct module_param))
 * </code>
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @param params
 *      original read-only list of startup parameters.
 *
 * @param count
 *      count of module's startup parameters.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int modparam_init(struct module *module,
                         const struct module_param *params,
                         int count);


/** Clean all the startup parameters.
 * All resources occupied by a list of startup parameters and their values
 * are freed.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @return
 *      nothing.
 */
extern void modparam_clean(struct module *module);


#endif

