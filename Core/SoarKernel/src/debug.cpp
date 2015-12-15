/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
                       debug.cpp

   @brief debug.cpp provides some utility functions for inspecting and
          manipulating the data structures of the Soar kernel at run
          time.

------------------------------------------------------------------ */

#include <ebc.h>
#include "debug.h"
#include "agent.h"
#include "test.h"
#include "episodic_memory.h"
#include "soar_module.h"
#include "lexer.h"
#include "soar_instance.h"
#include "print.h"
#include "output_manager.h"
#include "wmem.h"

#include <string>
#include <iostream>
#include <sstream>


/* -- For stack trace printing (works on Mac.  Not sure about other platforms) --*/
#ifdef DEBUG_MAC_STACKTRACE
#include <execinfo.h>
#include <cxxabi.h>
#include "debug_trace.h"
#endif

using namespace soar_module;

debug_param_container::debug_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    epmem_commands = new soar_module::boolean_param("epmem", off, new soar_module::f_predicate<boolean>());
    smem_commands = new soar_module::boolean_param("smem", off, new soar_module::f_predicate<boolean>());
    sql_commands = new soar_module::boolean_param("sql", off, new soar_module::f_predicate<boolean>());
    use_new_chunking = new soar_module::boolean_param("chunk", on, new soar_module::f_predicate<boolean>());

    add(epmem_commands);
    add(smem_commands);
    add(sql_commands);
    add(use_new_chunking);
}

#include "sqlite3.h"
bool symbol_matches_string(Symbol* sym, const char* match)
{
    std::string strName(sym->to_string());
    if (strName == match)
    {
//        dprint(DT_DEBUG, "%sFound %s(%i) | %s\n", message, strName.c_str(), sym->reference_count, "");
        return true;
    }
    return false;
}
bool wme_matches_string(wme *w, const char* match_id, const char* match_attr, const char* match_value)
{
    return (symbol_matches_string(w->id, match_id) &&
            symbol_matches_string(w->attr, match_attr) &&
            symbol_matches_string(w->value, match_value));
}

bool wme_matches_bug(wme *w)
{
    if (wme_matches_string(w, "@P3", "default", "@D1") ||
        wme_matches_string(w, "@P4", "default", "@D1") ||
        wme_matches_string(w, "@D1", "name", "pantry") ||
        wme_matches_string(w, "@P2", "1", "@P3") ||
        wme_matches_string(w, "S1", "goal", "@G1") ||
        wme_matches_string(w, "R6", "retrieved", "@G1") ||
        wme_matches_string(w, "@P2", "1", "@P4"))
    {
        if ((wme_matches_string(w, "S1", "goal", "@G1")) || wme_matches_string(w, "R6", "retrieved", "@G1"))

            dprint(DT_UNKNOWN_LEVEL, "(S1 ^ goal @G1) or (R6 ^retrieved @G1)found.\n");
        return true;
    }
    return false;

}
bool check_symbol(agent* thisAgent, Symbol* sym, const char* message)
{
#ifdef DEBUG_CHECK_SYMBOL
    std::string strName(sym->to_string());
    if (strName == DEBUG_CHECK_SYMBOL)
    {
        //    dprint(DT_DEBUG, "%sFound %s(%i) | %s\n", message, strName.c_str(), sym->reference_count, get_refcount_stacktrace_string().c_str());
        dprint(DT_DEBUG, "%sFound %s(%i) | %s\n", message, strName.c_str(), sym->reference_count, "");
        return true;
    }
#endif
    return false;
}

bool check_symbol_in_test(agent* thisAgent, test t, const char* message)
{
#ifdef DEBUG_CHECK_SYMBOL
    cons* c;
    if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {
            if (t->type == EQUALITY_TEST)
            {
                if (static_cast<test>(c->first)->original_test)
                {
                    return (check_symbol(thisAgent, static_cast<test>(c->first)->data.referent, message) || check_symbol(thisAgent, static_cast<test>(c->first)->original_test->data.referent, message));
                }
                else
                {
                    return (check_symbol(thisAgent, static_cast<test>(c->first)->data.referent, message));
                }
            }
        }
    }
    else if (t->type == EQUALITY_TEST)
    {
        if (t->original_test)
        {
            return (check_symbol(thisAgent, t->data.referent, message) || check_symbol_in_test(thisAgent, t->original_test, message));
        }
        else
        {
            return (check_symbol(thisAgent, t->data.referent, message));
        }
    }
#endif
    return false;
}

#ifdef DEBUG_TRACE_REFCOUNT_INVENTORY

#include "output_manager.h"

void debug_store_refcount(Symbol* sym, bool isAdd)
{
    std::string caller_string = get_stacktrace(isAdd ? "add_ref" : "remove_ref");
    debug_agent->outputManager->store_refcount(sym, caller_string.c_str() , isAdd);
}

#endif

#ifdef DEBUG_MAC_STACKTRACE
std::string get_stacktrace(const char* prefix)
{
    // storage array for stack trace data
    // you can change the size of the array to increase the depth of
    // the stack summarized in the string returned
    void* addrlist[7];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0)
    {
        return std::string("<empty, possibly corrupt>");
    }

    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);
    std::string return_string;
    if (prefix)
    {
        return_string += prefix;
        return_string +=  " | ";
    }
    // iterate over the returned symbol lines. skip the first two
    for (int i = 2; i < addrlen; i++)
    {
        return_string += tracey::demangle(std::string(symbollist[i]));
        if (i < (addrlen - 1))
        {
            return_string += " | ";
        }
    }
    free(funcname);
    free(symbollist);
    return return_string;
}
#endif
