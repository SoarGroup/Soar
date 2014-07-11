/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  output_manager_params.cpp
 *
 * =======================================================================
 */

#include "debug_defines.h"
#include "output_manager_params.h"

OM_Parameters::OM_Parameters(): soar_module::param_container(NULL)
{
    database = new soar_module::constant_param<soar_module::db_choices>("database", soar_module::file, new soar_module::f_predicate<soar_module::db_choices>());
    database->add_mapping(soar_module::memory, "memory");
    database->add_mapping(soar_module::file, "file");
    append_db = new soar_module::boolean_param("append-database", off, new soar_module::f_predicate<boolean>());
    path = new soar_module::string_param("path", "debug.db", new soar_module::predicate<const char*>(), new soar_module::f_predicate<const char*>());
    lazy_commit = new soar_module::boolean_param("lazy-commit", off, new soar_module::f_predicate<boolean>());
    page_size = new soar_module::constant_param<soar_module::page_choices>("page-size", soar_module::page_8k, new soar_module::f_predicate<soar_module::page_choices>());
    page_size->add_mapping(soar_module::page_1k, "1k");
    page_size->add_mapping(soar_module::page_2k, "2k");
    page_size->add_mapping(soar_module::page_4k, "4k");
    page_size->add_mapping(soar_module::page_8k, "8k");
    page_size->add_mapping(soar_module::page_16k, "16k");
    page_size->add_mapping(soar_module::page_32k, "32k");
    page_size->add_mapping(soar_module::page_64k, "64k");
    cache_size = new soar_module::integer_param("cache-size", 10000, new soar_module::gt_predicate<int64_t>(1, true), new soar_module::f_predicate<int64_t>());
    opt = new soar_module::constant_param<soar_module::opt_choices>("optimization", soar_module::opt_safety, new soar_module::f_predicate<soar_module::opt_choices>());
    opt->add_mapping(soar_module::opt_safety, "safety");
    opt->add_mapping(soar_module::opt_speed, "performance");
    
    db_mode = new soar_module::boolean_param("db_mode", OM_Init_db_mode, new soar_module::f_predicate<boolean>());
    XML_mode = new soar_module::boolean_param("XML_mode", OM_Init_XML_mode, new soar_module::f_predicate<boolean>());
    callback_mode = new soar_module::boolean_param("db_mode", OM_Init_callback_mode, new soar_module::f_predicate<boolean>());
    stdout_mode = new soar_module::boolean_param("stdout_mode", OM_Init_stdout_mode, new soar_module::f_predicate<boolean>());
    file_mode = new soar_module::boolean_param("file_mode", OM_Init_file_mode, new soar_module::f_predicate<boolean>());
    db_dbg_mode = new soar_module::boolean_param("db_dbg_mode", OM_Init_db_dbg_mode, new soar_module::f_predicate<boolean>());
    XML_dbg_mode = new soar_module::boolean_param("XML_dbg_mode", OM_Init_XML_dbg_mode, new soar_module::f_predicate<boolean>());
    callback_dbg_mode = new soar_module::boolean_param("db_dbg_mode", OM_Init_callback_dbg_mode, new soar_module::f_predicate<boolean>());
    stdout_dbg_mode = new soar_module::boolean_param("stdout_dbg_mode", OM_Init_stdout_dbg_mode, new soar_module::f_predicate<boolean>());
    file_dbg_mode = new soar_module::boolean_param("file_dbg_mode", OM_Init_file_dbg_mode, new soar_module::f_predicate<boolean>());
    
    add(database);
    add(append_db);
    add(path);
    add(lazy_commit);
    add(page_size);
    add(cache_size);
    add(opt);
    add(db_mode);
    add(XML_mode);
    add(callback_mode);
    add(stdout_mode);
    add(file_mode);
    add(db_dbg_mode);
    add(XML_dbg_mode);
    add(callback_dbg_mode);
    add(stdout_dbg_mode);
    add(file_dbg_mode);
    
}
