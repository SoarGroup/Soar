/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  output_manager.cpp
 *
 * =======================================================================
 */

#include "output_manager.h"

#include "agent.h"
#include "callback.h"
#include "debug.h"
#include "dprint.h"
#include "output_settings.h"
#include "output_db.h"
#include "print.h"
#include "soar_instance.h"

bool is_DT_mode_enabled(TraceMode mode) { return Output_Manager::Get_OM().is_debug_mode_enabled(mode); }

AgentOutput_Info::AgentOutput_Info()
{
    print_enabled = true;
    printer_output_column = 1;
    #if defined(DEBUG_OUTPUT_ON)
        set_output_params_agent(true);
    #else
        set_output_params_agent(false);
    #endif
}

void AgentOutput_Info::set_output_params_agent(bool pDebugEnabled){
    if (pDebugEnabled && !(Soar_Instance::Get_Soar_Instance().was_run_from_unit_test()))
    {
        callback_mode = false;
        stdout_mode = true;
        db_mode = false;
        callback_dbg_mode = false;
        stdout_dbg_mode = true;
        db_dbg_mode = false;
    } else {
        callback_mode = true;
        stdout_mode = false;
        db_mode = false;
        callback_dbg_mode = false;
        stdout_dbg_mode = false;
        db_dbg_mode = false;
    }
}


void Output_Manager::set_output_params_global(bool pDebugEnabled){
    if (pDebugEnabled && !(Soar_Instance::Get_Soar_Instance().was_run_from_unit_test()))
    {
        m_print_actual = true;
        m_print_identity = true;
        m_print_actual_effective = true;
        m_print_identity_effective = true;
        db_mode = false;
        stdout_mode = true;
        debug_set_mode_info(mode_info, true);
    } else {
        m_print_actual = true;
        m_print_identity = false;
        m_print_actual_effective = true;
        m_print_identity_effective = false;
        db_mode = false;
        stdout_mode = false;
        debug_set_mode_info(mode_info, false);
    }
}

void Output_Manager::set_output_mode(int modeIndex, bool pEnabled)
{
    mode_info[modeIndex].enabled = pEnabled;
    print_sf("Debug trace mode for '%s' is %s.\n", mode_info[modeIndex].prefix, (pEnabled ? "enabled" : "disabled"));
}

void Output_Manager::init_Output_Manager(sml::Kernel* pKernel, Soar_Instance* pSoarInstance)
{

    m_Kernel = pKernel;
    m_Soar_Instance = pSoarInstance;

    if (db_mode)
    {
        soar_module::sqlite_database* new_db = new soar_module::sqlite_database();
        m_db = new OM_DB(new_db);
        m_db->create_db();
    }
}

Output_Manager::Output_Manager()
{
    m_defaultAgent = NIL;
    m_params = new OM_Parameters();
    m_db = NIL;
    m_pre_string = strdup("          ");
    m_post_string = NULL;

    next_output_string = 0;
    reset_column_indents();

    initialize_debug_trace(mode_info);
    #if defined(DEBUG_OUTPUT_ON)
        set_output_params_global(true);
    #else
        set_output_params_global(false);
    #endif

    /* -- This is a string used when trying to print a null symbol.  Not sure if this is the best
     *    place to put it.  Leaving here for now. -- */
    NULL_SYM_STR = strdup("NULL");

}

Output_Manager::~Output_Manager()
{
    free(NULL_SYM_STR);
    if (m_pre_string) free(m_pre_string);
    if (m_post_string) free(m_post_string);

    for (int i = 0; i < num_trace_modes; i++)
    {
        free(mode_info[i].prefix);
    }

    delete m_params;
    if (m_db)
    {
        delete m_db;
    }
}


int Output_Manager::get_printer_output_column(agent* pSoarAgent)
{
    if (pSoarAgent)
    {
        return pSoarAgent->output_settings->printer_output_column;
    }
    else
    {
        return global_printer_output_column;
    }
}

void Output_Manager::set_printer_output_column(agent* thisAgent, int pOutputColumn)
{
    if (thisAgent)
    {
        thisAgent->output_settings->printer_output_column = pOutputColumn;
    }
    else
    {
        global_printer_output_column = pOutputColumn;
    }
}

void Output_Manager::start_fresh_line(agent* pSoarAgent)
{
    if (!pSoarAgent)
    {
        pSoarAgent = m_defaultAgent;
    }
    if ((global_printer_output_column != 1) || (pSoarAgent->output_settings->printer_output_column != 1))
    {
        printa(pSoarAgent, "\n");
    }
}

void Output_Manager::update_printer_columns(agent* pSoarAgent, const char* msg)
{
    const char* ch;

    for (ch = msg; *ch != 0; ch++)
    {
        if (*ch == '\n')
        {
            if (pSoarAgent)
            {
                pSoarAgent->output_settings->printer_output_column = 1;
                if (pSoarAgent->output_settings->stdout_mode)
                {
                    global_printer_output_column = 1;
                }
            } else if (stdout_mode)
            {
                global_printer_output_column = 1;
            }
        }
        else
        {
            if (pSoarAgent)
            {
                pSoarAgent->output_settings->printer_output_column++;
                if (pSoarAgent->output_settings->stdout_mode)
                {
                    global_printer_output_column++;
                }
            } else if (stdout_mode)
            {
                global_printer_output_column++;
            }
        }
    }
}

void Output_Manager::store_refcount(Symbol sym, const char* callers, bool isAdd)
{
    m_db->store_refcount(sym, callers, isAdd);
}

OM_Parameters::OM_Parameters(): soar_module::param_container(NULL)
{
    database = new soar_module::constant_param<soar_module::db_choices>("database", soar_module::file, new soar_module::f_predicate<soar_module::db_choices>());
    database->add_mapping(soar_module::memory, "memory");
    database->add_mapping(soar_module::file, "file");
    append_db = new soar_module::boolean_param("append", off, new soar_module::f_predicate<boolean>());
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

    add(database);
    add(append_db);
    add(path);
    add(lazy_commit);
    add(page_size);
    add(cache_size);
    add(opt);
}
