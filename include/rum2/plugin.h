/* $Id: plugin.h,v 1.3 2005/03/20 21:23:29 jirka Exp $ */
/*
 * Functions for dealing with dynamically loadable modules.
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
 * Functions for dealing with dynamically loadable modules.
 * @author Jiri Denemark
 * @date 2005
 * @version $Revision: 1.3 $ $Date: 2005/03/20 21:23:29 $
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include "module.h"
#include "rap-types.h"


/** Initialize plugin loading subsystem.
 * This function must be called before any call to plugin_load().
 *
 * @return
 *      NULL on success, pointer to an error string otherwise.
 */
extern const char *plugin_initialize(void);


/** Set plugin search path.
 *
 * @param path
 *      where the reflector should search for plugins.
 *
 * @return
 *      nothing.
 */
extern void plugin_path(const char *path);


/** Find and load plugin with a module according to its identifier.
 * Just before successful end of this function, the module is properly
 * initialized (i.e. module's initialize() function is called).
 *
 * Path to a dynamically loadable library containing the module is constructed
 * in the following way
 * <code>
 *      library_name = plugindir "/" module_class(mclass) "/" name
 * </code>
 * and then it is loaded using libltdl so that library's extension is
 * determined automatically.
 *
 * @param mclass
 *      class of the module.
 *
 * @param name
 *      name of the module.
 *
 * @param mod
 *      pointer to a module description structure which is to be initialized
 *      for the module.
 *
 * @return
 *      zero on success, nonzero otherwise. The module description structure
 *      is returned to uninitialized state on any error.
 */
extern int plugin_load(enum module_class mclass,
                       const char *name,
                       struct module *mod);


/** Unload module's plugin.
 * Does nothing in case mod->plugin is NULL, i.e. when the module was not
 * loaded as a plugin.
 *
 * @param mod
 *      pointer to a module's description structure.
 *
 * @return
 *      nothing.
 */
extern void plugin_unload(struct module *mod);


/** Get a name of module's plugin file.
 *
 * @param mod
 *      pointer to a module's description structure.
 *
 * @return
 *      plugin filename or NULL if the module is linked statically.
 */
extern const char *plugin_filename(const struct module *mod);


/** Generate a list of available dynamically loadable modules of a given class.
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
extern int plugin_avail(EC, enum module_class cls,
                        struct rap_content *cont,
                        struct module *mod);

#endif

