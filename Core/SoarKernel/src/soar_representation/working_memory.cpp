#include "working_memory.h"

#include "agent.h"
#include "decide.h"
#include "ebc.h"
#include "episodic_memory.h"
#include "io_link.h"
#include "output_manager.h"
#include "print.h"
#include "rete.h"
#include "slot.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "working_memory_activation.h"
#include "xml.h"

#include <stdlib.h>
#include "dprint.h"

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

WM_Manager::WM_Manager(agent* myAgent)
{
    thisAgent = myAgent;
    thisAgent->WM = this;

    wma_params = new wma_param_container(thisAgent);
    wma_stats = new wma_stat_container(thisAgent);
    wma_timers = new wma_timer_container(thisAgent);

#ifdef USE_MEM_POOL_ALLOCATORS
    wma_forget_pq = new wma_forget_p_queue(std::less< wma_d_cycle >(), soar_module::soar_memory_pool_allocator< std::pair< wma_d_cycle, wma_decay_set* > >());
    wma_touched_elements = new wma_pooled_wme_set(std::less< wme* >(), soar_module::soar_memory_pool_allocator< wme* >(thisAgent));
    wma_touched_sets = new wma_decay_cycle_set(std::less< wma_d_cycle >(), soar_module::soar_memory_pool_allocator< wma_d_cycle >(thisAgent));
#else
    wma_forget_pq = new wma_forget_p_queue();
    wma_touched_elements = new wma_pooled_wme_set();
    wma_touched_sets = new wma_decay_cycle_set();
#endif
    wma_initialized = false;
    wma_tc_counter = 2;

};


void WM_Manager::clean_up_for_agent_deletion()
{
    /* This is not in destructor because it may be called before other
     * deletion code that may need params, stats or timers to exist */

    wma_params->activation->set_value(off);
    delete wma_forget_pq;
    delete wma_touched_elements;
    delete wma_touched_sets;
    delete wma_params;
    delete wma_stats;
    delete wma_timers;

}

void reset_wme_timetags(agent* thisAgent)
{
    if (thisAgent->num_existing_wmes != 0)
    {
        thisAgent->outputManager->printa(thisAgent,  "Internal warning:  wanted to reset wme timetag generator, but\n");
        thisAgent->outputManager->printa(thisAgent,  "there are still some wmes allocated. (Probably a memory leak.)\n");
        thisAgent->outputManager->printa(thisAgent,  "(Leaving timetag numbers alone.)\n");
        xml_generate_warning(thisAgent, "Internal warning:  wanted to reset wme timetag generator, but\nthere are still some wmes allocated. (Probably a memory leak.)\n(Leaving timetag numbers alone.)");
        return;
    }
    thisAgent->current_wme_timetag = 1;
}

wme* make_wme(agent* thisAgent, Symbol* id, Symbol* attr, Symbol* value, bool acceptable)
{
    wme* w;

    thisAgent->num_existing_wmes++;
    thisAgent->memoryManager->allocate_with_pool(MP_wme, &w);
    w->id = id;
    w->attr = attr;
    w->value = value;
    thisAgent->symbolManager->symbol_add_ref(id);
    thisAgent->symbolManager->symbol_add_ref(attr);
    thisAgent->symbolManager->symbol_add_ref(value);
    w->acceptable = acceptable;
    w->timetag = thisAgent->current_wme_timetag++;
    w->reference_count = 0;
    w->preference = NIL;
    w->output_link = NIL;
    w->tc = 0;
    w->chunker_bt_last_ground_cond = NULL;

    w->next = NIL;
    w->prev = NIL;
    w->rete_next = NIL;
    w->rete_prev = NIL;

    /* When we first create a WME, it had no gds value.
       Do this for ALL wmes, regardless of the operand mode, so that no undefined pointers
       are floating around. */
    w->gds = NIL;
    w->gds_prev = NIL;
    w->gds_next = NIL;

    w->wma_decay_el = NIL;
    w->wma_tc_value = 0;

    w->epmem_id = EPMEM_NODEID_BAD;
    w->epmem_valid = NIL;

    return w;
}

/* --- lists of buffered WM changes --- */

void add_wme_to_wm(agent* thisAgent, wme* w)
{
    /* Not sure if this is necessary anymore now that we don't have LTIs in STM.
     *
     * We do have an agent that causes this assert to fire. If we disable the assert,
     * it seems to run fine, so perhaps the level gets set correctly soon after.  The
     * agent is a very weird one, so for now, we'll put in a warning with a debug statement
     * until we have time to investigate (or get a less crazy agent than Shane's.) */

    //    assert(((!w->id->is_sti()) || (w->id->id->level != NO_WME_LEVEL)) &&
    //           ((!w->attr->is_sti()) || (w->attr->id->level != NO_WME_LEVEL)) &&
    //           ((!w->value->is_sti()) || (w->value->id->level != NO_WME_LEVEL)));
    dprint_noprefix(DT_DEBUG, "%s", !(((!w->id->is_sti()) || (w->id->id->level != NO_WME_LEVEL)) &&
           ((!w->attr->is_sti()) || (w->attr->id->level != NO_WME_LEVEL)) &&
           ((!w->value->is_sti()) || (w->value->id->level != NO_WME_LEVEL))) ? "Missing ID level in WME!\n" : "");


    dprint(DT_WME_CHANGES, "Adding wme %w to wmes_to_add\n", w);
    push(thisAgent, w, thisAgent->wmes_to_add);

    if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        dprint(DT_WME_CHANGES, "Calling post-link addition for id %y and value %y.\n", w->id, w->value);
        post_link_addition(thisAgent, w->id, w->value);
        if (w->attr == thisAgent->symbolManager->soarSymbols.operator_symbol)
        {
            w->value->id->isa_operator++;
        }
    }

    #ifdef DEBUG_ATTR_AS_LINKS
    if (w->attr->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        dprint(DT_WME_CHANGES, "Calling post-link addition for id %y and attr %y.\n", w->id, w->attr);
        post_link_addition(thisAgent, w->id, w->attr);
    }
    #endif
}

void remove_wme_from_wm(agent* thisAgent, wme* w)
{
    dprint(DT_WME_CHANGES, "Removing wme %w by adding to wmes_to_remove list...\n", w);

    push(thisAgent, w, thisAgent->wmes_to_remove);

    if (w->value->is_sti())
    {
        dprint(DT_WME_CHANGES, "Calling post-link removal for id %y and value %y.\n", w->id, w->value);
        post_link_removal(thisAgent, w->id, w->value);
    #ifdef DEBUG_ATTR_AS_LINKS
    if (w->attr->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        dprint(DT_WME_CHANGES, "Calling post-link removal for id %y and attr %y.\n", w->id, w->attr);
        post_link_removal(thisAgent, w->id, w->attr);
    }
    #endif
    if (w->attr == thisAgent->symbolManager->soarSymbols.operator_symbol)
        {
            /* Do this afterward so that gSKI can know that this is an operator */
            w->value->id->isa_operator--;
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
            if (w->gds->goal)
            {
                w->gds->goal->id->gds = NIL;
            }
            thisAgent->memoryManager->free_with_pool(MP_gds, w->gds);
            /* printf("REMOVING GDS FROM MEMORY. \n"); */
        }
    }
    /* REW: end   09.15.96 */
}

void remove_wme_list_from_wm(agent* thisAgent, wme* w, bool updateWmeMap)
{
    wme* next_w;

    while (w)
    {
        next_w = w->next;

        if (updateWmeMap)
        {
            soar_invoke_callbacks(thisAgent, INPUT_WME_GARBAGE_COLLECTED_CALLBACK, static_cast< soar_call_data >(w));
            //remove_wme_from_wmeMap (thisAgent, w);
        }
        remove_wme_from_wm(thisAgent, w);

        w = next_w;
    }
}

void do_buffered_wm_changes(agent* thisAgent)
{
    cons* c, *next_c, *cr;
    wme* w;
    /*
    void filtered_print_wme_add(wme *w), filtered_print_wme_remove(wme *w);
    */

    dprint(DT_WME_CHANGES, "Doing buffered WM changes...\n");

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    soar_timer local_timer;
    local_timer.set_enabled(&(thisAgent->trace_settings[ TIMERS_ENABLED ]));
#endif
#endif

    /* --- if no wme changes are buffered, do nothing --- */
    if (!thisAgent->wmes_to_add && !thisAgent->wmes_to_remove)
    {
        dprint(DT_WME_CHANGES, "...nothing to do.\n");
        return;
    }

    /* --- call output module in case any changes are output link changes --- */
    dprint(DT_WME_CHANGES, "...informing output code of wm changes.\n");
    inform_output_module_of_wm_changes(thisAgent, thisAgent->wmes_to_add,
                                       thisAgent->wmes_to_remove);

    /* --- invoke callback routine.  wmes_to_add and wmes_to_remove can   --- */
    /* --- be fetched from the agent structure.                           --- */
    dprint(DT_WME_CHANGES, "...invoking wm changes callbacks.\n");
    soar_invoke_callbacks(thisAgent, WM_CHANGES_CALLBACK, 0);

    /* --- stuff wme changes through the rete net --- */
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    local_timer.start();
#endif
#endif
    dprint(DT_WME_CHANGES, "...adding wmes_to_add to rete.\n");
    for (c = thisAgent->wmes_to_add; c != NIL; c = c->rest)
    {
        dprint(DT_WME_CHANGES, "...adding %w to rete\n", static_cast<wme_struct*>(c->first));
        add_wme_to_rete(thisAgent, static_cast<wme_struct*>(c->first));
    }
    dprint(DT_WME_CHANGES, "...removing wmes_to_remove from rete.\n");
    for (c = thisAgent->wmes_to_remove; c != NIL; c = c->rest)
    {
        dprint(DT_WME_CHANGES, "...removing %w from rete.\n", static_cast<wme_struct*>(c->first));
        remove_wme_from_rete(thisAgent, static_cast<wme_struct*>(c->first));
    }
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    local_timer.stop();
    thisAgent->timers_match_cpu_time[thisAgent->current_phase].update(local_timer);
#endif
#endif
    dprint(DT_WME_CHANGES, "...looking for wmes added and removed in same phase.\n");
    /* --- warn if watching wmes and same wme was added and removed -- */
    if (thisAgent->trace_settings[TRACE_WM_CHANGES_SYSPARAM])
    {
        for (c = thisAgent->wmes_to_add; c != NIL; c = next_c)
        {
            next_c = c->rest;
            w = static_cast<wme_struct*>(c->first);
            for (cr = thisAgent->wmes_to_remove; cr != NIL; cr = next_c)
            {
                next_c = cr->rest;
                if (w == cr->first)
                {
                    dprint(DT_WME_CHANGES, "...found wme added and removed in same phase!\n");
                    const char* const kWarningMessage = "WARNING: WME added and removed in same phase : ";
                    thisAgent->outputManager->printa(thisAgent,  const_cast< char* >(kWarningMessage));
                    xml_begin_tag(thisAgent, kTagWarning);
                    xml_att_val(thisAgent, kTypeString, kWarningMessage);
                    print_wme(thisAgent, w);
                    xml_end_tag(thisAgent, kTagWarning);
                }
            }
        }
    }


    dprint(DT_WME_CHANGES, "...WMEs to add:\n");
    /* --- do tracing and cleanup stuff --- */
    for (c = thisAgent->wmes_to_add; c != NIL; c = next_c)
    {
        next_c = c->rest;
        w = static_cast<wme_struct*>(c->first);
        if (thisAgent->trace_settings[TRACE_WM_CHANGES_SYSPARAM])
        {
            /* print ("=>WM: ");
             * print_wme (w);
             */
            filtered_print_wme_add(thisAgent, w); /* kjh(CUSP-B2) begin */
        }

        dprint(DT_WME_CHANGES, "      %w:\n",w);
        wme_add_ref(w);
        free_cons(thisAgent, c);
        thisAgent->wme_addition_count++;
    }
    dprint(DT_WME_CHANGES, "...WMEs to remove:\n");
    for (c = thisAgent->wmes_to_remove; c != NIL; c = next_c)
    {
        next_c = c->rest;
        w = static_cast<wme_struct*>(c->first);
        if (thisAgent->trace_settings[TRACE_WM_CHANGES_SYSPARAM])
        {
            /* print ("<=WM: ");
             * print_wme (thisAgent, w);
             */
            filtered_print_wme_remove(thisAgent, w);   /* kjh(CUSP-B2) begin */
        }

        dprint(DT_WME_CHANGES, "      %w:\n",w);
        wme_remove_ref(thisAgent, w);
        free_cons(thisAgent, c);
        thisAgent->wme_removal_count++;
    }
    dprint(DT_WME_CHANGES, "Finished doing buffered WM changes\n");
    thisAgent->wmes_to_add = NIL;
    thisAgent->wmes_to_remove = NIL;
}

void deallocate_wme(agent* thisAgent, wme* w)
{
    dprint(DT_WME_CHANGES, "Deallocating wme %w\n", w);
    if (wma_enabled(thisAgent))
    {
        wma_remove_decay_element(thisAgent, w);
    }

    thisAgent->symbolManager->symbol_remove_ref(&w->id);
    thisAgent->symbolManager->symbol_remove_ref(&w->attr);
    thisAgent->symbolManager->symbol_remove_ref(&w->value);

    thisAgent->memoryManager->free_with_pool(MP_wme, w);
    thisAgent->num_existing_wmes--;
}

Symbol* find_name_of_object(agent* thisAgent, Symbol* object)
{
    slot* s;

    if (object->symbol_type != IDENTIFIER_SYMBOL_TYPE)
    {
        return NIL;
    }
    s = find_slot(object, thisAgent->symbolManager->soarSymbols.name_symbol);
    if (! s)
    {
        return NIL;
    }
    if (! s->wmes)
    {
        return NIL;
    }
    return s->wmes->value;
}

