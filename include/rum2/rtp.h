/* $Id: rtp.h,v 1.6 2006/03/11 15:37:00 jirka Exp $ */
/*
 * RTP/RTCP processing.
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
 * RTP/RTCP (RFC 3550) processing.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.6 $ $Date: 2006/03/11 15:37:00 $
 */

#ifndef RTP_H
#define RTP_H

#if HAVE_STDINT_H
# include <stdint.h>
#endif

/** @cond */
#ifndef HAVE_ATTRIBUTE_PACKED
# define __attribute__(x)   /* packed */
#endif
/** @endcond */

/** RTP protocol version. */
#define RTP_VERSION 2


/** RTP header. */
#if HAVE_PRAGMA_PACK
#pragma pack(1)
#endif
struct rtp_header {
#if WORDS_BIGENDIAN
    unsigned int version:2;
    unsigned int padding:1;
    unsigned int extension:1;
    unsigned int csrc_count:4;
    unsigned int marker:1;
    unsigned int payload_type:7;
    unsigned int seqno:16;
#else
    unsigned int seqno:16;
    unsigned int payload_type:7;
    unsigned int marker:1;
    unsigned int csrc_count:4;
    unsigned int extension:1;
    unsigned int padding:1;
    unsigned int version:2;
#endif
    uint32_t timestamp;
    uint32_t ssrc;
    uint32_t csrc;
} /** @cond */ __attribute__((packed)) /** @endcond */;
#if HAVE_PRAGMA_PACK
#pragma pack()
#endif


/** RTP header extension. */
#if HAVE_PRAGMA_PACK
#pragma pack(1)
#endif
struct rtp_header_ext {
#if WORDS_BIGENDIAN
    unsigned int profile_defined:16;
    unsigned int length:16;
#else
    unsigned int length:16;
    unsigned int profile_defined:16;
#endif
} /** @cond */ __attribute__((packed)) /** @endcond */;
#if HAVE_PRAGMA_PACK
#pragma pack()
#endif


/** RTCP packet types. */
enum rtcp_type {
    RTCP_SR     = 200,
    RTCP_RR     = 201,
    RTCP_SDES   = 202,
    RTCP_BYE    = 203,
    RTCP_APP    = 204
};


/** RTCP header. */
#if HAVE_PRAGMA_PACK
#pragma pack(1)
#endif
struct rtcp_header {
#if WORDS_BIGENDIAN
    unsigned int version:2;
    unsigned int padding:1;
    unsigned int count:5;
    unsigned int packet_type:8;
    unsigned int length:16;
#else
    unsigned int length:16;
    unsigned int packet_type:8;
    unsigned int count:5;
    unsigned int padding:1;
    unsigned int version:2;
#endif
} /** @cond */ __attribute__((packed)) /** @endcond */;
#if HAVE_PRAGMA_PACK
#pragma pack()
#endif


/** Cursor for traversing and creating RTCP packet. */
struct rtcp_cursor {
    /** Header of RTCP packet. */
    struct rtcp_header header;
    /** Pointer to the beginning of RTCP packet. */
    unsigned char *start;
    /** Current position in RTCP packet. */
    unsigned char *pos;
    /** Number of bytes in the RTCP packet from pos to its end. */
    unsigned int rest;
    /** Length of RTCP packet in bytes (including RTCP header). */
    unsigned int length;
    /** Data depending on the type of RTCP packet. */
    union {
        /** Data for SDES RTCP packet. */
        struct {
            /** Number of chunks. */
            int count;
            /** Current chunk. */
            int chunk;
            /** Nonzero when the @c pos points to an item; otherwise it points
             * to a SSRC/CSRC field. */
            int item;
        } sdes;
        /** Data for RR RTCP packet. */
        struct {
            /** Nonzero when the @c pos points to a sender's SSRC. */
            int sender;
        } rr;
    } type;
};


/** Types of source description RCTP packets. */
enum rtcp_sdes_type {
    RTCP_SDES_END   = 0,
    RTCP_SDES_CNAME = 1,
    RTCP_SDES_NAME  = 2,
    RTCP_SDES_EMAIL = 3,
    RTCP_SDES_PHONE = 4,
    RTCP_SDES_LOC   = 5,
    RTCP_SDES_TOOL  = 6,
    RTCP_SDES_NOTE  = 7,
    RTCP_SDES_PRIV  = 8
};


/** RTCP SDES item. */
#if HAVE_PRAGMA_PACK
#pragma pack(1)
#endif
struct rtcp_sdes_item {
    uint8_t type;
    uint8_t length;
    uint8_t data[255];
} /** @cond */ __attribute__((packed)) /** @endcond */;
#if HAVE_PRAGMA_PACK
#pragma pack()
#endif


/** Generate RTP session source identifier.
 *
 * @return
 *      generated SSRC.
 */
extern uint32_t rtp_ssrc(void);


/** Get canonical host name.
 * Caller is responsible for free()ing the memory pointed to by return value.
 *
 * @return
 *      canonical hostname on success, NULL on error.
 */
extern char *rtp_hostname(void);


/** Generate RTP CNAME.
 * The generated CNAME has the following form: name [ "-" mod ] "@" hostname.
 *
 * @param name
 *      user name (must NOT be NULL).
 *
 * @param mod
 *      string which is placed between user name and host name. It can also
 *      be NULL; in that case even '-' is not placed after name.
 *
 * @return
 *      generated CNAME.
 */
extern char *rtp_cname(const char *name, const char *mod);


/** Get RTP header of data packet.
 *
 * @param data
 *      data buffer.
 *
 * @param size
 *      size of the buffer.
 *
 * @param header
 *      where to store the content of the header.
 *
 * @return
 *      zero on success, nonzero when given data packet is not RTP packet.
 */
extern int rtp_get_header(void *data, int size, struct rtp_header *header);


/** Set RTP header of data packet.
 * Main purpose of this function is to save changes made in RTP header
 * obtained by rtp_get_header().
 *
 * @param data
 *      data buffer.
 *
 * @param size
 *      size of the buffer.
 *
 * @param header
 *      RTP header to be stored at the beginning of data packet.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rtp_set_header(void *data,
                          int size,
                          const struct rtp_header *header);


/** Get RTP payload of data packet.
 *
 * @param data
 *      data buffer.
 *
 * @param size
 *      size of the buffer.
 *
 * @param len
 *      where to store the length of RTP payload.
 *
 * @return
 *      pointer to the beginning of RTP payload, NULL when given data packet
 *      is not RTP packet.
 */
extern void *rtp_get_payload(void *data, int size, int *len);


/** Get RTCP header.
 *
 * @param data
 *      data buffer.
 *
 * @param size
 *      size of the buffer.
 *
 * @param header
 *      where to store the content of RTCP header.
 *
 * @param cursor
 *      cursor for traversing RTCP packet.
 *
 * @return
 *      NULL on failure or at the end of a compound packet. On success pointer
 *      to the next RTCP header in the compound packet is returned.
 */
extern void *rtcp_get_header(void *data,
                             int size,
                             struct rtcp_header *header,
                             struct rtcp_cursor *cursor);


/** Get SSRC/CSRC identifier of SDES RTCP packet.
 *
 * @param cursor
 *      pointer to a cursor initialized by rtcp_get_header().
 *
 * @param src
 *      where to place the identifier.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rtcp_sdes_get_src(struct rtcp_cursor *cursor, uint32_t *src);


/** Get next SDES item of a current chunk in RTCP packet.
 *
 * @param cursor
 *      pointer to a cursor initialized by rtcp_get_header().
 *
 * @param item
 *      where to place the item.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rtcp_sdes_get_item(struct rtcp_cursor *cursor,
                              struct rtcp_sdes_item *item);


/** Start a construction of a new RTCP packet.
 *
 * @param data
 *      buffer for the packet.
 *
 * @param size
 *      size of the buffer.
 *
 * @param type
 *      type of RTCP packet.
 *      @sa enum rtcp_type
 *
 * @param cursor
 *      cursor which will be used by other functions for constructing the
 *      while RTCP packet.
 *
 * @return
 *      zero on success, nonzero, otherwise.
 */
extern int rtcp_start(void *data,
                      int size,
                      int type,
                      struct rtcp_cursor *cursor);


/** Finish RTCP packet.
 *
 * @param cursor
 *      pointer to a cursor initialized by rtcp_start().
 *
 * @param length
 *      where to store length of the whole RTCP packet (in bytes).
 *
 * @return
 *      pointer to the first octet after the RTCP header or NULL on failure.
 */
extern void *rtcp_end(struct rtcp_cursor *cursor,
                      int *length);


/** Set sender's SSRC of RR RTCP packet.
 *
 * @param cursor
 *      pointer to a cursor initialized by rtcp_start().
 *
 * @param src
 *      SSRC of the sender.
 *
 * @return
 *      zero on success, nonzero, otherwise.
 */
extern int rtcp_rr_set_sender(struct rtcp_cursor *cursor, uint32_t src);


/** Start a new chunk of SDES RTCP packet for a given SSRC/CSRC identifier.
 *
 * @param cursor
 *      pointer to a cursor initialized by rtcp_start().
 *
 * @param src
 *      SSRC/CSRC identifier.
 *
 * @return
 *      zero on success, nonzero, otherwise.
 */
extern int rtcp_sdes_set_src(struct rtcp_cursor *cursor, uint32_t src);


/** Add an item into a current SDES chunk.
 *
 * @param cursor
 *      pointer to a cursor initialized by rtcp_start().
 *
 * @param type
 *      type of SDES item.
 *      @sa enum rtcp_sdes_type
 *
 * @param length
 *      length of item's data.
 *
 * @param data
 *      data for the item.
 *
 * @return
 *      zero on success, nonzero, otherwise.
 */
extern int rtcp_sdes_set_item(struct rtcp_cursor *cursor,
                              uint8_t type,
                              uint8_t length,
                              const void *data);

#endif

