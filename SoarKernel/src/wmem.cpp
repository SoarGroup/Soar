#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  wmem.cpp
 *
 * =======================================================================
 *  These are the working memory management routines and utility functions
 *  for Soar working memory elements.
 * =======================================================================
 */

/* ======================================================================
         Working memory routines for Soar 6
   ====================================================================== */

/* Debugging stuff:  #define DEBUG_WMES to get slot printouts */

//#define DEBUG_WMES

#include <stdlib.h>

#include "wmem.h"
#include "kernel.h"
#include "agent.h"
#include "gdatastructs.h"
#include "symtab.h"
#include "decide.h"
#include "io_soar.h"
#include "rete.h"
#include "print.h"
#include "tempmem.h"
#include "xml.h"
#include "soar_TraceNames.h"

#include "wma.h"
#include "episodic_memory.h"

using namespace soar_TraceNames;

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

void reset_wme_timetags (agent* thisAgent) {
  if (thisAgent->num_existing_wmes != 0) {
    print (thisAgent, "Internal warning:  wanted to reset wme timetag generator, but\n");
    print (thisAgent, "there are still some wmes allocated. (Probably a memory leak.)\n");
    print (thisAgent, "(Leaving timetag numbers alone.)\n");
	xml_generate_warning(thisAgent, "Internal warning:  wanted to reset wme timetag generator, but\nthere are still some wmes allocated. (Probably a memory leak.)\n(Leaving timetag numbers alone.)");
    return;
  }
  thisAgent->current_wme_timetag = 1;
}

wme *make_wme (agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value, Bool acceptable)
{
  wme *w;

  thisAgent->num_existing_wmes++;
  allocate_with_pool (thisAgent, &thisAgent->wme_pool, &w);
  w->id = id;
  w->attr = attr;
  w->value = value;
  symbol_add_ref (id);
  symbol_add_ref (attr);
  symbol_add_ref (value);
  w->acceptable = acceptable;
  w->metadata = '\0';
  w->timetag = thisAgent->current_wme_timetag++;
  w->reference_count = 0;
  w->preference = NIL;
  w->output_link = NIL;
  w->grounds_tc = 0;
  w->potentials_tc = 0;
  w->locals_tc = 0;

  w->next = NIL;
  w->prev = NIL;
  w->rete_next = NIL;
  w->rete_prev = NIL;

/* REW: begin 09.15.96 */
  /* When we first create a WME, it had no gds value.  
     Do this for ALL wmes, regardless of the operand mode, so that no undefined pointers
     are floating around. */
  w->gds = NIL;
  w->gds_prev = NIL;
  w->gds_next = NIL;
/* REW: end 09.15.96 */

  w->wma_decay_el = NIL;
  w->wma_tc_value = 0;

  w->epmem_id = EPMEM_NODEID_BAD;
  w->epmem_valid = NIL;

  return w;
}

/* --- lists of buffered WM changes --- */

void add_wme_to_wm (agent* thisAgent, wme *w) 
{
	assert( ( ( w->id->id.common_symbol_info.symbol_type != IDENTIFIER_SYMBOL_TYPE ) || ( w->id->id.level > SMEM_LTI_UNKNOWN_LEVEL ) ) &&
		( ( w->attr->id.common_symbol_info.symbol_type != IDENTIFIER_SYMBOL_TYPE ) || ( w->attr->id.level > SMEM_LTI_UNKNOWN_LEVEL ) ) &&
		( ( w->value->id.common_symbol_info.symbol_type != IDENTIFIER_SYMBOL_TYPE ) || ( w->value->id.level > SMEM_LTI_UNKNOWN_LEVEL ) ) );

	push (thisAgent, w, thisAgent->wmes_to_add);
	if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) 
	{
		post_link_addition (thisAgent, w->id, w->value);
		if (w->attr == thisAgent->operator_symbol) 
		{
			w->value->id.isa_operator++;
		}
	}
}

void remove_wme_from_wm (agent* thisAgent, wme *w) 
{
   push (thisAgent, w, thisAgent->wmes_to_remove);
   
   if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) 
   {
      post_link_removal (thisAgent, w->id, w->value);
      if (w->attr==thisAgent->operator_symbol) 
      {
         /* Do this afterward so that gSKI can know that this is an operator */
         w->value->id.isa_operator--;
      }
   }
   
   /* REW: begin 09.15.96 */
   /* When we remove a WME, we always have to determine if it's on a GDS, and, if
   so, after removing the WME, if there are no longer any WMEs on the GDS,
   then we can free the GDS memory */
   if (w->gds)
   {
      fast_remove_from_dll(w->gds->wmes_in_gds, w, wme, gds_next, gds_prev);
      /* printf("\nRemoving WME on some GDS"); */
      
      if (!w->gds->wmes_in_gds) 
	  {
		 if (w->gds->goal) w->gds->goal->id.gds = NIL;
		 free_with_pool( &( thisAgent->gds_pool ), w->gds );
         /* printf("REMOVING GDS FROM MEMORY. \n"); */
      }
   }
   /* REW: end   09.15.96 */
}

void remove_wme_list_from_wm (agent* thisAgent, wme *w, bool updateWmeMap) 
{
	wme *next_w;

	while (w) 
	{
		next_w = w->next;

		if (updateWmeMap) 
		{
			soar_invoke_callbacks( thisAgent, INPUT_WME_GARBAGE_COLLECTED_CALLBACK, static_cast< soar_call_data >( w ) ); 
			//remove_wme_from_wmeMap (thisAgent, w);
		}
		remove_wme_from_wm (thisAgent, w);

		w = next_w;
	}
}

void do_buffered_wm_changes (agent* thisAgent) 
{
  cons *c, *next_c, *cr;
  wme *w;
  /*
  void filtered_print_wme_add(wme *w), filtered_print_wme_remove(wme *w);
  */
  
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  soar_process_timer local_timer;
  local_timer.set_enabled( &( thisAgent->sysparams[ TIMERS_ENABLED ] ) );
#endif
#endif

  /* --- if no wme changes are buffered, do nothing --- */
  if (!thisAgent->wmes_to_add && !thisAgent->wmes_to_remove) return;

  /* --- call output module in case any changes are output link changes --- */
  inform_output_module_of_wm_changes (thisAgent, thisAgent->wmes_to_add, 
                                      thisAgent->wmes_to_remove);

  /* --- invoke callback routine.  wmes_to_add and wmes_to_remove can   --- */
  /* --- be fetched from the agent structure.                           --- */
  soar_invoke_callbacks(thisAgent, WM_CHANGES_CALLBACK, 0); 

  /* --- stuff wme changes through the rete net --- */
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  local_timer.start();
#endif
#endif
  for (c=thisAgent->wmes_to_add; c!=NIL; c=c->rest) 
  {
     add_wme_to_rete (thisAgent, static_cast<wme_struct *>(c->first));
  }
  for (c=thisAgent->wmes_to_remove; c!=NIL; c=c->rest)
  {
     remove_wme_from_rete (thisAgent, static_cast<wme_struct *>(c->first));
  }
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  local_timer.stop();
  thisAgent->timers_match_cpu_time[thisAgent->current_phase].update(local_timer);
#endif
#endif
  /* --- warn if watching wmes and same wme was added and removed -- */
  if (thisAgent->sysparams[TRACE_WM_CHANGES_SYSPARAM]) {
     for (c=thisAgent->wmes_to_add; c!=NIL; c=next_c) {
        next_c = c->rest;
        w = static_cast<wme_struct *>(c->first);
        for (cr=thisAgent->wmes_to_remove; cr!=NIL; cr=next_c) {
           next_c = cr->rest;
           if (w == cr->first) {
			  const char * const kWarningMessage = "WARNING: WME added and removed in same phase : ";
              print (thisAgent, const_cast< char* >( kWarningMessage) );
			  xml_begin_tag( thisAgent, kTagWarning );
			  xml_att_val( thisAgent, kTypeString, kWarningMessage );
              print_wme(thisAgent, w);
  			  xml_end_tag( thisAgent, kTagWarning );
           } 
        } 
     } 
  }


  /* --- do tracing and cleanup stuff --- */
  for (c=thisAgent->wmes_to_add; c!=NIL; c=next_c) {
    next_c = c->rest;
    w = static_cast<wme_struct *>(c->first);
    if (thisAgent->sysparams[TRACE_WM_CHANGES_SYSPARAM]) {
      /* print ("=>WM: ");
       * print_wme (w);
       */
      filtered_print_wme_add(thisAgent, w); /* kjh(CUSP-B2) begin */
    }

    wme_add_ref (w);
    free_cons (thisAgent, c);
    thisAgent->wme_addition_count++;
  }
  for (c=thisAgent->wmes_to_remove; c!=NIL; c=next_c) {
    next_c = c->rest;
    w = static_cast<wme_struct *>(c->first);
    if (thisAgent->sysparams[TRACE_WM_CHANGES_SYSPARAM]) {
      /* print ("<=WM: "); 
       * print_wme (thisAgent, w);
       */
      filtered_print_wme_remove (thisAgent, w);  /* kjh(CUSP-B2) begin */
    }

    wme_remove_ref (thisAgent, w);
    free_cons (thisAgent, c);
    thisAgent->wme_removal_count++;
  }
  thisAgent->wmes_to_add = NIL;
  thisAgent->wmes_to_remove = NIL;
}

void deallocate_wme (agent* thisAgent, wme *w) {
#ifdef DEBUG_WMES  
  print_with_symbols (thisAgent, "\nDeallocate wme: ");
  print_wme (thisAgent, w);
#endif

  if ( wma_enabled( thisAgent ) )
  {
    wma_remove_decay_element( thisAgent, w );
  }

  symbol_remove_ref (thisAgent, w->id);
  symbol_remove_ref (thisAgent, w->attr);
  symbol_remove_ref (thisAgent, w->value);
  free_with_pool (&thisAgent->wme_pool, w);
  thisAgent->num_existing_wmes--;
}

Symbol *find_name_of_object (agent* thisAgent, Symbol *object) {
  slot *s;

  if (object->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) return NIL;
  s = find_slot (object, thisAgent->name_symbol);
  if (! s) return NIL;
  if (! s->wmes) return NIL;
  return s->wmes->value;
}

