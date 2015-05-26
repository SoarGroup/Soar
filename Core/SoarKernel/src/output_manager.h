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
#include "output_manager_db.h"

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

#ifndef SOAR_RELEASE_VERSION
    #define OM_Default_print_actual true;
    #define OM_Default_print_original false;
    #define OM_Default_print_identity true;
#else
    #define OM_Default_print_actual true;
    #define OM_Default_print_original false;
    #define OM_Default_print_identity false;
#endif

typedef struct trace_mode_info_struct
{
        char* prefix;
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

        bool print_enabled;
        bool callback_mode, stdout_mode, db_mode;
        bool callback_dbg_mode, stdout_dbg_mode, db_dbg_mode;
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

        /* -- Array for each trace output mode.  Contains prefix strings and whether enabled -- */
        trace_mode_info mode_info[num_trace_modes];

        /* -- Global toggles for database, standard out -- */
        bool db_mode, stdout_mode;

        /* -- Settings for how tests are printed (actual, original production tests, test identity) -- */
        bool m_print_actual, m_print_original, m_print_identity;
        bool m_print_actual_effective, m_print_original_effective, m_print_identity_effective;
        char* m_pre_string, *m_post_string;

        /* -- A quick replacement for Soar's printed_output_strings system.  Rather than have
         *    one string buffer, it rotates through 10 of them.  It allows us to have multiple
         *    function calls that use that buffer within one print statements.  There are
         *    probably better approaches, but this avoided revising a lot of other code and
         *    does the job.  -- */
        char        printed_output_strings[output_string_size][num_output_strings];
        int64_t     next_output_string;

        /* -- The following tracks column of the next character to print if Soar is writing to cout --*/
        int     global_printer_output_column;
        void    update_printer_columns(agent* pSoarAgent, const char* msg);

        void fill_mode_info();

        char* action_to_string(agent* thisAgent, action* a, char* dest, size_t dest_size);
        char* action_list_to_string(agent* thisAgent, action* action_list, char* dest, size_t dest_size);
        char* condition_to_string(agent* thisAgent, condition* cond, char* dest, size_t dest_size);
        char* condition_cons_to_string(agent* thisAgent, cons* c, char* dest, size_t dest_size);
        char* condition_list_to_string(agent* thisAgent, condition* top_cond, char* dest, size_t dest_size);
        char* cond_prefs_to_string(agent* thisAgent, condition* top_cond, preference* top_pref, char* dest, size_t dest_size);
        char* cond_actions_to_string(agent* thisAgent, condition* top_cond, action* top_action, char* dest, size_t dest_size);
        char* cond_results_to_string(agent* thisAgent, condition* top_cond, preference* top_pref, char* dest, size_t dest_size);
        char* identity_to_string(agent* thisAgent, test t, char* dest, size_t dest_size);
        char* instantiation_to_string(agent* thisAgent, instantiation* inst, char* dest, size_t dest_size);
        char* pref_to_string(agent* thisAgent, preference* pref, char* dest, size_t dest_size);
        char* preflist_inst_to_string(agent* thisAgent, preference* top_pref, char* dest, size_t dest_size);
        char* preflist_result_to_string(agent* thisAgent, preference* top_pref, char* dest, size_t dest_size);
        char* rhs_value_to_string(agent* thisAgent, rhs_value rv, char* dest, size_t dest_size, struct token_struct* tok = NIL, wme* w = NIL);
        char* test_to_string(test t, char* dest, size_t dest_size, bool show_equality = false);
        const char* test_type_to_string_brief(byte test_type, const char* equality_str = "");
        char* wme_to_string(agent* thisAgent, wme* w, char* dest, size_t dest_size);
        char* WM_to_string(agent* thisAgent, char* dest, size_t dest_size);

        void vsnprint_sf(agent* thisAgent, char* dest, size_t dest_size, const char* format, va_list args);

    public:

        void init_Output_Manager(sml::Kernel* pKernel, Soar_Instance* pSoarInstance);

        bool debug_mode_enabled(TraceMode mode) { return mode_info[mode].enabled; }

        void set_default_agent(agent* pSoarAgent) { assert(pSoarAgent); m_defaultAgent = pSoarAgent; };
        void clear_default_agent() { m_defaultAgent = NULL; }
        agent* get_default_agent() { return m_defaultAgent; }


        /* Core printing functions */
        void printa(agent* pSoarAgent, const char* msg);
        void printa_sf(agent* pSoarAgent, const char* format, ...);
        void sprinta_sf(agent* thisAgent, char* dest, size_t dest_size, const char* format, ...);
        void start_fresh_line(agent* pSoarAgent = NULL);

        /* Print functions that will use default agent if set */
        void print(const char* msg) { if (m_defaultAgent) printa(m_defaultAgent, msg); }
        void print_sf(const char* format, ...);
        void sprint_sf(char* dest, size_t dest_size, const char* format, ...);

        /* Print to database */
        void printa_database(TraceMode mode, agent* pSoarAgent, MessageType msgType, const char* msg);
        void store_refcount(Symbol* sym, const char* callers, bool isAdd) { m_db->store_refcount(sym, callers, isAdd); }

        /* Versions that will check debug mode and only print if set */
        void debug_print(TraceMode mode, const char* msg);
        void debug_print_sf(TraceMode mode, const char* format, ...);
        void debug_print_sf_noprefix(TraceMode mode, const char* format, ...);
        void debug_print_header(TraceMode mode, Print_Header_Type whichHeaders, const char* format, ...);
        void debug_start_fresh_line(TraceMode mode);

        char* get_printed_output_string()
        {
            if (++next_output_string == num_output_strings)
            {
                next_output_string = 0;
            }
            return printed_output_strings[next_output_string];
        }

        int get_printer_output_column(agent* thisAgent = NULL);
        void set_printer_output_column(agent* thisAgent = NULL, int pOutputColumn = 1);

        void set_print_indents(const char* pPre = NULL, const char* pPost = NULL)
        {
            if (pPre)
            {
                if (m_pre_string) free(m_pre_string);
                if (strlen(pPre) > 0)
                {
                    m_pre_string = strdup(pPre);
                }
                else
                {
                    m_pre_string = NULL;
                }
            }
            if (pPost)
            {
                if (m_post_string) free(m_post_string);
                if (strlen(pPost) > 0)
                {
                    m_post_string = strdup(pPost);
                }
                else
                {
                    m_post_string = NULL;
                }
            }
        }
        void set_default_print_test_format(bool pActual, bool pOriginal, bool p_Identity)
        {
            m_print_actual = pActual;
            m_print_original = pOriginal;
            m_print_identity = p_Identity;
        }

        void set_print_test_format(bool pActual, bool pOriginal, bool p_Identity)
        {
            m_print_actual_effective = pActual;
            m_print_original_effective = pOriginal;
            m_print_identity_effective = p_Identity;
        }
        void reset_print_test_format()
        {
            m_print_actual = OM_Default_print_actual;
            m_print_original = OM_Default_print_original;
            m_print_identity = OM_Default_print_identity;
            clear_print_test_format();
        }

        void clear_print_test_format()
        {
            m_print_actual_effective = m_print_actual;
            m_print_original_effective = m_print_original;
            m_print_identity_effective = m_print_identity;
        }
        void clear_print_indents() { set_print_indents(); }

        void set_dprint_indents(TraceMode mode, const char* pPre = NULL, const char* pPost = NULL)
        {
            if (debug_mode_enabled(mode)) set_print_indents(pPre, pPost);
        }
        void set_dprint_test_format(TraceMode mode, bool pActual, bool pOriginal, bool p_Identity)
        {
            if (debug_mode_enabled(mode)) set_print_test_format(pActual, pOriginal, p_Identity);
        }
        void clear_dprint_indents(TraceMode mode)
        {
            if (debug_mode_enabled(mode)) clear_print_indents();
        }
        void reset_dprint_test_format(TraceMode mode) {
            if (debug_mode_enabled(mode)) reset_print_test_format();
        }

        void clear_dprint_test_format(TraceMode mode) {
            if (debug_mode_enabled(mode)) clear_print_test_format();
        }

        /* -- The following should all be refactored into to_string functions to be used with format strings -- */
        void debug_print_production(TraceMode mode, production* prod);

        void print_current_lexeme(TraceMode mode, soar::Lexer* lexer);
        void print_identifiers(TraceMode mode);
        void print_saved_test(TraceMode mode, saved_test* st);
        void print_saved_test_list(TraceMode mode, saved_test* st);
        void print_varnames(TraceMode mode, varnames* var_names);
        void print_varnames_node(TraceMode mode, node_varnames* var_names_node);
        void print_all_inst(TraceMode mode);
        void print_variables(TraceMode mode);

        /* -- Should be moved elsewhere -- */
        void debug_find_and_print_sym(char* find_string);
        char* NULL_SYM_STR;

};

/* ------------------------------------
 *    Format strings for Soar printing:
 *
 *       %c   character
 *       %i   int64_t
 *       %u   uint64_t
 *       %d   short
 *       %s   string
 *       %f   fresh line (adds newline if not at column 1)
 *
 *       %a   action
 *       %l   condition
 *       %7   instantiation
 *       %p   preference
 *       %y   symbol
 *       %o   symbol's original variable(s)
 *       %t   test
 *       %g   identity information for test
 *       %w   wme
 *
 *       %1   condition list
 *       %2   action list
 *       %3   cons list of conditions
 *
 *       %1   condition list
 *       %2   cons list of conditions
 *       %3   action lists
 *       %4   preference lists
 *       %5   results lists
 *
 *       %4   condition action lists (2 args: cond, action)
 *       %5   condition preference lists (2 args: cond, preference)
 *       %6   condition results lists (2 args: cond, preference)
 *
 *       %8   Working Memory
 *
   ------------------------------------*/
#endif /* OUTPUT_MANAGER_H_ */
