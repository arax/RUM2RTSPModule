/* $Id: limits.h,v 1.10 2005/12/10 10:06:34 jirka Exp $ */
/*
 * Compile time limits for the reflector.
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
 * Compile time limits for the reflector.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.10 $ $Date: 2005/12/10 10:06:34 $
 */

#ifndef LIMITS_H
#define LIMITS_H

#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif


/** Length of IP address in bits. */
#if USE_IP6
# define ADDR_BITS 128
#else
# define ADDR_BITS 32
#endif

/** Length of string representation of IPv[46] address. */
#if USE_IP6
# define ADDRSTR_LEN INET6_ADDRSTRLEN
#else
# define ADDRSTR_LEN INET_ADDRSTRLEN
#endif


/** Number of minutes of history for load computing. */
#define RUM_LOAD_MAX        15

/** The largest identifier of reflector's module.
 * Module number forms the most significant bits in globally unique parser
 * identifier (32b unsigned integer).
 */
#define RUM_MODULES_MAX     0xFF

/** Number of bits reserved for RAP parser in globally uniq parser identifier.
 * A globally unique identifier of any RAP parser is represented as
 * <tt>(<i>module number</i> &amp; RUM_MODULES_MAX) &lt;&lt; RUM_PARSER_BITS
 * | (<i>parser number</i> &amp; RUM_PARSER_MAX)</tt>
 */
#define RUM_PARSER_BITS     24

/** The largest identifier of RAP parser. */
#define RUM_PARSER_MAX      ((1 << RUM_PARSER_BITS) - 1)

/** Maximum size of thread's stack. */
#define RUM_STACK_SIZE      (1 << 19)

/** Default UDP memory block size = 2^11B = 2048B. */
#define RUM_UDP_SIZE        11

/** Default TCP memory block size = 2^14B = 16384B. */
#define RUM_TCP_SIZE        14

/** Maximum number of processors in path. */
#define RUM_PATH_LEN        5

/** Maximum number of errors in stack. 
 * This is usually not greater than the depth of function calls. */
#define RUM_ERRCTX_MAX      20

/** Number of bits per trie level. */
#define RUM_TRIE_BITS       4

/** Size of the listener_data::buffs array. */
#define RUM_BUFF_COUNT      50

/** Maximum length of a single log message. */
#define RUM_LOG_LEN         2048

/** Maximum number of processors of the same type.
 * Must correspond to a value of RUM_PROC_MAX_LEN.
 */
#define RUM_PROC_MAX        99

/** Maximum length (in characters) of processor number.
 * Must correspond to a value of RUM_PROC_MAX.
 */
#define RUM_PROC_MAX_LEN    2

/** Memory management thread sleep time (microseconds).
 * How long will the memory management thread will sleep before starting new
 * periodical (de)allocation cycle.
 */
#define RUM_MEM_SLEEP       500

/** Maximum memory block size (base-2 logarithm).
 * 2^23 = 8MiB */
#define RUM_MEM_SIZE        23

/** Default maximum length of data queue. */
#define RUM_QUEUE_LEN       100

/** Maximum size of queue description (including '\\0'). */
#define RUM_QUEUE_DESC      70

/** Size of buffer for writing responses.
 * Should be set to at least RUM_LOG_LEN + header (unknown size ;-) ). */
#define RUM_RESP_BUF        (RUM_LOG_LEN + 200)

/** Maximum length of name of listener module.
 * Protocol (max. "mcast") + dash + mcast IP + ":" + port + '\0'. */
#define RUM_LISTENER_LEN    (5 + 1 + ADDRSTR_LEN + 1 + 5 + 1)

#endif

