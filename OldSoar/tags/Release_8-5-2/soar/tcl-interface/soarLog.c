/*
 * =======================================================================
 *  File:  soarLog.c
 *
 * Interface routines to handle printing and logging for agents.
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

#include <string.h>
#include "soar.h"

extern Tcl_Interp *tcl_soar_agent_interpreters[MAX_SIMULTANEOUS_AGENTS];

#define BUF_SIZE 4096

void Soar_Print(the_agent, str)
agent *the_agent;
const char *str;
{
    soar_invoke_first_callback(the_agent, PRINT_CALLBACK, (ClientData) str);
}

void Soar_Log(the_agent, str)
agent *the_agent;
const char *str;
{
    soar_invoke_first_callback(the_agent, LOG_CALLBACK, (ClientData) str);
}

void Soar_LogAndPrint(the_agent, str)
agent *the_agent;
const char *str;
{
    Soar_Log(the_agent, str);
    Soar_Print(the_agent, str);
}

void Soar_PrintToFile(the_agent, data, call_data)
agent *the_agent;
soar_callback_data data;
soar_call_data call_data;
{
    FILE *f = (FILE *) data;
    fputs((char *) call_data, f);
}

void Soar_PrintToChannel(the_agent, data, call_data)
agent *the_agent;
soar_callback_data data;
soar_call_data call_data;
{
    Tcl_Channel channel = (Tcl_Channel) data;
    Tcl_Write(channel, (char *) call_data, strlen((char *) call_data));
    Tcl_Flush(channel);
}

void Soar_PrintToTextWidget(the_agent, data, call_data)
agent *the_agent;
soar_callback_data data;
soar_call_data call_data;
{
    Soar_TextWidgetPrintData *print_data = (Soar_TextWidgetPrintData *) data;

    char buf[BUF_SIZE];
    snprintf(buf, BUF_SIZE, "%s {%s} ", print_data->text_widget, (char *) call_data);
    buf[BUF_SIZE - 1] = 0;      /* snprintf doesn't set last char to null if output is truncated */
    Tcl_EvalEx(tcl_soar_agent_interpreters[the_agent->id], buf, -1, 0);

    /* RMJ 7-1-97 */
    snprintf(buf, BUF_SIZE, "%s {%s} ", print_data->text_widget, (char *) call_data);
    buf[BUF_SIZE - 1] = 0;      /* snprintf doesn't set last char to null if output is truncated */
    Tcl_EvalEx(tcl_soar_agent_interpreters[the_agent->id], buf, -1, 0);
    Tcl_EvalEx(tcl_soar_agent_interpreters[the_agent->id], "update", -1, 0);
}

/* RMJ 7-1-97 */
/* Uses text_widget as procedure name and passes the string to the proc */
/* Modified 8-19-98 to surround string with dbl quotes instead of braces
 * so productions are printed properly.  (productions have braces in
 * them and any line with braces was getting dropped) kjc
 */
void Soar_PrintToTclProc(the_agent, data, call_data)
agent *the_agent;
soar_callback_data data;
soar_call_data call_data;
{
    char buf[BUF_SIZE];
    Soar_TextWidgetPrintData *print_data = (Soar_TextWidgetPrintData *) data;

    snprintf(buf, BUF_SIZE, "%s {%s} ", print_data->text_widget, (char *) call_data);
    buf[BUF_SIZE - 1] = 0;      /* snprintf doesn't set last char to null if output is truncated */

    Tcl_EvalEx(tcl_soar_agent_interpreters[the_agent->id], buf, -1, 0);
    Tcl_EvalEx(tcl_soar_agent_interpreters[the_agent->id], "update", -1, 0);
}

void Soar_DiscardPrint(the_agent, data, call_data)
agent *the_agent;
soar_callback_data data;
soar_call_data call_data;
{
    /* No need to do anything */
}

void Soar_AppendResult(the_agent, data, call_data)
agent *the_agent;
soar_callback_data data;
soar_call_data call_data;
{
    Tcl_AppendStringsToObj(Tcl_GetObjResult(tcl_soar_agent_interpreters[the_agent->id]),
                           (char *) call_data, (char *) NULL);
}

void Soar_FClose(FILE * f)
{
    fclose(f);
}
