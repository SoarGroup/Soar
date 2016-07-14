/*
 * output_settings.h
 *
 *  Created on: Jul 14, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_OUTPUT_SETTINGS_H_
#define CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_OUTPUT_SETTINGS_H_

#include "kernel.h"
#include "soar_module.h"

class OM_Parameters: public soar_module::param_container
{
    public:

        OM_Parameters();

        // storage
        soar_module::constant_param<soar_module::db_choices>* database;
        soar_module::string_param* path;
        soar_module::boolean_param* lazy_commit;
        soar_module::boolean_param* append_db;

        // performance
        soar_module::constant_param<soar_module::page_choices>* page_size;
        soar_module::integer_param* cache_size;
        soar_module::constant_param<soar_module::opt_choices>* opt;

};


#endif /* CORE_SOARKERNEL_SRC_OUTPUT_MANAGER_OUTPUT_SETTINGS_H_ */
