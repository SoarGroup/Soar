/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  ebc_build.cpp
 *
 * =======================================================================
 *  These are the routines that support printing Soar data structures.
 *
 * =======================================================================
 */

#include "ebc.h"
#include "debug.h"
#include "decide.h"
#include "init_soar.h"
#include "rete.h"
#include "explain.h"
#include "print.h"
#include "xml.h"
#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "symbol.h"
#include "test.h"
#include "working_memory.h"
#include "working_memory_activation.h"
#include "soar_TraceNames.h"

#include <stdlib.h>
#include <cstring>
#include <ctype.h>

using namespace soar_TraceNames;

/* =====================================================================

                           Results Calculation

   Get_results_for_instantiation() finds and returns the result preferences
   for a given instantiation.  This is the main routine here.

   The results are accumulated in the list "results," linked via the
   "next_result" field of the preference structures.  (NOTE: to save
   space, just use conses for this.)

   Add_pref_to_results() adds a preference to the results.
   Add_results_for_id() adds any preferences for the given identifier.
   Identifiers are marked with results_tc_number as they are added.
===================================================================== */
void Explanation_Based_Chunker::add_results_if_needed(Symbol* sym)
{
    if ((sym)->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        if (((sym)->id->level >= results_match_goal_level) &&
                ((sym)->tc_num != results_tc_number))
        {
            add_results_for_id(sym);
        }
}

void Explanation_Based_Chunker::add_pref_to_results(preference* pref)
{
    preference* p;

    /* --- if an equivalent pref is already a result, don't add this one --- */
    for (p = results; p != NIL; p = p->next_result)
    {
        if (p->id != pref->id)
        {
            continue;
        }
        if (p->attr != pref->attr)
        {
            continue;
        }
        if (p->value != pref->value)
        {
            continue;
        }
        if (p->type != pref->type)
        {
            continue;
        }
        if (preference_is_unary(pref->type))
        {
            return;
        }
        if (p->referent != pref->referent)
        {
            continue;
        }
        return;
    }

    /* --- if pref isn't at the right level, find a clone that is --- */
    if (pref->inst->match_goal_level != results_match_goal_level)
    {
        for (p = pref->next_clone; p != NIL; p = p->next_clone)
            if (p->inst->match_goal_level == results_match_goal_level)
            {
                break;
            }
        if (!p)
            for (p = pref->prev_clone; p != NIL; p = p->prev_clone)
                if (p->inst->match_goal_level == results_match_goal_level)
                {
                    break;
                }
        if (!p)
        {
            return;    /* if can't find one, it isn't a result */
        }
        pref = p;
    }

    /* --- add this preference to the result list --- */
    pref->next_result = results;
    results = pref;

    /* --- follow transitive closure through value, referent links --- */
    add_results_if_needed(pref->value);
    if (preference_is_binary(pref->type))
    {
        add_results_if_needed(pref->referent);
    }
}

void Explanation_Based_Chunker::add_results_for_id(Symbol* id)
{
    slot* s;
    preference* pref;
    wme* w;

    id->tc_num = results_tc_number;

    /* --- scan through all preferences and wmes for all slots for this id --- */
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        add_results_if_needed(w->value);
    }
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
        {
            add_pref_to_results(pref);
        }
        for (w = s->wmes; w != NIL; w = w->next)
        {
            add_results_if_needed(w->value);
        }
    } /* end of for slots loop */
    /* --- now scan through extra prefs and look for any with this id --- */
    for (pref = extra_result_prefs_from_instantiation; pref != NIL;
            pref = pref->inst_next)
    {
        if (pref->id == id)
        {
            add_pref_to_results(pref);
        }
    }
}

preference* Explanation_Based_Chunker::get_results_for_instantiation(instantiation* inst)
{
    preference* pref;

    results = NIL;
    results_match_goal_level = inst->match_goal_level;
    results_tc_number = get_new_tc_number(thisAgent);
    extra_result_prefs_from_instantiation = inst->preferences_generated;
    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
        if ((pref->id->id->level < results_match_goal_level) &&
                (pref->id->tc_num != results_tc_number))
        {
            add_pref_to_results(pref);
            dprint(DT_VARIABLIZATION_MANAGER, "Pref %p added to results.\n", pref);
        } else {
            dprint(DT_VARIABLIZATION_MANAGER, "Did not add pref %p to results. %d >= %d\n", pref, pref->id->id->level, results_match_goal_level);
        }

    return results;
}

action* Explanation_Based_Chunker::copy_action_list(action* actions)
{
    action* old, *New, *prev, *first;
    char first_letter;

    prev = NIL;
    first = NIL;  /* unneeded, but without it gcc -Wall warns here */
    old = actions;
    while (old)
    {
        New = make_action(thisAgent);
        if (prev)
    {
            prev->next = New;
    }
        else
    {
            first = New;
    }
        prev = New;
        New->type = old->type;
        New->preference_type = old->preference_type;
        New->support = old->support;
        if (old->type == FUNCALL_ACTION)
    {
            New->value = copy_rhs_value(thisAgent, old->value);
    }
        else
    {
            New->id = copy_rhs_value(thisAgent, old->id);
            New->attr = copy_rhs_value(thisAgent, old->attr);
            first_letter = first_letter_from_rhs_value(New->attr);
            New->value = copy_rhs_value(thisAgent, old->value);
            if (preference_is_binary(old->preference_type))
    {
                New->referent = copy_rhs_value(thisAgent, old->referent);
            }
    }
        old = old->next;
        }
    if (prev)
    {
        prev->next = NIL;
    }
    else
    {
        first = NIL;
    }
    return first;
}

/* ====================================================================

     Chunk Conditions, and Chunk Conditions Set Manipulation Routines

   These structures have two uses.  First, for every ground condition,
   one of these structures maintains certain information about it--
   pointers to the original (instantiation's) condition, the chunks's
   instantiation's condition, and the variablized condition, etc.

   Second, for negated conditions, these structures are entered into
   a hash table with keys hash_condition(thisAgent, this_cond).  This hash
   table is used so we can add a new negated condition to the set of
   negated potentials quickly--we don't want to add a duplicate of a
   negated condition that's already there, and the hash table lets us
   quickly determine whether a duplicate is already there.

   I used one type of structure for both of these uses, (1) for simplicity
   and (2) to avoid having to do a second allocation when we move
   negated conditions over to the ground set.
==================================================================== */

/* --------------------------------------------------------------------
                      Chunk Cond Set Routines

   Init_chunk_cond_set() initializes a given chunk_cond_set to be empty.

   Make_chunk_cond_for_condition() takes a condition and returns a
   chunk_cond for it, for use in a chunk_cond_set.  This is used only
   for the negated conditions, not grounds.

   Add_to_chunk_cond_set() adds a given chunk_cond to a given chunk_cond_set
   and returns true if the condition isn't already in the set.  If the
   condition is already in the set, the routine deallocates the given
   chunk_cond and returns false.

   Remove_from_chunk_cond_set() removes a given chunk_cond from a given
   chunk_cond_set, but doesn't deallocate it.
-------------------------------------------------------------------- */

/* set of all negated conditions we encounter
                                during backtracing--these are all potentials
                                and (some of them) are added to the grounds
                                in one pass at the end of the backtracing */

void Explanation_Based_Chunker::init_chunk_cond_set(chunk_cond_set* set)
{
    int i;

    set->all = NIL;
    for (i = 0; i < CHUNK_COND_HASH_TABLE_SIZE; i++)
    {
        set->table[i] = NIL;
    }
}

/* -- Note:  add_to_chunk_cond_set and make_chunk_cond_for_negated_condition are both
 *           only used for negative conditions and NCCS.  Used in a single line in
 *           backtrace_through_instantiation() -- */

chunk_cond* Explanation_Based_Chunker::make_chunk_cond_for_negated_condition(condition* cond)
{
    chunk_cond* cc;
    uint32_t remainder, hv;

    thisAgent->memoryManager->allocate_with_pool(MP_chunk_cond, &cc);
    cc->cond = cond;
    cc->hash_value = hash_condition(thisAgent, cond);
    remainder = cc->hash_value;
    hv = 0;
    while (remainder)
    {
        hv ^= (remainder &
               masks_for_n_low_order_bits[LOG_2_CHUNK_COND_HASH_TABLE_SIZE]);
        remainder = remainder >> LOG_2_CHUNK_COND_HASH_TABLE_SIZE;
    }
    cc->compressed_hash_value = hv;
    return cc;
}

bool Explanation_Based_Chunker::add_to_chunk_cond_set(chunk_cond_set* set, chunk_cond* new_cc)
{
    chunk_cond* old;

    for (old = set->table[new_cc->compressed_hash_value]; old != NIL;
            old = old->next_in_bucket)
        if (old->hash_value == new_cc->hash_value)
            if (conditions_are_equal(old->cond, new_cc->cond))
            {
                break;
            }
    if (old)
    {
        /* --- the new condition was already in the set; so don't add it --- */
        thisAgent->memoryManager->free_with_pool(MP_chunk_cond, new_cc);
        return false;
    }
    /* --- add new_cc to the table --- */
    insert_at_head_of_dll(set->all, new_cc, next, prev);
    insert_at_head_of_dll(set->table[new_cc->compressed_hash_value], new_cc,
                          next_in_bucket, prev_in_bucket);
    return true;
}

void Explanation_Based_Chunker::remove_from_chunk_cond_set(chunk_cond_set* set, chunk_cond* cc)
{
    remove_from_dll(set->all, cc, next, prev);
    remove_from_dll(set->table[cc->compressed_hash_value],
                    cc, next_in_bucket, prev_in_bucket);
}

/* ====================================================================

                 Other Miscellaneous Chunking Routines

==================================================================== */

/* --------------------------------------------------------------------
            Build Chunk Conds For Grounds And Add Negateds

   This routine is called once backtracing is finished.  It goes through
   the ground conditions and builds a chunk_cond (see above) for each
   one.  The chunk_cond includes two new copies of the condition:  one
   to be used for the initial instantiation of the chunk, and one to
   be (variablized and) used for the chunk itself.

   This routine also goes through the negated conditions and adds to
   the ground set (again building chunk_cond's) any negated conditions
   that are connected to the grounds.

   At exit, the "dest_top" and "dest_bottom" arguments are set to point
   to the first and last chunk_cond in the ground set.  The "tc_to_use"
   argument is the tc number that this routine will use to mark the
   TC of the ground set.  At exit, this TC indicates the set of identifiers
   in the grounds.  (This is used immediately afterwards to figure out
   which Nots must be added to the chunk.)
-------------------------------------------------------------------- */

inline void add_cond(condition** c, condition** prev, condition** first)
{
    if (*prev)
    {
        (*c)->prev = *prev;
        (*prev)->next = *c;
    }
    else
    {
        *first = *c;
        *prev = NIL;
        (*c)->prev = NIL;
    }
    *prev = *c;

}

void Explanation_Based_Chunker::create_instantiated_counterparts(condition* vrblz_top, condition** inst_top, condition** inst_bottom)
{
    condition* copy_cond = vrblz_top;
    condition* c_inst = NULL, *first_inst = NULL, *prev_inst = NULL;
    while (copy_cond)
    {
        c_inst = copy_condition(thisAgent, copy_cond);

        /*-- Store a link from the variablized condition to the instantiated
         *   condition.  Used during merging if the chunker needs
         *   to delete a redundant condition.  Also used to reorder
         *   instantiated condition to match the re-ordered variablized
         *   conditions list (required by the rete.) -- */
        c_inst->counterpart = copy_cond;
        copy_cond->counterpart = c_inst;
        add_cond(&c_inst, &prev_inst, &first_inst);
        copy_cond = copy_cond->next;
    }
    if (prev_inst)
    {
        prev_inst->next = NIL;
    }
    else
    {
        first_inst->next = NIL;
    }

    *inst_top = first_inst;
    *inst_bottom = c_inst;
}

void Explanation_Based_Chunker::create_initial_chunk_condition_lists(
                                                    condition** inst_top,
                                                    condition** inst_bottom,
                                                    condition** vrblz_top,
                                                    tc_number tc_to_use,
                                                    bool* reliable)
{
    cons* c;
    condition* ground, *c_vrblz, *first_vrblz = nullptr, *prev_vrblz;
    bool should_unify_and_simplify = learning_is_on_for_instantiation();

    c_vrblz  = NIL; /* unnecessary, but gcc -Wall warns without it */

    dprint(DT_BUILD_CHUNK_CONDS, "Building conditions for new chunk...\n");
    dprint(DT_BUILD_CHUNK_CONDS, "Grounds from backtrace: \n");
    dprint_noprefix(DT_BUILD_CHUNK_CONDS, "%3", grounds);
    dprint(DT_BUILD_CHUNK_CONDS, "...creating positive conditions from final ground set.\n");
    /* --- build instantiated conds for grounds and setup their TC --- */
    reset_constraint_found_tc_num();
    prev_vrblz = NIL;
    while (grounds)
    {
        c = grounds;
        grounds = grounds->rest;
        ground = static_cast<condition_struct*>(c->first);
        free_cons(thisAgent, c);
        /* --- make the instantiated condition --- */
        dprint(DT_BACKTRACE, "   processing ground condition: %l\n", ground);

        /* -- Originally cc->cond would be set to ground and cc->inst was a copy-- */
        c_vrblz = copy_condition(thisAgent, ground, true, should_unify_and_simplify);
        add_cond(&c_vrblz, &prev_vrblz, &first_vrblz);

        /* --- add this condition to the TC.  Needed to see if NCC are grounded. --- */
        add_cond_to_tc(thisAgent, ground, tc_to_use, NIL, NIL);
    }

    dprint(DT_BACKTRACE, "...adding negated conditions from backtraced negated set.\n");
    /* --- scan through negated conditions and check which ones are connected
       to the grounds --- */
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
    {
        print_string(thisAgent, "\n\n*** Adding Grounded Negated Conditions ***\n");
    }

    chunk_cond *cc;
    while (negated_set.all)
    {
        cc = negated_set.all;
        remove_from_chunk_cond_set(&negated_set, cc);
        if (cond_is_in_tc(thisAgent, cc->cond, tc_to_use))
        {
            /* --- negated cond is in the TC, so add it to the grounds --- */
            if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
            {
                print_string(thisAgent, "\n-->Moving to grounds: ");
                print_condition(thisAgent, cc->cond);
            }
            c_vrblz = copy_condition(thisAgent, cc->cond, true, should_unify_and_simplify);

            add_cond(&c_vrblz, &prev_vrblz, &first_vrblz);
            }
            else
            {
            /* --- not in TC, so discard the condition --- */

            if (thisAgent->sysparams[CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM] == false)
            {
                // this chunk will be overgeneral! don't create it

                // SBW 5/07
                // report what local negations are preventing the chunk,
                // and set flags like we saw a ^quiescence t so it won't be created
                report_local_negation(cc->cond);    // in backtrace.cpp
                *reliable = false;
            }
#ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationLogger->increment_stat_tested_local_negation();
#endif
            thisAgent->memoryManager->free_with_pool(MP_chunk_cond, cc);
        }
    }

    if (prev_vrblz)
    {
        prev_vrblz->next = NIL;
    }
    else if (first_vrblz)
    {
        first_vrblz->next = NIL;
    }

    *vrblz_top = first_vrblz;

    if (first_vrblz)
    {
        add_additional_constraints(*vrblz_top);
        create_instantiated_counterparts(*vrblz_top, inst_top, inst_bottom);
    }
    dprint(DT_BUILD_CHUNK_CONDS, "Instantiated chunk conditions after identity unification: \n%1", *inst_top);
    dprint(DT_BUILD_CHUNK_CONDS, "Variablized chunk conditions after copying and adding additional conditions: \n%1", *vrblz_top);
    dprint(DT_BUILD_CHUNK_CONDS, "build_chunk_conds_for_grounds_and_add_negateds done.\n");
}

/* --------------------------------------------------------------------
                     Add Goal or Impasse Tests

   This routine adds goal id or impasse id tests to the variablized
   conditions.  For each id in the grounds that happens to be the
   identifier of a goal or impasse, we add a goal/impasse id test
   to the variablized conditions, to make sure that in the resulting
   chunk, the variablization of that id is constrained to match against
   a goal/impasse.  (Note:  actually, in the current implementation of
   chunking, it's impossible for an impasse id to end up in the ground
   set.  So part of this code is unnecessary.)
-------------------------------------------------------------------- */

void Explanation_Based_Chunker::add_goal_or_impasse_tests(condition* inst_top, condition* vrblz_top)
{
    condition* cc;
    tc_number tc;   /* mark each id as we add a test for it, so we don't add
                     a test for the same id in two different places */
    Symbol* id;
    test t;

    tc = get_new_tc_number(thisAgent);
    for (cc = inst_top; cc != NIL; cc = cc->next)
    {
        if (cc->type != POSITIVE_CONDITION)
        {
            continue;
        }
        id = cc->data.tests.id_test->eq_test->data.referent;
        if ((id->id->isa_goal || id->id->isa_impasse) &&
                (id->tc_num != tc))
        {
            t = make_test(thisAgent, NULL, ((id->id->isa_goal) ? GOAL_ID_TEST : IMPASSE_ID_TEST));
            add_test(thisAgent, &(cc->counterpart->data.tests.id_test), t);
            //t = make_test(thisAgent, NULL, ((id->id->isa_goal) ? GOAL_ID_TEST : IMPASSE_ID_TEST));
            //add_test(thisAgent, &(cc->data.tests.id_test), t);
            id->tc_num = tc;
        }
    }
}

/* --------------------------------------------------------------------
                    Reorder Instantiated Conditions

   The Rete routines require the instantiated conditions (on the
   instantiation structure) to be in the same order as the original
   conditions from which the Rete was built.  This means that the
   initial instantiation of the chunk must have its conditions in
   the same order as the variablized conditions.  The trouble is,
   the variablized conditions get rearranged by the reorderer.  So,
   after reordering, we have to rearrange the instantiated conditions
   to put them in the same order as the now-scrambled variablized ones.
   This routine does this.

-------------------------------------------------------------------- */

void Explanation_Based_Chunker::reorder_instantiated_conditions(condition* top_cond,
                                     condition** dest_inst_top,
                                     condition** dest_inst_bottom)
{
    dprint(DT_MERGE, "Re-ordering...\n");
    dprint_noprefix(DT_MERGE, "%1", top_cond->counterpart);
    dprint(DT_MERGE, "..to match...\n");
    dprint_noprefix(DT_MERGE, "%1", top_cond);

    condition* c, *p, *n;
    for (c = top_cond; c != NIL; c = c->next)
    {
        if (c->counterpart)
    {
            p = c->prev;
            n = c->next;
            if (!n)
        {
                c->counterpart->next = NULL;
                *dest_inst_bottom = c->counterpart;
            } else {
                c->counterpart->next = n->counterpart;
        }
            if (!p)
        {
                c->counterpart->prev = NULL;
                *dest_inst_top = c->counterpart;
            } else {
                c->counterpart->prev = p->counterpart;
        }
        }
        }
    dprint(DT_MERGE, "Result:\n");
    dprint_noprefix(DT_MERGE, "%1", *dest_inst_top);
}

/* --------------------------------------------------------------------
                       Make Clones of Results

   When we build the initial instantiation of the new chunk, we have
   to fill in preferences_generated with *copies* of all the result
   preferences.  These copies are clones of the results.  This routine
   makes these clones and fills in chunk_inst->preferences_generated.
-------------------------------------------------------------------- */

void Explanation_Based_Chunker::make_clones_of_results(preference* results,
                            instantiation* chunk_inst)
{
    preference* p, *result_p;

    chunk_inst->preferences_generated = NIL;
    for (result_p = results; result_p != NIL; result_p = result_p->next_result)
    {
        /* --- copy the preference --- */
        p = make_preference(thisAgent, result_p->type, result_p->id, result_p->attr,
                            result_p->value, result_p->referent,
                            result_p->o_ids, result_p->rhs_funcs);
        symbol_add_ref(thisAgent, p->id);
        symbol_add_ref(thisAgent, p->attr);
        symbol_add_ref(thisAgent, p->value);
        if (preference_is_binary(p->type))
        {
            symbol_add_ref(thisAgent, p->referent);
        }
        /* --- put it onto the list for chunk_inst --- */
        p->inst = chunk_inst;
        insert_at_head_of_dll(chunk_inst->preferences_generated, p,
                              inst_next, inst_prev);
        /* --- insert it into the list of clones for this preference --- */
        p->next_clone = result_p;
        p->prev_clone = result_p->prev_clone;
        result_p->prev_clone = p;
        if (p->prev_clone)
        {
            p->prev_clone->next_clone = p;
        }
    }
}


/* Before calling make_production, caller is responsible for using this
 * function to make sure the production is valid.  This was separated out
 * so EBC can try to fix unconnected conditions caused by LTI being at a
 * higher level. */

bool Explanation_Based_Chunker::reorder_and_validate_chunk(ProductionType   type,
                                                           condition**      lhs_top,
                                                           action**         rhs_top,
                                                           bool             reorder_nccs)
{
    /* This is called for justifications even though it does nothing because in the future
     * we might want to fix a justification that has conditions unconnected to
     * a state.  Chunks that have such variablized conditions seem to be able to
     * corrupt the rete, but we don't know if justifications can as well.  While we
     * could ground those conditions like we do with chunks to be safe, we're not doing
     * that right now because it will introduce a high computational cost that may
     * not be necessary.*/

    if (type != JUSTIFICATION_PRODUCTION_TYPE)
    {
        symbol_list* unconnected_syms = new symbol_list();

        EBCFailureType lFailureType = reorder_and_validate_lhs_and_rhs(thisAgent, lhs_top, rhs_top, reorder_nccs, true, unconnected_syms);

        if (lFailureType == ebc_failed_unconnected_conditions)
        {
            for (auto it = unconnected_syms->begin(); it != unconnected_syms->end(); it++) {
                generate_conditions_to_ground_lti(lhs_top, (*it));
            }
        }
        delete unconnected_syms;
        return (lFailureType == ebc_success);
    }
    return true;
}

void Explanation_Based_Chunker::build_chunk_or_justification(instantiation* inst, instantiation** custom_inst_list)
{
    goal_stack_level grounds_level;
    preference* results, *pref;
    action* rhs;
    production* prod;
    instantiation* chunk_inst;
    Symbol* prod_name=NULL;
    ProductionType prod_type;
    bool print_name, print_prod;
    byte rete_addition_result;
    condition* inst_top = NULL, * inst_bottom = NULL;
    condition* vrblz_top = NULL;
    bool reliable = true;
    bool variablize;
    bool lChunkValidated = false;
    inst_top = vrblz_top = NULL;
    condition*  saved_justification_top = 0;
    condition*  saved_justification_bottom = 0;
    uint64_t chunk_new_i_id = 0;
    production* duplicate_rule = NULL;

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    soar_timer local_timer;
    local_timer.set_enabled(&(thisAgent->sysparams[ TIMERS_ENABLED ]));
#endif
#endif

    /* --- if it only matched an attribute impasse, don't chunk --- */
    if (! inst->match_goal)
    {
        return;
    }

    /* --- if no preference is above the match goal level, exit --- */
    /* MToDo | This seems redundant given what get_results_for_instantiation does.  Is it faster
     *         and worth it because most calls won't have results, little less extra results? */
    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
    {
        if (pref->id->id->level < inst->match_goal_level)
        {
            break;
        }
    }
    if (! pref)
    {
        return;
    }

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    local_timer.start();
#endif
#endif

    results = get_results_for_instantiation(inst);
    if (!results)
    {
        goto chunking_abort;
    }

    dprint_header(DT_MILESTONES, PrintBoth, "chunk_instantiation() called for instance of rule %s (id=%u)\n",
        (inst->prod ? inst->prod->name->sc->name : "fake instantiation"), inst->i_id);

    /* --- If we're over MAX_CHUNKS, abort chunk --- */
    if (chunks_this_d_cycle > static_cast<uint64_t>(thisAgent->sysparams[MAX_CHUNKS_SYSPARAM]))
    {
        if (thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM])
        {
            print(thisAgent, "Warning: reached max-chunks! Halting system.\n");
            xml_generate_warning(thisAgent, "Warning: reached max-chunks! Halting system.");
        }
        max_chunks_reached = true;
#ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationLogger->increment_stat_max_chunks();
#endif
        goto chunking_abort;
    }

#ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationLogger->add_chunk_record(inst);
#endif

    /* set allow_bottom_up_chunks to false for all higher goals to prevent chunking */
    {
        Symbol* g;
        for (g = inst->match_goal->id->higher_goal; g && g->id->allow_bottom_up_chunks; g = g->id->higher_goal)
        {
            g->id->allow_bottom_up_chunks = false;
        }
    }

    grounds_level = inst->match_goal_level - 1;

    backtrace_number++;
    if (backtrace_number == 0)
    {
        backtrace_number = 1;
    }

    grounds_tc++;
    if (grounds_tc == 0)
    {
        grounds_tc = 1;
    }

    potentials_tc++;
    if (potentials_tc == 0)
    {
        potentials_tc = 1;
    }

    locals_tc++;
    if (locals_tc == 0)
    {
        locals_tc = 1;
    }

    grounds = NIL;
    positive_potentials = NIL;
    locals = NIL;

#ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationLogger->set_backtrace_number(backtrace_number);
#endif

    dprint(DT_BACKTRACE, "Backtracing through instantiations that produced result preferences...\n%6\n", NULL, results);
    /* --- backtrace through the instantiation that produced each result --- */
    dprint(DT_BACKTRACE,  "Backtracing through instantiation: \n%7", inst);
    for (pref = results; pref != NIL; pref = pref->next_result)
    {
        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
        {
            print_string(thisAgent, "\nFor result preference ");
            xml_begin_tag(thisAgent, kTagBacktraceResult);
            print_preference(thisAgent, pref);
            print_string(thisAgent, " ");
        }
        dprint(DT_BACKTRACE, "Backtracing through result preference: %p\n", pref);
        dprint(DT_BACKTRACE, " from instantiation...\n%7", pref->inst);
        backtrace_through_instantiation(pref->inst, grounds_level, NULL, &reliable, 0, pref->o_ids, pref->rhs_funcs);

        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
        {
            xml_end_tag(thisAgent, kTagBacktraceResult);
        }
    }

    dprint(DT_BACKTRACE, "Backtracing through results DONE.\n");
    dprint(DT_BACKTRACE, "Grounds:\n%3", grounds);

    while (true)
    {
        trace_locals(grounds_level, &reliable);
        trace_grounded_potentials();
        dprint(DT_BACKTRACE, "Grounds after trace_grounded_potentials:\n%3", grounds);
        if (! trace_ungrounded_potentials(grounds_level, &reliable))
        {
            break;
        }
    }

    dprint(DT_BACKTRACE, "Tracing DONE.\n");
    dprint(DT_VARIABLIZATION_MANAGER, "Grounds after tracing:\n%3", grounds);
//    dprint(DT_VARIABLIZATION_MANAGER, "Results:\n%6", pref);

    free_list(thisAgent, positive_potentials);

    /* --- Collect the grounds into the chunk condition lists --- */
    chunk_new_i_id = get_new_inst_id();
    {
        tc_number tc_for_grounds;
        tc_for_grounds = get_new_tc_number(thisAgent);
        create_initial_chunk_condition_lists(&inst_top, &inst_bottom, &vrblz_top, tc_for_grounds, &reliable);
    }
    /* --- Backtracing done.  If there aren't any grounds, abort chunk --- */
    if (!inst_top)
    {
        if (thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM])
        {
            print_string(thisAgent, "Warning: Chunk/justification has no conditions.  Soar cannot learn a rule for\n");
            print_string(thisAgent, "         the results created from this substate. To avoid this issue, the agent\n");
            print_string(thisAgent, "         needs to positively test at least one item in the superstate.\n");
            xml_generate_warning(thisAgent, "Warning: Chunk/justification has no conditions.  Soar cannot learn a rule for");
            xml_generate_warning(thisAgent, "         the results created by this substate. To avoid this issue, the agent");
            xml_generate_warning(thisAgent, "         needs to positively test at least one item in the superstate.");
        }
#ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationLogger->increment_stat_no_grounds();
#endif
        goto chunking_abort;
    }

    /* Determine if we create a justification or variablize and create a chunk */
    variablize = learning_is_on_for_instantiation() && reliable;

    /* Generate a new symbol for the name of the new chunk or justification */
    if (variablize)
    {
        chunks_this_d_cycle++;
        prod_name = generate_chunk_name(inst);

        prod_type = CHUNK_PRODUCTION_TYPE;
        print_name = (thisAgent->sysparams[TRACE_CHUNK_NAMES_SYSPARAM] != 0);
        print_prod = (thisAgent->sysparams[TRACE_CHUNKS_SYSPARAM] != 0);
#ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationLogger->increment_stat_chunks_attempted();
#endif
    }
    else
    {
        prod_name = generate_new_str_constant(thisAgent, "justification-", &justification_count);
        prod_type = JUSTIFICATION_PRODUCTION_TYPE;
        print_name = (thisAgent->sysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] != 0);
        print_prod = (thisAgent->sysparams[TRACE_JUSTIFICATIONS_SYSPARAM] != 0);
#ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationLogger->increment_stat_justifications_attempted();
#endif
    }

    if (print_name)
    {
        start_fresh_line(thisAgent);
        print_with_symbols(thisAgent, "Building rule %y\n", prod_name);

        xml_begin_tag(thisAgent, kTagLearning);
        xml_begin_tag(thisAgent, kTagProduction);
        xml_att_val(thisAgent, kProduction_Name, prod_name);
        xml_end_tag(thisAgent, kTagProduction);
        xml_end_tag(thisAgent, kTagLearning);
    }
    dprint(DT_MILESTONES, "Backtracing done.  Building chunk %y\n", prod_name);

    dprint(DT_PRINT_INSTANTIATIONS, "Chunk_instantiation instantiated conditions from backtrace:\n%6", inst_top, results);
    dprint(DT_BUILD_CHUNK_CONDS, "Counterparts conditions for variablization:\n%6", vrblz_top, results);

    if (variablize)
    {
        /* Save conditions and results in case we need to make a justification because chunking fails */
        copy_condition_list(thisAgent, vrblz_top, &saved_justification_top, &saved_justification_bottom);

        reset_variable_generator(thisAgent, vrblz_top, NIL);
        variablize_condition_list(vrblz_top);
        dprint(DT_VARIABLIZATION_MANAGER, "chunk_instantiation after variablizing conditions and relational constraints: \n%6", vrblz_top, results);
        #ifdef EBC_MERGE_CONDITIONS
        merge_conditions(vrblz_top);
        #endif
        dprint(DT_VARIABLIZATION_MANAGER, "chunk_instantiation after merging conditions: \n%6", vrblz_top, results);
    }

    dprint(DT_VARIABLIZATION_MANAGER, "Unifying identities in results... \n%6", vrblz_top, results);
    reset_variable_generator(thisAgent, vrblz_top, NIL);
    unify_identities_for_results(results);
    dprint(DT_VARIABLIZATION_MANAGER, "Polished and merged conditions/results with relational constraints: \n%6", vrblz_top, results);

    dprint_header(DT_VARIABLIZATION_MANAGER, PrintBefore, "Variablizing RHS results...\n");
    rhs = variablize_results_into_actions(results, variablize);
    dprint_header(DT_VARIABLIZATION_MANAGER, PrintAfter, "Done variablizing RHS results.\n");

    add_goal_or_impasse_tests(inst_top, vrblz_top);

    dprint(DT_CONSTRAINTS, "- Instantiated conds after add_goal_test\n%5", inst_top, NULL);
    dprint(DT_VARIABLIZATION_MANAGER, "chunk instantiation created variablized rule: \n%1-->\n%2", vrblz_top, rhs);

    thisAgent->name_of_production_being_reordered = prod_name->sc->name;
    lChunkValidated = reorder_and_validate_chunk(prod_type, &vrblz_top, &rhs, false);

    if (!lChunkValidated)
    {
        if (variablize)
        {
            /* Could not re-order chunk, so we need to go back and create a justification for the results instead */
            dprint(DT_VARIABLIZATION_MANAGER, "Could not create production for variablized rule.  Attempting to create justification instead... \n");
            print_with_symbols(thisAgent, "\nCould not create production for variablized rule:\n\nsp {%y\n", prod_name);
            print_condition_list(thisAgent, vrblz_top, 4, false);
            print(thisAgent, "\n  -->\n");
            print_action_list(thisAgent, rhs, 4, false);
            print(thisAgent, "\n}\n\nThis error is likely caused by the reasons outlined section 4 of the Soar\n");
            print(thisAgent, "manual, subsection \"revising the substructure of a previous result\". Check\n");
            print(thisAgent, "that the rules are not revising substructure of a result matched only\n");
            print(thisAgent, "through the local state.\n");
            xml_generate_warning(thisAgent, "\nCould not create production for variablized rule.\n...creating justification instead.\n");
            xml_generate_warning(thisAgent, "\n\nThis error is likely caused by the reasons outlined section 4 of the Soar\n");
            xml_generate_warning(thisAgent, "manual, subsection \"revising the substructure of a previous result\". Check\n");
            xml_generate_warning(thisAgent, "that the rules are not revising substructure of a result matched only\n");
            xml_generate_warning(thisAgent, "through the local state.\n\n");

            symbol_remove_ref(thisAgent, prod_name);
            prod_name = NULL;
            prod_name = generate_new_str_constant(thisAgent, "justification-", &justification_count);
            prod_type = JUSTIFICATION_PRODUCTION_TYPE;
            print_name = (thisAgent->sysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] != 0);
            print_prod = (thisAgent->sysparams[TRACE_JUSTIFICATIONS_SYSPARAM] != 0);

            deallocate_condition_list(thisAgent, vrblz_top);
            vrblz_top = saved_justification_top;
            saved_justification_top = saved_justification_bottom = NULL;

            deallocate_condition_list(thisAgent, inst_top);
            inst_top = inst_bottom = NULL;

            deallocate_action_list(thisAgent, rhs);
            rhs = NULL;

            create_instantiated_counterparts(vrblz_top, &inst_top, &inst_bottom);

            dprint_header(DT_VARIABLIZATION_MANAGER, PrintBefore, "Creating RHS actions from results...\n");
            rhs = variablize_results_into_actions(results, false);

            add_goal_or_impasse_tests(inst_top, vrblz_top);

            print_with_symbols(thisAgent, "\nCreating justification instead:\n\nsp {%y\n", prod_name);
            print_condition_list(thisAgent, vrblz_top, 4, false);
            print(thisAgent, "\n  -->\n");
            print_action_list(thisAgent, rhs, 4, false);
            print(thisAgent, "\n}\n\n");

            dprint(DT_CONSTRAINTS, "- Instantiated conds after add_goal_test\n%5", inst_top, NULL);
            dprint(DT_VARIABLIZATION_MANAGER, "chunk instantiation created variablized rule: \n%1-->\n%2", vrblz_top, rhs);
            prod = make_production(thisAgent, prod_type, prod_name, (inst->prod ? inst->prod->name->sc->name : prod_name->sc->name), &vrblz_top, &rhs, false, NULL);
            if (prod)
            {
                dprint(DT_VARIABLIZATION_MANAGER, "Successfully generated justification for failed chunk.\n");
                /* MToDo | Make this an option to interrrupt when an explanation is made*/
                //            thisAgent->stop_soar = true;
            }
        } else {
            print(thisAgent, "\nUnable to reorder this chunk:\n  ");
            print_condition_list(thisAgent, vrblz_top, 2, false);
            print(thisAgent, "\n  -->\n   ");
            print_action_list(thisAgent, rhs, 3, false);
            print(thisAgent, "\n\nThis error is likely caused by the reasons outlined section 4 of the Soar\n");
            print(thisAgent, "manual, subsection \"revising the substructure of a previous result\". Check\n");
            print(thisAgent, "that the rules are not revising substructure of a result matched only\n");
            print(thisAgent, "through the local state.\n");
            xml_generate_warning(thisAgent, "\nnUnable to reorder this chunk.\n");
            xml_generate_warning(thisAgent, "\n\nThis error is likely caused by the reasons outlined section 4 of the Soar\n");
            xml_generate_warning(thisAgent, "manual, subsection \"revising the substructure of a previous result\". Check\n");
            xml_generate_warning(thisAgent, "that the rules are not revising substructure of a result matched only\n");
            xml_generate_warning(thisAgent, "through the local state.\n");

            deallocate_condition_list(thisAgent, vrblz_top);
            vrblz_top = NULL;
            deallocate_condition_list(thisAgent, inst_top);
            inst_top = NULL;
            deallocate_action_list(thisAgent, rhs);
            rhs = NULL;
            /* Prior to 11/10/15, Soar would halt if it could not create the
             * production.  We're not sure if the conditions that would cause it to
             * crash previously can still occur, but we have cases now with chunks
             * formed from retrievals that we don't want Soar to stop on.  So far, we have
             * not had any issues with rejecting this chunk but allowing Soar to continue.
             * We do now create a justification instead in that ugly code prior to this
             * that re-does the final steps of chunk creation without variablizaiton.
             *
             * Previous comment:  // We cannot proceed, the GDS will crash in
             * decide.cpp:decide_non_context_slot */
            //        thisAgent->stop_soar = true;
            //        thisAgent->system_halted = true;
            //        thisAgent->reason_for_stopping = "Could not re-order chunk.";
            #ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationLogger->increment_stat_unorderable();
            #endif
            goto chunking_abort;

        }
    } else {
        prod = make_production(thisAgent, prod_type, prod_name, (inst->prod ? inst->prod->name->sc->name : prod_name->sc->name), &vrblz_top, &rhs, false, NULL);
    }
    /* Note that there cannot be a GOTO chunk_abort between creation of these saved conditions above and here */
    if (saved_justification_top)
    {
        deallocate_condition_list(thisAgent, saved_justification_top);
        saved_justification_top = saved_justification_bottom = NULL;
    }
    /* We don't want to accidentally delete it.  Production struct is now responsible for it. */
    prod_name = NULL;

    {
        condition* inst_lhs_top = 0, *inst_lhs_bottom = 0;

        reorder_instantiated_conditions(vrblz_top, &inst_lhs_top, &inst_lhs_bottom);

        thisAgent->memoryManager->allocate_with_pool(MP_instantiation, &chunk_inst);
        chunk_inst->prod = prod;
        chunk_inst->top_of_instantiated_conditions = inst_lhs_top;
        chunk_inst->bottom_of_instantiated_conditions = inst_lhs_bottom;

        chunk_inst->GDS_evaluated_already = false;  /* REW:  09.15.96 */
        chunk_inst->i_id = chunk_new_i_id;
        chunk_inst->reliable = reliable;

        chunk_inst->in_ms = true;  /* set true for now, we'll find out later... */

        make_clones_of_results(results, chunk_inst);
        fill_in_new_instantiation_stuff(thisAgent, chunk_inst, true, inst);
    }

    dprint(DT_VARIABLIZATION_MANAGER, "Refracted instantiation: \n%5", chunk_inst->top_of_instantiated_conditions, chunk_inst->preferences_generated);
    dprint(DT_VARIABLIZATION_MANAGER, "Saved instantiation with constraints: \n%5", inst_top, chunk_inst->preferences_generated);

    rete_addition_result = add_production_to_rete(thisAgent, prod, vrblz_top, chunk_inst, print_name, duplicate_rule);

    if (print_prod && (rete_addition_result != DUPLICATE_PRODUCTION))
    {
        print(thisAgent, "\n");
        xml_begin_tag(thisAgent, kTagLearning);
        print_production(thisAgent, prod, false);
        xml_end_tag(thisAgent, kTagLearning);
    }
    if (rete_addition_result == REFRACTED_INST_MATCHED)
    {
        #ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationLogger->increment_stat_succeeded();
        assert(prod);
        thisAgent->explanationLogger->record_chunk_contents(prod, vrblz_top, rhs, results, unification_map, inst);
        #endif
        if (prod_type == JUSTIFICATION_PRODUCTION_TYPE) {
            #ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationLogger->increment_stat_justifications();
            #endif
        }
        dprint(DT_VARIABLIZATION_MANAGER, "Add production to rete result: Refracted instantiation matched.\n");

    } else if (rete_addition_result == DUPLICATE_PRODUCTION) {
        #ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationLogger->increment_stat_duplicates(duplicate_rule);
        #endif
        excise_production(thisAgent, prod, false);
        chunk_inst->in_ms = false;
        dprint(DT_VARIABLIZATION_MANAGER, "Add production to rete result: Duplicate production.\n");
    } else if (rete_addition_result == REFRACTED_INST_DID_NOT_MATCH) {
        if (prod_type == JUSTIFICATION_PRODUCTION_TYPE)
    {
            #ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationLogger->increment_stat_justification_did_not_match();
            #endif
        excise_production(thisAgent, prod, false);
        } else {
            #ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationLogger->increment_stat_chunk_did_not_match();
            assert(prod);
            thisAgent->explanationLogger->record_chunk_contents(prod, vrblz_top, rhs, results, unification_map, inst);
            #endif
            /* MToDo | Why don't we excise the chunk here like we do non-matching
             * justifications? It doesn't seem like either case of non-matching rule
             * should be possible unless a chunking or gds problem has occurred.
             * Can we even get here?  Let's see.*/
            assert(false);
    }
        chunk_inst->in_ms = false;
        dprint(DT_VARIABLIZATION_MANAGER, "Add production to rete result: Refracted instantiation did not match.\n");
    } else {
        dprint(DT_VARIABLIZATION_MANAGER, "Add production to rete result: No refracted instantiation given.\n");
        /* Don't think this can happen either */
        assert(false);
    }

    /* --- deallocate chunks conds and variablized conditions --- */
    deallocate_condition_list(thisAgent, vrblz_top);
    vrblz_top = NULL;

    /* --- assert the preferences --- */
    chunk_inst->next = (*custom_inst_list);
    (*custom_inst_list) = chunk_inst;

    clean_up(&prod_name, &(vrblz_top));

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    local_timer.stop();
    thisAgent->timers_chunking_cpu_time[thisAgent->current_phase].update(local_timer);
#endif
#endif

    if (!max_chunks_reached)
    {
        dprint(DT_MILESTONES, "Calling chunk instantiation from chunk instantiation for i%u START\n", chunk_new_i_id);
        set_learning_for_instantiation(chunk_inst);
        build_chunk_or_justification(chunk_inst, custom_inst_list);
        dprint(DT_MILESTONES, "Chunk instantiation called from chunk instantiation for i%u DONE.\n", chunk_new_i_id);
    } else {
        #ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationLogger->increment_stat_max_chunks();
        #endif
    }

    return;

chunking_abort:
    {
        clean_up(&prod_name, &(vrblz_top));
    }

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    local_timer.stop();
    thisAgent->timers_chunking_cpu_time[thisAgent->current_phase].update(local_timer);
#endif
#endif
}


void Explanation_Based_Chunker::clean_up (Symbol** prod_name, condition** vrblz_top)
{
    thisAgent->explanationLogger->end_chunk_record();
    if (vrblz_top)
    {
        deallocate_condition_list(thisAgent, (*vrblz_top));
        (*vrblz_top) = NULL;
    }
    if (*prod_name)
    {
        dprint_header(DT_MILESTONES, PrintAfter, "chunk_instantiation() done building and cleaning up for chunk %y.\n", *prod_name);
        symbol_remove_ref(thisAgent, *prod_name);
        *prod_name = NULL;
    }
    clear_variablization_maps();
    clear_cached_constraints();
    clear_o_id_substitution_map();
    clear_attachment_map();
}

