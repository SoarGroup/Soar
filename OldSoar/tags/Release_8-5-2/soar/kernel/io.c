/*************************************************************************
 *
 *  file:  io.c
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
 * commands.  File io.c contains the system I/O mechanism itself (i.e.,
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

/* ==================================================================
                         I/O Code for Soar 6

         General Soar I/O System Routines, and Text I/O Routines

   See comments in soarkernel.h for more information.
   ================================================================== */

#include "soarkernel.h"
#include <ctype.h>
#include <errno.h>
#include "soarapiUtils.h"

#ifdef _AIX                     /* excludeFromBuildInfo */
#include <sys/select.h>
#endif

extern void gds_invalid_so_remove_goal(wme * w);
void calculate_output_link_tc_info(output_link * ol);

/* ====================================================================
                            Input Routines

   Get_new_io_identifier(), get_io_sym_constant(), get_io_int_constant(),
   and get_io_float_constant() just call the appropriate symbol table
   routines.  This has the effect of incrementing the reference count
   on the symbol (or creating one with a reference count of 1).
   Release_io_symbol() just decrements the reference count.

   Add_input_wme() and remove_input_wme() call the add_wme_to_wm() and
   remove_wme_from_wm() routines in decide.c to do their work.  

   Do_input_cycle() is the top-level routine which calls all the
   individual user-defined input functions, etc.  

   All this stuff is really simple, and consequently pretty vulnerable
   to buggy user-written I/O code.  A more sophisticated version would
   be bullet-proofed against bad arguments to get_xxx(), add_input_wme(),
   and remove_input_wme().  Right now add_input_wme() and remove_input_wme()
   do some error checking, but they're nowhere near bullet-proof.
==================================================================== */

Symbol *get_new_io_identifier(char first_letter)
{

    return make_new_identifier(first_letter, TOP_GOAL_LEVEL);
}

Symbol *get_io_sym_constant(char *name)
{
    return make_sym_constant(name);
}

Symbol *get_io_int_constant(long value)
{
    return make_int_constant(value);
}

Symbol *get_io_float_constant(float value)
{
    return make_float_constant(value);
}

void release_io_symbol(Symbol * sym)
{
    symbol_remove_ref(sym);
}

wme *add_input_wme(Symbol * id, Symbol * attr, Symbol * value)
{
    wme *w;

    /* --- a little bit of error checking --- */
    if (!(id && attr && value)) {
        print("Error: an input routine gave a NULL argument to add_input_wme.\n");
        return NIL;
    }
    /* --- go ahead and add the wme --- */
    w = make_wme(id, attr, value, FALSE);
    insert_at_head_of_dll(id->id.input_wmes, w, next, prev);
    add_wme_to_wm(w);

    /* SW NOTE:
     * In an ideal world, we could capture input wmes here as well as
     * in soar_cAddWme.  However the problem is that based on the arguments
     * to this function it is impossible to determine if the wme which is
     * being added references new, or just previously defined symbols. 
     * As a result, we cannot capture input wmes asserted using this function
     * with any hope of accurately replaying them in the future.
     */

    return w;
}

bool remove_input_wme(wme * w)
{
    wme *temp;

    /* --- a little bit of error checking --- */
    if (!w) {
        print("Error: an input routine called remove_input_wme on a NULL wme.\n");
        return FALSE;
    }
    for (temp = w->id->id.input_wmes; temp != NIL; temp = temp->next)
        if (temp == w)
            break;
    if (!temp) {
        print("Error: an input routine called remove_input_wme on a wme that\n");
        print("isn't one of the input wmes currently in working memory.\n");
        return FALSE;
    }

    /* SW NOTE:
     * Since we cannot capture wmes asserted during add_input_wme,
     * we will not capture wmes removed during this function call.
     */

    /* Note: for efficiency, it might be better to use a hash table for the
       above test, rather than scanning the linked list.  We could have one
       global hash table for all the input wmes in the system. */
    /* --- go ahead and remove the wme --- */
    remove_from_dll(w->id->id.input_wmes, w, next, prev);
    /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode)) {
#endif
        if (w->gds) {
            if (w->gds->goal != NIL) {
                if (current_agent(soar_verbose_flag))
                    print("\nremove_input_wme: Removing goal %d because element in GDS changed.\n",
                          w->gds->goal->id.level);
                gds_invalid_so_remove_goal(w);
                /* NOTE: the call to remove_wme_from_wm will take care
                   of checking if GDS should be removed */
            }
        }
#ifndef SOAR_8_ONLY
    }
#endif

    /* REW: end   09.15.96 */

    remove_wme_from_wm(w);

    return TRUE;
}

#ifndef NO_IO_CALLBACKS

void do_input_cycle(void)
{
    wme *w;

#ifndef TRACE_CONTEXT_DECISIONS_ONLY
    if (current_agent(sysparams)[TRACE_PHASES_SYSPARAM])
        print("\n--- Input Phase --- \n");
#endif

    if (current_agent(prev_top_state) && (!current_agent(top_state))) {
        /* --- top state was just removed --- */
        release_io_symbol(current_agent(io_header));
        release_io_symbol(current_agent(io_header_input));
        release_io_symbol(current_agent(io_header_output));
        current_agent(io_header) = NIL; /* RBD added 3/25/95 */
        current_agent(io_header_input) = NIL;   /* RBD added 3/25/95 */
        current_agent(io_header_output) = NIL;  /* KJC added 3/3/99 */
        current_agent(io_header_link) = NIL;    /* KJC added 3/3/99 */
        soar_invoke_callbacks(soar_agent, INPUT_PHASE_CALLBACK, (soar_call_data) TOP_STATE_JUST_REMOVED);
    } else if ((!current_agent(prev_top_state)) && current_agent(top_state)) {
        /* --- top state was just created --- */
        /* Create io structure on top state. */
        current_agent(io_header) = get_new_io_identifier('I');
        current_agent(io_header_link) = add_input_wme(current_agent(top_state),
                                                      current_agent(io_symbol), current_agent(io_header));
        current_agent(io_header_input) = get_new_io_identifier('I');
        current_agent(io_header_output) = get_new_io_identifier('I');
        w = add_input_wme(current_agent(io_header), make_sym_constant("input-link"), current_agent(io_header_input));

        w = add_input_wme(current_agent(io_header), make_sym_constant("output-link"), current_agent(io_header_output));

        /* --- add top state io link before calling input phase callback so
         * --- code can use "wmem" command.
         */
        do_buffered_wm_and_ownership_changes();

        soar_invoke_callbacks(soar_agent, INPUT_PHASE_CALLBACK, (soar_call_data) TOP_STATE_JUST_CREATED);
    }

    /* --- if there is a top state, do the normal input cycle --- */

    if (current_agent(top_state)) {
        soar_invoke_callbacks(soar_agent, INPUT_PHASE_CALLBACK, (soar_call_data) NORMAL_INPUT_CYCLE);
    }

    /* --- do any WM resulting changes --- */
    do_buffered_wm_and_ownership_changes();

    /* --- save current top state for next time --- */
    current_agent(prev_top_state) = current_agent(top_state);

    /* --- reset the output-link status flag to FALSE
     * --- when running til output, only want to stop if agent
     * --- does add-wme to output.  don't stop if add-wme done
     * --- during input cycle (eg simulator updates sensor status)
     *     KJC 11/23/98
     */
    current_agent(output_link_changed) = FALSE;
}

#else
void do_input_cycle(void)
{

    /*
       I don't really think we need to do this,
       so I'll try not using it.
     */
    /* do_buffered_wm_and_ownership_changes(); */
    current_agent(prev_top_state) = current_agent(top_state);
    current_agent(output_link_changed) = FALSE;

}

#endif                          /* NO_IO_CALLBACKS */

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
#define NEW_OL_STATUS 0         /* just created it */
#define UNCHANGED_OL_STATUS 1   /* normal status */
#define MODIFIED_BUT_SAME_TC_OL_STATUS 2        /* some value in its TC has been
                                                   modified, but the ids in its TC
                                                   are the same */
#define MODIFIED_OL_STATUS 3    /* the set of ids in its TC has
                                   changed */
#define REMOVED_OL_STATUS 4     /* link has just been removed */

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
void update_for_top_state_wme_addition(wme * w)
{
    output_link *ol;
    soar_callback *cb;
    char link_name[LINK_NAME_SIZE];

    /* --- check whether the attribute is an output function --- */
    symbol_to_string(w->attr, FALSE, link_name, LINK_NAME_SIZE);

    cb = soar_exists_callback_id(soar_agent, OUTPUT_PHASE_CALLBACK, link_name);

    if (!cb)
        return;

    /* --- create new output link structure --- */
    allocate_with_pool(&current_agent(output_link_pool), &ol);
    insert_at_head_of_dll(current_agent(existing_output_links), ol, next, prev);

    ol->status = NEW_OL_STATUS;
    ol->link_wme = w;
    wme_add_ref(w);
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

    current_agent(output_link_tc_num) = get_new_tc_number();
    ol->link_wme->value->id.tc_num = current_agent(output_link_tc_num);
    current_agent(output_link_for_tc) = ol;
    /* --- add output_link to id's list --- */
    push(current_agent(output_link_for_tc), ol->link_wme->value->id.associated_output_links);

}

void update_for_top_state_wme_removal(wme * w)
{
    if (!w->output_link)
        return;
    w->output_link->status = REMOVED_OL_STATUS;
}

void update_for_io_wme_change(wme * w)
{
    cons *c;
    output_link *ol;

    for (c = w->id->id.associated_output_links; c != NIL; c = c->rest) {
        ol = c->first;
        if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) {
            /* --- mark ol "modified" --- */
            if ((ol->status == UNCHANGED_OL_STATUS) || (ol->status == MODIFIED_BUT_SAME_TC_OL_STATUS))
                ol->status = MODIFIED_OL_STATUS;
        } else {
            /* --- mark ol "modified but same tc" --- */
            if (ol->status == UNCHANGED_OL_STATUS)
                ol->status = MODIFIED_BUT_SAME_TC_OL_STATUS;
        }
    }
}

void inform_output_module_of_wm_changes(list * wmes_being_added, list * wmes_being_removed)
{
    cons *c;
    wme *w;

    /* if wmes are added, set flag so can stop when running til output */
    for (c = wmes_being_added; c != NIL; c = c->rest) {
        w = c->first;

        if (w->id == current_agent(io_header)) {
            update_for_top_state_wme_addition(w);
            current_agent(output_link_changed) = TRUE;  /* KJC 11/23/98 */
        }
        if (w->id->id.associated_output_links) {
            update_for_io_wme_change(w);
            current_agent(output_link_changed) = TRUE;  /* KJC 11/23/98 */
        }
#if DEBUG_RTO
        else {
            char id[100];

            symbol_to_string(w->id, FALSE, id);
            if (!strcmp(id, "I3")) {
                print("--> Added to I3, but doesn't register as an OL change!");
            }
        }
#endif

    }
    for (c = wmes_being_removed; c != NIL; c = c->rest) {
        w = c->first;
        if (w->id == current_agent(io_header))
            update_for_top_state_wme_removal(w);
        if (w->id->id.associated_output_links)
            update_for_io_wme_change(w);
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

void remove_output_link_tc_info(output_link * ol)
{
    cons *c, *prev_c;
    Symbol *id;

    while (ol->ids_in_tc) {     /* for each id in the old TC... */
        c = ol->ids_in_tc;
        ol->ids_in_tc = c->rest;
        id = c->first;
        free_cons(c);

        /* --- remove "ol" from the list of associated_output_links(id) --- */
        prev_c = NIL;
        for (c = id->id.associated_output_links; c != NIL; prev_c = c, c = c->rest)
            if (c->first == ol)
                break;
        if (!c) {
            char msg[MESSAGE_SIZE];
            strncpy(msg, "io.c: Internal error: can't find output link in id's list\n", MESSAGE_SIZE);
            msg[MESSAGE_SIZE - 1] = 0;
            abort_with_fatal_error(msg);
        }
        if (prev_c)
            prev_c->rest = c->rest;
        else
            id->id.associated_output_links = c->rest;
        free_cons(c);
        symbol_remove_ref(id);
    }
}

void add_id_to_output_link_tc(Symbol * id)
{
    slot *s;
    wme *w;

    /* --- if id is already in the TC, exit --- */
    if (id->id.tc_num == current_agent(output_link_tc_num))
        return;
    id->id.tc_num = current_agent(output_link_tc_num);

    /* --- add id to output_link's list --- */
    push(id, current_agent(output_link_for_tc)->ids_in_tc);
    symbol_add_ref(id);         /* make sure the id doesn't get deallocated before we
                                   have a chance to free the cons cell we just added */

    /* --- add output_link to id's list --- */
    push(current_agent(output_link_for_tc), id->id.associated_output_links);

    /* --- do TC through working memory --- */
    /* --- scan through all wmes for all slots for this id --- */
    for (w = id->id.input_wmes; w != NIL; w = w->next)
        if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
            add_id_to_output_link_tc(w->value);
    for (s = id->id.slots; s != NIL; s = s->next)
        for (w = s->wmes; w != NIL; w = w->next)
            if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
                add_id_to_output_link_tc(w->value);
    /* don't need to check impasse_wmes, because we couldn't have a pointer
       to a goal or impasse identifier */
}

void calculate_output_link_tc_info(output_link * ol)
{
    /* --- if link doesn't have any substructure, there's no TC --- */
    if (ol->link_wme->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
        return;

    /* --- do TC starting with the link wme's value --- */
    current_agent(output_link_for_tc) = ol;
    current_agent(output_link_tc_num) = get_new_tc_number();
    add_id_to_output_link_tc(ol->link_wme->value);
}

/* --------------------------------------------------------------------
                    Building the list of IO_Wme's

   These routines create and destroy the list of io_wme's in the TC
   of a given output_link.  Get_io_wmes_for_output_link() and
   deallocate_io_wme_list() are the main entry points.  The TC info
   must have already been calculated for the given output link before
   get_io_wmes_for_output_link() is called.
-------------------------------------------------------------------- */

void add_wme_to_collected_io_wmes(wme * w)
{
    io_wme *new;

    allocate_with_pool(&current_agent(io_wme_pool), &new);
    new->next = current_agent(collected_io_wmes);
    current_agent(collected_io_wmes) = new;
    new->id = w->id;
    new->attr = w->attr;
    new->value = w->value;
}

io_wme *get_io_wmes_for_output_link(output_link * ol)
{
    cons *c;
    Symbol *id;
    slot *s;
    wme *w;

    current_agent(collected_io_wmes) = NIL;
    add_wme_to_collected_io_wmes(ol->link_wme);
    for (c = ol->ids_in_tc; c != NIL; c = c->rest) {
        id = c->first;
        for (w = id->id.input_wmes; w != NIL; w = w->next)
            add_wme_to_collected_io_wmes(w);
        for (s = id->id.slots; s != NIL; s = s->next)
            for (w = s->wmes; w != NIL; w = w->next)
                add_wme_to_collected_io_wmes(w);
    }
    return current_agent(collected_io_wmes);
}

void deallocate_io_wme_list(io_wme * iw)
{
    io_wme *next;

    while (iw) {
        next = iw->next;
        free_with_pool(&current_agent(io_wme_pool), iw);
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
output_call_info output_call_data;

#ifndef NO_IO_CALLBACKS

void do_output_cycle(void)
{
    output_link *ol, *next_ol;
    io_wme *iw_list;

#ifndef TRACE_CONTEXT_DECISIONS_ONLY
    if (current_agent(sysparams)[TRACE_PHASES_SYSPARAM])
        print("\n--- Output Phase ---\n");
#endif

    for (ol = current_agent(existing_output_links); ol != NIL; ol = next_ol) {
        next_ol = ol->next;

        switch (ol->status) {
        case UNCHANGED_OL_STATUS:
            /* --- link is unchanged, so do nothing --- */
            break;

        case NEW_OL_STATUS:

            /* --- calculate tc, and call the output function --- */
            calculate_output_link_tc_info(ol);
            iw_list = get_io_wmes_for_output_link(ol);
            output_call_data.mode = ADDED_OUTPUT_COMMAND;
            output_call_data.outputs = iw_list;
            (ol->cb->function) (soar_agent, ol->cb->data, &output_call_data);
            deallocate_io_wme_list(iw_list);
            ol->status = UNCHANGED_OL_STATUS;
            break;

        case MODIFIED_BUT_SAME_TC_OL_STATUS:
            /* --- don't have to redo the TC, but do call the output function --- */
            iw_list = get_io_wmes_for_output_link(ol);
            output_call_data.mode = MODIFIED_OUTPUT_COMMAND;
            output_call_data.outputs = iw_list;
            (ol->cb->function) (soar_agent, ol->cb->data, &output_call_data);
            deallocate_io_wme_list(iw_list);
            ol->status = UNCHANGED_OL_STATUS;
            break;

        case MODIFIED_OL_STATUS:
            /* --- redo the TC, and call the output function */
            remove_output_link_tc_info(ol);
            calculate_output_link_tc_info(ol);
            iw_list = get_io_wmes_for_output_link(ol);
            output_call_data.mode = MODIFIED_OUTPUT_COMMAND;
            output_call_data.outputs = iw_list;
            (ol->cb->function) (soar_agent, ol->cb->data, &output_call_data);
            deallocate_io_wme_list(iw_list);
            ol->status = UNCHANGED_OL_STATUS;
            break;

        case REMOVED_OL_STATUS:
            /* --- call the output function, and free output_link structure --- */
            remove_output_link_tc_info(ol);     /* sets ids_in_tc to NIL */
            iw_list = get_io_wmes_for_output_link(ol);  /* gives just the link wme */
            output_call_data.mode = REMOVED_OUTPUT_COMMAND;
            output_call_data.outputs = iw_list;
            (ol->cb->function) (soar_agent, ol->cb->data, &output_call_data);
            deallocate_io_wme_list(iw_list);
            wme_remove_ref(ol->link_wme);
            remove_from_dll(current_agent(existing_output_links), ol, next, prev);
            free_with_pool(&current_agent(output_link_pool), ol);
            break;
        }
    }                           /* end of for ol */
}
#else

void do_output_cycle(void)
{
    output_link *ol, *next_ol;

    for (ol = current_agent(existing_output_links); ol != NIL; ol = next_ol) {
        next_ol = ol->next;

        switch (ol->status) {
        case UNCHANGED_OL_STATUS:
            /* --- link is unchanged, so do nothing --- */
            break;

        default:
            print("io.c: Error -- Output Link has changed, but kernel was built with NO_IO_CALLBACKS\n");
        }
    }
}
#endif

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

Symbol *get_output_value(io_wme * outputs, Symbol * id, Symbol * attr)
{
    io_wme *iw;

    for (iw = outputs; iw != NIL; iw = iw->next)
        if (((id == NIL) || (id == iw->id)) && ((attr == NIL) || (attr == iw->attr)))
            return iw->value;
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

Symbol *get_io_symbol_from_tio_constituent_string(char *input_string)
{
    int int_val;
    float float_val;
    bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
    bool rereadable;

    determine_possible_symbol_types_for_string(input_string,
                                               strlen(input_string),
                                               &possible_id,
                                               &possible_var, &possible_sc, &possible_ic, &possible_fc, &rereadable);

    /* --- check whether it's an integer --- */
    if (possible_ic) {
        errno = 0;
        int_val = strtol(input_string, NULL, 10);
        if (errno) {
            print("Text Input Error: bad integer (probably too large)\n");
            return NIL;
        }
        return get_io_int_constant(int_val);
    }

    /* --- check whether it's a floating point number --- */
    if (possible_fc) {
        errno = 0;
        /*float_val = (float) strtod (input_string,NULL,10); */
        float_val = (float) strtod(input_string, NULL);
        if (errno) {
            print("Text Input Error: bad floating point number\n");
            return NIL;
        }
        return get_io_float_constant(float_val);
    }

    /* --- otherwise it must be a symbolic constant --- */
    return get_io_sym_constant(input_string);
}

#define MAX_TEXT_INPUT_LINE_LENGTH 1000 /* used to be in soarkernel.h */

Symbol *get_next_io_symbol_from_text_input_line(char **text_read_position)
{
    char *ch;
    char input_string[MAX_TEXT_INPUT_LINE_LENGTH + 2];
    int input_lexeme_length;

    ch = *text_read_position;

    /* --- scan past any whitespace --- */
    while (tio_whitespace[(unsigned char) (*ch)])
        ch++;

    /* --- if end of line, return NIL --- */
    if ((*ch == '\n') || (*ch == 0)) {
        *text_read_position = ch;
        return NIL;
    }

    /* --- if not a constituent character, return single-letter symbol --- */
    if (!tio_constituent_char[(unsigned char) (*ch)]) {
        input_string[0] = *ch++;
        input_string[1] = 0;
        *text_read_position = ch;
        return get_io_sym_constant(input_string);
    }

    /* --- read string of constituents --- */
    input_lexeme_length = 0;
    while (tio_constituent_char[(unsigned char) (*ch)])
        input_string[input_lexeme_length++] = *ch++;

    /* --- return the appropriate kind of symbol --- */
    input_string[input_lexeme_length] = 0;
    *text_read_position = ch;
    return get_io_symbol_from_tio_constituent_string(input_string);
}

/* ====================================================================

                   Initialization for Soar I/O

==================================================================== */

char extra_tio_constituents[] = "+-._";

void init_soar_io(void)
{
    unsigned int i;

    init_memory_pool(&current_agent(output_link_pool), sizeof(output_link), "output link");
    init_memory_pool(&current_agent(io_wme_pool), sizeof(io_wme), "io wme");

    /* --- setup constituent_char array --- */
    for (i = 0; i < 256; i++)
        tio_constituent_char[i] = (char) isalnum(i);
    for (i = 0; i < strlen(extra_tio_constituents); i++)
        tio_constituent_char[(int) extra_tio_constituents[i]] = TRUE;

    /* --- setup whitespace array --- */
    for (i = 0; i < 256; i++)
        tio_whitespace[i] = (char) isspace(i);
    tio_whitespace[(int) '\n'] = FALSE; /* for text i/o, crlf isn't whitespace */
}
