/* $Id: mod.h,v 1.18 2007/01/28 12:30:21 jirka Exp $ */
/*
 * Reflector module management code.
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
 * Reflector module management code.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.18 $ $Date: 2007/01/28 12:30:21 $
 */

#ifndef MOD_H
#define MOD_H

#if HAVE_PTHREAD_H
# include <pthread.h>
#endif

#include "error.h"
#include "module.h"
#include "rap-types.h"


/** Module status enumeration. */
enum module_status {
    MS_INCORRECT,
    MS_STOPPED,
    MS_RUNNING
};


/** Find and (if needed) load module according to its identifier.
 *
 * @param mclass
 *      class of the module.
 *
 * @param name
 *      name of the module.
 *
 * @return
 *      pointer to a module description structure initialized for the given
 *      module on success, NULL on error or when there is no statically
 *      linked nor dynamically loadable module matching the given identifier.
 */
extern struct module *module_load(EC, enum module_class mclass,
                                  const char *name);


/** Unload module.
 * Unload module's plugin if the module was loaded dynamically, clean module's
 * description structure and free it.
 *
 * @param mod
 *      pointer to a module's description structure.
 *
 * @return
 *      nothing.
 */
extern void module_unload(struct module **mod);


/** Run module_interface::name() function of the given module.
 */
#define module_name(module, id) \
    (((module)->iface->name != NULL)        \
     ? (module)->iface->name(module, id)    \
     : 0)


/** Run module_interface::conflicts() function of the given module.
 */
#define module_conflicts(module, ids, re) \
    (((module)->iface->conflicts != NULL)                   \
     ? (module)->iface->conflicts((module), (ids), (re))    \
     : 0)


/** Run module_interface::init() function of the given module.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
#define module_init(module) \
    ((module)->iface->init(module))


/** Start module.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int module_start(struct module *module);


/** Stop module's main thread.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int module_stop(struct module *module);


/** Clean module's description structure.
 * Field module::data is ensured to be NULL at the end of this macro.
 * @sa module_interface::clean()
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @param for_restart
 *      if nonzero, the module is being restarted. Otherwise it is being
 *      stopped.
 *
 * @return
 *      nothing.
 */
extern void module_clean(struct module *module, int for_restart);


/** Query module status.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @return
 *      module status.
 */
extern enum module_status module_status(struct module *module);


/** Insert a new module to the module list.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int mod_insert(struct module *module);


/** Search for the specified module in the module list.
 *
 * @param cls
 *      a class of the module in demand.
 *
 * @param name
 *      module name to search for.
 *
 * @param re
 *      indicates whether the name should be treated as POSIX.2 extended
 *      regular expression. A value of @c 0 means names are matched literally,
 *      nonzero means the name is a regular expression. Only the first matching
 *      module is returned in case of regular expression matching.
 *
 * @return
 *      pointer to the module description structure or NULL.
 */
extern struct module *mod_find(enum module_class cls, char *name, int re);


/** Find a given module and return its unique number.
 *
 * @param cls
 *      a class of the module in demand.
 *
 * @param name
 *      module name.
 *
 * @return
 *      module number or -1 on error.
 */
extern int mod_number(enum module_class cls, char *name);


/** Remove the module from the module list.
 *
 * @param cls
 *      class of the module to be removed.
 *
 * @param
 *      name of the module to be removed.
 *
 * @return
 *      nothing.
 */
extern void mod_remove(enum module_class cls, char *name);


/** Foreach function type.
 *
 * @param module
 *      the module this function is called for.
 *
 * @param arg
 *      additional data passed to mod_foreach().
 *
 * @return
 *      zero when mod_foreach() should continue in processing the rest
 *      of the modules; nonzero when mod_foreach() has to break the
 *      processing.
 */
typedef int (*foreach_function)(struct module *module, void *arg);

/** Call the specified function for each module of the given class.
 * If MC_REFLECTOR class is given for cls, the function is called for all
 * reflector modules from all classes.
 *
 * @param cls
 *      modules of which class have to be processed.
 *
 * @param fun
 *      function to be called for each of the modules.
 *
 * @param arg
 *      additional data for fun.
 *
 * @return
 *      nothing.
 */
extern void mod_foreach(enum module_class cls,
                        foreach_function fun,
                        void *arg);


/** Generate a list of available modules and their parameters.
 *
 * @param cont
 *      pointer to a content structure of a RAP response.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int mod_avail(EC, struct rap_content *cont);


/** Create a description of one available module and add it to a RAP response.
 *
 * @param mod
 *      pointer to the module description structure.
 *
 * @param cont
 *      pointer to a content structure of a RAP response.
 *
 * @return  
 *      zero on success, nonzero otherwise.
 */
extern int mod_avail_line(EC, const struct module *mod,
                          struct rap_content *cont);


/** Generate a list of running modules and their parameters.
 *
 * @param cont
 *      pointer to a content structure of a RAP response.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int mod_list(EC, struct rap_content *cont);

#endif

