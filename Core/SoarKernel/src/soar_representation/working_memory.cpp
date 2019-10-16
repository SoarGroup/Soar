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
#include "smem_timers.h"

#include <stdlib.h>

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

    wma_forget_pq = new wma_forget_p_queue();
    wma_touched_sets = new wma_decay_cycle_set();
    wma_touched_elements = new wme_set();
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
        thisAgent->outputManager->printa_sf(thisAgent,  "there are still %u wmes allocated. (Probably a memory leak.)\n", thisAgent->num_existing_wmes);
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
    w->is_singleton = false;
    w->singleton_status_checked = false;
    w->local_singleton_id_identity_set = NULL_IDENTITY_SET;
    w->local_singleton_value_identity_set = NULL_IDENTITY_SET;
    w->next = NIL;
    w->prev = NIL;
    w->rete_next = NIL;
    w->rete_prev = NIL;

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
    push(thisAgent, w, thisAgent->wmes_to_add);

    if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        post_link_addition(thisAgent, w->id, w->value);
        if (w->id->is_state() && (w->attr == thisAgent->symbolManager->soarSymbols.operator_symbol))
        {
            w->value->id->isa_operator++;
        }
    }
}

void remove_wme_from_wm(agent* thisAgent, wme* w)
{

    push(thisAgent, w, thisAgent->wmes_to_remove);

    if (w->value->is_sti())
    {
        post_link_removal(thisAgent, w->id, w->value);
        if (w->id->is_state() && w->attr == thisAgent->symbolManager->soarSymbols.operator_symbol)
        {
            w->value->id->isa_operator--;
        }
    }
    /* When we remove a WME, we always have to determine if it's on a GDS, and, if
    so, after removing the WME, if there are no longer any WMEs on the GDS,
    then we can free the GDS memory */
    if (w->gds)
    {
        fast_remove_from_dll(w->gds->wmes_in_gds, w, wme, gds_next, gds_prev);

        if (!w->gds->wmes_in_gds)
        {
            if (w->gds->goal) w->gds->goal->id->gds = NIL;
            thisAgent->memoryManager->free_with_pool(MP_gds, w->gds);
        }
    }
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
        }
        remove_wme_from_wm(thisAgent, w);

        w = next_w;
    }
}

void do_buffered_wm_changes(agent* thisAgent)
{
    cons* c, *next_c, *cr;
    wme* w;

    #ifndef NO_TIMING_STUFF
    #ifdef DETAILED_TIMING_STATS
    soar_timer local_timer;
    local_timer.set_enabled(&(thisAgent->timers_enabled));
    #endif
    #endif

    /* --- if no wme changes are buffered, do nothing --- */
    if (!thisAgent->wmes_to_add && !thisAgent->wmes_to_remove) return;

    /* --- call output module in case any changes are output link changes --- */
    inform_output_module_of_wm_changes(thisAgent, thisAgent->wmes_to_add, thisAgent->wmes_to_remove);

    /* --- invoke callback routine.  wmes_to_add and wmes_to_remove can   --- */
    /* --- be fetched from the agent structure.                           --- */
    soar_invoke_callbacks(thisAgent, WM_CHANGES_CALLBACK, 0);

    /* --- stuff wme changes through the rete net --- */
    #ifndef NO_TIMING_STUFF
    #ifdef DETAILED_TIMING_STATS
    local_timer.start();
    #endif
    #endif
    for (c = thisAgent->wmes_to_add; c != NIL; c = c->rest)
    {
        w = (wme_struct*)(c->first);
        #ifdef SPREADING_ACTIVATION_ENABLED
        if (w->id->symbol_type == IDENTIFIER_SYMBOL_TYPE && w->id->id->LTI_ID)
        {//We attempt to keep track of ltis currently in wmem.
            if (thisAgent->SMem->smem_in_wmem->find(w->id->id->LTI_ID) == thisAgent->SMem->smem_in_wmem->end())
            {
                (*(thisAgent->SMem->smem_in_wmem))[w->id->id->LTI_ID] = (uint64_t)1;
                //This must have been newly added.
                thisAgent->SMem->smem_context_additions->insert(w->id->id->LTI_ID);
                if (thisAgent->SMem->smem_context_removals->find(w->id->id->LTI_ID) != thisAgent->SMem->smem_context_removals->end())
                {
                    thisAgent->SMem->smem_context_removals->erase(w->id->id->LTI_ID);
                }
            }
            else
            {
                (*(thisAgent->SMem->smem_in_wmem))[w->id->id->LTI_ID] = (*(thisAgent->SMem->smem_in_wmem))[w->id->id->LTI_ID] + 1;
            }
        }
        #endif
        add_wme_to_rete(thisAgent, static_cast<wme_struct*>(c->first));
    }
    for (c = thisAgent->wmes_to_remove; c != NIL; c = c->rest)
    {//Instead of relying on existing code for managing wma_decay_elements, I can instead just here delete from smem the record of the decay element
        w = (wme_struct*)(c->first);
        #ifdef SPREADING_ACTIVATION_ENABLED
        if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE && w->value->id->LTI_ID)
        {
            thisAgent->SMem->timers->spreading_wma_2->start();
            auto wmas = thisAgent->SMem->smem_wmas->equal_range(w->value->id->LTI_ID);
            for (auto wma = wmas.first; wma != wmas.second; ++wma) //The equal_range gives a lower bound and upper bound iterator pair.
            {//The iterator itself is a pair for the key and the value. The value is the wma decay element pointer.
                if (wma->second == w->wma_decay_el)
                {
                    thisAgent->SMem->smem_wmas->erase(wma);//erase accepts an iterator to indicate what is to be erased.
                    break;
                }
            }
            thisAgent->SMem->timers->spreading_wma_2->stop();
        }
        if (w->id->symbol_type == IDENTIFIER_SYMBOL_TYPE && w->id->id->LTI_ID)
        {
            if (thisAgent->SMem->smem_in_wmem->find(w->id->id->LTI_ID) != thisAgent->SMem->smem_in_wmem->end())
            {
                if ((*(thisAgent->SMem->smem_in_wmem))[w->id->id->LTI_ID] == 1)
                {//We are removing the last instance = actual removal.
                    thisAgent->SMem->smem_in_wmem->erase(w->id->id->LTI_ID);
                    if (thisAgent->SMem->smem_context_additions->find(w->id->id->LTI_ID) != thisAgent->SMem->smem_context_additions->end())
                    {
                        thisAgent->SMem->smem_context_additions->erase(w->id->id->LTI_ID);
                    }
                    else
                    {
                        thisAgent->SMem->smem_context_removals->insert(w->id->id->LTI_ID);
                    }
                }
                else
                {//just reducing the number of instances.
                    (*(thisAgent->SMem->smem_in_wmem))[w->id->id->LTI_ID] = (*(thisAgent->SMem->smem_in_wmem))[w->id->id->LTI_ID] - 1;
                }
            }
        }
        #endif
        remove_wme_from_rete(thisAgent, static_cast<wme_struct*>(c->first));
    }
    #ifndef NO_TIMING_STUFF
    #ifdef DETAILED_TIMING_STATS
    local_timer.stop();
    thisAgent->timers_match_cpu_time[thisAgent->current_phase].update(local_timer);
    #endif
    #endif
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

    /* --- do tracing and cleanup stuff --- */
    for (c = thisAgent->wmes_to_add; c != NIL; c = next_c)
    {
        next_c = c->rest;
        w = static_cast<wme_struct*>(c->first);
        if (thisAgent->trace_settings[TRACE_WM_CHANGES_SYSPARAM])
        {
            filtered_print_wme_add(thisAgent, w);
        }

        wme_add_ref(w, true);
        free_cons(thisAgent, c);
        thisAgent->wme_addition_count++;
    }
    for (c = thisAgent->wmes_to_remove; c != NIL; c = next_c)
    {
        next_c = c->rest;
        w = static_cast<wme_struct*>(c->first);
        if (thisAgent->trace_settings[TRACE_WM_CHANGES_SYSPARAM])
        {
            filtered_print_wme_remove(thisAgent, w);
        }

        wme_remove_ref(thisAgent, w);
        free_cons(thisAgent, c);
        thisAgent->wme_removal_count++;
    }
    thisAgent->wmes_to_add = NIL;
    thisAgent->wmes_to_remove = NIL;
}

void deallocate_wme(agent* thisAgent, wme* w)
{
    if (wma_enabled(thisAgent)) wma_remove_decay_element(thisAgent, w);

    if (w->local_singleton_value_identity_set)
    {
        IdentitySet_remove_ref(thisAgent, w->local_singleton_id_identity_set);
        IdentitySet_remove_ref(thisAgent, w->local_singleton_value_identity_set);
    }
    thisAgent->symbolManager->symbol_remove_ref(&w->id);
    thisAgent->symbolManager->symbol_remove_ref(&w->attr);
    thisAgent->symbolManager->symbol_remove_ref(&w->value);
    thisAgent->memoryManager->free_with_pool(MP_wme, w);
    thisAgent->num_existing_wmes--;
}

Symbol* find_name_of_object(agent* thisAgent, Symbol* object)
{
    if (object->symbol_type != IDENTIFIER_SYMBOL_TYPE) return NIL;
    slot* s = find_slot(object, thisAgent->symbolManager->soarSymbols.name_symbol);
    if (! s) return NIL;
    if (! s->wmes) return NIL;
    return s->wmes->value;
}

