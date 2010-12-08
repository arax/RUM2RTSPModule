/* $Id: error.h,v 1.38 2007/01/09 13:21:18 jirka Exp $ */
/*
 * Error codes and strings.
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
 * Error codes and strings.
 *
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.38 $ $Date: 2007/01/09 13:21:18 $
 *
 * All functions which expect pointer to error context also accept NULL as
 * that pointer. In such a case, the function does nothing and returns
 * success.
 */

#ifndef ERROR_H
#define ERROR_H

#include "limits.h"


/** Reflector error identifiers. */
enum rum_error {
    RUM_ENO_ERROR,
    RUM_ENO_MEMORY,
    RUM_EQUEUE_INIT,
    RUM_EQUEUE_GROUP_INIT,
    RUM_EQUEUE_GROUP_IO,
    RUM_EQUEUE_PUSH,
    RUM_EQUEUE_MSG_DROP,
    RUM_ECONTEXT,
    RUM_EMOD_LIMIT,
    RUM_EMOD_START,
    RUM_EMOD_IFACE,
    RUM_EMOD_INCOMPATIBLE,
    RUM_EMOD_NOT_STARTED,
    RUM_EMOD_INVALID,
    RUM_EMOD_CONFLICT,
    RUM_EMOD_NEW,
    RUM_EMOD_PARAM_DESC,
    RUM_EMOD_PARAM,
    RUM_EMOD_SET_PARAM,
    RUM_EMOD_SUBTHREAD,
    RUM_ELOG_SOURCE_ADD,
    RUM_ELOG_NO_SOURCE,
    RUM_ELOG_REGISTER,
    RUM_ELISTENER_INIT,
    RUM_ELISTEN_PARAMS,
    RUM_ERAP_PARSE,
    RUM_ERAP_HEADER,
    RUM_ERAP_CONTENT_PARSE,
    RUM_ERAP_REQUEST,
    RUM_ERAP_REQUEST_COPY,
    RUM_ERAP_RESPONSE,
    RUM_EMSGIFACE_INIT,
    RUM_EMSGIFACE_PARAMS,
    RUM_EMSGIFACE_PARSER,
    RUM_EMSGIFACE_LOG_SRC,
    RUM_ECONFIG_INIT,
    RUM_ECONFIG_READ,
    RUM_ECONFIG_WRITE,
    RUM_ESYNC_RIR,
    RUM_ESENDER_INIT,
    RUM_ESESSION_INIT,
    RUM_ESESSION_NEW,
    RUM_EDATA_COPY,
    RUM_ETRIE_INIT,
    RUM_ETRIE_INSERT,
    RUM_EPROC_INIT,
    RUM_EPROC_PARAMS,
    RUM_EPROC_MANY,
    RUM_EPROC_PUSH,
    RUM_EPROC_PROCESS,
    RUM_EROUTE_INIT,
    RUM_EROUTE_PROC
};


/** Error context structure. */
struct rum_error_ctx {
    /** Error number stack. */
    enum rum_error errnos[RUM_ERRCTX_MAX];
    /** Number of errors in error stack. */
    int count;
};


/** Error number to error string convertor.
 *
 * @param err
 *      error number.
 *
 * @return
 *      error string.
 */
#define rum_errstr(err) (rum_error_strings[err])


/** Error string array indexed by error number. */
extern char *rum_error_strings[];


/** Error context macro.
 * This is for function declaration to make them easy:<br>
 *      <tt>type function(EC, type1 arg1, ...);</tt>
 */
#define EC  struct rum_error_ctx *errctx

/** An 'errctx' shortcut.
 * Useful for function calls:<br>
 *      <tt>function(E, arg1, ...);</tt>
 */
#define E errctx


/** Run rum_error() and return with retval.
 *
 * @param err
 *      error number to be passed to rum_error().
 *
 * @param retval
 *      return value.
 *
 * @return
 *      nothing.
 */
#define ERROR(err, retval)          \
    {                               \
        rum_error(errctx, err);     \
        return retval;              \
    }


/** Run rum_error_push() and return with retval.
 *
 * @param err
 *      error number to be passed to rum_error_push().
 *
 * @param retval
 *      return value.
 *
 * @return
 *      nothing.
 */
#define ERROR_P(err, retval)            \
    {                                   \
        rum_error_push(errctx, err);    \
        return retval;                  \
    }


/** Initialize error context.
 *
 * @return
 *      error context on success, NULL otherwise.
 */
extern struct rum_error_ctx *rum_error_init(void);


/** Free all the memory accopied by an error context.
 *
 * @param errctx
 *      error context.
 *
 * @return
 *      nothing.
 */
extern void rum_error_free(struct rum_error_ctx *errctx);


/** Reset error context with a new error.
 *
 * @param errctx
 *      error context.
 *
 * @param err
 *      error number. Use @c RUM_ENO_ERROR to reset the error context.
 *
 * @return
 *      nothing.
 */
extern void rum_error(struct rum_error_ctx *errctx, enum rum_error err);


/** Reset error context. */
#define rum_error_reset(errctx)     rum_error((errctx), RUM_ENO_ERROR)


/** Push error number to error context.
 *
 * @param errctx
 *      error context.
 *
 * @param err
 *      error number.
 *
 * @return
 *      nothing.
 */
extern void rum_error_push(struct rum_error_ctx *errctx, enum rum_error err);


/** Pop error number from error context.
 *
 * @param errctx
 *      error context.
 *
 * @return
 *      error number;
 *      @c RUM_ENO_ERROR when there is no error left in the context.
 */
extern enum rum_error rum_error_pop(struct rum_error_ctx *errctx);


/** Return the last error number (without popping it from error context).
 *
 * @param errctx
 *      error context.
 *
 * @return
 *      error number.
 */
#define rum_error_last(errctx)                      \
    (((errctx) == NULL || (errctx)->count <= 0)     \
        ? RUM_ENO_ERROR                             \
        : (errctx)->errnos[(errctx)->count - 1])

#endif

