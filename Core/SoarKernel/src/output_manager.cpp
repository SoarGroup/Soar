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
#include "debug_defines.h"
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
    fill_mode_info();

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

void Output_Manager::fill_mode_info()
{
    mode_info[TM_EPMEM].prefix =                        strdup("EpMem   | ");
    mode_info[TM_SMEM].prefix =                         strdup("SMem    | ");
    mode_info[TM_LEARNING].prefix =                     strdup("Learn   | ");
    mode_info[TM_CHUNKING].prefix =                     strdup("Chunk   | ");
    mode_info[TM_RL].prefix =                           strdup("RL      | ");
    mode_info[TM_WMA].prefix =                          strdup("WMA     | ");

    mode_info[No_Mode].prefix =                         strdup("        | ");
    mode_info[DT_DEBUG].prefix =                        strdup("Debug   | ");

    mode_info[DT_ID_LEAKING].prefix =                   strdup("ID Leak | ");
    mode_info[DT_DEALLOCATES].prefix =                  strdup("Memory  | ");
    mode_info[DT_DEALLOCATE_SYMBOLS].prefix =           strdup("Memory  | ");
    mode_info[DT_REFCOUNT_ADDS].prefix =                strdup("RefCnt  | ");
    mode_info[DT_REFCOUNT_REMS].prefix =                strdup("RefCnt  | ");

    mode_info[DT_SOAR_INSTANCE].prefix =                strdup("SoarInst| ");
    mode_info[DT_CLI_LIBRARIES].prefix =                strdup("CLI Lib | ");
    mode_info[DT_PARSER].prefix =                       strdup("Parser  | ");
    mode_info[DT_GDS].prefix =                          strdup("GDS     | ");
    mode_info[DT_EPMEM_CMD].prefix =                    strdup("EpMem Go| ");

    mode_info[DT_PRINT_INSTANTIATIONS].prefix =         strdup("PrntInst| ");
    mode_info[DT_MILESTONES].prefix =                   strdup("Milestne| ");

    mode_info[DT_ADD_ADDITIONALS].prefix =              strdup("AddAddtn| ");
    mode_info[DT_VARIABLIZATION_MANAGER].prefix =       strdup("VrblzMgr| ");
    mode_info[DT_VM_MAPS].prefix =                      strdup("VM Maps | ");
    mode_info[DT_BACKTRACE].prefix =                    strdup("BackTrce| ");
    mode_info[DT_IDENTITY_PROP].prefix =                strdup("ID Prop | ");
    mode_info[DT_UNIFICATION].prefix =                  strdup("Unify   | ");
    mode_info[DT_CONSTRAINTS].prefix =                  strdup("Cnstrnts| ");
    mode_info[DT_LHS_VARIABLIZATION].prefix =           strdup("VrblzLHS| ");
    mode_info[DT_RHS_VARIABLIZATION].prefix =           strdup("VrblzRHS| ");
    mode_info[DT_RL_VARIABLIZATION].prefix =            strdup("Vrblz RL| ");
    mode_info[DT_NCC_VARIABLIZATION].prefix =           strdup("VrblzNCC| ");
    mode_info[DT_UNGROUNDED_STI].prefix =               strdup("UngrnSTI| ");
    mode_info[DT_REORDERER].prefix =                    strdup("Reorder | ");
    mode_info[DT_MERGE].prefix =                        strdup("Merge Cs| ");
    mode_info[DT_BUILD_CHUNK_CONDS].prefix =            strdup("BChnkCnd| ");
    mode_info[DT_EBC_CLEANUP].prefix =                  strdup("CleanUp | ");
    mode_info[DT_RHS_VALUE].prefix =                    strdup("RHSValue| ");

    mode_info[DT_NONE_2].prefix =                       strdup("| ");
    mode_info[DT_NONE_3].prefix =                       strdup("| ");
    mode_info[DT_NONE_4].prefix =                       strdup("| ");

    mode_info[TM_EPMEM].enabled =                       TRACE_Init_TM_EPMEM;
    mode_info[TM_SMEM].enabled =                        TRACE_Init_TM_SMEM;
    mode_info[TM_LEARNING].enabled =                    TRACE_Init_TM_LEARNING;
    mode_info[TM_CHUNKING].enabled =                    TRACE_Init_TM_CHUNKING;
    mode_info[TM_RL].enabled =                          TRACE_Init_TM_RL;
    mode_info[TM_WMA].enabled =                         TRACE_Init_TM_WMA;

    mode_info[No_Mode].enabled =                        TRACE_Init_No_Mode;
    mode_info[DT_DEBUG].enabled =                       TRACE_Init_DT_DEBUG;

    mode_info[DT_ID_LEAKING].enabled =                  TRACE_Init_DT_ID_LEAKING;
    mode_info[DT_DEALLOCATES].enabled =                 TRACE_Init_DT_DEALLOCATES;
    mode_info[DT_DEALLOCATE_SYMBOLS].enabled =          TRACE_Init_DT_DEALLOCATE_SYMBOLS;
    mode_info[DT_REFCOUNT_ADDS].enabled =               TRACE_Init_DT_REFCOUNT_ADDS;
    mode_info[DT_REFCOUNT_REMS].enabled =               TRACE_Init_DT_REFCOUNT_REMS;

    mode_info[DT_SOAR_INSTANCE].enabled =               TRACE_Init_DT_SOAR_INSTANCE;
    mode_info[DT_CLI_LIBRARIES].enabled =               TRACE_Init_DT_CLI_LIBRARIES;
    mode_info[DT_PARSER].enabled =                      TRACE_Init_DT_PARSER;
    mode_info[DT_GDS].enabled =                         TRACE_Init_DT_GDS;
    mode_info[DT_EPMEM_CMD].enabled =                   TRACE_Init_DT_EPMEM_CMD;

    mode_info[DT_PRINT_INSTANTIATIONS].enabled =        TRACE_Init_DT_PRINT_INSTANTIATIONS;
    mode_info[DT_MILESTONES].enabled =                  TRACE_Init_DT_MILESTONES;

    mode_info[DT_ADD_ADDITIONALS].enabled =             TRACE_Init_DT_ADD_ADDITIONALS;
    mode_info[DT_VARIABLIZATION_MANAGER].enabled =      TRACE_Init_DT_VARIABLIZATION_MANAGER;
    mode_info[DT_VM_MAPS].enabled =                     TRACE_Init_DT_VM_MAPS;
    mode_info[DT_BACKTRACE].enabled =                   TRACE_Init_DT_BACKTRACE;
    mode_info[DT_IDENTITY_PROP].enabled =               TRACE_Init_DT_IDENTITY_PROP;
    mode_info[DT_UNIFICATION].enabled =                 TRACE_Init_DT_UNIFICATION;
    mode_info[DT_CONSTRAINTS].enabled =                 TRACE_Init_DT_CONSTRAINTS;
    mode_info[DT_LHS_VARIABLIZATION].enabled =          TRACE_Init_DT_LHS_VARIABLIZATION;
    mode_info[DT_RHS_VARIABLIZATION].enabled =          TRACE_Init_DT_RHS_VARIABLIZATION;
    mode_info[DT_RL_VARIABLIZATION].enabled =           TRACE_Init_DT_RL_VARIABLIZATION;
    mode_info[DT_NCC_VARIABLIZATION].enabled =          TRACE_Init_DT_NCC_VARIABLIZATION;
    mode_info[DT_UNGROUNDED_STI].enabled =              TRACE_Init_DT_UNGROUNDED_STI;
    mode_info[DT_REORDERER].enabled =                   TRACE_Init_DT_REORDERER;
    mode_info[DT_MERGE].enabled =                       TRACE_Init_DT_MERGE;
    mode_info[DT_BUILD_CHUNK_CONDS].enabled =           TRACE_Init_DT_BUILD_CHUNK_CONDS;
    mode_info[DT_EBC_CLEANUP].enabled =                 TRACE_Init_DT_EBC_CLEANUP;
    mode_info[DT_RHS_VALUE].enabled =                   TRACE_Init_DT_RHS_VALUE;

    mode_info[DT_NONE_2].enabled =                      TRACE_Init_DT_NONE_2;
    mode_info[DT_NONE_3].enabled =                      TRACE_Init_DT_NONE_3;
    mode_info[DT_NONE_4].enabled =                      TRACE_Init_DT_NONE_4;
}


