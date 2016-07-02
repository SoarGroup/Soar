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

#include "debug.h"

#include "agent.h"
#include "condition.h"
#include "ebc.h"
#include "episodic_memory.h"
#include "lexer.h"
#include "print.h"
#include "soar_module.h"
#include "soar_instance.h"
#include "test.h"
#include "output_manager.h"
#include "working_memory.h"

#include <string>
#include <iostream>
#include <sstream>

/* -- For stack trace printing (works on Mac.  Not sure about other platforms) --*/
#ifdef DEBUG_MAC_STACKTRACE
#include <execinfo.h>
#include <cxxabi.h>
#include <debug_stacktrace.h>
#endif

using namespace soar_module;

void debug_set_mode_info(trace_mode_info mode_info[num_trace_modes], bool pEnabled)
{
    for (int i=0; i < num_trace_modes; i++)
    {
        mode_info[i].enabled = false;
    }
    if (pEnabled)
    {
        mode_info[No_Mode].enabled =                        true;
        mode_info[DT_DEBUG].enabled =                       true;

    //    mode_info[DT_ID_LEAKING].enabled =                  true;
//        mode_info[DT_DEALLOCATES].enabled =                 true;
//        mode_info[DT_DEALLOCATE_SYMBOLS].enabled =          true;
    //    mode_info[DT_REFCOUNT_ADDS].enabled =               true;
    //    mode_info[DT_REFCOUNT_REMS].enabled =               true;
    //
    //    mode_info[DT_SOAR_INSTANCE].enabled =               true;
    //    mode_info[DT_CLI_LIBRARIES].enabled =               true;
    //    mode_info[DT_PARSER].enabled =                      true;
    //    mode_info[DT_GDS].enabled =                         true;
    //    mode_info[DT_EPMEM_CMD].enabled =                   true;
    //
//        mode_info[DT_PRINT_INSTANTIATIONS].enabled =        true;
//        mode_info[DT_MILESTONES].enabled =                  true;
    //
    //    mode_info[DT_ADD_ADDITIONALS].enabled =             true;
//        mode_info[DT_VARIABLIZATION_MANAGER].enabled =      true;
    //    mode_info[DT_VM_MAPS].enabled =                     true;
    //    mode_info[DT_BACKTRACE].enabled =                   true;
    //    mode_info[DT_IDENTITY_PROP].enabled =               true;
    //    mode_info[DT_UNIFICATION].enabled =                 true;
    //    mode_info[DT_CONSTRAINTS].enabled =                 true;
//        mode_info[DT_LHS_VARIABLIZATION].enabled =          true;
//        mode_info[DT_RHS_VARIABLIZATION].enabled =          true;
    //    mode_info[DT_RL_VARIABLIZATION].enabled =           true;
    //    mode_info[DT_NCC_VARIABLIZATION].enabled =          true;
    //    mode_info[DT_UNGROUNDED_STI].enabled =              true;
//        mode_info[DT_REORDERER].enabled =                   true;
    //    mode_info[DT_MERGE].enabled =                       true;
    //    mode_info[DT_BUILD_CHUNK_CONDS].enabled =           true;
    //    mode_info[DT_EBC_CLEANUP].enabled =                 true;
    //    mode_info[DT_RHS_VALUE].enabled =                   true;
    //    mode_info[DT_REV_BT].enabled =                      true;
    //
    //    mode_info[DT_WME_CHANGES].enabled =                 true;
    //    mode_info[DT_DEALLOCATES_TESTS].enabled =           true;
    //    mode_info[DT_LINKS].enabled =                       true;
    //    mode_info[DT_UNKNOWN_LEVEL].enabled =               true;
    //    mode_info[DT_RETE_PNODE_ADD].enabled =              true;
//        mode_info[DT_GROUND_LTI].enabled =                  true;

    //    mode_info[DT_EXPLAIN].enabled =                     true;
    //    mode_info[DT_EXPLAIN_DEP].enabled =                 true;
    //    mode_info[DT_EXPLAIN_ADD_INST].enabled =            true;
    //    mode_info[DT_EXPLAIN_CONNECT].enabled =             true;
    //    mode_info[DT_EXPLAIN_PATHS].enabled =               true;
    //    mode_info[DT_EXPLAIN_CONDS].enabled =               true;
        //    mode_info[DT_EXPLAIN_IDENTITIES].enabled =               true;
        //    mode_info[DT_UNIFY_SINGLETONS].enabled =               true;
//            mode_info[DT_EXTRA_RESULTS].enabled =               true;
 //            mode_info[DT_PARSER_PROMOTE].enabled =               true;
       //
    }
}

void initialize_debug_trace(trace_mode_info mode_info[num_trace_modes])
{
    mode_info[No_Mode].prefix =                         strdup("        | ");
    mode_info[DT_DEBUG].prefix =                        strdup("Debug   | ");

    mode_info[DT_REFCOUNT_ADDS].prefix =                strdup("RefCnt  | ");
    mode_info[DT_REFCOUNT_REMS].prefix =                strdup("RefCnt  | ");
    mode_info[DT_DEALLOCATES].prefix =                  strdup("Delete  | ");
    mode_info[DT_DEALLOCATE_SYMBOLS].prefix =           strdup("DelSymbl| ");
    mode_info[DT_DEALLOCATES_TESTS].prefix =            strdup("DelTests| ");
    mode_info[DT_ID_LEAKING].prefix =                   strdup("ID Leak | ");

    mode_info[DT_SOAR_INSTANCE].prefix =                strdup("SoarInst| ");
    mode_info[DT_CLI_LIBRARIES].prefix =                strdup("CLI Lib | ");
    mode_info[DT_EPMEM_CMD].prefix =                    strdup("EpMemCmd| ");
    mode_info[DT_PARSER].prefix =                       strdup("Parser  | ");
    mode_info[DT_GDS].prefix =                          strdup("GDS     | ");
    mode_info[DT_WME_CHANGES].prefix =                  strdup("WMEChngs| ");
    mode_info[DT_LINKS].prefix =                        strdup("Links   | ");
    mode_info[DT_UNKNOWN_LEVEL].prefix =                strdup("No Level| ");


    mode_info[DT_MILESTONES].prefix =                   strdup("Milestne| ");
    mode_info[DT_PRINT_INSTANTIATIONS].prefix =         strdup("PrntInst| ");

    mode_info[DT_ADD_ADDITIONALS].prefix =              strdup("AddAddtn| ");
    mode_info[DT_VARIABLIZATION_MANAGER].prefix =       strdup("VrblzMgr| ");
    mode_info[DT_VM_MAPS].prefix =                      strdup("VM Maps | ");
    mode_info[DT_BACKTRACE].prefix =                    strdup("BackTrce| ");
    mode_info[DT_BUILD_CHUNK_CONDS].prefix =            strdup("BChnkCnd| ");
    mode_info[DT_IDENTITY_PROP].prefix =                strdup("ID Prop | ");
    mode_info[DT_UNIFICATION].prefix =                  strdup("Unify   | ");
    mode_info[DT_CONSTRAINTS].prefix =                  strdup("Cnstrnts| ");
    mode_info[DT_LHS_VARIABLIZATION].prefix =           strdup("VrblzLHS| ");
    mode_info[DT_RHS_VARIABLIZATION].prefix =           strdup("VrblzRHS| ");
    mode_info[DT_RHS_VALUE].prefix =                    strdup("RHSValue| ");
    mode_info[DT_RL_VARIABLIZATION].prefix =            strdup("Vrblz RL| ");
    mode_info[DT_NCC_VARIABLIZATION].prefix =           strdup("VrblzNCC| ");
    mode_info[DT_UNGROUNDED_STI].prefix =               strdup("UngrnSTI| ");
    mode_info[DT_MERGE].prefix =                        strdup("Merge Cs| ");
    mode_info[DT_REORDERER].prefix =                    strdup("Reorder | ");
    mode_info[DT_EBC_CLEANUP].prefix =                  strdup("CleanUp | ");
    mode_info[DT_RETE_PNODE_ADD].prefix =               strdup("ReteNode| ");
    mode_info[DT_GROUND_LTI].prefix =                   strdup("Grnd LTI| ");
    mode_info[DT_EXPLAIN].prefix =                      strdup("Explain | ");
    mode_info[DT_EXPLAIN_PATHS].prefix =                strdup("EIDPaths| ");
    mode_info[DT_EXPLAIN_ADD_INST].prefix =             strdup("EAddInst| ");
    mode_info[DT_EXPLAIN_CONNECT].prefix =              strdup("EConnect| ");
    mode_info[DT_EXPLAIN_UPDATE].prefix =               strdup("EUpdate | ");
    mode_info[DT_EXPLAIN_CONDS].prefix =                strdup("EConds  | ");
    mode_info[DT_EXPLAIN_IDENTITIES].prefix =           strdup("EIdent  | ");
    mode_info[DT_UNIFY_SINGLETONS].prefix =             strdup("Unify_S | ");
    mode_info[DT_EXTRA_RESULTS].prefix =                strdup("ExtraRes| ");
    mode_info[DT_PARSER_PROMOTE].prefix =                strdup("ExtraRes| ");

#ifdef DEBUG_OUTPUT_ON
    debug_set_mode_info(mode_info, true);
#else
    debug_set_mode_info(mode_info, false);
#endif
}
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

void debug_trace_set(int dt_num, bool pEnable)
{
    agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
    if (!thisAgent)
    {
        return;
    }
    if (dt_num < num_trace_modes)
    {
        if (dt_num == 0)
        {
             Output_Manager::Get_OM().set_output_params_global(pEnable);
             thisAgent->output_settings->set_output_params_agent(pEnable);
             dprint(DT_DEBUG, "Debug output test statement...\n");
             return;
        }
        Output_Manager::Get_OM().set_output_mode(dt_num, pEnable);
    }
}

void debug_print_rhs(rhs_value rv)
{
    agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
    if (!thisAgent)
    {
        return;
    }
    dprint(DT_DEBUG, "%r\n", rv);
    Output_Manager::Get_OM().set_output_params_global(true);
    thisAgent->output_settings->set_output_params_agent(true);
}

void debug_test(int type)
{
    agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
    if (!thisAgent)
    {
        return;
    }

    switch (type)
    {
        case 1:
        {
            Symbol *sym = find_identifier(thisAgent, 'V', 30);
            if (sym)
            {
                dprint(DT_DEBUG, "%y found.\n", sym);
                condition* dummy = make_condition(thisAgent);

//                thisAgent->ebChunker->generate_conditions_to_ground_lti(&dummy, sym);
            } else {
                dprint(DT_DEBUG, "Could not find symbol.\n");
            }
            break;
        }
        case 2:
            dprint(DT_DEBUG, "%8");
            break;
        case 3:
        {
            /* -- Print all wme's -- */
            dprint(DT_DEBUG, "%8");
            break;
        }
        case 4:
        {
            Symbol *sym = find_identifier(thisAgent, 'G', 1);
            if (sym)
            {
                dprint(DT_DEBUG, "G1 found.  level = %d, promoted level = %d.\n", sym->id->level, sym->id->promotion_level);
            } else {
                dprint(DT_DEBUG, "Could not find G1.\n");
            }
        }
            break;

        case 5:
        {
            print_internal_symbols(thisAgent);
            dprint_identifiers(DT_DEBUG);
            break;
        }
        case 6:
        {
            dprint_variablization_table(DT_DEBUG);
            dprint_o_id_tables(DT_DEBUG);
            dprint_attachment_points(DT_DEBUG);
            dprint_constraints(DT_DEBUG);
            dprint_merge_map(DT_DEBUG);
            dprint_ovar_to_o_id_map(DT_DEBUG);
            dprint_o_id_substitution_map(DT_DEBUG);
            dprint_o_id_to_ovar_debug_map(DT_DEBUG);
            dprint_tables(DT_DEBUG);
            break;
        }
        case 7:
            /* -- Print all instantiations -- */
            dprint_all_inst(DT_DEBUG);
            break;
        case 8:
        {
            break;
        }
        case 9:
            break;
    }
}
