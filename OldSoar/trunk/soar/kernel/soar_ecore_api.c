/**
 * \file soar_ecore_api.c
 *   
 *                     The low-level interface to Soar
 * 
 *
 * Copyright 1995-2004 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 *
 * $Id$
 *
 */

#include "soarkernel.h"
#include <time.h>
#include "soar_ecore_api.h"
#include "soar_ecore_utils.h"
#include "rete.h"
#include "sysdep.h"

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 3:    ACCESSING, MODIFYING & WATCHING THE AGENT'S STATE
 *
 *
 * *************************************************************************
 * *************************************************************************
 */

/*
 *----------------------------------------------------------------------
 *
 * soar_ecSetDefaultWmeDepth
 *
 *       does what it says
 *
 *----------------------------------------------------------------------
 */
void soar_ecSetDefaultWmeDepth(int depth)
{

    current_agent(default_wme_depth) = depth;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_ecOpenLog
 *
 *       does what it says
 *
 *----------------------------------------------------------------------
 */
int soar_ecOpenLog(const char *filename, char *mode)
{

    FILE *f;

    f = fopen(filename, mode);

    if (!f) {
        return -1;
    }

    /* Install callback */
    soar_cPushCallback(soar_agent,
                       LOG_CALLBACK,
                       (soar_callback_fn) cb_soar_PrintToFile, (soar_callback_data) f, (soar_callback_free_fn) fclose);
    soar_invoke_first_callback(soar_agent, LOG_CALLBACK, "**** log opened ****\n");

    return 0;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_ecCloseLog
 *
 *       does what it says
 *
 *----------------------------------------------------------------------
 */
int soar_ecCloseLog()
{

    if (soar_exists_callback(soar_agent, LOG_CALLBACK)) {
        soar_invoke_first_callback(soar_agent, LOG_CALLBACK, "\n**** log closed ****\n");
        soar_cPopCallback(soar_agent, LOG_CALLBACK);

        return 0;
    }

    return -1;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_ecCaptureInput
 *
 *       Captures the input sent to the agent during by external
 *       calls to add-wme and remove-wme
 *----------------------------------------------------------------------
 */
#ifdef USE_CAPTURE_REPLAY
int soar_ecCaptureInput(const char *filename)
{

    FILE *f;

    if (filename == NIL) {
        /* Trying to close */
        if (!current_agent(capture_fileID)) {
            return -1;
        }
        fclose(current_agent(capture_fileID));
        current_agent(capture_fileID) = NIL;
        return 0;
    } else {
        if (current_agent(capture_fileID)) {
            return -2;
        }
        f = fopen(filename, "w");
        if (!f) {
            return -3;
        } else {
            current_agent(capture_fileID) = f;
            fprintf(f, "##soar captured input file.\n");
            return 0;
        }
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_ecReplayInput
 *
 *       Replays the input previously captured using the CaptureInput 
 *       command
 *----------------------------------------------------------------------
 */
int soar_ecReplayInput(const char *filename)
{

    if (filename == NIL) {
        soar_cRemoveInputFunction(soar_agent, "replay-input");

    } else {

        /* Read in the contents of the file... */
        long begin, end;
        char input[1024];
        char id[1024];
        char attr[1024];
        char value[1024];
        int action;
        int numargs, cycle;
        int hashfreq, lasthash;
        unsigned long old_timetag, tt;
        int ndc;
        captured_action *c_action = NIL;
        int last_dc;
        int actions_in_dc = 0;
        int i;
        FILE *f;
        char header[80];
        soarapi_wme *sapiw;

        f = fopen(filename, "r");
        if (!f) {
            return -1;
        } else {
            fgets(header, 28, f);
            if (strcmp(header, "##soar captured input file.") != 0) {
                return -2;
            }

            /* If we've made it here, things should be ok */

            soar_cAddInputFunction(soar_agent, replay_input_wme, NULL, NULL, "replay-input");

            ndc = -1;
            tt = 1;
            begin = ftell(f);
            while (!feof(f)) {
                numargs = fscanf(f, "%i : %*d : %*s : %ld", &cycle, &old_timetag);
                if (old_timetag > tt)
                    tt = old_timetag;
                if (cycle > ndc)
                    ndc = cycle;
                fgets(input, 1024, f);
            }

            end = ftell(f);     /* Save the location of the end of the file */

            hashfreq = (end - begin) / 10;
            lasthash = 0;

            /* print( "Max timetag is %d\n", tt ); */
            current_agent(replay_timetags) = (unsigned long *) allocate_memory(tt * sizeof(tt),
                                                                               current_agent(memory_for_usage)
                                                                               [MISCELLANEOUS_MEM_USAGE]);

            current_agent(dc_to_replay) = ndc;
            current_agent(replay_actions) = (captured_action **) allocate_memory(((ndc +
                                                                                   1) * sizeof(captured_action *)),
                                                                                 current_agent(memory_for_usage)
                                                                                 [MISCELLANEOUS_MEM_USAGE]);

            for (i = 0; i < ndc; i++) {
                current_agent(replay_actions)[i] = NULL;
            }

            fseek(f, begin, SEEK_SET);

            /* Now fill in the actual actions */
            last_dc = -1;
            while (!feof(f)) {
                numargs = fscanf(f, "%i : %i : %*s :", &cycle, &action);

                if (numargs < 2)
                    break;

                begin = ftell(f);       /* get the current position */
                if (begin / hashfreq > lasthash) {
                    lasthash = begin / hashfreq;
                    print("+");
                }

                if (cycle != last_dc) {
                    current_agent(replay_actions)[cycle] =
                        (captured_action *) allocate_memory(sizeof(captured_action),
                                                            current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);
                    c_action = current_agent(replay_actions)[cycle];

                    if (current_agent(soar_verbose_flag) && last_dc != -1) {
                        print(" -- (%d actions)\n", actions_in_dc);
                    }

                    actions_in_dc = 1;
                    last_dc = cycle;
                    c_action->next = NULL;

                    if (current_agent(soar_verbose_flag))
                        print("Building Actions for Decision Cycle %d\n", cycle);

                } else {
                    c_action->next = (captured_action *)
                        allocate_memory(sizeof(captured_action),
                                        current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);

                    c_action = c_action->next;
                    c_action->next = NULL;
                    actions_in_dc++;
                }

                c_action->dc = cycle;
                c_action->action = action;

                switch (action) {

                case ADD_WME:
                    if (current_agent(soar_verbose_flag))
                        print("  -- Reading ADD Wme Action\n");

                    fscanf(f, "%ld : %s %s %s", &old_timetag, id, attr, value);

                    c_action->args = malloc(sizeof(soarapi_wme));
                    sapiw = (soarapi_wme *) c_action->args;

                    sapiw->id = savestring(id);
                    sapiw->attr = savestring(attr);
                    sapiw->value = savestring(value);
                    sapiw->timetag = old_timetag;

                    break;

                case REMOVE_WME:
                    if (current_agent(soar_verbose_flag))
                        print("  -- Reading REMOVE Wme Action\n");

                    fscanf(f, " : %ld", &old_timetag);
                    c_action->args = malloc(sizeof(soarapi_wme));
                    ((soarapi_wme *) c_action->args)->timetag = old_timetag;
                    break;

                default:
                    print("Warning: don't know what to do with an action type of %d\n", action);
                    break;

                }
                fgets(input, 1025, f);

            }
            if (current_agent(soar_verbose_flag))
                print(" -- (%d actions)\n", actions_in_dc);
        }
        fclose(f);
    }
    return 0;
}
#endif

/* REW: begin 09.15.96 */
/*
 *----------------------------------------------------------------------
 *
 * soar_ecGDSPrint --
 *
 *----------------------------------------------------------------------
 */

void soar_ecGDSPrint()
{
    wme *w;
    Symbol *goal;

    print("********************* Current GDS **************************\n");
    print("stepping thru all wmes in rete, looking for any that are in a gds...\n");
    for (w = current_agent(all_wmes_in_rete); w != NIL; w = w->rete_next) {
        if (w->gds) {
            if (w->gds->goal) {
                print_with_symbols("  For Goal  %y  ", w->gds->goal);
            } else {
                print("  Old GDS value ");
            }
            print("(%lu: ", w->timetag);
            print_with_symbols("%y ^%y %y", w->id, w->attr, w->value);
            if (w->acceptable)
                print_string(" +");
            print_string(")");
            print("\n");
        }
    }
    print("************************************************************\n");
    for (goal = current_agent(top_goal); goal != NIL; goal = goal->id.lower_goal) {
        print_with_symbols("  For Goal  %y  ", goal);
        if (goal->id.gds) {
            /* Loop over all the WMEs in the GDS */
            print("\n");
            for (w = goal->id.gds->wmes_in_gds; w != NIL; w = w->gds_next) {
                print("                (%lu: ", w->timetag);
                print_with_symbols("%y ^%y %y", w->id, w->attr, w->value);
                if (w->acceptable)
                    print_string(" +");
                print_string(")");
                print("\n");
            }

        } else
            print(": No GDS for this goal.\n");
    }

    print("************************************************************\n");

}

/* REW: end   09.15.96 */

void soar_ecExplainChunkTrace(char *chunk_name)
{

    explain_chunk_str *chunk;

    chunk = find_chunk(current_agent(explain_chunk_list), chunk_name);
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

    if (chunk)
        explain_trace_chunk(chunk);
}

void soar_ecExplainChunkCondition(char *chunk_name, int cond_number)
{

    explain_chunk_str *chunk;
    condition *ground;

    chunk = find_chunk(current_agent(explain_chunk_list), chunk_name);
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

    if (chunk == NULL)
        return;

    ground = find_ground(chunk, cond_number);
    if (ground == NIL)
        return;

    explain_trace(chunk_name, chunk->backtrace, ground);
}

void soar_ecExplainChunkConditionList(char *chunk_name)
{

    explain_chunk_str *chunk;
    condition *cond, *ground;
    int i;

    chunk = find_chunk(current_agent(explain_chunk_list), chunk_name);
/* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */

    if (chunk == NULL)
        return;

    /* First print out the production in "normal" form */

    print("(sp %s\n  ", chunk->name);
    print_condition_list(chunk->conds, 2, FALSE);
    print("\n-->\n   ");
    print_action_list(chunk->actions, 3, FALSE);
    print(")\n\n");

    /* Then list each condition and the associated "ground" WME */

    i = 0;
    ground = chunk->all_grounds;

    for (cond = chunk->conds; cond != NIL; cond = cond->next) {
        i++;
        print(" %2d : ", i);
        print_condition(cond);
        while (get_printer_output_column() < COLUMNS_PER_LINE - 40)
            print(" ");

        print(" Ground :");
        print_condition(ground);
        print("\n");
        ground = ground->next;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_ecPrintFiringsForProduction
 *
 *   Print the number of times the specified production has fired, or
 *   "No production named <name>" if the specified production is not
 *   defined.
 *
 *----------------------------------------------------------------------
 */
void soar_ecPrintFiringsForProduction(const char *name)
{

    production *p;

    p = name_to_production(name);
    if (p) {
        print("%6lu:  %s\n", p->firing_count, name);
    } else {
        print("No production named %s", name);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_ecPrintTopProductionFirings
 *
 *   Print the top <n> productions with respect to their frequency of
 *   firing or "*** No productions defined ***" if no productions are
 *   defined.
 *----------------------------------------------------------------------
 */
void soar_ecPrintTopProductionFirings(int n)
{
    int i;
    long num_prods;
    production *((*all_prods)[]), **ap_item, *p;

    num_prods = current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
        current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
        current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE];

    if (num_prods == 0) {
        print("*** No productions Defined ***\n");
        return;
    }

    /* --- make an array of pointers to all the productions --- */
    all_prods = allocate_memory(num_prods * sizeof(production *), MISCELLANEOUS_MEM_USAGE);

    /* MVP - 6-8-94 where is it freed ? */

    ap_item = &((*all_prods)[0]);
    for (p = current_agent(all_productions_of_type)[DEFAULT_PRODUCTION_TYPE]; p != NIL; p = p->next)
        *(ap_item++) = p;
    for (p = current_agent(all_productions_of_type)[USER_PRODUCTION_TYPE]; p != NIL; p = p->next)
        *(ap_item++) = p;
    for (p = current_agent(all_productions_of_type)[CHUNK_PRODUCTION_TYPE]; p != NIL; p = p->next)
        *(ap_item++) = p;

    /* --- now print out the results --- */
    if (n == 0) {
        /* print only the productions that have never fired. */
        ap_item = &((*all_prods)[0]);
        for (i = 0; i < num_prods; i++) {
            if ((*ap_item)->firing_count == 0) {
                print_with_symbols("%y\n", (*ap_item)->name);
            }
            ap_item++;
        }
    } else {
        /* --- sort array according to firing counts --- */
        qsort(all_prods, num_prods, sizeof(production *), compare_firing_counts);

        if ((n < 0) || (n > num_prods)) {
            n = num_prods;
        }

        ap_item = &((*all_prods)[num_prods - 1]);
        while (n) {
            print("%6lu:  ", (*ap_item)->firing_count);
            print_with_symbols("%y\n", (*ap_item)->name);
            ap_item--;
            n--;
        }
    }

    /* MVP 6-8-94 also try this to plug memory leak */
    free_memory(all_prods, MISCELLANEOUS_MEM_USAGE);

}

void soar_exPrintMemoryPoolStatistics(void)
{
    memory_pool *p;

#ifdef MEMORY_POOL_STATS
    long total_items;
#endif

    print("Memory pool statistics:\n\n");
#ifdef MEMORY_POOL_STATS
    print("Pool Name        Used Items  Free Items  Item Size  Total Bytes\n");
    print("---------------  ----------  ----------  ---------  -----------\n");
#else
    print("Pool Name        Item Size  Total Bytes\n");
    print("---------------  ---------  -----------\n");
#endif

    for (p = current_agent(memory_pools_in_use); p != NIL; p = p->next) {
        print_string(p->name);
        print_spaces(MAX_POOL_NAME_LENGTH - strlen(p->name));
#ifdef MEMORY_POOL_STATS
        print("  %10lu", p->used_count);
        total_items = p->num_blocks * p->items_per_block;
        print("  %10lu", total_items - p->used_count);
#endif
        print("  %9lu", p->item_size);
        print("  %11lu\n", p->num_blocks * p->items_per_block * p->item_size);
    }
}

void soar_ecPrintMemoryStatistics(void)
{
    unsigned long total;
    int i;

    total = 0;
    for (i = 0; i < NUM_MEM_USAGE_CODES; i++)
        total += current_agent(memory_for_usage)[i];

    print("%8lu bytes total memory allocated\n", total);
    print("%8lu bytes statistics overhead\n", current_agent(memory_for_usage)[STATS_OVERHEAD_MEM_USAGE]);
    print("%8lu bytes for strings\n", current_agent(memory_for_usage)[STRING_MEM_USAGE]);
    print("%8lu bytes for hash tables\n", current_agent(memory_for_usage)[HASH_TABLE_MEM_USAGE]);
    print("%8lu bytes for various memory pools\n", current_agent(memory_for_usage)[POOL_MEM_USAGE]);
    print("%8lu bytes for miscellaneous other things\n", current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);
}

void soar_ecPrintReteStatistics(void)
{

#ifdef TOKEN_SHARING_STATS
    print("Token additions: %lu   If no sharing: %lu\n",
          current_agent(token_additions), current_agent(token_additions_without_sharing));
#endif

    print_node_count_statistics();
    print_null_activation_stats();
}

void soar_ecPrintSystemStatistics(void)
{

    unsigned long wme_changes;

    /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
    double total_kernel_time, total_kernel_msec, derived_kernel_time, monitors_sum, input_function_time, input_phase_total_time, output_function_time, output_phase_total_time, determine_level_phase_total_time,       /* REW: end   05.05.97 */
     preference_phase_total_time, wm_phase_total_time, decision_phase_total_time, derived_total_cpu_time;

#ifdef DETAILED_TIMING_STATS
    double match_time, match_msec;
    double ownership_time, chunking_time;
    double other_phase_kernel_time[6], other_total_kernel_time;
#endif
#endif
    /* REW: end 28.07.96 */

    /* MVP 6-8-94 */
    char hostname[MAX_LEXEME_LENGTH + 1];
    long current_time;

#if !defined (THINK_C) && !defined (__SC__) && !defined(MACINTOSH) && !defined(WIN32) && !defined(_WINDOWS)
    if (gethostname(hostname, MAX_LEXEME_LENGTH)) {
#endif

        strncpy(hostname, "[host name unknown]", MAX_LEXEME_LENGTH + 1);
        hostname[MAX_LEXEME_LENGTH] = 0;

#if !defined (THINK_C) && !defined (__SC__) && !defined(MACINTOSH) && !defined(WIN32) && !defined(_WINDOWS)
    }
#endif

    current_time = time(NULL);

/* REW: begin 28.07.96 */
/* See note in soarkernel.h for a description of the timers */
#ifndef NO_TIMING_STUFF
    total_kernel_time = timer_value(&current_agent(total_kernel_time));
    total_kernel_msec = total_kernel_time * 1000.0;

    /* derived_kernel_time := Total of the time spent in the phases of the decision cycle, 
       excluding Input Function, Output function, and pre-defined callbacks. 
       This computed time should be roughly equal to total_kernel_time, 
       as determined above. */

#ifndef KERNEL_TIME_ONLY
    derived_kernel_time = timer_value(&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[WM_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[DECISION_PHASE]));

    input_function_time = timer_value(&current_agent(input_function_cpu_time));

    output_function_time = timer_value(&current_agent(output_function_cpu_time));

    /* Total of the time spent in callback routines. */
    monitors_sum = timer_value(&current_agent(monitors_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[PREFERENCE_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[WM_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[DECISION_PHASE]));

    derived_total_cpu_time = derived_kernel_time + monitors_sum + input_function_time + output_function_time;

    /* Total time spent in the input phase */
    input_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(input_function_cpu_time));

    /* REW: begin 10.30.97 */
    determine_level_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE]));
    /* REW: end   10.30.97 */

    /* Total time spent in the preference phase */
    preference_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[PREFERENCE_PHASE]));

    /* Total time spent in the working memory phase */
    wm_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[WM_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[WM_PHASE]));

    /* Total time spent in the output phase */
    output_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(output_function_cpu_time));

    /* Total time spent in the decision phase */
    decision_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[DECISION_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[DECISION_PHASE]));

    /* The sum of these six phase timers is exactly equal to the 
     * derived_total_cpu_time
     */

#ifdef DETAILED_TIMING_STATS

    match_time = timer_value(&current_agent(match_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE]))
        + timer_value(&current_agent(match_cpu_time[PREFERENCE_PHASE]))
        + timer_value(&current_agent(match_cpu_time[WM_PHASE]))
        + timer_value(&current_agent(match_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(match_cpu_time[DECISION_PHASE]));

    match_msec = 1000 * match_time;

    ownership_time = timer_value(&current_agent(ownership_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[PREFERENCE_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[WM_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[DECISION_PHASE]));

    chunking_time = timer_value(&current_agent(chunking_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[PREFERENCE_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[WM_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[DECISION_PHASE]));

    /* O-support time should go to 0 with o-support-mode 2 */
    /* o_support_time = 
       timer_value (&current_agent(o_support_cpu_time[INPUT_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[DETERMINE_LEVEL_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[PREFERENCE_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[WM_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[OUTPUT_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[DECISION_PHASE])); */

    other_phase_kernel_time[INPUT_PHASE] = timer_value(&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
        - timer_value(&current_agent(match_cpu_time[INPUT_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[INPUT_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[INPUT_PHASE]));

    other_phase_kernel_time[DETERMINE_LEVEL_PHASE] =
        timer_value(&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]))
        - timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE]));

    other_phase_kernel_time[PREFERENCE_PHASE] =
        timer_value(&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]))
        - timer_value(&current_agent(match_cpu_time[PREFERENCE_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[PREFERENCE_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[PREFERENCE_PHASE]));

    other_phase_kernel_time[WM_PHASE] = timer_value(&current_agent(decision_cycle_phase_timers[WM_PHASE]))
        - timer_value(&current_agent(match_cpu_time[WM_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[WM_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[WM_PHASE]));

    other_phase_kernel_time[OUTPUT_PHASE] = timer_value(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]))
        - timer_value(&current_agent(match_cpu_time[OUTPUT_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[OUTPUT_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[OUTPUT_PHASE]));

    other_phase_kernel_time[DECISION_PHASE] = timer_value(&current_agent(decision_cycle_phase_timers[DECISION_PHASE]))
        - timer_value(&current_agent(match_cpu_time[DECISION_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[DECISION_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[DECISION_PHASE]));

    other_total_kernel_time = other_phase_kernel_time[INPUT_PHASE]
        + other_phase_kernel_time[DETERMINE_LEVEL_PHASE]
        + other_phase_kernel_time[PREFERENCE_PHASE]
        + other_phase_kernel_time[WM_PHASE]
        + other_phase_kernel_time[OUTPUT_PHASE]
        + other_phase_kernel_time[DECISION_PHASE];

#endif
#endif
#endif
/* REW: end 28.07.96 */

    print("Soar %s on %s at %s\n", soar_version_string, hostname, ctime((const time_t *) &current_time));

    print("%lu productions (%lu default, %lu user, %lu chunks)\n",
          current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
          current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
          current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE],
          current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE],
          current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE],
          current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);
    print("   + %lu justifications\n", current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);

    /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
    /* The fields for the timers are 8.3, providing an upper limit of 
       approximately 2.5 hours the printing of the run time calculations.  
       Obviously, these will need to be increased if you plan on needing 
       run-time data for a process that you expect to take longer than 
       2 hours. :) */

#ifndef KERNEL_TIME_ONLY
    print("                                                                |    Derived\n");
    print("Phases:      Input      DLP     Pref      W/M   Output Decision |     Totals\n");
    print("================================================================|===========\n");

    print("Kernel:   %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(decision_cycle_phase_timers[INPUT_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[WM_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[DECISION_PHASE])), derived_kernel_time);

#ifdef DETAILED_TIMING_STATS

    print("====================  Detailed Timing Statistics  ==============|===========\n");

    print("   Match: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(match_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(match_cpu_time[PREFERENCE_PHASE])),
          timer_value(&current_agent(match_cpu_time[WM_PHASE])),
          timer_value(&current_agent(match_cpu_time[OUTPUT_PHASE])),
          timer_value(&current_agent(match_cpu_time[DECISION_PHASE])), match_time);

    print("Own'ship: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(ownership_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[PREFERENCE_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[WM_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[OUTPUT_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[DECISION_PHASE])), ownership_time);

    print("Chunking: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(chunking_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[PREFERENCE_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[WM_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[OUTPUT_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[DECISION_PHASE])), chunking_time);

    print("   Other: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          other_phase_kernel_time[INPUT_PHASE],
          other_phase_kernel_time[DETERMINE_LEVEL_PHASE],
          other_phase_kernel_time[PREFERENCE_PHASE],
          other_phase_kernel_time[WM_PHASE],
          other_phase_kernel_time[OUTPUT_PHASE], other_phase_kernel_time[DECISION_PHASE], other_total_kernel_time);

    /* REW: begin 11.25.96 */
    print("Operand2: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(gds_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(gds_cpu_time[PREFERENCE_PHASE])),
          timer_value(&current_agent(gds_cpu_time[WM_PHASE])),
          timer_value(&current_agent(gds_cpu_time[OUTPUT_PHASE])),
          timer_value(&current_agent(gds_cpu_time[DECISION_PHASE])),
          timer_value(&current_agent(gds_cpu_time[INPUT_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[PREFERENCE_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[WM_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[OUTPUT_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[DECISION_PHASE])));

    /* REW: end   11.25.96 */

#endif

    print("================================================================|===========\n");
    print("Input fn: %8.3f                                              | %10.3f\n",
          input_function_time, input_function_time);

    print("================================================================|===========\n");
    print("Outpt fn:                                     %8.3f          | %10.3f\n",
          output_function_time, output_function_time);

    print("================================================================|===========\n");
    print("Callbcks: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(monitors_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(monitors_cpu_time[PREFERENCE_PHASE])),
          timer_value(&current_agent(monitors_cpu_time[WM_PHASE])),
          timer_value(&current_agent(monitors_cpu_time[OUTPUT_PHASE])),
          timer_value(&current_agent(monitors_cpu_time[DECISION_PHASE])), monitors_sum);

    print("================================================================|===========\n");
    print("Derived---------------------------------------------------------+-----------\n");
    print("Totals:   %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n\n",
          input_phase_total_time,
          determine_level_phase_total_time,
          preference_phase_total_time,
          wm_phase_total_time, output_phase_total_time, decision_phase_total_time, derived_total_cpu_time);

    if (!current_agent(stop_soar)) {
        /* Soar is still running, so this must have been invoked
         * from the RHS, therefore these timers need to be updated. */
        stop_timer(&current_agent(start_total_tv), &current_agent(total_cpu_time));
        stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
        start_timer(&current_agent(start_total_tv));
        start_timer(&current_agent(start_kernel_tv));
    }

    print("Values from single timers:\n");
#endif
#endif
#ifndef NO_TIMING_STUFF

#ifdef WARN_IF_TIMERS_REPORT_ZERO
    /* If a warning has occured since the last init-soar, the warn flag will
     * have been set to FALSE, so such a warning is not spammed to the screen
     * But lets repeat it here.
     */
    if (!current_agent(warn_on_zero_timers))
        print(" Warning: one or more timers have reported zero during this run\n");
#endif

#ifndef PII_TIMERS
    print(" Kernel CPU Time: %11.3f sec. \n", total_kernel_time);
    print(" Total  CPU Time: %11.3f sec.\n\n", timer_value(&current_agent(total_cpu_time)));
#else
    print(" Using PII Timers ... Assuming Processor Speed of %d MHZ\n", MHZ);
    print(" Kernel CPU Time: %11.5f sec. \n", total_kernel_time);
    print(" Total  CPU Time: %11.5f sec.\n\n", timer_value(&current_agent(total_cpu_time)));

#endif

#ifdef COUNT_KERNEL_TIMER_STOPS
    print(" Kernel CPU Timer Stops: %d\n", current_agent(kernelTimerStops));
    print(" Non-Kernel Timer Stops: %d\n", current_agent(nonKernelTimerStops));

#endif
#endif

#if !defined(NO_TIMING_STUFF)
    print("%lu decision cycles (%.3f msec/dc)\n",
          current_agent(d_cycle_count),
          current_agent(d_cycle_count) ? total_kernel_msec / current_agent(d_cycle_count) : 0.0);
    print("%lu elaboration cycles (%.3f ec's per dc, %.3f msec/ec)\n",
          current_agent(e_cycle_count),
          current_agent(d_cycle_count) ? (double) current_agent(e_cycle_count) / current_agent(d_cycle_count) : 0,
          current_agent(e_cycle_count) ? total_kernel_msec / current_agent(e_cycle_count) : 0);
    /* REW: begin 09.15.96 */

#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode))
#endif
        print("%lu p-elaboration cycles (%.3f pe's per dc, %.3f msec/pe)\n",
              current_agent(pe_cycle_count),
              current_agent(d_cycle_count) ? (double) current_agent(pe_cycle_count) / current_agent(d_cycle_count) : 0,
              current_agent(pe_cycle_count) ? total_kernel_msec / current_agent(pe_cycle_count) : 0);
    /* REW: end 09.15.96 */
    print("%lu production firings (%.3f pf's per ec, %.3f msec/pf)\n",
          current_agent(production_firing_count),
          current_agent(e_cycle_count) ? (double) current_agent(production_firing_count) /
          current_agent(e_cycle_count) : 0.0,
          current_agent(production_firing_count) ? total_kernel_msec / current_agent(production_firing_count) : 0.0);

#else
    print("%lu decision cycles\n", current_agent(d_cycle_count));
    print("%lu elaboration cycles \n", current_agent(e_cycle_count));
    print("%lu production firings \n", current_agent(production_firing_count));
#endif                          /* !NO_TIMING_STUFF */

    wme_changes = current_agent(wme_addition_count) + current_agent(wme_removal_count);
    print("%lu wme changes (%lu additions, %lu removals)\n",
          wme_changes, current_agent(wme_addition_count), current_agent(wme_removal_count));
#ifdef DETAILED_TIMING_STATS
    print("    match time: %.3f msec/wm change\n", wme_changes ? match_msec / wme_changes : 0.0);
#endif

    print("WM size: %lu current, %.3f mean, %lu maximum\n",
          current_agent(num_wmes_in_rete),
          (current_agent(num_wm_sizes_accumulated) ?
           (current_agent(cumulative_wm_size) / current_agent(num_wm_sizes_accumulated)) :
           0.0), current_agent(max_wm_size));

#ifndef NO_TIMING_STUFF
    print("\n");
    print("    *** Time/<x> statistics use the total kernel time from a ***\n");
    print("    *** single kernel timer.  Differences between this value ***\n");
    print("    *** and the derived total kernel time  are expected. See ***\n");
    print("    *** help  for the  stats command  for more  information. ***\n");
#endif
    /* REW: end 28.07.96 */
}

#ifdef DC_HISTOGRAM
/*
 *----------------------------------------------------------------------
 *
 * soar_ecPrintDCHistogram
 *
 *	This procedure prints the timing of decision cycles
 *
 * Results:
 *	Soar status code.
 *
 * Side effects:
 *
 *----------------------------------------------------------------------
 */
int soar_ecPrintDCHistogram(void)
{

    double total_k_time;
    int i, end;

    total_k_time = 0;
    end = current_agent(d_cycle_count) / current_agent(dc_histogram_freq);
    if (end > current_agent(dc_histogram_sz))
        end = current_agent(dc_histogram_sz);

    print("------------- D.C. Histogram --------------\n");
    print("Each Bucket encapsulates %d decision cycles\n", current_agent(dc_histogram_freq));
    print("%d Buckets have been filled during %d decision cycles\n", end, current_agent(d_cycle_count));
    print("-------------------------------------------\n");
    print("Bucket\t1st DC\tTime\n");
    for (i = 0; i < end; i++) {

        print("%d\t%d\t%8.5f\n", i, i * current_agent(dc_histogram_freq),
              timer_value(&current_agent(dc_histogram_tv)[i]));

        total_k_time += timer_value(&current_agent(dc_histogram_tv)[i]);
    }
    print("Total Time In Buckets:\t%8.5f\n", total_k_time);

    return SOAR_OK;
}
#endif

#ifdef KT_HISTOGRAM
/*
 *----------------------------------------------------------------------
 *
 * soar_ecPrintKTHistogram
 *
 *	This procedure prints the timing of decision cycles
 *
 * Results:
 *	Soar status code.
 *
 * Side effects:
 *
 *----------------------------------------------------------------------
 */
int soar_ecPrintKTHistogram(void)
{
    double total_k_time;
    int i, end;

    total_k_time = 0;
    end = current_agent(d_cycle_count);
    if (end > current_agent(dc_histogram_sz))
        end = current_agent(dc_histogram_sz);

    print("------------- K.T. Histogram --------------\n");

    print("Bucket\tTime\n");
    for (i = 0; i < end; i++) {

        print("%d\t%8.5f\n", i, timer_value(&current_agent(kt_histogram_tv)[i]));

        total_k_time += timer_value(&current_agent(kt_histogram_tv)[i]);
    }
    print("Total Time In Buckets:\t%8.5f\n", total_k_time);

    return SOAR_OK;
}
#endif

void soar_ecPrintMemories(int num_to_print, int to_print[])
{
    int i, num_prods;
    production_memory_use *temp, *first, *tempnext;
    production *prod;
    production_memory_use *print_memories_insert_in_list();

    print("\nMemory use for productions:\n\n");

    /* Start by doing ALL of them. */
    first = NULL;
    num_prods = 0;

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
        if (to_print[i])
            for (prod = current_agent(all_productions_of_type)[i]; prod != NIL; prod = prod->next) {
                temp = allocate_memory(sizeof(production_memory_use), MISCELLANEOUS_MEM_USAGE);
                temp->next = NULL;
                temp->name = prod->name;

                temp->mem = count_rete_tokens_for_production(prod);

                first = print_memories_insert_in_list(temp, first);
                num_prods++;
            }

    i = 0;
    if (num_to_print < 0)
        num_to_print = num_prods;

    for (temp = first; ((temp != NULL) && (i < num_to_print)); temp = tempnext) {
        print_with_symbols("%y: ", temp->name);
        print("%d\n", temp->mem);
        tempnext = temp->next;
        free_memory(temp, MISCELLANEOUS_MEM_USAGE);
        i++;
    }
}

int soar_ecPrintAllProductionsOfType(int type, bool internal, bool print_fname, bool full_prod)
{

    production *prod;

    /* we'll step through the list backwards, so chunks and justifications
     * are displayed in reverse cronological order.
     */

    if (type < 0 || type >= NUM_PRODUCTION_TYPES) {
        return -1;
    }

    for (prod = current_agent(all_productions_of_type)[type]; prod != NIL && prod->next != NIL; prod = prod->next)
        /* intentionally null */ ;

    while (prod != NIL) {
        /* Print it... */
        if (!full_prod) {
            print_with_symbols("%y  ", prod->name);
        }
        if (print_fname) {
            print_string("# sourcefile : ");
            if (prod->filename) {
                print_string(prod->filename);
            } else {
                print_string(" _unknown_ ");
            }
        }
        print("\n");
        if (full_prod) {
            print_production(prod, internal);
            print("\n");
        }
        prod = prod->prev;
    }

    return 0;
}

void soar_ecPrintAllProductionsWithInterruptSetting(enum soar_apiInterruptSetting interrupt_setting) {
    production *prod;

    /* we'll step through the list backwards, so chunks and justifications
     * are displayed in reverse cronological order.
     */

    int type;
    for (type = 0; type < NUM_PRODUCTION_TYPES; type++) {
        for (prod = current_agent(all_productions_of_type)[type]; prod != NIL && prod->next != NIL; prod = prod->next)
            /* intentionally null */ ;

        while (prod != NIL) {
            /* Print it... */

            if( (interrupt_setting == INTERRUPT_ON && prod->interrupt) || (interrupt_setting == INTERRUPT_OFF && !prod->interrupt) ) {
                print("%s\n", prod->name->var.name);
            }
        
            prod = prod->prev;
        }
    }
}

int soar_ecAddWmeFilter(const char *szId, const char *szAttr, const char *szValue, bool adds, bool removes)
{

    Symbol *id, *attr, *value;
    wme_filter *wf, *existing_wf;
    cons *c;
    int return_value;

    id = NIL;
    attr = NIL;
    value = NIL;
    return_value = 0;

    if (read_wme_filter_component(szId, &id)) {
        return_value = -1;
        goto error_out;
    }
    if (read_wme_filter_component(szAttr, &attr)) {
        return_value = -2;
        goto error_out;
    }
    if (read_wme_filter_component(szValue, &value)) {
        return_value = -3;
        goto error_out;
    }

    if (id && attr && value) {
        /* check to see if such a filter has already been added: */
        for (c = current_agent(wme_filter_list); c != NIL; c = c->rest) {
            existing_wf = (wme_filter *) c->first;
            if ((existing_wf->adds == adds) && (existing_wf->removes == removes)
                && (existing_wf->id == id) && (existing_wf->attr == attr)
                && (existing_wf->value == value)) {

                print("Filter already exists.\n");
                return_value = -4;
                goto error_out; /* already exists */
            }
        }

        wf = allocate_memory(sizeof(wme_filter), MISCELLANEOUS_MEM_USAGE);
        wf->id = id;
        wf->attr = attr;
        wf->value = value;
        wf->adds = adds;
        wf->removes = removes;

        /* Rather than add refs for the new filter symbols and then remove refs 
         * for the identical symbols created from the string parameters, skip
         * the two nullifying steps altogether and just return immediately
         * after pushing the new filter:
         */
        push(wf, current_agent(wme_filter_list));       /* note: nested macro */
        return 0;
    }
  error_out:
    /* clean up symbols created from string parameters */
    if (id)
        symbol_remove_ref(id);
    if (attr)
        symbol_remove_ref(attr);
    if (value)
        symbol_remove_ref(value);
    return return_value;
}

int soar_ecRemoveWmeFilter(const char *szId, const char *szAttr, const char *szValue, bool adds, bool removes)
{
    Symbol *id, *attr, *value;
    wme_filter *wf;
    int return_value = -4;
    cons *c;
    cons **prev_cons_rest;

    id = NIL;
    attr = NIL;
    value = NIL;

    if (read_wme_filter_component(szId, &id)) {
        return_value = -1;
        goto clean_up;
    }
    if (read_wme_filter_component(szAttr, &attr)) {
        return_value = -2;
        goto clean_up;
    }
    if (read_wme_filter_component(szValue, &value)) {
        return_value = -3;
        goto clean_up;
    }

    if (id && attr && value) {
        prev_cons_rest = &current_agent(wme_filter_list);
        for (c = current_agent(wme_filter_list); c != NIL; c = c->rest) {
            wf = (wme_filter *) c->first;
            if (((adds && wf->adds) || ((removes) && wf->removes))
                && (wf->id == id) && (wf->attr == attr) && (wf->value == value)) {
                *prev_cons_rest = c->rest;
                symbol_remove_ref(id);
                symbol_remove_ref(attr);
                symbol_remove_ref(value);
                free_memory(wf, MISCELLANEOUS_MEM_USAGE);
                free_cons(c);
                break;          /* assume that soar_ecAddWmeFilter did not add duplicates */
            }
            prev_cons_rest = &(c->rest);
        }
        if (c != NIL)
            return_value = 0;   /* filter was sucessfully removed */
    }

  clean_up:
    /* clean up symbols created from string parameters */
    if (id)
        symbol_remove_ref(id);
    if (attr)
        symbol_remove_ref(attr);
    if (value)
        symbol_remove_ref(value);
    return return_value;
}

int soar_ecResetWmeFilters(bool adds, bool removes)
{
    wme_filter *wf;
    cons *c;
    cons **prev_cons_rest;
    bool didRemoveSome;

    didRemoveSome = FALSE;
    prev_cons_rest = &current_agent(wme_filter_list);
    for (c = current_agent(wme_filter_list); c != NIL; c = c->rest) {
        wf = (wme_filter *) c->first;
        if ((adds && wf->adds) || (removes && wf->removes)) {
            *prev_cons_rest = c->rest;
            print_with_symbols("Removed: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
            print("%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
            symbol_remove_ref(wf->id);
            symbol_remove_ref(wf->attr);
            symbol_remove_ref(wf->value);
            free_memory(wf, MISCELLANEOUS_MEM_USAGE);
            free_cons(c);
            didRemoveSome = TRUE;
        }
        prev_cons_rest = &(c->rest);
    }
    if (didRemoveSome)
        return 0;
    else
        return -1;
}

void soar_ecListWmeFilters(bool adds, bool removes)
{
    wme_filter *wf;
    cons *c;

    for (c = current_agent(wme_filter_list); c != NIL; c = c->rest) {
        wf = (wme_filter *) c->first;
        if ((adds && wf->adds) || (removes && wf->removes)) {
            print_with_symbols("wme filter: (%y ^%y %y) ", wf->id, wf->attr, wf->value);

            print("%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
        }
    }
}

int soar_ecSp(const char *rule, const char *sourceFile)
{

    production *p;

    /* Use a callback instead? */
    /* ((agent *)clientData)->alternate_input_string = argv[1];
     * ((agent *)clientData)->alternate_input_suffix = ") ";
     *//* Soar-Bugs #54 TMH */
    soar_alternate_input(soar_agent, rule, ") ", TRUE);
    set_lexer_allow_ids(FALSE);
    get_lexeme();
    p = parse_production();
    set_lexer_allow_ids(TRUE);
    soar_alternate_input(soar_agent, NIL, NIL, FALSE);

    if (p) {

        if (sourceFile != NULL) {
            p->filename = make_memory_block_for_string(sourceFile);
        }

        if (current_agent(sysparams)[TRACE_LOADING_SYSPARAM])
            print("*");

        /* kjh CUSP(B14) begin */
        if (p->type == CHUNK_PRODUCTION_TYPE) {

            /* Extract chunk_count from chunk name.
             * It will always be the number before the first '*' or '\0' 
             * and after a '-'
             * (e.g. chunk-14*d8*tie*2 or chunk-123).
             * This hack eliminates long delays that would arise when generating
             *  new chunk names after having loaded in old chunks.  By extracting
             *  chunk_count here, the time spent looping and checking for pre-
             *  existing names is minimized.
             */
            char *chunk_name, *c;
            unsigned long this_chunk_count;

            chunk_name = p->name->sc.name;
            c = chunk_name;
            while (*c && (*c != '*'))
                c++;
            do
                c--;
            while (*c && (*c != '-'));
            c++;
            if (sscanf(c, "%lu", &this_chunk_count) != 1)
                print("Warning: failed to extract chunk_num from chunk \"%s\"\n", chunk_name);
            else if (this_chunk_count > current_agent(chunk_count)) {
                current_agent(chunk_count) = this_chunk_count;
                print("updated chunk_num=%lu\n", current_agent(chunk_count));
            }
        }
        /* kjh CUSP(B14) end */

        return 0;               /* OK */
    } else {
        /* DJP : Modified to make all sp errors appear to be non-fatal */
        /*       This is necessary because currently warnings are causing */
        /*       Soar to stop loading files, which is clearly wrong.  */
        /*       E.g. ignoring production P1 because it's a duplicate of P2 */

        return -1;              /* Non Fatal Error */

        /* THIS MAY NEED A BETTER SOLUTION ??? -- KJC */
    }
}

void soar_ecPrintInternalSymbols(void)
{
    print_string("\n--- Symbolic Constants: ---\n");
    do_for_all_items_in_hash_table(current_agent(sym_constant_hash_table), print_sym);
    print_string("\n--- Integer Constants: ---\n");
    do_for_all_items_in_hash_table(current_agent(int_constant_hash_table), print_sym);
    print_string("\n--- Floating-Point Constants: ---\n");
    do_for_all_items_in_hash_table(current_agent(float_constant_hash_table), print_sym);
    print_string("\n--- Identifiers: ---\n");
    do_for_all_items_in_hash_table(current_agent(identifier_hash_table), print_sym);
    print_string("\n--- Variables: ---\n");
    do_for_all_items_in_hash_table(current_agent(variable_hash_table), print_sym);
}

int soar_ecPrintPreferences(char *szId, char *szAttr, bool print_prod, wme_trace_type wtt)
{

    Symbol *id, *attr;
    slot *s;
    preference *p;
    int i;

    if (read_id_or_context_var_from_string(szId, &id) == SOAR_ERROR) {
        print("Could not find the id '%s'\n", szId);
        return -1;
    }
    if (read_attribute_from_string(id, szAttr, &attr) == SOAR_ERROR) {
        print("Could not find the id,attribute pair: %s ^%s\n", szId, szAttr);
        return -2;
    }

    s = find_slot(id, attr);
    if (!s) {
        print("There are no preferences for %s ^%s.", szId, szAttr);
        return -3;
    }

    print_with_symbols("Preferences for %y ^%y:\n", id, attr);

    for (i = 0; i < NUM_PREFERENCE_TYPES; i++) {
        if (s->preferences[i]) {
            print("\n%ss:\n", preference_name[i]);
            for (p = s->preferences[i]; p; p = p->next) {
                print_preference_and_source(p, print_prod, wtt);
            }
        }
    }

    return 0;
}

void soar_ecPrintProductionsBeingTraced()
{

    cons *c;

    for (c = current_agent(productions_being_traced); c != NIL; c = c->rest)
        print_with_symbols(" %y\n", ((production *) (c->first))->name);
}

void soar_ecStopAllProductionTracing()
{

    cons *c, *next;

    /*
     * We don't use c=c->rest in the increment step because 
     * remove_pwatch may release c as a side-effect.
     */

    for (c = current_agent(productions_being_traced); c != NIL; c = next) {
        production *prod;

        next = c->rest;
        prod = current_agent(productions_being_traced)->first;
        remove_pwatch(prod);
    }

}

int soar_ecBeginTracingProductions(int n, const char **names)
{

    int i;
    production *prod;

    for (i = 0; i < n; i++) {

        prod = name_to_production(names[i]);
        if (prod) {
            add_pwatch(prod);
        } else {
            print("No Production named %s", names[i]);
            return (-1 - i);
        }
    }
    return 0;
}

int soar_ecStopTracingProductions(int n, const char **names)
{

    int i;
    production *prod;

    for (i = 0; i < n; i++) {

        prod = name_to_production(names[i]);
        if (prod) {
            remove_pwatch(prod);
        } else {
            print("No Production named %s", names[i]);
            return (-1 - i);
        }
    }
    return 0;
}

void soar_ecPrintMatchSet(wme_trace_type wtt, ms_trace_type mst)
{
    print_match_set(wtt, mst);
}

int soar_ecPrintMatchInfoForProduction(const char *name, wme_trace_type wtt)
{
    production *p;
    struct rete_node_struct *p_node;

    p = name_to_production(name);
    if (!p) {
        return -1;
    }

    p_node = p->p_node;
    print_partial_match_information(p_node, wtt);
    return 0;
}

void soar_ecPrintMemoryPoolStatistics(void)
{
    memory_pool *p;
#ifdef MEMORY_POOL_STATS
    long total_items;
#endif

    print("Memory pool statistics:\n\n");
#ifdef MEMORY_POOL_STATS
    print("Pool Name        Used Items  Free Items  Item Size  Total Bytes\n");
    print("---------------  ----------  ----------  ---------  -----------\n");
#else
    print("Pool Name        Item Size  Total Bytes\n");
    print("---------------  ---------  -----------\n");
#endif

    for (p = current_agent(memory_pools_in_use); p != NIL; p = p->next) {
        print_string(p->name);
        print_spaces(MAX_POOL_NAME_LENGTH - strlen(p->name));
#ifdef MEMORY_POOL_STATS
        print("  %10lu", p->used_count);
        total_items = p->num_blocks * p->items_per_block;
        print("  %10lu", total_items - p->used_count);
#endif
        print("  %9lu", p->item_size);
        print("  %11lu\n", p->num_blocks * p->items_per_block * p->item_size);
    }
}

#ifdef  ATTENTION_LAPSE
void soar_ecPrintAttentionLapseSettings(void)
{
    print("Current attention-lapse setting:\n");
    print("   %s\n", current_agent(sysparams)[ATTENTION_LAPSE_ON_SYSPARAM] ? "-on" : "-off");

}
#endif                          /* ATTENTION_LAPSE */

/*
 *----------------------------------------------------------------------
 *
 * soar_ecWatchLevel
 *
 *     Set the current Watch level, the higher the value, the 
 *     greater the verbosity.
 *
 *----------------------------------------------------------------------
 */
int soar_ecWatchLevel(int level)
{

    if (level > 5 || level < 0)
        return -1;

    set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, FALSE);
    set_sysparam(TRACE_PHASES_SYSPARAM, FALSE);
    set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
    set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
    set_sysparam(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, FALSE);
    set_sysparam(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, FALSE);
    set_sysparam(TRACE_WM_CHANGES_SYSPARAM, FALSE);
    set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, FALSE);
    set_sysparam(TRACE_OPERAND2_REMOVALS_SYSPARAM, FALSE);

    switch (level) {
    case 0:
        /* make sure everything is off */
        set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
        set_sysparam(TRACE_CHUNK_NAMES_SYSPARAM, FALSE);
        set_sysparam(TRACE_JUSTIFICATION_NAMES_SYSPARAM, FALSE);
        set_sysparam(TRACE_CHUNKS_SYSPARAM, FALSE);
        set_sysparam(TRACE_JUSTIFICATIONS_SYSPARAM, FALSE);
        set_sysparam(TRACE_OPERAND2_REMOVALS_SYSPARAM, FALSE);
        break;

    case 5:

        set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, TRUE);

    case 4:

        set_sysparam(TRACE_WM_CHANGES_SYSPARAM, TRUE);

    case 3:

        set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
        set_sysparam(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, TRUE);

    case 2:

        set_sysparam(TRACE_PHASES_SYSPARAM, TRUE);

    case 1:

        set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, TRUE);
    }

    return 0;
}
