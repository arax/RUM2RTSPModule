/*
 Filter processor module.

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
/// Filter processor module for RUM2.
/// \author arax
/// \date 2010
/// \version 2010/03/17
//////////////////////////////////////////////////////////////////////////////

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rum2/limits.h>
#include <rum2/utils.h>
#include <rum2/error.h>
#include <rum2/mem.h>
#include <rum2/module.h>
#include <rum2/modparam.h>
#include <rum2/log.h>
#include <rum2/queue.h>
#include <rum2/rap-types.h>
#include <rum2/mod.h>
#include <rum2/data.h>
#include <rum2/rtp.h>
#include <rum2/processor.h>

#include "filter.h"

#if STATIC_PROCESSOR_FILTER || STATIC
int processor_filter_initialize(struct module *module)
#else
//////////////////////////////////////////////////////////////////////////////
/// Initialize processor/filter module.
/// \see module_initialize (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
int initialize(struct module *module)
#endif
{
    /* temporary module name */
    module->id.mclass = MC_PROCESSOR;
    module->id.name = PROCESSOR_FILTER;
    module->iface = &iface;

    if (modparam_init(module, params, params_count)) {
        rum_error_push(module->errctx, RUM_EPROC_INIT);
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
    char *name;
    int len;

    len = strlen(PROCESSOR_FILTER) + 1 + RUM_PROC_MAX_LEN;

    if ((name = (char *) malloc(len + 1)) == NULL) {
        rum_error(module->errctx, RUM_ENO_MEMORY);
        rum_error_push(module->errctx, RUM_EPROC_INIT);
        return -1;
    }

    snprintf(name, len + 1, "%s-%d", PROCESSOR_FILTER, id);
    name[len] = '\0';

    module->id.name = name;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Initialize module according to its parameters.
/// \see module_interface::init() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static int m_init(struct module *module){
    struct filter_data *data;

    module->data = (struct filter_data*) malloc(sizeof(struct filter_data));
    data = module_data(module, struct filter_data);
    if (module->data == NULL) {
        rum_error(module->errctx, RUM_ENO_MEMORY);
        rum_error_push(module->errctx, RUM_EPROC_INIT);
        return -1;
    }

    memset(data,'\0', sizeof(struct filter_data));

    data->qgroup = queue_group_reg(module->errctx, 1, module->input_data);
    if (data->qgroup == NULL) {
        rum_error_push(module->errctx, RUM_EPROC_INIT);
        return -1;
    }

    data->master = mod_find(MC_PROCESSOR, PROCESSOR_MASTER, 0);
    if (data->master == NULL) {
        rum_error_push(module->errctx, RUM_EPROC_INIT);
        return -1;
    }

    data->data_sample = modparam_get(module, PARAM_FILTER);
    if (data->data_sample == NULL) {
        rum_error_push(module->errctx, RUM_EPROC_INIT);
        return -1;
    }

    rlog(MC_MANAGEMENT, MANAGEMENT_MASTER, LOG_NOTICE,
         "Pre-start init done: %s/%s",
         module_class(module->id.mclass), module->id.name);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Module main function. Looks for data similar to \a Sample and masks them
/// in the output queue.
///
/// \param module Pointer to module structure.
//////////////////////////////////////////////////////////////////////////////
static void m_main(struct module *module){

    // Get data
    struct filter_data *data = module_data(module, struct filter_data);
    struct meta *meta;
    int stop = 0;
    int check_result;

    // Log start
    logm(&module->id, LOG_INFO, "Filter started with \"%s\" as data sample",
         data->data_sample);

    while(!stop){

        // Get item from queue
        if (queue_pop_data(module->input_data, (void **) &meta))
            queue_group_wait(data->qgroup);
        else {
            // Got data?
            if (meta != NULL && meta->data != NULL) {

                // Check data/sample similarities
                check_result = memcmp(data->data_sample, meta->data->buffer,
                                      sizeof(data->data_sample));

                // The same?
                if(check_result == 0){
                    // Mask it
                    meta_mask_all(meta, 0);
                    logm(&module->id, LOG_INFO,
                         "Removing \"%s\" from output_queue",
                         data->data_sample);
                }

                // Send along to the next module
                processor_path_pass(data->master, meta);
            }
            else{
                // Something went wrong
                rum_error_push(module->errctx, RUM_EPROC_PROCESS);
                logerrorm(module, LOG_ERROR);
            }
        }
    }

    logm(&module->id, LOG_INFO,"Filtering ended");
}

//////////////////////////////////////////////////////////////////////////////
/// Clean internal data and free all the memory.
/// \see module_interface::clean() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static void m_clean(struct module *module, int for_restart){
    struct filter_data *data = module_data(module, struct filter_data);

    if (data != NULL) {
        free(data);
    }

    if (!for_restart) {
        if (strcmp(module->id.name, PROCESSOR_FILTER)) {
            free(module->id.name);
            module->id.name = PROCESSOR_FILTER;
        }

        modparam_clean(module);
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Save module's configuration.
/// \see module_interface::config() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static int m_config(struct module *module, const char *name, int start){
    //////////////////////////////
    /// @todo save configuration
    /////////////////////////////
    UNUSED(module);
    UNUSED(name);
    UNUSED(start);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Stop all threads.
/// \see module_interface::stop() (RUM2 documentation)
//////////////////////////////////////////////////////////////////////////////
static void m_stop(struct module *module){
    ///////////////////////////
    /// @todo stop the filter
    ///////////////////////////
    UNUSED(module);
}
