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

#include <assert.h>
#include <string>
#include <list>

#define MAX_COLUMNS 10
#define MAX_LEXER_LINE_LENGTH 1000
#define MAX_LEXEME_LENGTH (MAX_LEXER_LINE_LENGTH+5)
#define output_string_size MAX_LEXEME_LENGTH*2+10
#define maxAgentTraces 100

typedef struct trace_mode_info_struct
{
        char* prefix;
        bool enabled;
} trace_mode_info;

inline size_t om_strncpy(char* s1, const char* s2, size_t n, size_t num_chars) {
    if ( n > 0) {
        if (num_chars == 0) return n;
        if (num_chars+1 > n) {
            num_chars = n-1;
        }
        memcpy(s1, s2, num_chars);
        s1[num_chars]=0;
        return n-num_chars-1;
    } else {
        return n;
    }
}

class AgentOutput_Info
{
    public:
        AgentOutput_Info();

        bool print_enabled;
        bool callback_mode;
        int  printer_output_column;
        bool agent_traces_enabled[maxAgentTraces] ;
        void set_output_params_agent(bool pDebugEnabled);
} ;

class Output_Manager
{
        friend class OM_DB;
        friend class cli::CommandLineInterface;
        friend class Explanation_Memory;
        friend class GraphViz_Visualizer;
        friend class action_record;

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

        /* -- Global toggles for database, standard out -- */
        bool stdout_mode;

        /* -- Settings for how tests are printed (actual, original production tests, test identity) -- */
        bool m_print_actual, m_print_identity;
        bool m_print_actual_effective, m_print_identity_effective;
        char* m_pre_string, *m_post_string;
        int  m_column_indent[MAX_COLUMNS];

        /* -- The following tracks column of the next character to print if Soar is writing to cout --*/
        int     global_printer_output_column;
        void    update_printer_columns(agent* pSoarAgent, const char* msg);

        void action_to_string(agent* thisAgent, action* a, std::string &destString);
        void action_list_to_string(agent* thisAgent, action* action_list, std::string &destString);
        void condition_to_string(agent* thisAgent, condition* cond, std::string &destString);
        void condition_cons_to_string(agent* thisAgent, cons* c, std::string &destString);
        void condition_list_to_string(agent* thisAgent, condition* top_cond, std::string &destString);
        void cond_prefs_to_string(agent* thisAgent, condition* top_cond, preference* top_pref, std::string &destString);
        void cond_actions_to_string(agent* thisAgent, condition* top_cond, action* top_action, std::string &destString);
        void cond_results_to_string(agent* thisAgent, condition* top_cond, preference* top_pref, std::string &destString);
        void instantiation_to_string(agent* thisAgent, instantiation* inst, std::string &destString);
        void pref_to_string(agent* thisAgent, preference* pref, std::string &destString);
        void preflist_inst_to_string(agent* thisAgent, preference* top_pref, std::string &destString);
        void preflist_result_to_string(agent* thisAgent, preference* top_pref, std::string &destString);
        void test_to_string(test t, std::string &destString, bool show_equality = false);
        const char* test_type_to_string(byte test_type);
        bool wme_to_string(agent* thisAgent, wme* w, std::string &destString);
        void WM_to_string(agent* thisAgent, std::string &destString);

        void vsnprint_sf(agent* thisAgent, std::string &destString, const char* format, va_list args);

    public:

        /* -- Array for each trace output mode.  Contains prefix strings and whether enabled -- */
        trace_mode_info mode_info[num_trace_modes], saved_mode_info[num_trace_modes];

        uint64_t settings[num_output_sysparams];

        void init_Output_Manager(sml::Kernel* pKernel, Soar_Instance* pSoarInstance);
        void set_output_params_global(bool pDebugEnabled);
        void set_output_mode(int modeIndex, bool pEnabled);
        void copy_output_modes(trace_mode_info mode_info_src[num_trace_modes], trace_mode_info mode_info_dest[num_trace_modes]);
        void cache_output_modes();
        void restore_output_modes();
        void print_output_modes(trace_mode_info mode_info_to_print[num_trace_modes]);
        void clear_output_modes();

        bool is_trace_enabled(TraceMode mode) { return mode_info[mode].enabled; }

        /* Methods for the cli output command */
        void print_output_summary();
        bool is_printing_to_stdout() { return stdout_mode; };
        void set_printing_to_stdout(bool pEnabled) { stdout_mode = pEnabled; };

        void set_default_agent(agent* pSoarAgent) { assert(pSoarAgent); m_defaultAgent = pSoarAgent; };
        void clear_default_agent() { m_defaultAgent = NULL; }
        agent* get_default_agent() { return m_defaultAgent; }

        /* Core printing functions */
        void printa(agent* pSoarAgent, const char* msg);
        void printa_sf(agent* pSoarAgent, const char* format, ...);
        void sprinta_sf(agent* thisAgent, std::string &destString, const char* format, ...);
        size_t sprinta_sf_cstr(agent* thisAgent, char* dest, size_t dest_size, const char* format, ...);

        void buffer_start_fresh_line(agent* thisAgent, std::string &destString);
        void start_fresh_line(agent* pSoarAgent = NULL);

        /* Print functions that will use default agent if set */
        void print(const char* msg) { if (m_defaultAgent) printa(m_defaultAgent, msg); }
        void print_sf(const char* format, ...);
        void sprint_sf(std::string &destString, const char* format, ...);
        size_t sprint_sf_cstr(char* dest, size_t dest_size, const char* format, ...);

        /* Versions that will check debug mode and only print if set */
        void debug_print(TraceMode mode, const char* msg);
        void debug_print_sf(TraceMode mode, const char* format, ...);
        void debug_print_sf_noprefix(TraceMode mode, const char* format, ...);
        void debug_print_header(TraceMode mode, Print_Header_Type whichHeaders, const char* format, ...);

        void identity_to_string(agent* thisAgent, uint64_t pID, identity_set* pIDSet, std::string &destString, bool pOwnsIdentity = false);
        const char* phase_to_string(top_level_phase pPhase);
        void rhs_value_to_string(rhs_value rv, std::string &destString, struct token_struct* tok = NULL, wme* w = NULL, bool pEmptyStringForNullIdentity = false);
        void rhs_value_to_cstring(rhs_value rv, char* dest, size_t dest_size);

        /* Methods to make printing prettier */
        int get_printer_output_column(agent* thisAgent = NULL);
        void set_printer_output_column(agent* thisAgent = NULL, int pOutputColumn = 1);

        void print_spaces(agent* thisAgent, int n)
        {
            std::string lStr = std::string(n, ' ');
            printa(thisAgent, lStr.c_str());
        }

        void set_print_indents(const char* pPre = NULL, const char* pPost = NULL)
        {
            if (pPre) {
                if (m_pre_string) free(m_pre_string);
                if (strlen(pPre) > 0) {
                    m_pre_string = strdup(pPre);
                } else {
                    m_pre_string = NULL;
                }
            }
            if (pPost) {
                if (m_post_string) free(m_post_string);
                if (strlen(pPost) > 0) {
                    m_post_string = strdup(pPost);
                } else {
                    m_post_string = NULL;
                }
            }
        }
        void set_default_print_test_format(bool pActual, bool pIdentity)
        {
            m_print_actual = pActual;
            m_print_identity = pIdentity;
        }

        void set_print_test_format(bool pActual, bool pIdentity)
        {
            m_print_actual_effective = pActual;
            m_print_identity_effective = pIdentity;
        }
        void clear_print_test_format()
        {
            m_print_actual_effective = m_print_actual;
            m_print_identity_effective = m_print_identity;
        }
        void clear_print_indents() { set_print_indents(); }

        void set_column_indent(int pColumnIndex, int pColumnNum) {
            if (pColumnIndex >= MAX_COLUMNS) return;
            m_column_indent[pColumnIndex] = pColumnNum; }

        void reset_column_indents() { for (int i=0; i<MAX_COLUMNS; i++) m_column_indent[i] = 0; }

        /* -- The following should all be refactored into to_string functions to be used with format strings -- */
        void print_identifiers(TraceMode mode);
        void print_msc(TraceMode mode, ms_change* p_ms_change);
        void print_partial_matches(TraceMode mode, rete_node* pNode);
        void print_saved_test(TraceMode mode, saved_test* st);
        void print_saved_test_list(TraceMode mode, saved_test* st);
        void print_varnames(TraceMode mode, varnames* var_names);
        void print_varnames_node(TraceMode mode, node_varnames* var_names_node);
        void print_all_inst(TraceMode mode);
        void print_variables(TraceMode mode);
        void print_identity_sets(TraceMode mode);

        /* A single function to print all pre-formatted Soar error messages.  Added
         * to make other code cleaner and easier to parse */
        void display_soar_feedback(agent* thisAgent, SoarCannedMessageType pErrorType, bool shouldPrint = true);
        void display_ebc_error(agent* thisAgent, EBCFailureType pErrorType, const char* pString1 = NULL, const char* pString2 = NULL);
        void display_ambiguous_command_error(agent* thisAgent, std::list< std::string > matched_objects_str);

        /* -- Should be moved elsewhere -- */
        void debug_find_and_print_sym(char* find_string);
        char* NULL_SYM_STR;

};

inline const char* capitalizeOnOff(bool isEnabled) { return isEnabled ? "[ ON | off ]" : "[ on | OFF ]"; }
inline const char* capitalizeYesNo(bool isEnabled) { return isEnabled ? "[ YES | no ]" : "[ yes | NO ]"; }

inline std::string concatJustified(const char* left_string, std::string right_string, int pWidth)
{
    std::string return_string = left_string;
    int sepLength = pWidth - strlen(left_string) - right_string.length();
    if (sepLength <= 0) sepLength = 1;
    return_string.append(sepLength, ' ');
    return_string += right_string;
    return return_string;
}

/* ------------------------------------
 *    Format strings for Soar printing:
 *
 *       %c   character
 *       %d   int64_t
 *       %u   uint64_t
 *       %s   string
 *       %e   fresh line (adds newline if not at column 1)
 *       %f   double
 *       %-   fill to next column indent with spaces
 *       %=   fill to next column indent with periods
 *
 *       %a   action
 *       %l   condition
 *       %n   funcall list
 *       %p   preference
 *       %r   rhs value
 *       %y   symbol
 *       %t   test
 *       %g   variablization identity of test
 *       %h   like %g but with second argument with symbol to use if STI
 *       %w   wme
 *
 *       %1   condition list
 *       %2   action list
 *       %3   cons list of conditions
 *       %4   condition action lists (2 args: cond, action)
 *       %5   condition preference lists (2 args: cond, preference)
 *       %6   condition results lists (2 args: cond, preference)
 *       %7   instantiation
 *
 *   Alphabetical
 *
 *       %-   fill to next column indent with spaces
 *       %=   fill to next column indent with periods
 *       %1   condition list
 *       %2   action list
 *       %3   cons list of conditions
 *       %4   condition action lists (2 args: cond, action)
 *       %5   condition preference lists (2 args: cond, preference)
 *       %6   condition results lists (2 args: cond, preference)
 *       %7   instantiation
 *       %a   action
 *       %c   character
 *       %d   int64_t
 *       %e   fresh line (adds newline if not at column 1)
 *       %f   double
 *       %g   variablization identity of test
 *       %h   like %g but with second argument containing symbol to use if STI
 *       %l   condition
 *       %n   funcall list
 *       %p   preference
 *       %r   rhs value
 *       %s   string
 *       %t   test
 *       %u   uint64_t
 *       %w   wme
 *       %y   symbol
   ------------------------------------*/
#endif /* OUTPUT_MANAGER_H_ */
