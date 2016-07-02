/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
                       debug.h

   @brief debug.h provides some utility functions for inspecting and
          manipulating the data structures of the Soar kernel at run
          time.

          (Not much here now.  Will move some other utility stuff from
           experimental chunking and memory consolidation branches
           later.)

------------------------------------------------------------------ */

#ifndef SOARDEBUG_H
#define SOARDEBUG_H

#include "kernel.h"

#include "soar_module.h"


extern void debug_set_mode_info(trace_mode_info mode_info[num_trace_modes], bool pEnabled);
/* --------------------------------------------------------------------------*/


#ifdef DEBUG_EPMEM_SQL
    static void profile_sql(void* context, const char* sql, sqlite3_uint64 ns)
    {
    fprintf(stderr, "Execution Time of %llu ms for: %s\n", ns / 1000000, sql);
    }
    static void trace_sql(void* /*arg*/, const char* query)
    {
    fprintf(stderr, "Query: %s\n", query);
    }
#endif

extern void initialize_debug_trace(trace_mode_info mode_info[num_trace_modes]);
extern void debug_init_db(agent* thisAgent);
extern void debug_print_db_err(TraceMode mode = DT_DEBUG);
extern void debug_print_epmem_table(const char* table_name, TraceMode mode = DT_DEBUG);
extern void debug_print_smem_table(const char* table_name, TraceMode mode = DT_DEBUG);

extern void debug_store_refcount(Symbol* sym, bool isAdd);
extern void debug_destroy_for_refcount(agent* delete_agent);

extern void debug_test(int type = 1);
extern void debug_trace_set(int dt_num, bool pEnable);

extern std::string get_stacktrace(const char* prefix = NULL);
extern bool check_symbol(agent* thisAgent, Symbol* sym, const char* message = "ChkSym | ");
extern bool check_symbol_in_test(agent* thisAgent, test t, const char* message = "ChkSym | ");

extern bool wme_matches_string(wme *w, const char* match_id, const char* match_attr, const char* match_value);
extern bool symbol_matches_string(Symbol* sym, const char* match);
extern bool wme_matches_bug(wme *w);

/**
 * @brief Contains the parameters for the debug command
 */
class debug_param_container: public soar_module::param_container
{
    public:

        soar_module::boolean_param* epmem_commands, *smem_commands, *sql_commands, *use_new_chunking;

        debug_param_container(agent* new_agent);

};

#endif

