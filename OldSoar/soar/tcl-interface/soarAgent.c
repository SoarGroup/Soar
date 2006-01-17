/*
 * =======================================================================
 *  File:  soarAgent.c
 *
 * This file includes the routines for creating, initializing and destroying
 * Soar agents.  It also includes the code for the "Tcl" rhs function
 * which allows Soar agents to call Tcl functions on the rhs of productions.
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
#include "scheduler.h"

#ifdef WIN32
#include <direct.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif                          /* WIN32 */

/*
#if defined(MACINTOSH)
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif
*/

#include "soarCommands.h"

extern int agent_counter;

extern Tcl_Interp *tcl_soar_agent_interpreters[MAX_SIMULTANEOUS_AGENTS];

/* --------------------------------------------------------------------
                                Tcl 

   Sends a string to the Tcl interpreter
-------------------------------------------------------------------- */

Symbol *tcl_rhs_function_code(list * args)
{
    Symbol *arg;
    growable_string script_to_run;
    int result;

    if (!args) {
        print("Error: 'tcl' function called with no arguments.\n");
        return NIL;
    }

    script_to_run = make_blank_growable_string();

    for (; args != NIL; args = args->rest) {
        arg = args->first;
        /* --- Note use of FALSE here--print the symbol itself, not a rereadable
           version of it --- */
        add_to_growable_string(&script_to_run, symbol_to_string(arg, FALSE, NIL, 0));
    }

    result = Tcl_EvalEx(tcl_soar_agent_interpreters[current_agent(id)],
                        text_of_growable_string(script_to_run), -1, TCL_EVAL_GLOBAL);

    if (result != TCL_OK) {
        print("Error: Failed RHS Tcl evaluation of \"%s\"\n", text_of_growable_string(script_to_run));
        print("Reason: %s\n", Tcl_GetObjResult(tcl_soar_agent_interpreters[current_agent(id)]));
        control_c_handler(0);
        free_growable_string(script_to_run);
        return NIL;
    }

    free_growable_string(script_to_run);

    return make_sym_constant(Tcl_GetString(Tcl_GetObjResult(tcl_soar_agent_interpreters[current_agent(id)])));
}
