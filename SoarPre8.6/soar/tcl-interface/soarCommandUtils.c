/*
 * =======================================================================
 *  File:  soarCommandUtils.c
 *
 * This file includes the utility routines for supporting the
 * Soar Command set, which is found in soarCommands.c.  
 *
 * =======================================================================
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
 */

#include <ctype.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "soar.h"
#include "soarCommandUtils.h"
#include "soar_core_api.h"

#define MAXPATHLEN 1024

extern Tcl_Interp *tcl_soar_agent_interpreters[MAX_SIMULTANEOUS_AGENTS];

/* DJP 4/3/96 */
/*----------------------------------------------------------------------
 *
 * install_tcl_soar_cmd --
 *
 *	This procedure installs a new Soar command in the Tcl
 *      interpreter.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	Command is installed in the Tcl interpreter.
 *
 *----------------------------------------------------------------------
 */

void install_tcl_soar_cmd(agent * the_agent, char *cmd_name, Tcl_ObjCmdProc * cmd_proc)
{
    Tcl_CreateObjCommand(tcl_soar_agent_interpreters[the_agent->id], cmd_name, cmd_proc, the_agent, NULL);
}

/* 
 *----------------------------------------------------------------------
 * voigtjr 2003-Oct-16:
 *
 * create_argv_from_objv --
 *
 * The new version of install_tcl_soar_cmd uses Tcl_ObjCmdProc.
 * Tcl_ObjCmdProc uses "objc" and "objv" (of type Tcl_Obj) instead of
 * "argc" and "argv" (of type char*).  The various commands used to pass
 * the argv straight to the kernel.  They cannot pass the objv to the
 * kernel, because that would violate the kernel/tcl separation.
 *
 * The quick fix is to use the following function to convert the objv to
 * an argv that they can pass directly to the kernel.
 *
 * Update: argv[argc] == NULL must be true
 *----------------------------------------------------------------------
 */
void create_argv_from_objv(int objc, Tcl_Obj * const *objv, char ***argv)
{
    int i;
    char *str = NULL;
    int len = 0;

    if (objc <= 0) {
        *argv = NULL;
        return;
    }

    *argv = (char **) malloc((objc + 1) * sizeof(char *));

    for (i = 0; i < objc; i++) {
        str = Tcl_GetStringFromObj(objv[i], &len);
        (*argv)[i] = (char *) malloc((len + 1) * sizeof(char));
        strncpy((*argv)[i], str, len);
        (*argv)[i][len] = '\0';
    }
	(*argv)[i] = NULL;
}

void free_argv(int objc, char **argv)
{
    int i;

    if (argv == NULL) {
        return;
    }

    for (i = 0; i < objc; i++) {
        free(argv[i]);
    }
    free(argv);
}

/* 
 *----------------------------------------------------------------------
 * voigtjr 2003-Oct-15:
 * This is the old way of doing install_tcl_soar_cmd, here to provide
 * a way to do incremental updating of the functions to use 
 * Tcl_CreateObjCommand() instead of Tcl_CreateCommand()
 *
 * install_tcl_soar_cmd --
 *
 *	This procedure installs a new Soar command in the Tcl
 *      interpreter.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	Command is installed in the Tcl interpreter.

 *----------------------------------------------------------------------
 */
void install_tcl_soar_cmd_deprecated(agent * the_agent, char *cmd_name, Tcl_CmdProc * cmd_proc)
{
    Tcl_CreateCommand(tcl_soar_agent_interpreters[the_agent->id], cmd_name, cmd_proc, (ClientData) the_agent, NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_callback_to_tcl --
 *
 *	This procedure invokes the Tcl interpreter of an agent
 *      from a Soar callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Anything -- depends on script to invoke.
 *
 *----------------------------------------------------------------------
 */

void soar_callback_to_tcl(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data)
{
    int code;

    code = Tcl_EvalEx(tcl_soar_agent_interpreters[((agent *) the_agent)->id], (char *) data, -1, TCL_EVAL_GLOBAL);
    if (code != TCL_OK) {
        print("Error: Failed callback attempt to globally eval: %s\n", (char *) data);
        print("Reason: %s\n", Tcl_GetObjResult(tcl_soar_agent_interpreters[((agent *) the_agent)->id]));
        control_c_handler(0);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_input_callback_to_tcl --
 *
 *	This procedure invokes the Tcl interpreter of an agent
 *      from a Soar input callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Anything -- depends on script to invoke.
 *
 *----------------------------------------------------------------------
 */

void soar_input_callback_to_tcl(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data)
{
    Tcl_DString command;
    int mode;
    char *mode_name;
    int code;

    mode = (int) call_data;
    if (mode == TOP_STATE_JUST_CREATED) {
        mode_name = "top-state-just-created";
    } else if (mode == NORMAL_INPUT_CYCLE) {
        mode_name = "normal-input-cycle";
    } else if (mode == TOP_STATE_JUST_REMOVED) {
        mode_name = "top-state-just-removed";
    } else {
        print("Error calling Tcl input procedure callback function -- unrecognized mode: %d", mode);
        return;
    }

    Tcl_DStringInit(&command);
    Tcl_DStringAppend(&command, data, strlen(data));
    Tcl_DStringAppendElement(&command, mode_name);

    code = Tcl_EvalEx(tcl_soar_agent_interpreters[((agent *) the_agent)->id],
                      (char *) Tcl_DStringValue(&command), -1, TCL_EVAL_GLOBAL);

    if (code != TCL_OK) {
        print("Error: Failed callback attempt to globally eval: %s\n", (char *) Tcl_DStringValue(&command));
        print("Reason: %s\n", Tcl_GetObjResult(tcl_soar_agent_interpreters[((agent *) the_agent)->id]));
        Tcl_DStringFree(&command);
        control_c_handler(0);
    }

    Tcl_DStringFree(&command);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_output_callback_to_tcl --
 *
 *	This procedure invokes the Tcl interpreter of an agent
 *      from a Soar output callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Anything -- depends on script to invoke.
 *
 *----------------------------------------------------------------------
 */

#define SOAR_OUTPUT_CALLBACK_STRING_SIZE 1000

void soar_output_callback_to_tcl(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data)
{
    Tcl_DString command;
    output_call_info *output_info;
    int mode;
    io_wme *outputs, *wme;
    char *mode_name;
    int code;

    output_info = (output_call_info *) call_data;
    mode = output_info->mode;
    outputs = output_info->outputs;

    if (mode == ADDED_OUTPUT_COMMAND) {
        mode_name = "added-output-command";
    } else if (mode == MODIFIED_OUTPUT_COMMAND) {
        mode_name = "modified-output-command";
    } else if (mode == REMOVED_OUTPUT_COMMAND) {
        mode_name = "removed-output-command";
    } else {
        print("Error calling Tcl output procedure callback function -- unrecognized mode: %d", mode);
        return;
    }

    Tcl_DStringInit(&command);
    Tcl_DStringAppend(&command, data, strlen(data));
    Tcl_DStringAppendElement(&command, mode_name);
    Tcl_DStringAppend(&command, " { ", 3);

    /* Build output list */
    for (wme = outputs; wme != NIL; wme = wme->next) {
        char wme_string[SOAR_OUTPUT_CALLBACK_STRING_SIZE];
        char obj_string[SOAR_OUTPUT_CALLBACK_STRING_SIZE];
        char attr_string[SOAR_OUTPUT_CALLBACK_STRING_SIZE];
        char value_string[SOAR_OUTPUT_CALLBACK_STRING_SIZE];

        /* BUGZILLA BUG #1: symbol_to_string doesn't guarantee these are safe */
        symbol_to_string(wme->id, FALSE, obj_string, SOAR_OUTPUT_CALLBACK_STRING_SIZE);
        symbol_to_string(wme->attr, FALSE, attr_string, SOAR_OUTPUT_CALLBACK_STRING_SIZE);
        symbol_to_string(wme->value, FALSE, value_string, SOAR_OUTPUT_CALLBACK_STRING_SIZE);

        snprintf(wme_string, SOAR_OUTPUT_CALLBACK_STRING_SIZE, "%s %s %s", obj_string, attr_string, value_string);
        wme_string[SOAR_OUTPUT_CALLBACK_STRING_SIZE - 1] = 0;   /* snprintf doesn't set last char to null if output is truncated */

        Tcl_DStringAppendElement(&command, wme_string);
    }

    Tcl_DStringAppend(&command, " }", 2);

    code = Tcl_EvalEx(tcl_soar_agent_interpreters[((agent *) the_agent)->id],
                      (char *) Tcl_DStringValue(&command), -1, TCL_EVAL_GLOBAL);

    if (code != TCL_OK) {
        print("Error: Failed callback attempt to globally eval: %s\n", (char *) Tcl_DStringValue(&command));
        print("Reason: %s\n", Tcl_GetObjResult(tcl_soar_agent_interpreters[((agent *) the_agent)->id]));
        Tcl_DStringFree(&command);
        control_c_handler(0);
    }

    Tcl_DStringFree(&command);
}

#define CANDID_SIZE 56
#define SOAR_CDEFAULTASKCALLBACK_TEMP_SIZE 50
void soar_ask_callback_to_tcl(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data)
{
    Tcl_DString command;
    int result;
    int num_candidates, code;
    preference *cand;

    Tcl_DStringInit(&command);
    Tcl_DStringAppend(&command, data, strlen(data));

    /* Build output list */
    num_candidates = 0;
    for (cand = ((soar_apiAskCallbackData *) call_data)->candidates; cand != NIL; cand = cand->next_candidate) {

        char candId[CANDID_SIZE];

        symbol_to_string(cand->value, FALSE, candId, CANDID_SIZE);
        Tcl_DStringAppendElement(&command, candId);
        num_candidates++;
    }

    code = Tcl_EvalEx(tcl_soar_agent_interpreters[((agent *) the_agent)->id],
                      (char *) Tcl_DStringValue(&command), -1, TCL_EVAL_GLOBAL);

    num_candidates += 3;        /* add 3 because first, last, random are always choices */

    if (code == TCL_OK) {
        /*result = atoi( tcl_soar_agent_interpreters[((agent *)the_agent)->id]->result ); */

        Tcl_GetIntFromObj(tcl_soar_agent_interpreters[((agent *) the_agent)->id],
                          Tcl_GetObjResult(tcl_soar_agent_interpreters[((agent *) the_agent)->id]), &result);

        if (result < 0 || result >= num_candidates) {
            print("Error: ask callback returned a value out of bounds: %d (0-%d)\n", result, num_candidates - 1);
            result = 0;
        }

    } else {
        print("Error: Failed callback attempt to globally eval: %s\n", (char *) Tcl_DStringValue(&command));
        print("Reason: %s\n", Tcl_GetObjResult(tcl_soar_agent_interpreters[((agent *) the_agent)->id]));
        Tcl_DStringFree(&command);
        result = 0;
        control_c_handler(0);
    }

    if (current_agent(logging_to_file)) {
        char temp[SOAR_CDEFAULTASKCALLBACK_TEMP_SIZE];
        snprintf(temp, SOAR_CDEFAULTASKCALLBACK_TEMP_SIZE, "%d\n", result);
        temp[SOAR_CDEFAULTASKCALLBACK_TEMP_SIZE - 1] = 0;       /* snprintf doesn't set last char to null if output is truncated */
        print_string_to_log_file_only(temp);
    }

    switch (num_candidates - result - 1) {

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

        result = sys_random() % (num_candidates - 3);

        cand = ((soar_apiAskCallbackData *) call_data)->candidates;
        while (result) {
            cand = cand->next_candidate;
            result--;
        }
        *((soar_apiAskCallbackData *) call_data)->selection = cand;
        break;

    default:
        cand = ((soar_apiAskCallbackData *) call_data)->candidates;
        while (result > 0) {
            cand = cand->next_candidate;
            result--;
        }
        *((soar_apiAskCallbackData *) call_data)->selection = cand;
    }

    Tcl_DStringFree(&command);
}

Soar_TextWidgetPrintData *Soar_MakeTextWidgetPrintData(Tcl_Interp * interp, const char *widget_name)
{
    Soar_TextWidgetPrintData *data;

    data = (Soar_TextWidgetPrintData *)
        malloc(sizeof(Soar_TextWidgetPrintData));
    data->interp = interp;
    data->text_widget = savestring(widget_name);

    return data;
}

extern void Soar_FreeTextWidgetPrintData(Soar_TextWidgetPrintData * data)
{
    free((void *) data->text_widget);
    free((void *) data);
}

/* kjh(CUSP-B2) begin */
extern Symbol *make_symbol_for_current_lexeme(void);

typedef struct wme_filter_struct {
    Symbol *id;
    Symbol *attr;
    Symbol *value;
    bool adds;
    bool removes;
} wme_filter;
