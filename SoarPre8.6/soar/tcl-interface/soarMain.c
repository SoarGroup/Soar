/* 
 * main.c --
 *
 *	Main program for Tcl shells and other Tcl-based applications.
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

#if defined(WIN32)
#include <windows.h>
#endif                          /* WIN32 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "soar.h"
#include "soarCommandUtils.h"
#include "soar_core_api.h"

extern void cb_tclSoar_NewAgent(soar_callback_agent a, soar_callback_data d, soar_call_data c);

/*
 * Declarations for various library procedures and variables (don't want
 * to include tkInt.h or tkPort.h here, because people might copy this
 * file out of the Tk source directory to make their own modified versions).
 * Note:  "exit" should really be declared here, but there's no way to
 * declare it without causing conflicts with other definitions elsewher
 * on some systems, so it's better just to leave it out.
 */

extern int isatty _ANSI_ARGS_((int fd));
extern int read _ANSI_ARGS_((int fd, char *buf, size_t size));

/*
 * Global variables used by the main program:
 */

RegisteredInterp *mainInterp = NULL;
                                /* The current active top-level interp. */
static Tcl_Interp *interp;      /* Interpreter for this application. */
static int tty;                 /* Non-zero means standard input is a
                                 * terminal-like device.  Zero means it's
                                 * a file. */
static char errorExitCmd[] = "exit 1";
static char *process_name;      /* Name used to start this process */

/*
 * Command-line options:
 */

static option_table options;    /* Table of interpreter creation options */

/**
 * Called when the DLL is first loaded into memory.
 */

#if defined(WIN32)
BOOL APIENTRY DllMain(hInst, reason, reserved)
HINSTANCE hInst;                /* Library instance handle. */
DWORD reason;                   /* Reason this function is being called. */
LPVOID reserved;                /* Not used. */
{
    if (reason == DLL_PROCESS_ATTACH) {
        init_soar();

        soar_cAddGlobalCallback(GLB_AGENT_CREATED, cb_tclSoar_NewAgent, NIL, NIL, "new-agent");
    }
    return TRUE;
}
#endif                          /* WIN32 */

/*
 *----------------------------------------------------------------------
 *
 * Soar_UseIPC --
 *
 *	This procedure is called to determine if IPCs are 
 *	to be used for Tk interpreters.  Determines whether they
 *      are registered with the X server.
 *
 * Results:
 *	0 if IPCs disabled (default), 1 if enabled.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int Soar_UseIPC(void)
{
    /* fprintf(stderr, "in Soar_UseIPC, value = %d\n",options.ipc_enabled); */
    return options.ipc_enabled;
}
