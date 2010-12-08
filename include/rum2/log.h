/* $Id: log.h,v 1.10 2005/05/06 19:00:22 jirka Exp $ */
/*
 * Logging related functions.
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
 * Logging related functions.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.10 $ $Date: 2005/05/06 19:00:22 $
 */

#ifndef LOGGER_H
#define LOGGER_H

#if HAVE_STDARG_H
# include <stdarg.h>
#endif

#include "module.h"


/** Priority level of the log message.
 * The higher number the lower priority and the more detailed logging
 * information. One can use any value, those mentioned bellow are just
 * giving more semantics information on some values.
 */
enum log_level {
    /** Fatal error, the reflector will stop itself immediately. */
    LOG_FATAL   = 0,
    /** Nonfatal error, the reflector still remains operational. */
    LOG_ERROR   = 1,
    /** Conditions the user should take into account. */
    LOG_WARNING = 10,
    /** Normal but still significant conditions. */
    LOG_NOTICE  = 20,
    /** Informational messages. */
    LOG_INFO    = 30,
    /** Information useful for debugging. */
    LOG_DEBUG   = 40
};

/** Logging level used for printing function calls. */
#define LOG_DEBUG_FUN (LOG_DEBUG + 5)


/** Log a message.
 * The log message is sent to all registered msg-interface modules and
 * (according to settings, see log_stderr()) to standard error output.
 *
 * This function does not call the va_end macro. Consequently, the value of ap
 * is undefined after the call. The application should call va_end(ap) itself
 * afterwards.
 *
 * @param mclass
 *      class of the module which generated the log message.
 *
 * @param name
 *      name of that module.
 *
 * @param level
 *      priority level of the message.
 *
 * @param msg
 *      log message format.
 *
 * @param ap
 *      va_list containing arguments for the format string.
 *
 * @return
 *      nothing.
 */
extern void vlog(enum module_class mclass,
                 char *name,
                 enum log_level level,
                 char *msg,
                 va_list ap);


/** Log a message.
 *
 * @param mclass
 *      class of the module which generated the log message.
 *
 * @param name
 *      name of that module.
 *
 * @param level
 *      priority level of the message.
 *
 * @param msg, ...
 *      message format and arguments (as for printf(3))
 *
 * @return
 *      nothing.
 */
extern void rlog(enum module_class mclass, char *name,
                 enum log_level level,
                 char *msg, ...);


/** Log a message.
 * This function does the same as log() except that the originating module is
 * specified by its module description structure (instead of class and name).
 *
 * @param module
 *      pointer to the module identifier.
 *
 * @param level
 *      priority level of the message.
 *
 * @param msg, ...
 *      message format and arguments (as for printf(3))
 *
 * @return
 *      nothing.
 */
extern void logm(const struct module_id *module,
                 enum log_level level,
                 char *msg, ...);


/** Log an error message.
 * Error string constructed from error context will be preceded by a user
 * message (if it is non-NULL).
 *
 * @param mclass
 *      class of the module which generated the log message.
 *
 * @param name
 *      name of that module.
 *
 * @param level
 *      priority level of the message.
 *
 * @param errctx
 *      error context which the error message will be generated from.
 *
 * @param fmt, ...
 *      message format and its arguments to be prepended to error string. It
 *      can also be NULL if no message should preced th error string.
 *
 * @return
 *      nothing.
 */
extern void logerror(enum module_class mclass,
                     char *name,
                     enum log_level level,
                     struct rum_error_ctx *errctx,
                     char *fmt, ...);


/** Log an error message.
 * The message is generated using module::errctx.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @param level
 *      priority level of the message.
 *
 * @return
 *      nothing.
 */
extern void logerrorm(struct module *module, enum log_level level);


/** Add new source module.
 * This function is called by management/master module (more specifically
 * its module loading code in mod.c) whenever some new module is created
 * (even the management/master module itself).
 *
 * @param source
 *      description structure of the source module.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int log_source_add(struct module *source);


/** Remove source module.
 * This function is called by management/master module (more specifically
 * its module loading code in mod.c) whenever a module is being destroyed.
 *
 * @param source
 *      description structure of the source module.
 *
 * @return
 *      nothing.
 */
extern void log_source_remove(struct module *source);


/** Register msg-interface module as a logger for a given source module.
 *
 * @param msg_iface
 *      msg-interface module.
 *
 * @param source
 *      description structure of the source module.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int log_register(struct module *msg_iface, struct module *source);


/** Unregister msg-interface logger.
 *
 * @param msg_iface
 *      msg-interface module.
 *
 * @param source
 *      source module from which the msg-interface module no longer wants
 *      to receive log messages. If it is NULL, msg-interface will be
 *      unregistered from all source modules.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int log_unregister(struct module *msg_iface, struct module *source);


/** Change stderr logging behaviour.
 *
 * @param copy_all
 *      if set to zero, only log messages which have no listener to be sent
 *      to are written to stderr, otherwise all log messages are written to
 *      stderr.
 *
 * @return
 *      the old value.
 */
extern int log_stderr(int copy_all);


#endif

