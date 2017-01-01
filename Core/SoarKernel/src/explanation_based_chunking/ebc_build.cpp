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

#include "agent.h"
#include "condition.h"
#include "decide.h"
#include "debug.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "run_soar.h"
#include "soar_TraceNames.h"
#include "slot.h"
#include "symbol.h"
#include "test.h"
#include "working_memory.h"
#include "working_memory_activation.h"
#include "xml.h"

#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include <ebc_repair.h>

using namespace soar_TraceNames;
void sanity_check_conditions(condition* top_cond);
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
void Explanation_Based_Chunker::add_results_if_needed(Symbol* sym, uint64_t linked_id)
{
    dprint(DT_EXTRA_RESULTS, "...looking for results that are children of %y (parent identity %u)", sym, linked_id);
    if ((sym)->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        if (((sym)->id->level >= m_results_match_goal_level) && ((sym)->tc_num != m_results_tc))
        {
            add_results_for_id(sym, linked_id);
            return;
        }
        dprint(DT_EXTRA_RESULTS, "...wrong level or not result.\n");
    }
    dprint(DT_EXTRA_RESULTS, "...not identifier.\n");
}

void Explanation_Based_Chunker::add_pref_to_results(preference* pref, uint64_t linked_id)
{
    preference* p;

    dprint(DT_EXTRA_RESULTS, "Attempting to add pref to results: %p \n", pref);
    /* --- if an equivalent pref is already a result, don't add this one --- */
    for (p = m_results; p != NIL; p = p->next_result)
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
    if (pref->inst->match_goal_level != m_results_match_goal_level)
    {
        for (p = pref->next_clone; p != NIL; p = p->next_clone)
            if (p->inst->match_goal_level == m_results_match_goal_level)
            {
                break;
            }
        if (!p)
            for (p = pref->prev_clone; p != NIL; p = p->prev_clone)
                if (p->inst->match_goal_level == m_results_match_goal_level)
                {
                    break;
                }
        if (!p)
        {
            return;    /* if can't find one, it isn't a result */
        }
        pref = p;
    }
    dprint(DT_EXTRA_RESULTS, "...not a duplicate and at correct level (or clone found.)\n");

    /* --- add this preference to the result list --- */
    pref->next_result = m_results;
    m_results = pref;
    if (pref->identities.id && linked_id)
    {
        dprint(DT_EXTRA_RESULTS, "...adding identity mapping from identifier element to parent value element: %u -> %u\n", pref->identities.id, linked_id);
        add_identity_unification(pref->identities.id, linked_id);
    }
    /* --- follow transitive closure through value, referent links --- */
    add_results_if_needed(pref->value, pref->identities.value);
    if (preference_is_binary(pref->type))
    {
        add_results_if_needed(pref->referent, pref->identities.referent);
    }
}

void Explanation_Based_Chunker::add_results_for_id(Symbol* id, uint64_t linked_id)
{
    slot* s;
    preference* pref;
    wme* w;

    id->tc_num = m_results_tc;

    /* --- scan through all preferences and wmes for all slots for this id --- */
    dprint(DT_EXTRA_RESULTS, "...iterating through input wmes...\n");
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        add_results_if_needed(w->value, w->preference ? w->preference->identities.value : 0);
    }
    dprint(DT_EXTRA_RESULTS, "...iterating through slots...\n");
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        dprint(DT_EXTRA_RESULTS, "...iterating through prefs of slot...\n");
        for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
        {
            add_pref_to_results(pref, linked_id);
        }
        dprint(DT_EXTRA_RESULTS, "...iterating through wmes of slot...\n");
        for (w = s->wmes; w != NIL; w = w->next)
        {
            add_results_if_needed(w->value, w->preference ? w->preference->identities.value : 0);
        }
    } /* end of for slots loop */
    dprint(DT_EXTRA_RESULTS, "...iterating through extra results looking for id...\n");
    /* --- now scan through extra prefs and look for any with this id --- */
    for (pref = m_extra_results; pref != NIL;
            pref = pref->inst_next)
    {
        if (pref->id == id)
        {
            add_pref_to_results(pref, linked_id);
        }
    }
}

void Explanation_Based_Chunker::get_results_for_instantiation()
{
    preference* pref;

    m_results = NIL;
    m_results_match_goal_level = m_inst->match_goal_level;
    m_results_tc = get_new_tc_number(thisAgent);
    m_extra_results = m_inst->preferences_generated;
    for (pref = m_inst->preferences_generated; pref != NIL; pref = pref->inst_next)
    {
        if ((pref->id->id->level < m_results_match_goal_level) &&
                (pref->id->tc_num != m_results_tc))
        {
            add_pref_to_results(pref, 0);
        } else {
            dprint(DT_EXTRA_RESULTS, "Did not add pref %p to results. %d >= %d\n", pref, static_cast<int64_t>(pref->id->id->level), static_cast<int64_t>(m_results_match_goal_level));
        }
    }
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

/* set of all negated conditions we encounter during backtracing--these are
 * all potentials and (some of them) are added to the grounds in one pass at
 * the end of the backtracing */

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

void Explanation_Based_Chunker::create_initial_chunk_condition_lists()
{
    cons* c;
    condition* ground, *c_vrblz, *first_vrblz = nullptr, *prev_vrblz;
    bool should_unify_and_simplify = m_learning_on_for_instantiation;

    tc_number tc_to_use = get_new_tc_number(thisAgent);

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

        c_vrblz = copy_condition(thisAgent, ground, true, should_unify_and_simplify, true);
        c_vrblz->inst = m_chunk_inst;
        add_cond(&c_vrblz, &prev_vrblz, &first_vrblz);

        /* --- add this condition to the TC.  Needed to see if NCC are grounded. --- */
        add_cond_to_tc(thisAgent, ground, tc_to_use, NIL, NIL);
    }

    dprint(DT_BACKTRACE, "...adding negated conditions from backtraced negated set.\n");
    /* --- scan through negated conditions and check which ones are connected
       to the grounds --- */
    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        thisAgent->outputManager->printa(thisAgent, "\n\n*** Adding Grounded Negated Conditions ***\n");
    }

    chunk_cond *cc;
    bool has_local_negation = false;
    while (negated_set.all)
    {
        cc = negated_set.all;
        remove_from_chunk_cond_set(&negated_set, cc);
        if (cond_is_in_tc(thisAgent, cc->cond, tc_to_use))
        {
            /* --- negated cond is in the TC, so add it to the grounds --- */
            if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
            {
                thisAgent->outputManager->printa(thisAgent, "\n-->Moving to grounds: ");
                print_condition(thisAgent, cc->cond);
            }
            c_vrblz = copy_condition(thisAgent, cc->cond, true, should_unify_and_simplify);
            c_vrblz->inst = m_chunk_inst;

            add_cond(&c_vrblz, &prev_vrblz, &first_vrblz);
        }
        else
        {
            /* --- not in TC, so discard the condition --- */

            if (ebc_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS] == false)
            {
                if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
                {
                    report_local_negation(cc->cond);
                }
                m_reliable = false;
            }
            has_local_negation = true;
        }
        thisAgent->memoryManager->free_with_pool(MP_chunk_cond, cc);
    }

    #ifdef BUILD_WITH_EXPLAINER
    if (has_local_negation)
    {
        thisAgent->explanationMemory->increment_stat_tested_local_negation();
    }
    #endif

    if (prev_vrblz)
    {
        prev_vrblz->next = NIL;
    }
    else if (first_vrblz)
    {
        first_vrblz->next = NIL;
    }

    m_vrblz_top = first_vrblz;

    if (first_vrblz)
    {
        add_additional_constraints();
    }
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

void Explanation_Based_Chunker::add_goal_or_impasse_tests()
{
    condition* cc;
    tc_number tc;   /* mark each id as we add a test for it, so we don't add
                     a test for the same id in two different places */
    Symbol* idSym, *id_vrblz;
    test t;
    bool isa_goal, isa_impasse;

    tc = get_new_tc_number(thisAgent);
    for (cc = m_vrblz_top; cc != NIL; cc = cc->next)
    {
        if (cc->type != POSITIVE_CONDITION)
        {
            continue;
        }
        idSym = cc->data.tests.id_test->eq_test->data.referent;
        isa_goal = idSym->is_variable() ? idSym->var->instantiated_sym->id->isa_goal : idSym->id->isa_goal;
        isa_impasse = idSym->is_variable() ? idSym->var->instantiated_sym->id->isa_impasse : idSym->id->isa_impasse;
        if ((isa_goal || isa_impasse) && (idSym->tc_num != tc))
        {
            t = make_test(thisAgent, NULL, (isa_goal ? GOAL_ID_TEST : IMPASSE_ID_TEST));
            add_test(thisAgent, &(cc->data.tests.id_test), t);
            idSym->tc_num = tc;
        }
    }

    dprint(DT_VARIABLIZATION_MANAGER, "Conditions after add goal tests: \n%1", m_vrblz_top);

}

/* --------------------------------------------------------------------
                       Make Clones of Results

   When we build the initial instantiation of the new chunk, we have
   to fill in preferences_generated with *copies* of all the result
   preferences.  These copies are clones of the results.  This routine
   makes these clones and fills in chunk_inst->preferences_generated.
-------------------------------------------------------------------- */

void Explanation_Based_Chunker::make_clones_of_results()
{
    preference* lClonedPref, *lResultPref;

    m_chunk_inst->preferences_generated = NIL;
    for (lResultPref = m_results; lResultPref != NIL; lResultPref = lResultPref->next_result)
    {
        /* --- copy the preference --- */
        dprint(DT_CLONES, "Creating clone for result preference %p (instantiation i%u %y)\n", lResultPref, lResultPref->inst->i_id, lResultPref->inst->prod_name);
        lClonedPref = make_preference(thisAgent, lResultPref->type, lResultPref->id, lResultPref->attr, lResultPref->value, lResultPref->referent, lResultPref->clone_identities, true);
        thisAgent->symbolManager->symbol_add_ref(lClonedPref->id);
        thisAgent->symbolManager->symbol_add_ref(lClonedPref->attr);
        thisAgent->symbolManager->symbol_add_ref(lClonedPref->value);
        if (preference_is_binary(lClonedPref->type))
        {
            thisAgent->symbolManager->symbol_add_ref(lClonedPref->referent);
        }
        lClonedPref->inst = m_chunk_inst;
        dprint(DT_CLONES, "Created clone for result preference %p (instantiation i%u %y)\n", lClonedPref, lClonedPref->inst->i_id, lClonedPref->inst->prod_name);

        /* Move cloned_rhs_funcs into rhs_funs of cloned pref */
        if (lResultPref->cloned_rhs_funcs.id)
        {
            lClonedPref->rhs_funcs.id = lResultPref->cloned_rhs_funcs.id;
            lResultPref->cloned_rhs_funcs.id = NULL;
        }
        if (lResultPref->cloned_rhs_funcs.attr)
        {
            lClonedPref->rhs_funcs.attr = lResultPref->cloned_rhs_funcs.attr;
            lResultPref->cloned_rhs_funcs.attr = NULL;
        }
        if (lResultPref->cloned_rhs_funcs.value)
        {
            lClonedPref->rhs_funcs.value = lResultPref->cloned_rhs_funcs.value;
            lResultPref->cloned_rhs_funcs.value = NULL;
        }

        /* --- put it onto the list for chunk_inst --- */
        insert_at_head_of_dll(m_chunk_inst->preferences_generated, lClonedPref, inst_next, inst_prev);

        /* --- insert it into the list of clones for this preference --- */
        lClonedPref->next_clone = lResultPref;
        lClonedPref->prev_clone = lResultPref->prev_clone;
        lResultPref->prev_clone = lClonedPref;
        if (lClonedPref->prev_clone)
        {
            lClonedPref->prev_clone->next_clone = lClonedPref;
        }

    }
}

void Explanation_Based_Chunker::remove_chunk_instantiation()
{
    preference* lNext, *lResultPref;
    bool lRemoved;

    m_chunk_inst->in_ms = false;
    for (lResultPref = m_chunk_inst->preferences_generated; lResultPref != NIL; lResultPref = lNext)
    {
        lNext = lResultPref->inst_next;
        assert(lResultPref->reference_count == 0);
        dprint(DT_EBC_CLEANUP, "Removing cloned preference %p (%d)\n", lResultPref, lResultPref->reference_count);
        lRemoved = remove_preference_from_clones(thisAgent, lResultPref);
        assert(lRemoved);
    }
    m_chunk_inst = NULL;
}

bool Explanation_Based_Chunker::can_learn_from_instantiation()
{
    preference* pref;

    /* --- if it only matched an attribute impasse, don't chunk --- */
    if (! m_inst->match_goal) return false;

    /* --- if no preference is above the match goal level, exit --- */
    for (pref = m_inst->preferences_generated; pref != NIL; pref = pref->inst_next)
        if (pref->id->id->level < m_inst->match_goal_level) break;

    /* Note that in Soar 9.3.4, chunking would not create a chunk for a
     * a result if another result was also being returned to an even higher
     * superstate.  In this part of the code, it would set learning off
     * so that an intermediate justification is created instead.  We disabled that
     * aspect in 2/2013 to fix issues that Shiwali was having with her research
     * agents. Documenting here in case this ever needs to changed back or made
     * an option. */

    return (pref != NULL);
}

void Explanation_Based_Chunker::perform_dependency_analysis()
{
    preference* pref;
    goal_stack_level grounds_level = m_inst->match_goal_level - 1;

    outputManager->set_print_test_format(true, false);
    dprint(DT_BACKTRACE,  "\nBacktracing through base instantiation %y: \n", m_inst->prod_name);
    dprint_header(DT_BACKTRACE, PrintBefore, "Starting dependency analysis...\n");

    increment_counter(backtrace_number);
    increment_counter(grounds_tc);
    grounds = NIL;
    locals = NIL;

    #ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationMemory->set_backtrace_number(backtrace_number);
    #endif

    /* Backtrace through the instantiation that produced each result --- */
    for (pref = m_results; pref != NIL; pref = pref->next_result)
    {
        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "\nFor result preference ");
            xml_begin_tag(thisAgent, kTagBacktraceResult);
            print_preference(thisAgent, pref);
            thisAgent->outputManager->printa(thisAgent, " ");
        }
        backtrace_through_instantiation(pref->inst, grounds_level, NULL, pref->identities, pref->rhs_funcs, 0, (pref->inst == m_inst) ? BT_BaseInstantiation : BT_ExtraResults);

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            xml_end_tag(thisAgent, kTagBacktraceResult);
        }
    }

    trace_locals(grounds_level);

    outputManager->clear_print_test_format();
    dprint_header(DT_BACKTRACE, PrintAfter, "Dependency analysis complete.\n");
    dprint_unification_map(DT_BACKTRACE);
    dprint(DT_BACKTRACE, "Grounds:\n%3", grounds);
    dprint(DT_BACKTRACE, "Locals:\n%3", locals);

}

void Explanation_Based_Chunker::deallocate_failed_chunk()
{
    deallocate_condition_list(thisAgent, m_vrblz_top);
    m_vrblz_top = NULL;
    deallocate_action_list(thisAgent, m_rhs);
    m_rhs = NULL;
}

bool Explanation_Based_Chunker::add_chunk_to_rete()
{
    byte rete_addition_result;
    production* duplicate_rule = NULL;

    rete_addition_result = add_production_to_rete(thisAgent, m_prod, m_vrblz_top, m_chunk_inst, m_should_print_name, duplicate_rule);

    if (m_should_print_prod && (rete_addition_result != DUPLICATE_PRODUCTION))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "\n");
        xml_begin_tag(thisAgent, kTagLearning);
        print_production(thisAgent, m_prod, false);
        xml_end_tag(thisAgent, kTagLearning);
    }
    if (rete_addition_result == REFRACTED_INST_MATCHED)
    {
        assert(m_prod);
        thisAgent->explanationMemory->record_chunk_contents(m_prod, m_vrblz_top, m_rhs, m_results, unification_map, m_inst, m_chunk_inst);
        if (m_prod_type == JUSTIFICATION_PRODUCTION_TYPE) {
            thisAgent->explanationMemory->increment_stat_justifications_succeeded();
        } else {
            thisAgent->explanationMemory->increment_stat_chunks_succeeded();
            if (ebc_settings[SETTING_EBC_INTERRUPT])
            {
                thisAgent->stop_soar = true;
                thisAgent->reason_for_stopping = "Soar learned a new rule.";

            }
            if (ebc_settings[SETTING_EBC_INTERRUPT_WATCHED] && m_prod->explain_its_chunks && thisAgent->explanationMemory->isRecordingChunk())
            {
                thisAgent->stop_soar = true;
                thisAgent->reason_for_stopping = "Soar learned a new rule from a watched production.";
            }
            //            chunk_history += "Successfully created chunk\n";
            //            outputManager->sprinta_sf(thisAgent, chunk_history, "Successfully built chunk %y at time %u.");

        }
        dprint(DT_VARIABLIZATION_MANAGER, "Add production to rete result: Refracted instantiation matched.\n");
        return true;
    } else if (rete_addition_result == DUPLICATE_PRODUCTION) {
        if (m_inst->prod)
        {
            if (thisAgent->d_cycle_count == m_inst->prod->last_duplicate_dc)
            {
                m_inst->prod->duplicate_chunks_this_cycle++;
            } else {
                m_inst->prod->duplicate_chunks_this_cycle = 1;
                m_inst->prod->last_duplicate_dc = thisAgent->d_cycle_count;
            }
        }
        thisAgent->explanationMemory->increment_stat_duplicates(duplicate_rule);
        thisAgent->explanationMemory->cancel_chunk_record();
        dprint(DT_VARIABLIZATION_MANAGER, "Add production to rete result: Duplicate production.\n");
        return false;
    } else if (rete_addition_result == REFRACTED_INST_DID_NOT_MATCH) {
        if (m_prod_type == JUSTIFICATION_PRODUCTION_TYPE)
        {
            thisAgent->explanationMemory->increment_stat_justification_did_not_match();
            if (ebc_settings[SETTING_EBC_INTERRUPT_WARNING])
            {
                thisAgent->stop_soar = true;
                thisAgent->reason_for_stopping = "Warning:  Justification did not match working memory.  Potential issue.";
                print_current_built_rule("Justification that did not match WM: ");
            }
        } else {
            thisAgent->explanationMemory->increment_stat_chunk_did_not_match();
            if (ebc_settings[SETTING_EBC_INTERRUPT_WARNING])
            {
                thisAgent->stop_soar = true;
                thisAgent->reason_for_stopping = "Warning:  Chunk did not match working memory.  Potential issue.";
                print_current_built_rule("Chunk that did not match WM: ");
            }
        }
        dprint(DT_VARIABLIZATION_MANAGER, "Add production to rete result: Refracted instantiation did not match.\n");

        assert(m_prod);
        thisAgent->explanationMemory->record_chunk_contents(m_prod, m_vrblz_top, m_rhs, m_results, unification_map, m_inst, m_chunk_inst);

        m_chunk_inst->in_ms = false;
        return true;
    }

    /* Don't think this can happen */
    dprint(DT_VARIABLIZATION_MANAGER, "Add production to rete result: No refracted instantiation given.\n");
    assert(false);

}

void Explanation_Based_Chunker::learn_EBC_rule(instantiation* inst, instantiation** new_inst_list)
{
    preference*         pref;
    bool                lChunkValidated = true;
    condition*          l_inst_top = NULL;
    condition*          l_inst_bottom = NULL;

    #if !defined(NO_TIMING_STUFF) && defined(DETAILED_TIMING_STATS)
    soar_timer local_timer;
    local_timer.set_enabled(&(thisAgent->trace_settings[ TIMERS_ENABLED ]));
    #endif

    /* --- If we're over MAX_CHUNKS, abort chunk --- */
    if (chunks_this_d_cycle >= max_chunks)
    {
        thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_max_chunks, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);
        #ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationMemory->increment_stat_max_chunks();
        #endif
        m_extra_results = NULL;
        m_inst = NULL;
        return;
    }

    m_inst = inst;
    m_chunk_new_i_id = 0;
    m_failure_type = ebc_success;

    if (!can_learn_from_instantiation()) { m_inst = NULL; return; }

    #if !defined(NO_TIMING_STUFF) && defined(DETAILED_TIMING_STATS)
    local_timer.start();
    #endif

    get_results_for_instantiation();
    if (!m_results) {
        m_extra_results = NULL;
        m_inst = NULL;
        return;
    }

    dprint_header(DT_MILESTONES, PrintBoth, "EBC learning new rule for firing of %y (i%u)\n", inst->prod_name, inst->i_id);
    dprint(DT_VARIABLIZATION_MANAGER, "   Match of %y (i%u):\n%5", inst->prod_name, inst->i_id, inst->top_of_instantiated_conditions, inst->preferences_generated);

    m_reliable = true;
    m_vrblz_top = NULL;


    if (m_inst->prod && (thisAgent->d_cycle_count == m_inst->prod->last_duplicate_dc) && (m_inst->prod->duplicate_chunks_this_cycle >= max_dupes))
    {
        if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
        {
            thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_max_dupes, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);
            thisAgent->outputManager->printa_sf(thisAgent, "         Rule that has reached the max-dupes limit: %y\n", m_inst->prod_name);
        }
        #ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationMemory->increment_stat_max_dupes();
        #endif
        m_extra_results = NULL;
        m_inst = NULL;
        return;
    }

    /* --- Assign a new instantiation ID for this chunk --- */
    set_new_chunk_id();
    thisAgent->memoryManager->allocate_with_pool(MP_instantiation, &m_chunk_inst);

    #ifdef DEBUG_ONLY_CHUNK_ID
    #ifndef DEBUG_ONLY_CHUNK_ID_LAST
    if (m_chunk_new_i_id == DEBUG_ONLY_CHUNK_ID)
    #else
    if ((m_chunk_new_i_id >= DEBUG_ONLY_CHUNK_ID) && (m_chunk_new_i_id <= DEBUG_ONLY_CHUNK_ID_LAST))
    #endif
        {
            dprint(DT_DEBUG, "Turning on debug tracing for chunk ID %u that is flagged for debugging.\n", m_chunk_new_i_id);
            debug_trace_on();
        }
#endif

    dprint(DT_MILESTONES, "Assigning instantiation ID %u to possible chunk forming from match of %y.\n", m_chunk_new_i_id, m_inst->prod_name);
//    dprint(DT_DEBUG, "Chunk number %u\n", m_chunk_new_i_id);
//    if (m_chunk_new_i_id == 9)
//    {
//        dprint(DT_DEBUG, "Chunk found.\n");
//    }
    #ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationMemory->add_chunk_record(m_inst);
    #endif

    /* Set allow_bottom_up_chunks to false for all higher goals to prevent chunking */
    {
        Symbol* g;
        for (g = m_inst->match_goal->id->higher_goal; g && g->id->allow_bottom_up_chunks; g = g->id->higher_goal)
        {
            g->id->allow_bottom_up_chunks = false;
        }
    }

    /* Determine which WMEs in the topstate were relevent to problem-solving */
    perform_dependency_analysis();

    #ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationMemory->increment_stat_chunks_attempted();
    #endif

    /* Collect the grounds into the chunk condition lists */
    create_initial_chunk_condition_lists();

    /* If there aren't any conditions, abort chunk */
    if (!m_vrblz_top)
    {
        if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
        {
            thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_no_conditions, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);
            thisAgent->outputManager->printa_sf(thisAgent, "\nRule firing that led to invalid chunk: %y\n", m_inst->prod_name);
        }
        #ifdef BUILD_WITH_EXPLAINER
        thisAgent->explanationMemory->increment_stat_no_grounds();
        thisAgent->explanationMemory->cancel_chunk_record();
        #endif
    if (ebc_settings[SETTING_EBC_INTERRUPT_WARNING])
        {
            thisAgent->stop_soar = true;
            thisAgent->reason_for_stopping = "Chunking failure:  Rule learned had no conditions.";
        }
        clean_up();
        return;
    }
    dprint(DT_MILESTONES, "Dependency analysis complete.  Unified chunk conditions built for chunk id %u based on firing of %y (i %u)\n", m_chunk_new_i_id, inst->prod_name, inst->i_id);
    dprint(DT_VARIABLIZATION_MANAGER, "Starting conditions from dependency analysis: \n%1", m_vrblz_top);

    /* Determine if we create a justification or chunk */
    m_rule_type = m_learning_on_for_instantiation ? ebc_chunk : ebc_justification;

    if ((m_rule_type == ebc_chunk) && !m_reliable)
    {
        m_rule_type = ebc_justification;
        if (ebc_settings[SETTING_EBC_INTERRUPT_WARNING] && !ebc_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS])
        {
            thisAgent->stop_soar = true;
            thisAgent->reason_for_stopping = "Chunking failure:  Problem-solving contained negated reasoning about sub-state structures.";
        }
    }

    #ifdef BUILD_WITH_EXPLAINER
    if ((m_rule_type == ebc_justification) && !thisAgent->explanationMemory->isRecordingJustifications())
    {
        thisAgent->explanationMemory->cancel_chunk_record();
    }
    #endif

    /* Create the name of the rule based on the type and circumstances of the problem-solving */
    set_up_rule_name();

    #ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationMemory->add_result_instantiations(m_inst, m_results);
    #endif

    /* Variablize the LHS */
    thisAgent->symbolManager->reset_variable_generator(m_vrblz_top, NIL);
    variablize_condition_list(m_vrblz_top);
    dprint(DT_VARIABLIZATION_MANAGER, "Conditions after variablizing: \n%1", m_vrblz_top);

    //if (m_rule_type == ebc_chunk) sanity_check_conditions(m_vrblz_top);

    /* Merge redundant conditions (same identity sets in each element) */
    merge_conditions();

    /* Variablize the RHS preferences into actions */
    m_rhs = variablize_results_into_actions();

    /* Add isa_goal tests for first conditions seen with a goal identifier */
    add_goal_or_impasse_tests();

    /* Validate connectedness of chunk, repair if necessary and then re-order conditions to reduce match costs */
    thisAgent->name_of_production_being_reordered = m_prod_name->sc->name;
    if ((m_rule_type == ebc_chunk) || (ebc_settings[SETTING_EBC_REPAIR_JUSTIFICATIONS]))
    {
        lChunkValidated = reorder_and_validate_chunk();
        dprint(DT_VARIABLIZATION_MANAGER, "Conditions after re-ordering and repair:\n%1", m_vrblz_top);
    }

    /* Handle rule learning failure.  With the addition of rule repair, this should only happen when there
     * is a repair failure.  Unless there's a bug in the repair code, all rules should be reparable. */
    if (!lChunkValidated)
    {
        if (m_rule_type == ebc_chunk)
        {
            /* Could not re-order chunk, so we create a justification for the results instead */
            m_rule_type = ebc_justification;
            if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
            {
                thisAgent->outputManager->printa_sf(thisAgent, "Soar will learn a justification instead of a variablized rule.");
            }
            #ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationMemory->increment_stat_chunks_reverted();
            #endif
        } else if (ebc_settings[SETTING_EBC_DONT_ADD_BAD_JUSTIFICATIONS]){
            thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_invalid_justification, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);
            deallocate_failed_chunk();
            #ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationMemory->cancel_chunk_record();
            #endif
            clean_up();
            #ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationMemory->increment_stat_justifications_ungrounded_ignored();
            #endif
            return;
        } else {
            #ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationMemory->increment_stat_justifications_ungrounded_added();
            #endif
        }
    }

    /* Perform re-instantiation
     *
     * The next step in EBC will create an instantiated list of conditions based on the
     * variablized conditions.  This is used as the instantiation for the chunk at the goal
     * level (m_chunk_inst).  After being submitted to the RETE, it is then chunked over
     * if necessary for bottom up chunking.
     *
     * Note:  Until this point, justification and chunk learning has been nearly identical.
     * The only difference is that a justification has not had its conditions re-ordered.
     * This changes during re-instantiation.  If the rule being formed is a justification,
     * both m_vrblz_top and m_rhs will become instantiated as well. */

    l_inst_top = reinstantiate_current_rule();
    l_inst_bottom = l_inst_top;
    while (l_inst_bottom->next) l_inst_bottom = l_inst_bottom->next;

    /* Create the production that will be added to the RETE */
    m_prod = make_production(thisAgent, m_prod_type, m_prod_name, m_inst->prod ? m_inst->prod->original_rule_name : m_inst->prod_name->sc->name, &m_vrblz_top, &m_rhs, false, NULL);

    #ifdef BUILD_WITH_EXPLAINER
    if (m_inst->prod && m_inst->prod->explain_its_chunks)
    {
        m_prod->explain_its_chunks = true;
    }
    #endif
    m_prod_name = NULL;     /* Production struct is now responsible for the production name, so clear local pointer so we don't accidentally delete. */


    /* Fill out the instantiation for the chunk */
    m_chunk_inst->top_of_instantiated_conditions    = NULL;
    m_chunk_inst->bottom_of_instantiated_conditions = NULL;
    m_chunk_inst->preferences_cached = NULL;
    m_chunk_inst->top_of_instantiated_conditions    = l_inst_top;
    m_chunk_inst->bottom_of_instantiated_conditions = l_inst_bottom;
    m_chunk_inst->prod                              = m_prod;
    m_chunk_inst->prod_name                         = m_prod->name;
    thisAgent->symbolManager->symbol_add_ref(m_chunk_inst->prod_name);
    m_chunk_inst->GDS_evaluated_already             = false;
    m_chunk_inst->i_id                              = m_chunk_new_i_id;
    m_chunk_inst->reliable                          = m_reliable;
    m_chunk_inst->in_ms                             = true;  /* set true for now, we'll find out later... */
    m_chunk_inst->in_newly_created                  = true;
    m_chunk_inst->in_newly_deleted                  = false;
    m_chunk_inst->explain_status                    = explain_unrecorded;
    m_chunk_inst->explain_depth                     = 0;
    m_chunk_inst->explain_tc_num                    = 0;
    make_clones_of_results();
    init_instantiation(thisAgent, m_chunk_inst, true, m_inst);

    dprint(DT_VARIABLIZATION_MANAGER, "m_chunk_inst adding to RETE: \n%5", m_chunk_inst->top_of_instantiated_conditions, m_chunk_inst->preferences_generated);
    dprint(DT_DEALLOCATE_INST, "Allocating instantiation %u (match of %y) for new chunk and adding to newly_created_instantion list.\n", m_chunk_new_i_id, m_inst->prod_name);

    /* Add to RETE */
    bool lAddedSuccessfully = add_chunk_to_rete();

    if (lAddedSuccessfully)
    {
        /* --- Add chunk instantiation to list of newly generated instantiations --- */
        m_chunk_inst->next = (*new_inst_list);
        (*new_inst_list) = m_chunk_inst;

        /* Clean up.  (Now that m_chunk_inst s on the list of insts to be asserted, we
         *             set it to to null because so that clean_up() won't delete it.) */
        m_chunk_inst = NULL;
        clean_up();

        /* Initiate bottom-up chunking, i.e. tell EBC to learn a rule for the chunk instantiation,
         * which is what would have been created if the chunk had fired on its goal level */
        dprint(DT_MILESTONES, "Starting bottom-up call to learn_ebc_rule() from %y\n", (*new_inst_list)->prod_name);
        set_learning_for_instantiation(*new_inst_list);
        learn_EBC_rule(*new_inst_list, new_inst_list);
        dprint(DT_MILESTONES, "Finished bottom-up call to learn_ebc_rule()\n");

    } else {
        /* Clean up failed chunk completely*/
        dprint(DT_DEALLOCATE_INST, "Rule addition failed.  Deallocating chunk instantiation.\n");
        m_chunk_inst->in_newly_created = false;
        excise_production(thisAgent, m_chunk_inst->prod, false, true);
        m_chunk_inst->prod = NULL;
        remove_chunk_instantiation();
        clean_up();
    }
}

void Explanation_Based_Chunker::clean_up ()
{
    if (m_chunk_new_i_id)
    {
        thisAgent->explanationBasedChunker->cleanup_after_instantiation_creation(m_chunk_new_i_id);
    }
    thisAgent->explanationMemory->end_chunk_record();
    if (m_chunk_inst)
    {
        thisAgent->memoryManager->free_with_pool(MP_instantiation, m_chunk_inst);
        m_chunk_inst = NULL;
    }
    if (m_vrblz_top)
    {
        deallocate_condition_list(thisAgent, m_vrblz_top);
    }
    if (m_prod_name)
    {
        dprint_header(DT_MILESTONES, PrintAfter, "chunk_instantiation() done building and cleaning up for chunk %y.\n", m_prod_name);
        thisAgent->symbolManager->symbol_remove_ref(&m_prod_name);
    }
    m_inst                              = NULL;
    m_results                           = NULL;
    m_extra_results                     = NULL;
    m_vrblz_top                         = NULL;
    m_rhs                               = NULL;
    m_prod                              = NULL;
    m_chunk_inst                        = NULL;
    m_prod_name                         = NULL;
    m_rule_type                         = ebc_no_rule;
    m_failure_type                      = ebc_success;
    clear_variablization_maps();
    clear_cached_constraints();
    clear_o_id_substitution_map();
    clear_attachment_map();
    clear_singletons();
    #ifdef DEBUG_ONLY_CHUNK_ID
    #ifndef DEBUG_ONLY_CHUNK_ID_LAST
    if (m_chunk_new_i_id == DEBUG_ONLY_CHUNK_ID)
    #else
    if (m_chunk_new_i_id >= DEBUG_ONLY_CHUNK_ID_LAST)
    #endif
    {
        dprint(DT_DEBUG, "Turning off debug tracing for chunk ID %u.\n", m_chunk_new_i_id);
        debug_trace_off();
    }
    #endif

    #if !defined(NO_TIMING_STUFF) && defined(DETAILED_TIMING_STATS)
    local_timer.stop();
    thisAgent->timers_chunking_cpu_time[thisAgent->current_phase].update(local_timer);
    #endif

}

