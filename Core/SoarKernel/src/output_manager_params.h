/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  output_manager_params.h
 *
 * =======================================================================
 */

#ifndef OUTPUT_MANAGER_PARAMS_H_
#define OUTPUT_MANAGER_PARAMS_H_

#include "soar_db.h"

/**
 * @brief Contains the parameters for the debug command
 */
class OM_Parameters: public soar_module::param_container
{
  public:

    OM_Parameters();

    // storage
    soar_module::constant_param<soar_module::db_choices> *database;
    soar_module::string_param *path;
    soar_module::boolean_param *lazy_commit;
    soar_module::boolean_param *append_db;

    // performance
    soar_module::constant_param<soar_module::page_choices> *page_size;
    soar_module::integer_param *cache_size;
    soar_module::constant_param<soar_module::opt_choices> *opt;

    // Output Manager trace message output modes
    soar_module::boolean_param *db_mode;
    soar_module::boolean_param *XML_mode;
    soar_module::boolean_param *callback_mode;
    soar_module::boolean_param *stdout_mode;
    soar_module::boolean_param *file_mode;

    // Output Manager trace message debug output modes
    soar_module::boolean_param *db_dbg_mode;
    soar_module::boolean_param *XML_dbg_mode;
    soar_module::boolean_param *callback_dbg_mode;
    soar_module::boolean_param *stdout_dbg_mode;
    soar_module::boolean_param *file_dbg_mode;

};

#endif /* OUTPUT_MANAGER_PARAMS_H_ */
