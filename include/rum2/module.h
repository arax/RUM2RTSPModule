/* $Id: module.h,v 1.27 2007/01/28 12:30:21 jirka Exp $ */
/*
 * Reflector module interface.
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
 * Reflector module interface.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.27 $ $Date: 2007/01/28 12:30:21 $
 */

#ifndef MODULE_H
#define MODULE_H

#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_PTHREAD_H
# include <pthread.h>
#endif

#include "error.h"
#include "queue.h"
#include "event.h"


/** Reflector module classes. */
enum module_class {
    /** Class @c reflector contains pseudomodules for any other class. */
    MC_REFLECTOR,
    /** Network listeners. */
    MC_LISTENER,
    /** Processors and Processor Scheduler. */
    MC_PROCESSOR,
    /** Packet scheduler/sender. */
    MC_SENDER,
    /** Administrative and Routing AAA modules. */
    MC_AAA,
    /** Management modules -- the core of the reflector. */
    MC_MANAGEMENT,
    /** Messaging interfaces (for management protocol). */
    MC_MSGINTERFACE
};


/** Reflector module identifier. */
struct module_id {
    /** Module's class. */
    enum module_class mclass;
    /** Module's name. */
    char *name;
};


/** @defgroup modulenames Fixed module names.
 *
 * Built-in (kernel or special) module names.
 *
 * @{
 */

/** "reflector/all" pseudomodule. */
#define REFLECTOR_ALL           "all"
/** "reflector/listener" pseudomodule. */
#define REFLECTOR_LISTENER      module_class(MC_LISTENER)
/** "reflector/processor" pseudomodule. */
#define REFLECTOR_PROCESSOR     module_class(MC_PROCESSOR)
/** "reflector/sender" pseudomodule. */
#define REFLECTOR_SENDER        module_class(MC_SENDER)
/** "reflector/aaa" pseudomodule. */
#define REFLECTOR_AAA           module_class(MC_AAA)
/** "reflector/management" pseudomodule. */
#define REFLECTOR_MANAGEMENT    module_class(MC_MANAGEMENT)
/** "reflector/msg-interface" pseudomodule. */
#define REFLECTOR_MSGINTERFACE  module_class(MC_MSGINTERFACE)

/** "processor/master" Processor Scheduler module. */
#define PROCESSOR_MASTER        "master"
/** "processor/void" pseudomodule. */
#define PROCESSOR_VOID          "void"

/** "sender/master" Packet Scheduler/Sender module. */
#define SENDER_MASTER           "master"

/** "aaa/administrative" module. */
#define AAA_ADMINISTRATIVE      "administrative"
/** "aaa/routing" module. */
#define AAA_ROUTING             "routing"

/** "management/master" kernel core module. */
#define MANAGEMENT_MASTER       "master"
/** "management/memory" module. */
#define MANAGEMENT_MEMORY       "memory"
/** "management/session" module. */
#define MANAGEMENT_SESSION      "session"

/** @} */


/** Module description structure. */
struct module {
    /** Pointer to the next module in the list. */
    struct module *next;
    /** Pointer to the previous module in the list. */
    struct module *prev;
    /** Nonzero if the module has been inserted into the list. */
    int inserted;

    /** Handle to a plugin implementing the module.
     * Set to NULL if the module is linked statically into the reflector. */
    void *plugin;

    /** Module number which is unique amongst all running modules. */
    int number;
    /** Nonzero if the module is part of reflector's kernel. */
    int kernel;
    /** Module identifier. */
    struct module_id id;
    /** Module startup parameter list. */
    struct module_param *params;
    /** Module function interface. */
    struct module_interface *iface;

    /** Error context. */
    struct rum_error_ctx *errctx;

    /** POSIX thread identifier. */
    pthread_t thread;
    /** Nonzero when module thread is running. */
    int started;

    /** Which queues should be freed when cleaning a module. */
    struct {
        uint32_t in_data:1;
        uint32_t out_data:1;
        uint32_t in_msg:1;
        uint32_t out_msg:1;
    } qclean;
    /** Input data queue. */
    struct queue *input_data;
    /** Output data queue. */
    struct queue *output_data;
    /** Input control message queue. */
    struct queue *input_message;
    /** Output control message queue. */
    struct queue *output_message;

    /** Module private data. */
    void *data;
};


/** Module startup parameter structure. */
struct module_param {
    /** Next parameter in the list. */
    struct module_param *next;

    /** Parameter name. */
    const char *name;
    /** Short parameter description.
     * Can contain any character except for comma (','), CR, and LF. */
    char *desc;
    /** Default value. */
    char *defval;
    /** Current value. */
    char *value;
};


/** Prototype for module initialization function.
 *
 * This is the first function to be called when a new module is to be started.
 * The function is responsible for initializing a blank module description
 * structure passed on to this function. To be more specific, the function has
 * to the following:
 *  - set pointers to all other module interface functions,
 *  - set a module's name (possibly temporary if the real name is derived from
 *    module's parameters) of the module being initialized,
 *  - and set up a list of startup parameters used by the module.
 * 
 * The only field that is guaranteed to be initialized before the initialize()
 * function is called is errctx. All other fields can be in undefined state
 * and they should be left untouched (except for module interface, identifier,
 * and parameters).
 *
 * For static modules (i.e. linked directly into the reflector binary) this
 * function is usually called <i>modulename</i>_initialize() to avoid
 * collision amongst several modules. On the other hand, for dynamically
 * loadable modules the name of this function must remain unchanged as this
 * function is the only entry point to a dynamically loadable library which
 * contains the module.
 *
 * @param module
 *      module description structure to be initialized.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
typedef int (*module_initialize)(struct module *module);


/** Current version of module_interface.
 * This must change everytime the interface structure changes. */
#define MODULE_VERSION 0x03

/** Module function interface. */
struct module_interface {
    /** Interface version.
     * If the value differs from MODULE_VERSION, the RUM_EMOD_INCOMPATIBLE
     * error code is issued and the module is not loaded. */
    int version;

    /** Construct a parameters-depended name of the module.
     * In case the name of a module is derived from parameters passed on when
     * starting the module, this function is supposed to construct the name.
     * This function can remain undefined in the oposite case.
     *
     * <b>Optional</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @param id
     *      use this number (if it is greater or equal to zero) to distinguish
     *      several modules of the same name. This parameter is generally used
     *      only for processor class modules.
     *
     * @return
     *      zero on success, nonzero otherwise.
     */
    int (*name)(struct module *module, int id);

    /** Get a list of conflicting module names.
     * In case the module conflicts not only with another module of the same
     * name, this function is supposed to provide a list of IDs of modules
     * which are incompatible with this one. If any of the modules specified
     * in the list is already running within the reflector, the
     * RUM_EMOD_CONFLICT (previously known as RUM_EMOD_RUNNING) error code will
     * be issued and the module will not be loaded into the reflector.
     *
     * <b>Optional</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @param ids
     *      place where a pointer to a list of conflicting module IDs shell
     *      be stored. The caller has to free() the list when it is no longer
     *      needed.
     *
     * @param re
     *      indicates whether module names contained in ids array should be
     *      treated as POSIX.2 extended regular expressions. A value of @c 0
     *      means names are plain text, nonzero means they are regular
     *      expressions.
     *
     * @return
     *      number of IDs in the list.
     */
    int (*conflicts)(struct module *module, struct module_id **ids, int *re);

    /** Initialize the module according to startup parameters.
     * Generally, all actions which have to be done before the main loop of the
     * module and which can fail (such as allocating memory for private data
     * structure needed by the module, opening network sockets, files, etc.)
     * have to be performed within this function. The init() function is the
     * last function which may fail. Once the initialization is succesfull, the
     * module must be able to start and stay running until it is stopped.
     *
     * In particular if the module reads from any queue it also has to register
     * queue group for waiting on that queue(s).
     *
     * <b>Required</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @return
     *      zero on success, nonzero otherwise.
     */
    int (*init)(struct module *module);

    /** Main module function.
     * This function contains the main execution loop of a module and is
     * (except for some rare cases) started as a separate thread. Usually the
     * main loop remains running until an external request to stop the module
     * is received.
     *
     * <b>Required</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @return
     *      nothing.
     */
    void (*main)(struct module *module);

    /** Callback function which is called after stopping the module.
     * This function is called immediately after stopping the module, even
     * if the thread was forced to stop (it was cancelled).
     *
     * If defined, it can be used to free resources allocated during the main
     * module function.
     *
     * <b>Optional</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @return
     *      nothing.
     */
    void (*stop)(struct module *module);

    /** Free resources allocated by the module.
     * After stopping a module, it can be either destroyed or restarted. In
     * both cases, clean() function is called to ensure all resources allocated
     * by the module are freed. When the module is being restarted, all data
     * allocated by init() and main() functions must be freed but any resource
     * which was allocated before init() was called must remain untouched, so
     * that the module can be restarted with the same name and parameters that
     * were used to start the module. All allocated resources (with no
     * exception) must be freed when the module is being stopped.
     *
     * <b>Required</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @param for_restart
     *      if nonzero, the module is being restarted. Otherwise it is
     *      being stopped.
     *
     * @return
     *      nothing.
     */
    void (*clean)(struct module *module, int for_restart);

    /** Push data directly to the module.
     * This function, if implemented, provides an alternative way to pass on
     * data to a module. In contrast to the other way using queues, this
     * function can be used for synchronous communication amongst modules.
     *
     * <b>Optional</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @param data
     *      pointer to data to be passed on.
     *
     * @return
     *      nothing.
     */
    void (*push_data)(struct module *module, void *data);

    /** Push control message directly to the module.
     * This function, if implemented, provides an alternative way to pass on
     * RAP messages to a module. In contrast to the other way using queues,
     * this function can be used for synchronous communication amongst modules.
     *
     * <b>Optional</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @param message
     *      pointer to the message to be passed on.
     *
     * @return
     *      nothing.
     */
    void (*push_message)(struct module *module, void *message);

    /** Callback used for delivering various events.
     * When a module wants to receive various internal events (as for now only
     * events regarding new modules are triggered), it has to implement this
     * callback function. All msg-interface class modules which want to receive
     * log messages are required to handle events regarding new modules and
     * thus they have to implement the events() callback.
     *
     * <b>Optional</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @param event
     *      event to be delivered.
     *
     * @param arg
     *      argument to the event.
     *
     * @return
     *      nothing.
     */
    void (*events)(struct module *module, enum event event, void *arg);

    /** Save module's configuration.
     * This function is called whenever the reflector needs to save its
     * current configuration. First, all reflector's modules are asked to
     * create RAP requests with START method which can be used to restart the
     * modules. This is done by calling config() function with start=1
     * parameter. When all START requests are saved, all modules are asked to
     * save their current configuration so that the reflector can later be
     * restarted and reconfigured to its current state. Again, this is done by
     * calling config() function but now start=0 parameter is passed on.
     *
     * <b>Required</b>
     *
     * @param module
     *      pointer to the module description structure.
     *
     * @param name
     *      if non-NULL, module has to use this name instead of its current
     *      one. This is used for setting correct names of processor class
     *      modules.
     *
     * @param start
     *      if nonzero, this function is supposed to write only START method
     *      which can later be used to start the module. In case the value is
     *      zero, all RAP requests (except for START method) describing current
     *      state of the module have to be saved.
     *
     * @return
     *      zero on success, nonzero otherwise.
     */
    int (*config)(struct module *module, const char *name, int start);
};


/** Check wheter all required functions of module's interface are implemented.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @return
 *      zero if all required functions are implemented, nonzero if any of them
 *      is missing.
 */
extern int module_check_iface(const struct module *module);


/** Macro for module data typecasting.
 *
 * @param module
 *      pointer to the module description structure (struct module).
 *
 * @param type
 *      the real type of module->data (e.g., struct private_module_data).
 *
 * @return
 *      pointer to the module data typecasted to (type *).
 */
#define module_data(module, type)   \
    ((type *) (module)->data)


/** Module class to module class string convertor.
 *
 * @param cls
 *      module class.
 *
 * @return
 *      module class string.
 */
extern char *module_class(enum module_class cls);


/** Decode module class from reflector/<i>mod</i> module name.
 *
 * @param name
 *      name of the module of class reflector. It MUST be a valid module
 *      identifier.
 *
 * @return
 *      module class.
 */
extern enum module_class reflector_class(const char *name);


/** Push data to module.
 * This function makes use of push interface if it is available. If the
 * module does not implement push interface queue interface is used instead.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @param data
 *      pointer to the data to be passed on.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int module_push_data(EC, struct module *module, void *data);


/** Push control message to module.
 * This function makes use of push interface if it is available. If the
 * module does not implement push interface queue interface is used instead.
 *
 * @param module
 *      pointer to the module description structure.
 *
 * @param message
 *      pointer to the control message to be passed on.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int module_push_message(EC, struct module *module, void *message);


/** Deliver synchronous event to a module.
 *
 * @param module
 *      target module.
 *
 * @param event
 *      event to be delivered.
 *
 * @param arg
 *      argument to the event.
 *
 * @return
 *      zero on success, nonzero when the given module has callback function
 *      for receiving events.
 */
extern int module_send_event(struct module *module, enum event event, void *arg);


#endif

