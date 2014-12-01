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

typedef char* rhs_value;
typedef struct test_struct test_info;
typedef test_info* test;
typedef struct condition_struct condition;
typedef struct action_struct action;
typedef struct production_struct production;
typedef struct saved_test_struct saved_test;
typedef char varnames;
typedef struct node_varnames_struct node_varnames;
typedef struct identity_struct identity_info;

// TODO: this isn't good enough. Arbitrary length should be acceptable.
#define MAX_LEXER_LINE_LENGTH 1000
// a little bigger to avoid any off-by-one-errors
#define MAX_LEXEME_LENGTH (MAX_LEXER_LINE_LENGTH+5)

#define output_string_size MAX_LEXEME_LENGTH*2+10
#define num_output_strings 10

typedef struct trace_mode_info_struct
{
        std::string* prefix;
        bool enabled;
} trace_mode_info;

namespace sml
{
    class Kernel;
    class Agent;
}

namespace cli
{
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

    private:

        Output_Manager();

        /* The following two functions are declared but not implemented to avoid copies of singletons */
        Output_Manager(Output_Manager const&) {};
        void operator=(Output_Manager const&) {};

        Soar_Instance*                            m_Soar_Instance;
        sml::Kernel*                              m_Kernel;
        agent*                                    m_defaultAgent;
        OM_Parameters*                            m_params;
        OM_DB*                                    m_db;

        bool print_enabled, db_mode, stdout_mode, file_mode;
        bool dprint_enabled, db_dbg_mode, stdout_dbg_mode, file_dbg_mode;

        bool m_print_actual, m_print_original, m_print_identity;
        char* m_pre_string, *m_post_string;

        /* -- A quick replacement for Soar's printed_output_strings system.  Rather than have
         *    one string buffer, it rotates through 10 of them.  It allows us to have multiple
         *    function calls that use that buffer within one print statements.  There are
         *    probably better approaches, but this avoided revising a lot of other code and
         *    does the job.  -- */
        char    printed_output_strings[output_string_size][num_output_strings];
        int64_t                         next_output_string;

        int     global_printer_output_column;
        void    update_printer_columns(agent* pSoarAgent, const char* msg);

    public:

        void init_Output_Manager(sml::Kernel* pKernel, Soar_Instance* pSoarInstance);

        trace_mode_info mode_info[num_trace_modes];

        void init();
        void fill_mode_info();

        void set_dprint_enabled(bool activate) { dprint_enabled = activate; }
        bool debug_mode_enabled(TraceMode mode) { return mode_info[mode].enabled && dprint_enabled; }

        void set_default_agent(agent* pSoarAgent);
        void clear_default_agent() { m_defaultAgent = NULL; }
        agent* get_default_agent() { return m_defaultAgent; }


        /* Core printing functions */
        void printa(agent* pSoarAgent, const char* msg);
        void printa_prefix(TraceMode mode, agent* pSoarAgent, const char* msg);
        void printa_database(TraceMode mode, agent* pSoarAgent, MessageType msgType, const char* msg);

        /* Core variadic printing functions */
        void printa_f(agent* pSoarAgent, const char* format, ...);
        void printa_sf(agent* pSoarAgent, const char* format, ...);

        /* Print functions that will use default agent if set */
        void print(const char* msg) { if (m_defaultAgent) printa(m_defaultAgent, msg); }
        void print_prefix(TraceMode mode, const char* msg) { if (m_defaultAgent) printa_prefix(mode, m_defaultAgent, msg); }
        void print_f(const char* format, ...);
        void print_sf(const char* format, ...);

        /* Versions that will check debug mode and only print if set */
        void debug_print(TraceMode mode, const char* msg);
        void debug_print_f(TraceMode mode, const char* format, ...);
        void debug_print_sf(TraceMode mode, const char* format, ...);
        void debug_print_sf_noprefix(TraceMode mode, const char* format, ...);
        void debug_start_fresh_line(TraceMode mode);

        char* get_printed_output_string()
        {
            if (++next_output_string == num_output_strings)
            {
                next_output_string = 0;
            }
            return printed_output_strings[next_output_string];
        }

        void store_refcount(Symbol* sym, const char* trace, bool isAdd);
        int get_printer_output_column(agent* thisAgent = NULL);
        void set_printer_output_column(agent* thisAgent = NULL, int pOutputColumn = 1);
        void start_fresh_line(agent* pSoarAgent = NULL);

        void set_dprint_params(TraceMode mode, const char* pPre = NULL, const char* pPost = NULL, bool pActual = true, bool pOriginal = false, bool p_Identity = true)
        {
            if (!debug_mode_enabled(mode) || !dprint_enabled) return;
            m_print_actual = pActual;
            m_print_original = pOriginal;
            m_print_identity = p_Identity;
            if (pPre) m_pre_string = strdup(pPre);
            if (pPost) m_pre_string = strdup(pPost);
        }
        void clear_dprint_params(TraceMode mode) { set_dprint_params(mode); }

        void debug_print_production(TraceMode mode, production* prod);
        void debug_print_preference(TraceMode mode, preference* pref);
        void debug_print_preflist_inst(TraceMode mode, preference* top_pref);
        void debug_print_preflist_result(TraceMode mode, preference* top_pref);

        void print_test(TraceMode mode, test t, bool pActual = true, bool pOriginal = false, bool pIdentity = true, const char* pre_string = "", const char* post_string = "");
        void print_identity(TraceMode mode, identity_info* i, const char* pre_string = "", const char* post_string = "");
        void print_current_lexeme(TraceMode mode, soar::Lexer* lexer);
        void print_condition(TraceMode mode, condition* cond);
        void print_condition_list(TraceMode mode, condition* top_cond);
        void print_action(TraceMode mode, action* a);
        void print_action_list(TraceMode mode, action* action_list);
        void print_instantiation(TraceMode mode, instantiation* inst);
        void print_cond_prefs(TraceMode mode, condition* top_cond, preference* top_pref);
        void print_cond_actions(TraceMode mode, condition* top_cond, action* top_action);
        void print_cond_results(TraceMode mode, condition* top_cond, preference* top_pref);
        void print_identifiers(TraceMode mode);
        void print_condition_cons(TraceMode mode, cons* c);
        void print_rhs_value(TraceMode mode, rhs_value rv, struct token_struct* tok = NIL, wme* w = NIL);
        void print_saved_test(TraceMode mode, saved_test* st);
        void print_saved_test_list(TraceMode mode, saved_test* st);
        void print_varnames(TraceMode mode, varnames* var_names);
        void print_varnames_node(TraceMode mode, node_varnames* var_names_node);
        void print_all_inst(TraceMode mode);
        void print_variables(TraceMode mode);
        void print_wmes(TraceMode mode, bool pOnlyWithIdentity = false);
        void print_wme(TraceMode mode, wme* w, bool pOnlyWithIdentity = false);

        void debug_find_and_print_sym(char* find_string);
        char* NULL_SYM_STR;

};

#endif /* OUTPUT_MANAGER_H_ */
