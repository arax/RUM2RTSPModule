/*
 Module msg-interface/rtsp.

 This file is part of RUM2.

 RUM2 is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//////////////////////////////////////////////////////////////////////////////
/// \file
/// Module msg-interface/rtsp for RUM2.
/// \author arax
/// \date 2010
/// \version 0.99 2010/02/27
//////////////////////////////////////////////////////////////////////////////

#ifndef MSGIFACE_RTSP_H
#define MSGIFACE_RTSP_H

#include <rum2/module.h>
#include <glib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

//////////////////////////////////////////////////////////////////////////////
/// Default name of the module.
///
/// This name is just temporary (for init purposes).
//////////////////////////////////////////////////////////////////////////////
#define MSGIFACE_RTSP "rtsp"


#if STATIC_MSGIFACE_RTSP || STATIC
# define STATIC_MSGIFACE_RTSP_ITEM \
    { MSGIFACE_RTSP, msgiface_rtsp_initialize },
#else
//////////////////////////////////////////////////////////////////////////////
/// Static module description (MUST end with a comma).
//////////////////////////////////////////////////////////////////////////////
# define STATIC_MSGIFACE_RTSP_ITEM
#endif

#if STATIC_MSGIFACE_RTSP || STATIC
extern int msgiface_rtsp_initialize(struct module *module);
#else
//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
extern int initialize(struct module *module);
#endif

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static int m_name(struct module *module, int id);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static int m_init(struct module *module);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static void m_main(struct module *module);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static void m_clean(struct module *module, int for_restart);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static void m_stop(struct module *module);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static int m_config(struct module *module, const char *name, int start);

//////////////////////////////////////////////////////////////////////////////
/// Module interface structure.
//////////////////////////////////////////////////////////////////////////////
static struct module_interface iface = {
    MODULE_VERSION, ///< version
    m_name,         ///< name()
    NULL,           ///< conflicts()
    m_init,         ///< init()
    m_main,         ///< main()
    m_stop,         ///< stop()
    m_clean,        ///< clean()
    NULL,           ///< push_data()
    NULL,           ///< push_message()
    NULL,           ///< events()
    m_config        ///< config()
};

//////////////////////////////////////////////////////////////////////////////
/// Default input buffer size (max. size of received messages).
//////////////////////////////////////////////////////////////////////////////
#define MAX_MSG 1024

//////////////////////////////////////////////////////////////////////////////
/// Default output buffer size (max. size of sent messages).
//////////////////////////////////////////////////////////////////////////////
#define MAX_MSG_OUT 600

//////////////////////////////////////////////////////////////////////////////
/// Listening backlog (for TCP socket listener).
//////////////////////////////////////////////////////////////////////////////
#define LISTENQ 10

//////////////////////////////////////////////////////////////////////////////
/// Default timeout for read_ev events (MUST end with a comma).
//////////////////////////////////////////////////////////////////////////////
#define TIMEOUT 60.

//////////////////////////////////////////////////////////////////////////////
/// Types of response messages (a few fixed responses).
//////////////////////////////////////////////////////////////////////////////
typedef enum {
    OPTIONS_PUBLIC_OK,  ///< Response to a valid OPTIONS request.
    BAD_REQUEST,        ///< Response to invalid requests (CSeq, RTSP ver. etc.).
    NOT_IMPLEMENTED,    ///< Response to an unknown request (method).
    DESCRIBE_OK,        ///< Response to a valid DESCRIBE request (with SDP).
    SETUP_OK,           ///< Response to a valid SETUP request (with SessionID).
    SESSION_NOT_FOUND,  ///< Response to an unknown/invalid SessionID.
    PLAY_OK,            ///< Response to a valid PLAY request.
    STOP_OK,            ///< Response to a valid STOP request (== TEARDOWN_OK).
    TEARDOWN_OK         ///< Response to a valid TEARDOWN request (clean-up).
} RTSP_Response_Msg;

//////////////////////////////////////////////////////////////////////////////
/// Server states.
//////////////////////////////////////////////////////////////////////////////
typedef enum {
    RTSP_SERVER_INIT,   ///< Server is not ready yet, init is running.
    RTSP_SERVER_READY,  ///< Server is ready and will accept clients.
    RTSP_SERVER_HALT    ///< Server has been stopped, main loop ended.
} RTSP_Server_State;

//////////////////////////////////////////////////////////////////////////////
/// Types of RTSP methods (MUST be the same as in rtsp_ragel_request_line.rl).
/// Method and RTSP version are identified in \a ragel_parse_request_line(...).
//////////////////////////////////////////////////////////////////////////////
enum RTSP_method_token {
  RTSP_ID_ERROR,            ///< An ERROR message (not used).
  RTSP_ID_DESCRIBE,         ///< A DESCRIBE message (used).
  RTSP_ID_ANNOUNCE,         ///< An ANNOUNCE message (not used).
  RTSP_ID_GET_PARAMETERS,   ///< A GET_PARAMETERS message (not used).
  RTSP_ID_OPTIONS,          ///< An OPTIONS message (used).
  RTSP_ID_PAUSE,            ///< A PAUSE message (not used).
  RTSP_ID_PLAY,             ///< A PLAY message (used, MUST contain SessID).
  RTSP_ID_RECORD,           ///< A RECORD message (not used).
  RTSP_ID_REDIRECT,         ///< A REDIRECT message (not used).
  RTSP_ID_SETUP,            ///< A SETUP message (used)
  RTSP_ID_STOP,             ///< A STOP message (== RTSP_ID_TEARDOWN, for now)
  RTSP_ID_TEARDOWN          ///< A TEARDOWN message (used, MUST contain SessID.)
};

//////////////////////////////////////////////////////////////////////////////
/// Types of RAP msgs - only for adding and removing clients from sessions
//////////////////////////////////////////////////////////////////////////////
enum msg_type{
    RAP_CLIENTS_ADD,
    RAP_CLIENTS_REMOVE
};

//////////////////////////////////////////////////////////////////////////////
/// RTSP request structure - client info, method, headers
//////////////////////////////////////////////////////////////////////////////
typedef struct {
    //////////////////////////////////////////////////////////////////
    /// String representing the method used
    ///
    /// Mostly used for logging purposes.
    //////////////////////////////////////////////////////////////////
    char *method;

    //////////////////////////////////////////////////////////////////
    /// Machine-readable ID of the method
    ///
    /// Used by the state machine to choose the callback method.
    //////////////////////////////////////////////////////////////////
    enum RTSP_method_token method_id;

    //////////////////////////////////////////////////////////////////
    /// Object of the request
    ///
    /// Represents the object to work on, usually the URL for the
    /// request (either a resource URL or a track URL). It can be "*"
    /// for methods like OPTIONS.
    //////////////////////////////////////////////////////////////////
    char *object;

    //////////////////////////////////////////////////////////////////
    /// Protocol version used
    ///
    /// This can only be RTSP/1.0 right now.
    //////////////////////////////////////////////////////////////////
    char *version;

    //////////////////////////////////////////////////////////////////
    /// All the headers of the request, unparsed.
    ///
    /// This hash table contains all the headers of the request, in
    /// unparsed string form; they can used for debugging purposes or
    /// simply to access the original value of an header for
    /// pass-through copy.
    /////////////////////////////////////////////////////////////////
    GHashTable *headers;
}RTSP_Request;

//////////////////////////////////////////////////////////////////////////////
/// Channel parameter name.
//////////////////////////////////////////////////////////////////////////////
#define PARAM_PORT  "Port"

//////////////////////////////////////////////////////////////////////////////
/// Channel parameter description.
//////////////////////////////////////////////////////////////////////////////
#define PARAM_PORT_DESC "port number to listen on (defaults to 554)"

//////////////////////////////////////////////////////////////////////////////
/// Address parameter name.
//////////////////////////////////////////////////////////////////////////////
#define PARAM_BIND_ADDR  "Bind-Address"

//////////////////////////////////////////////////////////////////////////////
/// Address parameter description.
//////////////////////////////////////////////////////////////////////////////
#define PARAM_BIND_ADDR_DESC  "local address to listen on (defaults to 0.0.0.0)"

//////////////////////////////////////////////////////////////////////////////
/// Listener parameter name.
//////////////////////////////////////////////////////////////////////////////
#define PARAM_LISTENER_ID  "Listener"

//////////////////////////////////////////////////////////////////////////////
/// Listener parameter description.
//////////////////////////////////////////////////////////////////////////////
#define PARAM_LISTENER_ID_DESC "listener ID e.g. listener/udp-0.0.0.0:1234"

//////////////////////////////////////////////////////////////////////////////
/// UNIX socket parameter name.
//////////////////////////////////////////////////////////////////////////////
#define PARAM_UNIX_SOCKET "Socket"

//////////////////////////////////////////////////////////////////////////////
/// UNIX socket parameter description.
//////////////////////////////////////////////////////////////////////////////
#define PARAM_UNIX_SOCKET_DESC "UNIX socket (target for RAP msgs)"

//////////////////////////////////////////////////////////////////////////////
/// Module parameters.
//////////////////////////////////////////////////////////////////////////////
static struct module_param params[] = {
    { NULL, PARAM_PORT, PARAM_PORT_DESC, "554", NULL },
    { NULL, PARAM_BIND_ADDR, PARAM_BIND_ADDR_DESC, "0.0.0.0", NULL },
    { NULL, PARAM_LISTENER_ID, PARAM_LISTENER_ID_DESC, "listener/udp-0.0.0.0:1234", NULL },
    { NULL, PARAM_UNIX_SOCKET, PARAM_UNIX_SOCKET_DESC, "/tmp/reflector", NULL },
};

//////////////////////////////////////////////////////////////////////////////
/// Number of startup parameters.
//////////////////////////////////////////////////////////////////////////////
#define params_count (sizeof(params) / sizeof(struct module_param))

//////////////////////////////////////////////////////////////////////////////
/// Server structure - address, port, listening socket, number of clients etc.
//////////////////////////////////////////////////////////////////////////////
typedef struct {
    ADDR_TYPE *servaddr;            ///< Local address the server is listening on.
    int list_s;                     ///< Socket for incoming messages.
    GHashTable *client_list;        ///< List of active clients (sessID -> addr).
    RTSP_Server_State server_state; ///< Current state of the server.
    pthread_mutex_t srv_mutex;      ///< Structure mutex (mainly for client list).
    ev_io ev_accept;                ///< Watcher structure (waiting for READ event).
    struct ev_loop *loop;           ///< Main loop (runs until m_stop is called).
    int unixsocket_fd;              ///< Local UNIX socket (msg-interface) for RAP
    char *listener_id;
}RTSP_Server;

//////////////////////////////////////////////////////////////////////////////
/// Client structure - address, port, session ID, server state
//////////////////////////////////////////////////////////////////////////////
typedef struct {
    int socket;             ///< Socket for IN/OUT messages (after accept).
    int msg_cseq;           ///< Current CSeq value (ignored at the time).
    ADDR_TYPE *clientaddr;  ///< Remote address of the client.
    RTSP_Request *req;      ///< Structure for request data (headers and content).
    char *response;         ///< Pointer to response message (c_str).
    char *sessionID;        ///< Pointer to client sessionID (for PLAY, TEARDOWN).
    gboolean stop;          ///< Processing state (true == error and end).
    ev_io ev_write;         ///< Watcher structure (waiting for READ events).
    ev_io ev_read;          ///< Watcher structure (waiting for WRITE events).
    ev_timer timer;         ///< Timer structure (for READ timeout in \a TIMEOUT).
}RTSP_Client;

//////////////////////////////////////////////////////////////////////////////
/// Timeout structure - data needed for successfull timeout (and clean-up)
//////////////////////////////////////////////////////////////////////////////
typedef struct {
    struct module *module;  ///< Pointer to module structure (module data, log).
    RTSP_Client *client;    ///< Pointer to client structure (clean-up).
}timeout_data;

//////////////////////////////////////////////////////////////////////////////
/// Structure for RAP msg sender - contains IP in c_str and msg type,
///                                the rest is a template
//////////////////////////////////////////////////////////////////////////////
typedef struct{
    char *ip;               ///< client's IP address in c_str
    enum msg_type type;     ///< msg type {ADD, REMOVE}
}RAP_Msg_data;

//////////////////////////////////////////////////////////////////////////////
/// Preliminary request parser from Feng. This function will get protocol type,
/// protocol version and requested method from received message.
///
/// \param msg Message to be parsed (c_str).
/// \param length Length of the message passed on in \a msg.
/// \param req Target request structure (allocated).
/// \return Length of headers read (and parsed) from message.
//////////////////////////////////////////////////////////////////////////////
extern size_t ragel_parse_request_line(const char *msg,
                                       const size_t length,
                                       RTSP_Request *req);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static void process(struct ev_loop *loop,
                    struct ev_io *w,
                    int revents);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static int set_response(RTSP_Client *client,
                        RTSP_Response_Msg msg_type,
                        char *session_hdr);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static char *rtsp_timestamp();

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static char *gen_sess_id(RTSP_Client *client);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static void accept_connection(struct ev_loop *loop,
                              struct ev_io *w,
                              int revents);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static void timeout(struct ev_loop *loop,
                    struct ev_timer *timer,
                    int revents);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static void send_msg(struct ev_loop *loop,
                     struct ev_io *w,
                     int revents);

//////////////////////////////////////////////////////////////////////////////
/// \see rtsp.c
//////////////////////////////////////////////////////////////////////////////
static int send_rap_msg(struct module *module, RAP_Msg_data *data);

//////////////////////////////////////////////////////////////////////////////
/// Default RTSP_OK response header
//////////////////////////////////////////////////////////////////////////////
const char rtsp_ok[] = "RTSP/1.0 200 OK";

//////////////////////////////////////////////////////////////////////////////
/// Default new line characters
//////////////////////////////////////////////////////////////////////////////
const char msg_newline[] = "\r\n";

//////////////////////////////////////////////////////////////////////////////
/// Default characters indicating end of the message
//////////////////////////////////////////////////////////////////////////////
const char msg_end[] = "\r\n\r\n";

//////////////////////////////////////////////////////////////////////////////
/// Default CSeq word
//////////////////////////////////////////////////////////////////////////////
const char msg_cseq[] = "CSeq: ";

//////////////////////////////////////////////////////////////////////////////
/// Default Date word
//////////////////////////////////////////////////////////////////////////////
const char msg_date[] = "Date: ";

//////////////////////////////////////////////////////////////////////////////
/// Default Session word
//////////////////////////////////////////////////////////////////////////////
const char msg_sess[] = "Session: ";

//////////////////////////////////////////////////////////////////////////////
/// Default server timeout
//////////////////////////////////////////////////////////////////////////////
const char msg_timeout[] = ";timeout=60";

//////////////////////////////////////////////////////////////////////////////
/// Default transport info
//////////////////////////////////////////////////////////////////////////////
const char msg_transport[] =
"Transport: udp;"
"source=127.0.0.1;"
"server_port=1234;"
"client_port=1234";

//////////////////////////////////////////////////////////////////////////////
/// Default server info
//////////////////////////////////////////////////////////////////////////////
const char msg_server[] = "Server: RTSP module for RUM2";

//////////////////////////////////////////////////////////////////////////////
/// Default Bad Request response header
//////////////////////////////////////////////////////////////////////////////
const char rtsp_bad_request[] = "RTSP/1.0 400 Bad Request";

//////////////////////////////////////////////////////////////////////////////
/// Default Not Implemented response header
//////////////////////////////////////////////////////////////////////////////
const char rtsp_not_implemented[] = "RTSP/1.0 501 Not Implemented";

//////////////////////////////////////////////////////////////////////////////
/// Default Internal Server Error response header
//////////////////////////////////////////////////////////////////////////////
const char rtsp_internal_srv_error[] = "RTSP/1.0 500 Internal Server Error";

//////////////////////////////////////////////////////////////////////////////
/// Default Session Not Found response header
//////////////////////////////////////////////////////////////////////////////
const char rtsp_sess_not_found[] = "RTSP/1.0 454 Session Not Found";

//////////////////////////////////////////////////////////////////////////////
/// Default Options response -- Public
//////////////////////////////////////////////////////////////////////////////
const char options_public_ok[] = "Public: DESCRIBE, SETUP, STOP, TEARDOWN, PLAY";

//////////////////////////////////////////////////////////////////////////////
/// Default Describe response headers and msg body
//////////////////////////////////////////////////////////////////////////////
const char describe_ok_headers[] =
"Content-Type: application/sdp\r\n"
"Content-Length: ";

//////////////////////////////////////////////////////////////////////////////
/// Default Describe response - SDP
//////////////////////////////////////////////////////////////////////////////
const char describe_ok_body[] =
"v=0\r\n"
"o=fi.muni 2890844526 2890842807 IN IP4 127.0.0.1\r\n"
"s=FI Lectures\r\n"
"i=Live stream from D1/D2/D3\r\n"
"e=support@video.muni.cz\r\n"
"c=IN IP4 127.0.0.1\r\n"
"t=0 0\r\n"
"a=recvonly\r\n"
"m=video 1234 udp 33";

//////////////////////////////////////////////////////////////////////////////
/// Templates for RAP messages (CLIENTS ADD and CLIENTS REMOVE)
//////////////////////////////////////////////////////////////////////////////

const char rap_add_msg[] =
"CLIENTS RAP/1.0\r\n"
"Target: %s\r\n"
"Action: add\r\n"
"Address: %s/32\r\n\r\n";

const char rap_remove_msg[] =
"CLIENTS RAP/1.0\r\n"
"Target: %s\r\n"
"Action: remove\r\n"
"Address: %s/32\r\n\r\n";

#endif
