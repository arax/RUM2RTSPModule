/* $Id: rap.h,v 1.7 2005/12/11 22:48:23 jirka Exp $ */
/*
 * RAP parser.
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
 * RAP parser.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.7 $ $Date: 2005/12/11 22:48:23 $
 */

#ifndef RAP_H
#define RAP_H

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_PTHREAD_H
# include <pthread.h>
#endif

#include "module.h"


/** Function the parser uses for reading messages.
 *
 * @param fd
 *      file descriptor (or whatever) which determines the connection
 *      this function should read from.
 *
 * @param buf
 *      a buffer where to place characters.
 *
 * @param max_size
 *      maximum number of characters that can be written to the buffer.
 *
 * @return
 *      number of characters read or 0 to indicate end of input.
 */
typedef int (*fn_read)(int fd, void *buf, size_t max_size);


/** Start RAP parser either directly or as a separate thread.
 *
 * @param thid
 *      where to store POSIX thread identifier od the parser. If NULL is
 *      passed on as thid argument, this function waits for parser's thread
 *      to finish and then returns.
 *
 * @param msg_iface
 *      msg-interface module identifier the parser belongs to.
 *
 * @param parser_id
 *      globally unique parser identifier.
 *
 * @param fd
 *      file descriptor (or whatever) which determines the connection the
 *      parser will read data from.
 *
 * @param data_read
 *      function which will be used for reading data from fd.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int start_parser(EC, pthread_t *thid,
                        const struct module *msg_iface,
                        uint32_t parser_id,
                        int fd,
                        fn_read data_read);


/** Generate globally unique identifier for RAP parser.
 * This macro increments parser number as a side effect.
 *
 * @param module
 *      module number.
 *
 * @param parser
 *      parser number.
 *
 * @return
 *      globally unique RAP parser identifier.
 */
#define parser_GUID(module, parser) \
    (((parser) = ((parser) + 1) & RUM_PARSER_MAX),          \
     (uint32_t) (((module) << RUM_PARSER_BITS)              \
                 | (((parser) - 1) & RUM_PARSER_MAX)))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

