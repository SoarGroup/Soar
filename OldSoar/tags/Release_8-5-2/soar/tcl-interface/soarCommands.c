/*
 * =======================================================================
 *  File:  soarCommands.c
 *
 * This file includes the definitions of the Soar Command set.
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
/*----------------------------------------------------------------------
 *
 * PLEASE NOTE!  Only functions implementing commands should appear
 * in this file.  All supporting functions should be placed in 
 * soarCommandUtils.c.
 *
 *----------------------------------------------------------------------
 */

#include "soar.h"
#include "soarCommands.h"
#include "soarCommandUtils.h"
#include "soarapi.h"
#include "soar_core_api.h"

#if defined(WIN32)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define popen(command, mode) _popen((command), (mode))
#define pclose(stream) _pclose(stream)
#endif

#define USE_CAPTURE_REPLAY

extern Tcl_Interp *tcl_soar_agent_interpreters[MAX_SIMULTANEOUS_AGENTS];
extern remove_rhs_function(Symbol * name);

#ifdef DC_HISTOGRAM
int initDCHistogramCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    soar_cInitializeDCHistogram(450, 1);
    return TCL_OK;
}
#endif

#ifdef KT_HISTOGRAM
int initKTHistogramCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    soar_cInitializeKTHistogram(450);
    return TCL_OK;
}
#endif

int AddWmeCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_AddWme(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int AttributePreferencesModeCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_AttributePreferencesMode(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        printf("DONE\n");
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }

}

int ChunkNameFormatCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_ChunkNameFormat(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int DefWmeDepthCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_DefaultWmeDepth(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int DestroyAgentCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    int agent_id;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    agent_id = soar_agent->id;
    /*printf( "Calling Destroy Agent on Agent %d\n", agent_id ); */

    remove_rhs_function(make_sym_constant("tcl"));

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_DestroyAgent(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        tcl_soar_agent_interpreters[agent_id] = NIL;
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int ExciseCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Excise(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int ExplainBacktracesCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_ExplainBacktraces(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;

    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int FiringCountsCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_FiringCounts(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        /*Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));*/
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int FormatWatchCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_FormatWatch(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int GDS_PrintCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soar_ecGDSPrint();
    return TCL_OK;
}

int IndifferentSelectionCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_IndifferentSelection(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int InitSoarCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const argv[])
{
    Soar_SelectGlobalInterpByInterp(interp);

    soar_cReInitSoar();

    return TCL_OK;
}

int InputPeriodCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_InputPeriod(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int InternalSymbolsCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_InternalSymbols(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int LearnCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Learn(objc, argv, &res);
    free_argv(objc, argv);

    if (ret != SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
    return TCL_OK;
}

int MatchesCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Matches(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {

        /* 
           We can't call this next line since the interp's result object has already been built up (via
           soar_Matches calling Soar_AppendResult many times).  If we called this now, we would overwrite
           the output.  For now I'm just commenting it out (since it seems that the soar result is always
           empty when the function returns SOAR_OK), but if some useful information does ever get output
           here, then it should be appended to the existing info, not set to overwrite it

         */
        /*Tcl_SetObjResult( interp, Tcl_NewStringObj( res.result, -1 ) ); */

        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int MaxChunksCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_MaxChunks(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int MaxElaborationsCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_MaxElaborations(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int MemoriesCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Memories(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int MultiAttrCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_MultiAttributes(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int NumericIndifferentCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_NumericIndifferentMode(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int OSupportModeCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_OSupportMode(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int Operand2Cmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Operand2(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int ProductionFindCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_ProductionFind(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int PreferencesCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Preferences(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        /*Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));*/
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int PrintCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Print(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int PwatchCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_PWatch(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int SoarExcludedBuildInfoCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    soar_ExcludedBuildInfo(objc, argv, &res);
    free_argv(objc, argv);

    return TCL_OK;
}

int SoarBuildInfoCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    soar_BuildInfo(objc, argv, &res);
    free_argv(objc, argv);

    return TCL_OK;
}

#ifdef USE_DEBUG_UTILS

int PrintPoolCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Pool(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

#endif

int QuitCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    static char cmd[] = "exit";
    soarResult res;

    init_soarResult(res);
    soar_cQuit();

    (void) Tcl_EvalEx(interp, cmd, -1, 0);
    return TCL_OK;              /* Unreachable, but here to placate the compiler */
}

int RemoveWmeCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_RemoveWme(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int RunCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Run(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int SpCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Sp(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int StatsCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{

    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Stats(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        /*Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));*/
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int StopSoarCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Stop(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int VerboseCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);

    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Verbose(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

/*
#ifdef MACINTOSH
int LogCmd(ClientData clientData,
	   Tcl_Interp * interp,
	   int objc, Tcl_Obj* const objv[] ) 
{
  soarResult res;
  char ** argv;
  int ret;

  init_soarResult(res);

  Soar_SelectGlobalInterpByInterp(interp);

  create_argv_from_objv(objc, objv, &argv);
  ret = soar_Log( objc, argv, &res );
  free_argv(objc, argv);

  if ( ret == SOAR_OK ) {
    Tcl_SetObjResult( interp, Tcl_NewStringObj( res.result, -1 ) );
    return TCL_OK;
  } else {
    Tcl_SetObjResult( interp, Tcl_NewStringObj( res.result, -1 ) );
    return TCL_ERROR;
  }
}

#else 
*/
int LogCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **newArgv;
    bool tildeOccurs, tildeFlag;
    int i, result;
    const char *c;
    Tcl_DString buffer;
    char **argv;

    /* Make sure to free this argv before returning!
     * Currently freed under call to soar_Log
     */
    create_argv_from_objv(objc, objv, &argv);

    /* switch to the proper agent */
    Soar_SelectGlobalInterpByInterp(interp);

    /*
       Before we pass these arguments to Soar, we need to determine
       whether Tilde substitution is necessary.
       we'll look through all arguments to and do any tilde substitution
       necessary although in reality it should only occur when the -new or
       -existing flags are being used.
     */

    init_soarResult(res);

    tildeOccurs = FALSE;
    for (i = 1; i < objc && !tildeOccurs; i++) {
        c = argv[i];
        while (*c) {
            if (*c++ == '~') {
                tildeOccurs = TRUE;
                break;
            }
        }
    }
    /* If the tilde occurs, we need a new Argv array. */
    if (tildeOccurs) {

        newArgv = malloc(objc * sizeof(char *));
        for (i = 0; i < objc; i++) {
            c = argv[i];
            tildeFlag = FALSE;
            while (*c) {
                if (*c++ == '~') {
                    tildeFlag = TRUE;
                    break;
                }
            }
            if (tildeFlag) {
                c = Tcl_TildeSubst(interp, argv[i], &buffer);
                newArgv[i] = malloc((strlen(c) + 1) * sizeof(char));
                strcpy(newArgv[i], c);  /* this is a safe copy since the correct amount of memory is allocated on the previous line */
                printf("Substituting '%s' for '%s'\n", argv[i], c);
            } else {
                newArgv[i] = malloc((strlen(argv[i]) + 1) * sizeof(char));
                strcpy(newArgv[i], argv[i]);    /* this is a safe copy since the correct amount of memory is allocated on the previous line */
            }
        }
    }

    if (tildeOccurs) {
        printf("Substitution occured\n");
        result = soar_Log(objc, newArgv, &res);
        for (i = 0; i < objc; i++)
            free(newArgv[i]);
        free(newArgv);
        Tcl_DStringFree(&buffer);
    } else {
        result = soar_Log(objc, argv, &res);
    }

    free_argv(objc, argv);

    Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));

    if (result == SOAR_OK)
        return TCL_OK;

    return TCL_ERROR;

}

/* from MACTINTOSH ifdef above
#endif
*/

int WaitSNCCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_WaitSNC(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int WarningsCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    char **argv;
    int ret;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
    ret = soar_Warnings(objc, argv, &res);
    free_argv(objc, argv);

    if (ret == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        return TCL_ERROR;
    }
}

int WatchCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    int i;
    char **newArgv;
    int newArgc;
    int result;
    const char *a;
    char **argv;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);

    for (i = 1; i < objc; i++) {

        if (string_match("aliases", argv[i])) {
            if (argv[i + 1] == NULL) {
                Tcl_SetObjResult(interp, Tcl_NewStringObj("Missing setting for watch alias, should be -on|-off", -1));
                free_argv(objc, argv);
                return TCL_ERROR;
            } else if (string_match("-on", argv[i + 1])) {
                Tcl_SetVar(interp, "print_alias_switch", "on", TCL_GLOBAL_ONLY);
                argv[i] = '\0';
                argv[i + 1] = '\0';
                break;
            } else if (string_match("-off", argv[i + 1])) {
                Tcl_SetVar(interp, "print_alias_switch", "off", TCL_GLOBAL_ONLY);
                argv[i] = '\0';
                argv[i + 1] = '\0';
                break;
            } else {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Unrecognized argument to watch alias : ", argv[i + 1],
                                       (char *) NULL);
                free_argv(objc, argv);
                return TCL_ERROR;
            }
        }
    }

    /*
       Since we've got here, we handled all the interface dependendent stuff,
       thus we need to remove the already acted upon arguments, and pass the
       rest to Soar 
     */

	/* Count remaining arguments: */
	newArgc = 0;
	for (i = 0; i < objc; i++) {
		if (argv[i] != NULL) {
			newArgc++;
		}
	}

	/* Did we do everything? */
    if (newArgc == 1 && objc != 1) {
        /* Then we did all that was necessary, so return */
        free_argv(objc, argv);
        return TCL_OK;
    }

	/* Create new argv */
    newArgv = (char **) malloc((newArgc + 1) * sizeof(char *));
    for (i = 0; i < newArgc; i++) {
		if (argv[i] != NULL) {
			newArgv[i] = (char *) malloc((strlen(argv[i]) + 1) * sizeof(char));
            strcpy(newArgv[i], argv[i]);        /* this is a safe copy since the correct amount of memory is allocated on the previous line */
			newArgv[i][strlen(argv[i])] = '\0';
		}
    }
	newArgv[i] = NULL;

	/* Hand new argv over to kernel */
    result = soar_Watch(newArgc, newArgv, &res);

    /* In the case of newArgc == 1, we are printing watch settings.
       The kernel does not know about the print_alias_switch, so
       we need to do that here
     */
    if (newArgc == 1) {
        print("  Alias printing: %s\n", (a = Tcl_GetVar(interp, "print_alias_switch", 0)) ? a : "(null)");
    }

    /* Free the new argv */
    for (i = 0; i < newArgc; i++) {
        free(newArgv[i]);
    }
    free(newArgv);

	/* Free the original argv */
    free_argv(objc, argv);

    if (result == SOAR_OK)
        return TCL_OK;

    Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
    return TCL_ERROR;

}

/*
 *----------------------------------------------------------------------
 *
 * EchoCmd --
 *
 *      This is the command procedure for the "echo" command, which 
 *      prints text to the currently specified output-strings-destination.
 *      IF logging is enabled, the text is also printed to the log
 *      destination.
 *
 * Syntax:  echo strings
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Prints the given strings according to the currently specified
 *      output-strings-destination
 *
 *----------------------------------------------------------------------
 */

int EchoCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    int i;
    bool newline = TRUE;

    Soar_SelectGlobalInterpByInterp(interp);

    for (i = 1; i < objc; i++) {
        if (string_match_up_to("-nonewline", Tcl_GetStringFromObj(objv[i], NULL), 2)) {
            newline = FALSE;
        } else {
            Soar_LogAndPrint((agent *) clientData, Tcl_GetStringFromObj(objv[i], NULL));
            if ((i + 1) < objc) {
                Soar_LogAndPrint((agent *) clientData, " ");
            }
        }
    }

    if (newline) {
        Soar_LogAndPrint((agent *) clientData, "\n");
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * AskCmd --
 *
 *      This is the command procedure for the "ask" command, 
 *      which manages the attachment of scripts to handle Soar I/O.
 *
 * Syntax:  ask [-add <ask-proc>] [-remove]
 *
 * Results:
 *      Returns a standard Tcl completion code. 
 *
 * Side effects:
 *      Adds and/or removes indicated ask procedure from the system.
 *      This command may also print information about defined ask
 *      procedures.  Returns the name of the new ask procedure
 *      if one is created.
 *
 *----------------------------------------------------------------------
 */

int AskCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    static char *too_few_args_string = "Too few arguments, should be: ask [-add <proc>] | [-remove]";
    static char *too_many_args_string = "Too many arguments, should be: ask [-add <proc>] | [-remove]";

    Soar_SelectGlobalInterpByInterp(interp);

    if (objc == 1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
        return TCL_ERROR;
    }

    if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-add", 2)) {
        if (objc < 3) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
            return TCL_ERROR;
        }

        if (objc > 3) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_many_args_string, -1));
            return TCL_ERROR;
        }

        if (objc == 3) {
            soar_cPushCallback((agent *) clientData, ASK_CALLBACK,
                               soar_ask_callback_to_tcl,
                               (soar_callback_data) savestring(Tcl_GetStringFromObj(objv[2], NULL)),
                               soar_callback_data_free_string);
        }
    } else if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-remove", 2)) {
        soar_cRemoveAllCallbacksForEvent((agent *) clientData, ASK_CALLBACK);
    }

    soar_cPushCallback((soar_callback_agent) clientData,
                       PRINT_CALLBACK,
                       (soar_callback_fn) Soar_AppendResult, (soar_callback_data) NULL, (soar_callback_free_fn) NULL);

    /* if a log file is open, then we need to push a dummy callback
     * so that we don't get extraneous prints mucking up the log file.
     * Addresses bug # 248.  KJC 01/00 
     */

    if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {
        soar_cPushCallback((soar_callback_agent) clientData,
                           LOG_CALLBACK,
                           (soar_callback_fn) Soar_DiscardPrint,
                           (soar_callback_data) NULL, (soar_callback_free_fn) NULL);
    }

    soar_cListAllCallbacksForEvent((agent *) clientData, ASK_CALLBACK);
    soar_cPopCallback((soar_callback_agent) clientData, PRINT_CALLBACK);

    if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {
        soar_cPopCallback((soar_callback_agent) clientData, LOG_CALLBACK);
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * IOCmd --
 *
 *      This is the command procedure for the "io" command, 
 *      which manages the attachment of scripts to handle Soar I/O.
 *
 * Syntax:  io <add-specification> | 
 *             <removal-specification> |
 *             <list-specification> |
 *
 *          <add-specification>     := -add -input  script [id] |
 *                                     -add -output script  id
 *          <removal-specification> := -delete [-input | -output] id
 *          <list-specification>    := -list [-input | -output]
 *
 * Results:
 *      Returns a standard Tcl completion code. 
 *
 * Side effects:
 *      Adds and/or removes indicated I/O procedure from the system.
 *      This command may also print information about defined I/O
 *      procedures.  Returns the name of the new I/O procedure
 *      if one is created.
 *
 *----------------------------------------------------------------------
 */

#define SOAR_COMMANDS_BUFFER_SIZE 10    /* What size is good here? */

static int io_proc_counter = 1;

int IOCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    static char *too_few_args_string =
        "Too few arguments, should be: io [-add -input script [id]] | [-add -output script id] | [-delete [-input|-output] id] | [-list [-input|-output]";
    static char *too_many_args_string =
        "Too many arguments, should be: io [-add -input script [id]] | [-add -output script id] | [-delete [-input|-output] id] | [-list [-input|-output]";
    const char *io_id;
    char buff[SOAR_COMMANDS_BUFFER_SIZE];

    Soar_SelectGlobalInterpByInterp(interp);

    if (objc == 1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
        return TCL_ERROR;
    }
    if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-add", 2)) {
        if (objc < 4) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
            return TCL_ERROR;
        }

        if (objc > 5) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_many_args_string, -1));
            return TCL_ERROR;
        }

        if (objc == 5) {
            io_id = Tcl_GetStringFromObj(objv[4], NULL);
        } else {
            snprintf(buff, SOAR_COMMANDS_BUFFER_SIZE, "m%d", io_proc_counter++);
            buff[SOAR_COMMANDS_BUFFER_SIZE - 1] = 0;    /* snprintf doesn't set last char to null if output is truncated */
            io_id = buff;
        }

        {
            if (string_match_up_to(Tcl_GetStringFromObj(objv[2], NULL), "-input", 2)) {
                soar_cAddInputFunction((agent *) clientData,
                                       soar_input_callback_to_tcl,
                                       (soar_callback_data) savestring(Tcl_GetStringFromObj(objv[3], NULL)),
                                       soar_callback_data_free_string, (soar_callback_id) io_id);
            } else if (string_match_up_to(Tcl_GetStringFromObj(objv[2], NULL), "-output", 2)) {
                /* Soar-Bugs #131, id required for output - TMH */
                if (objc < 5) {
                    Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
                    return TCL_ERROR;
                }

                soar_cAddOutputFunction((agent *) clientData,
                                        soar_output_callback_to_tcl,
                                        (soar_callback_data) savestring(Tcl_GetStringFromObj(objv[3], NULL)),
                                        soar_callback_data_free_string, (soar_callback_id) io_id);
            } else {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), Tcl_GetStringFromObj(objv[0], NULL),
                                       ": Unrecognized IO type: ", Tcl_GetStringFromObj(objv[1], NULL), " ",
                                       Tcl_GetStringFromObj(objv[2], NULL), (char *) NULL);
                return TCL_ERROR;
            }

            Tcl_SetObjResult(interp, Tcl_NewStringObj((char *) io_id, -1));
            return TCL_OK;
        }
    } else if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-delete", 2)) {
        switch (objc) {
        case 2:
        case 3:
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
            return TCL_ERROR;
        case 4:                /* Delete single callback for given event */
            {
                if (string_match_up_to(Tcl_GetStringFromObj(objv[2], NULL), "-input", 2)) {
                    soar_cRemoveInputFunction((agent *) clientData, Tcl_GetStringFromObj(objv[3], NULL));
                } else if (string_match_up_to(Tcl_GetStringFromObj(objv[2], NULL), "-output", 2)) {
                    soar_cRemoveOutputFunction((agent *) clientData, Tcl_GetStringFromObj(objv[3], NULL));
                } else {
                    Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Attempt to delete unrecognized io type: ",
                                           Tcl_GetStringFromObj(objv[2], NULL), (char *) NULL);
                    return TCL_ERROR;
                }
            }
            break;
        default:
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_many_args_string, -1));
            return TCL_ERROR;
        }
    } else if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-list", 2)) {
        switch (objc) {
        case 2:
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
            return TCL_ERROR;
        case 3:
            {
                SOAR_CALLBACK_TYPE ct;

                if (string_match_up_to(Tcl_GetStringFromObj(objv[2], NULL), "-input", 2)) {
                    ct = INPUT_PHASE_CALLBACK;
                } else if (string_match_up_to(Tcl_GetStringFromObj(objv[2], NULL), "-output", 2)) {
                    ct = OUTPUT_PHASE_CALLBACK;
                } else {
                    Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Attempt to list unrecognized io type: ",
                                           Tcl_GetStringFromObj(objv[2], NULL), (char *) NULL);
                    return TCL_ERROR;
                }

                soar_cPushCallback((soar_callback_agent) clientData,
                                   PRINT_CALLBACK,
                                   (soar_callback_fn) Soar_AppendResult,
                                   (soar_callback_data) NULL, (soar_callback_free_fn) NULL);
                /* if a log file is open, then we need to push a dummy callback
                 * so that we don't get extraneous prints mucking up the log file.
                 * Addresses bug # 248.  KJC 01/00 
                 */
                if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {

                    soar_cPushCallback((soar_callback_agent) clientData,
                                       LOG_CALLBACK,
                                       (soar_callback_fn) Soar_DiscardPrint,
                                       (soar_callback_data) NULL, (soar_callback_free_fn) NULL);
                }

                soar_cListAllCallbacksForEvent((agent *) clientData, ct);
                soar_cPopCallback((soar_callback_agent) clientData, PRINT_CALLBACK);
                if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {
                    soar_cPopCallback((soar_callback_agent) clientData, LOG_CALLBACK);
                }
            }
            break;
        default:
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_many_args_string, -1));
            return TCL_ERROR;
        }

    } else {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Unrecognized option to io command: ",
                               Tcl_GetStringFromObj(objv[1], NULL), (char *) NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

#ifdef ATTENTION_LAPSE
/* RMJ */

/*
 *----------------------------------------------------------------------
 *
 * AttentionLapseCmd --
 *
 *      This is the command procedure for the "attention-lapse" command.
 *      With no arguments, this command prints out the current attentional 
 *      lapsing status.  Any of the following arguments may be given:
 *
 *        on         - turns attentional lapsing on 
 *        off        - turns attentional lapsing off 
 *
 * See also: wake-from-attention-lapse, start-attention-lapse
 *
 * Syntax:  attention-lapse arg*
 *            arg  ::=  -on | -off 
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets boolean for whether or not attentional lapsing will occur.
 *
 *----------------------------------------------------------------------
 */

int AttentionLapseCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{

    Soar_SelectGlobalInterpByInterp(interp);

    if (objc == 1) {
        print_current_attention_lapse_settings();
        return TCL_OK;
    }

    int i;

    for (i = 1; i < objc; i++) {
        if (string_match("-on", Tcl_GetStringFromObj(objv[i], NULL))) {
            set_sysparam(ATTENTION_LAPSE_ON_SYSPARAM, TRUE);
            wake_from_attention_lapse();
        } else if (string_match_up_to("-off", Tcl_GetStringFromObj(objv[i], NULL), 3)) {
            set_sysparam(ATTENTION_LAPSE_ON_SYSPARAM, FALSE);
        } else {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Unrecognized argument to attention-lapse command: ",
                                   Tcl_GetStringFromObj(objv[1], NULL), (char *) NULL);
            return TCL_ERROR;
        }
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * WakeFromAttentionLapseCmd --
 *
 *      This is the command procedure for the "wake-from-attention-lapse"
 *      command, which is primarily intended to be called from the RHS
 *      of a production rule.
 *      This sets the "attention-lapsing" variable to FALSE (0), and
 *      starts tracking the amount of real time that has passed since
 *      the last lapse.
 *
 * See also: attention-lapse, start-attention-lapse
 *
 * Syntax:  wake-from-attention-lapse
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets boolean "attention-lapsing" variable to 0; resets
 *      "attention_lapse_tracker" to current real time of day.
 *
 *----------------------------------------------------------------------
 */

int WakeFromAttentionLapseCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    Soar_SelectGlobalInterpByInterp(interp);

    if (objc == 1) {
        wake_from_attention_lapse();
        return TCL_OK;

    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Too many arguments, should be: wake-from-attention-lapse", -1));
        return TCL_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * StartAttentionLapseCmd --
 *
 *      This is the command procedure for the "start-attention-lapse"
 *      command, which should not normally be called by the user or
 *      an agent (attention lapses normally get started automatically
 *      by the architecture).
 *      This sets the "attention-lapsing" variable to TRUE (1), and
 *      starts tracking the amount of real time that should pass before
 *      ending the lapse (with wake_from_attention_lapse).  The duration
 *      of the lapse is the number of milleseconds specified by the
 *      argument to this command (in real time).
 *
 * See also: attention-lapse, wake-from-attention-lapse
 *
 * Syntax:  start-attention-lapse integer
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets boolean "attention-lapsing" variable to 1; resets
 *      "attention_lapse_tracker" to current real time of day plus
 *      the number of milleseconds specified by the integer argument.
 *
 *----------------------------------------------------------------------
 */

int StartAttentionLapseCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    int duration;

    Soar_SelectGlobalInterpByInterp(interp);

    if (objc < 2) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Too few arguments, should be: start-attention-lapse integer", -1));
        return TCL_ERROR;
    } else if (objc > 2) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Too many arguments, should be: start-attention-lapse integer", -1));
        return TCL_ERROR;
    }

    if (Tcl_GetInt(interp, Tcl_GetStringFromObject(objv[1], NULL), &duration) == TCL_OK) {
        start_attention_lapse((long) duration);
    } else {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Expected integer for attention lapse duration: ",
                               Tcl_GetStringFromObject(objv[1], NULL), (char *) NULL);

        return TCL_ERROR;
    }

    return TCL_OK;
}

#endif                          /* ATTENTION_LAPSE */

/*
 *----------------------------------------------------------------------
 *
 * MonitorCmd --
 *
 *      This is the command procedure for the "monitor" command, 
 *      which manages the attachment of scripts to Soar events.
 *
 * Syntax:  monitor <add-specification> | 
 *                  <removal-specification> |
 *                  <list-specification> |
 *                  -test |
 *                  -clear
 *
 *          <add-specification>     := -add soar-event script [id]
 *          <removal-specification> := -delete soar-event [id]
 *          <list-specification>    := -list [soar-event]
 *
 * Results:
 *      Returns a standard Tcl completion code. 
 *
 * Side effects:
 *      Adds and/or removes indicated monitor from the system.
 *      This command may also print information about defined
 *      monitors.  Returns the name of the new monitor attachments
 *      if one is created.
 *
 *----------------------------------------------------------------------
 */

static int monitor_counter = 1;

int MonitorCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    static char *too_few_args_string =
        "Too few arguments, should be: monitor [-add event script [id]] | [-delete event [id]] | [-list [event] | clear]";
    static char *too_many_args_string =
        "Too many arguments, should be: monitor [-add event script [id]] | [-delete event [id]] | [-list [event] | -clear]";
    const char *monitor_id;
    char buff[SOAR_COMMANDS_BUFFER_SIZE];       /* What size is good here? */

    Soar_SelectGlobalInterpByInterp(interp);

    if (objc == 1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
        return TCL_ERROR;
    }

    if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-add", 2)) {
        if (objc < 4) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
            return TCL_ERROR;
        }

        if (objc > 5) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_many_args_string, -1));
            return TCL_ERROR;
        }

        if (objc == 5) {
            monitor_id = Tcl_GetStringFromObj(objv[4], NULL);
        } else {
            snprintf(buff, SOAR_COMMANDS_BUFFER_SIZE, "m%d", monitor_counter++);
            buff[SOAR_COMMANDS_BUFFER_SIZE - 1] = 0;    /* snprintf doesn't set last char to null if output is truncated */
            monitor_id = buff;
        }

        {
            SOAR_CALLBACK_TYPE ct;

            ct = soar_cCallbackNameToEnum(Tcl_GetStringFromObj(objv[2], NULL), TRUE);
            if (ct) {
                soar_cAddCallback((agent *) clientData,
                                  ct,
                                  soar_callback_to_tcl,
                                  (soar_callback_data) savestring(Tcl_GetStringFromObj(objv[3], NULL)),
                                  soar_callback_data_free_string, (soar_callback_id) monitor_id);
                Tcl_SetObjResult(interp, Tcl_NewStringObj((char *) monitor_id, -1));
                return TCL_OK;
            } else {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Attempt to add unrecognized callback event: ",
                                       Tcl_GetStringFromObj(objv[2], NULL), (char *) NULL);
                return TCL_ERROR;
            }
        }
    } else if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-delete", 2)) {
        switch (objc) {
        case 2:
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args_string, -1));
            return TCL_ERROR;
        case 3:                /* Delete all callbacks of the given type */
            {
                SOAR_CALLBACK_TYPE ct;

                ct = soar_cCallbackNameToEnum(Tcl_GetStringFromObj(objv[2], NULL), TRUE);
                if (ct) {
                    soar_cRemoveAllCallbacksForEvent((agent *) clientData, ct);
                } else {
                    Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Attempt to delete unrecognized callback event: ",
                                           Tcl_GetStringFromObj(objv[2], NULL), (char *) NULL);
                    return TCL_ERROR;
                }
            }
            break;
        case 4:                /* Delete single callback for given event */
            {
                SOAR_CALLBACK_TYPE ct;

                ct = soar_cCallbackNameToEnum(Tcl_GetStringFromObj(objv[2], NULL), TRUE);
                if (ct) {
                    soar_cRemoveCallback((agent *) clientData, ct, Tcl_GetStringFromObj(objv[3], NULL));
                } else {
                    Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Attempt to delete unrecognized callback event: ",
                                           Tcl_GetStringFromObj(objv[2], NULL), (char *) NULL);
                    return TCL_ERROR;
                }
            }
            break;
        default:
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_many_args_string, -1));
            return TCL_ERROR;
        }
    } else if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-list", 2)) {
        switch (objc) {
        case 2:
            soar_cListAllCallbacks((agent *) clientData, TRUE);
            break;
        case 3:
            {
                SOAR_CALLBACK_TYPE ct;

                ct = soar_cCallbackNameToEnum(Tcl_GetStringFromObj(objv[2], NULL), TRUE);
                if (ct) {
                    soar_cPushCallback((soar_callback_agent) clientData,
                                       PRINT_CALLBACK,
                                       (soar_callback_fn) Soar_AppendResult,
                                       (soar_callback_data) NULL, (soar_callback_free_fn) NULL);

                    /* if a log file is open, then we need to push a dummy callback
                     * so that we don't get extraneous prints mucking up the log 
                     * file. Addresses bug # 248.  KJC 01/00 
                     */
                    if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {

                        soar_cPushCallback((soar_callback_agent) clientData,
                                           LOG_CALLBACK,
                                           (soar_callback_fn) Soar_DiscardPrint,
                                           (soar_callback_data) NULL, (soar_callback_free_fn) NULL);
                    }

                    soar_cListAllCallbacksForEvent((agent *) clientData, ct);
                    soar_cPopCallback((soar_callback_agent) clientData, PRINT_CALLBACK);
                    if (soar_exists_callback((soar_callback_agent) clientData, LOG_CALLBACK)) {

                        soar_cPopCallback((soar_callback_agent) clientData, LOG_CALLBACK);

                    }

                } else {
                    Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Attempt to list unrecognized callback event: ",
                                           Tcl_GetStringFromObj(objv[2], NULL), (char *) NULL);
                    return TCL_ERROR;
                }
            }
            break;
        default:
            Tcl_SetObjResult(interp, Tcl_NewStringObj(too_many_args_string, -1));
            return TCL_ERROR;
        }

    } else if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-test", 2)) {
        soar_cTestAllMonitorableCallbacks((agent *) clientData);
    } else if (string_match_up_to(Tcl_GetStringFromObj(objv[1], NULL), "-clear", 2)) {
        /* Delete all callbacks of all types */
        soar_cRemoveAllMonitorableCallbacks((agent *) clientData);
    } else {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Unrecognized option to monitor command: ",
                               Tcl_GetStringFromObj(objv[1], NULL), (char *) NULL);
        return TCL_ERROR;

    }
    return TCL_OK;
}

/* REW: end   09.15.96 */

/*
 *----------------------------------------------------------------------
 *
 * OutputStringsDestCmd --
 *
 *      This is the command procedure for the "output-strings-destination"
 *      command which redirects strings printed by Soar_PrintCmd to the
 *      selected destination.
 *
 *      If output-strings-destination is set to -append-to-result and 
 *      the C code performs an assignment to interp->result then           <--- voigtjr, deprecated!
 *      the intermediate results will be lost (memory leak?).
 *
 * Syntax:  output-strings-destination [-push [ [-text-widget widget-name 
 *                                                           [interp-name]]
 *                                   | [-channel channel-id]
 * RMJ 7-1-97 *                      | [-procedure procedure-name]
 *                                   | -discard 
 *                                   | -append-to-result 
 *                                   ]
 *                            | -pop ]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Changes the destination of Soar_Print commands.
 *
 *----------------------------------------------------------------------
 */
int OutputStringsDestCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    static char *too_few_args =
        "Too few arguments, should be: output-strings-destination [ -push [[-text-widget widget-name [interp-name]] | [-channel channel-id] | [-procedure tcl-procedure-name] | -discard |-append-to-result] | -pop]";

    Soar_SelectGlobalInterpByInterp(interp);

    if (objc == 1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args, -1));
        return TCL_ERROR;
    }

    if (string_match("-push", Tcl_GetStringFromObj(objv[1], NULL))) {
        if (string_match("-text-widget", Tcl_GetStringFromObj(objv[2], NULL))) {
            if (objc == 3) {
                Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args, -1));
                return TCL_ERROR;
            } else {
                /* We assume that we'll be printing to the same interp. */
                Tcl_Interp *print_interp = interp;
                Soar_TextWidgetPrintData *data;

                if (objc > 4) {
                    /* Too many arguments */
                    Tcl_SetObjResult(interp, Tcl_NewStringObj("Too many arguments", -1));
                    return TCL_ERROR;
                }

                data = Soar_MakeTextWidgetPrintData(print_interp, Tcl_GetStringFromObj(objv[3], NULL));
                soar_cPushCallback((soar_callback_agent) clientData,
                                   PRINT_CALLBACK,
                                   (soar_callback_fn) Soar_PrintToTextWidget,
                                   (soar_callback_data) data, (soar_callback_free_fn) Soar_FreeTextWidgetPrintData);
            }
        }
/* RMJ 7-1-97 */
        else if (string_match("-procedure", Tcl_GetStringFromObj(objv[2], NULL))) {
            if (objc == 3) {
                Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args, -1));
                return TCL_ERROR;
            } else {
                /* We assume that we'll be printing to the same interp. */
                Tcl_Interp *print_interp = interp;
                Soar_TextWidgetPrintData *data;

                if (objc > 4) {
                    /* Too many arguments */
                    Tcl_SetObjResult(interp, Tcl_NewStringObj("Too many arguments", -1));
                    return TCL_ERROR;
                }

                data = Soar_MakeTextWidgetPrintData(print_interp, Tcl_GetStringFromObj(objv[3], NULL));
                soar_cPushCallback((soar_callback_agent) clientData,
                                   PRINT_CALLBACK,
                                   (soar_callback_fn) Soar_PrintToTclProc,
                                   (soar_callback_data) data, (soar_callback_free_fn) Soar_FreeTextWidgetPrintData);
            }
        } else if (string_match("-channel", Tcl_GetStringFromObj(objv[2], NULL))) {
            Tcl_Channel channel;
            int mode;

            if (objc == 3) {
                Tcl_SetObjResult(interp, Tcl_NewStringObj(too_few_args, -1));
                return TCL_ERROR;
            }

            if ((channel = Tcl_GetChannel(interp, Tcl_GetStringFromObj(objv[3], NULL), &mode)) == NULL
                || !(mode & TCL_WRITABLE)) {
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), Tcl_GetStringFromObj(objv[3], NULL),
                                       " is not a valid channel for writing.", (char *) NULL);
                return TCL_ERROR;
            }

            soar_cPushCallback((soar_callback_agent) clientData,
                               PRINT_CALLBACK,
                               (soar_callback_fn) Soar_PrintToChannel,
                               (soar_callback_data) channel, (soar_callback_free_fn) NULL);

        } else if (string_match("-discard", Tcl_GetStringFromObj(objv[2], NULL))) {
            soar_cPushCallback((soar_callback_agent) clientData,
                               PRINT_CALLBACK,
                               (soar_callback_fn) Soar_DiscardPrint,
                               (soar_callback_data) NULL, (soar_callback_free_fn) NULL);
        } else if (string_match("-append-to-result", Tcl_GetStringFromObj(objv[2], NULL))) {
            soar_cPushCallback((soar_callback_agent) clientData,
                               PRINT_CALLBACK,
                               (soar_callback_fn) Soar_AppendResult,
                               (soar_callback_data) NULL, (soar_callback_free_fn) NULL);
        } else {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Unrecognized argument to ",
                                   Tcl_GetStringFromObj(objv[0], NULL), " ", Tcl_GetStringFromObj(objv[1], NULL), ": ",
                                   Tcl_GetStringFromObj(objv[2], NULL), (char *) NULL);
            return TCL_ERROR;
        }
    } else if (string_match("-pop", Tcl_GetStringFromObj(objv[1], NULL))) {
        soar_cPopCallback((soar_callback_agent) clientData, PRINT_CALLBACK);
    } else {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "Unrecognized argument to ",
                               Tcl_GetStringFromObj(objv[0], NULL), ": ", Tcl_GetStringFromObj(objv[1], NULL),
                               (char *) NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

#ifdef USE_CAPTURE_REPLAY

/*
 *----------------------------------------------------------------------
 *
 * CaptureInputCmd --
 *
 *      This is the command procedure for the "capture-input" command
 *      which records input wme commands (add|remove) from the INPUT phase.
 *
 *      This command may be used to start and stop the recording of
 *      input wmes as created by an external simulation.  wmes are
 *      recorded decision cycle by decision cycle.  Use the command
 *      replay-input to replay the sequence.
 *
 * Syntax:  capture-input <action>
 *          <action> ::= -open pathname 
 *          <action> ::= -query
 *          <action> ::= -close
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Opens and/or closes captured input files.  
 *
 *----------------------------------------------------------------------
 */

int CaptureInputCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
    soarResult res;
    const char **new_argv;
    int i;
    char *buffer;
    Tcl_DString temp;
    char **argv;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);
/*
#ifndef MACINTOSH
*/
    if (objc > 2) {
        /* Then in theory we should be opening a file,
         * and we need to do some tilde substitution
         */

        new_argv = (char **) malloc(objc * sizeof(char *));
        for (i = 0; i < 2; i++) {
            new_argv[i] = (char *) malloc(sizeof(char) * (strlen(argv[i]) + 1));
            strcpy((char *) new_argv[i], argv[i]);      /* this is a safe copy since the correct amount of memory is allocated on the previous line */
        }
        for (i = 2; i < objc; i++) {
            /* Hopefully, there will just be 1 iteration through here.... */
            buffer = Tcl_TildeSubst(interp, argv[i], &temp);
            new_argv[i] = (char *) malloc(sizeof(char) * (strlen(buffer) + 1));
            strcpy((char *) new_argv[i], buffer);       /* this is a safe copy since the correct amount of memory is allocated on the previous line */
        }
    } else {
        new_argv = argv;
    }
/* from MACINTOSH ifndef above
#else
  new_argv = argv;

#endif
*/

    if (soar_CaptureInput(objc, new_argv, &res) == SOAR_OK) {
        /*interp->result = res.result; voigtjr, deprecated */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        free_argv(objc, argv);
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        free_argv(objc, argv);
        return TCL_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ReplayInputCmd --
 *
 *      This is the command procedure for the "replay-input" command
 *      which reads input wme commands (add|remove) from a file.
 *
 *      This command may be used to start and stop the reading of
 *      input wmes from a file created by the "capture-input" command.  
 *      The routine replay-input-wme is registered as an input function
 *      to read input wmes from the file decision cycle by decision cycle.
 *      If an EOF is reached, the file is closed and the callback removed.
 *      Use the command capture-input to create the sequence.
 *
 * Syntax:  replay-input <action>
 *          <action> ::= -open pathname 
 *          <action> ::= -query
 *          <action> ::= -close
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Opens and/or closes captured input files.  
 *
 *----------------------------------------------------------------------
 */

int ReplayInputCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{

    soarResult res;
    const char **new_argv;
    int i;
    char *buffer;
    Tcl_DString temp;
    char **argv;

    init_soarResult(res);
    Soar_SelectGlobalInterpByInterp(interp);

    create_argv_from_objv(objc, objv, &argv);

/*
#ifndef MACINTOSH
*/

    if (objc > 2) {
        /* Then in theory we should be opening a file,
         * and we need to do some tilde substitution
         */

        new_argv = (char **) malloc(objc * sizeof(char *));
        for (i = 0; i < 2; i++) {
            new_argv[i] = (char *) malloc(sizeof(char) * (strlen(argv[i]) + 1));
            strcpy((char *) new_argv[i], argv[i]);      /* this is a safe copy since the correct amount of memory is allocated on the previous line */
        }
        for (i = 2; i < objc; i++) {
            /* Hopefully, there will just be 1 iteration through here.... */
            buffer = Tcl_TildeSubst(interp, argv[i], &temp);
            new_argv[i] = (char *) malloc(sizeof(char) * (strlen(buffer) + 1));
            strcpy((char *) new_argv[i], buffer);       /* this is a safe copy since the correct amount of memory is allocated on the previous line */
        }
    } else {
        new_argv = argv;
    }

/* from MACINTOSH ifndef above
#else
  new_argv = argv;

#endif
*/

    if (soar_ReplayInput(objc, new_argv, &res) == SOAR_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        free_argv(objc, argv);
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        free_argv(objc, argv);
        return TCL_ERROR;
    }
}

#endif                          /* USE_CAPTURE_REPLAY */

/*
 *----------------------------------------------------------------------
 *
 * ReteNetCmd --
 *
 *      This is the command procedure for the "rete-net" command, 
 *      which saves and restores the state of the Rete network.
 *
 * Syntax:  rete-net option filename
 *            option ::= -save | -load
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Loads or saves the Rete network using the file "filename"
 *
 *----------------------------------------------------------------------
 */

int ReteNetCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{

    Tcl_DString buffer;
    char *fullname;
    int (*rete_net_op) (char *);

    Soar_SelectGlobalInterpByInterp(interp);

    if (objc < 3) {
        /*interp->result =  "Too few arguments.\nUsage: rete-net {-save | -load} filename."; voigtjr, deprecated */
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Too few arguments.\nUsage: rete-net {-save | -load} filename.", -1));
        return TCL_ERROR;
    }

    if (objc > 3) {
        /*interp->result = "Too many arguments.\nUsage: rete-net {-save | -load} filename."; voigtjr, deprecated */
        Tcl_SetObjResult(interp,
                         Tcl_NewStringObj("Too many arguments.\nUsage: rete-net {-save | -load} filename.", -1));
        return TCL_ERROR;
    }

    fullname = Tcl_TildeSubst(interp, Tcl_GetStringFromObj(objv[2], NULL), &buffer);

    if (string_match(Tcl_GetStringFromObj(objv[1], NULL), "-save"))
        rete_net_op = soar_cSaveReteNet;
    else if (string_match(Tcl_GetStringFromObj(objv[1], NULL), "-load"))
        rete_net_op = soar_cLoadReteNet;
    else {
        /* interp->result = "Unrecognized argument to ReteNet command: %s. Should be -save|-load"; voigtjr, deprecated */
        Tcl_SetObjResult(interp,
                         Tcl_NewStringObj("Unrecognized argument to ReteNet command: %s. Should be -save|-load", -1));
        return TCL_ERROR;
    }

    switch ((rete_net_op) (fullname)) {
    case SOAR_OK:
        return TCL_OK;
    case SOAR_ERROR:
        return TCL_ERROR;
    default:
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Unrecognized return value from internal RETE function.", -1));
        return TCL_ERROR;
    }
    return TCL_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * InterruptCmd --
 *
 *      This command sets and queries information regarding
 *      interrupts on productions.  It can turn interrupts on single
 *      productions on or off, list the current setting for a
 *      production, or list all productions which currently have
 *      interrupts on or off.
 *
 * Syntax:  interrupt [-on|-off] [production name]
 *
 * Results:
 *      Returns a standard Tcl completion code.
 *
 * Side effects:
 *      Sets the interrupt byte for a production or prints
 *           current interrupt settings
 *
 *----------------------------------------------------------------------
 */

int InterruptCmd(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
{
	char **argv;
	soarResult res;

	init_soarResult(res);
	Soar_SelectGlobalInterpByInterp(interp);
	create_argv_from_objv(objc, objv, &argv);

	if( soar_Interrupt(objc, argv, &res) == SOAR_OK ) {
		free_argv(objc, argv);
		return TCL_OK;
	} else {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(res.result, -1));
        free_argv(objc, argv);
        return TCL_ERROR;
	}

}

void Soar_InstallCommands(agent * the_agent)
{
    install_tcl_soar_cmd(the_agent, "add-wme", AddWmeCmd);
    install_tcl_soar_cmd(the_agent, "ask", AskCmd);

#ifdef ATTENTION_LAPSE          /* RMJ */
    install_tcl_soar_cmd(the_agent, "attention-lapse", AttentionLapseCmd);
    install_tcl_soar_cmd(the_agent, "start-attention-lapse", StartAttentionLapseCmd);
    install_tcl_soar_cmd(the_agent, "wake-from-attention-lapse", WakeFromAttentionLapseCmd);
#endif                          /* ATTENTION_LAPSE */

    install_tcl_soar_cmd(the_agent, "attribute-preferences-mode", AttributePreferencesModeCmd);
    install_tcl_soar_cmd(the_agent, "chunk-name-format", ChunkNameFormatCmd);   /* kjh(CUSP-B14) */
    install_tcl_soar_cmd(the_agent, "default-wme-depth", DefWmeDepthCmd);
    install_tcl_soar_cmd(the_agent, "echo", EchoCmd);
    install_tcl_soar_cmd(the_agent, "excise", ExciseCmd);
    install_tcl_soar_cmd(the_agent, "explain-backtraces", ExplainBacktracesCmd);
    install_tcl_soar_cmd(the_agent, "firing-counts", FiringCountsCmd);
    install_tcl_soar_cmd(the_agent, "format-watch", FormatWatchCmd);
    install_tcl_soar_cmd(the_agent, "indifferent-selection", IndifferentSelectionCmd);
    install_tcl_soar_cmd(the_agent, "init-soar", InitSoarCmd);
    install_tcl_soar_cmd(the_agent, "input-period", InputPeriodCmd);
    install_tcl_soar_cmd(the_agent, "internal-symbols", InternalSymbolsCmd);
	install_tcl_soar_cmd(the_agent, "interrupt", InterruptCmd); /* RPM 11.24.03 */
    install_tcl_soar_cmd(the_agent, "io", IOCmd);
    install_tcl_soar_cmd(the_agent, "learn", LearnCmd);
    install_tcl_soar_cmd(the_agent, "log", LogCmd);
    install_tcl_soar_cmd(the_agent, "matches", MatchesCmd);
    install_tcl_soar_cmd(the_agent, "max-chunks", MaxChunksCmd);
    install_tcl_soar_cmd(the_agent, "max-elaborations", MaxElaborationsCmd);
    install_tcl_soar_cmd(the_agent, "memories", MemoriesCmd);
    install_tcl_soar_cmd(the_agent, "monitor", MonitorCmd);
    install_tcl_soar_cmd(the_agent, "multi-attributes", MultiAttrCmd);
    install_tcl_soar_cmd(the_agent, "numeric-indifferent-mode", NumericIndifferentCmd);
    install_tcl_soar_cmd(the_agent, "o-support-mode", OSupportModeCmd);
    install_tcl_soar_cmd(the_agent, "output-strings-destination", OutputStringsDestCmd);
    install_tcl_soar_cmd(the_agent, "production-find", ProductionFindCmd);
    install_tcl_soar_cmd(the_agent, "preferences", PreferencesCmd);
    install_tcl_soar_cmd(the_agent, "print", PrintCmd);
    install_tcl_soar_cmd(the_agent, "pwatch", PwatchCmd);
    install_tcl_soar_cmd(the_agent, "quit", QuitCmd);
/*  install_tcl_soar_cmd(the_agent, "record",              RecordCmd);  /* kjh(CUSP-B10) */
/*  install_tcl_soar_cmd(the_agent, "replay",              ReplayCmd);  /* kjh(CUSP-B10) */
    install_tcl_soar_cmd(the_agent, "remove-wme", RemoveWmeCmd);
    install_tcl_soar_cmd(the_agent, "rete-net", ReteNetCmd);
    install_tcl_soar_cmd(the_agent, "run", RunCmd);
    install_tcl_soar_cmd(the_agent, "sp", SpCmd);
    install_tcl_soar_cmd(the_agent, "stats", StatsCmd);
    install_tcl_soar_cmd(the_agent, "stop-soar", StopSoarCmd);
    install_tcl_soar_cmd(the_agent, "warnings", WarningsCmd);
    install_tcl_soar_cmd(the_agent, "watch", WatchCmd);
/* REW: begin 09.15.96 */
    install_tcl_soar_cmd(the_agent, "gds_print", GDS_PrintCmd);
    /* REW: 7.1/waterfall:soarAppInit.c  merge */
    install_tcl_soar_cmd(the_agent, "verbose", VerboseCmd);
    install_tcl_soar_cmd(the_agent, "soar8", Operand2Cmd);
    install_tcl_soar_cmd(the_agent, "waitsnc", WaitSNCCmd);
/* REW: end   09.15.96 */

#ifdef USE_DEBUG_UTILS
    install_tcl_soar_cmd(the_agent, "pool", PrintPoolCmd);

#endif
    install_tcl_soar_cmd(the_agent, "build-info", SoarBuildInfoCmd);
    install_tcl_soar_cmd(the_agent, "ex-build-info", SoarExcludedBuildInfoCmd);

#ifdef USE_CAPTURE_REPLAY
    install_tcl_soar_cmd(the_agent, "capture-input", CaptureInputCmd);
    install_tcl_soar_cmd(the_agent, "replay-input", ReplayInputCmd);
#endif

#ifdef KT_HISTOGRAM
    install_tcl_soar_cmd(the_agent, "init-kt", initKTHistogramCmd);
#endif
#ifdef DC_HISTOGRAM
    install_tcl_soar_cmd(the_agent, "init-dc", initDCHistogramCmd);
#endif

}
