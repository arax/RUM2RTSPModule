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


#if HAVE_CONFIG_H
    #include "config.h"
#endif

#ifdef USE_IP6
    #error RTSP module cannot be used with IPv6!
#endif

#include <rum2/utils.h>
#include <rum2/error.h>
#include <rum2/module.h>
#include <rum2/modparam.h>
#include <rum2/log.h>
#include <rum2/queue.h>
#include <rum2/rap-types.h>
#include <rum2/rap.h>
#include <rum2/mod.h>
#include <rum2/data.h>
#include <rum2/session.h>
#include <rum2/mem.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <time.h>
#include <fcntl.h>
#include <ev.h>

#include "rtsp.h"
#include "rtsp_eris_headers.h"



#if STATIC_MSGIFACE_RTSP || STATIC
int msgiface_rtsp_initialize(struct module *module)
#else
//////////////////////////////////////////////////////////////////////////////
/// Initialize listener/rtsp module.
/// \see module_initialize (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
int initialize(struct module *module)
#endif
{
    // temporary module name
    module->id.mclass = MC_MSGINTERFACE;
    module->id.name = MSGIFACE_RTSP;
    module->iface = &iface;

    if (modparam_init(module, params, params_count)) {
        rum_error_push(module->errctx, RUM_EMSGIFACE_INIT);
        return -1;
    }

    rlog(MC_MANAGEMENT, MANAGEMENT_MASTER, LOG_NOTICE,
         "Module initialized: %s/%s",
         module_class(module->id.mclass), module->id.name);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Generate module's name according to parameters.
/// \see module_interface::name() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static int m_name(struct module *module, int id){
    long port;
    int len;
    char *name;
    char *address;

    UNUSED(id);

    if ((port = atol(modparam_get(module, PARAM_PORT))) <= 0
        || port > 65535) {
        rum_error(module->errctx, RUM_EMSGIFACE_PARAMS);
        return -1;
    }

    if ((address = modparam_get(module, PARAM_BIND_ADDR)) == NULL) {
        rum_error(module->errctx, RUM_EMSGIFACE_PARAMS);
        return -1;
    }

    len = strlen(MSGIFACE_RTSP) + 1 + 5 + strlen(address) + 1;

    if ((name = (char *) g_malloc0(len  + 1)) == NULL) {
        rum_error(module->errctx, RUM_ENO_MEMORY);
        rum_error_push(module->errctx, RUM_EMSGIFACE_INIT);
        return -1;
    }

    snprintf(name, len + 1, "%s-%s:%d",MSGIFACE_RTSP, address, (uint16_t) port);

    name[len] = '\0';

    module->id.name = name;
    
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Initialize module according to its parameters.
/// \see module_interface::init() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static int m_init(struct module *module){
    int list_s;                 // listening socket
    int port;                   // port number
    ADDR_TYPE servaddr;         // socket address structure
    RTSP_Server *srv;           // server structure
    char *address;              // addres in c_str
    char *listener_id;          // listener ID in c_str
    char *unix_socket;          // unix socket in c_str
    int reuseaddr_on = 1;       // setsockopt flag
    int flags;                  // fcntl flags
    int sockfd, servlen;
    struct sockaddr_un unixsocket_addr;

    // Get the port number from module parameters
    if((port = atol(modparam_get(module, PARAM_PORT))) <= 0 || port > 65535){
            rum_error(module->errctx, RUM_EMSGIFACE_PARAMS);
            return -1;
    }

    // Get the IP address from module parameters
    if ((address = modparam_get(module, PARAM_BIND_ADDR)) == NULL) {
            rum_error(module->errctx, RUM_EMSGIFACE_PARAMS);
            return -1;
    }

    // Get listener ID from module parameters
    if ((listener_id = modparam_get(module, PARAM_LISTENER_ID)) == NULL) {
            rum_error(module->errctx, RUM_EMSGIFACE_PARAMS);
            return -1;
    }

    // Get UNIX socket from module parameters
    if ((unix_socket = modparam_get(module, PARAM_UNIX_SOCKET)) == NULL) {
            rum_error(module->errctx, RUM_EMSGIFACE_PARAMS);
            return -1;
    }

    // Create a socket
    if ((list_s = socket(AF_INET46, SOCK_STREAM, 0)) < 0 ) {
	rum_error(module->errctx, RUM_EMSGIFACE_INIT);
        return -1;
    }

    setsockopt(list_s, SOL_SOCKET, SO_REUSEADDR,
               &reuseaddr_on, sizeof(reuseaddr_on));

    // Reset all bytes in socket address structure and fill in IP + port
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.SIN_AF = AF_INET46;
    if (inet_pton(AF_INET46, address, &servaddr.SIN_ADDR) <= 0) {
        rum_error(module->errctx, RUM_EMSGIFACE_PARAMS);
        return -1;
    }
    servaddr.SIN_PORT = htons(port);


    // Bind socket addresss to the listening socket
    if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0 ) {
	rum_error(module->errctx, RUM_EMSGIFACE_INIT);
        return -1;
    }

    // Start listening
    if ( listen(list_s, LISTENQ) != 0 ) {
	rum_error(module->errctx, RUM_EMSGIFACE_INIT);
        return -1;
    }

    // Set socket NONBLOCK flag
    flags = fcntl(list_s, F_GETFL);
    if (flags < 0) return -1;

    flags |= O_NONBLOCK;
    if (fcntl(list_s, F_SETFL, flags) < 0) return -1;

    // Allocate memory for private data structure
    module->data = (RTSP_Server*)g_malloc0(sizeof(RTSP_Server));
    if(module->data == NULL) {
	rum_error(module->errctx, RUM_EMSGIFACE_INIT);
        return -1;
    }

    //
    bzero((char *)&unixsocket_addr,sizeof(unixsocket_addr));
    unixsocket_addr.sun_family = AF_UNIX;
    strcpy(unixsocket_addr.sun_path, unix_socket);
    servlen = strlen(unixsocket_addr.sun_path) + sizeof(unixsocket_addr.sun_family);

    //
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM,0)) < 0){
       rum_error(module->errctx, RUM_EMSGIFACE_INIT);
       return -1;
    }
    if (connect(sockfd, (struct sockaddr *) &unixsocket_addr, servlen) < 0){
       rum_error(module->errctx, RUM_EMSGIFACE_INIT);
       return -1;
    }

    // Assign vars to members in srv
    srv = module_data(module, RTSP_Server);
    srv->servaddr = g_malloc0(sizeof(servaddr));
    memcpy(srv->servaddr,&servaddr,sizeof(servaddr));
    srv->list_s = list_s;
    srv->client_list = g_hash_table_new_full(g_str_hash, g_str_equal,
                                             g_free, g_free);
    srv->server_state = RTSP_SERVER_INIT;
    srv->srv_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    srv->unixsocket_fd = sockfd;
    srv->listener_id = listener_id;

    // Prepare main loop
    srv->loop = ev_default_loop (0);
    
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Main function of the module.
/// \see module_interface::main() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static void m_main(struct module *module){
    int list_s;                     // listener socket
    char srv_straddr[ADDRSTR_LEN];  // address as a c_str
    RTSP_Server *srv;               // server structure

    // Get data from module
    srv = module_data(module, RTSP_Server);
    if(srv == NULL){
        logerrorm(module, LOG_ERROR);
        return;
    }

    // Assign module pointer to data in ev_accept (accessible later)
    srv->ev_accept.data = module;

    // Get listener socket and set state READY
    list_s = srv->list_s;
    srv->server_state = RTSP_SERVER_READY;

    // Get c_str for server address
    inet_ntop(AF_INET46, &(srv->servaddr->SIN_ADDR),
              srv_straddr,sizeof(srv_straddr));

    logm(&module->id, LOG_INFO,"RTSP server started, listening on %s:%d",
            srv_straddr, ntohs(srv->servaddr->SIN_PORT));

    // Start listening for READ events on list_s socket
    ev_io_init(&srv->ev_accept,accept_connection,list_s,EV_READ);
    ev_io_start(srv->loop,&srv->ev_accept);

    // Start the main loop
    ev_loop (srv->loop, 0);
}

//////////////////////////////////////////////////////////////////////////////
/// Clean internal data and free all the memory.
/// \see module_interface::clean() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static void m_clean(struct module *module, int for_restart){
    RTSP_Server *srv = module_data(module, RTSP_Server);

    // Free memory allocated for private data structure
    if(srv != NULL){
        g_hash_table_destroy(srv->client_list);
        pthread_mutex_destroy(&srv->srv_mutex);
        g_free(srv->servaddr);
        g_free(srv);
    }

    // Clean module parameters if it's not a restart
    if (!for_restart) {
        if (strcmp(module->id.name, MSGIFACE_RTSP)) {
            free(module->id.name);
            module->id.name = MSGIFACE_RTSP;
        }

        modparam_clean(module);
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Stop all threads (and loops).
/// \see module_interface::stop() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static void m_stop(struct module *module){
    RTSP_Server *srv = module_data(module, RTSP_Server);

    ev_io_stop(srv->loop, &srv->ev_accept);
    ev_unloop (srv->loop, EVUNLOOP_ALL);
}

//////////////////////////////////////////////////////////////////////////////
/// Save module's configuration.
/// \see module_interface::config() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static int m_config(struct module *module, const char *name, int start){
    ////////////////////////////
    /// @todo save configuration
    ////////////////////////////
    UNUSED(module);
    UNUSED(name);
    UNUSED(start);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Accept connection from a client after READ event has been triggered in
/// the main watcher (in m_main()). Prepare structures, start new READ watcher
/// and timeout timer.
///
/// \param loop The main loop (we need to set timeout and new watcher for READ).
/// \param w The watcher structure calling accept_connection (with data).
/// \param revents Flag for current event.
//////////////////////////////////////////////////////////////////////////////
static void accept_connection(struct ev_loop *loop, struct ev_io *w, int revents){
    RTSP_Client *client;                // client structure
    char clnt_straddr[ADDRSTR_LEN];     // client address as a c_str
    ADDR_TYPE clientaddr;               // client address (structure)
    int addrsize;                       // size of the address
    int sd;                             // accepted socket descriptor
    int list_s = w->fd;                 // listener socket descriptor
    struct module *module;              // module structure
    timeout_data *to_data;              // timeout data structure

    // Get module data from the watcher
    module = (struct module *) w->data;

    // Accept connection (NONBLOCK)
    addrsize = sizeof(clientaddr);
    sd = accept(list_s,(struct sockaddr *)&clientaddr, &addrsize);

    // Allocate memory for client data
    client = (RTSP_Client *) g_malloc0(sizeof(RTSP_Client));
    if(client == NULL){
        logerror(module->id.mclass, module->id.name, LOG_ERROR,
             module->errctx, "Memory allocation failure - struct client");
        return;
    }

    inet_ntop(AF_INET46, &(clientaddr.SIN_ADDR),
              clnt_straddr, sizeof(clnt_straddr));

    logm(&module->id, LOG_INFO,"Connection with %s:%d established",
        clnt_straddr, ntohs(clientaddr.SIN_PORT));

    // Assign vars to members
    client->clientaddr = g_malloc0(sizeof(clientaddr));
    memcpy(client->clientaddr,&clientaddr,sizeof(clientaddr));
    client->socket = sd;
    client->ev_read.data = module;
    client->stop = FALSE;
    client->msg_cseq = 0;
    client->req = NULL;
    client->response = NULL;
    client->sessionID = NULL;

    // Initialize timeout timer
    ev_init(&client->timer, timeout);
    client->timer.repeat = TIMEOUT;

    // Allocate memory for timeout data
    to_data = g_malloc0(sizeof(timeout_data));
    to_data->client = client;
    to_data->module = module;
    client->timer.data = to_data;

    // Start timer
    ev_timer_again(loop, &client->timer);

    // Start READ watcher on the accepted connection
    ev_io_init(&client->ev_read,process,client->socket,EV_READ);
    ev_io_start(loop,&client->ev_read);
}

//////////////////////////////////////////////////////////////////////////////
/// Clean-up memory after timeout occurs, remove client from \a client_list
/// and terminate the connection.
///
/// \param loop The main loop (\a module_data and logs).
/// \param timer The timer structure triggering timeout (client data).
/// \param revents Flags for this event.
//////////////////////////////////////////////////////////////////////////////
static void timeout(struct ev_loop *loop, ev_timer *timer, int revents){

    if(timer == NULL) return;

    // Get data
    timeout_data *to_data = (timeout_data *) timer->data;

    if(to_data == NULL) return;

    RTSP_Client *client = to_data->client;
    struct module *module = to_data->module;
    RTSP_Server *srv = module_data(module, RTSP_Server);
    char clnt_straddr[ADDRSTR_LEN];
    gboolean hashtableret = FALSE;

    // Stop timer
    ev_timer_stop(loop, timer);

    // Clean-up after timeout
    if(client != NULL && module != NULL && srv != NULL){
        inet_ntop(AF_INET46, &(client->clientaddr->SIN_ADDR), clnt_straddr,
                  sizeof(clnt_straddr));

        // Remove client from client list
        if(client->sessionID != NULL){
            pthread_mutex_lock(&srv->srv_mutex);
            hashtableret = g_hash_table_remove(srv->client_list,client->sessionID);
            pthread_mutex_unlock(&srv->srv_mutex);

            // Send RAP to remove this client
            RAP_Msg_data *rap_msg_data = (RAP_Msg_data *) g_malloc0(sizeof(RAP_Msg_data));

            rap_msg_data->ip = (char *) g_malloc0(sizeof(clnt_straddr));
            strcpy(rap_msg_data->ip, clnt_straddr);
            rap_msg_data->type = RAP_CLIENTS_REMOVE;

            send_rap_msg(module, rap_msg_data);

            g_free(client->sessionID);
        }

        // Removed?
        if(hashtableret)
            logm(&module->id, LOG_INFO, "Connection to %s:%d terminated - "
             "timeout (client removed from the client list)",
             clnt_straddr, ntohs(client->clientaddr->SIN_PORT));
        else
            logm(&module->id, LOG_INFO, "Connection to %s:%d terminated - "
             "timeout (client was not in the client list)",
             clnt_straddr, ntohs(client->clientaddr->SIN_PORT));

        // Stop READ/WRITE watchers
        ev_io_stop(loop, &client->ev_read);
        ev_io_stop(loop, &client->ev_write);

        // Terminate connection
        close(client->socket);

        // Clean-up client data
        g_free(client->clientaddr);
        if(client->req != NULL) g_free(client->req);
        g_free(client);
    }

    // Clean-up timer data
    g_free(to_data);
}

//////////////////////////////////////////////////////////////////////////////
/// Message processing function launched from \a accept_connection after READ
/// event occured.
///
/// \param loop Event loop structure (main loop).
/// \param w The watcher structure calling process (after a READ event).
/// \param revents Event flags for current event.
//////////////////////////////////////////////////////////////////////////////
static void process(struct ev_loop *loop, struct ev_io *w, int revents){

    // Something is very wrong!
    if(w == NULL) return;
    
    char msg[MAX_MSG];                          // input message buffer
    int headers_length,request_line_length;     // for parser retvals
    char *session_hdr = NULL;                   // client session ID (from msg)
    gboolean hashtableret;                      // hashtable remove retval
    char clnt_straddr[ADDRSTR_LEN];             // client address as c_str
    int req_err = FALSE;                        // processing error
    struct module *module;                      // module structure
    RTSP_Server *srv;                           // server structure
    RTSP_Client *client;                        // client structure

    // Get data from watcher
    module = (struct module *)w->data;
    srv = module_data(module, RTSP_Server);
    client = ((RTSP_Client *) (((char *)w)
                           - offsetof(RTSP_Client, ev_read)));

    if(client == NULL){
        logerror(module->id.mclass, module->id.name, LOG_ERROR,
                 module->errctx, "Unexpected null pointer (client)");

        ev_io_stop(EV_A_ w);

        return;
    }

    // Read incoming msg from socket
    if (revents & EV_READ){
        memset(msg,'\0',sizeof(msg));

        read(client->socket, msg, sizeof(msg) - 1);

        // Reset timeout timer
        ev_timer_again(loop, &client->timer);
    }

    inet_ntop(AF_INET46, &(client->clientaddr->SIN_ADDR),
              clnt_straddr,sizeof(clnt_straddr));

    // Could not read message from socket buffer
    if(msg == NULL || strlen(msg) == 0){
        logm(&module->id, LOG_INFO, "Unable to read incoming message (wrong "
             "format or connection closed)");

        // Stop READ watcher
        ev_io_stop(EV_A_ w);

        // Clean-up and terminate
        if(client != NULL){
            close(client->socket);

            logm(&module->id, LOG_INFO, "Connection to %s:%d terminated",
                 clnt_straddr, ntohs(client->clientaddr->SIN_PORT));

            // Remove client from client list
            if(client->sessionID != NULL){
                pthread_mutex_lock(&srv->srv_mutex);
                g_hash_table_remove(srv->client_list,client->sessionID);
                pthread_mutex_unlock(&srv->srv_mutex);

                // Send RAP to remove this client
                RAP_Msg_data *rap_msg_data = (RAP_Msg_data *) g_malloc0(sizeof(RAP_Msg_data));

                rap_msg_data->ip = (char *) g_malloc0(sizeof(clnt_straddr));
                strcpy(rap_msg_data->ip, clnt_straddr);
                rap_msg_data->type = RAP_CLIENTS_REMOVE;

                send_rap_msg(module, rap_msg_data);

                g_free(client->sessionID);
            }

            g_free(client->clientaddr);
            ev_timer_stop(loop, &client->timer);
            g_free(client->timer.data);
            g_free(client);
        }

        return;
    }

    // Allocate memory for request data structure
    client->req = g_malloc0(sizeof(RTSP_Request));

    // Allocation failed - clean-up and terminate
    if(client->req == NULL){
        logerror(module->id.mclass, module->id.name, LOG_ERROR,
                 module->errctx, "Memory allocation failure - connection closed");

        ev_io_stop(EV_A_ w);

        close(client->socket);

        // Remove client from client list
        if(client->sessionID != NULL){
            pthread_mutex_lock(&srv->srv_mutex);
            g_hash_table_remove(srv->client_list,client->sessionID);
            pthread_mutex_unlock(&srv->srv_mutex);

            // Send RAP to remove this client
            RAP_Msg_data *rap_msg_data = (RAP_Msg_data *) g_malloc0(sizeof(RAP_Msg_data));

            rap_msg_data->ip = (char *) g_malloc0(sizeof(clnt_straddr));
            strcpy(rap_msg_data->ip, clnt_straddr);
            rap_msg_data->type = RAP_CLIENTS_REMOVE;

            send_rap_msg(module, rap_msg_data);

            g_free(client->sessionID);
        }

        g_free(client->clientaddr);
        ev_timer_stop(loop, &client->timer);
        g_free(client->timer.data);
        g_free(client);

        return;
    }

    // Log request origin
    logm(&module->id, LOG_INFO,"Processing RTSP request/message from %s:%d",
        clnt_straddr, ntohs(client->clientaddr->SIN_PORT));

    // Reset the request data structure
    memset(client->req,'\0',sizeof(RTSP_Request));

    // Preliminary request check
    request_line_length = ragel_parse_request_line(msg, strlen(msg), client->req);


    // Parser did not recognize RTSP protocol, stop timer and set req_err
    if(client->req->version == NULL || client->req->method == NULL){
        logm(&module->id, LOG_INFO, "Received request is invalid (parser)");
        ev_timer_stop(loop, &client->timer);
        req_err = TRUE;
    }

    // Everything is ok, RTSP ver. is 1.0
    if( !req_err && strcmp(client->req->version,"RTSP/1.0") == 0){

        // RTSP request info - version and method
        logm(&module->id, LOG_INFO, "Request: %s from %s:%d",
             client->req->method, clnt_straddr,
             ntohs(client->clientaddr->SIN_PORT));

        // Parse the rest of RTSP headers
        headers_length = eris_parse_headers(msg + request_line_length,
                                            strlen(msg) - request_line_length,
                                            &client->req->headers);

        // No headers, set response and skip the rest
        if(headers_length == 0 || client->req->headers == NULL){
            set_response(client, BAD_REQUEST, session_hdr);
            ev_timer_stop(loop, &client->timer);
            client->stop = TRUE;
        }
        else{

            // Get CSeq value from headers
            client->msg_cseq = atoi(g_hash_table_lookup(client->req->headers,
                                                        eris_hdr_cseq));

            // Check CSeq existence (VLC problems)
            if(client->msg_cseq <= 0){
                // Basic cseq check - something is wrong
                set_response(client,BAD_REQUEST,session_hdr);
                ev_timer_stop(loop, &client->timer);
                client->stop = TRUE;
            }
            else{

                // Get session ID from headers
                session_hdr = g_hash_table_lookup(client->req->headers,
                                                  eris_hdr_session);

                // Select method handler
                switch(client->req->method_id){
                    case RTSP_ID_OPTIONS:
                        // Respond with server capabilities
                        set_response(client,OPTIONS_PUBLIC_OK,session_hdr);
                        break;
                    case RTSP_ID_DESCRIBE:
                        // Send audio/video specs in SDP
                        set_response(client,DESCRIBE_OK,session_hdr);
                        break;
                    case RTSP_ID_SETUP:
                        // Generate session id
                        if(session_hdr == NULL){
                            session_hdr = gen_sess_id(client);

                            logm(&module->id, LOG_INFO, "New RTSP session ID "
                                 "for %s:%d is %s", clnt_straddr,
                                 ntohs(client->clientaddr->SIN_PORT),
                                 session_hdr);

                            // Remember ID, basic security feature
                            client->sessionID = g_strdup(session_hdr);
                        }

                        set_response(client,SETUP_OK,session_hdr);

                        break;
                    case RTSP_ID_PLAY:
                        // Test Sess ID and add client
                        if((session_hdr != NULL) && (client->sessionID != NULL)
                            && (strcmp(session_hdr, client->sessionID) == 0)){
                            
                            // Client is now listening
                            // Send RAP message to add this client
                            RAP_Msg_data *rap_msg_data = (RAP_Msg_data *) g_malloc0(sizeof(RAP_Msg_data));
                            
                            rap_msg_data->ip = (char *) g_malloc0(sizeof(clnt_straddr));
                            strcpy(rap_msg_data->ip, clnt_straddr);
                            rap_msg_data->type = RAP_CLIENTS_ADD;

                            send_rap_msg(module, rap_msg_data);

                            // Prepare response and set timer
                            set_response(client,PLAY_OK, session_hdr);

                            ev_timer_stop(loop, &client->timer);

                            // Register a new client
                            pthread_mutex_lock(&srv->srv_mutex);
                            g_hash_table_insert(srv->client_list,
                                                g_strdup(session_hdr),
                                                g_memdup(client->clientaddr,
                                                sizeof(ADDR_TYPE)));
                            pthread_mutex_unlock(&srv->srv_mutex);
                        }
                        else{
                            set_response(client,SESSION_NOT_FOUND, session_hdr);
                        }
                        break;
                    case RTSP_ID_TEARDOWN:
                        // STOP requests will be described as RTSP_ID_TEARDOWN
                        // End session, remove client info from the client_list
                        if((session_hdr != NULL) && (client->sessionID != NULL)
                            && (strcmp(session_hdr, client->sessionID) == 0)){

                            set_response(client,TEARDOWN_OK, session_hdr);

                            logm(&module->id, LOG_INFO,
                                 "Session with ID %s ended", session_hdr);

                            // Remove the client
                            pthread_mutex_lock(&srv->srv_mutex);
                            hashtableret = g_hash_table_remove(srv->client_list,
                                                               session_hdr);
                            pthread_mutex_unlock(&srv->srv_mutex);

                            if(!hashtableret)
                                logerror(module->id.mclass, module->id.name,
                                         LOG_ERROR, module->errctx,
                                         "Failed to remove a client from "
                                         "client list (wrong session ID?)");
                            ev_timer_stop(loop, &client->timer);

                            // Send RAP to remove this client
                            RAP_Msg_data *rap_msg_data = (RAP_Msg_data *) g_malloc0(sizeof(RAP_Msg_data));

                            rap_msg_data->ip = (char *) g_malloc0(sizeof(clnt_straddr));
                            strcpy(rap_msg_data->ip, clnt_straddr);
                            rap_msg_data->type = RAP_CLIENTS_REMOVE;

                            send_rap_msg(module, rap_msg_data);

                            client->stop = TRUE;
                        }
                        else{
                            set_response(client,SESSION_NOT_FOUND, session_hdr);
                        }
                        break;
                    default:
                        // Send 501 Not Implemented message to the client
                        set_response(client,NOT_IMPLEMENTED, session_hdr);
                        ev_timer_stop(loop, &client->timer);
                        client->stop = TRUE;
                }
            }
        }
    }
    else{
        // Send 400 Bad Request message to the client
        logm(&module->id, LOG_INFO,
             "Unknown request type/format (probably not RTSP/1.0)",
             clnt_straddr, ntohs(client->clientaddr->SIN_PORT));

        set_response(client,BAD_REQUEST,session_hdr);
        ev_timer_stop(loop, &client->timer);
        client->stop = TRUE;
    }

    // Prepare data for response
    client->ev_write.data = module;

    // Is this the end? Stop the READ watcher
    if(client->stop) ev_io_stop(EV_A_ w);

    // Start the WRITE watcher
    ev_io_init(&client->ev_write,send_msg,client->socket,EV_WRITE);
    ev_io_start(loop,&client->ev_write);
    
    logm(&module->id, LOG_INFO,"Request from %s:%d processed",
        clnt_straddr, ntohs(client->clientaddr->SIN_PORT));

    // Clean-up request structure
    g_free(client->req->method);
    g_free(client->req->object);
    g_free(client->req->version);

    if(client->req->headers != NULL) g_hash_table_destroy(client->req->headers);

    // Session_hdr has been freed in client->req->headers
    g_free(client->req);
    client->req = NULL;
}

//////////////////////////////////////////////////////////////////////////////
/// Sends a response message to the client as soon as the socket is ready
/// for write (WRITE event).
///
/// \param loop The main loop (\a module_data and logs).
/// \param w The watcher structure triggering send_msg (client data).
/// \param revents Flags for this event.
//////////////////////////////////////////////////////////////////////////////
static void send_msg(struct ev_loop *loop, struct ev_io *w, int revents){

    if(w == NULL) return;

    char clnt_straddr[ADDRSTR_LEN];     // client address as c_str
    RTSP_Client *client;                // client structure
    struct module *module;              // module structure
    RTSP_Server *srv;                   // module structure

    // Get data
    client = ((RTSP_Client *) (((char *)w) - offsetof(RTSP_Client,ev_write)));
    module = (struct module *)w->data;
    srv = module_data(module, RTSP_Server);

    // Check data existence
    if(client == NULL || client->response == NULL || srv == NULL){
         logerror(module->id.mclass, module->id.name, LOG_ERROR, module->errctx,
                  "Couldn't send a response - unexpected NULL pointer");
         return;
    }

    inet_ntop(AF_INET46, &(client->clientaddr->SIN_ADDR), clnt_straddr,
              sizeof(clnt_straddr));

    // Send response to the client
    if (revents & EV_WRITE){
        write(client->socket,client->response,strlen(client->response));

        logm(&module->id, LOG_INFO, "Response sent to %s:%d",
                 clnt_straddr, ntohs(client->clientaddr->SIN_PORT));
    }

    // Stop WRITE watcher and clean-up
    ev_io_stop(EV_A_ w);
    g_free(client->response);
    client->response = NULL;

    // Is this the end? Clean-up and terminate
    if(client->stop){
        // Stop timer
        ev_timer_stop(loop, &client->timer);
        close(client->socket);

        logm(&module->id, LOG_INFO, "Connection to %s:%d terminated",
                 clnt_straddr, ntohs(client->clientaddr->SIN_PORT));

        // Remove client from client list
        if(client->sessionID != NULL){
            pthread_mutex_lock(&srv->srv_mutex);
            g_hash_table_remove(srv->client_list,client->sessionID);
            pthread_mutex_unlock(&srv->srv_mutex);

            // Send RAP to remove this client
            RAP_Msg_data *rap_msg_data = (RAP_Msg_data *) g_malloc0(sizeof(RAP_Msg_data));

            rap_msg_data->ip = (char *) g_malloc0(sizeof(clnt_straddr));
            strcpy(rap_msg_data->ip, clnt_straddr);
            rap_msg_data->type = RAP_CLIENTS_REMOVE;

            send_rap_msg(module, rap_msg_data);

            g_free(client->sessionID);
        }

        g_free(client->clientaddr);
        g_free(client->timer.data);
        g_free(client);
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Sends RAP msg to the chosen UNIX socket (address is stored in srv data
/// structure) - CLIENTS ADD or CLIENTS REMOVE msgs only.
///
/// \param module Module data structure pointer (logging and srv data).
/// \param data Client's IP address and msg type structure.
//////////////////////////////////////////////////////////////////////////////
static int send_rap_msg(struct module *module, RAP_Msg_data *data){
    RTSP_Server *srv;                   // module structure
    char buffer[250];
    char out_buffer[100];

    if(module == NULL || data == NULL) return -1;

    srv = module_data(module, RTSP_Server);
    
    if(srv == NULL) return -1;

    bzero(out_buffer, strlen(out_buffer));

    switch(data->type){
        case RAP_CLIENTS_ADD:
            sprintf(out_buffer, rap_add_msg, srv->listener_id, data->ip);
            break;
        case RAP_CLIENTS_REMOVE:
            sprintf(out_buffer, rap_remove_msg, srv->listener_id, data->ip);
            break;
        default:
            logerror(module->id.mclass, module->id.name, LOG_ERROR, module->errctx,
                  "Unknown RAP msg type in send_rap_msg!");
            return -1;
    }

    if(write(srv->unixsocket_fd, out_buffer, strlen(out_buffer)) < 0) return -1;

    bzero(buffer, strlen(buffer));

    read(srv->unixsocket_fd, buffer, 248);

    g_free(data->ip);
    g_free(data);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Set default response. Static messages are combined with dynamic session
/// IDs and time stamps.
///
/// \param client The target client structure.
/// \param msg_type Type of the response message (enum).
/// \param session_hdr Session ID of the current client.
//////////////////////////////////////////////////////////////////////////////
static int set_response(RTSP_Client *client,RTSP_Response_Msg msg_type, char* session_hdr){
    char *msg;                              // outgoing message
    char clnt_straddr[ADDRSTR_LEN];         // client address as c_str
    char *timestamp;                        // current timestamp

    // MAX_MSG_OUT-1 characters (default value is 600)
    msg = g_malloc0(MAX_MSG_OUT*sizeof(char));

    if(msg == NULL || client == NULL){
        return -1;
    }

    // Temporary STOP solution (STOP is not being used anyway)
    if(msg_type == STOP_OK) msg_type = TEARDOWN_OK;

    inet_ntop(AF_INET46, &(client->clientaddr->SIN_ADDR),
              clnt_straddr,sizeof(clnt_straddr));

    // Select message type and construct the message
    switch(msg_type){
        case OPTIONS_PUBLIC_OK:
            sprintf(msg, "%s%s%s%d%s%s%s", rtsp_ok, msg_newline,
                    msg_cseq, client->msg_cseq, msg_newline, options_public_ok,
                    msg_end);
            break;
        case BAD_REQUEST:
            sprintf(msg, "%s%s%s%d%s", rtsp_bad_request, msg_newline,
                    msg_cseq ,client->msg_cseq, msg_end);
            break;
        case NOT_IMPLEMENTED:
            sprintf(msg, "%s%s%s%d%s", rtsp_not_implemented, msg_newline,
                    msg_cseq, client->msg_cseq, msg_end);
            break;
        case DESCRIBE_OK:
            timestamp = rtsp_timestamp();
            sprintf(msg, "%s%s%s%d%s%s%s%s%s%d%s%s%s", rtsp_ok, msg_newline,
                    msg_cseq, client->msg_cseq, msg_newline, msg_date,
                    timestamp, msg_newline, describe_ok_headers,
                    sizeof(describe_ok_body), msg_end, describe_ok_body,
                    msg_end);
            g_free(timestamp);
            break;
        case SETUP_OK:
            sprintf(msg, "%s%s%s%d%s%s%s%s%s%s%s%s%s", rtsp_ok, msg_newline,
                    msg_cseq, client->msg_cseq, msg_newline, msg_server,
                    msg_newline, msg_sess, session_hdr, msg_timeout,
                    msg_newline, msg_transport, msg_end);
            break;
        case SESSION_NOT_FOUND:
            sprintf(msg, "%s%s%s%d%s", rtsp_sess_not_found, msg_newline,
                    msg_cseq, client->msg_cseq, msg_end);
            break;
        case PLAY_OK:
            sprintf(msg, "%s%s%s%d%s%s%s%s", rtsp_ok, msg_newline, msg_cseq,
                    client->msg_cseq, msg_newline, msg_sess, session_hdr,
                    msg_end);
            break;
        case TEARDOWN_OK:
            sprintf(msg, "%s%s%s%d%s%s%s%s", rtsp_ok, msg_newline, msg_cseq,
                    client->msg_cseq, msg_newline, msg_sess, session_hdr,
                    msg_end);
            break;
        default:
            //unknown error msg
            sprintf(msg, "%s%s%s%d%s", rtsp_internal_srv_error, msg_newline,
                    msg_cseq, client->msg_cseq, msg_end);
            break;
    }

    // Assign message to the client
    client->response = msg;
    
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Get a time stamp for the response message or a session ID generator.
//////////////////////////////////////////////////////////////////////////////
static char *rtsp_timestamp() {
  char buffer[31] = { 0, };     // buffer for timestamp

  // Get current time
  time_t now = time(NULL);
  struct tm *t = gmtime(&now);

  // Formatting
  strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", t);

  return g_strdup(buffer);
}

//////////////////////////////////////////////////////////////////////////////
/// Generate a new session ID from the client IP, port and current timestamp.
///
/// \param client The client structure (with IP and port number).
/// \return Pointer to a c_str with the new session ID.
//////////////////////////////////////////////////////////////////////////////
static char *gen_sess_id(RTSP_Client *client){

    char tmp[ADDRSTR_LEN + 31 + 5];         // text buffer
    char clnt_straddr[ADDRSTR_LEN];         // client address as c_str
    char tmp1[12];                          // session ID buffer
    char *timestamp;                        // timestamp

    inet_ntop(AF_INET46, &(client->clientaddr->SIN_ADDR),
              clnt_straddr,sizeof(clnt_straddr));

    // Reset buffers
    memset(tmp,'\0',sizeof(tmp));
    memset(tmp1,'\0',sizeof(tmp1));

    timestamp = rtsp_timestamp();

    // Formatting
    sprintf(tmp,"%s%d%s",clnt_straddr,
            ntohs(client->clientaddr->SIN_PORT),timestamp);
    sprintf(tmp1,"%d",g_str_hash(tmp));

    // Free timestamp
    g_free(timestamp);

    return g_strdup(tmp1);
}
