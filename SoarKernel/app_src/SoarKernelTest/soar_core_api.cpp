typedef struct agent_struct agent;
/*************************************************************************
 *************************************************************************/
extern agent * glbAgent;
#define current_agent(x) (glbAgent->x)
/************************************************************************/
/**
 * \file soar_core_api.c
 *   
 * This file contains the low-level (core) interface to the Soar production
 * system. 
 * 
 *
 * Copyright (c) 1995-1999 Carnegie Mellon University,
 *                         University of Michigan,
 *                         University of Southern California/Information
 *                         Sciences Institute.  All rights reserved.
 *
 * The Soar consortium proclaims this software is in the public domain, and
 * is made available AS IS.  Carnegie Mellon University, The University of 
 * Michigan, and The University of Southern California/Information Sciences 
 * Institute make no warranties about the software or its performance,
 * implied or otherwise.  All rights reserved.
 *
 * $Id$
 *
 */

#include "soar_core_api.h"
#include "soar_ecore_api.h"
#include "soarapi.h"
#include "utilities.h"
#include "new_soar.h"
#include "definitions.h"

#include "io.h"
#include "wmem.h"
#include "decide.h"
#include "explain.h"
#include "init_soar.h"
#include "production.h"
#include "rete.h"
#include "print.h"
#include "recmem.h"
#include "symtab.h"
#include "gsysparam.h"
#include "gdatastructs.h"
#include "kernel_struct.h"

#include <string>
#include <stdlib.h>
#include <assert.h>

#ifndef CDECL
#  ifdef _WIN32
#     define CDECL __cdecl
#  else
#     define CDECL
#  endif
#endif

extern Kernel * SKT_kernel;

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
int soar_cRun( long n, Bool allAgents, enum go_type_enum type, 
	       enum soar_apiSlotType slot  ) {
  

  int levels_up;
  Symbol *attribute, *goal;


  if ( type  != GO_SLOT && slot != NO_SLOT ) 
    return -1;
  
  if ( n < -1 ) n = -1;
  

  switch( type ) {
  
  case GO_PHASE:
    run_for_n_phases ( glbAgent, n );
    break;
  case GO_ELABORATION:
    run_for_n_elaboration_cycles ( glbAgent, n );
    break;
  case GO_DECISION:
    run_for_n_decision_cycles ( glbAgent, n );
    break;
  case GO_STATE:
    run_for_n_selections_of_slot ( glbAgent, n, glbAgent->state_symbol);
    break;
  case GO_OPERATOR:
    run_for_n_selections_of_slot ( glbAgent, n, glbAgent->operator_symbol);
    break;

  case GO_OUTPUT:
    run_for_n_modifications_of_output ( glbAgent, n );
    break;

  case GO_SLOT:
    switch ( slot ) {

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

    } /* End of switch (slot) */
    
    goal = current_agent(bottom_goal);
    while ( goal && levels_up ) {
      goal = goal->id.higher_goal;
      levels_up--;
    }
    
    if ( !goal )  return -3;      
    
    run_all_agents( SKT_kernel, n, GO_SLOT, attribute, goal->id.level );

    break;

  } /* End of switch type */

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
void soar_cStopAllAgents( void ) {
    /* control_c_handler no longer exists. -AJC (8/6/02) */
	//control_c_handler( 0 );

	/* Its functionality is duplicated here: */
    cons * c;
    agent * the_agent;
  
    for(c = SKT_kernel->all_soar_agents; c != NIL; c = c->rest) {
      the_agent = ((agent *) c->first);
      the_agent->stop_soar = TRUE;
	}
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
void soar_cStopCurrentAgent( char *reason ) {
  current_agent(stop_soar) = TRUE;
  current_agent(reason_for_stopping) =  reason;

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

int soar_cDestroyAgentByName( char *name ) {
  cons *c;
  int name_count = 0;
  psoar_agent *delete_me = NULL;

    for(c = SKT_kernel->all_soar_agents; c != NIL; c = c->rest) 
    {
      if (string_match( name, ((agent *)c->first)->name)) 
	  {
	    name_count++;
	    delete_me = (psoar_agent *)c->first;
	  }
    }
    if ( name_count > 1 ) return -1;
    if ( name_count == 0 ) return -2;
    soar_cDestroyAgentByAddress( delete_me );

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

int soar_cDestroyAllAgentsWithName( char *name ) {
  cons *c;
  int count;

  count = 0;
  for(c = SKT_kernel->all_soar_agents; c != NIL; c = c->rest) 
    {
      if (string_match( name, ((agent *)c->first)->name)) 
	{
	  count++;
	  soar_cDestroyAgentByAddress( (psoar_agent)c->first );
	  /* 
	   * Rewind list, to ensure that we don't get messed up
	   * as a result of our surgery.  (This probably isnt necessary)
	   */
	  c = SKT_kernel->all_soar_agents;
	}
    }
  if ( count == 0 ) return -1;
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
void soar_cDestroyAgentByAddress (psoar_agent delete_agent)
{
  if ( soar_exists_global_callback( GLB_DESTROY_AGENT ) ) {
    soar_invoke_global_callbacks( delete_agent, (SOAR_CALLBACK_TYPE)(GLB_DESTROY_AGENT),
				  (soar_call_data) delete_agent );
  }
  
  // Destroying the soar agent using the appropriate kernel function
  destroy_soar_agent(SKT_kernel, reinterpret_cast<agent*> (delete_agent));

  // Making sure that glbAgent is currently valid
  if ( SKT_kernel && SKT_kernel->all_soar_agents ) {
     glbAgent = reinterpret_cast<agent*> (SKT_kernel->all_soar_agents->first);
  } else {
     glbAgent = 0;
  }
}

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

int soar_agent_ids[MAX_SIMULTANEOUS_AGENTS];
soar_global_callback_array soar_global_callbacks;

void soar_cInitializeSoar (void)
{
  init_soar(SKT_kernel);

  return;

  char buffer[1000];
  int i;

  /* SW 081299 */
  /* No assigned agent ids */
  for ( i = 0; i < MAX_SIMULTANEOUS_AGENTS; i++ ) 
    soar_agent_ids[i] = UNTOUCHED;

  //soar_init_global_callbacks();
  /* The following code was copied and pasted from 
     soar_init_global_callbacks(). */
  int gct;
  
  for (gct = 1; gct < NUMBER_OF_GLOBAL_CALLBACKS; gct++) {
    soar_global_callbacks[gct] = (list *)NIL;
  }

  if ( MICRO_VERSION_NUMBER > 0 ) {
    sprintf( buffer, 
	     "%d.%d.%d", 
	     MAJOR_VERSION_NUMBER, 
	     MINOR_VERSION_NUMBER, 
	     MICRO_VERSION_NUMBER);

  } else {
    sprintf(buffer,
	    "%d.%d",
	    MAJOR_VERSION_NUMBER, 
	    MINOR_VERSION_NUMBER);
  }
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
  sys_srandom( /*JNW:::!!!!time(NULL)*/ 1);

  /* This is deprecated. -AJC (8/6/02) */
  //setup_signal_handling();
    


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
int soar_cReInitSoar (void) {

  reinitialize_soar(SKT_kernel);

  return 0;

  /* kjh (CUSP-B4) begin */
  long cur_TRACE_CONTEXT_DECISIONS_SYSPARAM;
  long cur_TRACE_PHASES_SYSPARAM;
  long cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM;
  long cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM;
  long cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM;
  long cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM;
  long cur_TRACE_WM_CHANGES_SYSPARAM;
  /* kjh (CUSP-B4) end */

  current_agent(did_PE) = FALSE;    /* RCHONG:  10.11 */

  soar_invoke_callbacks(glbAgent, glbAgent, 
		       BEFORE_INIT_SOAR_CALLBACK,
		       (soar_call_data) NULL);		       

  /* kjh (CUSP-B4) begin */
  /* Stash trace state: */
  cur_TRACE_CONTEXT_DECISIONS_SYSPARAM        = 
    current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM];
  cur_TRACE_PHASES_SYSPARAM                   = 
    current_agent(sysparams)[TRACE_PHASES_SYSPARAM];
  cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM = 
    current_agent(sysparams)[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM];
  cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM    = 
    current_agent(sysparams)[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM];
  cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM   = 
    current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM];
  cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM      = 
    current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM];
  cur_TRACE_WM_CHANGES_SYSPARAM               = 
    current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM];

  /* Temporarily disable tracing: */
  set_sysparam(glbAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM,        FALSE);
  set_sysparam(glbAgent, TRACE_PHASES_SYSPARAM,                   FALSE);
  set_sysparam(glbAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
  set_sysparam(glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,    FALSE);
  set_sysparam(glbAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,   NONE_WME_TRACE);
  set_sysparam(glbAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM,      FALSE);
  set_sysparam(glbAgent, TRACE_WM_CHANGES_SYSPARAM,               FALSE);
  /* kjh (CUSP-B4) end */

  clear_goal_stack (glbAgent);

#ifndef SOAR_8_ONLY
  if (current_agent(operand2_mode) == TRUE) {
#endif

    /* Signal that everything should be retracted */
     current_agent(active_level) = 0; 

     current_agent(FIRING_TYPE) = IE_PRODS;
     do_preference_phase (glbAgent);   /* allow all i-instantiations to retract */

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
  do_preference_phase (glbAgent);   /* allow all instantiations to retract */
#endif

  reset_explain(glbAgent);
  reset_id_counters (glbAgent);
  reset_wme_timetags (glbAgent);
  reset_statistics (glbAgent);
  current_agent(system_halted) = FALSE;
  current_agent(go_number) = 1;
  current_agent(go_type) = GO_DECISION;

  /* kjh (CUSP-B4) begin */
  /* Restore trace state: */
  set_sysparam(glbAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM,
	       cur_TRACE_CONTEXT_DECISIONS_SYSPARAM);
  set_sysparam(glbAgent, TRACE_PHASES_SYSPARAM,
	       cur_TRACE_PHASES_SYSPARAM);
  set_sysparam(glbAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM,
	       cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM);
  set_sysparam(glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,
	       cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM);
  set_sysparam(glbAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,
	       cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM);
  set_sysparam(glbAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM,
	       cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM);
  set_sysparam(glbAgent, TRACE_WM_CHANGES_SYSPARAM,
               cur_TRACE_WM_CHANGES_SYSPARAM);
  /* kjh (CUSP-B4) end */


  soar_invoke_callbacks(glbAgent, glbAgent, 
		       AFTER_INIT_SOAR_CALLBACK,
		       (soar_call_data) NULL);

  current_agent(input_cycle_flag) = TRUE;  /* reinitialize flag  AGR REW1 */

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
    current_agent(warn_on_zero_timers)        = TRUE;
#endif


  return 0;
}


void soar_cSetLearning( int setting ) {
  
  switch( setting ) {
    
  case ON:
    set_sysparam (glbAgent, LEARNING_ON_SYSPARAM, TRUE); 
    set_sysparam (glbAgent, LEARNING_ONLY_SYSPARAM, FALSE);
    set_sysparam (glbAgent, LEARNING_EXCEPT_SYSPARAM, FALSE);
    break;
  case OFF:
    set_sysparam (glbAgent, LEARNING_ON_SYSPARAM, FALSE); 
    set_sysparam (glbAgent, LEARNING_ONLY_SYSPARAM, FALSE);
    set_sysparam (glbAgent, LEARNING_EXCEPT_SYSPARAM, FALSE);
    break;
  case ONLY:
    set_sysparam (glbAgent, LEARNING_ON_SYSPARAM, TRUE); 
    set_sysparam (glbAgent, LEARNING_ONLY_SYSPARAM, TRUE);
    set_sysparam (glbAgent, LEARNING_EXCEPT_SYSPARAM, FALSE);
    break;
  case EXCEPT:
    set_sysparam (glbAgent, LEARNING_ON_SYSPARAM, TRUE); 
    set_sysparam (glbAgent, LEARNING_ONLY_SYSPARAM, FALSE);
    set_sysparam (glbAgent, LEARNING_EXCEPT_SYSPARAM, TRUE);
    break;
  case ALL_LEVELS:
    set_sysparam (glbAgent, LEARNING_ALL_GOALS_SYSPARAM, TRUE);
    break;
  case BOTTOM_UP:
    set_sysparam (glbAgent, LEARNING_ALL_GOALS_SYSPARAM, FALSE);
    break;
   

  }

}

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
double soar_cDetermineTimerResolution( double *min, double *max) {

  double delta, max_delta, min_delta, min_nz_delta;
  float q;  
  int i,j, top;
#ifdef PII_TIMERS
  unsigned long long int start, end, total;
#else
  struct timeval start, end, total;
#endif
  
  
  top = ONE_MILLION;
  min_delta = ONE_MILLION;
  min_nz_delta = ONE_MILLION;
  max_delta = -1;
  reset_timer( &total );

  for( i = 0; i < ONE_MILLION; i = (i+1)*2 ) {
    reset_timer( &end );
    start_timer( &start );
    for( j = 0; j < i*top; j++ ) {
      q = j*i;
    }
    stop_timer( &start, &end );
    stop_timer( &start, &total );
    delta = timer_value( &end );
    
    if ( delta < min_delta ) min_delta = delta;
    if ( delta && delta < min_nz_delta ) min_nz_delta = delta;
    if ( delta > max_delta ) max_delta = delta;
    
    /* when we have gone through this loop for 2 seconds, stop */
    if ( timer_value( &total ) >= 2 ) { break; }

  }

  if ( min_nz_delta == ONE_MILLION ) min_nz_delta = -1;
  if ( min_delta == ONE_MILLION ) min_delta = -1;

  if ( min != NULL ) *min = min_delta;
  if ( max != NULL ) *max = max_delta;
  return min_nz_delta;

}
#endif  

void soar_cSetCurrentAgent( psoar_agent a ) {
  
  glbAgent = (agent *)a;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cLoadReteNet --
 *
 *     load a Rete Network into the agent from a specified file
 *
 *----------------------------------------------------------------------
 */
int soar_cLoadReteNet( char *filename ) {


  char pipe_command[]  = "zcat ";
  Bool using_compression_filter;
  char * append_loc, * command_line;
  FILE * f;
  Bool result;


  if ( !filename ) 
    return -1; 

  /* --- check for empty system --- */
  /*
    This is done inside the load_rete_net function 
    and is not necessary here.

    if (current_agent(all_wmes_in_rete)) 
    return -2;


    for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if (current_agent(num_productions_of_type)[i]) 
    return -3;
  */
    
 

#if !defined(MACINTOSH) /* Mac doesn't have pipes */
  if (   (!(strcmp((char *) (filename + strlen(filename) - 2), ".Z")))
      || (!(strcmp((char *) (filename + strlen(filename) - 2), ".z"))))
    {

      /* The popen can succeed if given an non-existant file   */

      /* creating an unusable pipe.  So we check to see if the */
      /* file exists first, on a load action.                  */
      if (!(f = fopen(filename, "rb"))) {

	/* --- error when opening the file --- */    
	return -4;
      }
      else {
	fclose(f);
      }
	

      command_line = static_cast<char *>(allocate_memory( glbAgent, strlen( pipe_command ) + 
				      strlen( filename ) + 1,
				      STRING_MEM_USAGE ));
      
      strcpy( command_line, pipe_command );
      
      append_loc = command_line;
      while( *append_loc ) append_loc++;
      
      strcpy( append_loc, filename );

      f = (FILE *) popen( command_line, "rb");
      free_memory( glbAgent, command_line, STRING_MEM_USAGE );
      
      using_compression_filter = TRUE;
    }
  else
#endif /* !MACINTOSH */
    {

      f = fopen( filename, "rb");
      using_compression_filter = FALSE;
    }

  if (! f) 
    {
      /* --- error when opening the pipe or file --- */
      return -4;
    }

  result = load_rete_net(SKT_kernel, glbAgent, f);
  if (!result) return -2;

#if !defined(MACINTOSH)
  if (using_compression_filter == TRUE)
    {
      pclose(f);
    }
  else
#endif /* !MACINTOSH */
    {
      fclose(f);
    }

  return 0;
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

int soar_cSaveReteNet( char *filename ) {
  
  char *command_line;
  char pipe_command[]  = "compress > ";
  FILE *f;
  Bool using_compression_filter = FALSE;
  char *append_loc;
  
  if (current_agent(all_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE])
    return -1;


#if !defined(MACINTOSH) /* Mac doesn't have pipes */
  if (   (!(strcmp((char *) (filename + strlen(filename) - 2), ".Z")))
      || (!(strcmp((char *) (filename + strlen(filename) - 2), ".z"))))
    {

      command_line = static_cast<char *>(allocate_memory( glbAgent, strlen( pipe_command ) + 
				      strlen( filename ) + 1,
				      STRING_MEM_USAGE ));
      
      strcpy( command_line, pipe_command );
      
      append_loc = command_line;
      while( *append_loc ) append_loc++;
      
      strcpy( append_loc, filename );

      f = (FILE *) popen( command_line , "wb");
      free_memory( glbAgent, command_line, STRING_MEM_USAGE );
      
      using_compression_filter = TRUE;

    }
  else
#endif /* !MACINTOSH */
    {

      f = fopen( filename, "wb");
      using_compression_filter = FALSE;
    }

  if (! f) 
    {
      /* --- error when opening the pipe or file --- */
      return -2;
    }

  save_rete_net(glbAgent, f);

#if !defined(MACINTOSH)
  if (using_compression_filter == TRUE)
    {
      pclose(f);
    }
  else
#endif /* !MACINTOSH */
    {
      fclose(f);
    }

  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cAddInputFunction
 *
 *     Installs a function which will provide input to the agent
 *
 *----------------------------------------------------------------------
 */
void soar_cAddInputFunction (agent * a, soar_callback_fn f, 
			     soar_callback_data cb_data, 
			     soar_callback_free_fn free_fn, char * name) {
  soar_cAddCallback(a, INPUT_PHASE_CALLBACK, f, cb_data, free_fn, name);
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
int soar_cAddWme( char *szId, char *szAttr, char *szValue, 
			    Bool acceptable_preference, psoar_wme *new_wme ) {

  Symbol *id, *attr, *value;
  wme *w;


  /* --- get id --- */
  if (read_id_or_context_var_from_string( szId, &id) == SOAR_ERROR)
	  return -1;
  
  
  /* --- get optional '^', if present --- */
  
  if ( *szAttr == '^' ) szAttr++;
  
  
  /* --- get attribute or '*' --- */
  if ( string_match("*", szAttr) == TRUE )
  {
#ifdef USE_AGENT_DBG_FILE
      fprintf( current_agent(dbgFile), "'%s' matches '*'\n", szAttr );
#endif
	  
      attr = make_new_identifier (glbAgent, 'I', id->id.level);
  } 
  else 
  {
      Bool rv = get_lexeme_from_string(szAttr);
	  if(!rv)
	  {
		  return -2;
	  }
	  
      switch (current_agent(lexeme).type) 
	  {
	  case SYM_CONSTANT_LEXEME:
		  attr = make_sym_constant (glbAgent, current_agent(lexeme).string); 
		  break;
	  case INT_CONSTANT_LEXEME:
		  attr = make_int_constant (glbAgent, current_agent(lexeme).int_val); 
		  break;
	  case FLOAT_CONSTANT_LEXEME:
		  attr = make_float_constant (glbAgent, current_agent(lexeme).float_val); 
		  break;
	  case IDENTIFIER_LEXEME:
	  case VARIABLE_LEXEME:      
		  attr = read_identifier_or_context_variable();
		  if (!attr) 
		  {
			  return -2;
		  }
		  symbol_add_ref (attr);
		  break;
	  default:
		  return -2;
	  }
    }

  /* --- get value or '*' --- */
  
  if (string_match("*", szValue) == TRUE )
  {
      value = make_new_identifier (glbAgent, 'I', id->id.level);
  } 
  else 
  {
      Bool rv = get_lexeme_from_string(szValue);
	  if(!rv)
	  {
		  return -3;
	  }
	  
      switch (current_agent(lexeme).type) 
	  {
	  case SYM_CONSTANT_LEXEME:
		  value = make_sym_constant (glbAgent, current_agent(lexeme).string); 
		  break;
	  case INT_CONSTANT_LEXEME:
		  value = make_int_constant (glbAgent, current_agent(lexeme).int_val); 
		  break;
	  case FLOAT_CONSTANT_LEXEME:
		  value = make_float_constant (glbAgent, current_agent(lexeme).float_val); 
		  break;
	  case IDENTIFIER_LEXEME:
	  case VARIABLE_LEXEME:
		  value = read_identifier_or_context_variable();
		  if (!value) 
		  { 
			  symbol_remove_ref (glbAgent, attr); 
			  return -3; 
		  }
		  symbol_add_ref (value);
		  break;
	  default:
		  symbol_remove_ref (glbAgent, attr);
		  return -3;
	  }
  }


  /* --- now create and add the wme --- */
  w = make_wme (glbAgent, id, attr, value, acceptable_preference);


  
  symbol_remove_ref (glbAgent, w->attr);
  symbol_remove_ref (glbAgent, w->value);
  insert_at_head_of_dll (w->id->id.input_wmes, w, next, prev);
  add_wme_to_wm (glbAgent, w);

#ifdef USE_CAPTURE_REPLAY
  /* KJC 11/99 begin: */
  /* if input capturing is enabled, save any input wmes to capture file */
  if (current_agent(capture_fileID) && 
      (current_agent(current_phase) == INPUT_PHASE)) {
    
    soarapi_wme sapi_wme;

    /* Dont copy, since capture_input_wme is just going to print
     * the contents of the structure into a file... 
     */
    sapi_wme.id = szId;
    sapi_wme.attr = szAttr;
    sapi_wme.value = szValue;
    sapi_wme.timetag = w->timetag;

    capture_input_wme( ADD_WME, &sapi_wme, w );
  }
  /* KJC 11/99 end */

#endif /* USE_CAPTURE_REPLAY */


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
  if ( current_agent(current_phase) == INPUT_PHASE ) {
    /* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
       stop_timer (&current_agent(start_kernel_tv), 
                   &current_agent(input_function_cpu_time));
       start_timer (&current_agent(start_kernel_tv));

#ifndef KERNEL_TIME_ONLY
       start_timer (&current_agent(start_phase_tv)); 
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
       do_buffered_wm_and_ownership_changes(glbAgent); 
#endif


/* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
       if (current_agent(current_phase) == INPUT_PHASE) {

#ifndef KERNEL_TIME_ONLY
	 stop_timer (&current_agent(start_phase_tv), 
		     &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif
	 stop_timer (&current_agent(start_kernel_tv), 
		     &current_agent(total_kernel_time));
	 start_timer (&current_agent(start_kernel_tv));
       }
#endif
/* REW: end 28.07.96 */
/* #endif */

  *new_wme = (psoar_wme)w;
  return w->timetag;

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
int soar_cRemoveWme( psoar_wme the_wme ) {

  wme *w, *w2;
  Symbol *id;
  slot *s;


  w = (wme *)the_wme;

  id = w->id;
  
  /* --- remove w from whatever list of wmes it's on --- */
  for (w2=id->id.input_wmes; w2!=NIL; w2=w2->next)
    if (w==w2) break;
  
  if (w2) remove_from_dll (id->id.input_wmes, w, next, prev);
  
  for (w2=id->id.impasse_wmes; w2!=NIL; w2=w2->next)
    if (w==w2) break;
    
  if (w2) remove_from_dll (id->id.impasse_wmes, w, next, prev);
  
  for (s=id->id.slots; s!=NIL; s=s->next) {
    
    for (w2=s->wmes; w2!=NIL; w2=w2->next)
      if (w==w2) break;
    
    if (w2) remove_from_dll (s->wmes, w, next, prev);    
	
    for (w2=s->acceptable_preference_wmes; w2!=NIL; w2=w2->next)
      if (w==w2) break;
	
    if (w2) remove_from_dll (s->acceptable_preference_wmes, w, next, prev);
  }


#ifdef USE_CAPTURE_REPLAY
  
    /* KJC 11/99 begin: */
    /* if input capturing is enabled, save any input changes to capture file */
    if (current_agent(capture_fileID) && 
	(current_agent(current_phase) == INPUT_PHASE)) {
      soarapi_wme sapi_wme;

      sapi_wme.id = NULL;
      sapi_wme.attr = NULL;
      sapi_wme.value = NULL;
      sapi_wme.timetag = w->timetag;

      capture_input_wme( REMOVE_WME, &sapi_wme, NULL );
    }
    /* KJC 11/99 end */
#endif /* USE_CAPTURE_REPLAY */

  
  /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
  if (current_agent(operand2_mode)){
#endif
    if (w->gds) {
      if (w->gds->goal != NIL){



	gds_invalid_so_remove_goal(glbAgent, w);

	/* NOTE: the call to remove_wme_from_wm will take care of checking if
	   GDS should be removed */
      }
    }
#ifndef SOAR_8_ONLY
  }
#endif
  

  /* REW: end   09.15.96 */
  
  /* --- now remove w from working memory --- */
  remove_wme_from_wm (glbAgent, w);
  
  /* REW: begin 28.07.96 */
  /* See AddWme for description of what's going on here */

     if (current_agent(current_phase) != INPUT_PHASE) {
#ifndef NO_TIMING_STUFF
       start_timer (&current_agent(start_kernel_tv));
#ifndef KERNEL_TIME_ONLY
       start_timer (&current_agent(start_phase_tv)); 
#endif
#endif  
       
       /* do_buffered_wm_and_ownership_changes(); */
	

#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
       stop_timer (&current_agent(start_phase_tv), 
		   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif
       stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
       start_timer (&current_agent(start_kernel_tv));
#endif
     }
  /* note: 
   *  See note at the NO_TOP_LEVEL_REFS flag in soar_cAddWme
   */
#ifndef NO_TOP_LEVEL_REFS
      do_buffered_wm_and_ownership_changes(glbAgent);  
#endif

     return 0;
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
int soar_cRemoveWmeUsingTimetag( int num ) {

  wme *w;
      
  for (w=current_agent(all_wmes_in_rete); w!=NIL; w=w->rete_next)
    if (w->timetag == (unsigned long) num) 
      break;
  
  if (!w) 
    return -1;
  
  if ( !soar_cRemoveWme( w ) )
    return 0;
  
  return -2; /* Unspecified Failure */
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
void soar_cRemoveInputFunction (agent * a, char * name) {
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
void soar_cAddOutputFunction (agent * a, soar_callback_fn f, 
			  soar_callback_data cb_data, 
			  soar_callback_free_fn free_fn,
			  char * output_link_name)
{
  if (soar_exists_callback_id (a, OUTPUT_PHASE_CALLBACK, output_link_name)
      != NULL)
    {
      print (glbAgent, "Error: tried to add_output_function with duplicate name %s\n",
             output_link_name);
      /* Replaced deprecated control_c_handler with an appropriate assertion */
	  //control_c_handler(0);
	  assert(0 && "error in lexer.cpp (control_c_handler() used to be called here)");
    }
  else
    {
      soar_cAddCallback(a, OUTPUT_PHASE_CALLBACK, f, cb_data, free_fn, 
			output_link_name);
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
void soar_cRemoveOutputFunction (agent * a, char * name) {
  soar_callback * cb;
  output_link *ol;

  /* Remove indexing structures ... */

  cb = soar_exists_callback_id(a, OUTPUT_PHASE_CALLBACK, name);
  if (!cb) return;

  for (ol=a->existing_output_links; ol!=NIL; ol=ol->next) 
    {
      if (ol->cb == cb)
	{
	  /* Remove ol entry */
	  ol->link_wme->output_link = NULL;
	  wme_remove_ref(glbAgent, ol->link_wme);
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
 * soar_cAddCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cAddCallback (soar_callback_agent the_agent, 
		 	 SOAR_CALLBACK_TYPE callback_type, 
			 soar_callback_fn fn, 
			 soar_callback_data data,
			 soar_callback_free_fn free_fn,
			 soar_callback_id id)
{
  soar_callback * cb;

  cb = (soar_callback *) malloc (sizeof(soar_callback));
  cb->function      = fn;
  cb->data          = data;
  cb->free_function = free_fn;
  cb->id            = savestring((char *)id);
  

  push(glbAgent, cb, ((agent *) the_agent)->soar_callbacks[callback_type]);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cRemoveCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cRemoveCallback (soar_callback_agent the_agent, 
			   SOAR_CALLBACK_TYPE callback_type, 
			   soar_callback_id id)
{
  cons * c;
  cons * prev_c = NULL;     /* Initialized to placate gcc -Wall */
  list * head;

  head = ((agent *)the_agent)->soar_callbacks[callback_type];

  for (c = head; c != NIL; c = c->rest)
    {
      soar_callback * cb;

      cb = (soar_callback *) c->first;

      if (!strcmp(cb->id, id))
	{
	  if (c != head)
	    {
	      prev_c->rest = c->rest;
	      soar_destroy_callback(cb);
	      free_cons(glbAgent, c);
	      return;
	    }
	  else
	    {
	      ((agent *)the_agent)->soar_callbacks[callback_type] 
		= head->rest;
	      soar_destroy_callback(cb);
	      free_cons(glbAgent, c);
	      return;
	    }
	}
      prev_c = c;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cPushCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cPushCallback (soar_callback_agent the_agent, 
		 	 SOAR_CALLBACK_TYPE callback_type, 
			 soar_callback_fn fn, 
			 soar_callback_data data,
			 soar_callback_free_fn free_fn)
{
  soar_callback * cb;

  cb = (soar_callback *) malloc (sizeof(soar_callback));
  cb->function      = fn;
  cb->data          = data;
  cb->free_function = free_fn;
  cb->id            = NULL;
  
/*
  printf( "Pushing callback function %p onto callback slot %d\n", 
	fn, callback_type );
  fflush( stdout );
*/

  push(glbAgent, cb, ((agent *) the_agent)->soar_callbacks[callback_type]);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cPopCallback
 *
 *
 *----------------------------------------------------------------------
 */
void soar_cPopCallback (soar_callback_agent the_agent, 
			SOAR_CALLBACK_TYPE callback_type)
{
  list * head;
  soar_callback * cb;

  head = ((agent *)the_agent)->soar_callbacks[callback_type];
  
  if (head == NULL)
    {
      print_string(glbAgent, "Attempt to remove non-existant callback.\n");
      return;
    }

  if (   (callback_type == PRINT_CALLBACK)
      && (head->rest == NULL))
    {
      print_string(glbAgent, "Attempt to remove last print callback. Ignored.\n");
      return;
    }

  cb = (soar_callback *) head->first;

  ((agent *)the_agent)->soar_callbacks[callback_type] = head->rest;
  soar_destroy_callback(cb);
  free_cons(glbAgent, head);
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
void soar_cQuit ( void )
{

  /* If there aren't any agents, then there's nothing to do */
  if ( !glbAgent ) return;

  just_before_exit_soar(glbAgent);


  /* Soar-Bugs #58, TMH */
  while (soar_exists_callback(glbAgent,
			      LOG_CALLBACK))
  {
    
    soar_invoke_first_callback(glbAgent, glbAgent,
				LOG_CALLBACK,
				(void*)("\n**** quit cmd issued ****\n"));
    soar_cPopCallback(glbAgent, LOG_CALLBACK);
  }
#ifdef USE_AGENT_DBG_FILE
  fclose( current_agent(dbgFile) );
#endif

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
int soar_cDestroyAgentById( int agent_id ) {
  cons *c;


    for(c = SKT_kernel->all_soar_agents; c != NIL; c = c->rest)
    {
      ////if ( agent_id == ((agent *)c->first)->id )
      ////{
    	////soar_cDestroyAgentByAddress( (psoar_agent)c->first );
    	////return 0;
      ////}
    }
    

    /* Didn't find agent id */
    return -1;      
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
int soar_cExciseProductionByName ( char *name ) {
  production *p;
  
  if (  (p = name_to_production( name ))  ) {
    excise_production( glbAgent, p, (TRUE &&
			   current_agent(sysparams)[TRACE_LOADING_SYSPARAM]));

    return 0;
  }

  return -1;       /* production not found */
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
void soar_cExciseAllProductions (void) {
    
  soar_cExciseAllProductionsOfType( DEFAULT_PRODUCTION_TYPE );
  soar_cExciseAllProductionsOfType( CHUNK_PRODUCTION_TYPE );
  soar_cExciseAllProductionsOfType( JUSTIFICATION_PRODUCTION_TYPE );
  soar_cExciseAllProductionsOfType( USER_PRODUCTION_TYPE );
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

void soar_cExciseAllTaskProductions (void) {
  soar_cExciseAllProductionsOfType( CHUNK_PRODUCTION_TYPE );
  soar_cExciseAllProductionsOfType( JUSTIFICATION_PRODUCTION_TYPE );
  soar_cExciseAllProductionsOfType( USER_PRODUCTION_TYPE );
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

void soar_cExciseAllProductionsOfType ( byte type ) {
  while (current_agent(all_productions_of_type)[type])
    excise_production (glbAgent, current_agent(all_productions_of_type)[type],
		    (TRUE&&current_agent(sysparams)[TRACE_LOADING_SYSPARAM]));

}

char *soar_cGetAgentOutputLinkId( psoar_agent a, char *buff ) {
  char *temp;
  char *ret;


  if ( ((agent *)a)->io_header_output == NULL ) {
	if ( buff ) *buff = '\0';
	return "";
  }

  temp = symbol_to_string( glbAgent, ((agent *)a)->io_header_output, TRUE, buff );
  if ( buff ) return buff;

  ret = (char *) malloc( (strlen(temp) + 1) * sizeof(char) );
  strcpy( ret, temp );

  return ret;  
}

psoar_agent soar_cGetAgentByName( char *name ) {
  cons *c;

  for( c = SKT_kernel->all_soar_agents; c != NIL; c = c->rest ) {
    if ( !strcmp( ((agent *)c->first)->name, name ) ) {
      return (psoar_agent)c->first;
    }
  }
  return NIL;

  
}

/*
 *----------------------------------------------------------------------
 *
 * soar_cSetOperand2
 *
 *----------------------------------------------------------------------
 */
int soar_cSetOperand2( Bool turnOn ) {
  int i;
  
  
  /* --- check for empty system --- */
  if (current_agent(all_wmes_in_rete)) {
    return -1;
  }
  for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if (current_agent(num_productions_of_type)[i])  {
      return -2;
    }

  
  current_agent(operand2_mode) = turnOn;
  soar_cReInitSoar();
  
  return 0;
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
int soar_cMultiAttributes( char *attribute, int value ){
  multi_attribute *m;
  Symbol *s;

  get_lexeme_from_string( attribute );
  
  if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME) {
    return -1;
  }
  if ( value < 1 ) {
    return -2;
  }
  
  m = current_agent(multi_attributes);
  s = make_sym_constant(glbAgent, attribute);


  while(m) {
    if(m->symbol == s) {
      m->value = value;
      symbol_remove_ref(glbAgent, s);
      return 0;
    }
    m = m->next;
  }
  /* sym wasn't in the table if we get here, so add it */
  m = (multi_attribute *)allocate_memory(glbAgent, sizeof(multi_attribute),
                                         MISCELLANEOUS_MEM_USAGE);
  m->value = value;
  m->symbol = s;
  m->next = current_agent(multi_attributes);
  current_agent(multi_attributes) = m;

  return 0;
}

extern "C" void CDECL compress_print( soar_callback_agent a, 
				      soar_callback_data cb_data, 
				      soar_call_data call_data);
/*
 *----------------------------------------------------------------------
 *
 * soar_cCreateAgent --
 *
 *      Create a new soar agent with the specified name.
 *
 *----------------------------------------------------------------------
 */

void soar_cCreateAgent (const char * agent_name, bool OperandSupportMode2) {

  if (soar_exists_global_callback( GLB_CREATE_AGENT )) 
  {
    soar_invoke_global_callbacks( NULL, (SOAR_CALLBACK_TYPE)(GLB_CREATE_AGENT), 
				                      (soar_call_data)agent_name);
  }
  else {

    glbAgent = create_soar_agent( SKT_kernel, const_cast<char *>(agent_name) );
    //set_sysparam(soar_agent, TRACE_WM_CHANGES_SYSPARAM, TRUE);
    soar_add_callback(glbAgent, glbAgent, PRINT_CALLBACK, compress_print, NULL, NULL, "compress_print"); /* Again, is this right? */

    soar_cSetOperand2(OperandSupportMode2);
	
    int argc=0;
    char *argv[16];
    soarResult res;
    
    // Setting the watch level in a round about way using an argc argv type function
    //std::string watch_string("watch decisions");
    std::string watch_string("watch decisions phases productions wmes preferences -fullwmes -timetags learning -fullprint loading -on");
    watch_string.append("\0");
    
    
    unsigned int cur_loc=0;
    Bool done=false;
    for(;;){
      argv[argc++] = &(watch_string[cur_loc]);
      cur_loc = watch_string.find_first_of(" ");
      if(cur_loc == std::string::npos){ break; }
      watch_string[cur_loc] = 0;
      cur_loc++;
    }
	
    soar_Watch(argc, argv, &res);

    // Now with the print callback and watch levels appropriately set up, begin initializing the agent
    initialize_soar_agent( SKT_kernel, glbAgent);
  }
}
