#include "debug.h"
#include "debug_defines.h"
#include "agent.h"
#include "test.h"
#include "variablization_manager.h"
#include "episodic_memory.h"
#include "soar_module.h"
#include "lexer.h"
#include "soar_instance.h"
#include "print.h"
#include "test.h"
#include "output_manager.h"
#include <string>
#include <iostream>
#include <sstream>
#include "sml_Names.h"
#include "stats.h"

/* -- Just a simple function that can be called from the debug command.  Something to put random code for testing/debugging -- */
extern void test_print_speed();
extern void test_print_speed_y();
using namespace sml;

void debug_memory_stats(agent* thisAgent)
{
    // Hostname
    char hostname[256];
    memset(hostname, 0, 256);
    if (gethostname(hostname, 255) == SOCKET_ERROR)
    {
        strncpy(hostname, "[host name unknown]", 255);
    }

    // Time
    time_t current_time = time(NULL);

#ifndef NO_TIMING_STUFF
    double total_kernel_time = thisAgent->timers_total_kernel_time.get_sec();
    double total_kernel_msec = total_kernel_time * 1000.0;

    double input_function_time = thisAgent->timers_input_function_cpu_time.get_sec();
    double output_function_time = thisAgent->timers_output_function_cpu_time.get_sec();

    /* Total of the time spent in callback routines. */
    double monitors_sum = thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[PREFERENCE_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[WM_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec()
                          + thisAgent->timers_monitors_cpu_time[DECISION_PHASE].get_sec();

    double derived_kernel_time = get_derived_kernel_time_usec(thisAgent) / 1000000.0;
    double derived_total_cpu_time = derived_kernel_time + monitors_sum + input_function_time + output_function_time;

    /* Total time spent in the input phase */
    double input_phase_total_time = thisAgent->timers_decision_cycle_phase[INPUT_PHASE].get_sec()
                                    + thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec()
                                    + thisAgent->timers_input_function_cpu_time.get_sec();

    /* Total time spent in the propose phase */
    double propose_phase_total_time = thisAgent->timers_decision_cycle_phase[PROPOSE_PHASE].get_sec()
                                      + thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec();

    /* Total time spent in the apply phase */
    double apply_phase_total_time = thisAgent->timers_decision_cycle_phase[APPLY_PHASE].get_sec()
                                    + thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec();

    /* Total time spent in the output phase */
    double output_phase_total_time = thisAgent->timers_decision_cycle_phase[OUTPUT_PHASE].get_sec()
                                     + thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec()
                                     + thisAgent->timers_output_function_cpu_time.get_sec();

    /* Total time spent in the decision phase */
    double decision_phase_total_time = thisAgent->timers_decision_cycle_phase[DECISION_PHASE].get_sec()
                                       + thisAgent->timers_monitors_cpu_time[DECISION_PHASE].get_sec();
#endif // NO_TIMING_STUFF

    /* The sum of these phase timers is exactly equal to the
    * derived_total_cpu_time
    */

    std::cout << "Soar " << sml_Names::kSoarVersionValue << " on " << hostname << " at " << ctime(&current_time) << "\n";

    uint64_t totalProductions = thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
    totalProductions += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];
    totalProductions += thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];

    std::cout << totalProductions << " productions ("
             << thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE] << " default, "
             << thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE] << " user, "
             << thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE] << " chunks)\n";

    std::cout << "   + " << thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE] << " justifications\n";

    /* The fields for the timers are 8.3, providing an upper limit of
    approximately 2.5 hours the printing of the run time calculations.
    Obviously, these will need to be increased if you plan on needing
    run-time data for a process that you expect to take longer than
    2 hours. :) */

#ifndef NO_TIMING_STUFF
    std::cout << "                                                        |   Computed\n";
    std::cout << "Phases:      Input   Propose   Decide   Apply    Output |     Totals\n";
    std::cout << "========================================================|===========\n";

    std::cout << "Kernel:   "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[DECISION_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_decision_cycle_phase[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << derived_kernel_time << "\n";

    std::cout << "========================================================|===========\n";

    std::cout << "Input fn: "
             << std::setw(8) << input_function_time << "                                      | "
             << std::setw(10) << input_function_time << "\n";

    std::cout << "========================================================|===========\n";
    std::cout << "Outpt fn:                                     "
             << std::setw(8) << output_function_time << "  | "
             << std::setw(10) << output_function_time << "\n";

    std::cout << "========================================================|===========\n";

    std::cout << "Callbcks: "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[DECISION_PHASE].get_sec() << " "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8) << thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << monitors_sum << "\n";

    std::cout << "========================================================|===========\n";
    std::cout << "Computed------------------------------------------------+-----------\n";
    std::cout << "Totals:   "
             << std::setw(8) << input_phase_total_time << " "
             << std::setw(8) << propose_phase_total_time << " "
             << std::setw(8) << decision_phase_total_time << " "
             << std::setw(8) << apply_phase_total_time << " "
             << std::setw(8) << output_phase_total_time << "  | "
             << std::setw(10) << derived_total_cpu_time << "\n\n";

#ifdef DETAILED_TIMING_STATS
    double match_sum = thisAgent->timers_match_cpu_time[INPUT_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[PROPOSE_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[APPLY_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[PREFERENCE_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[WM_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[OUTPUT_PHASE].get_sec()
                       + thisAgent->timers_match_cpu_time[DECISION_PHASE].get_sec();
    double own_sum = thisAgent->timers_ownership_cpu_time[INPUT_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[PROPOSE_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[APPLY_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[PREFERENCE_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[WM_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[OUTPUT_PHASE].get_sec()
                     + thisAgent->timers_ownership_cpu_time[DECISION_PHASE].get_sec();
    double chunk_sum = thisAgent->timers_chunking_cpu_time[INPUT_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[PROPOSE_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[APPLY_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[PREFERENCE_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[WM_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[OUTPUT_PHASE].get_sec()
                       + thisAgent->timers_chunking_cpu_time[DECISION_PHASE].get_sec();
    double gds_sum = thisAgent->timers_gds_cpu_time[INPUT_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[PROPOSE_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[APPLY_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[PREFERENCE_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[WM_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[OUTPUT_PHASE].get_sec()
                     + thisAgent->timers_gds_cpu_time[DECISION_PHASE].get_sec();

    std::cout << "Detailed Timing Stats:   " << "\n\n";
    std::cout << "=============================================================================|===========\n";
    std::cout << "Phases:        Input   Propose   Decide   Apply    Pref     WrkMem    Output |     Totals\n";
    std::cout << "=============================================================================|===========\n";
    std::cout << "Match:      "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[DECISION_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[OUTPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[PREFERENCE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_match_cpu_time[WM_PHASE].get_sec() << "  | "
             << std::setw(10) << match_sum << "\n";
    std::cout << "Ownership:  "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[DECISION_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[PREFERENCE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[WM_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_ownership_cpu_time[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << own_sum << "\n";
    std::cout << "Chunking:   "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[DECISION_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[PREFERENCE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[WM_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_chunking_cpu_time[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << chunk_sum << "\n";
    std::cout << "GDS:        "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[INPUT_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[PROPOSE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[DECISION_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[APPLY_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[PREFERENCE_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[WM_PHASE].get_sec() << " "
             << std::setw(8)  << thisAgent->timers_gds_cpu_time[OUTPUT_PHASE].get_sec() << "  | "
             << std::setw(10) << gds_sum << "\n";
#endif // DETAILED_TIMING_STATS

    std::cout << "Values from single timers:\n";

    std::cout << " Kernel CPU Time: "
             << std::setw(11) << total_kernel_time << " sec. \n";

    std::cout << " Total  CPU Time: "
             << std::setw(11) << thisAgent->timers_total_cpu_time.get_sec() << " sec.\n\n";

    ///* v8.6.2: print out decisions executed, not # full cycles */

    std::cout << thisAgent->decision_phases_count << " decisions ("
             << (thisAgent->decision_phases_count ? total_kernel_msec / thisAgent->decision_phases_count : 0.0)
             << " msec/decision)\n";
    std::cout << thisAgent->e_cycle_count << " elaboration cycles ("
             << (thisAgent->decision_phases_count ? static_cast<double>(thisAgent->e_cycle_count) / thisAgent->decision_phases_count : 0)
             << " ec's per dc, "
             << (thisAgent->e_cycle_count ? total_kernel_msec / thisAgent->e_cycle_count : 0)
             << " msec/ec)\n";
    std::cout << thisAgent->inner_e_cycle_count << " inner elaboration cycles\n";

    std::cout << thisAgent->pe_cycle_count << " p-elaboration cycles ("
             << (thisAgent->decision_phases_count ? static_cast<double>(thisAgent->pe_cycle_count) / thisAgent->decision_phases_count : 0)
             << " pe's per dc, "
             << (thisAgent->pe_cycle_count ? total_kernel_msec / thisAgent->pe_cycle_count : 0)
             << " msec/pe)\n";

    std::cout << thisAgent->production_firing_count << " production firings ("
             << (thisAgent->e_cycle_count ? static_cast<double>(thisAgent->production_firing_count) / thisAgent->e_cycle_count : 0.0)
             << " pf's per ec, "
             << (thisAgent->production_firing_count ? total_kernel_msec / thisAgent->production_firing_count : 0.0)
             << " msec/pf)\n";
#endif // NO_TIMING_STUFF

    uint64_t wme_changes = thisAgent->wme_addition_count + thisAgent->wme_removal_count;
    std::cout << wme_changes << " wme changes ("
             << thisAgent->wme_addition_count << " additions, "
             << thisAgent->wme_removal_count << " removals)\n";

    std::cout << "WM size: "
             << thisAgent->num_wmes_in_rete << " current, "
             << (thisAgent->num_wm_sizes_accumulated ? (thisAgent->cumulative_wm_size / thisAgent->num_wm_sizes_accumulated) : 0.0)
             << " mean, "
             << thisAgent->max_wm_size << " maximum\n\n";

    std::cout << "Single decision cycle maximums:\n";

    std::cout << "Stat             Value       Cycle\n";
    std::cout << "---------------- ----------- -----------\n";

#ifndef NO_TIMING_STUFF
    std::cout << std::setw(16) << "Time (sec)"
             << std::setw(11) << std::setprecision(6) << (thisAgent->max_dc_time_usec / 1000000.0) << " "
             << std::setw(11) << thisAgent->max_dc_time_cycle << "\n";

    std::cout << std::setw(16) << "EpMem Time (sec)"
             << std::setw(11) << std::setprecision(6) << thisAgent->max_dc_epmem_time_sec << " "
             << std::setw(11) << thisAgent->max_dc_epmem_time_cycle << "\n";

    std::cout << std::setw(16) << "SMem Time (sec)"
             << std::setw(11) << std::setprecision(6) << thisAgent->max_dc_smem_time_sec << " "
             << std::setw(11) << thisAgent->max_dc_smem_time_cycle << "\n";
#endif // NO_TIMING_STUFF

    std::cout << std::setw(16) << "WM changes"
             << std::setw(11) << thisAgent->max_dc_wm_changes_value << " "
             << std::setw(11) << thisAgent->max_dc_wm_changes_cycle << "\n";

    std::cout << std::setw(16) << "Firing count"
             << std::setw(11) << thisAgent->max_dc_production_firing_count_value << " "
             << std::setw(11) << thisAgent->max_dc_production_firing_count_cycle << "\n";
                 size_t total = 0;

    for (int i = 0; i < NUM_MEM_USAGE_CODES; i++)
    {
        total += thisAgent->memory_for_usage[i];
    }

    std::cout << std::setw(8) << total << " bytes total memory allocated\n";
    std::cout << std::setw(8) << thisAgent->memory_for_usage[STATS_OVERHEAD_MEM_USAGE] << " bytes statistics overhead\n";
    std::cout << std::setw(8) << thisAgent->memory_for_usage[STRING_MEM_USAGE] << " bytes for strings\n";
    std::cout << std::setw(8) << thisAgent->memory_for_usage[HASH_TABLE_MEM_USAGE] << " bytes for hash tables\n";
    std::cout << std::setw(8) << thisAgent->memory_for_usage[POOL_MEM_USAGE] << " bytes for various memory pools\n";
    std::cout << std::setw(8) << thisAgent->memory_for_usage[MISCELLANEOUS_MEM_USAGE] << " bytes for miscellaneous other things\n";

    std::cout << "Memory pool statistics:\n\n";
#ifdef MEMORY_POOL_STATS
    std::cout << "Pool Name        Used Items  Free Items  Item Size  Itm/Blk  Blocks  Total Bytes\n";
    std::cout << "---------------  ----------  ----------  ---------  -------  ------  -----------\n";
#else
    std::cout << "Pool Name        Item Size  Itm/Blk  Blocks  Total Bytes\n";
    std::cout << "---------------  ---------  -------  ------  -----------\n";
#endif

    for (memory_pool* p = thisAgent->memory_pools_in_use; p != NIL; p = p->next)
    {
        std::cout << std::setw(MAX_POOL_NAME_LENGTH) << p->name;
#ifdef MEMORY_POOL_STATS
        std::cout << "  " << std::setw(10) << p->used_count;
        size_t total_items = p->num_blocks * p->items_per_block;
        std::cout << "  " << std::setw(10) << total_items - p->used_count;
#endif
        std::cout << "  " << std::setw(9) << p->item_size;
        std::cout << "  " << std::setw(7) << p->items_per_block;
        std::cout << "  " << std::setw(6) << p->num_blocks;
        std::cout << "  " << std::setw(11) << p->num_blocks* p->items_per_block* p->item_size;
        std::cout << "\n";
    }

}
void debug_test(int type)
{
    agent* debug_agent = Output_Manager::Get_OM().get_default_agent();
    if (!debug_agent)
    {
        return;
    }

//    Symbol* sym  = make_new_identifier(debug_agent, 'M', 1, 1);
//    test t = make_test(debug_agent, sym, GREATER_TEST);
//    int64_t i1=-23;
//    uint64_t ui1=33;
//    Symbol* newSym  = find_identifier(debug_agent, 'S', 3);
//    dprint(DT_DEBUG, "S1 refcount %d\n", newSym->reference_count);

    switch (type)
    {
        case 1:
            print_internal_symbols(debug_agent);
            dprint_identifiers(DT_DEBUG);
            break;
        case 2:
            /* -- Print all instantiations -- */
            dprint_all_inst(DT_DEBUG);
            break;
        case 3:
        {
            /* -- Print all wme's -- */
            dprint(DT_DEBUG, "%8");
            break;
        }
        case 4:
            debug_memory_stats(debug_agent);
            break;

        case 5:
        {
            reset_variable_generator(debug_agent, NULL, NULL);
            Symbol* sym1  = generate_new_variable(debug_agent, "m");
            Symbol* sym2  = generate_new_variable(debug_agent, "m");
            Symbol* sym3  = generate_new_variable(debug_agent, "m");
            Symbol* newSym1  = find_identifier(debug_agent, 'S', 1);
            Symbol* newSym2  = find_identifier(debug_agent, 'I', 2);
            Symbol* newSym3  = find_identifier(debug_agent, 'I', 3);
            test t1 = make_test(debug_agent, sym1, EQUALITY_TEST);
            test t2 = make_test(debug_agent, sym2, EQUALITY_TEST);
            test t3 = make_test(debug_agent, sym3, EQUALITY_TEST);
            t1->identity = 23;
            t2->identity = 33;
            t3->identity = 91;
            dprint(DT_DEBUG, "%y %y %u\n", sym1, sym2, t1->identity);
            dprint(DT_DEBUG, "%y %y %u %u\n", sym1, sym2, t2->identity, t1->identity);
            dprint(DT_DEBUG, "%y %y %y %u\n", newSym1, newSym2, newSym3, t1->identity);
            dprint(DT_DEBUG, "%y %t %y %y %u\n", sym1, t1, sym2, sym3, t1->identity);
            dprint(DT_DEBUG, "%y %g %y %y %u\n", sym1, t1, sym2, sym3, t1->identity);
            dprint(DT_DEBUG, "%y(o%u) %y(o%u)\n", t1->data.referent, t1->identity, t2->data.referent, t2->identity);
            dprint(DT_DEBUG, "%t[%g] %t[%g]\n", t1, t1, t2, t2);
            break;
        }
        case 6:
        {
            agent* thisAgent = debug_agent;
            dprint_variablization_tables(DT_DEBUG);
            dprint_variablization_tables(DT_DEBUG, 1);
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
            debug_agent->variablizationManager->print_o_id_tables(DT_DEBUG);
            break;
        case 8:
            test_print_speed();
            break;
        case 9:
            test_print_speed_y();
            break;
    }
}
void debug_test_structs()
{
    agent* debug_agent = Output_Manager::Get_OM().get_default_agent();
    if (!debug_agent)
    {
        return;
    }

    Symbol* newID01  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID02  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID03  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID04  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID05  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newID06  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newID07  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newID08  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newStr01 = make_str_constant(debug_agent, "attr1");
    Symbol* newStr02 = make_str_constant(debug_agent, "attr2");
    Symbol* newStr03 = make_str_constant(debug_agent, "attr3");
    Symbol* newStr04 = make_str_constant(debug_agent, "attr4");
    Symbol* newStr05 = make_str_constant(debug_agent, "str1");
    Symbol* newStr06 = make_str_constant(debug_agent, "str2");
    Symbol* newStr07 = make_str_constant(debug_agent, "str3");
    Symbol* newStr08 = make_str_constant(debug_agent, "str4");
    Symbol* newVar01 = make_variable(debug_agent, "var1");
    Symbol* newVar02 = make_variable(debug_agent, "var2");
    Symbol* newVar03 = make_variable(debug_agent, "var3");
    Symbol* newVar04 = make_variable(debug_agent, "var4");
    Symbol* newVar05 = make_variable(debug_agent, "var5");
    Symbol* newVar06 = make_variable(debug_agent, "var6");
    Symbol* newVar07 = make_variable(debug_agent, "var7");
    Symbol* newVar08 = make_variable(debug_agent, "var8");
    Symbol* newInt01 = make_int_constant(debug_agent, 1);
    Symbol* newInt02 = make_int_constant(debug_agent, 2);
    Symbol* newInt03 = make_int_constant(debug_agent, 3);
    Symbol* newInt04 = make_int_constant(debug_agent, 4);
    Symbol* newInt05 = make_int_constant(debug_agent, 5);
    Symbol* newInt06 = make_int_constant(debug_agent, 6);
    Symbol* newInt07 = make_int_constant(debug_agent, 7);
    Symbol* newInt08 = make_int_constant(debug_agent, 8);

    test idEqTest01 = make_test(debug_agent, newID01, EQUALITY_TEST);
    test idEqTest02 = make_test(debug_agent, newID02, EQUALITY_TEST);
    test idEqTest03 = make_test(debug_agent, newID03, EQUALITY_TEST);
    test idEqTest04 = make_test(debug_agent, newID04, EQUALITY_TEST);
    test idEqTest05 = make_test(debug_agent, newID05, EQUALITY_TEST);
    test idEqTest06 = make_test(debug_agent, newID06, EQUALITY_TEST);
    test idEqTest07 = make_test(debug_agent, newID07, EQUALITY_TEST);
    test idEqTest08 = make_test(debug_agent, newID08, EQUALITY_TEST);
    test strEqTest01 = make_test(debug_agent, newStr01, EQUALITY_TEST);
    test strEqTest02 = make_test(debug_agent, newStr02, EQUALITY_TEST);
    test strEqTest03 = make_test(debug_agent, newStr03, EQUALITY_TEST);
    test strEqTest04 = make_test(debug_agent, newStr04, EQUALITY_TEST);
    test strEqTest05 = make_test(debug_agent, newStr05, EQUALITY_TEST);
    test strEqTest06 = make_test(debug_agent, newStr06, EQUALITY_TEST);
    test strEqTest07 = make_test(debug_agent, newStr07, EQUALITY_TEST);
    test strEqTest08 = make_test(debug_agent, newStr08, EQUALITY_TEST);
    test varEqTest01 = make_test(debug_agent, newVar01, EQUALITY_TEST);
    test varEqTest02 = make_test(debug_agent, newVar02, EQUALITY_TEST);
    test varEqTest03 = make_test(debug_agent, newVar03, EQUALITY_TEST);
    test varEqTest04 = make_test(debug_agent, newVar04, EQUALITY_TEST);
    test varEqTest05 = make_test(debug_agent, newVar05, EQUALITY_TEST);
    test varEqTest06 = make_test(debug_agent, newVar06, EQUALITY_TEST);
    test varEqTest07 = make_test(debug_agent, newVar07, EQUALITY_TEST);
    test varEqTest08 = make_test(debug_agent, newVar08, EQUALITY_TEST);
    test intEqTest01 = make_test(debug_agent, newInt01, EQUALITY_TEST);
    test intEqTest02 = make_test(debug_agent, newInt02, EQUALITY_TEST);
    test intEqTest03 = make_test(debug_agent, newInt03, EQUALITY_TEST);
    test intEqTest04 = make_test(debug_agent, newInt04, EQUALITY_TEST);
    test intEqTest05 = make_test(debug_agent, newInt05, EQUALITY_TEST);
    test intEqTest06 = make_test(debug_agent, newInt06, EQUALITY_TEST);
    test intEqTest07 = make_test(debug_agent, newInt07, EQUALITY_TEST);
    test intEqTest08 = make_test(debug_agent, newInt08, EQUALITY_TEST);
    test blankTest = NULL;


    test dest, add_me;

    /* Test 1 - Bug in last version */
    //  dest = copy_test(debug_agent, idEqTest01);
    //  add_test(debug_agent, &blankTest, dest, varEqTest01);
    //  deallocate_test(debug_agent, dest);

    dest = copy_test(debug_agent, idEqTest01);
    add_me = copy_test(debug_agent, idEqTest02);
    add_test(debug_agent, &dest, add_me);
    add_test(debug_agent, &dest, idEqTest03);

    deallocate_test(debug_agent, dest);
    deallocate_test(debug_agent, idEqTest01);
    deallocate_test(debug_agent, idEqTest02);
    deallocate_test(debug_agent, idEqTest03);
    deallocate_test(debug_agent, idEqTest04);
    deallocate_test(debug_agent, idEqTest05);
    deallocate_test(debug_agent, idEqTest06);
    deallocate_test(debug_agent, idEqTest07);
    deallocate_test(debug_agent, idEqTest08);
    deallocate_test(debug_agent, strEqTest01);
    deallocate_test(debug_agent, strEqTest02);
    deallocate_test(debug_agent, strEqTest03);
    deallocate_test(debug_agent, strEqTest04);
    deallocate_test(debug_agent, strEqTest05);
    deallocate_test(debug_agent, strEqTest06);
    deallocate_test(debug_agent, strEqTest07);
    deallocate_test(debug_agent, strEqTest08);
    deallocate_test(debug_agent, varEqTest01);
    deallocate_test(debug_agent, varEqTest02);
    deallocate_test(debug_agent, varEqTest03);
    deallocate_test(debug_agent, varEqTest04);
    deallocate_test(debug_agent, varEqTest05);
    deallocate_test(debug_agent, varEqTest06);
    deallocate_test(debug_agent, varEqTest07);
    deallocate_test(debug_agent, varEqTest08);
    deallocate_test(debug_agent, intEqTest01);
    deallocate_test(debug_agent, intEqTest02);
    deallocate_test(debug_agent, intEqTest03);
    deallocate_test(debug_agent, intEqTest04);
    deallocate_test(debug_agent, intEqTest05);
    deallocate_test(debug_agent, intEqTest06);
    deallocate_test(debug_agent, intEqTest07);
    deallocate_test(debug_agent, intEqTest08);
}

void debug_test_find_delete_sym(agent* debug_agent, test* dest, Symbol* sym)
{
    ::list* c;
    dprint(DT_DEBUG, "Starting tests: %t\n", (*dest));
    dprint(DT_DEBUG, "Looking for %y.  Comparing against...", sym);
    c = (*dest)->data.conjunct_list;
    while (c)
    {
        dprint(DT_DEBUG, "%t", static_cast<test>(c->first));
        if (static_cast<test>(c->first)->data.referent == sym)
        {
            dprint_noprefix(DT_DEBUG, "<-- FOUND\n");
            c = delete_test_from_conjunct(debug_agent, dest, c);
            dprint_noprefix(DT_DEBUG, "...after deletion: %t\n", (*dest));
        }
        else
        {
            c = c->rest;
        }
    }
    dprint(DT_DEBUG, "Final tests: %t\n", (*dest));
}

void debug_test_delete_conjuncts()
{

    agent* debug_agent = Output_Manager::Get_OM().get_default_agent();
    if (!debug_agent)
    {
        return;
    }

    dprint(DT_DEBUG, "Delete conjunct test.  Creating tests...\n");

    Symbol* newID01  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newStr01 = make_str_constant(debug_agent, "attr1");
    Symbol* newInt01 = make_int_constant(debug_agent, 1);
    Symbol* newInt02 = make_int_constant(debug_agent, 2);

    test dest = NULL;
    add_test(debug_agent, &dest, make_test(debug_agent, newID01, EQUALITY_TEST));
    add_test(debug_agent, &dest, make_test(debug_agent, newStr01, EQUALITY_TEST));
    add_test(debug_agent, &dest, make_test(debug_agent, newInt01, GREATER_OR_EQUAL_TEST));
    add_test(debug_agent, &dest, make_test(debug_agent, newInt02, LESS_TEST));
    add_test(debug_agent, &dest, make_test(debug_agent, NULL, GOAL_ID_TEST));

    debug_test_find_delete_sym(debug_agent, &dest, NULL);
    debug_test_find_delete_sym(debug_agent, &dest, newInt01);
    debug_test_find_delete_sym(debug_agent, &dest, newID01);
    debug_test_find_delete_sym(debug_agent, &dest, newStr01);

    dprint(DT_DEBUG, "Deallocating tests and finishing...\n");
    deallocate_test(debug_agent, dest);

    symbol_remove_ref(debug_agent, newID01);
    symbol_remove_ref(debug_agent, newStr01);
    symbol_remove_ref(debug_agent, newInt01);
    symbol_remove_ref(debug_agent, newInt02);

//    print_internal_symbols(debug_agent);

}

