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
#include "debug_defines.h"
#include "output_manager.h"
#include "output_manager_db.h"
#include "output_manager_params.h"
#include "print.h"
#include "agent.h"

AgentOutput_Info::AgentOutput_Info() :
    print_enabled(OM_Init_print_enabled),
    dprint_enabled(OM_Init_dprint_enabled),
    db_mode(OM_Init_db_mode),
    callback_mode(OM_Init_callback_mode),
    file_mode(OM_Init_file_mode),
    db_dbg_mode(OM_Init_db_dbg_mode),
    callback_dbg_mode(OM_Init_callback_dbg_mode),
    file_dbg_mode(OM_Init_file_dbg_mode),
    printer_output_column(1)
{}

void Output_Manager::init_Output_Manager(sml::Kernel* pKernel, Soar_Instance* pSoarInstance)
{

    m_Kernel = pKernel;
    m_Soar_Instance = pSoarInstance;

    if (db_dbg_mode || db_mode)
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

    m_print_actual = true;
    m_print_original = false;
    m_print_identity = true;
    m_pre_string = NULL;
    m_post_string = NULL;

    next_output_string = 0;

    print_enabled = OM_Init_print_enabled;
    dprint_enabled = OM_Init_dprint_enabled;
    db_mode = OM_Init_db_mode;
    stdout_mode = OM_Init_stdout_mode;
    file_mode = OM_Init_file_mode;

    db_dbg_mode = OM_Init_db_dbg_mode;
    stdout_dbg_mode = OM_Init_stdout_dbg_mode;
    file_dbg_mode = OM_Init_file_dbg_mode;

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
        delete mode_info[i].prefix;
    }

    delete m_params;
    if (m_db)
    {
        delete m_db;
    }
}


void Output_Manager::set_default_agent(agent* pSoarAgent)
{
    std::string errString;
    if (!pSoarAgent)
    {
        errString = "OutputManager passed an empty default agent!\n";
        printa_prefix(DT_DEBUG, pSoarAgent, errString.c_str());
    }
    m_defaultAgent = pSoarAgent;
};

int Output_Manager::get_printer_output_column(agent* thisAgent)
{
    if (thisAgent)
    {
        return thisAgent->output_settings->printer_output_column;
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
    if (global_printer_output_column != 1)
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
            }
            if (stdout_mode)
            {
                global_printer_output_column = 1;
            }
        }
        else
        {
            if (pSoarAgent)
            {
                pSoarAgent->output_settings->printer_output_column++;
            }
            if (stdout_mode)
            {
                global_printer_output_column++;
            }
        }
    }
}

void Output_Manager::fill_mode_info()
{
    mode_info[No_Mode].prefix =                       new std::string("        ");
    mode_info[TM_EPMEM].prefix =                      new std::string("EpMem   ");
    mode_info[TM_SMEM].prefix =                       new std::string("SMem    ");
    mode_info[TM_LEARNING].prefix =                   new std::string("Learn   ");
    mode_info[TM_CHUNKING].prefix =                   new std::string("Chunk   ");
    mode_info[TM_RL].prefix =                         new std::string("RL      ");
    mode_info[TM_WMA].prefix =                        new std::string("WMA     ");

    mode_info[DT_DEBUG].prefix =                      new std::string("Debug   ");
    mode_info[DT_ID_LEAKING].prefix =                 new std::string("ID Leak ");
    mode_info[DT_LHS_VARIABLIZATION].prefix =         new std::string("VrblzLHS");
    mode_info[DT_ADD_CONSTRAINTS_ORIG_TESTS].prefix = new std::string("Add Orig");
    mode_info[DT_RHS_VARIABLIZATION].prefix =         new std::string("VrblzRHS");
    mode_info[DT_PRINT_INSTANTIATIONS].prefix =       new std::string("PrntInst");
    mode_info[DT_ADD_TEST_TO_TEST].prefix =           new std::string("Add Test");
    mode_info[DT_DEALLOCATES].prefix =                new std::string("Memory  ");
    mode_info[DT_DEALLOCATE_SYMBOLS].prefix =         new std::string("Memory  ");
    mode_info[DT_REFCOUNT_ADDS].prefix =              new std::string("RefCnt  ");
    mode_info[DT_REFCOUNT_REMS].prefix =              new std::string("RefCnt  ");
    mode_info[DT_VARIABLIZATION_MANAGER].prefix =     new std::string("VrblzMgr");
    mode_info[DT_PARSER].prefix =                     new std::string("Parser  ");
    mode_info[DT_FUNC_PRODUCTIONS].prefix =           new std::string("FuncCall");
    mode_info[DT_OVAR_MAPPINGS].prefix =              new std::string("OVar Map");
    mode_info[DT_REORDERER].prefix =                  new std::string("Reorder ");
    mode_info[DT_BACKTRACE].prefix =                  new std::string("BackTrce");
    mode_info[DT_SAVEDVARS].prefix =                  new std::string("SavedVar");
    mode_info[DT_GDS].prefix =                        new std::string("GDS     ");
    mode_info[DT_RL_VARIABLIZATION].prefix =          new std::string("Vrblz RL");
    mode_info[DT_NCC_VARIABLIZATION].prefix =         new std::string("VrblzNCC");
    mode_info[DT_IDENTITY_PROP].prefix =              new std::string("ID Prop ");
    mode_info[DT_SOAR_INSTANCE].prefix =              new std::string("SoarInst");
    mode_info[DT_CLI_LIBRARIES].prefix =              new std::string("CLI Lib ");
    mode_info[DT_CONSTRAINTS].prefix =                new std::string("Cnstrnts");
    mode_info[DT_MERGE].prefix =                      new std::string("Merge Cs");
    mode_info[DT_FIX_CONDITIONS].prefix =             new std::string("Fix Cond");
    mode_info[DT_EPMEM_CMD].prefix =                  new std::string("EpMem Go");

    mode_info[No_Mode].enabled =                      TRACE_Init_No_Mode;
    mode_info[TM_EPMEM].enabled =                     TRACE_Init_TM_EPMEM;
    mode_info[TM_SMEM].enabled =                      TRACE_Init_TM_SMEM;
    mode_info[TM_LEARNING].enabled =                  TRACE_Init_TM_LEARNING;
    mode_info[TM_CHUNKING].enabled =                  TRACE_Init_TM_CHUNKING;
    mode_info[TM_RL].enabled =                        TRACE_Init_TM_RL;
    mode_info[TM_WMA].enabled =                       TRACE_Init_TM_WMA;

    mode_info[No_Mode].enabled =                        TRACE_Init_DT_No_Mode;
    mode_info[DT_DEBUG].enabled =                       TRACE_Init_DT_DEBUG;
    mode_info[DT_ID_LEAKING].enabled =                  TRACE_Init_DT_ID_LEAKING;
    mode_info[DT_LHS_VARIABLIZATION].enabled =          TRACE_Init_DT_LHS_VARIABLIZATION;
    mode_info[DT_ADD_CONSTRAINTS_ORIG_TESTS].enabled =  TRACE_Init_DT_ADD_CONSTRAINTS_ORIG_TESTS;
    mode_info[DT_RHS_VARIABLIZATION].enabled =          TRACE_Init_DT_RHS_VARIABLIZATION;
    mode_info[DT_PRINT_INSTANTIATIONS].enabled =        TRACE_Init_DT_PRINT_INSTANTIATIONS;
    mode_info[DT_ADD_TEST_TO_TEST].enabled =            TRACE_Init_DT_ADD_TEST_TO_TEST;
    mode_info[DT_DEALLOCATES].enabled =                 TRACE_Init_DT_DEALLOCATES;
    mode_info[DT_DEALLOCATE_SYMBOLS].enabled =          TRACE_Init_DT_DEALLOCATE_SYMBOLS;
    mode_info[DT_REFCOUNT_ADDS].enabled =               TRACE_Init_DT_REFCOUNT_ADDS;
    mode_info[DT_REFCOUNT_REMS].enabled =               TRACE_Init_DT_REFCOUNT_REMS;
    mode_info[DT_VARIABLIZATION_MANAGER].enabled =      TRACE_Init_DT_VARIABLIZATION_MANAGER;
    mode_info[DT_PARSER].enabled =                      TRACE_Init_DT_PARSER;
    mode_info[DT_FUNC_PRODUCTIONS].enabled =            TRACE_Init_DT_FUNC_PRODUCTIONS;
    mode_info[DT_OVAR_MAPPINGS].enabled =               TRACE_Init_DT_OVAR_MAPPINGS;
    mode_info[DT_REORDERER].enabled =                   TRACE_Init_DT_REORDERER;
    mode_info[DT_BACKTRACE].enabled =                   TRACE_Init_DT_BACKTRACE;
    mode_info[DT_SAVEDVARS].enabled =                   TRACE_Init_DT_SAVEDVARS;
    mode_info[DT_GDS].enabled =                         TRACE_Init_DT_GDS;
    mode_info[DT_RL_VARIABLIZATION].enabled =           TRACE_Init_DT_RL_VARIABLIZATION;
    mode_info[DT_NCC_VARIABLIZATION].enabled =          TRACE_Init_DT_NCC_VARIABLIZATION;
    mode_info[DT_IDENTITY_PROP].enabled =               TRACE_Init_DT_IDENTITY_PROP;
    mode_info[DT_CONSTRAINTS].enabled =                 TRACE_Init_DT_CONSTRAINTS;
    mode_info[DT_MERGE].enabled =                       TRACE_Init_DT_MERGE;
    mode_info[DT_FIX_CONDITIONS].enabled =              TRACE_Init_DT_FIX_CONDITIONS;
    mode_info[DT_EPMEM_CMD].enabled =                   TRACE_Init_DT_EPMEM_CMD;

}


