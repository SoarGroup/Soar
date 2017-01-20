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
#include "print.h"
#include "soar_instance.h"

bool is_DT_mode_enabled(TraceMode mode) { return Output_Manager::Get_OM().is_trace_enabled(mode); }

AgentOutput_Info::AgentOutput_Info()
{
    print_enabled = true;
    printer_output_column = 1;
    for (int i=0; i < maxAgentTraces; ++i)
    {
        agent_traces_enabled[i] = true;
    }
    #ifndef SOAR_RELEASE_VERSION
        set_output_params_agent(true);
    #else
        set_output_params_agent(false);
    #endif
}

void AgentOutput_Info::set_output_params_agent(bool pDebugEnabled){
    if (Soar_Instance::Get_Soar_Instance().was_run_from_unit_test())
    {
        callback_mode = true;
        return;
    }
    callback_mode = !pDebugEnabled;
}


void Output_Manager::set_output_params_global(bool pDebugEnabled){
    if (pDebugEnabled && !(Soar_Instance::Get_Soar_Instance().was_run_from_unit_test()))
    {
        m_print_actual = true;
        m_print_identity = true;
        m_print_actual_effective = true;
        m_print_identity_effective = true;
        stdout_mode = true;
    } else {
        m_print_actual = true;
        m_print_identity = false;
        m_print_actual_effective = true;
        m_print_identity_effective = false;
        stdout_mode = false;
    }
}

void Output_Manager::set_output_mode(int modeIndex, bool pEnabled)
{
    mode_info[modeIndex].enabled = pEnabled;
    print_sf("Debug trace mode for '%s' is %s.\n", mode_info[modeIndex].prefix, (pEnabled ? "enabled" : "disabled"));
}

void Output_Manager::clear_output_modes()
{
    for (int i = 0; i < num_trace_modes; i++)
    {
        mode_info[i].enabled = false;
    }
}

void Output_Manager::copy_output_modes(trace_mode_info mode_info_src[num_trace_modes], trace_mode_info mode_info_dest[num_trace_modes])
{
    for (int i = 0; i < num_trace_modes; i++)
    {
        mode_info_dest[i].enabled = mode_info_src[i].enabled;
    }
}

void Output_Manager::cache_output_modes()
{
    copy_output_modes(mode_info, saved_mode_info);
}

void Output_Manager::restore_output_modes()
{
    copy_output_modes(saved_mode_info, mode_info);
}

void Output_Manager::print_output_modes(trace_mode_info mode_info_to_print[num_trace_modes])
{
    for (int i = 0; i < num_trace_modes; i++)
    {
        print_sf("%s: %s\n", mode_info_to_print[i].prefix, (mode_info_to_print[i].enabled ? "enabled" : "disabled"));
    }
}

void Output_Manager::init_Output_Manager(sml::Kernel* pKernel, Soar_Instance* pSoarInstance)
{
    m_Kernel = pKernel;
    m_Soar_Instance = pSoarInstance;
}

Output_Manager::Output_Manager()
{
    m_defaultAgent = NIL;
    m_params = new OM_Parameters(NULL, settings);
    m_pre_string = strdup("          ");
    m_post_string = NULL;

    reset_column_indents();

    initialize_debug_trace(mode_info);
    #ifndef SOAR_RELEASE_VERSION
        set_output_params_global(true);
        debug_set_mode_info(mode_info, true);
    #else
        set_output_params_global(false);
        debug_set_mode_info(mode_info, false);
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
                if (stdout_mode)
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
                if (stdout_mode)
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
