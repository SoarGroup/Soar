/*************************************************************************
 *
 *  file:  wmem.c
 *
 * =======================================================================
 *  These are the working memory management routines and utility functions
 *  for Soar working memory elements.
 * =======================================================================
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

/* ======================================================================
         Working memory routines for Soar 6
   ====================================================================== */

/* Debugging stuff:  #define DEBUG_WMES to get slot printouts */

/* #define DEBUG_WMES */

#include "soarkernel.h"

extern void filtered_print_wme_add(wme *w);
extern void filtered_print_wme_remove(wme *w);

/* ======================================================================

             Working Memory Management and Utility Routines

   Reset_wme_timetags() resets the wme timetag generator back to 1.
   This should be called during an init-soar.

   Make_wme() creates and returns a new wme.  The caller should add the
   wme onto the appropriate dll (e.g., my_slot->wmes) and should call
   add_wme_to_wm() on it.

   Add_wme_to_wm() and remove_wme_from_wm() make changes to WM.  Again,
   the caller is responsible for manipulating the appropriate dll.  WM
   changes don't actually get stuffed down the rete until the end of the
   phase, when do_buffered_wm_changes() gets be called.

   Remove_wme_list_from_wm() is a utility routine that scans through a
   list of wmes, linked by their "next" fields, and calls remove_wme_from_wm()
   on each one.

   Deallocate_wme() deallocates a wme.  This should only be invoked via
   the wme_remove_ref() macro.

   Find_name_of_object() is a utility function for finding the value of
   the ^name attribute on a given object (Symbol).  It returns the name,
   or NIL if the object has no name.
====================================================================== */


void reset_wme_timetags (void) {
  if (current_agent(num_existing_wmes) != 0) {
    print ("Internal warning:  wanted to reset wme timetag generator, but\n");
    print ("there are still some wmes allocated. (Probably a memory leak.)\n");
    print ("(Leaving timetag numbers alone.)\n");
    return;
  }
  current_agent(current_wme_timetag) = 1;
}

wme *make_wme (Symbol *id, Symbol *attr, Symbol *value, bool acceptable) {
  wme *w;

  current_agent(num_existing_wmes)++;
  allocate_with_pool (&current_agent(wme_pool), &w);
  w->id = id;
  w->attr = attr;
  w->value = value;
  symbol_add_ref (id);
  symbol_add_ref (attr);
  symbol_add_ref (value);
  w->acceptable = acceptable;
  w->timetag = current_agent(current_wme_timetag)++;
  w->reference_count = 0;
  w->preference = NIL;
  w->output_link = NIL;
  w->grounds_tc = 0;
  w->potentials_tc = 0;
  w->locals_tc = 0;

/* REW: begin 09.15.96 */
  /* When we first create a WME, it had no gds value.  
     Do this for ALL wmes, regardless of the operand mode, so that no undefined pointers
     are floating around. */
     w->gds = NIL;
/* REW: end 09.15.96 */

  return w;
}

/* --- lists of buffered WM changes --- */

void add_wme_to_wm (wme *w) {
  push (w, current_agent(wmes_to_add));
  if (w->value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) {
    post_link_addition (w->id, w->value);
    if (w->attr==current_agent(operator_symbol)) w->value->id.isa_operator++;
  }
}

void remove_wme_from_wm (wme *w) {
  push (w, current_agent(wmes_to_remove));
  if (w->value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) {
    post_link_removal (w->id, w->value);
    if (w->attr==current_agent(operator_symbol)) w->value->id.isa_operator--;
  }
  /* REW: begin 09.15.96 */
  /* When we remove a WME, we always have to determine if it's on a GDS, and, if
     so, after removing the WME, if there are no longer any WMEs on the GDS,
     then we can free the GDS memory */
  if (w->gds){
    fast_remove_from_dll(w->gds->wmes_in_gds, w, wme, gds_next, gds_prev);
    /* printf("\nRemoving WME on some GDS"); */
          if (!w->gds->wmes_in_gds) {
		 free_memory(w->gds, MISCELLANEOUS_MEM_USAGE);
                 /* printf("REMOVING GDS FROM MEMORY. \n"); */
	  }
  }
  /* REW: end   09.15.96 */
}

void remove_wme_list_from_wm (wme *w) {
  wme *next_w;

  while (w) {
    next_w = w->next;
    remove_wme_from_wm (w);
    w = next_w;
  }
}

void do_buffered_wm_changes (void) {
  cons *c, *next_c, *cr;
  wme *w;
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  struct timeval start_tv;
#endif
#endif

  /* --- if no wme changes are buffered, do nothing --- */
  if (!current_agent(wmes_to_add) && !current_agent(wmes_to_remove)) return;

  /* --- call output module in case any changes are output link changes --- */
  inform_output_module_of_wm_changes (current_agent(wmes_to_add), current_agent(wmes_to_remove));

  /* --- invoke callback routine.  wmes_to_add and wmes_to_remove can   --- */
  /* --- be fetched from the agent structure.                           --- */
  soar_invoke_callbacks(soar_agent, WM_CHANGES_CALLBACK, 
			(soar_call_data) NULL); 

  /* --- stuff wme changes through the rete net --- */
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  start_timer (&start_tv);
#endif
#endif
  for (c=current_agent(wmes_to_add); c!=NIL; c=c->rest) add_wme_to_rete (c->first);
  for (c=current_agent(wmes_to_remove); c!=NIL; c=c->rest) remove_wme_from_rete (c->first);
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  stop_timer (&start_tv, &current_agent(match_cpu_time[current_agent(current_phase)]));
#endif
#endif
  /* --- warn if watching wmes and same wme was added and removed -- */
  if (current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM]) {
    for (c=current_agent(wmes_to_add); c!=NIL; c=next_c) {
      next_c = c->rest;
      w = c->first;
      for (cr=current_agent(wmes_to_remove); cr!=NIL; cr=next_c) {
	next_c = cr->rest;
	if (w == cr->first) {
	  print ("WARNING: WME added and removed in same phase : ");
	  print_wme(w);
	} } } }


  /* --- do tracing and cleanup stuff --- */
  for (c=current_agent(wmes_to_add); c!=NIL; c=next_c) {
    next_c = c->rest;
    w = c->first;
    if (current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM]) {
      /* print ("=>WM: ");
       * print_wme (w);
       */
      filtered_print_wme_add(w); /* kjh(CUSP-B2) begin */
    }
    wme_add_ref (w);
    free_cons (c);
    current_agent(wme_addition_count)++;
  }
  for (c=current_agent(wmes_to_remove); c!=NIL; c=next_c) {
    next_c = c->rest;
    w = c->first;
    if (current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM]) {
      /* print ("<=WM: "); 
       * print_wme (w);
       */
      filtered_print_wme_remove (w);  /* kjh(CUSP-B2) begin */
    }
    wme_remove_ref (w);
    free_cons (c);
    current_agent(wme_removal_count)++;
  }
  current_agent(wmes_to_add) = NIL;
  current_agent(wmes_to_remove) = NIL;
}

void deallocate_wme (wme *w) {
#ifdef DEBUG_WMES  
  print_with_symbols ("\nDeallocate wme: ");
  print_wme (w);
#endif
  symbol_remove_ref (w->id);
  symbol_remove_ref (w->attr);
  symbol_remove_ref (w->value);
  free_with_pool (&current_agent(wme_pool), w);
  current_agent(num_existing_wmes)--;
}

Symbol *find_name_of_object (Symbol *object) {
  slot *s;

  if (object->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) return NIL;
  s = find_slot (object, current_agent(name_symbol));
  if (! s) return NIL;
  if (! s->wmes) return NIL;
  return s->wmes->value;
}

