/* -*- c -*- */
/* *
 * This file is part of Feng
 *
 * Copyright (C) 2009 by LScube team <team@lscube.org>
 * See AUTHORS for more details
 *
 * feng is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * feng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with feng; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * */

#include <glib.h>
#include <stdio.h>
#include <ev.h>

/** Type of socket address. */
#if USE_IP6
# define ADDR_TYPE   struct sockaddr_in6
#else
# define ADDR_TYPE   struct sockaddr_in
#endif

/** RTSP method types */
enum RTSP_method_token {
  RTSP_ID_ERROR,
  RTSP_ID_DESCRIBE,
  RTSP_ID_ANNOUNCE,
  RTSP_ID_GET_PARAMETERS,
  RTSP_ID_OPTIONS,
  RTSP_ID_PAUSE,
  RTSP_ID_PLAY,
  RTSP_ID_RECORD,
  RTSP_ID_REDIRECT,
  RTSP_ID_SETUP,
  RTSP_ID_STOP,
  RTSP_ID_TEARDOWN
};

/** RTSP request structure - client info, method, headers */
typedef struct RTSP_Request{
    /**
     * @brief String representing the method used
     *
     * Mostly used for logging purposes.
     */
    char *method;
    /**
     * @brief Machine-readable ID of the method
     *
     * Used by the state machine to choose the callback method.
     */
    enum RTSP_method_token method_id;

    /**
     * @brief Object of the request
     *
     * Represents the object to work on, usually the URL for the
     * request (either a resource URL or a track URL). It can be "*"
     * for methods like OPTIONS.
     */
    char *object;

    /**
     * @brief Protocol version used
     *
     * This can only be RTSP/1.0 right now. We log it here for access.log and to
     * remove more logic from the parser itself.
     *
     * @todo This could be HTTP/1.0 or 1.1 when requests come from
     *       QuickTime's proxy-passthrough. Currently that is not the
     *       case though.
     */
    char *version;

    /**
     * @brief All the headers of the request, unparsed.
     *
     * This hash table contains all the headers of the request, in
     * unparsed string form; they can used for debugging purposes or
     * simply to access the original value of an header for
     * pass-through copy.
     */
    GHashTable *headers;
} RTSP_Request;

/** Basic client information - address, port, session ID, server state */
typedef struct RTSP_Client{
    int socket;
    int msg_cseq;
    ADDR_TYPE *clientaddr;
    RTSP_Request *req;
    char *response;
    char *sessionID;
    gboolean stop;
    ev_io ev_write;
    ev_io ev_read;
    ev_timer timer;
} RTSP_Client;

%% machine rtsp_request_line;

size_t ragel_parse_request_line(const char *msg, const size_t length, RTSP_Request *req) {
    int cs;
    const char *p = msg, *pe = p + length + 1, *s = NULL, *eof = pe;

    /* We want to express clearly which versions we support, so that we
     * can return right away if an unsupported one is found.
     */

    %%{
        SP = ' ';
        CRLF = "\r\n";

        action set_s {
            s = p;
        }

        action end_method {
            req->method = g_strndup(s, p-s);
        }

        Supported_Method =
            "DESCRIBE" % { req->method_id = RTSP_ID_DESCRIBE; } |
            "OPTIONS" % { req->method_id = RTSP_ID_OPTIONS; } |
            "PAUSE" % { req->method_id = RTSP_ID_PAUSE; } |
            "PLAY" % { req->method_id = RTSP_ID_PLAY; } |
            "SETUP" % { req->method_id = RTSP_ID_SETUP; } |
            "STOP" % { req->method_id = RTSP_ID_TEARDOWN; } |
            "TEARDOWN" % { req->method_id = RTSP_ID_TEARDOWN; };

        Method = (Supported_Method | alpha+ )
            > set_s % end_method;

        action end_version {
            req->version = g_strndup(s, p-s);
        }

        Version = (alpha+ . '/' . [0-9] '.' [0-9]);

        action end_object {
            req->object = g_strndup(s, p-s);
        }

        Request_Line = (Supported_Method | Method) . SP
            (print*) > set_s % end_object . SP
            Version > set_s % end_version . CRLF;

        Header = (alpha|'-')+  . ':' . SP . print+ . CRLF;

        action stop_parser {
            return p-msg;
        }

        main := Request_Line % stop_parser . /.*/;

        write data noerror;
        write init;
        write exec;
    }%%

    if ( cs < rtsp_request_line_first_final )
        return 0;

    cs = rtsp_request_line_en_main; // Kill a warning

    return p-msg;
}
