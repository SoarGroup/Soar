/*
 * =======================================================================
 *  File:  soarVars.c
 *
 * Routines for linking Tcl variables to Soar C code variables.
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

#include "soar.h"

#define OLDVAL_SIZE 10

/* forward declarations of prototypes */

char *PositiveIntegerCheck(ClientData clientdata, Tcl_Interp * interp, const char *name1, const char *name2, int flags);
char *Mode_0_1_2_3_Check(ClientData clientData, Tcl_Interp * interp, const char *name1, const char *name2, int flags);
char *soar8Mode_Attr_Check(ClientData clientData, Tcl_Interp * interp, const char *name1, const char *name2, int flags);

/*
 *----------------------------------------------------------------------
 *
 * LinkInterpVars2Agent --
 *
 *      This procedure links various soar agent variables to Tcl vars. 
 *      The Tcl set command is then used to update these values.
 *      Trace routines are used to enforce allowable ranges of values.
 *
 *        Tcl Var            Agent parameter
 *        -------            ---------------
 *        attribute_preferences_mode
 *                           agent->attribute_preferences_mode
 *        default_wme_depth  agent->default_wme_depth
 *        max_chunks         agent->sysparams[MAX_CHUNKS_SYSPARAM]
 *        max_elaborations   agent->sysparams[MAX_ELABORATIONS_SYSPARAM]
 *        o_support_mode     agent->o_support_calculation_type
 *        save_backtraces    agent->sysparams[EXPLAIN_SYSPARAM]
 *        warnings           agent->sysparams[PRINT_WARNINGS_SYSPARAM]
 *        real_time_per_decision agent->sysparams[REAL_TIME_SYSPARAM]
 *        attention_lapsing  agent->attention_lapsing
 *
 *
 * Results:
 *      None
 *
 * Side effects:
 *      Tcl vars are linked to select agent variables.
 *
 *----------------------------------------------------------------------
 */
void Soar_LinkInterpVars2Agent(new_interp, new_agent)
Tcl_Interp *new_interp;
agent *new_agent;
{
    Tcl_LinkVar(new_interp,
                "attribute_preferences_mode", (char *) &new_agent->attribute_preferences_mode, TCL_LINK_INT);
    Tcl_TraceVar(new_interp,
                 "attribute_preferences_mode",
                 TCL_TRACE_WRITES, soar8Mode_Attr_Check, (ClientData) & new_agent->attribute_preferences_mode);
    Tcl_LinkVar(new_interp, "default_wme_depth", (char *) &new_agent->default_wme_depth, TCL_LINK_INT);
    Tcl_TraceVar(new_interp,
                 "default_wme_depth",
                 TCL_TRACE_WRITES, PositiveIntegerCheck, (ClientData) & new_agent->default_wme_depth);
    Tcl_LinkVar(new_interp, "max_chunks", (char *) &new_agent->sysparams[MAX_CHUNKS_SYSPARAM], TCL_LINK_INT);
    Tcl_TraceVar(new_interp,
                 "max_chunks",
                 TCL_TRACE_WRITES, PositiveIntegerCheck, (ClientData) & new_agent->sysparams[MAX_CHUNKS_SYSPARAM]
        );
    Tcl_LinkVar(new_interp,
                "max_elaborations", (char *) &new_agent->sysparams[MAX_ELABORATIONS_SYSPARAM], TCL_LINK_INT);
    Tcl_TraceVar(new_interp,
                 "max_elaborations",
                 TCL_TRACE_WRITES, PositiveIntegerCheck, (ClientData) & new_agent->sysparams[MAX_ELABORATIONS_SYSPARAM]
        );
    Tcl_LinkVar(new_interp, "o_support_mode", (char *) &new_agent->o_support_calculation_type, TCL_LINK_INT);
    Tcl_TraceVar(new_interp,
                 "o_support_mode",
                 TCL_TRACE_WRITES, Mode_0_1_2_3_Check, (ClientData) & new_agent->o_support_calculation_type);
    Tcl_LinkVar(new_interp, "save_backtraces", (char *) &new_agent->sysparams[EXPLAIN_SYSPARAM], TCL_LINK_BOOLEAN);
    Tcl_LinkVar(new_interp, "warnings", (char *) &new_agent->sysparams[PRINT_WARNINGS_SYSPARAM], TCL_LINK_BOOLEAN);
#ifdef REAL_TIME_BEHAVIOR
    /* RMJ; real-time execution parameter */
    Tcl_LinkVar(new_interp, "real_time_per_decision", (char *) &new_agent->sysparams[REAL_TIME_SYSPARAM], TCL_LINK_INT);
    Tcl_TraceVar(new_interp,
                 "real_time_per_decision",
                 TCL_TRACE_WRITES, PositiveIntegerCheck, (ClientData) & new_agent->sysparams[REAL_TIME_SYSPARAM]
        );
#endif

#ifdef ATTENTION_LAPSE
    /* RMJ; indicate whether agent is currently lapsing */
    Tcl_LinkVar(new_interp,
                "attention_lapsing", (char *) &new_agent->attention_lapsing, TCL_LINK_BOOLEAN | TCL_LINK_READ_ONLY);
#endif

}

/*
 *----------------------------------------------------------------------
 *
 *  PositiveIntegerCheck -
 *
 *      This is the trace function which enforces that certain
 *      soar-agent variables be set to positive integers.
 *
 *  Results:
 *      normally returns NULL;  if error occurs, the return value
 *      points to a static string containing the error msg.
 *
 *----------------------------------------------------------------------
 */

char *PositiveIntegerCheck(ClientData clientData, Tcl_Interp * interp, const char *name1, const char *name2, int flags)
{
    int n;
    const char *value;
    long *correct = clientData;
    char oldval[OLDVAL_SIZE];

    value = Tcl_GetVar(interp, name1, flags & TCL_GLOBAL_ONLY);
    Tcl_GetInt(interp, value, &n);

    if (n < 0) {
        snprintf(oldval, OLDVAL_SIZE, "%ld", *correct);
        oldval[OLDVAL_SIZE - 1] = 0;    /* snprintf doesn't set last char to null if output is truncated */
        Tcl_SetVar(interp, name1, oldval, flags & TCL_GLOBAL_ONLY);
        return "variable must be a positive integer, value unchanged";
    } else {
        return NULL;
    }
}

/*
 *----------------------------------------------------------------------
 *
 *  Mode_0_1_2_3_Check -
 *
 *      This is the trace function which enforces that certain
 *      soar-agent variables be set to either 0, 1, 2, or 3.
 *
 *  Results:
 *      normally returns NULL;  if error occurs, the return value
 *      points to a static string containing the error msg.
 *
 *----------------------------------------------------------------------
 */
char *Mode_0_1_2_3_Check(ClientData clientData, Tcl_Interp * interp, const char *name1, const char *name2, int flags)
{
    int n;
    const char *value;
    long *correct = clientData;
    char oldval[OLDVAL_SIZE];

    value = Tcl_GetVar(interp, name1, flags & TCL_GLOBAL_ONLY);
    Tcl_GetInt(interp, value, &n);

    if ((n < 0) || (n > 3)) {
        snprintf(oldval, OLDVAL_SIZE, "%ld", *correct);
        oldval[OLDVAL_SIZE - 1] = 0;    /* snprintf doesn't set last char to null if output is truncated */
        Tcl_SetVar(interp, name1, oldval, flags & TCL_GLOBAL_ONLY);
        return "variable must be 0, 1, 2, or 3; value unchanged";
    } else {
        return NULL;
    }
}

/*
 *----------------------------------------------------------------------
 *
 *  soar8Mode_Attr_Check -
 *
 *      This is the trace function which enforces that certain
 *      soar-agent variables be set to either 0, 1, or 2.
 *      In Soar8 Mode, attribute_preferences_mode is always 2. 
 *
 *  Results:
 *      normally returns NULL;  if error occurs, the return value
 *      points to a static string containing the error msg.
 *
 *----------------------------------------------------------------------
 */
char *soar8Mode_Attr_Check(ClientData clientData, Tcl_Interp * interp, const char *name1, const char *name2, int flags)
{
    int n;
    const char *value;
    long *correct = clientData;
    char oldval[OLDVAL_SIZE];

    value = Tcl_GetVar(interp, name1, flags & TCL_GLOBAL_ONLY);
    Tcl_GetInt(interp, value, &n);

    /* need to check in soar8 mode. if so mode must = 2 */
    if (current_agent(operand2_mode) == TRUE) {
        print("attr_pref_mode = %ld\n", n);
        if (n != 2) {
            snprintf(oldval, OLDVAL_SIZE, "%ld", *correct);
            oldval[OLDVAL_SIZE - 1] = 0;        /* snprintf doesn't set last char to null if output is truncated */
            Tcl_SetVar(interp, name1, oldval, flags & TCL_GLOBAL_ONLY);
            return "In soar8 mode, attr_pref_mode is obsolete.\nThe code automatically uses the value 2.";
        } else {
            return NULL;
        }
    } else {
        if ((n < 0) || (n > 2)) {
            snprintf(oldval, OLDVAL_SIZE, "%ld", *correct);
            oldval[OLDVAL_SIZE - 1] = 0;        /* snprintf doesn't set last char to null if output is truncated */
            Tcl_SetVar(interp, name1, oldval, flags & TCL_GLOBAL_ONLY);
            return "variable must be 0, 1, or 2; value unchanged";

        } else {
            return NULL;
        }
    }
}
