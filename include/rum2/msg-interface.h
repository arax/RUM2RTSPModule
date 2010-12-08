/* $Id: msg-interface.h,v 1.4 2005/04/06 09:10:01 jirka Exp $ */
/*
 * General code for msg-interface modules.
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
 * General code for msg-interface modules.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.4 $ $Date: 2005/04/06 09:10:01 $
 */

#ifndef MSGIFACE_H
#define MSGIFACE_H

#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_PTHREAD_H
# include <pthread.h>
#endif

#include "error.h"
#include "module.h"
#include "rap-types.h"


/** Common part of msg-interface module internal data.
 * This sctructure must always be the first element of a particular module
 * internal data structure.
 */
struct msgiface_data {
    /** Number of log sources in msgiface_data::sources array. */
    int source_count;
    /** Array of log sources.
     * To save number of memory blocks needed to allocate module names are
     * stored within the same memory block as the list of sources just behind
     * the last item (i.e. sources[1].name points to (sources + source_count).
     */
    struct module_id *sources;
    /** Mutex for accessing list of sources. */
    pthread_mutex_t src_mutex;
};


/** Parser description structure. */
struct msgiface_parser {
    /** Pointer to the previous parser. */
    struct msgiface_parser *prev;
    /** Pointer to the next parser. */
    struct msgiface_parser *next;
    /** Internal parser ID, i.e. without module ID part. */
    unsigned int int_id;
    /** Parser ID. */
    uint32_t id;
    /** File descriptor. */
    int fd;
    /** Parser thread. */
    pthread_t thread;
};


/** List of RAP parsers sorted by parser ID. */
struct msgiface_plist {
    /** Numeric identifier of the module owning the parser list. */
    int module;
    /** Pointer to the first parser in the list. */
    struct msgiface_parser *first;
    /** Pointer to the last parser in the list. */
    struct msgiface_parser *last;
    /** Internal ID to be assigned to the next parser.
     * It is automatically incremented with each new parser and thus after
     * RUM_PARSER_MAX parsers it reverts back to zero. */
    unsigned int next_id;
};


/** Initialize common part of module internal data.
 *
 * @param module
 *      msg-interface module.
 *
 * @return
 *      nothing.
 */
extern void msgiface_data_init(struct module *module);


/** Free all additional memory used by common part of module internal data.
 *
 * @param module
 *      msg-interface module.
 *
 * @return
 *      nothing.
 */
extern void msgiface_data_clean(struct module *module);


/** Initialize parser list.
 *
 * @param list
 *      parser list.
 *
 * @param module
 *      numeric identifier of the module owning the parser list.
 *
 * @return
 *      nothing.
 */
extern void msgiface_plist_init(struct msgiface_plist *list, int module);


/** Add new parser to a list (its thread has to be started separately).
 *
 * @param list
 *      parser list.
 *
 * @param fd
 *      file descriptor the new parser will use for communication.
 *
 * @return
 *      pointer to the new parser structure or NULL on error.
 */
extern struct msgiface_parser *msgiface_new_parser(EC,
                    struct msgiface_plist *list, int fd);


/** Find parser identified by a given ID.
 *
 * @param list
 *      parser list.
 *
 * @param id
 *      parser ID.
 *
 * @return
 *      pointer to a parser or NULL when no parser with a given ID is found.
 */
extern struct msgiface_parser *msgiface_find_parser(
                    struct msgiface_plist *list, unsigned int id);


/** Remove and optionally stop parser identified by a given ID.
 *
 * @param list
 *      parser list.
 *
 * @param id
 *      parser ID.
 *
 * @param stop
 *      nonzero if the parser has to be stopped when removed.
 *
 * @return
 *      nothing.
 */
extern void msgiface_remove_parser(struct msgiface_plist *list,
                                   unsigned int id,
                                   int stop);


/** Set list of sources which the module wants to receive log messages from.
 *
 * @param module
 *      msg-interface module.
 *
 * @param ids
 *      single linked list of source module ids.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int msgiface_log_sources(struct module *module,
                                const struct rap_module_list *ids);


/** Event handler for EVENT_LOG_SOURCE_NEW.
 * @see module_interface::events().
 */
extern void msgiface_log_event(struct module *module,
                               enum event event,
                               void *arg);

#endif

