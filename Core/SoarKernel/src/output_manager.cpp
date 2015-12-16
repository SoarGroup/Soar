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

#include "callback.h"
#include "debug.h"
#include "output_manager.h"
#include "output_manager_db.h"
#include "output_manager_params.h"
#include "print.h"
#include "agent.h"

AgentOutput_Info::AgentOutput_Info() :
    print_enabled(OM_Init_print_enabled),
    callback_mode(OM_Init_callback_mode),
    stdout_mode(OM_Init_stdout_mode),
    db_mode(OM_Init_db_mode),
    callback_dbg_mode(OM_Init_callback_dbg_mode),
    stdout_dbg_mode(OM_Init_stdout_dbg_mode),
    db_dbg_mode(OM_Init_db_dbg_mode),
    printer_output_column(1)
{}

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
    initialize_debug_trace(mode_info);

    m_defaultAgent = NIL;
    m_params = new OM_Parameters();
    m_db = NIL;

    m_print_actual = OM_Default_print_actual;
    m_print_identity = OM_Default_print_identity;
    m_print_actual_effective = OM_Default_print_actual;
    m_print_identity_effective = OM_Default_print_identity;
    m_pre_string = strdup("          ");
    m_post_string = NULL;

    next_output_string = 0;

    db_mode = OM_Init_db_mode;
    stdout_mode = OM_Init_stdout_mode;

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


