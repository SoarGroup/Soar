/*
 * =======================================================================
 *  File:  soarCommands.c
 *
 * This file includes the definitions of the Soar Command set.
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

extern Tcl_Interp *tcl_soar_agent_interpreters[MAX_SIMULTANEOUS_AGENTS];
extern remove_rhs_function( Symbol *name );

#ifdef DC_HISTOGRAM 
int initDCHistogramCmd (ClientData clientData, 
	       Tcl_Interp * interp,
	       int argc, char *argv[])
{
  soarResult res;

  init_soarResult(res);

  Soar_SelectGlobalInterpByInterp(interp);

   soar_cInitializeDCHistogram( 450, 1 );
    return TCL_OK;


}
#endif 
#ifdef KT_HISTOGRAM 
int initKTHistogramCmd (ClientData clientData, 
	       Tcl_Interp * interp,
	       int argc, char *argv[])
{
  soarResult res;

  init_soarResult(res);

  Soar_SelectGlobalInterpByInterp(interp);

   soar_cInitializeKTHistogram( 450 );
    return TCL_OK;


}
#endif 






int AddWmeCmd (ClientData clientData, 
	       Tcl_Interp * interp,
	       int argc, const char *argv[])
{
  int i;
	soarResult res;

  init_soarResult(res);

  Soar_SelectGlobalInterpByInterp(interp);

  // SAN
//  for (i = 0; i < argc; i++)
//	  print("%s\n", argv[i]); 

  if( soar_AddWme( argc, argv, &res ) == SOAR_OK ) {
    sprintf( interp->result, "%s", res.result);
    return TCL_OK;
  } else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}


int AttributePreferencesModeCmd (ClientData clientData, 
				 Tcl_Interp * interp,
				 int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);

  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_AttributePreferencesMode( argc, argv, &res ) == SOAR_OK ) {
    printf( "DONE\n" );
	interp->result = res.result;
	return TCL_OK;
  }
  else {
       interp->result = res.result;
       return TCL_ERROR;
  }

}


int ChunkNameFormatCmd (ClientData clientData, 
	                Tcl_Interp * interp,
	                int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);

  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_ChunkNameFormat( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}



int DefWmeDepthCmd (ClientData clientData, 
		    Tcl_Interp * interp,
		    int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);

  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_DefaultWmeDepth( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}



int 
DestroyAgentCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, char *argv[])
{
  soarResult res;
  int agent_id;

  init_soarResult(res);

  Soar_SelectGlobalInterpByInterp(interp);

  agent_id = soar_agent->id;
/*  printf( "Calling Destroy Agent on Agent %d\n", agent_id ); */

  remove_rhs_function( make_sym_constant("tcl") );
  if( soar_DestroyAgent( argc, argv, &res ) == SOAR_OK ) {
    tcl_soar_agent_interpreters[agent_id] = NIL;
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}



int ExciseCmd (ClientData clientData, 
	       Tcl_Interp * interp,
	       int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);


  if( soar_Excise( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}


int ExplainBacktracesCmd (ClientData clientData, 
		Tcl_Interp * interp,
		int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);

  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_ExplainBacktraces( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}


int FiringCountsCmd (ClientData clientData, 
		     Tcl_Interp * interp,
		     int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_FiringCounts( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}



int FormatWatchCmd (ClientData clientData, 
		    Tcl_Interp * interp,
		    int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_FormatWatch( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}

int GDS_PrintCmd (ClientData clientData, 
		  Tcl_Interp * interp,
		  int argc, const char *argv[])
{


  soar_ecGDSPrint();
  return TCL_OK;
}



int IndifferentSelectionCmd (ClientData clientData, 
			     Tcl_Interp * interp,
			     int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_IndifferentSelection( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}

int InitSoarCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, const char *argv[])
{

  Soar_SelectGlobalInterpByInterp(interp);


  soar_cReInitSoar( );

  return TCL_OK;

}


int InputPeriodCmd (ClientData clientData, 
		    Tcl_Interp * interp,
		    int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);


  if ( soar_InputPeriod( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}

int InternalSymbolsCmd (ClientData clientData, 
			Tcl_Interp * interp,
			int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_InternalSymbols( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}


int LearnCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  
  if( soar_Learn( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}

int MatchesCmd (ClientData clientData, 
		Tcl_Interp * interp,
		int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_Matches( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}


int MaxChunksCmd (ClientData clientData, 
		  Tcl_Interp * interp,
		  int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_MaxChunks( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}


int MaxElaborationsCmd (ClientData clientData, 
			Tcl_Interp * interp,
			int argc, const char *argv[])
{

  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_MaxElaborations( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}



int MemoriesCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, const char *argv[])
{
 
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_Memories( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}



int MultiAttrCmd (ClientData clientData, 
		  Tcl_Interp * interp,
		  int argc, const char *argv[])
{

  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_MultiAttributes( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}


int OSupportModeCmd (ClientData clientData, 
		     Tcl_Interp * interp,
		     int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);


  if ( soar_OSupportMode( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}



int Operand2Cmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{

  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_Operand2( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}



int ProductionFindCmd (ClientData clientData, 
	   Tcl_Interp * interp,
	   int argc, const char *argv[])
{  
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_ProductionFind( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}


int PreferencesCmd (ClientData clientData,
                    Tcl_Interp * interp,
                    int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_Preferences( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}




int PrintCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_Print( argc, argv, &res ) == SOAR_OK ) {
    /* interp->result = res.result; */
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}

int PwatchCmd (ClientData clientData, 
	       Tcl_Interp * interp,
	       int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_PWatch( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}


int SoarExcludedBuildInfoCmd( ClientData clientData,
		Tcl_Interp * interp,
		int argc, const char *argv[] ){
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);
  
  soar_ExcludedBuildInfo( argc, argv, &res );

  return TCL_OK;
}

int SoarBuildInfoCmd( ClientData clientData,
		Tcl_Interp * interp,
		int argc, const char *argv[] ){
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);
  
  soar_BuildInfo( argc, argv, &res );

  return TCL_OK;
}

#ifdef USE_DEBUG_UTILS


int PrintPoolCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);
  
  if ( soar_Pool( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  }
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
    

  return TCL_OK;
}


#endif

int QuitCmd (ClientData clientData, 
	     Tcl_Interp * interp,
	     int argc, const char *argv[])
{
  static char cmd[] = "exit";
  soarResult res;

  init_soarResult(res);
  soar_cQuit( );

  (void) Tcl_Eval(interp, cmd);
  return TCL_OK; /* Unreachable, but here to placate the compiler */
}



int RemoveWmeCmd (ClientData clientData, 
		  Tcl_Interp * interp,
		  int argc, const char *argv[])
{

  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);



  if ( soar_RemoveWme( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }

}


int RunCmd (ClientData clientData, 
	    Tcl_Interp * interp,
	    int argc, const char *argv[])
{

  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_Run( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}




int SpCmd (ClientData clientData, 
	   Tcl_Interp * interp,
	   int argc, const char *argv[])
{

  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);


  if ( soar_Sp( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}



int StatsCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{

  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_Stats( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}





int StopSoarCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if ( soar_Stop( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}




int VerboseCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{
  soarResult res;

  init_soarResult(res);
  if ( soar_Verbose( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}



#ifdef MACINTOSH
int LogCmd(ClientData clientData,
	   Tcl_Interp * interp,
	   int argc, const char *argv[] ) {

  soarResult res;

  init_soarResult(res);
  if ( soar_Log( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}

#else 

int LogCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{
  soarResult res;
  char **newArgv;
  bool tildeOccurs, tildeFlag;
  int i, result;
  const char *c;
  Tcl_DString buffer;

  /*
    Before we pass these arguments to Soar, we need to determine
    whether Tilde substitution is necessary.
    we'll look through all arguments to and do any tilde substitution
    necessary although in reality it should only occur when the -new or
    -existing flags are being used.
  */

  init_soarResult(res);

  tildeOccurs = FALSE;
  for( i = 1; i < argc && !tildeOccurs; i++ ) {
    c = argv[i];
    while( *c ) {
      if ( *c++ == '~' ) {
	tildeOccurs = TRUE;
	break;
      }
    }
  }
  /* If the tilde occurs, we need a new Argv array. */
  if ( tildeOccurs ) {
    
    newArgv = malloc( argc * sizeof( char * ) );
    for ( i = 0; i < argc; i++ ) {
      c = argv[i];
      tildeFlag = FALSE;
      while( *c ) {
	if ( *c++ == '~' ) {
	  tildeFlag = TRUE;
	  break;
	}
      }
      if ( tildeFlag ) {
	c = Tcl_TildeSubst( interp, argv[i], &buffer );
	newArgv[i] = malloc( (strlen( c ) + 1) * sizeof(char) );
	strcpy( newArgv[i], c );
	printf( "Substituting '%s' for '%s'\n", argv[i], c );
      }
      else {
	newArgv[i] = malloc( (strlen( argv[i] ) + 1) * sizeof(char) );
	strcpy( newArgv[i], argv[i] );
      }
    }
  }
      
  if ( tildeOccurs ) {
    printf( "Substitution occured\n" );
    result = soar_Log( argc, newArgv, &res );
    for( i = 0; i < argc; i++ ) 
      free( newArgv[i] );
    free( newArgv );
    Tcl_DStringFree( &buffer );
  }
  else {
    result = soar_Log( argc, argv, &res );
  }
  
  interp->result = res.result; 
  if ( result  == SOAR_OK ) 
    return TCL_OK;

  return TCL_ERROR;
  
}

#endif


int WaitSNCCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{

  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  if( soar_WaitSNC( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}



int WarningsCmd (ClientData clientData, 
		 Tcl_Interp * interp,
		 int argc, const char *argv[])
{

  soarResult res;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);


  if ( soar_Warnings( argc, argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}





int WatchCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{
  soarResult res;
  int i;
  char **newArgv;
  int newArgc;
  int result;
  const char *a;


  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

  for( i = 1; i < argc; i++ ) {
    
    if ( string_match("aliases", argv[i]) ) {
      if( argv[i+1] == NULL ) {
	interp->result = "Missing setting for watch alias, should be -on|-off";
	return TCL_ERROR;
      }
      else if ( string_match("-on", argv[i+1]) ) {
	Tcl_SetVar( interp, "print_alias_switch", "on", TCL_GLOBAL_ONLY );
	argv[i] = '\0';
	argv[i+1] = '\0';
	break;
      }
      else if ( string_match("-off", argv[i+1] ) ) {
	Tcl_SetVar( interp, "print_alias_switch", "off", TCL_GLOBAL_ONLY );
	argv[i] = '\0';
	argv[i+1] = '\0';
	break;
      }
      else {
	sprintf( interp->result, "Unrecognized argument to watch alias : %s",
		 argv[i+1] );
	return TCL_ERROR;
      }
    }
  }

  /*
    Since we've got here, we handled all the interface dependendent stuff,
    thus we need to remove the already acted upon arguments, and pass the
    rest to Soar 
  */
  
  newArgc = 0;
  newArgv = malloc(  argc * sizeof( char * ) );
  for ( i = 0; i < argc; i++ ) {
    if ( argv[i] != NULL ) {
      
      newArgc++;
      newArgv[i] = malloc( (strlen( argv[i] ) + 1)*sizeof( char) );
      strcpy( newArgv[i], argv[i] );

    }
  }

  if ( newArgc == 1 && argc != 1 ) {
    /* Then we did all that was necessary, so return */
    return TCL_OK;
  }

  result =  soar_Watch( newArgc, newArgv, &res );
   
  /* In the case of newArgc == 1, we are printing watch settings.
     The kernel does not know about the print_alias_switch, so
     we need to do that here
  */
  if ( newArgc == 1 ) {
    print( "  Alias printing: %s\n", (a=Tcl_GetVar( interp, "print_alias_switch", 0)) ? a : "(null)" );
  }


  
  /* Free the argument block */
  for ( i = 0; i < newArgc; i++ ) {
    free( newArgv[i] );
  }
  free( newArgv ); 
  
  interp->result = res.result;
  
  if ( result == SOAR_OK ) 
    return TCL_OK;

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


int EchoCmd (ClientData clientData, 
	     Tcl_Interp * interp,
	     int argc, const char *argv[])
{
  int i;
  bool newline = TRUE;

  Soar_SelectGlobalInterpByInterp(interp);

  for (i = 1; i < argc; i++)
    {
      if (string_match_up_to("-nonewline", argv[i], 2))
	{
	  newline = FALSE;
	}
      else 
	{
	  Soar_LogAndPrint((agent *) clientData, argv[i]);
	  if ((i + 1) < argc) 
	    {
	      Soar_LogAndPrint((agent *) clientData, " ");
	    }
	}
    }

  if (newline)
    {
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

int AskCmd (ClientData clientData, 
			Tcl_Interp * interp,
			int argc, const char *argv[])
{
  static char * too_few_args_string = "Too few arguments, should be: ask [-add <proc>] | [-remove]";
  static char * too_many_args_string = "Too many arguments, should be: ask [-add <proc>] | [-remove]";


  Soar_SelectGlobalInterpByInterp(interp);


  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = too_few_args_string;
      return TCL_ERROR;
    }
  if (string_match_up_to(argv[1], "-add", 2))
    {
      if (argc < 3)
		{
		  interp->result = too_few_args_string;
		  return TCL_ERROR;
		}

      if (argc > 3)
		{
		  interp->result = too_many_args_string;
		  return TCL_ERROR;
		}

      if (argc == 3)
		{
		  
		  soar_cPushCallback((agent *) clientData, ASK_CALLBACK,
							 soar_ask_callback_to_tcl,
							 (soar_callback_data) savestring(argv[2]), 
							 soar_callback_data_free_string );
		}
	}
  else if (string_match_up_to(argv[1], "-remove", 2))
	{
	  soar_cRemoveAllCallbacksForEvent( (agent *)clientData, ASK_CALLBACK );
	}

  soar_cPushCallback((soar_callback_agent) clientData,
					 PRINT_CALLBACK,
					 (soar_callback_fn) Soar_AppendResult, 
					 (soar_callback_data) NULL,
					 (soar_callback_free_fn) NULL);

  /* if a log file is open, then we need to push a dummy callback
   * so that we don't get extraneous prints mucking up the log file.
   * Addresses bug # 248.  KJC 01/00 
   */
  if (soar_exists_callback((soar_callback_agent) clientData, 
						   LOG_CALLBACK)) {
	
	soar_cPushCallback((soar_callback_agent) clientData, 
					   LOG_CALLBACK,
					   (soar_callback_fn) Soar_DiscardPrint, 
					   (soar_callback_data) NULL,
					   (soar_callback_free_fn) NULL);
  }
	  
  soar_cListAllCallbacksForEvent((agent *) clientData, ASK_CALLBACK);
  soar_cPopCallback((soar_callback_agent) clientData, 
					PRINT_CALLBACK);
  if ( soar_exists_callback((soar_callback_agent) clientData, 
							LOG_CALLBACK)) {
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

static int io_proc_counter = 1;

int IOCmd (ClientData clientData, 
	   Tcl_Interp * interp,
	   int argc, const char *argv[])
{
  static char * too_few_args_string = "Too few arguments, should be: io [-add -input script [id]] | [-add -output script id] | [-delete [-input|-output] id] | [-list [-input|-output]";
  static char * too_many_args_string = "Too many arguments, should be: io [-add -input script [id]] | [-add -output script id] | [-delete [-input|-output] id] | [-list [-input|-output]";
  const char * io_id;
  char   buff[10];          /* What size is good here? */

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = too_few_args_string;
      return TCL_ERROR;
    }
  if (string_match_up_to(argv[1], "-add", 2))
    {
      if (argc < 4)
	{
	  interp->result = too_few_args_string;
	  return TCL_ERROR;
	}

      if (argc > 5)
	{
	  interp->result = too_many_args_string;
	  return TCL_ERROR;
	}

      if (argc == 5)
	{
	  io_id = argv[4];
	}
      else
	{
	  sprintf(buff, "m%d", io_proc_counter++);
	  io_id = buff;
	}

      {
	if (string_match_up_to(argv[2], "-input", 2))
	  {
	    soar_cAddInputFunction((agent *) clientData, 
				   soar_input_callback_to_tcl,
				   (soar_callback_data) savestring(argv[3]), 
				   soar_callback_data_free_string,
				   (soar_callback_id) io_id);
	  }
	else if (string_match_up_to(argv[2], "-output", 2))
	  {
            /* Soar-Bugs #131, id required for output - TMH */
            if (argc < 5)
	    {
	      interp->result = too_few_args_string;
	      return TCL_ERROR;
	    }

	    soar_cAddOutputFunction((agent *) clientData, 
				    soar_output_callback_to_tcl,
				    (soar_callback_data) savestring(argv[3]), 
				    soar_callback_data_free_string,
				    (soar_callback_id) io_id);
	  }
	else
	  {
	    sprintf(interp->result,
		    "%s: Unrecognized IO type: %s %s",
		    argv[0], argv[1], argv[2]);
	    return TCL_ERROR;
	  }

	interp->result = (char*)io_id;
	return TCL_OK;
      }
    }
  else if (string_match_up_to(argv[1], "-delete", 2))
    {
      switch (argc) {
      case 2:
      case 3:	  
	interp->result = too_few_args_string;
	return TCL_ERROR;
      case 4:	  /* Delete single callback for given event */
	  {
	    if (string_match_up_to(argv[2], "-input", 2))
	      {
		soar_cRemoveInputFunction((agent *) clientData, argv[3]);
	      }
	    else if (string_match_up_to(argv[2], "-output", 2))
	      {
		soar_cRemoveOutputFunction((agent *) clientData, argv[3]);
	      }
	    else
	      {
		sprintf(interp->result,
			"Attempt to delete unrecognized io type: %s",
			argv[2]);
		return TCL_ERROR;
	      }
	  }
	  break;
      default:
	interp->result = too_many_args_string;
	return TCL_ERROR;
      }
    }
  else if (string_match_up_to(argv[1], "-list", 2))
    {
      switch (argc) {
      case 2:	  
	interp->result = too_few_args_string;
	return TCL_ERROR;
      case 3:	  
	{
	  SOAR_CALLBACK_TYPE ct;
	  
	  if (string_match_up_to(argv[2], "-input", 2))
	    {
	      ct = INPUT_PHASE_CALLBACK;
	    }
	  else if (string_match_up_to(argv[2], "-output", 2))
	    {
	      ct = OUTPUT_PHASE_CALLBACK;
	    }
	  else
	    {
	      sprintf(interp->result,
		      "Attempt to list unrecognized io type: %s",
		      argv[2]);
	      return TCL_ERROR;
	    }

	  soar_cPushCallback((soar_callback_agent) clientData,
			     PRINT_CALLBACK,
			     (soar_callback_fn) Soar_AppendResult, 
			     (soar_callback_data) NULL,
			     (soar_callback_free_fn) NULL);
	  /* if a log file is open, then we need to push a dummy callback
	   * so that we don't get extraneous prints mucking up the log file.
	   * Addresses bug # 248.  KJC 01/00 
	   */
	  if (soar_exists_callback((soar_callback_agent) clientData, 
				   LOG_CALLBACK)) {

	    soar_cPushCallback((soar_callback_agent) clientData, 
			       LOG_CALLBACK,
			       (soar_callback_fn) Soar_DiscardPrint, 
			       (soar_callback_data) NULL,
			       (soar_callback_free_fn) NULL);
	  }
	  
	  soar_cListAllCallbacksForEvent((agent *) clientData, ct);
	  soar_cPopCallback((soar_callback_agent) clientData, 
			    PRINT_CALLBACK);
	  if ( soar_exists_callback((soar_callback_agent) clientData, 
				    LOG_CALLBACK)) {
	    soar_cPopCallback((soar_callback_agent) clientData, LOG_CALLBACK);
	  }
	}
	break;
      default:
	interp->result = too_many_args_string;
	return TCL_ERROR;
      }

    }
  else
    {
      sprintf(interp->result, 
	      "Unrecognized option to io command: %s", argv[1]);
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

int AttentionLapseCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      print_current_attention_lapse_settings();
      return TCL_OK;
    }

  {int i;

   for (i = 1; i < argc; i++)
     {
       if (string_match("-on", argv[i]))
	 {
	   set_sysparam (ATTENTION_LAPSE_ON_SYSPARAM, TRUE); 
           wake_from_attention_lapse();
	 }
       else if (string_match_up_to("-off", argv[i], 3))
	 {
	   set_sysparam (ATTENTION_LAPSE_ON_SYSPARAM, FALSE); 
	 }
       else
	 {
	   sprintf(interp->result,
		   "Unrecognized argument to attention-lapse command: %s",
		   argv[i]);
	   return TCL_ERROR;
	 }
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

int WakeFromAttentionLapseCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{
  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1) {
     wake_from_attention_lapse();
     return TCL_OK;
  } else {
      interp->result = "Too many arguments, should be: wake-from-attention-lapse";
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

int StartAttentionLapseCmd (ClientData clientData, 
	      Tcl_Interp * interp,
	      int argc, const char *argv[])
{
  int duration;

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc < 2) {
      interp->result = "Too few arguments, should be: start-attention-lapse integer";
     return TCL_ERROR;
  } else if (argc > 2) {
      interp->result = "Too many arguments, should be: start-attention-lapse integer";
     return TCL_ERROR;
  }

  if (Tcl_GetInt(interp, argv[1], &duration) == TCL_OK)
    {
      start_attention_lapse((long)duration);
    }
  else
    {
      sprintf(interp->result, 
	      "Expected integer for attention lapse duration: %s", 
	      argv[1]);
      return TCL_ERROR;
    }

  return TCL_OK;
}

#endif  /* ATTENTION_LAPSE */



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

int MonitorCmd (ClientData clientData, 
		Tcl_Interp * interp,
		int argc, const char *argv[])
{
  static char * too_few_args_string = "Too few arguments, should be: monitor [-add event script [id]] | [-delete event [id]] | [-list [event] | clear]";
  static char * too_many_args_string = "Too many arguments, should be: monitor [-add event script [id]] | [-delete event [id]] | [-list [event] | -clear]";
  const char * monitor_id;
  char   buff[10];          /* What size is good here? */

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = too_few_args_string;
      return TCL_ERROR;
    }

  if (string_match_up_to(argv[1], "-add", 2))
    {
      if (argc < 4)
	{
	  interp->result = too_few_args_string;
	  return TCL_ERROR;
	}

      if (argc > 5)
	{
	  interp->result = too_many_args_string;
	  return TCL_ERROR;
	}

      if (argc == 5)
	{
	  monitor_id = argv[4];
	}
      else
	{
	  sprintf(buff, "m%d", monitor_counter++);
	  monitor_id = buff;
	}

      {
	SOAR_CALLBACK_TYPE ct;
	
	ct = soar_cCallbackNameToEnum(argv[2], TRUE);
	if (ct)
	  {
	    soar_cAddCallback((agent *) clientData, 
			      ct, 
			      soar_callback_to_tcl, 
			      (soar_callback_data) savestring(argv[3]), 
			      soar_callback_data_free_string,
			      (soar_callback_id) monitor_id);
	    interp->result = (char*)monitor_id;
	    return TCL_OK;
	  }
	else
	  {
	    sprintf(interp->result,
		    "Attempt to add unrecognized callback event: %s",
		    argv[2]);
	    return TCL_ERROR;
	  }
      }
    }
  else if (string_match_up_to(argv[1], "-delete", 2))
    {
      switch (argc) {
      case 2:
	interp->result = too_few_args_string;
	return TCL_ERROR;
      case 3:	  /* Delete all callbacks of the given type */
	  {
	    SOAR_CALLBACK_TYPE ct;

	    ct = soar_cCallbackNameToEnum(argv[2], TRUE);
	    if (ct)
	      {
		soar_cRemoveAllCallbacksForEvent((agent *) clientData, ct);
	      }
	    else
	      {
		sprintf(interp->result,
			"Attempt to delete unrecognized callback event: %s",
			argv[2]);
		return TCL_ERROR;
	      }
	  }
	  break;
      case 4:	  /* Delete single callback for given event */
	  {
	    SOAR_CALLBACK_TYPE ct;

	    ct = soar_cCallbackNameToEnum(argv[2], TRUE);
	    if (ct)
	      {
		soar_cRemoveCallback((agent *) clientData, ct, argv[3]);
	      }
	    else
	      {
		sprintf(interp->result,
			"Attempt to delete unrecognized callback event: %s",
			argv[2]);
		return TCL_ERROR;
	      }
	  }
	  break;
      default:
	interp->result = too_many_args_string;
	return TCL_ERROR;
      }
    }
  else if (string_match_up_to(argv[1], "-list", 2))
    {
      switch (argc) {
      case 2:	  
	soar_cListAllCallbacks((agent *) clientData, TRUE);
	break;
      case 3:	  
	{
	  SOAR_CALLBACK_TYPE ct;
	  
	  ct = soar_cCallbackNameToEnum(argv[2], TRUE);
	  if (ct) {
	    soar_cPushCallback((soar_callback_agent) clientData,
			       PRINT_CALLBACK,
			       (soar_callback_fn) Soar_AppendResult, 
			       (soar_callback_data) NULL,
			       (soar_callback_free_fn) NULL);
	    
	    /* if a log file is open, then we need to push a dummy callback
	     * so that we don't get extraneous prints mucking up the log 
	     * file. Addresses bug # 248.  KJC 01/00 
	     */
	    if (soar_exists_callback((soar_callback_agent) clientData,
				     LOG_CALLBACK)) {
	      
	      soar_cPushCallback((soar_callback_agent) clientData, 
				 LOG_CALLBACK,
				 (soar_callback_fn) Soar_DiscardPrint, 
				 (soar_callback_data) NULL,
				 (soar_callback_free_fn) NULL);
	    }
	    
	    soar_cListAllCallbacksForEvent((agent *) clientData, ct);
	    soar_cPopCallback((soar_callback_agent) clientData, 
			      PRINT_CALLBACK);
	    if (soar_exists_callback((soar_callback_agent) clientData, 
				     LOG_CALLBACK)) {
	      
	      soar_cPopCallback((soar_callback_agent) clientData, 
				LOG_CALLBACK);
	      
	    }

	  }
	  else {
	    sprintf(interp->result,
		    "Attempt to list unrecognized callback event: %s",
		    argv[2]);
	    return TCL_ERROR;
	  }
	}
	break;
      default:
	interp->result = too_many_args_string;
	return TCL_ERROR;
      }

    }
  else if (string_match_up_to(argv[1], "-test", 2))
    {
      soar_cTestAllMonitorableCallbacks((agent *)clientData);
    }
  else if (string_match_up_to(argv[1], "-clear", 2))
    {
      /* Delete all callbacks of all types */
      soar_cRemoveAllMonitorableCallbacks((agent *) clientData);
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized option to monitor command: %s",
	      argv[1]);
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
 *      the C code performs an assignment to interp->result then
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
int OutputStringsDestCmd (ClientData clientData, 
			  Tcl_Interp * interp,
			  int argc, const char *argv[])
{
  static char * too_few_args = "Too few arguments, should be: output-strings-destination [ -push [[-text-widget widget-name [interp-name]] | [-channel channel-id] | [-procedure tcl-procedure-name] | -discard |-append-to-result] | -pop]";

  Soar_SelectGlobalInterpByInterp(interp);

  if (argc == 1)
    {
      interp->result = too_few_args;
      return TCL_ERROR;
    }

  if (string_match("-push", argv[1]))
    {
      if (string_match("-text-widget", argv[2]))
	{
	  if (argc == 3)
	    {
	      interp->result = too_few_args;
	      return TCL_ERROR;
	    }
	  else
	    {
	      /* We assume that we'll be printing to the same interp. */
	      Tcl_Interp * print_interp = interp;
	      Soar_TextWidgetPrintData * data;

	      if (argc > 4)
		{
		  /* Too many arguments */
		  interp->result = "Too many arguments";
		  return TCL_ERROR;
		}

	      data = Soar_MakeTextWidgetPrintData (print_interp, argv[3]);
	      soar_cPushCallback((soar_callback_agent) clientData, 
				 PRINT_CALLBACK,
				 (soar_callback_fn) Soar_PrintToTextWidget,
				 (soar_callback_data) data,
				 (soar_callback_free_fn) Soar_FreeTextWidgetPrintData);
     	    }
	}
/* RMJ 7-1-97 */
      else if (string_match("-procedure", argv[2]))
	{
	  if (argc == 3)
	    {
	      interp->result = too_few_args;
	      return TCL_ERROR;
	    }
	  else
	    {
	      /* We assume that we'll be printing to the same interp. */
	      Tcl_Interp * print_interp = interp;
	      Soar_TextWidgetPrintData * data;

	      if (argc > 4)
		{
		  /* Too many arguments */
		  interp->result = "Too many arguments";
		  return TCL_ERROR;
		}

	      data = Soar_MakeTextWidgetPrintData (print_interp, argv[3]);
	      soar_cPushCallback((soar_callback_agent) clientData, 
				 PRINT_CALLBACK,
				 (soar_callback_fn) Soar_PrintToTclProc,
				 (soar_callback_data) data,
				 (soar_callback_free_fn) Soar_FreeTextWidgetPrintData);
     	    }
	}
      else if (string_match("-channel", argv[2]))
        {
		Tcl_Channel channel;
		int mode;

		if (argc == 3) {
			interp->result = too_few_args;
			return TCL_ERROR;
		}

		if ((channel = Tcl_GetChannel(interp, argv[3], &mode)) == NULL
		||  ! (mode & TCL_WRITABLE)) {
			sprintf(interp->result, "%s is not a valid channel for writing.", argv[3]);
			return TCL_ERROR;
		}

		soar_cPushCallback((soar_callback_agent) clientData,
			PRINT_CALLBACK,
			(soar_callback_fn) Soar_PrintToChannel,
			(soar_callback_data) channel,
			(soar_callback_free_fn) NULL);

	}
      else if (string_match("-discard", argv[2]))
	{
	  soar_cPushCallback((soar_callback_agent) clientData, 
			     PRINT_CALLBACK,
			     (soar_callback_fn) Soar_DiscardPrint,
			     (soar_callback_data) NULL,
			     (soar_callback_free_fn) NULL);
	}
      else if (string_match("-append-to-result", argv[2]))
	{
	  soar_cPushCallback((soar_callback_agent) clientData, 
			     PRINT_CALLBACK,
			     (soar_callback_fn) Soar_AppendResult,
			     (soar_callback_data) NULL,
			     (soar_callback_free_fn) NULL);
	}
      else
	{
	  sprintf(interp->result,
		  "Unrecognized argument to %s %s: %s",
		  argv[0], argv[1], argv[2]);
	  return TCL_ERROR;      
	}
    }
  else if (string_match("-pop", argv[1]))
    {
      soar_cPopCallback((soar_callback_agent) clientData,
			PRINT_CALLBACK);
    }
  else
    {
      sprintf(interp->result,
	      "Unrecognized argument to %s: %s",
	      argv[0], argv[1]);
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

int CaptureInputCmd (ClientData clientData, 
	    Tcl_Interp * interp,
	    int argc, const char *argv[])
{
  soarResult res;
  const char **new_argv;
  int i;
  char *buffer;
  Tcl_DString temp;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

#ifndef MACINTOSH
  if ( argc > 2 ) {
    /* Then in theory we should be opening a file,
     * and we need to do some tilde substitution
     */
    
    new_argv = (char **) malloc( argc * sizeof( char *) );
    for( i = 0; i < 2; i++ ) {
      new_argv[i] = (char *) malloc( sizeof(char) * (strlen( argv[i] ) + 1) );
      strcpy( (char*)new_argv[i], argv[i] );
    }
    for( i = 2; i < argc; i++ ) {
      /* Hopefully, there will just be 1 iteration through here.... */
      buffer = Tcl_TildeSubst(interp, argv[i], &temp);
      new_argv[i] = (char *) malloc( sizeof(char) * (strlen( buffer ) + 1) );
      strcpy( (char*)new_argv[i], buffer );
    }
  }
  else {
    new_argv = argv;
  }
#else
  new_argv = argv;

#endif /* MACINTOSH */


  if( soar_CaptureInput( argc, new_argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
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

int ReplayInputCmd (ClientData clientData, 
	    Tcl_Interp * interp,
	    int argc, const char *argv[])
{

  soarResult res;
  const char **new_argv;
  int i;
  char *buffer;
  Tcl_DString temp;

  init_soarResult(res);
  Soar_SelectGlobalInterpByInterp(interp);

#ifndef MACINTOSH
  if ( argc > 2 ) {
    /* Then in theory we should be opening a file,
     * and we need to do some tilde substitution
     */
    
    new_argv = (char **) malloc( argc * sizeof( char *) );
    for( i = 0; i < 2; i++ ) {
      new_argv[i] = (char *) malloc( sizeof(char) * (strlen( argv[i] ) + 1) );
      strcpy( (char*)new_argv[i], argv[i] );
    }
    for( i = 2; i < argc; i++ ) {
      /* Hopefully, there will just be 1 iteration through here.... */
      buffer = Tcl_TildeSubst(interp, argv[i], &temp);
      new_argv[i] = (char *) malloc( sizeof(char) * (strlen( buffer ) + 1) );
      strcpy( (char*)new_argv[i], buffer );
    }
  }
  else {
    new_argv = argv;
  }
#else
  new_argv = argv;

#endif /* MACINTOSH */


  if( soar_ReplayInput( argc, new_argv, &res ) == SOAR_OK ) {
    interp->result = res.result;
    return TCL_OK;
  } 
  else {
    interp->result = res.result;
    return TCL_ERROR;
  }
}

#endif /* USE_CAPTURE_REPLAY */





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

int ReteNetCmd (ClientData clientData, 
		Tcl_Interp * interp,
		int argc, const char *argv[])
{
  

  Tcl_DString buffer;
  char * fullname;
  int (*rete_net_op)( char *); 

  Soar_SelectGlobalInterpByInterp(interp);
  
  if (argc < 3)
    {
      interp->result =  
         "Too few arguments.\nUsage: rete-net {-save | -load} filename.";
      return TCL_ERROR;
    }

  if (argc > 3)
    {
      interp->result = 
         "Too many arguments.\nUsage: rete-net {-save | -load} filename.";
      return TCL_ERROR;
    }

  fullname = Tcl_TildeSubst(interp, argv[2], &buffer);
  
  if ( string_match( argv[1], "-save" ) ) 
    rete_net_op = soar_cSaveReteNet;
  else if ( string_match( argv[1], "-load" ) ) 
    rete_net_op = soar_cLoadReteNet;
  else {
    interp->result = 
      "Unrecognized argument to ReteNet command: %s. Should be -save|-load";
    return TCL_ERROR;
  }
  
  if ( (rete_net_op)( fullname ) == SOAR_ERROR ) { 
    return TCL_ERROR;
  }
  return TCL_OK;

}






void Soar_InstallCommands (agent * the_agent)
{
  install_tcl_soar_cmd(the_agent, "add-wme",             AddWmeCmd);
  install_tcl_soar_cmd(the_agent, "ask",                 AskCmd);
  #ifdef ATTENTION_LAPSE  /* RMJ */
  install_tcl_soar_cmd(the_agent, "attention-lapse",     AttentionLapseCmd);
  install_tcl_soar_cmd(the_agent, "start-attention-lapse", StartAttentionLapseCmd);
  install_tcl_soar_cmd(the_agent, "wake-from-attention-lapse", WakeFromAttentionLapseCmd);
  #endif  /* ATTENTION_LAPSE */

  install_tcl_soar_cmd(the_agent, "attribute-preferences-mode", AttributePreferencesModeCmd);
  install_tcl_soar_cmd(the_agent, "chunk-name-format",   ChunkNameFormatCmd); /* kjh(CUSP-B14) */
  install_tcl_soar_cmd(the_agent, "default-wme-depth",   DefWmeDepthCmd);
  install_tcl_soar_cmd(the_agent, "echo",                EchoCmd);
  install_tcl_soar_cmd(the_agent, "excise",              ExciseCmd);
  install_tcl_soar_cmd(the_agent, "explain-backtraces",  ExplainBacktracesCmd);
  install_tcl_soar_cmd(the_agent, "firing-counts",       FiringCountsCmd);
  install_tcl_soar_cmd(the_agent, "format-watch",        FormatWatchCmd); 
  install_tcl_soar_cmd(the_agent, "indifferent-selection", IndifferentSelectionCmd);
  install_tcl_soar_cmd(the_agent, "init-soar",           InitSoarCmd);
  install_tcl_soar_cmd(the_agent, "input-period",        InputPeriodCmd);
  install_tcl_soar_cmd(the_agent, "internal-symbols",    InternalSymbolsCmd);  
  install_tcl_soar_cmd(the_agent, "io",                  IOCmd);
  install_tcl_soar_cmd(the_agent, "learn",               LearnCmd);
  install_tcl_soar_cmd(the_agent, "log",                 LogCmd);
  install_tcl_soar_cmd(the_agent, "matches",             MatchesCmd);
  install_tcl_soar_cmd(the_agent, "max-chunks",          MaxChunksCmd);
  install_tcl_soar_cmd(the_agent, "max-elaborations",    MaxElaborationsCmd);
  install_tcl_soar_cmd(the_agent, "memories",            MemoriesCmd);
  install_tcl_soar_cmd(the_agent, "monitor",             MonitorCmd);
  install_tcl_soar_cmd(the_agent, "multi-attributes",     MultiAttrCmd);
  install_tcl_soar_cmd(the_agent, "o-support-mode",      OSupportModeCmd);
  install_tcl_soar_cmd(the_agent, "output-strings-destination", OutputStringsDestCmd);
  install_tcl_soar_cmd(the_agent, "production-find",     ProductionFindCmd);
  install_tcl_soar_cmd(the_agent, "preferences",         PreferencesCmd);
  install_tcl_soar_cmd(the_agent, "print",               PrintCmd);
  install_tcl_soar_cmd(the_agent, "pwatch",              PwatchCmd);
  install_tcl_soar_cmd(the_agent, "quit",                QuitCmd);  
/*  install_tcl_soar_cmd(the_agent, "record",              RecordCmd);  /* kjh(CUSP-B10) */
/*  install_tcl_soar_cmd(the_agent, "replay",              ReplayCmd);  /* kjh(CUSP-B10) */
  install_tcl_soar_cmd(the_agent, "remove-wme",          RemoveWmeCmd);
  install_tcl_soar_cmd(the_agent, "rete-net",            ReteNetCmd);
  install_tcl_soar_cmd(the_agent, "run",                 RunCmd);
  install_tcl_soar_cmd(the_agent, "sp",                  SpCmd);
  install_tcl_soar_cmd(the_agent, "stats",               StatsCmd);
  install_tcl_soar_cmd(the_agent, "stop-soar",           StopSoarCmd);
  install_tcl_soar_cmd(the_agent, "warnings",            WarningsCmd);
  install_tcl_soar_cmd(the_agent, "watch",               WatchCmd);
/* REW: begin 09.15.96 */
  install_tcl_soar_cmd(the_agent, "gds_print",           GDS_PrintCmd);
  /* REW: 7.1/waterfall:soarAppInit.c  merge */
  install_tcl_soar_cmd(the_agent, "verbose",            VerboseCmd);
  install_tcl_soar_cmd(the_agent, "soar8",              Operand2Cmd);
  install_tcl_soar_cmd(the_agent, "waitsnc",            WaitSNCCmd);
/* REW: end   09.15.96 */

#ifdef USE_DEBUG_UTILS
  install_tcl_soar_cmd( the_agent, "pool",   PrintPoolCmd);


#endif
  install_tcl_soar_cmd( the_agent, "build-info", SoarBuildInfoCmd);
  install_tcl_soar_cmd( the_agent, "ex-build-info", SoarExcludedBuildInfoCmd);

#ifdef USE_CAPTURE_REPLAY
 install_tcl_soar_cmd(the_agent, "capture-input",             CaptureInputCmd);
 install_tcl_soar_cmd(the_agent, "replay-input",        ReplayInputCmd);
#endif

#ifdef KT_HISTOGRAM
 install_tcl_soar_cmd(the_agent, "init-kt",        initKTHistogramCmd);
#endif
#ifdef DC_HISTOGRAM
 install_tcl_soar_cmd(the_agent, "init-dc",        initDCHistogramCmd);
#endif

}
