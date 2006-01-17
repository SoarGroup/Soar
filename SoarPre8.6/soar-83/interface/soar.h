/*
 * soar.h --
 *
 *      This header file describes the externally-visible facilities
 *      of the Soar system interface to Tcl.
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


#ifndef SOAR_H_INCLUDED
#define SOAR_H_INCLUDED

#include <tcl.h>

#ifdef USE_TK
#include <tk.h>
#endif /* USE_TK */

#include <soarkernel.h>
#include <callback.h>

/*----------------------------------------------------------------------*/

/* ARGC/ARGV Processing structures and functions.                       */

/*
 * Structure used to specify how to handle argv options.
 */

typedef struct {
    char *key;		/* The key string that flags the option in the
			 * argv array. */
    int type;		/* Indicates option type;  see below. */
    char *src;		/* Value to be used in setting dst;  usage
			 * depends on type. */
    char *dst;		/* Address of value to be modified;  usage
			 * depends on type. */
    char *help;		/* Documentation message describing this option. */
} Soar_ArgvInfo;


/*
 * Legal values for the type field of a Soar_ArgvInfo: see the user
 * documentation for details.
 */

#define SOAR_ARGV_CONSTANT		1
#define SOAR_ARGV_STRING		2
#define SOAR_ARGV_REST			3
#define SOAR_ARGV_HELP			4
#define SOAR_ARGV_AGENT			5
#define SOAR_ARGV_TCLSH			6
#define SOAR_ARGV_WISH			7
#define SOAR_ARGV_END			8

/*
 * Flag bits for passing to Soar_ParseArgv:
 */

#define SOAR_ARGV_NO_LEFTOVERS		0x1

typedef struct {
  int start_argv;
  int end_argv;
} soar_argv_list;

typedef struct {
  int   tk_enabled;
  int   ipc_enabled;
  int   synchronize;
  int   verbose;
  char *path;
  char *display;
  char *fileName;
  char *geometry;
  soar_argv_list names;
} option_table;

extern int Soar_ParseArgv (int * argcPtr, char ** argv, 
			   Soar_ArgvInfo * argTable, int flags, 
			   int * srcIndex, int * dstIndex,
			   int * interp_type, int * options_done);

/*----------------------------------------------------------------------*/

/* 
 * The following structure is used to keep track of the interpreters 
 * registered by this process.
 */

typedef struct RegisteredInterp {
    Tcl_Interp *interp;		/* Interpreter associated with name.  NULL
				 * means that the application was unregistered
				 * or deleted while a send was in progress
				 * to it. */
    agent * agentPtr;           /* Agent associated with name. */
    struct RegisteredInterp *nextPtr;
				/* Next in list of names associated
				 * with interps in this process.
				 * NULL means end of list. */
} RegisteredInterp;

extern RegisteredInterp *registry;
				/* List of all interpreters
				 * registered by this process. */

extern RegisteredInterp * mainInterp;  
                                /* The current active top-level interp. */
/*----------------------------------------------------------------------*/

typedef struct Soar_TextWidgetPrintData {
    Tcl_Interp *interp;		/* Interpreter containing text widget */
    char * text_widget;         /* Text widget to print in. */
  } Soar_TextWidgetPrintData;

/*----------------------------------------------------------------------*/

extern void Soar_AgentInit (agent * a);
extern void Soar_AppendResult (agent *, soar_callback_data, soar_call_data);
extern void Soar_DestroyRegisterEntry (RegisteredInterp * riPtr);
extern void Soar_DiscardPrint (agent *, soar_callback_data, soar_call_data);
extern void Soar_FClose (FILE *);
extern void Soar_FreeTextWidgetPrintData (Soar_TextWidgetPrintData * data);
extern RegisteredInterp * Soar_GetRegisteredInterp (char * name);
extern RegisteredInterp * Soar_GetRegisteredInterpByInterp (Tcl_Interp * interp);
extern void Soar_InstallCommands(agent * new_agent);
extern void Soar_LinkInterpVars2Agent(Tcl_Interp * interp, agent * new_agent);
extern void Soar_Log (agent *, char *);
extern void Soar_LogAndPrint (agent *, char *);
extern void Soar_Main (int argc, char ** argv);
extern RegisteredInterp * Soar_MakeRegisterEntry (Tcl_Interp * interp, agent * agentPtr);
extern Soar_TextWidgetPrintData *
       Soar_MakeTextWidgetPrintData (Tcl_Interp * interp, char * widget_name);
extern int  Soar_NameInRegistry (char * name);
extern void Soar_Print (agent *, char *);
extern void Soar_PrintToFile (agent *, soar_callback_data, soar_call_data);
extern void Soar_PrintToChannel (agent *, soar_callback_data, soar_call_data);
extern void Soar_PrintToTextWidget (agent *, soar_callback_data, 
				    soar_call_data);
/* extern char *Soar_Read (agent *, char *, int); /* kjh(CUSP-B10)*/
/* extern void Soar_RecordToFile (agent *, soar_callback_data, soar_call_data); /* kjh(CUSP-B10)*/
/* extern void Soar_ReadFromFile (agent *, soar_callback_data, soar_call_data); /* kjh(CUSP-B10)*/
/* extern void Soar_EndReplay(agent *); /* kjh(CUSP-B10)*/
/* Soar_PrintToTclProc RMJ 7-1-97 */
extern void Soar_PrintToTclProc (agent *, soar_callback_data, 
				    soar_call_data);
extern void Soar_RegisterArgv (int argc, char ** argv, 
			       RegisteredInterp * Rinterp);
extern void Soar_SelectGlobalInterpByInterp (Tcl_Interp * interp);
extern int  Soar_UseIPC ();

/* In tkText.c */
EXTERN void Tk_PrintToTextWidget _ANSI_ARGS_((Tcl_Interp *interp,
					      ClientData data,
					      ClientData call_data));

#endif /* Recursive include guard */

