/* $Id: utils.h,v 1.17 2006/12/12 21:17:15 jirka Exp $ */
/*
 * Useful functions (often wrapper functions) used by the reflector.
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
 * Useful functions (often wrapper functions) used by the reflector.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.17 $ $Date: 2006/12/12 21:17:15 $
 */

#ifndef UTILS_H
#define UTILS_H

#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if VA_COPY_MEMCPY
# if STDC_HEADERS
#  include <string.h>
# endif
# if HAVE_STDARG_H
#  include <stdarg.h>
# endif
#endif


/** Macro to shut up compiler warnings about unused function arguments. */
#define UNUSED(x)   ((x) = (x))


#if VA_COPY_MEMCPY
#define va_copy(dst, src)   memcpy(&dst, &src, sizeof(va_list))
#endif


#if USE_IP6
/** Data type for IPv6 address to 32-bit integer conversion. */
typedef uint32_t rum_in_addr6_32[4];
#endif

/** IPv[46] address to 32-bit integer convertor.
 * It can be used as in6_addr32(ip)[i] to get i-th 32-bit part of IPv6
 * address or just in_addr32(ip) in case of IPv4 address.
 *
 * @param ip
 *      pointer to the IP address (of type IN_ADDR).
 *
 * @return
 *      IP address (or its part) as uint32_t.
 */
#if USE_IP6
#define in6_addr32(ip)  (*(rum_in_addr6_32 *)(ip))
#else
#define in_addr32(ip)   (*(uint32_t *)(ip))
#endif


/** Type of IP address. */
#if USE_IP6
# define IN_ADDR    struct in6_addr
#else
# define IN_ADDR    struct in_addr
#endif

/** Type of socket address. */
#if USE_IP6
# define ADDR_TYPE   struct sockaddr_in6
#else
# define ADDR_TYPE   struct sockaddr_in
#endif

/** Name of sockaddr_in{,6} member which contains address family. */
#if USE_IP6
# define SIN_AF     sin6_family
#else
# define SIN_AF     sin_family
#endif

/** Name of sockaddr_in{,6} member which contains port number. */
#if USE_IP6
# define SIN_PORT   sin6_port
#else
# define SIN_PORT   sin_port
#endif

/** Name of sockaddr_in{,6} member which contains IPv[46] address structure. */
#if USE_IP6
# define SIN_ADDR   sin6_addr
#else
# define SIN_ADDR   sin_addr
#endif

/** Protocol family. */
#if USE_IP6
# define PF_INET46  PF_INET6
#else
# define PF_INET46  PF_INET
#endif

/** Address family. */
#if USE_IP6
# define AF_INET46  AF_INET6
#else
# define AF_INET46  AF_INET
#endif


/** Compare two IP addresses.
 *
 * @param ip1
 *      pointer to the first IP address (of type IN_ADDR).
 *
 * @param ip2
 *      pointer to the second IP address (of type IN_ADDR).
 *
 * @return
 *      an integer less than, equal to, or greater than zero if ip1 is found
 *      to be less than, to match, or be greater than ip2.
 */
#define ip_cmp(ip1,ip2)  \
            memcmp((void *)(ip1), (void *)(ip2), sizeof(IN_ADDR))


/** Compare two IP addresses with respect to mask.
 *
 * @param ip1
 *      pointer to the first IP address (of type IN_ADDR).
 *
 * @param ip2
 *      pointer to the second IP address (of type IN_ADDR).
 *
 * @return
 *      an integer less than, equal to, or greater than zero if ip1 is found
 *      to be less than, to match, or be greater than ip2.
 */
extern int ip_cmp_masked(const IN_ADDR *ip1, const IN_ADDR *ip2, int mask);


/** Get specified bits of 32-bit integer.
 *
 * @param i
 *      32-bit integer.
 *
 * @param pos
 *      from which bit (including) to start. The most significant bit is
 *      number 0 and the least significant bit is number 31.
 *
 * @param n
 *      number of bits to get (this must be the number of significant bits
 *      in mask).
 *
 * @param mask
 *      n-bit mask specifying the bits we are interested in. The most usual
 *      value is (2^n - 1).
 *
 * @return
 *      masked n bits of i starting at pos.
 */
#define masked_bits32(i, pos, n, mask)  \
            (((uint32_t)(i) >> (32 - (pos) - (n))) & (mask))


/** Get specified bits of IPv[46] address.
 * Bits to be returned must not overlap 32bit boundary. That is all the bits
 * must be either between 0 and 31 or 32 and 63 or 64 and 95 or 96 and 127.
 *
 * @param ip
 *      pointer to the IP address (of type IN_ADDR).
 *
 * @param pos
 *      from which bit (including) to start. The most significant bit is
 *      number 0 and the least significant bit is number {31,127}.
 *
 * @param n
 *      number of bits to get (this must be the number of significant bits
 *      in mask).
 *
 * @param mask
 *      n-bit mask specifying the bits we are interested in. The most usual
 *      value is (2^n - 1).
 *
 * @return
 *      masked n bits of IP address starting at pos.
 */
#if USE_IP6
# define ip_bits(ip, pos, n, mask)  \
            (masked_bits32(ntohl(in6_addr32(ip)[(pos) / 32]), \
                           (pos) % 32, (n), (mask)))
#else
# define ip_bits(ip, pos, n, mask)  \
            (masked_bits32(ntohl(in_addr32(ip)), (pos), (n), (mask)))
#endif


/** Network mask (prefix) to bit mask conversion array. */
extern uint32_t prefix32_mask[33];


/** Apply network mask (prefix) to IP address.
 *
 * @param ip
 *      32-bit integer specifying either IPv4 address or some of four 32-bit
 *      portions of IPv6 address.
 *
 * @param pfx
 *      network prefix, i.e. number of bits (taken from the most significant
 *      one) to be left untouched; the rest of bits are set to 0.
 *
 * @return
 *      ip changed accorindg to pfx.
 */
#define ip_mask32(ip, pfx)  \
            (htonl((uint32_t)(ntohl(ip)) & prefix32_mask[(pfx)]))


/** Return a string describing the system error code.
 * This function behaves almost the same way as strerror_r() does. The
 * exception is that it returns the appropriate error description string
 * instead of error code. In case of failure, it returns empty string (i.e.
 * not NULL). The function may, but need not, use the user-supplied buffer.
 *
 * @param errnum
 *      system error code.
 *
 * @param buf
 *      pointer to the memory where the string should be stored.
 *
 * @param n
 *      size of the supplied buffer.
 *
 * @return
 *      the error string; usually it is the same pointer as buf, but it can
 *      also be anything different.
 */
extern char *sys_error(int errnum, char *buf, size_t n);


/** Sleep for a specified number of microseconds.
 * This function does the same thing as usleep() but makes use of nanosleep().
 *
 * @param usec
 *      microseconds.
 *
 * @return
 *      nothing.
 */
extern void micro_sleep(long usec);


/** Trim all white-space characters from the beginning and the end of string.
 *
 * @param str
 *      source string.
 *
 * @return
 *      pointer to the result string contained in the source string.
 */
extern char *chop(char *str);


/** Write all data from buffer to filedescriptor.
 * This function differs from write() in number of bytes written; write() does
 * not ensure all data are written so it has to be called in loop.
 *
 * @param fd
 *      where to write.
 *
 * @param buf
 *      buffer with data to write.
 *
 * @param count
 *      number of bytes to be written.
 *
 * @return
 *      number of bytes written or -1 with error code in errno on error.
 */
extern ssize_t write_all(int fd, const void *buf, size_t count);

#endif

