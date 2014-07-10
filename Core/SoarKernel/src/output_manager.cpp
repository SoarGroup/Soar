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

void Output_Manager::init_Output_Manager(sml::Kernel *pKernel, Soar_Instance *pSoarInstance)
{

  m_Kernel = pKernel;
  m_Soar_Instance = pSoarInstance;

  if (db_dbg_mode || db_mode) {
    soar_module::sqlite_database *new_db = new soar_module::sqlite_database();
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

bool Output_Manager::debug_mode_enabled(TraceMode mode)
{
    return mode_info[mode].debug_enabled;
}

void Output_Manager::set_default_agent(agent *pSoarAgent) {
  std::string errString;
  if (!pSoarAgent)
  {
    errString = "OutputManager passed an empty default agent!\n";
    print_debug_agent(pSoarAgent, errString.c_str(), DT_DEBUG);
  }
  m_defaultAgent = pSoarAgent;
};

void Output_Manager::set_dprint_enabled (bool activate)
{
  //printv("Debug printing %s.\n", (activate ? "enabled" : "disabled"));
  dprint_enabled = activate;
}


Output_Manager::~Output_Manager()
{
  free(NULL_SYM_STR);

  for(int i=0;i<num_trace_modes;i++)
  {
    delete mode_info[i].prefix;
  }

  delete m_params;
  if (m_db)
    delete m_db;
}

void Output_Manager::start_fresh_line(agent *pSoarAgent)
{
    if (!pSoarAgent)
        pSoarAgent = m_defaultAgent;
    if (printer_output_column != 1)
        print_agent(pSoarAgent, "\n");
}

void Output_Manager::update_printer_columns(agent *pSoarAgent, bool update_global, const char *msg)
{
  const char *ch;

  if (pSoarAgent)
  {
      for (ch=msg; *ch!=0; ch++) {
          if (*ch=='\n')
              pSoarAgent->output_settings->printer_output_column = 1;
          else
              pSoarAgent->output_settings->printer_output_column++;
      }
  }
  if (update_global)
  {
      for (ch=msg; *ch!=0; ch++) {
          if (*ch=='\n')
              printer_output_column = 1;
          else
              printer_output_column++;
      }
  }
}
void Output_Manager::print_db_agent(agent *pSoarAgent, MessageType msgType, TraceMode mode, const char *msg) {
  soar_module::sqlite_statement   *target_statement= NIL;

  if (((msgType == trace_msg) && mode_info[mode].trace_enabled) ||
      ((msgType == debug_msg) && mode_info[mode].debug_enabled))
  {
    m_db->print_db(msgType, mode_info[mode].prefix->c_str(), msg);
  }
}

void Output_Manager::printv(const char *format, ...)
{
    if (m_defaultAgent)
    {
        va_list args;
        char buf[PRINT_BUFSIZE];

        va_start (args, format);
        vsprintf (buf, format, args);
        va_end (args);
        print_agent(m_defaultAgent, buf);
    }
}

void Output_Manager::printv_agent(agent *pSoarAgent, const char *format, ...) {
  va_list args;
  char buf[PRINT_BUFSIZE];

  va_start (args, format);
  vsprintf (buf, format, args);
  va_end (args);
  print_agent(pSoarAgent, buf);
}

void Output_Manager::print_agent (agent *pSoarAgent, const char *msg)
{
    if (print_enabled)
    {
        if (pSoarAgent && pSoarAgent->output_settings->callback_mode && pSoarAgent->output_settings->print_enabled) {
            soar_invoke_callbacks(pSoarAgent, PRINT_CALLBACK, static_cast<soar_call_data>(const_cast<char *>(msg)));
        }

        if (stdout_mode) {
            fputs(msg, stdout);
        }

    }

    update_printer_columns(pSoarAgent, stdout_mode, msg);

    if (db_mode) {
        m_db->print_db (trace_msg, mode_info[No_Mode].prefix->c_str(), msg);
    }
}

void Output_Manager::print_debug_agent (agent *pSoarAgent, const char *msg, TraceMode mode, bool no_prefix)
{
  bool printer_column_updated = false;
  std::string newTrace;

  if (mode_info[mode].debug_enabled && dprint_enabled) {
    if (!no_prefix)
    {
      newTrace.assign(mode_info[mode].prefix->c_str());
      newTrace.append("| ");
      newTrace.append(msg);
    } else {
      newTrace.assign(msg);
    }
    if (dprint_enabled)
    {
        if (pSoarAgent && pSoarAgent->output_settings->callback_mode && pSoarAgent->output_settings->dprint_enabled) {
            soar_invoke_callbacks(pSoarAgent, PRINT_CALLBACK, static_cast<soar_call_data>(const_cast<char *>(newTrace.c_str())));
        }

        if (stdout_mode) {
            fputs(newTrace.c_str(), stdout);
        }

    }

    update_printer_columns(pSoarAgent, stdout_mode, msg);

    if (db_mode) {
        m_db->print_db (debug_msg, mode_info[mode].prefix->c_str(), msg);
    }
  }
}

void Output_Manager::print_prefix_agent (agent *pSoarAgent, const char *msg, TraceMode mode, bool no_prefix)
{
  bool printer_column_updated = false;

  std::string newTrace;

  if (mode_info[mode].trace_enabled) {
    if (!no_prefix)
    {
      newTrace.assign(mode_info[mode].prefix->c_str());
      newTrace.append("| ");
      newTrace.append(msg);
    } else {
      newTrace.assign(msg);
    }

    print_agent(pSoarAgent, newTrace.c_str());
  }
}

void Output_Manager::fill_mode_info()
{
  mode_info[No_Mode].prefix =                   new std::string("      ");
  mode_info[TM_EPMEM].prefix =                  new std::string("EpMem ");
  mode_info[TM_SMEM].prefix =                   new std::string("SMem  ");
  mode_info[TM_LEARNING].prefix =               new std::string("Learn ");
  mode_info[TM_CHUNKING].prefix =               new std::string("Chunk ");
  mode_info[TM_RL].prefix =                     new std::string("RL    ");
  mode_info[TM_WMA].prefix =                    new std::string("WMA   ");

  mode_info[DT_DEBUG].prefix =                      new std::string("Debug ");
  mode_info[DT_ID_LEAKING].prefix =                 new std::string("ID Leak ");
  mode_info[DT_LHS_VARIABLIZATION].prefix =         new std::string("VrblzLHS");
  mode_info[DT_ADD_CONSTRAINTS_ORIG_TESTS].prefix = new std::string("Add Orig");
  mode_info[DT_RHS_VARIABLIZATION].prefix =         new std::string("VrblzRHS");
  mode_info[DT_PRINT_INSTANTIATIONS].prefix =       new std::string("PrntInst");
  mode_info[DT_ADD_TEST_TO_TEST].prefix =           new std::string("Add Test");
  mode_info[DT_DEALLOCATES].prefix =                new std::string("Memory");
  mode_info[DT_DEALLOCATE_SYMBOLS].prefix =         new std::string("Memory");
  mode_info[DT_REFCOUNT_ADDS].prefix =              new std::string("RefCnt");
  mode_info[DT_REFCOUNT_REMS].prefix =              new std::string("RefCnt");
  mode_info[DT_VARIABLIZATION_MANAGER].prefix =     new std::string("VrblzMgr");
  mode_info[DT_PARSER].prefix =                     new std::string("Parser");
  mode_info[DT_FUNC_PRODUCTIONS].prefix =           new std::string("FuncCall");
  mode_info[DT_OVAR_MAPPINGS].prefix =              new std::string("OVar Map");
  mode_info[DT_REORDERER].prefix =                  new std::string("Reorder ");
  mode_info[DT_BACKTRACE].prefix =                  new std::string("BackTrce");
  mode_info[DT_SAVEDVARS].prefix =                  new std::string("SavedVar");
  mode_info[DT_GDS].prefix =                        new std::string("GDS   ");
  mode_info[DT_RL_VARIABLIZATION].prefix =          new std::string("Vrblz RL");
  mode_info[DT_NCC_VARIABLIZATION].prefix =         new std::string("VrblzNCC");
  mode_info[DT_IDENTITY_PROP].prefix =              new std::string("IDProp");
  mode_info[DT_SOAR_INSTANCE].prefix =              new std::string("SoarInst");
  mode_info[DT_CLI_LIBRARIES].prefix =              new std::string("CLILib");
  mode_info[DT_CONSTRAINTS].prefix =                new std::string("Cnstrnts");
  mode_info[DT_MERGE].prefix =                      new std::string("Merge Cs");
  mode_info[DT_FIX_CONDITIONS].prefix =             new std::string("Fix Cond");

  mode_info[No_Mode].trace_enabled =                      TRACE_Init_No_Mode;
  mode_info[TM_EPMEM].trace_enabled =                     TRACE_Init_TM_EPMEM;
  mode_info[TM_SMEM].trace_enabled =                      TRACE_Init_TM_SMEM;
  mode_info[TM_LEARNING].trace_enabled =                  TRACE_Init_TM_LEARNING;
  mode_info[TM_CHUNKING].trace_enabled =                  TRACE_Init_TM_CHUNKING;
  mode_info[TM_RL].trace_enabled =                        TRACE_Init_TM_RL;
  mode_info[TM_WMA].trace_enabled =                       TRACE_Init_TM_WMA;

  mode_info[No_Mode].debug_enabled =                        TRACE_Init_DT_No_Mode;
  mode_info[DT_DEBUG].debug_enabled =                       TRACE_Init_DT_DEBUG;
  mode_info[DT_ID_LEAKING].debug_enabled =                  TRACE_Init_DT_ID_LEAKING;
  mode_info[DT_LHS_VARIABLIZATION].debug_enabled =          TRACE_Init_DT_LHS_VARIABLIZATION;
  mode_info[DT_ADD_CONSTRAINTS_ORIG_TESTS].debug_enabled =  TRACE_Init_DT_ADD_CONSTRAINTS_ORIG_TESTS;
  mode_info[DT_RHS_VARIABLIZATION].debug_enabled =          TRACE_Init_DT_RHS_VARIABLIZATION;
  mode_info[DT_PRINT_INSTANTIATIONS].debug_enabled =        TRACE_Init_DT_PRINT_INSTANTIATIONS;
  mode_info[DT_ADD_TEST_TO_TEST].debug_enabled =            TRACE_Init_DT_ADD_TEST_TO_TEST;
  mode_info[DT_DEALLOCATES].debug_enabled =                 TRACE_Init_DT_DEALLOCATES;
  mode_info[DT_DEALLOCATE_SYMBOLS].debug_enabled =          TRACE_Init_DT_DEALLOCATE_SYMBOLS;
  mode_info[DT_REFCOUNT_ADDS].debug_enabled =               TRACE_Init_DT_REFCOUNT_ADDS;
  mode_info[DT_REFCOUNT_REMS].debug_enabled =               TRACE_Init_DT_REFCOUNT_REMS;
  mode_info[DT_VARIABLIZATION_MANAGER].debug_enabled =      TRACE_Init_DT_VARIABLIZATION_MANAGER;
  mode_info[DT_PARSER].debug_enabled =                      TRACE_Init_DT_PARSER;
  mode_info[DT_FUNC_PRODUCTIONS].debug_enabled =            TRACE_Init_DT_FUNC_PRODUCTIONS;
  mode_info[DT_OVAR_MAPPINGS].debug_enabled =               TRACE_Init_DT_OVAR_MAPPINGS;
  mode_info[DT_REORDERER].debug_enabled =                   TRACE_Init_DT_REORDERER;
  mode_info[DT_BACKTRACE].debug_enabled =                   TRACE_Init_DT_BACKTRACE;
  mode_info[DT_SAVEDVARS].debug_enabled =                   TRACE_Init_DT_SAVEDVARS;
  mode_info[DT_GDS].debug_enabled =                         TRACE_Init_DT_GDS;
  mode_info[DT_RL_VARIABLIZATION].debug_enabled =           TRACE_Init_DT_RL_VARIABLIZATION;
  mode_info[DT_NCC_VARIABLIZATION].debug_enabled =          TRACE_Init_DT_NCC_VARIABLIZATION;
  mode_info[DT_IDENTITY_PROP].debug_enabled =               TRACE_Init_DT_IDENTITY_PROP;
  mode_info[DT_CONSTRAINTS].debug_enabled =                 TRACE_Init_DT_CONSTRAINTS;
  mode_info[DT_MERGE].debug_enabled =                       TRACE_Init_DT_MERGE;
  mode_info[DT_FIX_CONDITIONS].debug_enabled =              TRACE_Init_DT_FIX_CONDITIONS;

}


