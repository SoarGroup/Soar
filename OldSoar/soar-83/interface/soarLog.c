/*
 * =======================================================================
 *  File:  soarLog.c
 *
 * Interface routines to handle printing and logging for agents.
 *
 * =======================================================================
 *
 *
 * Copyright 1995-2003 Carnegie Mellon University,
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

void
Soar_Print (the_agent, str)
     agent * the_agent;
     char * str;
{
  soar_invoke_first_callback(the_agent, PRINT_CALLBACK, (ClientData) str);
}

void
Soar_Log (the_agent, str)
     agent * the_agent;
     char * str;
{
  soar_invoke_first_callback(the_agent, LOG_CALLBACK, (ClientData) str);
}

void
Soar_LogAndPrint (the_agent, str)
     agent * the_agent;
     char * str;
{
  Soar_Log(the_agent, str);
  Soar_Print(the_agent, str);
}

void
Soar_PrintToFile (the_agent, data, call_data)
     agent * the_agent;	
     soar_callback_data data;
     soar_call_data call_data;
{
  FILE * f = (FILE *) data;
  fputs((char *) call_data, f);
}

void
Soar_PrintToChannel(the_agent, data, call_data)
	agent * the_agent;
	soar_callback_data data;
	soar_call_data call_data;
{
	Tcl_Channel channel = (Tcl_Channel) data;
	Tcl_Write(channel, (char*) call_data, strlen((char*) call_data));
	Tcl_Flush(channel);
}

void
Soar_PrintToTextWidget (the_agent, data, call_data)
     agent * the_agent;	
     soar_callback_data data;
     soar_call_data call_data;
{
	Soar_TextWidgetPrintData * print_data = (Soar_TextWidgetPrintData *) data;

	char buf[1024];
	sprintf(buf, "%s insert end \"%s\" ", print_data->text_widget, (char*) call_data);
	Tcl_Eval(the_agent->interpreter, buf);

        /* RMJ 7-1-97 */
        sprintf(buf, "%s see end", print_data->text_widget);
	Tcl_Eval(the_agent->interpreter, buf);
	Tcl_Eval(the_agent->interpreter, "update");
}

/* RMJ 7-1-97 */
/* Uses text_widget as procedure name and passes the string to the proc */
/* Modified 8-19-98 to surround string with dbl quotes instead of braces
 * so productions are printed properly.  (productions have braces in
 * them and any line with braces was getting dropped) kjc
 */
void
Soar_PrintToTclProc (the_agent, data, call_data)
     agent * the_agent;	
     soar_callback_data data;
     soar_call_data call_data;
{
	Soar_TextWidgetPrintData * print_data = (Soar_TextWidgetPrintData *) data;

	char buf[1024];
	/*    printf("args:   %s<>%s\n", print_data->text_widget, (char*) call_data); */
	sprintf(buf, "%s \"%s\" ", print_data->text_widget, (char*) call_data);
	/*    printf("here's the buf: >> %s\nendbuf\n",buf);  */

	Tcl_Eval(the_agent->interpreter, buf);
	Tcl_Eval(the_agent->interpreter, "update");
}

void
Soar_DiscardPrint (the_agent, data, call_data)
     agent * the_agent;
     soar_callback_data data;
     soar_call_data call_data;
{
  /* No need to do anything */
}

void
Soar_AppendResult (the_agent, data, call_data)
     agent * the_agent;
     soar_callback_data data;
     soar_call_data call_data;
{
  Tcl_AppendResult(the_agent->interpreter, 
	           (char *) call_data, 
	           (char *) NULL);
}

void
Soar_FClose (FILE * f)
{
  fclose(f);
}

