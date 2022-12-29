/*************************************************************************
 *
 *  file:  io.cpp
 *
 * =======================================================================
 *
 *
 *                General Soar I/O System Routines
 *
 * User-defined Soar I/O routines should be added at system startup time
 * via calls to add_input_function() and add_output_function().  These
 * calls add things to the system's list of (1) functions to be called
 * every input cycle, and (2) symbol-to-function mappings for output
 * commands.  File io.cpp contains the system I/O mechanism itself (i.e.,
 * the stuff that calls the input and output functions), plus the text
 * I/O routines.
 *
 * Init_soar_io() does what it says.  Do_input_cycle() and do_output_cycle()
 * perform the entire input and output cycles -- these routines are called
 * once per elaboration cycle.  (once per Decision cycle in Soar 8).
 * The output module is notified about WM changes via a call to
 * inform_output_module_of_wm_changes().
 *
 * =======================================================================
 */

#include "io_link.h"

#include "agent.h"
#include "callback.h"
#include "decide.h"
#include "lexer.h"
#include "output_manager.h"
#include "print.h"
#include "production.h"
#include "slot.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "working_memory.h"
#include "working_memory_activation.h"
#include "xml.h"

#include <cassert>
#include <ctype.h>
#include "run_soar.h"
#include <stdlib.h>

using namespace soar_TraceNames;

void gds_invalid_so_remove_goal(agent* thisAgent, wme* w);

/* ====================================================================
                  Adding New Input and Output Functions

   The system maintains a list of all the input functions to be called
   every input cycle, and another list of all the symbol-to-function
   mappings for output commands.  Add_input_function() and
   add_output_function() should be called at system startup time to
   install each I/O function.
==================================================================== */

void add_input_function(agent* thisAgent, soar_callback_fn f,
                        soar_callback_data cb_data,
                        soar_callback_free_fn free_fn, const char* name)
{
    soar_add_callback(thisAgent, INPUT_PHASE_CALLBACK, f, INPUT_PHASE_CALLBACK, cb_data, free_fn, name);
}

void remove_input_function(agent* thisAgent, const char* name)
{
    soar_remove_callback(thisAgent, INPUT_PHASE_CALLBACK, name);
}

void add_output_function(agent* thisAgent,
                         soar_callback_fn f,
                         soar_callback_data cb_data,
                         soar_callback_free_fn free_fn,
                         int eventID,
                         const char* output_link_name)
{
    if (soar_exists_callback_id(thisAgent, OUTPUT_PHASE_CALLBACK, output_link_name)
            != NULL)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Error: tried to add_output_function with duplicate name %s\n",
              output_link_name);
        /* Replaced deprecated control_c_handler with an appropriate assertion */
        //control_c_handler(0);
        assert(0 && "error in io.cpp (control_c_handler() used to be called here)");
    }
    else
    {
        soar_add_callback(thisAgent, OUTPUT_PHASE_CALLBACK, f, eventID, cb_data, free_fn,
                          output_link_name);
    }
}

void remove_output_function(agent* thisAgent, const char* name)
{
    soar_callback* cb;
    output_link* ol;

    /* Remove indexing structures ... */

    cb = soar_exists_callback_id(thisAgent, OUTPUT_PHASE_CALLBACK, name);
    if (!cb)
    {
        return;
    }

    for (ol = thisAgent->existing_output_links; ol != NIL; ol = ol->next)
    {
        if (ol->cb == cb)
        {
            /* Remove ol entry */
            ol->link_wme->output_link = NULL;
            wme_remove_ref(thisAgent, ol->link_wme);
            remove_from_dll(thisAgent->existing_output_links, ol, next, prev);
            thisAgent->memoryManager->free_with_pool(MP_output_link, ol);
            break;
        }
    }

    soar_remove_callback(thisAgent, OUTPUT_PHASE_CALLBACK, name);
}

/* ====================================================================
                            Input Routines

   Get_new_io_identifier(), get_io_str_constant(), get_io_int_constant(),
   and get_io_float_constant() just call the appropriate symbol table
   routines.  This has the effect of incrementing the reference count
   on the symbol (or creating one with a reference count of 1).
   Release_io_symbol() just decrements the reference count.

   Add_input_wme() and remove_input_wme() call the add_wme_to_wm() and
   remove_wme_from_wm() routines in decide.cpp to do their work.

   Do_input_cycle() is the top-level routine which calls all the
   individual user-defined input functions, etc.

   All this stuff is really simple, and consequently pretty vulnerable
   to buggy user-written I/O code.  A more sophisticated version would
   be bullet-proofed against bad arguments to get_xxx(), add_input_wme(),
   and remove_input_wme().  Right now add_input_wme() and remove_input_wme()
   do some error checking, but they're nowhere near bullet-proof.
==================================================================== */

Symbol* get_new_io_identifier(agent* thisAgent, char first_letter)
{
    return thisAgent->symbolManager->make_new_identifier(first_letter, TOP_GOAL_LEVEL, NIL);
}

Symbol* get_io_identifier(agent* thisAgent, char first_letter, uint64_t number)
{
    Symbol* id = thisAgent->symbolManager->find_identifier(first_letter, number) ;

    // DJP: The other "make_<type>" methods either make a new object or incremenent the refence
    // on an existing object.  So I'm going to make this method function the same way for identifiers.
    if (id)
    {
        thisAgent->symbolManager->symbol_add_ref(id);
    }
    else
    {
        id = thisAgent->symbolManager->make_new_identifier(first_letter, TOP_GOAL_LEVEL, NIL);
    }

    return id ;
}

Symbol* get_io_str_constant(agent* thisAgent, char const* name)
{
    return thisAgent->symbolManager->make_str_constant(name);
}

Symbol* get_io_int_constant(agent* thisAgent, int64_t value)
{
    return thisAgent->symbolManager->make_int_constant(value);
}

Symbol* get_io_float_constant(agent* thisAgent, double value)
{
    return thisAgent->symbolManager->make_float_constant(value);
}

void release_io_symbol(agent* thisAgent, Symbol* sym)
{
    thisAgent->symbolManager->symbol_remove_ref(&sym);
}

wme* add_input_wme(agent* thisAgent, Symbol* id, Symbol* attr, Symbol* value)
{
    wme* w;

    /* --- a little bit of error checking --- */
    if (!(id && attr && value))
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Error: an input routine gave a NULL argument to add_input_wme.\n");
        return NIL;
    }

    /* --- go ahead and add the wme --- */
    w = make_wme(thisAgent, id, attr, value, false);
    insert_at_head_of_dll(id->id->input_wmes, w, next, prev);

    if (wma_enabled(thisAgent))
    {
        wma_activate_wme(thisAgent, w);
    }

    add_wme_to_wm(thisAgent, w);

    //PrintDebugFormat("Added wme with timetag %d to id %c%d ",w->timetag,id->id->name_letter,id->id->name_number) ;

    return w;
}

wme* find_input_wme_by_timetag_from_id(agent* thisAgent, Symbol* idSym, uint64_t timetag, tc_number tc)
{
    wme* pWME, *w;

    //PrintDebugFormat("Scanning id %c%d", idSym->id->name_letter, idSym->id->name_number) ;

    // Mark this id as having been visited (the key here is that tc numbers always increase so tc_num must be < tc until it's marked)
    idSym->id->tc_num = tc ;

    // This is inefficient.  Using a hash table could save a lot here.
    for (pWME = idSym->id->input_wmes; pWME != NIL; pWME = pWME->next)
    {
        //PrintDebugFormat("Timetag %" SCNu64, pWME->timetag) ;
        if (pWME->timetag == timetag)
        {
            return pWME ;
        }

        // NOTE: The test for the tc_num keeps us from getting stuck in loops within graphs
        if (pWME->value->symbol_type == IDENTIFIER_SYMBOL_TYPE && pWME->value->id->tc_num != tc)
        {
            w = find_input_wme_by_timetag_from_id(thisAgent, pWME->value, timetag, tc) ;
            if (w)
            {
                return w ;
            }
        }
    }

    return NIL ;
}

bool remove_input_wme(agent* thisAgent, wme* w)
{
    wme* temp;

    /* --- a little bit of error checking --- */
    if (!w)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Error: an input routine called remove_input_wme on a NULL wme.\n");
        return false;
    }
    for (temp = w->id->id->input_wmes; temp != NIL; temp = temp->next)
        if (temp == w)
        {
            break;
        }
    if (!temp)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Error: an input routine called remove_input_wme on a wme that\n");
        thisAgent->outputManager->printa_sf(thisAgent,  "isn't one of the input wmes currently in working memory.\n");
        return false;
    }
    /* Note: for efficiency, it might be better to use a hash table for the
    above test, rather than scanning the linked list.  We could have one
    global hash table for all the input wmes in the system. */
    /* --- go ahead and remove the wme --- */
    remove_from_dll(w->id->id->input_wmes, w, next, prev);
    /* REW: begin 09.15.96 */
    if (w->gds)
    {
        if (w->gds->goal != NIL)
        {
            gds_invalid_so_remove_goal(thisAgent, w);
            /* NOTE: the call to remove_wme_from_wm will take care
            of checking if GDS should be removed */
        }
    }
    /* REW: end   09.15.96 */

    remove_wme_from_wm(thisAgent, w);

    return true;
}


void do_input_cycle(agent* thisAgent)
{

    if (thisAgent->prev_top_state && (!thisAgent->top_state))
    {
        /* --- top state was just removed --- */
        soar_invoke_callbacks(thisAgent, INPUT_PHASE_CALLBACK,
                              reinterpret_cast<soar_call_data>(TOP_STATE_JUST_REMOVED));
        release_io_symbol(thisAgent, thisAgent->io_header);
        release_io_symbol(thisAgent, thisAgent->io_header_input);
        release_io_symbol(thisAgent, thisAgent->io_header_output);
        thisAgent->io_header = NIL;       /* RBD added 3/25/95 */
        thisAgent->io_header_input = NIL;       /* RBD added 3/25/95 */
        thisAgent->io_header_output = NIL;       /* KJC added 3/3/99 */
        thisAgent->io_header_link = NIL;  /* KJC added 3/3/99 */
    }
    else if ((!thisAgent->prev_top_state) && thisAgent->top_state)
    {
        /* --- top state was just created --- */
        /* Create io structure on top state. */
        /*
          thisAgent->io_header = get_new_io_identifier (thisAgent, 'I');
          thisAgent->io_header_link = add_input_wme (thisAgent,
          thisAgent->top_state,
          thisAgent->io_symbol,
          thisAgent->io_header);
          thisAgent->io_header_input = get_new_io_identifier (thisAgent, 'I');
          thisAgent->io_header_output = get_new_io_identifier (thisAgent, 'I');
          add_input_wme (thisAgent, thisAgent->io_header,
          thisAgent->symbolManager->make_str_constant("input-link"),
          thisAgent->io_header_input);
          add_input_wme (thisAgent, thisAgent->io_header,
          thisAgent->symbolManager->make_str_constant("output-link"),
          thisAgent->io_header_output);
        */
        /* --- add top state io link before calling input phase callback so
         * --- code can use "wmem" command.
         */
        /*
          do_buffered_wm_and_ownership_changes(thisAgent);

          soar_invoke_callbacks(thisAgent, thisAgent, INPUT_PHASE_CALLBACK,
          (soar_call_data) TOP_STATE_JUST_CREATED);
        */
    }

    /* --- if there is a top state, do the normal input cycle --- */

    if (thisAgent->top_state)
    {
        soar_invoke_callbacks(thisAgent, INPUT_PHASE_CALLBACK,
                              reinterpret_cast<soar_call_data>(NORMAL_INPUT_CYCLE));
    }

    /* --- do any WM resulting changes --- */
    do_buffered_wm_and_ownership_changes(thisAgent);

    /* --- save current top state for next time --- */
    thisAgent->prev_top_state = thisAgent->top_state;

    /* --- reset the output-link status flag to false
     * --- when running til output, only want to stop if agent
     * --- does add-wme to output.  don't stop if add-wme done
     * --- during input cycle (eg simulator updates sensor status)
     *     KJC 11/23/98
     */
    thisAgent->output_link_changed = false;

}

/* ====================================================================
                          Output Routines

   Inform_output_module_of_wm_changes() and do_output_cycle() are the
   two top-level entry points to the output routines.  The former is
   called by the working memory manager, and the latter from the top-level
   phase sequencer.

   This module maintains information about all the existing output links
   and the identifiers and wmes that are in the transitive closure of them.
   On each output link wme, we put a pointer to an output_link structure.
   Whenever inform_output_module_of_wm_changes() is called, we look for
   new output links and modifications/removals of old ones, and update
   the output_link structures accordingly.

   Transitive closure information is kept as follows:  each output_link
   structure has a list of all the ids in the link's TC.  Each id in
   the system has a list of all the output_link structures that it's
   in the TC of.

   After some number of calls to inform_output_module_of_wm_changes(),
   eventually do_output_cycle() gets called.  It scans through the list
   of output links and calls the necessary output function for each
   link that has changed in some way (add/modify/remove).
==================================================================== */

/* --- output link statuses --- */
#define NEW_OL_STATUS 0                    /* just created it */
#define UNCHANGED_OL_STATUS 1              /* normal status */
#define MODIFIED_BUT_SAME_TC_OL_STATUS 2   /* some value in its TC has been
                                              modified, but the ids in its TC
                                              are the same */
#define MODIFIED_OL_STATUS 3               /* the set of ids in its TC has
                                              changed */
#define REMOVED_OL_STATUS 4                /* link has just been removed */

/* --------------------------------------------------------------------
                   Output Link Status Updates on WM Changes

   Top-state link changes:

     For wme addition: (top-state ^link-attr anything)
        create new output_link structure; mark it "new"
     For wme removal:  (top-state ^link-attr anything)
        mark the output_link "removed"

   TC of existing link changes:

     For wme addition or removal: (<id> ^att constant):
       for each link in associated_output_links(id),
         mark link "modified but same tc" (unless it's already marked
         some other more serious way)

     For wme addition or removal: (<id> ^att <id2>):
       for each link in associated_output_links(id),
         mark link "modified" (unless it's already marked
         some other more serious way)

   Note that we don't update all the TC information after every WM change.
   The TC info doesn't get updated until do_output_cycle() is called.
-------------------------------------------------------------------- */

#define LINK_NAME_SIZE 1024
void update_for_top_state_wme_addition(agent* thisAgent, wme* w)
{
    output_link* ol;
    soar_callback* cb;
    char link_name[LINK_NAME_SIZE];

    /* --- check whether the attribute is an output function --- */
    w->attr->to_string(false, false, link_name, LINK_NAME_SIZE);
    cb = soar_exists_callback_id(thisAgent, OUTPUT_PHASE_CALLBACK, link_name);
    if (!cb)
    {
        return;
    }

    /* --- create new output link structure --- */
    thisAgent->memoryManager->allocate_with_pool(MP_output_link, &ol);
    insert_at_head_of_dll(thisAgent->existing_output_links, ol, next, prev);

    ol->status = NEW_OL_STATUS;
    ol->link_wme = w;
    wme_add_ref(w, true);
    ol->ids_in_tc = NIL;
    ol->cb = cb;
    /* --- make wme point to the structure --- */
    w->output_link = ol;

    /* SW 07 10 2003
       previously, this wouldn't be done until the first OUTPUT phase.
       However, if we add an output command in the 1st decision cycle,
       Soar seems to ignore it.

       There may be two things going on, the first having to do with the tc
       calculation, which may get done too late, in such a way that the
       initial calculation includes the command.  The other thing appears
       to be that some data structures are not initialized until the first
       output phase.  Namely, id->associated_output_links does not seem
       reflect the current output links until the first output-phase.

       To get past these issues, we fake a transitive closure calculation
       with the knowledge that the only thing on the output link at this
       point is the output-link identifier itself.  This way, we capture
       a snapshot of the empty output link, so Soar can detect any changes
       that might occur before the first output_phase. */

    /* KJC & RPM 10/06 commenting out SW's change.
       See near end of init_agent_memory for details  */
    //thisAgent->output_link_tc_num = get_new_tc_number(thisAgent);
    //ol->link_wme->value->id->tc_num = thisAgent->output_link_tc_num;
    //thisAgent->output_link_for_tc = ol;
    ///* --- add output_link to id's list --- */
    //push(thisAgent, thisAgent->output_link_for_tc, ol->link_wme->value->id->associated_output_links);
}

void update_for_top_state_wme_removal(wme* w)
{
    if (! w->output_link)
    {
        return;
    }
    w->output_link->status = REMOVED_OL_STATUS;
}

void update_for_io_wme_change(wme* w)
{
    cons* c;
    output_link* ol;

    for (c = w->id->id->associated_output_links; c != NIL; c = c->rest)
    {
        ol = static_cast<output_link_struct*>(c->first);
        if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        {
            /* --- mark ol "modified" --- */
            if ((ol->status == UNCHANGED_OL_STATUS) ||
                    (ol->status == MODIFIED_BUT_SAME_TC_OL_STATUS))
            {
                ol->status = MODIFIED_OL_STATUS;
            }
        }
        else
        {
            /* --- mark ol "modified but same tc" --- */
            if (ol->status == UNCHANGED_OL_STATUS)
            {
                ol->status = MODIFIED_BUT_SAME_TC_OL_STATUS;
            }
        }
    }
}

void inform_output_module_of_wm_changes(agent* thisAgent,
                                        cons* wmes_being_added,
                                        cons* wmes_being_removed)
{
    cons* c;
    wme* w;

    /* if wmes are added, set flag so can stop when running til output */
    for (c = wmes_being_added; c != NIL; c = c->rest)
    {
        w = static_cast<wme_struct*>(c->first);
        if (w->id == thisAgent->io_header)
        {
            update_for_top_state_wme_addition(thisAgent, w);
            thisAgent->output_link_changed = true; /* KJC 11/23/98 */
            thisAgent->d_cycle_last_output = thisAgent->d_cycle_count;   /* KJC 11/17/05 */
        }
        if (w->id->id->associated_output_links)
        {
            update_for_io_wme_change(w);
            thisAgent->output_link_changed = true; /* KJC 11/23/98 */
            thisAgent->d_cycle_last_output = thisAgent->d_cycle_count;   /* KJC 11/17/05 */
        }

#if DEBUG_RTO
        else
        {
            char id[100];

            w->id->to_string(false, false, id, 100);
            if (!strcmp(id, "I3"))
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "--> Added to I3, but doesn't register as an OL change!");
            }
        }
#endif

    }
    for (c = wmes_being_removed; c != NIL; c = c->rest)
    {
        w = static_cast<wme_struct*>(c->first);
        if (w->id == thisAgent->io_header)
        {
            update_for_top_state_wme_removal(w);
        }
        if (w->id->id->associated_output_links)
        {
            update_for_io_wme_change(w);
        }
    }
}

/* --------------------------------------------------------------------
                     Updating Link TC Information

   We make no attempt to do the TC updating intelligently.  Whenever the
   TC changes, we throw away all the old TC info and recalculate the new
   TC from scratch.  I figure that this part of the system won't get
   used very frequently and I hope it won't be a time hog.

   Remove_output_link_tc_info() and calculate_output_link_tc_info() are
   the main routines here.
-------------------------------------------------------------------- */

void remove_output_link_tc_info(agent* thisAgent, output_link* ol)
{
    cons* c, *prev_c;
    Symbol* id;

    while (ol->ids_in_tc)    /* for each id in the old TC... */
    {
        c = ol->ids_in_tc;
        ol->ids_in_tc = c->rest;
        id = static_cast<symbol_struct*>(c->first);
        free_cons(thisAgent, c);

        /* --- remove "ol" from the list of associated_output_links(id) --- */
        prev_c = NIL;
        for (c = id->id->associated_output_links; c != NIL; prev_c = c, c = c->rest)
            if (c->first == ol)
            {
                break;
            }
        if (!c)
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg, "io.c: Internal error: can't find output link in id's list\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }
        if (prev_c)
        {
            prev_c->rest = c->rest;
        }
        else
        {
            id->id->associated_output_links = c->rest;
        }
        free_cons(thisAgent, c);
        thisAgent->symbolManager->symbol_remove_ref(&id);
    }
}


void add_id_to_output_link_tc(agent* thisAgent, Symbol* id)
{
    slot* s;
    wme* w;

    /* --- if id is already in the TC, exit --- */
    if (id->tc_num == thisAgent->output_link_tc_num)
    {
        return;
    }
    id->tc_num = thisAgent->output_link_tc_num;


    /* --- add id to output_link's list --- */
    push(thisAgent, id, thisAgent->output_link_for_tc->ids_in_tc);
    thisAgent->symbolManager->symbol_add_ref(id);  /* make sure the id doesn't get deallocated before we
                           have a chance to free the cons cell we just added */

    /* --- add output_link to id's list --- */
    push(thisAgent, thisAgent->output_link_for_tc, id->id->associated_output_links);

    /* --- do TC through working memory --- */
    /* --- scan through all wmes for all slots for this id --- */
    for (w = id->id->input_wmes; w != NIL; w = w->next)
        if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        {
            add_id_to_output_link_tc(thisAgent, w->value);
        }
    for (s = id->id->slots; s != NIL; s = s->next)
        for (w = s->wmes; w != NIL; w = w->next)
            if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
            {
                add_id_to_output_link_tc(thisAgent, w->value);
            }
    /* don't need to check impasse_wmes, because we couldn't have a pointer
       to a goal or impasse identifier */
}

void calculate_output_link_tc_info(agent* thisAgent, output_link* ol)
{
    /* --- if link doesn't have any substructure, there's no TC --- */
    if (ol->link_wme->value->symbol_type != IDENTIFIER_SYMBOL_TYPE)
    {
        return;
    }

    /* --- do TC starting with the link wme's value --- */
    thisAgent->output_link_for_tc = ol;
    thisAgent->output_link_tc_num = get_new_tc_number(thisAgent);
    add_id_to_output_link_tc(thisAgent, ol->link_wme->value);
}

/* --------------------------------------------------------------------
                    Building the list of IO_Wme's

   These routines create and destroy the list of io_wme's in the TC
   of a given output_link.  Get_io_wmes_for_output_link() and
   deallocate_io_wme_list() are the main entry points.  The TC info
   must have already been calculated for the given output link before
   get_io_wmes_for_output_link() is called.
-------------------------------------------------------------------- */

void add_wme_to_collected_io_wmes(agent* thisAgent, wme* w)
{
    io_wme* New;

    thisAgent->memoryManager->allocate_with_pool(MP_io_wme, &New);
    New->next = thisAgent->collected_io_wmes;
    thisAgent->collected_io_wmes = New;
    New->id = w->id;
    New->attr = w->attr;
    New->value = w->value;
    New->timetag = w->timetag ;
}

io_wme* get_io_wmes_for_output_link(agent* thisAgent, output_link* ol)
{
    cons* c;
    Symbol* id;
    slot* s;
    wme* w;

    thisAgent->collected_io_wmes = NIL;
    add_wme_to_collected_io_wmes(thisAgent, ol->link_wme);
    for (c = ol->ids_in_tc; c != NIL; c = c->rest)
    {
        id = static_cast<symbol_struct*>(c->first);
        for (w = id->id->input_wmes; w != NIL; w = w->next)
        {
            add_wme_to_collected_io_wmes(thisAgent, w);
        }
        for (s = id->id->slots; s != NIL; s = s->next)
            for (w = s->wmes; w != NIL; w = w->next)
            {
                add_wme_to_collected_io_wmes(thisAgent, w);
            }
    }
    return thisAgent->collected_io_wmes;
}

void deallocate_io_wme_list(agent* thisAgent, io_wme* iw)
{
    io_wme* next;

    while (iw)
    {
        next = iw->next;
        thisAgent->memoryManager->free_with_pool(MP_io_wme, iw);
        iw = next;
    }
}

/* --------------------------------------------------------------------
                           Do Output Cycle

   This routine is called from the top-level sequencer, and it performs
   the whole output phase.  It scans through the list of existing output
   links, and takes the appropriate action on each one that's changed.
-------------------------------------------------------------------- */

/* Struct used to pass output data to callback functions */

void do_output_cycle(agent* thisAgent)
{
    output_link* ol, *next_ol;
    io_wme* iw_list;
    output_call_info output_call_data;

    for (ol = thisAgent->existing_output_links; ol != NIL; ol = next_ol)
    {
        next_ol = ol->next;

        switch (ol->status)
        {
            case UNCHANGED_OL_STATUS:
                /* --- output link is unchanged, so do nothing --- */
                break;

            case NEW_OL_STATUS:
                /* --- calculate tc, and call the output function --- */
                calculate_output_link_tc_info(thisAgent, ol);
                iw_list = get_io_wmes_for_output_link(thisAgent, ol);
                output_call_data.mode = ADDED_OUTPUT_COMMAND;
                output_call_data.outputs = iw_list;
#ifndef NO_TIMING_STUFF     /* moved here from do_one_top_level_phase June 05.  KJC */
                thisAgent->timers_phase.stop();
                thisAgent->timers_kernel.stop();
                thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
                thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
                thisAgent->timers_kernel.start();
#endif
                if (ol->cb)
                {
                    (ol->cb->function)(thisAgent, ol->cb->eventid, ol->cb->data, &output_call_data);
                }
#ifndef NO_TIMING_STUFF
                thisAgent->timers_kernel.stop();
                thisAgent->timers_output_function_cpu_time.update(thisAgent->timers_kernel);
                thisAgent->timers_kernel.start();
                thisAgent->timers_phase.start();
#endif
                deallocate_io_wme_list(thisAgent, iw_list);
                ol->status = UNCHANGED_OL_STATUS;
                break;

            case MODIFIED_BUT_SAME_TC_OL_STATUS:
                /* --- don't have to redo the TC, but do call the output function --- */
                iw_list = get_io_wmes_for_output_link(thisAgent, ol);
                output_call_data.mode = MODIFIED_OUTPUT_COMMAND;
                output_call_data.outputs = iw_list;
#ifndef NO_TIMING_STUFF     /* moved here from do_one_top_level_phase June 05.  KJC */
                thisAgent->timers_phase.stop();
                thisAgent->timers_kernel.stop();
                thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
                thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
                thisAgent->timers_kernel.start();
#endif
                if (ol->cb)
                {
                    (ol->cb->function)(thisAgent, ol->cb->eventid, ol->cb->data, &output_call_data);
                }
#ifndef NO_TIMING_STUFF
                thisAgent->timers_kernel.stop();
                thisAgent->timers_output_function_cpu_time.update(thisAgent->timers_kernel);
                thisAgent->timers_kernel.start();
                thisAgent->timers_phase.start();
#endif
                deallocate_io_wme_list(thisAgent, iw_list);
                ol->status = UNCHANGED_OL_STATUS;
                break;

            case MODIFIED_OL_STATUS:
                /* --- redo the TC, and call the output function */
                remove_output_link_tc_info(thisAgent, ol);
                calculate_output_link_tc_info(thisAgent, ol);
                iw_list = get_io_wmes_for_output_link(thisAgent, ol);
                output_call_data.mode = MODIFIED_OUTPUT_COMMAND;
                output_call_data.outputs = iw_list;
#ifndef NO_TIMING_STUFF     /* moved here from do_one_top_level_phase June 05.  KJC */
                thisAgent->timers_phase.stop();
                thisAgent->timers_kernel.stop();
                thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
                thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
                thisAgent->timers_kernel.start();
#endif
                if (ol->cb)
                {
                    (ol->cb->function)(thisAgent, ol->cb->eventid, ol->cb->data, &output_call_data);
                }
#ifndef NO_TIMING_STUFF
                thisAgent->timers_kernel.stop();
                thisAgent->timers_output_function_cpu_time.update(thisAgent->timers_kernel);
                thisAgent->timers_kernel.start();
                thisAgent->timers_phase.start();
#endif
                deallocate_io_wme_list(thisAgent, iw_list);
                ol->status = UNCHANGED_OL_STATUS;
                break;

            case REMOVED_OL_STATUS:
                /* --- call the output function, and free output_link structure --- */
                remove_output_link_tc_info(thisAgent, ol);             /* sets ids_in_tc to NIL */
                iw_list = get_io_wmes_for_output_link(thisAgent, ol);  /* gives just the link wme */
                output_call_data.mode = REMOVED_OUTPUT_COMMAND;
                output_call_data.outputs = iw_list;
#ifndef NO_TIMING_STUFF     /* moved here from do_one_top_level_phase June 05.  KJC */
                thisAgent->timers_phase.stop();
                thisAgent->timers_kernel.stop();
                thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
                thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
                thisAgent->timers_kernel.start();
#endif
                if (ol->cb)
                {
                    (ol->cb->function)(thisAgent, ol->cb->eventid, ol->cb->data, &output_call_data);
                }
#ifndef NO_TIMING_STUFF
                thisAgent->timers_kernel.stop();
                thisAgent->timers_output_function_cpu_time.update(thisAgent->timers_kernel);
                thisAgent->timers_kernel.start();
                thisAgent->timers_phase.start();
#endif
                deallocate_io_wme_list(thisAgent, iw_list);
                wme_remove_ref(thisAgent, ol->link_wme);
                remove_from_dll(thisAgent->existing_output_links, ol, next, prev);
                thisAgent->memoryManager->free_with_pool(MP_output_link, ol);
                break;
        }
    } /* end of for ol */

}

/* --------------------------------------------------------------------
                          Get Output Value

   This is a simple utility function for use in users' output functions.
   It finds things in an io_wme chain.  It takes "outputs" (the io_wme
   chain), and "id" and "attr" (symbols to match against the wmes), and
   returns the value from the first wme in the chain with a matching id
   and attribute.  Either "id" or "attr" (or both) can be specified as
   "don't care" by giving NULL (0) pointers for them instead of pointers
   to symbols.  If no matching wme is found, the function returns a
   NULL pointer.
-------------------------------------------------------------------- */

Symbol* get_output_value(io_wme* outputs, Symbol* id, Symbol* attr)
{
    io_wme* iw;

    for (iw = outputs; iw != NIL; iw = iw->next)
        if (((id == NIL) || (id == iw->id)) &&
                ((attr == NIL) || (attr == iw->attr)))
        {
            return iw->value;
        }
    return NIL;
}

/* ====================================================================

   Utilities that used to be part of text I/O, but we still need
   them now that text I/O is gone.

   Get_next_io_symbol_from_text_input_line() is used by the "accept"
   RHS function.

==================================================================== */

/* --------------------------------------------------------------------
                    Parsing a Line of Text Input

   Get_next_io_symbol_from_text_input_line (char **text_read_position) is
   the main text input parser.  It reads text from text_read_position
   and returns a (Symbol *) for the first item read.  It updates
   text_read_position to point to the next character not yet read.
   If end-of-line is reached without any symbol being read, NIL is
   returned.
-------------------------------------------------------------------- */

bool tio_constituent_char[256];
bool tio_whitespace[256];

Symbol* get_io_symbol_from_tio_constituent_string(agent* thisAgent, char* input_string)
{
    long int_val;
    double float_val;
    bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
    bool rereadable;

    soar::Lexer::determine_possible_symbol_types_for_string(input_string,
            strlen(input_string),
            &possible_id,
            &possible_var,
            &possible_sc,
            &possible_ic,
            &possible_fc,
            &rereadable);

    /* --- check whether it's an integer --- */
    if (possible_ic)
    {
        errno = 0;
        int_val = strtol(input_string, NULL, 10);
        if (errno)
        {
            thisAgent->outputManager->printa_sf(thisAgent,  "Text Input Error: bad integer (probably too large)\n");
            return NIL;
        }
        return get_io_int_constant(thisAgent, int_val);
    }

    /* --- check whether it's a floating point number --- */
    if (possible_fc)
    {
        errno = 0;
        float_val = strtod(input_string, NULL);
        if (errno)
        {
            thisAgent->outputManager->printa_sf(thisAgent,  "Text Input Error: bad floating point number\n");
            return NIL;
        }
        return get_io_float_constant(thisAgent, float_val);
    }

    /* --- otherwise it must be a symbolic constant --- */
    return get_io_str_constant(thisAgent, input_string);
}

#define MAX_TEXT_INPUT_LINE_LENGTH 1000 /* used to be in soarkernel.h */

Symbol* get_next_io_symbol_from_text_input_line(agent* thisAgent,
        char** text_read_position)
{
    char* ch;
    char input_string[MAX_TEXT_INPUT_LINE_LENGTH + 2];
    int input_lexeme_length;

    ch = *text_read_position;

    /* --- scan past any whitespace --- */
    while (tio_whitespace[static_cast<unsigned char>(*ch)])
    {
        ch++;
    }

    /* --- if end of line, return NIL --- */
    if ((*ch == '\n') || (*ch == 0))
    {
        *text_read_position = ch;
        return NIL;
    }

    /* --- if not a constituent character, return single-letter symbol --- */
    if (! tio_constituent_char[static_cast<unsigned char>(*ch)])
    {
        input_string[0] = *ch++;
        input_string[1] = 0;
        *text_read_position = ch;
        return get_io_str_constant(thisAgent, input_string);
    }

    /* --- read string of constituents --- */
    input_lexeme_length = 0;
    while (tio_constituent_char[static_cast<unsigned char>(*ch)])
    {
        input_string[input_lexeme_length++] = *ch++;
    }

    /* --- return the appropriate kind of symbol --- */
    input_string[input_lexeme_length] = 0;
    *text_read_position = ch;
    return get_io_symbol_from_tio_constituent_string(thisAgent, input_string);
}

/* ====================================================================

                   Initialization for Soar I/O

==================================================================== */

char extra_tio_constituents[] = "+-._";

void init_soar_io(agent* thisAgent)
{
    unsigned int i;

    thisAgent->memoryManager->init_memory_pool(MP_output_link, sizeof(output_link), "output link");
    thisAgent->memoryManager->init_memory_pool(MP_io_wme, sizeof(io_wme), "io wme");

    /* --- setup constituent_char array --- */
    for (i = 0; i < 256; i++)
    {
        tio_constituent_char[i] = (isalnum(i) != 0);
    }
    for (i = 0; i < strlen(extra_tio_constituents); i++)
    {
        tio_constituent_char[static_cast<int>(extra_tio_constituents[i])] = true;
    }

    /* --- setup whitespace array --- */
    for (i = 0; i < 256; i++)
    {
        tio_whitespace[i] = (isspace(i) != 0);
    }
    tio_whitespace[static_cast<int>('\n')] = false; /* for text i/o, crlf isn't whitespace */
}

