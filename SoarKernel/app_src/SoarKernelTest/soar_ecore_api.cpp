typedef struct agent_struct agent;
/*************************************************************************
 *************************************************************************/
extern agent * glbAgent;
#define current_agent(x) (glbAgent->x)
/************************************************************************/

#include "soar_ecore_api.h"
#include "soarapi.h"
#include "sk.h"
#include "utilities.h"
#include "new_soar.h"
#include "definitions.h"

/* Need to include time.h on linux */
#ifndef _WIN32
#include <time.h>
#endif

#include "explain.h"
#include "init_soar.h"
#include "production.h"
#include "gdatastructs.h"
#include "rete.h"
#include "wmem.h"
#include "print.h"
#include "parser.h"
#include "gsysparam.h"

extern Kernel* SKT_kernel;

/*
 *----------------------------------------------------------------------
 *
 * soar_ecCaptureInput
 *
 *       Captures the input sent to the agent during by external
 *       calls to add-wme and remove-wme
 *----------------------------------------------------------------------
 */

int soar_ecCaptureInput( char *filename ) { 
#ifdef USE_CAPTURE_REPLAY
  FILE *f;
  
  if ( filename == NIL ) {
    /* Trying to close */
    if ( !current_agent(capture_fileID) ) {
      return -1;
    }
    fclose(current_agent(capture_fileID));
    current_agent(capture_fileID) = NIL;
    return 0;
  }
  else {
    if ( current_agent(capture_fileID) ) {
      return -2;
    }
    if ( !(f = fopen(filename, "w" ) )) {
      return -3;
    }
    else {
      current_agent(capture_fileID) = f;
      fprintf(f, "##soar captured input file.\n");
      return 0;
    }
  }
#else
  return 0;
#endif
}


int soar_ecSp ( char *rule, char *sourceFile ) {

  production *p;

  /* Use a callback instead? */
  /* ((agent *)clientData)->alternate_input_string = argv[1];
   * ((agent *)clientData)->alternate_input_suffix = ") ";
   */  /* Soar-Bugs #54 TMH */
  soar_alternate_input( glbAgent, rule, ") ", TRUE); 
  set_lexer_allow_ids (glbAgent, FALSE);
  get_lexeme(glbAgent);
  p = parse_production(glbAgent);
  set_lexer_allow_ids (glbAgent, TRUE);
  soar_alternate_input( glbAgent, NIL, NIL, FALSE);

  if (p) {
    
    if ( sourceFile != NULL ) {
      p->filename = make_memory_block_for_string( glbAgent, sourceFile );
    }
    
    
    if (current_agent(sysparams)[TRACE_LOADING_SYSPARAM]) print(glbAgent, "*");
    
    /* kjh CUSP(B14) begin */
    if (p->type == CHUNK_PRODUCTION_TYPE) {

      /* Extract chunk_count from chunk name.
       * It will always be the number before the first '*' or '\0' 
       * and after a '-'
       * (e.g. chunk-14*d8*tie*2 or chunk-123).
       * This hack eliminates long delays that would arise when generating
       *  new chunk names after having loaded in old chunks.  By extracting
       *  chunk_count here, the time spent looping and checking for pre-
       *  existing names is minimized.
       */
      char *chunk_name, *c;
      unsigned long this_chunk_count;
      
      chunk_name = p->name->sc.name;
      c = chunk_name;
      while (*c && (*c != '*'))
	c++;
      do 
	c--;
      while (*c && (*c != '-'));
      c++;
      if (sscanf(c,"%lu",&this_chunk_count) != 1)
	print(glbAgent, "Warning: failed to extract chunk_num from chunk \"%s\"\n",
	      chunk_name);
      else if (this_chunk_count > current_agent(chunk_count)) {
	current_agent(chunk_count) = this_chunk_count;
	print(glbAgent, "updated chunk_num=%lu\n",current_agent(chunk_count));
      }
    }
    /* kjh CUSP(B14) end */  
    
    return 0;    /* OK */
  }
  else {
    /* DJP : Modified to make all sp errors appear to be non-fatal */
    /*       This is necessary because currently warnings are causing */
    /*       Soar to stop loading files, which is clearly wrong.  */
    /*       E.g. ignoring production P1 because it's a duplicate of P2 */
    
    return -1;  /* Non Fatal Error */
    
      /* THIS MAY NEED A BETTER SOLUTION ??? -- KJC */
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_ecWatchLevel
 *
 *     Set the current Watch level, the higher the value, the 
 *     greater the verbosity.
 *
 *----------------------------------------------------------------------
 */
int soar_ecWatchLevel( int level ) {

  if ( level > 5 || level < 0 )
    return -1;

  
  set_sysparam(glbAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM,         FALSE);
  set_sysparam(glbAgent, TRACE_PHASES_SYSPARAM,                    FALSE);
  set_sysparam(glbAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM,  FALSE);
  set_sysparam(glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,     FALSE);
  set_sysparam(glbAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM,         FALSE);
  set_sysparam(glbAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, FALSE);
  set_sysparam(glbAgent, TRACE_WM_CHANGES_SYSPARAM,                FALSE);
  set_sysparam(glbAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM,       FALSE);
  set_sysparam(glbAgent, TRACE_OPERAND2_REMOVALS_SYSPARAM,         FALSE);
  
  switch (level) {
  case 0:
    /* make sure everything is off */
    set_sysparam(glbAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
    set_sysparam(glbAgent, TRACE_CHUNK_NAMES_SYSPARAM,            FALSE);
    set_sysparam(glbAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM,    FALSE);
    set_sysparam(glbAgent, TRACE_CHUNKS_SYSPARAM,                 FALSE);
    set_sysparam(glbAgent, TRACE_JUSTIFICATIONS_SYSPARAM,         FALSE);
    set_sysparam(glbAgent, TRACE_OPERAND2_REMOVALS_SYSPARAM,      FALSE);
    break;

  case 5:
    
    set_sysparam(glbAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM,      TRUE);

  case 4:

    set_sysparam(glbAgent, TRACE_WM_CHANGES_SYSPARAM,               TRUE);

  case 3:

    set_sysparam(glbAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM,  TRUE);
    set_sysparam(glbAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,     TRUE);
    set_sysparam(glbAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM,         TRUE);
    set_sysparam(glbAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, TRUE);

  case 2:

    set_sysparam(glbAgent, TRACE_PHASES_SYSPARAM,                   TRUE);

  case 1:

    set_sysparam(glbAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM,        TRUE);
  }
  
  return 0;
}

int soar_ecAddWmeFilter( char *szId, char *szAttr, char *szValue, 
			 Bool adds, Bool removes ) {

  Symbol *id,*attr,*value;
  wme_filter *wf, *existing_wf;
  cons *c;
  int return_value;


  id = NIL;
  attr = NIL;
  value = NIL;
  return_value = 0;

  if ( read_wme_filter_component(glbAgent, szId,&id) ) {
    return_value = -1;
    goto error_out;
  }
  if ( read_wme_filter_component(glbAgent, szAttr,&attr) ) {
    return_value = -2;
    goto error_out;
  }
  if ( read_wme_filter_component(glbAgent, szValue,&value) ) {
    return_value = -3;
    goto error_out;
  }

  if (id && attr && value) {
    /* check to see if such a filter has already been added: */
    for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
      existing_wf = (wme_filter *) c->first;
      if (  (existing_wf->adds == adds) && (existing_wf->removes == removes)
	    && (existing_wf->id == id) && (existing_wf->attr == attr) 
	    && (existing_wf->value == value)) {

	print(glbAgent, "Filter already exists.\n");
	return_value = -4;
	goto error_out;	/* already exists */
      }  
    }
    
    wf = static_cast<wme_filter *>(allocate_memory (glbAgent, sizeof(wme_filter), MISCELLANEOUS_MEM_USAGE));
    wf->id = id;
    wf->attr = attr;
    wf->value = value;
    wf->adds = adds;
    wf->removes  = removes;
    
    /* Rather than add refs for the new filter symbols and then remove refs 
     * for the identical symbols created from the string parameters, skip
     * the two nullifying steps altogether and just return immediately
     * after pushing the new filter:
     */
    push(glbAgent, wf,current_agent(wme_filter_list));	/* note: nested macro */
    return 0;
  }
error_out:
  /* clean up symbols created from string parameters */
  if (id) symbol_remove_ref(glbAgent, id);
  if (attr) symbol_remove_ref(glbAgent, attr);
  if (value) symbol_remove_ref(glbAgent, value);
  return return_value;
}

int soar_ecRemoveWmeFilter( char *szId, char *szAttr, char *szValue, 
			    Bool adds, Bool removes ) {
  Symbol *id,*attr,*value;
  wme_filter *wf;
  int return_value = -4;
  cons *c;
  cons **prev_cons_rest;
  
  id = NIL;
  attr = NIL;
  value = NIL;
  
  if ( read_wme_filter_component(glbAgent, szId,&id)  ) {
    return_value = -1;
    goto clean_up;
  }
  if ( read_wme_filter_component(glbAgent, szAttr,&attr) ) {
    return_value = -2;
    goto clean_up;
  }
  if ( read_wme_filter_component(glbAgent, szValue,&value) ) {
    return_value = -3;
    goto clean_up;
  }


  if (id && attr && value) {
    prev_cons_rest = &current_agent(wme_filter_list);
    for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
      wf = (wme_filter *) c->first;
      if (  ((adds && wf->adds) || ((removes) && wf->removes))
	    && (wf->id == id) && (wf->attr == attr) && (wf->value == value)) {
        *prev_cons_rest = c->rest;
        symbol_remove_ref(glbAgent, id);
        symbol_remove_ref(glbAgent, attr);
        symbol_remove_ref(glbAgent, value);
        free_memory (glbAgent, wf, MISCELLANEOUS_MEM_USAGE);
        free_cons(glbAgent, c);
        break;	/* assume that soar_ecAddWmeFilter did not add duplicates */
      }
      prev_cons_rest = &(c->rest);
    }
    if (c != NIL)
      return_value = 0; /* filter was sucessfully removed */
  }
  
clean_up:
  /* clean up symbols created from string parameters */
  if (id) symbol_remove_ref(glbAgent, id);
  if (attr) symbol_remove_ref(glbAgent, attr);
  if (value) symbol_remove_ref(glbAgent, value);
  return return_value;
}

#pragma warning (disable:4129)
int soar_ecResetWmeFilters( Bool adds, Bool removes) {
  wme_filter *wf;
  cons *c;
  cons **prev_cons_rest;
  Bool didRemoveSome;
  
  didRemoveSome = FALSE;
  prev_cons_rest = &current_agent(wme_filter_list);
  for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
    wf = (wme_filter *) c->first;
    if ((adds && wf->adds) || (removes && wf->removes)) {
      *prev_cons_rest = c->rest;
      print_with_symbols(glbAgent, "Removed: (%y ^%y %y) ",wf->id,wf->attr,wf->value);
      print(glbAgent, "%s %s\n", (wf->adds ? "adds" : ""),(wf->removes ? "removes":""));
      symbol_remove_ref(glbAgent, wf->id);
      symbol_remove_ref(glbAgent, wf->attr);
      symbol_remove_ref(glbAgent, wf->value);
      free_memory (glbAgent, wf, MISCELLANEOUS_MEM_USAGE);
      free_cons(glbAgent, c);
      didRemoveSome = TRUE;
    }
    prev_cons_rest = &(c->rest);
  }
  if (didRemoveSome)
    return 0;
  else
    return -1;
}

void soar_ecListWmeFilters( Bool adds, Bool removes) {
  wme_filter *wf;
  cons *c;
  
  for (c=current_agent(wme_filter_list); c!=NIL; c=c->rest) {
    wf = (wme_filter *) c->first;
    if ((adds && wf->adds) || (removes && wf->removes)) {
      print_with_symbols(glbAgent, "wme filter: (%y ^%y %y) ", wf->id, 
			 wf->attr, wf->value);

      print(glbAgent, "%s %s\n", (wf->adds ? "adds" : ""),(wf->removes ? "removes":""));
    }
  }
}

void soar_ecPrintSystemStatistics( void ) {

  unsigned long wme_changes;

  /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
  double total_kernel_time, total_kernel_msec, derived_kernel_time, 
    monitors_sum,
    input_function_time,  input_phase_total_time,
    output_function_time, output_phase_total_time,
    determine_level_phase_total_time,  /* REW: end   05.05.97 */
    preference_phase_total_time, wm_phase_total_time, 
    decision_phase_total_time, derived_total_cpu_time;


#ifdef DETAILED_TIMING_STATS
  double match_time, match_msec;
  double ownership_time, chunking_time;
  double other_phase_kernel_time[6], other_total_kernel_time;
#endif
#endif
  /* REW: end 28.07.96 */

  /* MVP 6-8-94 */
  char hostname[MAX_LEXEME_LENGTH+1];
  long current_time;  



  if (sys_gethostname (hostname, MAX_LEXEME_LENGTH)) 
    strcpy (hostname, "[host name unknown]");

  current_time = time(NULL);


/* REW: begin 28.07.96 */   
/* See note in soarkernel.h for a description of the timers */
#ifndef NO_TIMING_STUFF
  total_kernel_time = timer_value (&current_agent(total_kernel_time));
  total_kernel_msec = total_kernel_time * 1000.0;
  
  /* derived_kernel_time := Total of the time spent in the phases of the decision cycle, 
     excluding Input Function, Output function, and pre-defined callbacks. 
     This computed time should be roughly equal to total_kernel_time, 
     as determined above.*/
  
#ifndef KERNEL_TIME_ONLY       
  derived_kernel_time = 
    timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
    + timer_value (&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE])) 
    + timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])) 
    + timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE])) 
    + timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])) 
    + timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE]));

  input_function_time  = timer_value (&current_agent(input_function_cpu_time));
  
  output_function_time = timer_value (&current_agent(output_function_cpu_time));


  /* Total of the time spent in callback routines. */
  monitors_sum =  timer_value (&current_agent(monitors_cpu_time[INPUT_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[PREFERENCE_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[WM_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[OUTPUT_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[DECISION_PHASE]));
  
  derived_total_cpu_time  = derived_kernel_time + monitors_sum + 
    input_function_time + output_function_time;

  /* Total time spent in the input phase */
  input_phase_total_time = 
    timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[INPUT_PHASE])) 
    + timer_value (&current_agent(input_function_cpu_time));
  
  /* REW: begin 10.30.97 */
  determine_level_phase_total_time = 
    timer_value (&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE]));
  /* REW: end   10.30.97 */      
  
  /* Total time spent in the preference phase */
  preference_phase_total_time = 
   timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[PREFERENCE_PHASE]));
  
  /* Total time spent in the working memory phase */
  wm_phase_total_time = 
    timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[WM_PHASE]));
  
  /* Total time spent in the output phase */
  output_phase_total_time = 
    timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[OUTPUT_PHASE])) 
    + timer_value (&current_agent(output_function_cpu_time));
  
  /* Total time spent in the decision phase */
  decision_phase_total_time = 
    timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE])) 
    + timer_value (&current_agent(monitors_cpu_time[DECISION_PHASE]));
  
  /* The sum of these six phase timers is exactly equal to the 
   * derived_total_cpu_time
   */
  
#ifdef DETAILED_TIMING_STATS
  
  match_time = timer_value (&current_agent(match_cpu_time[INPUT_PHASE])) 
    + timer_value (&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE])) 
    + timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE])) 
    + timer_value (&current_agent(match_cpu_time[WM_PHASE])) 
    + timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE])) 
    + timer_value (&current_agent(match_cpu_time[DECISION_PHASE]));
  
  match_msec = 1000 * match_time; 
  
  ownership_time = timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE])) 
    + timer_value (&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE])) 
    + timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE])) 
    + timer_value (&current_agent(ownership_cpu_time[WM_PHASE])) 
    + timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE])) 
    + timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE]));

  chunking_time = timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE])) 
    + timer_value (&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE])) 
    + timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE])) 
    + timer_value (&current_agent(chunking_cpu_time[WM_PHASE])) 
    + timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE])) 
    + timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE]));
  
  /* O-support time should go to 0 with o-support-mode 2 */
  /* o_support_time = 
     timer_value (&current_agent(o_support_cpu_time[INPUT_PHASE])) 
     + timer_value (&current_agent(o_support_cpu_time[DETERMINE_LEVEL_PHASE])) 
     + timer_value (&current_agent(o_support_cpu_time[PREFERENCE_PHASE])) 
     + timer_value (&current_agent(o_support_cpu_time[WM_PHASE])) 
     + timer_value (&current_agent(o_support_cpu_time[OUTPUT_PHASE])) 
     + timer_value (&current_agent(o_support_cpu_time[DECISION_PHASE])); */

  other_phase_kernel_time[INPUT_PHASE] = 
    timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
    -  timer_value (&current_agent(match_cpu_time[INPUT_PHASE]))
    -  timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE]))
    -  timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE]));
  
  other_phase_kernel_time[DETERMINE_LEVEL_PHASE] = 
    timer_value (&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]))
    -  timer_value (&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE]))
    -  timer_value (&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE]))
    -  timer_value (&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE]));


  other_phase_kernel_time[PREFERENCE_PHASE] = 
    timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]))
    -  timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE]))
    -  timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE]))
    -  timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE]));
  
  other_phase_kernel_time[WM_PHASE] = 
    timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE]))
    -  timer_value (&current_agent(match_cpu_time[WM_PHASE]))
    -  timer_value (&current_agent(ownership_cpu_time[WM_PHASE]))
    -  timer_value (&current_agent(chunking_cpu_time[WM_PHASE]));
  
  other_phase_kernel_time[OUTPUT_PHASE] = 
    timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]))
    -  timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE]))
    -  timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE]))
    -  timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE]));
  
  other_phase_kernel_time[DECISION_PHASE] = 
    timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE]))
    -  timer_value (&current_agent(match_cpu_time[DECISION_PHASE]))
    -  timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE]))
    -  timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE]));
  
  other_total_kernel_time = other_phase_kernel_time[INPUT_PHASE] 
    + other_phase_kernel_time[DETERMINE_LEVEL_PHASE]
    + other_phase_kernel_time[PREFERENCE_PHASE]
    + other_phase_kernel_time[WM_PHASE]
    + other_phase_kernel_time[OUTPUT_PHASE]
    + other_phase_kernel_time[DECISION_PHASE];

#endif
#endif
#endif
/* REW: end 28.07.96 */      


  print (glbAgent, "Soar %s on %s at %s\n", soar_version_string,
	 hostname, ctime((const time_t *)&current_time));
  
  print (glbAgent, "%lu productions (%lu default, %lu user, %lu chunks)\n",
	 current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
	 current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
	 current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE],
	 current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE],
	 current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE],
	 current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);
  print(glbAgent, "   + %lu justifications\n",
	current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);
  
  /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
  /* The fields for the timers are 8.3, providing an upper limit of 
     approximately 2.5 hours the printing of the run time calculations.  
     Obviously, these will need to be increased if you plan on needing 
     run-time data for a process that you expect to take longer than 
     2 hours. :) */
  
#ifndef KERNEL_TIME_ONLY
  print (glbAgent, "                                                                |    Derived\n");
  print (glbAgent, "Phases:      Input      DLP     Pref      W/M   Output Decision |     Totals\n");
  print (glbAgent, "================================================================|===========\n");

      
  print (glbAgent, "Kernel:   %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n", timer_value (&current_agent(decision_cycle_phase_timers[INPUT_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[WM_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])), timer_value (&current_agent(decision_cycle_phase_timers[DECISION_PHASE])), derived_kernel_time);
  

#ifdef DETAILED_TIMING_STATS

  print(glbAgent, "====================  Detailed Timing Statistics  ==============|===========\n");


  print(glbAgent, "   Match: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n", 
	 timer_value (&current_agent(match_cpu_time[INPUT_PHASE])), 
	 timer_value (&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE])),
	 timer_value (&current_agent(match_cpu_time[PREFERENCE_PHASE])), 
	 timer_value (&current_agent(match_cpu_time[WM_PHASE])), 
	 timer_value (&current_agent(match_cpu_time[OUTPUT_PHASE])), 
	 timer_value (&current_agent(match_cpu_time[DECISION_PHASE])) , 
	 match_time);
  
  print(glbAgent, "Own'ship: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
	 timer_value (&current_agent(ownership_cpu_time[INPUT_PHASE])), 
	 timer_value (&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE])),
	 timer_value (&current_agent(ownership_cpu_time[PREFERENCE_PHASE])), 
	 timer_value (&current_agent(ownership_cpu_time[WM_PHASE])), 
	 timer_value (&current_agent(ownership_cpu_time[OUTPUT_PHASE])), 
	 timer_value (&current_agent(ownership_cpu_time[DECISION_PHASE])), 
	 ownership_time);
  
  print(glbAgent, "Chunking: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
	 timer_value (&current_agent(chunking_cpu_time[INPUT_PHASE])), 
	 timer_value (&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE])),
	 timer_value (&current_agent(chunking_cpu_time[PREFERENCE_PHASE])), 
	 timer_value (&current_agent(chunking_cpu_time[WM_PHASE])), 
	 timer_value (&current_agent(chunking_cpu_time[OUTPUT_PHASE])), 
	 timer_value (&current_agent(chunking_cpu_time[DECISION_PHASE])), 
	 chunking_time);
  
  print(glbAgent, "   Other: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
	 other_phase_kernel_time[INPUT_PHASE],
	 other_phase_kernel_time[DETERMINE_LEVEL_PHASE],
	 other_phase_kernel_time[PREFERENCE_PHASE],
	 other_phase_kernel_time[WM_PHASE],
	 other_phase_kernel_time[OUTPUT_PHASE],
	 other_phase_kernel_time[DECISION_PHASE],
	 other_total_kernel_time);
  
  
  /* REW: begin 11.25.96 */ 
  print(glbAgent, "Operand2: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
	 timer_value (&current_agent(gds_cpu_time[INPUT_PHASE])), 
	 timer_value (&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])), 
	 timer_value (&current_agent(gds_cpu_time[PREFERENCE_PHASE])), 
	 timer_value (&current_agent(gds_cpu_time[WM_PHASE])), 
	 timer_value (&current_agent(gds_cpu_time[OUTPUT_PHASE])), 
	 timer_value (&current_agent(gds_cpu_time[DECISION_PHASE])),
	 timer_value (&current_agent(gds_cpu_time[INPUT_PHASE])) + 
	 timer_value (&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])) + 
	 timer_value (&current_agent(gds_cpu_time[PREFERENCE_PHASE])) +
	 timer_value (&current_agent(gds_cpu_time[WM_PHASE])) +
	 timer_value (&current_agent(gds_cpu_time[OUTPUT_PHASE])) +
	 timer_value (&current_agent(gds_cpu_time[DECISION_PHASE]))); 
  
  /* REW: end   11.25.96 */ 
  
  
#endif
  
  
  print (glbAgent, "================================================================|===========\n");
  print (glbAgent, "Input fn: %8.3f                                              | %10.3f\n",  
	 input_function_time, input_function_time); 
  
  print (glbAgent, "================================================================|===========\n");
  print (glbAgent, "Outpt fn:                                     %8.3f          | %10.3f\n",  
	 output_function_time, output_function_time);
  
  print (glbAgent, "================================================================|===========\n");
  print (glbAgent, "Callbcks: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
	 timer_value (&current_agent(monitors_cpu_time[INPUT_PHASE])), 
	 timer_value (&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE])), 
	 timer_value (&current_agent(monitors_cpu_time[PREFERENCE_PHASE])), 
	 timer_value (&current_agent(monitors_cpu_time[WM_PHASE])), 
	 timer_value (&current_agent(monitors_cpu_time[OUTPUT_PHASE])), 
	 timer_value (&current_agent(monitors_cpu_time[DECISION_PHASE])), 
	 monitors_sum);
  
  print (glbAgent, "================================================================|===========\n");
  print (glbAgent, "Derived---------------------------------------------------------+-----------\n");
  print (glbAgent, "Totals:   %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n\n",
	 input_phase_total_time,
	 determine_level_phase_total_time, 
	 preference_phase_total_time,
	 wm_phase_total_time,
	 output_phase_total_time, 
	 decision_phase_total_time,
	 derived_total_cpu_time);

  if (!current_agent(stop_soar)) {
    /* Soar is still running, so this must have been invoked
     * from the RHS, therefore these timers need to be updated. */
    stop_timer(&current_agent(start_total_tv),
	       &current_agent(total_cpu_time));
    stop_timer(&current_agent(start_kernel_tv),
	       &current_agent(total_kernel_time));
    start_timer(&current_agent(start_total_tv));
    start_timer(&current_agent(start_kernel_tv));
  }

  print (glbAgent, "Values from single timers:\n");
#endif
#endif
#ifndef NO_TIMING_STUFF

#ifdef WARN_IF_TIMERS_REPORT_ZERO
  /* If a warning has occured since the last init-soar, the warn flag will
   * have been set to FALSE, so such a warning is not spammed to the screen
   * But lets repeat it here.
   */
  if ( !current_agent(warn_on_zero_timers) )
    print(glbAgent, " Warning: one or more timers have reported zero during this run\n");
#endif

#ifndef PII_TIMERS
  print(glbAgent, " Kernel CPU Time: %11.3f sec. \n", total_kernel_time);     
  print(glbAgent, " Total  CPU Time: %11.3f sec.\n\n", timer_value (&current_agent(total_cpu_time)));
#else
  print(glbAgent, " Using PII Timers ... Assuming Processor Speed of %d MHZ\n", MHZ );
  print(glbAgent, " Kernel CPU Time: %11.5f sec. \n", total_kernel_time);     
  print(glbAgent, " Total  CPU Time: %11.5f sec.\n\n", timer_value (&current_agent(total_cpu_time)));

#endif

#ifdef COUNT_KERNEL_TIMER_STOPS
  print(glbAgent, " Kernel CPU Timer Stops: %d\n", current_agent(kernelTimerStops));
  print(glbAgent, " Non-Kernel Timer Stops: %d\n", current_agent(nonKernelTimerStops));
  
#endif
#endif
  
#if !defined(NO_TIMING_STUFF)
  print(glbAgent, "%lu decision cycles (%.3f msec/dc)\n",
	 current_agent(d_cycle_count),
	 current_agent(d_cycle_count) ? total_kernel_msec/current_agent(d_cycle_count) : 0.0);
  print(glbAgent, "%lu elaboration cycles (%.3f ec's per dc, %.3f msec/ec)\n",
	 current_agent(e_cycle_count),
	 current_agent(d_cycle_count) ? (double)current_agent(e_cycle_count)/current_agent(d_cycle_count) : 0,
	 current_agent(e_cycle_count) ? total_kernel_msec/current_agent(e_cycle_count) : 0);
  /* REW: begin 09.15.96 */
  
#ifndef SOAR_8_ONLY
  if (current_agent(operand2_mode))
#endif
    print(glbAgent, "%lu p-elaboration cycles (%.3f pe's per dc, %.3f msec/pe)\n",
	   current_agent(pe_cycle_count),
	   current_agent(d_cycle_count) ? (double)current_agent(pe_cycle_count)/current_agent(d_cycle_count) : 0,
	   current_agent(pe_cycle_count) ? total_kernel_msec/current_agent(pe_cycle_count) : 0);
  /* REW: end 09.15.96 */
  print(glbAgent, "%lu production firings (%.3f pf's per ec, %.3f msec/pf)\n",
	 current_agent(production_firing_count),
	 current_agent(e_cycle_count) ? (double)current_agent(production_firing_count)/current_agent(e_cycle_count) : 0.0,
	 current_agent(production_firing_count) ? total_kernel_msec/current_agent(production_firing_count) : 0.0);
  
#else
  print(glbAgent, "%lu decision cycles\n", current_agent(d_cycle_count));
  print(glbAgent, "%lu elaboration cycles \n", current_agent(e_cycle_count));
  print(glbAgent, "%lu production firings \n", current_agent(production_firing_count));
#endif /* !NO_TIMING_STUFF */      

      
  wme_changes = current_agent(wme_addition_count) + current_agent(wme_removal_count);
  print(glbAgent, "%lu wme changes (%lu additions, %lu removals)\n",
	 wme_changes, current_agent(wme_addition_count), current_agent(wme_removal_count));
#ifdef DETAILED_TIMING_STATS
  print(glbAgent, "    match time: %.3f msec/wm change\n",
         wme_changes ? match_msec/wme_changes : 0.0);
#endif
  
  print(glbAgent, "WM size: %lu current, %.3f mean, %lu maximum\n",
	 current_agent(num_wmes_in_rete),
	 (current_agent(num_wm_sizes_accumulated) ?
	  (current_agent(cumulative_wm_size) / current_agent(num_wm_sizes_accumulated)) :
	  0.0),
	 current_agent(max_wm_size));
  
#ifndef NO_TIMING_STUFF
  print(glbAgent, "\n");
  print(glbAgent, "    *** Time/<x> statistics use the total kernel time from a ***\n");
  print(glbAgent, "    *** single kernel timer.  Differences between this value ***\n");
  print(glbAgent, "    *** and the derived total kernel time  are expected. See ***\n");
  print(glbAgent, "    *** help  for the  stats command  for more  information. ***\n");   
#endif
  /* REW: end 28.07.96 */
}

void soar_exPrintMemoryPoolStatistics (void) {
  memory_pool *p;

#ifdef MEMORY_POOL_STATS
  long total_items; 
#endif

  print(glbAgent, "Memory pool statistics:\n\n");
#ifdef MEMORY_POOL_STATS
  print(glbAgent, "Pool Name        Used Items  Free Items  Item Size  Total Bytes\n");
  print(glbAgent, "---------------  ----------  ----------  ---------  -----------\n");
#else
  print(glbAgent, "Pool Name        Item Size  Total Bytes\n");
  print(glbAgent, "---------------  ---------  -----------\n");
#endif

  for (p=current_agent(memory_pools_in_use); p!=NIL; p=p->next) {
    print_string (glbAgent, p->name);
    print_spaces (glbAgent, MAX_POOL_NAME_LENGTH - strlen(p->name));
#ifdef MEMORY_POOL_STATS
    print(glbAgent, "  %10lu", p->used_count);
    total_items = p->num_blocks * p->items_per_block;
    print(glbAgent, "  %10lu", total_items - p->used_count);
#endif
    print(glbAgent, "  %9lu", p->item_size);
    print(glbAgent, "  %11lu\n", p->num_blocks * p->items_per_block * p->item_size);
  }
}

void soar_ecPrintMemoryPoolStatistics (void) {
/* Since p is not used, we can comment this out. */
//   mem_pool *p=0;
#ifdef MEMORY_POOL_STATS
   long total_items;
#endif
   
   
//   print(glbAgent, "Memory pool statistics:\n\n");
#ifdef MEMORY_POOL_STATS
//   print(glbAgent, "Pool Name        Used Items  Free Items  Item Size  Total Bytes\n");
//   print(glbAgent, "---------------  ----------  ----------  ---------  -----------\n");
#else
//   print(glbAgent, "Pool Name        Item Size  Total Bytes\n");
//   print(glbAgent, "---------------  ---------  -----------\n");
#endif
   
//   for (p=current_agent(memory_pools_in_use); p!=NIL; p=p->next) {
//      print_string (glbAgent, p->name);
//      print_spaces (MAX_POOL_NAME_LENGTH - strlen(p->name));
//#ifdef MEMORY_POOL_STATS
//      print(glbAgent, "  %10lu", p->used_count);
//      total_items = p->num_blocks * p->ItemsPerBlock();
//      print(glbAgent, "  %10lu", total_items - p->used_count);
//#endif
//      print(glbAgent, "  %9lu", p->item_size);
//      print(glbAgent, "  %11lu\n", p->num_blocks * p->ItemsPerBlock() * p->item_size);
//   }
}

void soar_ecPrintMemoryStatistics (void) {
  unsigned long total;
  int i;

  total = 0;
  for (i=0; i<NUM_MEM_USAGE_CODES; i++) total += current_agent(memory_for_usage)[i];
  
  print(glbAgent, "%8lu bytes total memory allocated\n", total);
  print(glbAgent, "%8lu bytes statistics overhead\n",
         current_agent(memory_for_usage)[STATS_OVERHEAD_MEM_USAGE]);
  print(glbAgent, "%8lu bytes for strings\n",
         current_agent(memory_for_usage)[STRING_MEM_USAGE]);
  print(glbAgent, "%8lu bytes for hash tables\n",
         current_agent(memory_for_usage)[HASH_TABLE_MEM_USAGE]);
  print(glbAgent, "%8lu bytes for various memory pools\n",
         current_agent(memory_for_usage)[POOL_MEM_USAGE]);
  print(glbAgent, "%8lu bytes for miscellaneous other things\n",
         current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);
}


void soar_ecPrintReteStatistics (void) {

#ifdef TOKEN_SHARING_STATS
  print(glbAgent, "Token additions: %lu   If no sharing: %lu\n",
         current_agent(token_additions),
         current_agent(token_additions_without_sharing));
#endif

  print_node_count_statistics(glbAgent);
  print_null_activation_stats();
}

void soar_ecExplainChunkTrace(char *chunk_name) {
   
   explain_chunk_str *chunk;
   
   chunk = find_chunk(glbAgent, current_agent(explain_chunk_list),chunk_name);
   /* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */
   
   if (chunk)
      explain_trace_chunk(glbAgent, chunk);
}

void soar_ecExplainChunkCondition(char *chunk_name, int cond_number) {
   
   explain_chunk_str *chunk;
   condition *ground;
   
   chunk = find_chunk(glbAgent, current_agent(explain_chunk_list),chunk_name);
   /* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */
   
   if (chunk == NULL) return;
   
   ground = find_ground(glbAgent, chunk,cond_number);
   if (ground == NIL) return;
   
   explain_trace(glbAgent, chunk_name,chunk->backtrace,ground);
}


void soar_ecExplainChunkConditionList(char *chunk_name) {
   
   explain_chunk_str *chunk;
   condition *cond, *ground;
   int i;
   
   chunk = find_chunk(glbAgent, current_agent(explain_chunk_list),chunk_name);
   /* AGR 564  In previous statement, current_agent(...) was added.  2-May-94 */
   
   if (chunk == NULL) return;
   
   /* First print out the production in "normal" form */
   
   print(glbAgent, "(sp %s\n  ", chunk->name);
   print_condition_list (glbAgent, chunk->conds, 2, FALSE);
   print(glbAgent, "\n-->\n   ");
   print_action_list (glbAgent, chunk->actions, 3, FALSE);
   print(glbAgent, ")\n\n");
   
   /* Then list each condition and the associated "ground" WME */
   
   i = 0; 
   ground = chunk->all_grounds;
   
   for (cond = chunk->conds; cond != NIL; cond = cond->next) {
      i++; print(glbAgent, " %2d : ",i);
      print_condition(glbAgent, cond);
      while (get_printer_output_column(glbAgent) < COLUMNS_PER_LINE-40)
         print(glbAgent, " ");
      
      print(glbAgent, " Ground :");
      print_condition(glbAgent, ground);
      print(glbAgent, "\n");
      ground=ground->next;
   }
}

int soar_ecPrintAllProductionsOfType( int type, Bool internal,
                                     Bool print_fname, Bool full_prod ) {
   
   production *prod;
   
   /* we'll step through the list backwards, so chunks and justifications
   * are displayed in reverse cronological order.
   */
   
   if (type <= 0 || type >= NUM_PRODUCTION_TYPES ) { return -1; }
   
   for( prod = current_agent(all_productions_of_type)[type];
   prod != NIL && prod->next != NIL;
   prod = prod->next )
      /* intentionally null */ ;
      
      while ( prod != NIL ) {
         
         /* Print it... */
         if (!full_prod) {
            print_with_symbols(glbAgent, "%y  ",prod->name);
         }
         if (print_fname) {
            print_string(glbAgent, "# sourcefile : ");
            if (prod->filename) {
               print_string(glbAgent, prod->filename);
            } else {
               print_string(glbAgent, " _unknown_ ");
            }
         }
         print(glbAgent, "\n");
         if (full_prod) {
            print_production (glbAgent, prod, internal);
            print(glbAgent, "\n");
         }
         prod = prod->prev;
      }
      
      return 0;
}

void soar_ecPrintProductionsBeingTraced() {

  cons *c;
  
  for (c=current_agent(productions_being_traced); c!=NIL; c=c->rest)
    print_with_symbols (glbAgent, " %y\n", ((production *)(c->first))->name);  
}

void soar_ecStopAllProductionTracing() {
  
  cons *c, *next;

  /*
   * We don't use c=c->rest in the increment step because 
   * remove_pwatch may release c as a side-effect.
   */
  
  for (c=current_agent(productions_being_traced); c!=NIL; c=next) {
    production * prod;
    
    next = c->rest;
    prod = static_cast<production_struct *>(current_agent(productions_being_traced)->first);
    remove_pwatch(glbAgent, prod);
  }
  
}

int soar_ecBeginTracingProductions( int n, char **names ) {

  int i;
  production *prod;

  for( i = 0; i < n; i++ ) {
    
    if ( (prod = name_to_production( names[i] )) ) {
      add_pwatch( glbAgent, prod );
    }
    else {
      print( glbAgent, "No Production named %s", names[i] );
      return (-1-i);
    }
  }
  return 0;
}


int soar_ecStopTracingProductions( int n, char **names ) {
  
  int i;
  production *prod;


  for( i = 0; i < n; i++ ) {
    
    if ( (prod = name_to_production( names[i] )) ) {
      remove_pwatch( glbAgent, prod );
    }
    else {
      print( glbAgent, "No Production named %s", names[i] );
      return (-1-i);
    }
  }
  return 0;
}
