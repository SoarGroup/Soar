/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  output_manager.h
 *
 * =======================================================================
 */

#ifndef OUTPUT_MANAGER_H_
#define OUTPUT_MANAGER_H_

#include "kernel.h"
#include "lexer.h"
#include "soar_db.h"

#define num_output_strings 10

typedef struct trace_mode_info_struct {
    std::string *prefix;
    bool debug_enabled;
    bool trace_enabled;
} trace_mode_info;

namespace sml {
  class Kernel;
  class Agent;
}

namespace cli {
  class CommandLineInterface;
}

class Soar_Instance;
class OM_DB;
class OM_Parameters;

class AgentOutput_Info
{
    public:
        AgentOutput_Info();

        bool print_enabled, db_mode, callback_mode, file_mode;
        bool dprint_enabled, db_dbg_mode, callback_dbg_mode, stdout_dbg_mode, file_dbg_mode;
        int  printer_output_column;
} ;



class Output_Manager
{
    friend class OM_DB;
    friend class cli::CommandLineInterface;

  public:
    static Output_Manager& Get_OM()
    {
      static Output_Manager instance;
      return instance;
    }
    virtual ~Output_Manager();

    void init_Output_Manager(sml::Kernel *pKernel, Soar_Instance *pSoarInstance);

    trace_mode_info mode_info[num_trace_modes];

    void init();
    void fill_mode_info();
    bool debug_mode_enabled(TraceMode mode);
    void    set_default_agent(agent *pSoarAgent);
    agent* get_default_agent() {return m_defaultAgent;};
    void set_dprint_enabled (bool activate);

    /* Print functions that don't take an agent.  Will use default agent if necessary */
    void printv(const char *format, ...);
    void print_trace(const char *msg ) { print_agent(m_defaultAgent, msg); }
    void print_trace_prefix(const char *msg, TraceMode mode=No_Mode, bool no_prefix = false) { print_prefix_agent(m_defaultAgent, msg, mode, no_prefix);}
    void print_debug(const char *msg, TraceMode mode=No_Mode, bool no_prefix = false) { print_debug_agent(m_defaultAgent, msg, mode, no_prefix); }
    void print_db(MessageType msgType, TraceMode mode, const char *msg) { print_db_agent(m_defaultAgent, msgType, mode, msg); }

    void printv_agent(agent *pSoarAgent, const char *format, ...);
    void print_agent(agent *pSoarAgent, const char *msg );
    void print_prefix_agent(agent *pSoarAgent, const char *msg, TraceMode mode=No_Mode, bool no_prefix = false);
    void print_debug_agent(agent *pSoarAgent, const char *msg, TraceMode mode=No_Mode, bool no_prefix = false);
    void print_db_agent(agent *pSoarAgent, MessageType msgType, TraceMode mode, const char *msg);

    char * get_printed_output_string() {
      if (++next_output_string == num_output_strings)
        next_output_string = 0;
      return printed_output_strings[next_output_string];
    }

    int get_printer_output_column(agent* thisAgent=NULL) { return printer_output_column;}
    void set_printer_output_column(agent* thisAgent, int pOutputColumn=1) { printer_output_column = pOutputColumn;}
    void start_fresh_line(agent *pSoarAgent = NULL);

    char* NULL_SYM_STR;

  private:

    Output_Manager();

    /* The following two functions are declared but not implemented to avoid copies of singletons */
    Output_Manager(Output_Manager const&) {};
    void operator=(Output_Manager const&) {};

    Soar_Instance                           * m_Soar_Instance;
    sml::Kernel                             * m_Kernel;
    agent                                   * m_defaultAgent;
    OM_Parameters                           * m_params;
    OM_DB                                   * m_db;

    bool print_enabled, db_mode, stdout_mode, file_mode;
    bool dprint_enabled, db_dbg_mode, stdout_dbg_mode, file_dbg_mode;

    /* -- A quick replacement for Soar's printed_output_strings system.  Rather than have
     *    one string buffer, it rotates through 10 of them.  It allows us to have multiple
     *    function calls that use that buffer within one print statements.  There are
     *    probably better approaches, but this avoided revising a lot of other code and
     *    does the job.  -- */
    char    printed_output_strings[MAX_LEXEME_LENGTH*2+10][num_output_strings];
    int64_t next_output_string;

    int     printer_output_column;
    void    update_printer_columns(agent *pSoarAgent, bool update_global, const char *msg);

};

#endif /* OUTPUT_MANAGER_H_ */
