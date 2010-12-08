/* $Id: rap-types.h,v 1.18 2005/04/06 09:10:01 jirka Exp $ */
/*
 * Reflector Administration Protocol data types.
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
 * Reflector Administration Protocol data types.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.18 $ $Date: 2005/04/06 09:10:01 $
 */

#ifndef RAP_TYPES_H
#define RAP_TYPES_H

#if HAVE_STDARG_H
# include <stdarg.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include "error.h"
#include "module.h"
#include "utils.h"


/** Unknown content length. */
#define RAP_CONTLEN_UNKNOWN -1
/** Undefined content length. */
#define RAP_CONTLEN_UNDEFINED -2


/** New line indication for RAP responses (CR LF). */
#define NL  "\r\n"


/** Format string for printing RAP parser ID. */
#define FMT_PARSER_ID   "%08lx"

/** Format string for printing RAP request ID (including parser ID). */
#define FMT_REQUEST_ID  FMT_PARSER_ID ",%ld"


/** RAP module list.
 * This structure can be safely typecasted to (struct module_id). */
struct rap_module_list {
    struct module_id module;
    struct rap_module_list *next;
};


/** RAP methods enumeration. */
enum rap_method_type {
    MT_ACL,
    MT_AVAIL,
    MT_CLIENTS,
    MT_KEEP_ALIVE,
    MT_LIST,
    MT_LOG,
    MT_LOGIN,
    MT_LOGOUT,
    MT_PASS,
    MT_PROCESS,
    MT_RESTART,
    MT_SESSION,
    MT_START,
    MT_STAT,
    MT_STOP,
    MT_USER,
    MT_USERPASS,
    MT_EXTENSION
};


/** RAP headings enumeration.
 * Since values of Content-Encoding, Content-Type, and Target headers are
 * stored directly in request structure they will never be inserted into
 * header list. */
enum rap_header_type {
    H_ACCESS,
    H_ACTION,
    H_ADDRESS,
    H_BREAK,
    H_CONTENT_ENCODING,
    H_CONTENT_LENGTH,
    H_EMAIL,
    H_FROM,
    H_ID,
    H_IFACE,
    H_INSTITUTION,
    H_KILL,
    H_LEVEL,
    H_LISTENER,
    H_LOGGING,
    H_METHOD,
    H_MOBILE,
    H_MODULE,
    H_NAME,
    H_NEW_PASSWORD,
    H_OLD_PASSWORD,
    H_PASSWORD,
    H_PHONE,
    H_PROCESSOR,
    H_REMOVE,
    H_SYNC,
    H_TARGET,
    H_TIMEOUT,
    H_TO,
    H_USER,
    H_USER_ADDRESS,
    H_EXTENSION
};


/** RAP method description structure. */
struct rap_method {
    /** Method type. */
    enum rap_method_type type;
    /** Method name in case rap_method::type == rap_method_type::MT_EXTENSION. */
    char *extension;
};


/** @defgroup raptypes RAP header types.
 *
 * Data types for RAP headers.
 *
 * @{
 */

/** Access header type. */
enum raphdr_access {
    ACCESS_RW,
    ACCESS_RO,
    ACCESS_WO,
    ACCESS_NONE
};

/** Action header type. */
enum raphdr_action {
    ACTION_LIST,
    ACTION_ADD,
    ACTION_EDIT,
    ACTION_REMOVE
};

/** Address, From, and To headers type. */
struct raphdr_address {
    /** IPv4 or IPv6 address. */
    IN_ADDR addr;
    /** Network bits. */
    int mask;
};


/** Content-Encoding header type. */
enum raphdr_cont_enc {
    ENCODING_UTF8,
    ENCODING_USASCII,
    ENCODING_BASE64,
    ENCODING_8BIT,
    /** No content at all. */
    ENCODING_NONE
};

/** Kill and Break header type. */
enum raphdr_yesno {
    YN_YES,
    YN_NO
};

/** Logging header type. */
enum raphdr_logging {
    LOGGING_START,
    LOGGING_STOP
};

/** Sync header type. */
enum raphdr_sync {
    SYNC_ON,
    SYNC_OFF
};

/** @} */


/** RAP header value types. */
union rap_header_value {
    enum raphdr_access access;
    enum raphdr_action action;
    struct raphdr_address address;
    enum raphdr_cont_enc encoding;
    enum raphdr_logging logging;
    struct rap_method method;
    struct module_id module;
    struct rap_module_list module_list;
    long number;
    char *string;
    enum raphdr_sync sync;
    enum raphdr_yesno yesno;
};


/** RAP heading description structure. */
struct rap_heading {
    /** Header type. */
    enum rap_header_type type;
    /** Heading in case rap_heading::type == rap_header_type::H_EXTENSION. */
    char *extension;
};


/** RAP header description structure. */
struct rap_header {
    /** Pointer to the next header in the list. */
    struct rap_header *next;
    /** Heading. */
    struct rap_heading heading;
    /** Header value. */
    union rap_header_value value;
};


/** RAP request content structure. */
struct rap_content {
    /** Content-Length. */
    long length;
    /** Content-Encoding. */
    enum raphdr_cont_enc encoding;
    /** Message body. */
    void *data;
};


/** RAP message type. */
enum rap_message_type {
    RAP_REQUEST,
    RAP_RESPONSE
};


/** RAP message (request/response) description structure.
 * Both rap_request and rap_response structures can be typecasted to
 * rap_message structure.
 */
struct rap_message {
    /** Message type -- either request or response. */
    enum rap_message_type type;
    /** Request ID.
     * Can be zero for out-of-band response (e.g., log messages). */
    unsigned long req_id;
    /** Identifier of msg-interface the request was received by. */
    struct module_id iface;
    /** Identifier of the connection the request belongs to. */
    unsigned int connection;
    /** Message header list (excluding Content-* and those contained directly
     * in rap_request or rap_response). */
    struct rap_header *headers;
    /** Message content. */
    struct rap_content content;
};


/** RAP request description structure. */
struct rap_request {
    /** RAP message description. */
    struct rap_message message;
    /** Requested method. */
    struct rap_method method;
    /** Target header. */
    struct module_id target;
    /** Sync header. */
    enum raphdr_sync sync;
};


/** RAP response codes. */
enum rap_response_code {
    RC_INFO_MESSAGE             = 100,
    RC_LOG_MESSAGE              = 101,
    RC_OK                       = 200,
    RC_BAD_REQUEST              = 400,
    RC_UNAUTHORIZED             = 401,
    RC_FORBIDDEN                = 403,
    RC_NOT_FOUND                = 404,
    RC_CONFLICT                 = 407,
    RC_INTERNAL_SERVER_ERROR    = 500,
    RC_NOT_IMPLEMENTED          = 501
};


/** RAP response description structure. */
struct rap_response {
    /** RAP message description. */
    struct rap_message message;
    /** RAP response code. */
    enum rap_response_code code;
    /** Source header. */
    struct module_id source;
    /** Sync header of the request. */
    enum raphdr_sync sync;
};


/** Return a string describing RAP response code.
 *
 * @param code
 *      RAP response code.
 *
 * @return
 *      string representation of response code, or NULL when the code given
 *      is not valid.
 */
extern const char *rap_response_code_str(enum rap_response_code code);


/** Return string representation of RAP header type.
 *
 * @param hdr
 *      pointer to the RAP heading.
 *
 * @return
 *      the particular string or NULL when the header type given is not valid.
 */
extern const char *rap_header_str(const struct rap_heading *hdr);


/** Return string representation of RAP method.
 *
 * @param method
 *      RAP method to be converted to string.
 *
 * @return
 *      the particular string or NULL when the header type given is not valid.
 */
extern const char *rap_method_str(const struct rap_method *method);


/** Return string representation of RAP access mode.
 *
 * @param mode
 *      access mode.
 *
 * @return
 *      the particular string or NULL when mode is invalid.
 */
extern const char *rap_access_str(enum raphdr_access mode);


/** Increment reference counter to rap_message structure.
 *
 * @param message
 *      pointer to the message structure.
 *
 * @return
 *      nothing.
 */
extern void rap_message_ref(struct rap_message *message);


/** Free all the memory used by a message (either request or response).
 *
 * @param message
 *      pointer to the place the pointer to the message structure
 *      is stored.
 *
 * @return
 *      nothing.
 */
extern void rap_message_free(struct rap_message **message);


/** Free all the memory used by a message (usable as a queue_flush_cb).
 *
 * @param message
 *      void pointer to a message (either request or response) structure.
 *
 * @return
 *      nothing.
 */
extern void rap_message_freeq(void *message);


/** Initialize RAP request structure.
 *
 * @param request
 *      pointer to the place the pointer to the request structure
 *      is (or will be) stored.
 *
 * @param allocate
 *      nonzero if this function should first allocate memory for the request.
 *
 * @param id
 *      pointer to the request identifier. The value will be automatically
 *      incremented during request initialization.
 *
 * @param iface
 *      identifier of msg-interface the request was received by.
 *
 * @param connection
 *      identifier of the connection the request belongs to.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rap_request_init(EC, struct rap_request **request,
                            int allocate,
                            unsigned long *id,
                            struct module_id *iface,
                            unsigned int connection);


/** Create a deep copy of RAP request.
 * Field rap_request::target is not copied and the calling function must set
 * it or make a copy of it.
 *
 * @param req
 *      a request to be copied.
 *
 * @return
 *      pointer to the new request or NULL on error.
 */
extern struct rap_request *rap_request_copy(EC, struct rap_request *req);


/** Create and initialize new RAP response.
 * All the memory occupied by the response will be freed when it is no longer
 * needed. The function makes a copy of arguments when it is needed (namely
 * module names).
 *
 * @param request_id
 *      identifier of the request this response is responding to.
 *
 * @param code
 *      response code.
 *
 * @param msg_iface
 *      identifier of the msg-interface module the response will be sent to.
 *
 * @param connection
 *      identifier of a connection within the msg-interface module.
 *
 * @param source
 *      source module identifier.
 *
 * @param sync
 *      Sync header of the request this response is responding to.
 *
 * @return
 *      pointer to the new RAP response on success, NULL otherwise.
 */
extern struct rap_response *rap_response_init(EC,
                                unsigned long request_id,
                                enum rap_response_code code,
                                const struct module_id *msg_iface,
                                unsigned int connection,
                                const struct module_id *source,
                                enum raphdr_sync sync);


/** Function which rap_response_write() uses for sending response to the conn.
 *
 * @param response
 *      response the data to be sent belongs to. Request ID,
 *      msg-interface module, and connection identifier are taken from
 *      response description structure.
 *
 * @param buf
 *      data to be written.
 *
 * @param count
 *      number of bytes to be written from buf.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
typedef int (*fn_write)(const struct rap_response *response,
                        const void *buf,
                        int count);


/** Write response to connection.
 * The response is written to the connection identified within the response
 * itself.
 *
 * @param response
 *      pointer to the response being sent. The response is neither freed
 *      nor modified.
 *
 * @param data_write
 *      function which is used for writing data to a connection.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rap_response_write(EC, const struct rap_response *response,
                              fn_write data_write);


/** Set message content.
 * The response must have NO content prior to calling this function,
 * otherwise the new content will be silently ignored. All the
 * memory occupied by the content will be freed when it is no longer needed
 * (see also rap_response_init()).
 *
 * @param content
 *      pointer to the message content structure.
 *
 * @param encoding
 *      content encoding.
 *
 * @param length
 *      length of response content.
 *
 * @param data
 *      pointer to the content.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rap_content_set(EC, struct rap_content *content,
                           enum raphdr_cont_enc encoding,
                           long length,
                           void *data);


/** Add a line to the message content.
 * In case content encoding is not set yet (i.e. it is set to ENCODING_NONE)
 * this function also sets it to ENCODING_USASCII value.
 *
 * @param content
 *      pointer to the message content.
 *
 * @param line
 *      format string of the line to be added to the
 *      content. The function will make a copy of this line and thus it can
 *      be destroyed after the function returns. <br>
 *      No new line character is appended to the line. Thus the line can
 *      actually be only a part of line or even more than one line. <br>
 *      When it is desired to mark end of line, NL macro should be used.
 *
 * @param ap
 *      arguments for the format string.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rap_content_vline(EC, struct rap_content *content,
                             const char *line,
                             va_list ap);


/** Add a line to the message content.
 * This function is actually just a wrapper to rap_content_vline().
 *
 * @param content
 *      pointer to the content.
 *
 * @param line, ...
 *      format string (and its arguments) of the line to be added to the
 *      content. The function will make a copy of this line and thus it can
 *      be destroyed after the function returns. <br>
 *      No new line character is appended to the line. Thus the line can
 *      actually be only a part of line or even more than one line. <br>
 *      When it is desired to mark end of line, NL macro should be used.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rap_content_line(EC, struct rap_content *content,
                            const char *line, ...);


/** Remove all headers referenced by RAP message.
 *
 * @param message
 *      pointer to the RAP message structure.
 *
 * @return
 *      nothing.
 */
extern void rap_message_remove_all_headers(struct rap_message *message);


/** Copy all the headers from source message to target message.
 * This function performs a deep copy of all headers. Order of headers in the
 * target message may differ from the one in the source message.
 *
 * @param dst
 *      target RAP message.
 *
 * @param src
 *      source message.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rap_message_copy_headers(EC, struct rap_message *dst,
                                    struct rap_message *src);


/** Add a header and its value to the message.
 * The header will be appended to the end of header list.
 *
 * This function does NOT perform deep copy. The caller must not free any
 * memory pointed to by value or heading structures. To be more sure about
 * this on success both heading and value structures will be memset() to
 * '\\0'.
 *
 * @param message
 *      the message the header will be added to.
 *
 * @param heading
 *      heading of the header.
 *
 * @param value
 *      value of the header.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rap_message_add_header(EC, struct rap_message *message,
                                  struct rap_heading *heading,
                                  union rap_header_value *value);


/** Free all the memory referenced from the specified header.
 * The memory occupied by the header structure itself is NOT freed!
 *
 * @param header
 *      pointer to the header structure.
 *
 * @return
 *      nothing.
 */
extern void rap_header_free(struct rap_header *header);


/** Add module to module list header.
 * The module will be appended to the end of module list.
 *
 * This function does NOT perform deep copy. The caller must not free any
 * memory pointed to by module identifier structure (i.e. name of the module).
 * To be more sure about this on success module_id structure will be memset()
 * to '\\0'.
 *
 * @param value
 *      pointer to the header value the module will be added to.
 *
 * @param module_id
 *      identifier of the module to be added.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int rap_header_add_module(EC, union rap_header_value *value,
                                 const struct module_id *module_id);


/** Print header in the form: 'Heading: value\\n'.
 *
 * @param header
 *      header to be printed.
 *
 * @param buf
 *      buffer where to store string representation of the header.
 *
 * @param size
 *      maximum number of characters (including trailing \\0) that can be
 *      printed to the buffer buf.
 *
 * @return
 *      number of characters printed or zero on error.
 */
extern int rap_header_print(const struct rap_header *header,
                            char *buf,
                            int size);


/** Log error message preceded by request identifier.
 *
 * @param mod
 *      pointer to a module description structure.
 *
 * @param msg
 *      RAP message (for connection and request identifier).
 *
 * @return
 *      nothing.
 */
extern void logerror_req(struct module *mod, const struct rap_message *msg);


/* High-level response generation functions. */

/** Send and log internal server error response.
 *
 * @param mod
 *      pointer to a module description structure.
 *
 * @param req
 *      a request which was being processed when an error occured.
 *
 * @return
 *      nothing.
 */
extern void response_error(struct module *mod, const struct rap_request *req);


/** Send request to the appropriate msg-interface and handle errors.
 *
 * @param mod
 *      pointer to a module description structure.
 *
 * @param resp
 *      pointer to the place where the pointer to the response is stored.
 *      The pointer to the response is set to NULL before this function
 *      returns.
 *
 * @return
 *      nothing.
 */
extern void response_send(struct module *mod, struct rap_response **resp);


/** Generate, log, and send response to the given request.
 *
 * @param code
 *      code of the generated response.
 *
 * @param mod
 *      pointer to a module description structure.
 *
 * @param req
 *      original request.
 *
 * @param msg, ...
 *      format string (and its arguments) that will be sent as a content of
 *      the generated response. New line mark (macro NL) is automatically
 *      appended at the and of msg. If msg is NULL no content will be sent.
 *
 * @return
 *      nothing.
 */
extern void response(enum rap_response_code code,
                     struct module *mod,
                     const struct rap_request *req,
                     char *msg, ...);


#endif

