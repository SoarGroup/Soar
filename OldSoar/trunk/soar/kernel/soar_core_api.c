/**
 * \file soar_core_api.c
 *   
 * This file contains the low-level (core) interface to the Soar production
 * system. 
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
#include <stdlib.h>

#include "soar_core_api.h"
#include "soar_core_utils.h"
#include "scheduler.h"

extern char *soar_callback_names[];
void soar_cTestCallback(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data);

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 0:    ACCESSOR FUNCTIONS AND MACROS
 *
 *               - Agent State
 *
 * *************************************************************************
 * *************************************************************************
 */

/* Will be renamed to soar_cSetSysparam */
void set_sysparam(int param_number, long new_value)
{

    if ((param_number < 0) || (param_number > HIGHEST_SYSPARAM_NUMBER)) {
        print("Internal error: tried to set bad sysparam #: %d\n", param_number);
        return;
    }
    current_agent(sysparams)[param_number] = new_value;

    soar_invoke_callbacks(soar_agent, SYSTEM_PARAMETER_CHANGED_CALLBACK, (soar_call_data) param_number);

}

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 1:    CRITICAL FUNCTIONS
 *
 *               - (Re)Initializing Soar
 *	             - Creating Agents
 *               - Destroying Agents
 *               - Starting and Stopping Agents
 *
 * *************************************************************************
 * *************************************************************************
 */

extern int soar_agent_ids[];
extern int agent_counter;
extern soar_global_callback_array soar_global_callbacks;

extern void gds_invalid_so_remove_goal(wme * w);

#if defined(WIN32)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define popen(command, mode) _popen((command), (mode))
#define pclose(stream) _pclose(stream)
#endif

/*
 *----------------------------------------------------------------------
 *
 * soar_cInitializeSoar
 *
 *     Initialize Soar for the very first time. 
 *     (Before any agents are created)
 *
 *----------------------------------------------------------------------
 */

#define SOAR_CINITIALIZESOAR_BUFFER_SIZE 1000
void soar_cInitializeSoar(void)
{
    char buffer[SOAR_CINITIALIZESOAR_BUFFER_SIZE];
    int i;

#if MICRO_VERSION_NUMBER > 0
    snprintf(buffer, SOAR_CINITIALIZESOAR_BUFFER_SIZE,
             "%d.%d.%d", MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER, MICRO_VERSION_NUMBER);
    buffer[SOAR_CINITIALIZESOAR_BUFFER_SIZE - 1] = 0;   /* snprintf doesn't set last char to null if output is truncated */
#else
    snprintf(buffer, SOAR_CINITIALIZESOAR_BUFFER_SIZE, "%d.%d", MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER);
    buffer[SOAR_CINITIALIZESOAR_BUFFER_SIZE - 1] = 0;   /* snprintf doesn't set last char to null if output is truncated */
#endif

    soar_version_string = savestring(buffer);

    /*  RCHONG and REW added following to soar_version_string.
     *  KJC leaving it out for now, since Tcl Pkg mechanism
     *  relies on soar_version_string to load right package.
     *  Will add it in interface level with other options.
     */

    /* 
     *   if (current_agent(operand2_mode) == TRUE)
     *      sprintf(buffer,"%s-%s", OPERAND_MODE_NAME);
     */
/* REW: end   09.15.96 */

    /* --- set the random number generator seed to a "random" value --- */
    sys_srandom(time(NULL));

    setup_signal_handling();

    /* SW 081299 */
    /* No assigned agent ids */
    for (i = 0; i < MAX_SIMULTANEOUS_AGENTS; i++)
        soar_agent_ids[i] = UNTOUCHED;

    soar_init_global_callbacks();
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cReInitSoar --
 *
 *      ReInitialize Soar by clearing the working memory of all agents
 *      and preparing them for a "new" execution
 *
 *----------------------------------------------------------------------
 */
int soar_cReInitSoar(void)
{

    /* kjh (CUSP-B4) begin */
    long cur_TRACE_CONTEXT_DECISIONS_SYSPARAM;
    long cur_TRACE_PHASES_SYSPARAM;
    long cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM;
    long cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM;
    long cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM;
    long cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM;
    long cur_TRACE_WM_CHANGES_SYSPARAM;
    /* kjh (CUSP-B4) end */

    current_agent(did_PE) = FALSE;      /* RCHONG:  10.11 */

    soar_invoke_callbacks(soar_agent, BEFORE_INIT_SOAR_CALLBACK, (soar_call_data) NULL);

    /* kjh (CUSP-B4) begin */
    /* Stash trace state: */
    cur_TRACE_CONTEXT_DECISIONS_SYSPARAM = current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM];
    cur_TRACE_PHASES_SYSPARAM = current_agent(sysparams)[TRACE_PHASES_SYSPARAM];
    cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM = current_agent(sysparams)[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM];
    cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM = current_agent(sysparams)[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM];
    cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM = current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM];
    cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM = current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM];
    cur_TRACE_WM_CHANGES_SYSPARAM = current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM];

    /* Temporarily disable tracing: */
    set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, FALSE);
    set_sysparam(TRACE_PHASES_SYSPARAM, FALSE);
    set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
    set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
    set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
    set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, FALSE);
    set_sysparam(TRACE_WM_CHANGES_SYSPARAM, FALSE);
    /* kjh (CUSP-B4) end */

    clear_goal_stack();

#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode) == TRUE) {
#endif

        /* Signal that everything should be retracted */
        current_agent(active_level) = 0;

        current_agent(FIRING_TYPE) = IE_PRODS;
        do_preference_phase();  /* allow all i-instantiations to retract */

        /* REW: begin  09.22.97 */

        /* In Operand2,  any retractions, regardless of i-instantitations or
           o-instantitations, are retracted at the saem time (in an IE_PRODS
           phase).  So one call to the preference phase should be sufficient. */

        /* DELETED code to set FIRING_TYPE to PE and call preference phase. */

        /* REW: end    09.22.97 */
#ifndef SOAR_8_ONLY
    }

    /* REW: end  09.15.96 */
    else
        do_preference_phase();  /* allow all instantiations to retract */
#endif

    reset_explain();
    reset_id_counters();
    reset_wme_timetags();
    reset_statistics();
    current_agent(system_halted) = FALSE;
    current_agent(go_number) = 1;
    current_agent(go_type) = GO_DECISION;

    /* kjh (CUSP-B4) begin */
    /* Restore trace state: */
    set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM, cur_TRACE_CONTEXT_DECISIONS_SYSPARAM);
    set_sysparam(TRACE_PHASES_SYSPARAM, cur_TRACE_PHASES_SYSPARAM);
    set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM);
    set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM);
    set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM);
    set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM);
    set_sysparam(TRACE_WM_CHANGES_SYSPARAM, cur_TRACE_WM_CHANGES_SYSPARAM);
    /* kjh (CUSP-B4) end */

    soar_invoke_callbacks(soar_agent, AFTER_INIT_SOAR_CALLBACK, (soar_call_data) NULL);

    current_agent(input_cycle_flag) = TRUE;     /* reinitialize flag  AGR REW1 */

    /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode) == TRUE) {
#endif

        current_agent(FIRING_TYPE) = IE_PRODS;  /* KJC 10.05.98 was PE */
        current_agent(current_phase) = INPUT_PHASE;
        current_agent(did_PE) = FALSE;
#ifndef SOAR_8_ONLY
    }
#endif
    /* REW: end 09.15.96 */

#ifdef WARN_IF_TIMERS_REPORT_ZERO
    current_agent(warn_on_zero_timers) = TRUE;
#endif

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cCreateAgent --
 *
 *      Create a new soar agent with the specified name.
 *
 *----------------------------------------------------------------------
 */

void soar_cCreateAgent(const char *agent_name)
{

    if (soar_exists_global_callback(GLB_CREATE_AGENT)) {
        soar_invoke_global_callbacks(NULL, GLB_CREATE_AGENT, (soar_call_data) agent_name);
    } else {
        soar_default_create_agent_procedure(agent_name);
    }

}

/*
 *----------------------------------------------------------------------
 *
 * soar_cRun --
 *
 *      Run the current agent, or all agents for a specified
 *      period ...
 *
 *----------------------------------------------------------------------
 */
int soar_cRun(long n, bool allAgents, enum go_type_enum type, enum soar_apiSlotType slot)
{

    int levels_up;
    Symbol *attribute, *goal;
    cons *c;

    if (type != GO_SLOT && slot != NO_SLOT)
        return -1;

    if (n < -1)
        n = -1;

    switch (type) {

    case GO_PHASE:
        run_for_n_phases(n);
        break;
    case GO_ELABORATION:
        run_for_n_elaboration_cycles(n);
        break;
    case GO_DECISION:
        run_for_n_decision_cycles(n);
        break;
    case GO_STATE:
        run_for_n_selections_of_slot(n, soar_agent->state_symbol);
        break;
    case GO_OPERATOR:
        run_for_n_selections_of_slot(n, soar_agent->operator_symbol);
        break;

    case GO_OUTPUT:
        run_for_n_modifications_of_output(n);
        break;

    case GO_SLOT:
        switch (slot) {

        case STATE_SLOT:
            levels_up = 0;
            attribute = current_agent(state_symbol);
            break;
        case OPERATOR_SLOT:
            levels_up = 0;
            attribute = current_agent(operator_symbol);
            break;
        case SUPERSTATE_SLOT:
            levels_up = 1;
            attribute = current_agent(state_symbol);
            break;
        case SUPEROPERATOR_SLOT:
            levels_up = 1;
            attribute = current_agent(operator_symbol);
            break;
        case SUPERSUPERSTATE_SLOT:
            levels_up = 2;
            attribute = current_agent(state_symbol);
            break;
        case SUPERSUPEROPERATOR_SLOT:
            levels_up = 2;
            attribute = current_agent(operator_symbol);
            break;

        default:
            return -2;
            break;

        }                       /* End of switch (slot) */

        goal = current_agent(bottom_goal);
        while (goal && levels_up) {
            goal = goal->id.higher_goal;
            levels_up--;
        }

        if (!goal)
            return -3;

        run_all_agents(n, GO_SLOT, attribute, goal->id.level);

        break;

    }                           /* End of switch type */

    if (allAgents) {
        for (c = all_soar_agents; c != NIL; c = c->rest) {
            if (((agent *) c->first)->system_halted)
                return -4;
        }
    } else {
        if (current_agent(system_halted))
            return -4;
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cStopAllAgents()
 *
 *     Stops all agents
 *
 *----------------------------------------------------------------------
 */
void soar_cStopAllAgents(void)
{
    control_c_handler(0);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cStopCurrentAgent()
 *
 *     Stops the current agent
 *
 *----------------------------------------------------------------------
 */
void soar_cStopCurrentAgent(const char *reason)
{
    current_agent(stop_soar) = TRUE;
    current_agent(reason_for_stopping) = reason;

}

/* 
 *----------------------------------------------------------------------
 *
 * soar_cDestroyAgentByName --
 *
 *     Destroy an agent, given its name
 *
 *     (calls the common DestroyAgent ancestor: 
 *         soar_cDestroyAgentByAddress)
 *
 *----------------------------------------------------------------------
 */
int soar_cDestroyAgentByName(const char *name)
{
    cons *c;
    int name_count = 0;
    psoar_agent *delete_me = NULL;

    for (c = all_soar_agents; c != NIL; c = c->rest) {
        if (string_match(name, ((agent *) c->first)->name)) {
            name_count++;
            delete_me = (psoar_agent) c->first;
        }
    }
    if (name_count > 1)
        return -1;
    if (name_count == 0)
        return -2;
    soar_cDestroyAgentByAddress(delete_me);

    return 0;
}

/* 
 *----------------------------------------------------------------------
 *
 * soar_cDestroyAllAgentsWithName --
 *
 *     Destroy all agents with a given name
 *
 *     (calls the common DestroyAgent ancestor: 
 *         soar_cDestroyAgentByAddress)
 *
 *----------------------------------------------------------------------
 */
int soar_cDestroyAllAgentsWithName(char *name)
{
    cons *c;
    int count;

    count = 0;
    for (c = all_soar_agents; c != NIL; c = c->rest) {
        if (string_match(name, ((agent *) c->first)->name)) {
            count++;
            soar_cDestroyAgentByAddress((psoar_agent) c->first);

        }
    }
    if (count == 0)
        return -1;
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cDestroyAgentByAddress --
 *
 *     Destroy an agent, given a pointer to it.
 *
 *     (This function is the common ancestor 
 *      of all other DestroyAgent functions)
 *
 *----------------------------------------------------------------------
 */
void soar_cDestroyAgentByAddress(psoar_agent delete_agent)
{

    if (soar_exists_global_callback(GLB_DESTROY_AGENT)) {
        soar_invoke_global_callbacks(delete_agent, GLB_DESTROY_AGENT, (soar_call_data) delete_agent);
    } else {
        soar_default_destroy_agent_procedure(delete_agent);
    }
}

/* 
 *----------------------------------------------------------------------
 *
 * soar_cDestroyAgentById --
 *
 *     Destroy an agent, given its unique id
 *
 *     (calls the common DestroyAgent ancestor: 
 *         soar_cDestroyAgentByAddress)
 *
 *----------------------------------------------------------------------
 */
int soar_cDestroyAgentById(int agent_id)
{
    cons *c;

    for (c = all_soar_agents; c != NIL; c = c->rest) {
        if (agent_id == ((agent *) c->first)->id) {
            soar_cDestroyAgentByAddress((psoar_agent) c->first);
            return 0;
        }
    }

    /* Didn't find agent id */
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cQuit --
 *
 *     stop the log files, and quit
 *
 *----------------------------------------------------------------------
 */
void soar_cQuit(void)
{

    /* If there aren't any agents, then there's nothing to do */
    if (!soar_agent)
        return;

    just_before_exit_soar();

    /* Soar-Bugs #58, TMH */
    while (soar_exists_callback(soar_agent, LOG_CALLBACK)) {

        soar_invoke_first_callback(soar_agent, LOG_CALLBACK, "\n**** quit cmd issued ****\n");
        soar_cPopCallback(soar_agent, LOG_CALLBACK);
    }
#ifdef USE_AGENT_DBG_FILE
    fclose(current_agent(dbgFile));
#endif

}

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 2:    MODIFYING AGENT MEMORY
 *
 *               - Production Memory
 *	         - Working Memory
 *
 * *************************************************************************
 * *************************************************************************
 */

/*
 *----------------------------------------------------------------------
 *
 * soar_cLoadReteNet --
 *
 *     load a Rete Network into the agent from a specified file
 *
 *----------------------------------------------------------------------
 */
int soar_cLoadReteNet(const char *filename)
{

    char pipe_command[] = "zcat ";
    bool using_compression_filter;
    char *append_loc, *command_line;
    FILE *f;
    bool result;
    int i;

    if (!filename) {
        print("Internal error: No file name specified.\n");
        return SOAR_ERROR;
    }

    /* --- check for empty system --- */
    if (current_agent(all_wmes_in_rete)) {
        print("Internal error: Can't load RETE in non-empty system.  Restart Soar first.\n");
        return SOAR_ERROR;
    }

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
        if (current_agent(num_productions_of_type)[i]) {
            print("Internal error: Can't load RETE in non-empty system.  Restart Soar first.\n");
            return SOAR_ERROR;
        }

#if !defined(MACINTOSH)         /* Mac doesn't have pipes */
    if ((!(strcmp((char *) (filename + strlen(filename) - 2), ".Z")))
        || (!(strcmp((char *) (filename + strlen(filename) - 2), ".z")))) {

        /* The popen can succeed if given an non-existant file   
           creating an unusable pipe.  So we check to see if the 
           file exists first, on a load action.                  */

        f = fopen(filename, "rb");

        if (!f) {
            /* --- error when opening the file --- */
            print("Internal error: Error opening file.\n");
            return SOAR_ERROR;

        } else {
            fclose(f);
        }

        command_line = allocate_memory(strlen(pipe_command) + strlen(filename) + 1, STRING_MEM_USAGE);

        strcpy(command_line, pipe_command);     /* this is relatively safe since the memory is allocated on the previous line */

        append_loc = command_line;
        while (*append_loc)
            append_loc++;

        strcpy(append_loc, filename);   /* this is relatively safe since sufficient memory is allocated earlier */

        f = (FILE *) popen(command_line, "rb");
        free_memory(command_line, STRING_MEM_USAGE);

        using_compression_filter = TRUE;
    } else
#endif                          /* !MACINTOSH */
    {

        f = fopen(filename, "rb");
        using_compression_filter = FALSE;
    }

    if (!f) {
        /* --- error when opening the pipe or file --- */
        print("Internal error: error opening file.\n");
        return SOAR_ERROR;

    }

    result = load_rete_net(f);

#if !defined(MACINTOSH)
    if (using_compression_filter == TRUE) {
        pclose(f);
    } else
#endif                          /* !MACINTOSH */
    {
        fclose(f);
    }

    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cSaveReteNet --
 *
 *     save a Rete Network into the agent from a specified file
 *
 *----------------------------------------------------------------------
 */

int soar_cSaveReteNet(const char *filename)
{

    char *command_line;
    char pipe_command[] = "compress > ";
    FILE *f;
    bool using_compression_filter = FALSE;
    char *append_loc;

    if (current_agent(all_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]) {
        /* printing message as per bugzilla bug #154 */
        print("Internal error: can't save rete with justifications present. If you want to save, do an init-soar and try again.\n");
        return SOAR_ERROR;
    }

#if !defined(MACINTOSH)         /* Mac doesn't have pipes */
    if ((!(strcmp((char *) (filename + strlen(filename) - 2), ".Z")))
        || (!(strcmp((char *) (filename + strlen(filename) - 2), ".z")))) {

        command_line = allocate_memory(strlen(pipe_command) + strlen(filename) + 1, STRING_MEM_USAGE);

        strcpy(command_line, pipe_command);     /* this is relatively safe since the memory is allocated on the previous line */

        append_loc = command_line;
        while (*append_loc)
            append_loc++;

        strcpy(append_loc, filename);   /* this is relatively safe since the memory is allocated earlier */

        f = (FILE *) popen(command_line, "wb");
        free_memory(command_line, STRING_MEM_USAGE);

        using_compression_filter = TRUE;

    } else
#endif                          /* !MACINTOSH */
    {

        f = fopen(filename, "wb");
        using_compression_filter = FALSE;
    }

    if (!f) {
        /* --- error when opening the pipe or file --- */
        print("Internal error: error opening file.\n");
        return SOAR_ERROR;
    }

    save_rete_net(f);

#if !defined(MACINTOSH)
    if (using_compression_filter == TRUE) {
        pclose(f);
    } else
#endif                          /* !MACINTOSH */
    {
        fclose(f);
    }

    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cAddWme
 *
 *     Remove a working memory element, given its timetag
 *
 *----------------------------------------------------------------------
 */
/*unsigned long soar_cAddWme( const char *szId, const char *szAttr, const char *szValue, */
long soar_cAddWme(const char *szId, const char *szAttr, const char *szValue,
                  bool acceptable_preference, psoar_wme * new_wme)
{

    Symbol *id, *attr, *value;
    wme *w;

    /* --- get id --- */
    if (read_id_or_context_var_from_string(szId, &id) == SOAR_ERROR)
        return -1;

    /* --- get optional '^', if present --- */

    if (*szAttr == '^')
        szAttr++;

    /* --- get attribute or '*' --- */
    if (string_match("*", szAttr) == TRUE) {
#ifdef USE_AGENT_DBG_FILE
        fprintf(current_agent(dbgFile), "'%s' matches '*'\n", szAttr);
#endif

        attr = make_new_identifier('I', id->id.level);
    } else {
        get_lexeme_from_string(szAttr);

        switch (current_agent(lexeme).type) {
        case SYM_CONSTANT_LEXEME:
            attr = make_sym_constant(current_agent(lexeme).string);
            break;
        case INT_CONSTANT_LEXEME:
            attr = make_int_constant(current_agent(lexeme).int_val);
            break;
        case FLOAT_CONSTANT_LEXEME:
            attr = make_float_constant(current_agent(lexeme).float_val);
            break;
        case IDENTIFIER_LEXEME:
        case VARIABLE_LEXEME:
            attr = read_identifier_or_context_variable();
            if (!attr) {
                return -2;
            }
            symbol_add_ref(attr);
            break;
        default:
            return -2;
        }
    }

    /* --- get value or '*' --- */

    if (string_match("*", szValue) == TRUE) {
        value = make_new_identifier('I', id->id.level);
    } else {
        get_lexeme_from_string(szValue);
        switch (current_agent(lexeme).type) {
        case SYM_CONSTANT_LEXEME:
            value = make_sym_constant(current_agent(lexeme).string);
            break;
        case INT_CONSTANT_LEXEME:
            value = make_int_constant(current_agent(lexeme).int_val);
            break;
        case FLOAT_CONSTANT_LEXEME:
            value = make_float_constant(current_agent(lexeme).float_val);
            break;
        case IDENTIFIER_LEXEME:
        case VARIABLE_LEXEME:
            value = read_identifier_or_context_variable();
            if (!value) {
                symbol_remove_ref(attr);
                return -3;
            }
            symbol_add_ref(value);
            break;
        default:
            symbol_remove_ref(attr);
            return -3;
        }
    }

    /* --- now create and add the wme --- */
    w = make_wme(id, attr, value, acceptable_preference);

    symbol_remove_ref(w->attr);
    symbol_remove_ref(w->value);
    insert_at_head_of_dll(w->id->id.input_wmes, w, next, prev);
    add_wme_to_wm(w);

#ifdef USE_CAPTURE_REPLAY
    /* KJC 11/99 begin: */
    /* if input capturing is enabled, save any input wmes to capture file */
    if (current_agent(capture_fileID) && (current_agent(current_phase) == INPUT_PHASE)) {

        soarapi_wme sapi_wme;

        /* Dont copy, since capture_input_wme is just going to print
         * the contents of the structure into a file... 
         */
        sapi_wme.id = szId;
        sapi_wme.attr = szAttr;
        sapi_wme.value = szValue;
        sapi_wme.timetag = w->timetag;

        capture_input_wme(ADD_WME, &sapi_wme, w);
    }
    /* KJC 11/99 end */

#endif                          /* USE_CAPTURE_REPLAY */

    /* REW: begin 28.07.96 */
    /* OK.  This is an ugly hack.  Basically we want to keep track of kernel
       time and callback time.  add-wme is called from either a callback
       (for input routines) or from the command line (or someplace else?).
       Here, I'm just assuming that we'll normally call add-wme from an
       input routine so I turn off the input_function_timer and turn on
       the kernel timers before the call to the low-level function:
       do_buffered_wm_and_ownership_changes.

       This assumption is problematic because anytime add-wme is called 
       from some place other than the input function, there is a potential
       to get some erroneous (if start_kernel_tv wasn't set for the 
       input function) or just bad (if start_kernel_tv isn't defined)
       timing data.   The real problem is that this very high-level 
       routine is going deep into the kernel.  We can either ignore
       this and just call the time spent doing wm changes here time
       spent outside the kernel or we can try to do the accounting,
       what this hack is a first-attempt at doing.  

       However, my testing turned up no problems -- I was able to add
       and remove WMEs without messing up the timers.  So it`s a 
       hack that seems to work.  For now.  (there is a plan to 
       add routines for specifically adding and deleting input
       WMEs which should help clear up this isse)               REW */

    /* REW: end 28.07.96 */

#ifndef NO_TIMING_STUFF
    if (current_agent(current_phase) == INPUT_PHASE) {
        /* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
        stop_timer(&current_agent(start_kernel_tv), &current_agent(input_function_cpu_time));
        start_timer(&current_agent(start_kernel_tv));

#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
    }
#endif
/* REW: end 28.07.96 */

    /* note: 
     * I don't completely understand this:
     * The deal seems to be that when NO_TOP_LEVEL_REFS is used, wmes on the i/o
     * link (obviously) have fewer references than thy would otherwise.
     * Although calling this here (in soar_cAddWme) doesn't seem to matter
     * one way or the other, calling it in soar_cRemoveWme really leads to 
     * problems.  What happens is that the i/o wme is removed prior to 
     * fully figuring out the match set.  This means that productions which
     * should have fired, dont.  However, if we comment this out for the
     * NO_TOP_LEVEL_REFS fix we don't seem to get this problem.  There might be
     * an underlying pathology here, but so far I don't know what it is.
     * This suspicion is heightened by the fact that even when this fix
     * is made, wmes are deallocated in a different place (e.g. at the end of
     * the input cycle) than using a normal build.
     *
     * an interesting aside seems to be that we don't need to do buffered
     * wme and own changes here regardless of whether or not L1R is used
     * so long as we test to make sure we're in the INPUT_PHASE.  I will 
     * look into this more later.
     */
#ifndef NO_TOP_LEVEL_REFS
    do_buffered_wm_and_ownership_changes();
#endif

/* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
    if (current_agent(current_phase) == INPUT_PHASE) {

#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv),
                   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif
        stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
        start_timer(&current_agent(start_kernel_tv));
    }
#endif
/* REW: end 28.07.96 */
/* #endif */

    *new_wme = (psoar_wme) w;
    return w->timetag;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_cRemoveWmeUsingTimetag
 *
 *     Remove a working memory element, given its timetag
 *
 *     Note: this function essentially searches the entire working memory
 *     contents to find the given wme.  It is much more efficient to use
 *     the ancestor function, although it offers less encapsulation.
 *
 *     (calls the common RemoveWme ancestor: 
 *         soar_cRemoveWme)
 *
 *----------------------------------------------------------------------
 */
int soar_cRemoveWmeUsingTimetag(int num)
{

    wme *w;

    for (w = current_agent(all_wmes_in_rete); w != NIL; w = w->rete_next)
        if (w->timetag == (unsigned long) num)
            break;

    if (!w)
        return -1;

    if (!soar_cRemoveWme(w))
        return 0;

    return -2;                  /* Unspecified Failure */
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cRemoveWme
 *
 *     Remove a working memory element, given a pointer to it.
 *
 *     (this function is the common ancestor of
 *      all other RemoveWme functions)
 *
 *----------------------------------------------------------------------
 */
int soar_cRemoveWme(psoar_wme the_wme)
{

    wme *w, *w2;
    Symbol *id;
    slot *s;

    w = (wme *) the_wme;

    id = w->id;

    /* --- remove w from whatever list of wmes it's on --- */
    for (w2 = id->id.input_wmes; w2 != NIL; w2 = w2->next)
        if (w == w2)
            break;

    if (w2)
        remove_from_dll(id->id.input_wmes, w, next, prev);

    for (w2 = id->id.impasse_wmes; w2 != NIL; w2 = w2->next)
        if (w == w2)
            break;

    if (w2)
        remove_from_dll(id->id.impasse_wmes, w, next, prev);

    for (s = id->id.slots; s != NIL; s = s->next) {

        for (w2 = s->wmes; w2 != NIL; w2 = w2->next)
            if (w == w2)
                break;

        if (w2)
            remove_from_dll(s->wmes, w, next, prev);

        for (w2 = s->acceptable_preference_wmes; w2 != NIL; w2 = w2->next)
            if (w == w2)
                break;

        if (w2)
            remove_from_dll(s->acceptable_preference_wmes, w, next, prev);
    }

#ifdef USE_CAPTURE_REPLAY

    /* KJC 11/99 begin: */
    /* if input capturing is enabled, save any input changes to capture file */
    if (current_agent(capture_fileID) && (current_agent(current_phase) == INPUT_PHASE)) {
        soarapi_wme sapi_wme;

        sapi_wme.id = NULL;
        sapi_wme.attr = NULL;
        sapi_wme.value = NULL;
        sapi_wme.timetag = w->timetag;

        capture_input_wme(REMOVE_WME, &sapi_wme, NULL);
    }
    /* KJC 11/99 end */
#endif                          /* USE_CAPTURE_REPLAY */

    /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode)) {
#endif
        if (w->gds) {
            if (w->gds->goal != NIL) {

                gds_invalid_so_remove_goal(w);

                /* NOTE: the call to remove_wme_from_wm will take care of checking if
                   GDS should be removed */
            }
        }
#ifndef SOAR_8_ONLY
    }
#endif

    /* REW: end   09.15.96 */

    /* --- now remove w from working memory --- */
    remove_wme_from_wm(w);

    /* REW: begin 28.07.96 */
    /* See AddWme for description of what's going on here */

    if (current_agent(current_phase) != INPUT_PHASE) {
#ifndef NO_TIMING_STUFF
        start_timer(&current_agent(start_kernel_tv));
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
#endif

        /* do_buffered_wm_and_ownership_changes(); */

#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv),
                   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif
        stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
        start_timer(&current_agent(start_kernel_tv));
#endif
    }
    /* note: 
     *  See note at the NO_TOP_LEVEL_REFS flag in soar_cAddWme
     */
#ifndef NO_TOP_LEVEL_REFS
    do_buffered_wm_and_ownership_changes();
#endif

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cExciseAllProductions
 *
 *     Remove all productions from the agents memory and 
 *     ReInitialize the agent
 *
 *----------------------------------------------------------------------
 */
void soar_cExciseAllProductions(void)
{

    soar_cExciseAllProductionsOfType(DEFAULT_PRODUCTION_TYPE);
    soar_cExciseAllProductionsOfType(CHUNK_PRODUCTION_TYPE);
    soar_cExciseAllProductionsOfType(JUSTIFICATION_PRODUCTION_TYPE);
    soar_cExciseAllProductionsOfType(USER_PRODUCTION_TYPE);
    soar_cReInitSoar();
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cExciseAllTaskProductions
 *
 *     Remove all but the default productions from the agents memory
 *     and ReInitialize the agent
 *
 *---------------------------------------------------------------------- 
 */

void soar_cExciseAllTaskProductions(void)
{
    soar_cExciseAllProductionsOfType(CHUNK_PRODUCTION_TYPE);
    soar_cExciseAllProductionsOfType(JUSTIFICATION_PRODUCTION_TYPE);
    soar_cExciseAllProductionsOfType(USER_PRODUCTION_TYPE);
    soar_cReInitSoar();
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cExciseAllProductionsOfType
 *
 *     Remove all productions of a specific type from the agents
 *     memory
 *
 *---------------------------------------------------------------------- */

void soar_cExciseAllProductionsOfType(byte type)
{
    while (current_agent(all_productions_of_type)[type])
        excise_production(current_agent(all_productions_of_type)[type],
                          (bool) (TRUE && current_agent(sysparams)[TRACE_LOADING_SYSPARAM]));
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cExciseProductionByName
 *
 *     Remove the production with the specified name
 *
 *----------------------------------------------------------------------
 */
int soar_cExciseProductionByName(const char *name)
{
    production *p;

    p = name_to_production(name);
    if (p) {
        excise_production(p, (bool) (TRUE && current_agent(sysparams)[TRACE_LOADING_SYSPARAM]));
        return 0;
    }

    return -1;                  /* production not found */
}

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 3:    ACCESSING, MODIFYING & WATCHING THE AGENT'S STATE
 *
 *
 * *************************************************************************
 * *************************************************************************
 */

#ifndef NO_TIMING_STUFF
/*
 *----------------------------------------------------------------------
 *
 * soar_cDetermineTimerResolution
 *
 *   check the resolution of the system timers.     
 *
 *----------------------------------------------------------------------
 */
double soar_cDetermineTimerResolution(double *min, double *max)
{

    double delta, max_delta, min_delta, min_nz_delta;
    float q;
    int i, j, top;
#ifdef PII_TIMERS
    unsigned long long int start, end, total;
#else
    struct timeval start, end, total;
#endif

    top = ONE_MILLION;
    min_delta = ONE_MILLION;
    min_nz_delta = ONE_MILLION;
    max_delta = -1;
    reset_timer(&total);

    for (i = 0; i < ONE_MILLION; i = (i + 1) * 2) {
        reset_timer(&end);
        start_timer(&start);
        for (j = 0; j < i * top; j++) {
            q = (float) (j * i);
        }
        stop_timer(&start, &end);
        stop_timer(&start, &total);
        delta = timer_value(&end);

        if (delta < min_delta)
            min_delta = delta;
        if (delta && delta < min_nz_delta)
            min_nz_delta = delta;
        if (delta > max_delta)
            max_delta = delta;

        /* when we have gone through this loop for 2 seconds, stop */
        if (timer_value(&total) >= 2) {
            break;
        }

    }

    if (min_nz_delta == ONE_MILLION)
        min_nz_delta = -1;
    if (min_delta == ONE_MILLION)
        min_delta = -1;

    if (min != NULL)
        *min = min_delta;
    if (max != NULL)
        *max = max_delta;
    return min_nz_delta;

}
#endif

#ifdef DC_HISTOGRAM
/*
 *----------------------------------------------------------------------
 *
 * soar_cInitializeDCHistogram
 *
 *     
 *
 *----------------------------------------------------------------------
 */
void soar_cInitializeDCHistogram(int nDC, int freq)
{
    int i;

    current_agent(dc_histogram_freq) = freq;

    if (nDC < current_agent(dc_histogram_sz)) {
        current_agent(dc_histogram_sz) = nDC;
    } else {
        free(current_agent(dc_histogram_tv));
        current_agent(dc_histogram_sz) = nDC;
        current_agent(dc_histogram_tv) = (struct timeval *) malloc(nDC * sizeof(struct timeval));
    }

    for (i = 0; i < nDC; i++) {
        reset_timer(&current_agent(dc_histogram_tv)[i]);
    }
}
#endif

#ifdef KT_HISTOGRAM
/*
 *----------------------------------------------------------------------
 *
 * soar_cInitializeKTHistogram
 *
 *     
 *
 *----------------------------------------------------------------------
 */
void soar_cInitializeKTHistogram(int size)
{
    int i;

    if (size < current_agent(kt_histogram_sz)) {
        current_agent(kt_histogram_sz) = size;
    } else {
        free(current_agent(kt_histogram_tv));
        current_agent(kt_histogram_sz) = size;
        current_agent(kt_histogram_tv) = (struct timeval *) malloc(size * sizeof(struct timeval));
    }

    for (i = 0; i < size; i++) {
        reset_timer(&current_agent(kt_histogram_tv)[i]);
    }
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * soar_cSetChunkNameLong
 *
 *     set long or short chunk names according to the specified
 *     parameter ( TRUE or FALSE respectively )
 *
 *----------------------------------------------------------------------
 */
void soar_cSetChunkNameLong(bool truly)
{

    set_sysparam(USE_LONG_CHUNK_NAMES, truly);

}

/*
 *----------------------------------------------------------------------
 *
 * soar_cSetChunkNameCount
 *
 *     set the chunk count.
 *       this must be greater than zero, less than max chunks and
 *       greater than the current chunk count.
 *
 *----------------------------------------------------------------------
 */
int soar_cSetChunkNameCount(long count)
{

    if (count < 0)
        return -1;

    if (count >= current_agent(sysparams)[MAX_CHUNKS_SYSPARAM])
        return -2;

    if ((unsigned long) count < current_agent(chunk_count))
        return -3;

    current_agent(chunk_count) = count;
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cSetChunkNamePrefix
 *
 *     set long or short chunk names according to the specified
 *     parameter ( TRUE or FALSE respectively )
 *
 *----------------------------------------------------------------------
 */
int soar_cSetChunkNamePrefix(const char *prefix)
{

    if (strchr(prefix, '*')) {
        return -1;
    }
    strncpy(current_agent(chunk_name_prefix), prefix, kChunkNamePrefixMaxLength);
    current_agent(chunk_name_prefix)[kChunkNamePrefixMaxLength - 1] = 0;
    return 0;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_cSetLearning
 *
 *       Adjust the learning settings
 *
 *----------------------------------------------------------------------
 */
void soar_cSetLearning(enum soar_apiLearningSetting setting)
{

    switch (setting) {

    case ON:
        set_sysparam(LEARNING_ON_SYSPARAM, TRUE);
        set_sysparam(LEARNING_ONLY_SYSPARAM, FALSE);
        set_sysparam(LEARNING_EXCEPT_SYSPARAM, FALSE);
        break;
    case OFF:
        set_sysparam(LEARNING_ON_SYSPARAM, FALSE);
        set_sysparam(LEARNING_ONLY_SYSPARAM, FALSE);
        set_sysparam(LEARNING_EXCEPT_SYSPARAM, FALSE);
        break;
    case ONLY:
        set_sysparam(LEARNING_ON_SYSPARAM, TRUE);
        set_sysparam(LEARNING_ONLY_SYSPARAM, TRUE);
        set_sysparam(LEARNING_EXCEPT_SYSPARAM, FALSE);
        break;
    case EXCEPT:
        set_sysparam(LEARNING_ON_SYSPARAM, TRUE);
        set_sysparam(LEARNING_ONLY_SYSPARAM, FALSE);
        set_sysparam(LEARNING_EXCEPT_SYSPARAM, TRUE);
        break;
    case ALL_LEVELS:
        set_sysparam(LEARNING_ALL_GOALS_SYSPARAM, TRUE);
        break;
    case BOTTOM_UP:
        set_sysparam(LEARNING_ALL_GOALS_SYSPARAM, FALSE);
        break;

    }

}

/*
 *----------------------------------------------------------------------
 *
 * soar_cSetOperand2
 *
 *----------------------------------------------------------------------
 */
int soar_cSetOperand2(bool turnOn)
{
    int i;

    /* --- check for empty system --- */
    if (current_agent(all_wmes_in_rete)) {
        return -1;
    }
    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
        if (current_agent(num_productions_of_type)[i]) {
            return -2;
        }

    current_agent(operand2_mode) = turnOn;
    soar_cReInitSoar();

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cSetWaitSNC
 *
 *    if False, the agent generates State-No-Change impasses.
 *    otherwise, it just sits around waiting...
 *    
 *
 *----------------------------------------------------------------------
 */
void soar_cSetWaitSNC(bool on)
{

    current_agent(waitsnc) = on;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cMultiAttributes
 *
 *    Set the matching priority of a particular attribute
 *    
 *
 *----------------------------------------------------------------------
 */
int soar_cMultiAttributes(const char *attribute, int value)
{
    multi_attribute *m;
    Symbol *s;

    get_lexeme_from_string(attribute);

    if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME) {
        return -1;
    }
    if (value < 1) {
        return -2;
    }

    m = current_agent(multi_attributes);
    s = make_sym_constant(attribute);

    while (m) {
        if (m->symbol == s) {
            m->value = value;
            symbol_remove_ref(s);
            return 0;
        }
        m = m->next;
    }
    /* sym wasn't in the table if we get here, so add it */
    m = (multi_attribute *) allocate_memory(sizeof(multi_attribute), MISCELLANEOUS_MEM_USAGE);
    m->value = value;
    m->symbol = s;
    m->next = current_agent(multi_attributes);
    current_agent(multi_attributes) = m;

    return 0;
}

/* 
 *----------------------------------------------------------------------
 *
 * soar_cAttributePreferencesMode
 *
 *           Determine how preferences for non-context slots should be 
 *           handled.  
 *----------------------------------------------------------------------
 */
#ifndef SOAR_8_ONLY
int soar_cAttributePreferencesMode(int mode)
{

    if (current_agent(operand2_mode) && (mode != 2)) {
        /* we're in Soar8 mode, but tried setting mode != 2 */

        return -1;
    }

    if (mode >= 0 && mode <= 2) {
        current_agent(attribute_preferences_mode) = mode;
    } else {
        /*
         * Attribute Preferences Mode must be an integer 0, 1, or 2. 
         */
        return -2;
    }

    return 0;
}

#else

int soar_cAttributePreferencesMode(int mode)
{

    if (mode != 2) {
        /* we're in Soar8 mode, but tried setting mode != 2 */

        return -1;
    }

    return 0;
}

#endif                          /* SOAR_8_ONLY */

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 4:    CALLBACKS
 *
 *
 * *************************************************************************
 * *************************************************************************
 */

/* ====================================================================
                  Adding New Input and Output Functions

   The system maintains a list of all the input functions to be called
   every input cycle, and another list of all the symbol-to-function
   mappings for output commands.  Add_input_function() and
   add_output_function() should be called at system startup time to 
   install each I/O function.
==================================================================== */

/*
 *----------------------------------------------------------------------
 *
 * soar_cAddInputFunction
 *
 *     Installs a function which will provide input to the agent
 *
 *----------------------------------------------------------------------
 */
void soar_cAddInputFunction(agent * a, soar_callback_fn f,
                            soar_callback_data cb_data, soar_callback_free_fn free_fn, const char *name)
{
    soar_cAddCallback(a, INPUT_PHASE_CALLBACK, f, cb_data, free_fn, name);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cRemoveInputFunction
 *
 *     Remove a previously installed input function
 *
 *----------------------------------------------------------------------
 */
void soar_cRemoveInputFunction(agent * a, const char *name)
{
    soar_cRemoveCallback(a, INPUT_PHASE_CALLBACK, name);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cAddOutputFunction
 *
 *     Install a function to handle output from the agent
 *
 *----------------------------------------------------------------------
 */
void soar_cAddOutputFunction(agent * a, soar_callback_fn f,
                             soar_callback_data cb_data, soar_callback_free_fn free_fn, const char *output_link_name)
{
    if (soar_exists_callback_id(a, OUTPUT_PHASE_CALLBACK, output_link_name)
        != NULL) {
        print("Error: tried to add_output_function with duplicate name %s\n", output_link_name);
        control_c_handler(0);
    } else {
        soar_cAddCallback(a, OUTPUT_PHASE_CALLBACK, f, cb_data, free_fn, output_link_name);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cRemoveOutputFunction
 *
 *     Remove a previously installed output function
 *
 *----------------------------------------------------------------------
 */
void soar_cRemoveOutputFunction(agent * a, const char *name)
{
    soar_callback *cb;
    output_link *ol;

    /* Remove indexing structures ... */

    cb = soar_exists_callback_id(a, OUTPUT_PHASE_CALLBACK, name);
    if (!cb)
        return;

    for (ol = a->existing_output_links; ol != NIL; ol = ol->next) {
        if (ol->cb == cb) {
            /* Remove ol entry */
            ol->link_wme->output_link = NULL;
            wme_remove_ref(ol->link_wme);
            remove_from_dll(a->existing_output_links, ol, next, prev);
            free_with_pool(&(a->output_link_pool), ol);
            break;
        }
    }

    soar_cRemoveCallback(a, OUTPUT_PHASE_CALLBACK, name);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cPushCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cPushCallback(soar_callback_agent the_agent,
                        SOAR_CALLBACK_TYPE callback_type,
                        soar_callback_fn fn, soar_callback_data data, soar_callback_free_fn free_fn)
{
    soar_callback *cb;

    cb = (soar_callback *) malloc(sizeof(soar_callback));
    cb->function = fn;
    cb->data = data;
    cb->free_function = free_fn;
    cb->id = NULL;

/*
  printf( "Pushing callback function %p onto callback slot %d\n", 
	fn, callback_type );
  fflush( stdout );
*/

    push(cb, ((agent *) the_agent)->soar_callbacks[callback_type]);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cAddCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cAddCallback(soar_callback_agent the_agent,
                       SOAR_CALLBACK_TYPE callback_type,
                       soar_callback_fn fn, soar_callback_data data, soar_callback_free_fn free_fn, soar_callback_id id)
{
    soar_callback *cb;

    cb = (soar_callback *) malloc(sizeof(soar_callback));
    cb->function = fn;
    cb->data = data;
    cb->free_function = free_fn;
    cb->id = savestring((char *) id);

    push(cb, ((agent *) the_agent)->soar_callbacks[callback_type]);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cPopCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cPopCallback(soar_callback_agent the_agent, SOAR_CALLBACK_TYPE callback_type)
{
    list *head;
    soar_callback *cb;

    head = ((agent *) the_agent)->soar_callbacks[callback_type];

    if (head == NULL) {
        print_string("Attempt to remove non-existant callback.\n");
        return;
    }

    if ((callback_type == PRINT_CALLBACK)
        && (head->rest == NULL)) {
        print_string("Attempt to remove last print callback. Ignored.\n");
        return;
    }

    cb = (soar_callback *) head->first;

    ((agent *) the_agent)->soar_callbacks[callback_type] = head->rest;
    soar_destroy_callback(cb);
    free_cons(head);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cRemoveCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cRemoveCallback(soar_callback_agent the_agent, SOAR_CALLBACK_TYPE callback_type, soar_callback_id id)
{
    cons *c;
    cons *prev_c = NULL;        /* Initialized to placate gcc -Wall */
    list *head;

    head = ((agent *) the_agent)->soar_callbacks[callback_type];

	/* This is a non-standard for loop.
	   Because we have to juggle the elements of this list around as we are
	   deleting them, it is difficult to ensure that we can use a single
	   incrementer, like c = c->rest.  The primary problem is that, if we
	   delete the last item in the list, then the incrementer will fail
	   (since c will be NULL).  Because of this, we manually increment the list
	   inside the loop based on what the circumstances are.  Thus, the statements
	   which change the value of c are the incrementers.
	*/

    for (c = head; c != NULL; /*c = c->rest*/) {
        soar_callback *cb;

        cb = (soar_callback *) c->first;

        if (!strcmp(cb->id, id)) {
            if (c != head) {
                prev_c->rest = c->rest;
                soar_destroy_callback(cb);
                free_cons(c);
				c = prev_c->rest;
                /*return;*/
            } else {
                ((agent *) the_agent)->soar_callbacks[callback_type]
                    = head->rest;
                soar_destroy_callback(cb);
                free_cons(c);
				head = ((agent *) the_agent)->soar_callbacks[callback_type]; 
				c = head;
				prev_c = NULL;
                /*return;*/
            }

        }
		else {
			prev_c = c;
			c = c->rest;

		}

        /*prev_c = c;*/
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cAddGlobalCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cAddGlobalCallback(SOAR_GLOBAL_CALLBACK_TYPE callback_type,
                             soar_callback_fn fn,
                             soar_callback_data data, soar_callback_free_fn free_fn, soar_callback_id id)
{
    soar_callback *cb;

    cb = (soar_callback *) malloc(sizeof(soar_callback));
    cb->function = fn;
    cb->data = data;
    cb->free_function = free_fn;
    cb->id = savestring((char *) id);

    /* We can't use the push macro because it allocates memory
     *  from an agent's memory pool.
     */
    {
        cons *push_cons_xy298;
        push_cons_xy298 = (cons *) malloc(sizeof(cons));
        push_cons_xy298->first = (cb);
        push_cons_xy298->rest = (soar_global_callbacks[callback_type]);
        soar_global_callbacks[callback_type] = push_cons_xy298;
    }

}

/*
 *----------------------------------------------------------------------
 *
 * soar_cRemoveGlobalCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cRemoveGlobalCallback(SOAR_GLOBAL_CALLBACK_TYPE callback_type, soar_callback_id id)
{
    list *head;
    cons *c;
    cons *prev_c = NULL;        /* Initialized to placate gcc -Wall */
    soar_callback *cb;

    head = soar_global_callbacks[callback_type];

    for (c = head; c != NIL; c = c->rest) {

        cb = (soar_callback *) c->first;

        if (!strcmp(cb->id, id)) {
            if (c != head) {
                prev_c->rest = c->rest;
                soar_destroy_callback(cb);
                free_cons(c);
                return;
            } else {
                soar_global_callbacks[callback_type] = head->rest;
                soar_destroy_callback(cb);
                free_cons(c);
                return;
            }
        }
        prev_c = c;
    }
}

void soar_cListAllCallbacks(soar_callback_agent the_agent, bool monitorable_only)
{
    int limit;
    SOAR_CALLBACK_TYPE ct;

    if (monitorable_only) {
        limit = NUMBER_OF_MONITORABLE_CALLBACKS;
    } else {
        limit = NUMBER_OF_CALLBACKS;
    }

    for (ct = 1; ct < limit; ct++) {
        print("%s: ", soar_callback_enum_to_name(ct, FALSE));
        soar_cListAllCallbacksForEvent(the_agent, ct);
        print("\n");
    }
}

void soar_cListAllCallbacksForEvent(soar_callback_agent the_agent, SOAR_CALLBACK_TYPE ct)
{
    cons *c;

    for (c = ((agent *) the_agent)->soar_callbacks[ct]; c != NIL; c = c->rest) {
        soar_callback *cb;

        cb = (soar_callback *) c->first;

        print("%s ", cb->id);
    }
}

void soar_cRemoveAllMonitorableCallbacks(soar_callback_agent the_agent)
{
    SOAR_CALLBACK_TYPE ct;

    for (ct = 1; ct < NUMBER_OF_MONITORABLE_CALLBACKS; ct++) {
        soar_cRemoveAllCallbacksForEvent(the_agent, ct);
    }
}

void soar_cRemoveAllCallbacksForEvent(soar_callback_agent the_agent, SOAR_CALLBACK_TYPE ct)
{
    cons *c;
    list *next;

    next = ((agent *) the_agent)->soar_callbacks[ct];

    for (c = next; c != NIL; c = next) {
        soar_callback *cb;

        cb = (soar_callback *) c->first;

        next = next->rest;
        soar_destroy_callback(cb);
        free_cons(c);
    }

    ((agent *) the_agent)->soar_callbacks[ct] = NIL;
}

void soar_cTestAllMonitorableCallbacks(soar_callback_agent the_agent)
{
    SOAR_CALLBACK_TYPE i;
    static char *test_callback_name = "test";

    for (i = 1; i < NUMBER_OF_MONITORABLE_CALLBACKS; i++) {
        soar_cAddCallback(the_agent, i,
                          (soar_callback_fn) soar_cTestCallback,
                          soar_callback_enum_to_name(i, TRUE), NULL, test_callback_name);
    }
}

SOAR_CALLBACK_TYPE soar_cCallbackNameToEnum(const char *name, bool monitorable_only)
{
    int limit;
    SOAR_CALLBACK_TYPE i;

    if (monitorable_only) {
        limit = NUMBER_OF_MONITORABLE_CALLBACKS;
    } else {
        limit = NUMBER_OF_CALLBACKS;
    }

    for (i = 1; i < limit; i++) {
        if (!strcmp(name, soar_callback_names[i])) {
            return i;
        }
    }

    return NO_CALLBACK;
}

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 5:    ETC
 *
 *
 *               - Wme Accessors
 *               - AddWme Wrappers
 *               - Multi Agent Controls
 *
 * *************************************************************************
 * *************************************************************************
 */

/*
 * string must be freed using free()
 * I am refraining from using Soar's internal memory because strings,
 * like all other memory pools are allocated on a per-agent basis.
 * This means that the user would have to ensure that the same agent
 * was selected when one of the wme accessor functions was called 
 * as was selected when the returned string was freed.  Otherwise,
 * untold things could happen.  Thus here, I opt for the potentially
 * slower, but safer, accessor function.
 */
char *soar_cGetWmeId(psoar_wme w, char *buff, size_t buff_size)
{
    char *temp;
    char *ret;

    temp = symbol_to_string(((wme *) w)->id, TRUE, buff, buff_size);
    if (buff)
        return buff;

    ret = (char *) malloc((strlen(temp) + 1) * sizeof(char));
    strcpy(ret, temp);          /* this is relatively safe since the memory is allocated on the previous line */

    return ret;
}

char *soar_cGetWmeAttr(psoar_wme w, char *buff, size_t buff_size)
{
    char *temp;
    char *ret;

    temp = symbol_to_string(((wme *) w)->attr, TRUE, buff, buff_size);
    if (buff)
        return buff;

    ret = (char *) malloc((strlen(temp) + 1) * sizeof(char));
    strcpy(ret, temp);          /* this is relatively safe since the memory is allocated on the previous line */

    return ret;

}

char *soar_cGetWmeValue(psoar_wme w, char *buff, size_t buff_size)
{
    char *temp;
    char *ret;

    temp = symbol_to_string(((wme *) w)->value, TRUE, buff, buff_size);
    if (buff)
        return buff;

    ret = (char *) malloc((strlen(temp) + 1) * sizeof(char));
    strcpy(ret, temp);          /* this is relatively safe since the memory is allocated on the previous line */

    return ret;

}

unsigned long soar_cGetWmeTimetag(psoar_wme w)
{
    return ((wme *) w)->timetag;
}

#define SOAR_CADDINTWME_TEMP_SIZE 128
unsigned long soar_cAddIntWme(char *szId, char *szAttr, int value, bool acceptable, psoar_wme * w)
{
    char temp[SOAR_CADDINTWME_TEMP_SIZE];

    snprintf(temp, SOAR_CADDINTWME_TEMP_SIZE, "%d", value);
    temp[SOAR_CADDINTWME_TEMP_SIZE - 1] = 0;    /* snprintf doesn't set last char to null if output is truncated */

    return soar_cAddWme(szId, szAttr, temp, acceptable, w);
}

#define SOAR_CADDFLOATWME_TEMP_SIZE 128
unsigned long soar_cAddFloatWme(char *szId, char *szAttr, float value, bool acceptable, psoar_wme * w)
{
    char temp[SOAR_CADDFLOATWME_TEMP_SIZE];

    snprintf(temp, SOAR_CADDFLOATWME_TEMP_SIZE, "%f", value);
    temp[SOAR_CADDFLOATWME_TEMP_SIZE - 1] = 0;  /* snprintf doesn't set last char to null if output is truncated */

    return soar_cAddWme(szId, szAttr, temp, acceptable, w);
}

void soar_cInitAgentIterator(soar_apiAgentIterator * ai)
{
    cons *c;

    ai->_begin = NIL;

    for (c = all_soar_agents; c != NIL; c = c->rest) {
        if (((agent *) c->first) == soar_agent) {
            ai->_begin = c;
            ai->_current = c;
            ai->more = (bool) ((agent_count > 0) ? TRUE : FALSE);
        }
    }
    if (ai->_begin == NIL) {
        print("ERROR!!!!!!!!!!");
    }

}

bool soar_cStepAgentIterator(soar_apiAgentIterator * ai)
{

    ai->_current = ai->_current->rest;
    if (ai->_current == NIL) {
        ai->_current = all_soar_agents; /* cycle to beginning of list */
    }
    ai->more = TRUE;

    if (ai->_current == ai->_begin) {
        ai->more = FALSE;
    } else if (ai->_current->rest == NIL && all_soar_agents == ai->_current) {
        ai->more = FALSE;
    }

    soar_agent = ai->_current->first;

    return ai->more;
}

psoar_agent soar_cGetAgentByName(char *name)
{
    cons *c;

    for (c = all_soar_agents; c != NIL; c = c->rest) {
        if (!strcmp(((agent *) c->first)->name, name)) {
            return (psoar_agent) c->first;
        }
    }
    return NIL;

}

int soar_cGetIdForAgentByName(char *name)
{
    psoar_agent a;

    a = soar_cGetAgentByName(name);
    if (!a)
        return -1;

    return ((agent *) a)->id;

}

bool soar_cSetCurrentAgentByName(char *name)
{
    psoar_agent a;

    a = soar_cGetAgentByName(name);
    if (!a)
        return FALSE;

    soar_agent = (agent *) a;
    return TRUE;
}

void soar_cSetCurrentAgent(psoar_agent a)
{

    soar_agent = (agent *) a;
}

psoar_agent soar_cGetCurrentAgent()
{
    return (psoar_agent) soar_agent;
}

/*
 * string must be freed using free()
 * I am refraining from using Soar's internal memory because strings,
 * like all other memory pools are allocated on a per-agent basis.
 * This means that the user would have to ensure that the same agent
 * was selected when  the accessor function was called 
 * as was selected when the returned string was freed.  Otherwise,
 * untold things could happen.  Thus here, I opt for the potentially
 * slower, but safer, accessor function.
 */

char *soar_cGetAgentInputLinkId(psoar_agent a, char *buff, size_t buff_size)
{
    char *temp;
    char *ret;

    if (((agent *) a)->io_header_input == NULL) {
        if (buff)
            *buff = '\0';
        return "";
    }

    temp = symbol_to_string(((agent *) a)->io_header_input, TRUE, buff, buff_size);
    if (buff)
        return buff;

    ret = (char *) malloc((strlen(temp) + 1) * sizeof(char));
    strcpy(ret, temp);          /* this is relatively safe since the memory is allocated on the previous line */

    return ret;
}

char *soar_cGetAgentOutputLinkId(psoar_agent a, char *buff, size_t buff_size)
{
    char *temp;
    char *ret;

    if (((agent *) a)->io_header_output == NULL) {
        if (buff)
            *buff = '\0';
        return "";
    }

    temp = symbol_to_string(((agent *) a)->io_header_output, TRUE, buff, buff_size);
    if (buff)
        return buff;

    ret = (char *) malloc((strlen(temp) + 1) * sizeof(char));
    strcpy(ret, temp);          /* this is relatively safe since the memory is allocated on the previous line */

    return ret;
}

int soar_cGetAgentId(psoar_agent a)
{

    if (a == NULL)
        return -1;

    return ((agent *) a)->id;
}

void soar_cTestCallback(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data)
{
    the_agent = the_agent;      /* stops compiler warning */
    call_data = call_data;      /* stops compiler warning */

    print("%s test callback executed.\n", (char *) data);
}

#define SOAR_CDEFAULTASKCALLBACK_TEMP_SIZE 50
void soar_cDefaultAskCallback(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data)
{

    int num_candidates, chosen_num;
    preference *cand;

    the_agent = the_agent;      /* stops compiler warning */
    data = data;                /* stops compiler warning */

    *((soar_apiAskCallbackData *) call_data)->selection = NULL;

    num_candidates = 0;
    print("\nPlease choose one of the following:\n");
    for (cand = ((soar_apiAskCallbackData *) call_data)->candidates; cand != NIL; cand = cand->next_candidate) {

        num_candidates++;
        print("  %d:  ", num_candidates);
        print_object_trace(cand->value);
        print("\n");
    }

    /* AGR 615 begin */
    print("Or choose one of the following to change the user-select mode\n");
    print("to something else:  %d (first)", ++num_candidates);
    print(", %d (last)", ++num_candidates);
    print(", %d (random)\n", ++num_candidates);
    /* AGR 615 end */
    for (;;) {
        char ch;

        /*  char buf[256]; *//* kjh(CUSP-B10) */
        print("Enter selection (1-%d): ", num_candidates);
        chosen_num = -1;
        scanf(" %d", &chosen_num);
        do {
            ch = (char) getchar();
        } while ((ch != '\n') && (ch != EOF_AS_CHAR));

        if (ch == EOF_AS_CHAR)
            clearerr(stdin);    /* Soar-Bugs #103, TMH */

        /* kjh(CUSP-B10) BEGIN */
        /* Soar_Read(soar_agent, buf, 256);
           sscanf(buf,"%d",&chosen_num); */
        /* kjh(CUSP-B10) END */

        if ((chosen_num >= 1) && (chosen_num <= num_candidates))
            break;
        print("You must enter a number between 1 and %d\n", num_candidates);
    }
    if (current_agent(logging_to_file)) {
        char temp[SOAR_CDEFAULTASKCALLBACK_TEMP_SIZE];
        snprintf(temp, SOAR_CDEFAULTASKCALLBACK_TEMP_SIZE, "%d\n", chosen_num);
        temp[SOAR_CDEFAULTASKCALLBACK_TEMP_SIZE - 1] = 0;       /* snprintf doesn't set last char to null if output is truncated */
        print_string_to_log_file_only(temp);
    }
    /* AGR 615 begin */
    switch (num_candidates - chosen_num) {

    case 2:
        set_sysparam(USER_SELECT_MODE_SYSPARAM, USER_SELECT_FIRST);
        print("User-select mode changed to:  first\n");
        *((soar_apiAskCallbackData *) call_data)->selection = ((soar_apiAskCallbackData *) call_data)->candidates;
        break;

    case 1:
        set_sysparam(USER_SELECT_MODE_SYSPARAM, USER_SELECT_LAST);
        print("User-select mode changed to:  last\n");
        for (cand = ((soar_apiAskCallbackData *) call_data)->candidates;
             cand->next_candidate != NIL; cand = cand->next_candidate);

        *((soar_apiAskCallbackData *) call_data)->selection = cand;
        break;

    case 0:
        set_sysparam(USER_SELECT_MODE_SYSPARAM, USER_SELECT_RANDOM);
        print("User-select mode changed to:  random\n");

        chosen_num = sys_random() % (num_candidates - 3);

        cand = ((soar_apiAskCallbackData *) call_data)->candidates;
        while (chosen_num) {
            cand = cand->next_candidate;
            chosen_num--;
        }
        *((soar_apiAskCallbackData *) call_data)->selection = cand;
        break;

    default:
        cand = ((soar_apiAskCallbackData *) call_data)->candidates;
        while (chosen_num > 1) {
            cand = cand->next_candidate;
            chosen_num--;
        }
        *((soar_apiAskCallbackData *) call_data)->selection = cand;
    }
    /* AGR 615 end */

    return;
}
