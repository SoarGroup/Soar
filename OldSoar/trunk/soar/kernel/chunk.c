/*************************************************************************
 *
 *  file:  chunk.c
 *
 * =======================================================================
 *  Supports the learning mechanism in Soar.  Learning can be set
 *  on | off | only | except (for other choices see soarCommands.c: learn).
 *  If set to "only" | "except" users must specify rhs functions in
 *  productions: dont-learn | force-learn.   See rhsfun.c
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

/* ====================================================================

                          Chunking Routines

   ==================================================================== */

#include "soarkernel.h"
#include <ctype.h>
#include "soar_core_utils.h"
#include "explain.h"

extern byte type_of_existing_impasse(Symbol * goal);
extern wme *find_impasse_wme(Symbol * id, Symbol * attr);
extern void find_match_goal(instantiation * inst);

/* =====================================================================

                           Results Calculation

   Get_results_for_instantiation() finds and returns the result preferences
   for a given instantiation.  This is the main routine here.

   The results are accumulated in the list "results," linked via the
   "next_result" field of the preference structures.  (BUGBUG: to save
   space, just use conses for this.)

   Add_pref_to_results() adds a preference to the results.
   Add_results_for_id() adds any preferences for the given identifier.
   Identifiers are marked with results_tc_number as they are added.
===================================================================== */

void add_results_for_id(Symbol * id);
#ifdef DONT_CALC_GDS_OR_BT
static void add_named_superstate_attribute_to_grounds(instantiation * inst, char *name);
#endif

#ifdef SINGLE_THIN_JUSTIFICATION
void deallocate_inst_members_to_be_rewritten(instantiation * inst);
void second_stage_chunk_instantiation(instantiation * inst);
void re_fill_in_instantiation_stuff_for_modified_lhs(instantiation * inst, bool need_to_do_support_calculations);

#endif                          /* SINGLE_THIN_JUSTIFICATION */

#define add_results_if_needed(sym) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      if ( ((sym)->id.level >= current_agent(results_match_goal_level)) && \
           ((sym)->id.tc_num != current_agent(results_tc_number)) ) \
        add_results_for_id(sym); }

void add_pref_to_results(preference * pref)
{
    preference *p;

    /* --- if an equivalent pref is already a result, don't add this one --- */
    for (p = current_agent(results); p != NIL; p = p->next_result) {
        if (p->id != pref->id)
            continue;
        if (p->attr != pref->attr)
            continue;
        if (p->value != pref->value)
            continue;
        if (p->type != pref->type)
            continue;
        if (preference_is_unary(pref->type))
            return;
        if (p->referent != pref->referent)
            continue;
        return;
    }

#ifdef NO_TOP_JUST
    /* --- if pref isn't at the right level, find a clone that is --- */
    if (pref->match_goal_level != current_agent(results_match_goal_level)) {
        for (p = pref->next_clone; p != NIL; p = p->next_clone)
            if (p->match_goal_level == current_agent(results_match_goal_level))
                break;
        if (!p)
            for (p = pref->prev_clone; p != NIL; p = p->prev_clone)
                if (p->match_goal_level == current_agent(results_match_goal_level))
                    break;
        if (!p)
            return;             /* if can't find one, it isn't a result */
        pref = p;
    }
#else

    /* --- if pref isn't at the right level, find a clone that is --- */
    if (pref->inst->match_goal_level != current_agent(results_match_goal_level)) {
        for (p = pref->next_clone; p != NIL; p = p->next_clone)
            if (p->inst->match_goal_level == current_agent(results_match_goal_level))
                break;
        if (!p)
            for (p = pref->prev_clone; p != NIL; p = p->prev_clone)
                if (p->inst->match_goal_level == current_agent(results_match_goal_level))
                    break;
        if (!p)
            return;             /* if can't find one, it isn't a result */
        pref = p;
    }
#endif

    /* --- add this preference to the result list --- */
    pref->next_result = current_agent(results);
    current_agent(results) = pref;

    /* --- follow transitive closuse through value, referent links --- */
    add_results_if_needed(pref->value);
    if (preference_is_binary(pref->type))
        add_results_if_needed(pref->referent);
}

void add_results_for_id(Symbol * id)
{
    slot *s;
    preference *pref;
    wme *w;

    id->id.tc_num = current_agent(results_tc_number);

    /* --- scan through all preferences and wmes for all slots for this id --- */
    for (w = id->id.input_wmes; w != NIL; w = w->next)
        add_results_if_needed(w->value);
    for (s = id->id.slots; s != NIL; s = s->next) {
        for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
            add_pref_to_results(pref);
        for (w = s->wmes; w != NIL; w = w->next)
            add_results_if_needed(w->value);
    }                           /* end of for slots loop */
    /* --- now scan through extra prefs and look for any with this id --- */
    for (pref = current_agent(extra_result_prefs_from_instantiation); pref != NIL; pref = pref->inst_next) {
        if (pref->id == id)
            add_pref_to_results(pref);
    }
}

preference *get_results_for_instantiation(instantiation * inst)
{
    preference *pref;

    current_agent(results) = NIL;
    current_agent(results_match_goal_level) = inst->match_goal_level;
    current_agent(results_tc_number) = get_new_tc_number();
    current_agent(extra_result_prefs_from_instantiation) = inst->preferences_generated;
    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
        if ((pref->id->id.level < current_agent(results_match_goal_level)) &&
            (pref->id->id.tc_num != current_agent(results_tc_number))) {
            add_pref_to_results(pref);

        }
    return current_agent(results);
}

/* =====================================================================

                  Variablizing Conditions and Results

   Variablizing of conditions is done by walking over a condition list
   and destructively modifying it, replacing tests of identifiers with
   tests of tests of variables.  The identifier-to-variable mapping is
   built as we go along:  identifiers that have already been assigned
   a variablization are marked with id.tc_num==variablization_tc, and
   id.variablization points to the corresponding variable.

   Variablizing of results can't be done destructively because we need
   to convert the results--preferences--into actions.  This is done
   by copy_and_variablize_result_list(), which takes the result preferences
   and returns an action list.

   The global variable "variablize_this_chunk" indicates whether to
   variablize at all.  This flag is set to TRUE or FALSE before and during
   backtracing.  FALSE means the new production will become a justification;
   TRUE means it will be a chunk.
===================================================================== */

void variablize_symbol(Symbol ** sym)
{
    char prefix[2];
    Symbol *var;

    if ((*sym)->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
        return;
    if (!current_agent(variablize_this_chunk))
        return;

    if ((*sym)->id.tc_num == current_agent(variablization_tc)) {
        /* --- it's already been variablized, so use the existing variable --- */
        var = (*sym)->id.variablization;
        symbol_remove_ref(*sym);
        *sym = var;
        symbol_add_ref(var);
        return;
    }

    /* --- need to create a new variable --- */
    (*sym)->id.tc_num = current_agent(variablization_tc);
    prefix[0] = (char) tolower((*sym)->id.name_letter);
    prefix[1] = 0;
    var = generate_new_variable(prefix);
    (*sym)->id.variablization = var;
    symbol_remove_ref(*sym);
    *sym = var;
}

void variablize_test(test * t)
{
    cons *c;
    complex_test *ct;

    if (test_is_blank_test(*t))
        return;
    if (test_is_blank_or_equality_test(*t)) {
        variablize_symbol((Symbol **) t);
        /* Warning: this relies on the representation of tests */
        return;
    }

    ct = complex_test_from_test(*t);

    switch (ct->type) {
    case GOAL_ID_TEST:
    case IMPASSE_ID_TEST:
    case DISJUNCTION_TEST:
        return;
    case CONJUNCTIVE_TEST:
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            variablize_test((test *) (&(c->first)));
        return;
    default:                   /* relational tests other than equality */
        variablize_symbol(&(ct->data.referent));
        return;
    }
}

void variablize_condition_list(condition * cond)
{
    for (; cond != NIL; cond = cond->next) {
        switch (cond->type) {
        case POSITIVE_CONDITION:
        case NEGATIVE_CONDITION:
            variablize_test(&(cond->data.tests.id_test));
            variablize_test(&(cond->data.tests.attr_test));
            variablize_test(&(cond->data.tests.value_test));
            break;
        case CONJUNCTIVE_NEGATION_CONDITION:
            variablize_condition_list(cond->data.ncc.top);
            break;
        }
    }
}

action *copy_and_variablize_result_list(preference * pref)
{
    action *a;
    Symbol *temp;

    if (!pref)
        return NIL;
    allocate_with_pool(&current_agent(action_pool), &a);
    a->type = MAKE_ACTION;

    temp = pref->id;
    symbol_add_ref(temp);
    variablize_symbol(&temp);
    a->id = symbol_to_rhs_value(temp);

    temp = pref->attr;
    symbol_add_ref(temp);
    variablize_symbol(&temp);
    a->attr = symbol_to_rhs_value(temp);

    temp = pref->value;
    symbol_add_ref(temp);
    variablize_symbol(&temp);
    a->value = symbol_to_rhs_value(temp);

    a->preference_type = pref->type;

    if (preference_is_binary(pref->type)) {
        temp = pref->referent;
        symbol_add_ref(temp);
        variablize_symbol(&temp);
        a->referent = symbol_to_rhs_value(temp);
    }

    a->next = copy_and_variablize_result_list(pref->next_result);
    return a;
}

/* ====================================================================

     Chunk Conditions, and Chunk Conditions Set Manipulation Routines

   These structures have two uses.  First, for every ground condition,
   one of these structures maintains certain information about it--
   pointers to the original (instantiation's) condition, the chunks's
   instantiation's condition, and the variablized condition, etc.

   Second, for negated conditions, these structures are entered into
   a hash table with keys hash_condition(this_cond).  This hash table
   is used so we can add a new negated condition to the set of negated
   potentials quickly--we don't want to add a duplicate of a negated
   condition that's already there, and the hash table lets us quickly
   determine whether a duplicate is already there.

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
   and returns TRUE if the condition isn't already in the set.  If the 
   condition is already in the set, the routine deallocates the given
   chunk_cond and returns FALSE.

   Remove_from_chunk_cond_set() removes a given chunk_cond from a given
   chunk_cond_set, but doesn't deallocate it.
-------------------------------------------------------------------- */

                             /* set of all negated conditions we encounter
                                during backtracing--these are all potentials
                                and (some of them) are added to the grounds
                                in one pass at the end of the backtracing */

void init_chunk_cond_set(chunk_cond_set * set)
{
    int i;

    set->all = NIL;
    for (i = 0; i < CHUNK_COND_HASH_TABLE_SIZE; i++)
        set->table[i] = NIL;
}

chunk_cond *make_chunk_cond_for_condition(condition * cond)
{
    chunk_cond *cc;
    unsigned long remainder, hv;

    allocate_with_pool(&current_agent(chunk_cond_pool), &cc);
    cc->cond = cond;
    cc->hash_value = hash_condition(cond);
    remainder = cc->hash_value;
    hv = 0;
    while (remainder) {
        hv ^= (remainder & masks_for_n_low_order_bits[LOG_2_CHUNK_COND_HASH_TABLE_SIZE]);
        remainder = remainder >> LOG_2_CHUNK_COND_HASH_TABLE_SIZE;
    }
    cc->compressed_hash_value = hv;
    return cc;
}

bool add_to_chunk_cond_set(chunk_cond_set * set, chunk_cond * new_cc)
{
    chunk_cond *old;

    for (old = set->table[new_cc->compressed_hash_value]; old != NIL; old = old->next_in_bucket)
        if (old->hash_value == new_cc->hash_value)
            if (conditions_are_equal(old->cond, new_cc->cond))
                break;
    if (old) {
        /* --- the new condition was already in the set; so don't add it --- */
        free_with_pool(&current_agent(chunk_cond_pool), new_cc);
        return FALSE;
    }
    /* --- add new_cc to the table --- */
    insert_at_head_of_dll(set->all, new_cc, next, prev);
    insert_at_head_of_dll(set->table[new_cc->compressed_hash_value], new_cc, next_in_bucket, prev_in_bucket);
    return TRUE;
}

void remove_from_chunk_cond_set(chunk_cond_set * set, chunk_cond * cc)
{
    remove_from_dll(set->all, cc, next, prev);
    remove_from_dll(set->table[cc->compressed_hash_value], cc, next_in_bucket, prev_in_bucket);
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

void build_chunk_conds_for_grounds_and_add_negateds(chunk_cond ** dest_top,
                                                    chunk_cond ** dest_bottom, tc_number tc_to_use)
{
    cons *c;
    condition *ground;
    chunk_cond *cc, *first_cc, *prev_cc;

    first_cc = NIL;             /* unnecessary, but gcc -Wall warns without it */

    /* --- build instantiated conds for grounds and setup their TC --- */
    prev_cc = NIL;
    while (current_agent(grounds)) {
        c = current_agent(grounds);
        current_agent(grounds) = current_agent(grounds)->rest;
        ground = c->first;
        free_cons(c);
        /* --- make the instantiated condition --- */
        allocate_with_pool(&current_agent(chunk_cond_pool), &cc);
        cc->cond = ground;
        cc->instantiated_cond = copy_condition(cc->cond);
        cc->variablized_cond = copy_condition(cc->cond);
        if (prev_cc) {
            prev_cc->next = cc;
            cc->prev = prev_cc;
            cc->variablized_cond->prev = prev_cc->variablized_cond;
            prev_cc->variablized_cond->next = cc->variablized_cond;
        } else {
            first_cc = cc;
            cc->prev = NIL;
            cc->variablized_cond->prev = NIL;
        }
        prev_cc = cc;
        /* --- add this in to the TC --- */
        add_cond_to_tc(ground, tc_to_use, NIL, NIL);
    }

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

    /* --- scan through negated conditions and check which ones are connected
       to the grounds --- */
    if (current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM])
        print_string("\n\n*** Adding Grounded Negated Conditions ***\n");
#endif

    while (current_agent(negated_set).all) {
        cc = current_agent(negated_set).all;
        remove_from_chunk_cond_set(&current_agent(negated_set), cc);
        if (cond_is_in_tc(cc->cond, tc_to_use)) {

#ifndef TRACE_CONTEXT_DECISIONS_ONLY
            /* --- negated cond is in the TC, so add it to the grounds --- */
            if (current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM]) {
                print_string("\n-->Moving to grounds: ");
                print_condition(cc->cond);
            }
#endif

            cc->instantiated_cond = copy_condition(cc->cond);
            cc->variablized_cond = copy_condition(cc->cond);
            if (prev_cc) {
                prev_cc->next = cc;
                cc->prev = prev_cc;
                cc->variablized_cond->prev = prev_cc->variablized_cond;
                prev_cc->variablized_cond->next = cc->variablized_cond;
            } else {
                first_cc = cc;
                cc->prev = NIL;
                cc->variablized_cond->prev = NIL;
            }
            prev_cc = cc;
        } else {
            /* --- not in TC, so discard the condition --- */
            free_with_pool(&current_agent(chunk_cond_pool), cc);
        }
    }

    if (prev_cc) {
        prev_cc->next = NIL;
        prev_cc->variablized_cond->next = NIL;
    } else {
        first_cc = NIL;
    }

    *dest_top = first_cc;
    *dest_bottom = prev_cc;
}

/* --------------------------------------------------------------------
                  Get Nots For Instantiated Conditions

   This routine looks through all the Nots in the instantiations in
   instantiations_with_nots, and returns copies of the ones involving
   pairs of identifiers in the grounds.  Before this routine is called,
   the ids in the grounds must be marked with "tc_of_grounds."  
-------------------------------------------------------------------- */

not *get_nots_for_instantiated_conditions(list * instantiations_with_nots, tc_number tc_of_grounds)
{
    cons *c;
    instantiation *inst;
    not *n1, *n2, *new_not, *collected_nots;

    /* --- collect nots for which both id's are marked --- */
    collected_nots = NIL;
    while (instantiations_with_nots) {
        c = instantiations_with_nots;
        instantiations_with_nots = c->rest;
        inst = c->first;
        free_cons(c);
        for (n1 = inst->nots; n1 != NIL; n1 = n1->next) {
            /* --- Are both id's marked? If no, goto next loop iteration --- */
            if (n1->s1->id.tc_num != tc_of_grounds)
                continue;
            if (n1->s2->id.tc_num != tc_of_grounds)
                continue;
            /* --- If the pair already in collected_nots, goto next iteration --- */
            for (n2 = collected_nots; n2 != NIL; n2 = n2->next) {
                if ((n2->s1 == n1->s1) && (n2->s2 == n1->s2))
                    break;
                if ((n2->s1 == n1->s2) && (n2->s2 == n1->s1))
                    break;
            }
            if (n2)
                continue;
            /* --- Add the pair to collected_nots --- */
            allocate_with_pool(&current_agent(not_pool), &new_not);
            new_not->next = collected_nots;
            collected_nots = new_not;
            new_not->s1 = n1->s1;
            symbol_add_ref(new_not->s1);
            new_not->s2 = n1->s2;
            symbol_add_ref(new_not->s2);
        }                       /* end of for n1 */
    }                           /* end of while instantiations_with_nots */

    return collected_nots;
}

/* --------------------------------------------------------------------
              Variablize Nots And Insert Into Conditions
             
   This routine goes through the given list of Nots and, for each one,
   inserts a variablized copy of it into the given condition list at
   the earliest possible location.  (The given condition list should
   be the previously-variablized condition list that will become the
   chunk's LHS.)  The given condition list is destructively modified;
   the given Not list is unchanged.
-------------------------------------------------------------------- */

void variablize_nots_and_insert_into_conditions(not * nots, condition * conds)
{
    not *n;
    Symbol *var1, *var2;
    test t;
    complex_test *ct;
    condition *c;
    bool added_it;

    /* --- don't bother Not-ifying justifications --- */
    if (!current_agent(variablize_this_chunk))
        return;

    for (n = nots; n != NIL; n = n->next) {
        var1 = n->s1->id.variablization;
        var2 = n->s2->id.variablization;
        /* --- find where var1 is bound, and add "<> var2" to that test --- */
        allocate_with_pool(&current_agent(complex_test_pool), &ct);
        t = make_test_from_complex_test(ct);
        ct->type = NOT_EQUAL_TEST;
        ct->data.referent = var2;
        symbol_add_ref(var2);
        added_it = FALSE;
        for (c = conds; c != NIL; c = c->next) {
            if (c->type != POSITIVE_CONDITION)
                continue;
            if (test_includes_equality_test_for_symbol(c->data.tests.id_test, var1)) {
                add_new_test_to_test(&(c->data.tests.id_test), t);
                added_it = TRUE;
                break;
            }
            if (test_includes_equality_test_for_symbol(c->data.tests.attr_test, var1)) {
                add_new_test_to_test(&(c->data.tests.attr_test), t);
                added_it = TRUE;
                break;
            }
            if (test_includes_equality_test_for_symbol(c->data.tests.value_test, var1)) {
                add_new_test_to_test(&(c->data.tests.value_test), t);
                added_it = TRUE;
                break;
            }
        }
        if (!added_it) {
            char msg[MESSAGE_SIZE];
            strncpy(msg, "chunk.c: Internal error: couldn't add Not test to chunk\n", MESSAGE_SIZE);
            msg[MESSAGE_SIZE - 1] = 0;
            abort_with_fatal_error(msg);
        }
    }                           /* end of for n=nots */
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

void add_goal_or_impasse_tests(chunk_cond * all_ccs)
{
    chunk_cond *cc;
    tc_number tc;               /* mark each id as we add a test for it, so we don't add
                                   a test for the same id in two different places */
    Symbol *id;
    test t;
    complex_test *ct;

    tc = get_new_tc_number();
    for (cc = all_ccs; cc != NIL; cc = cc->next) {
        if (cc->instantiated_cond->type != POSITIVE_CONDITION)
            continue;
        id = referent_of_equality_test(cc->instantiated_cond->data.tests.id_test);
        if ((id->id.isa_goal || id->id.isa_impasse) && (id->id.tc_num != tc)) {
            allocate_with_pool(&current_agent(complex_test_pool), &ct);
            ct->type = (char) ((id->id.isa_goal) ? GOAL_ID_TEST : IMPASSE_ID_TEST);
            t = make_test_from_complex_test(ct);
            add_new_test_to_test(&(cc->variablized_cond->data.tests.id_test), t);
            id->id.tc_num = tc;
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

   Okay, so the obvious way is to have each variablized condition (VCond)
   point to the corresponding instantiated condition (ICond).  Then after
   reordering the VConds, we'd scan through the VConds and say
      VCond->Icond->next = VCond->next->Icond
      VCond->Icond->prev = VCond->prev->Icond
   (with some extra checks for the first and last VCond in the list).

   The problem with this is that it takes an extra 4 bytes per condition,
   for the "ICond" field.  Conditions were taking up a lot of memory in
   my test cases, so I wanted to shrink them.  This routine avoids needing
   the 4 extra bytes by using the following trick:  first "swap out" 4
   bytes from each VCond; then use that 4 bytes for the "ICond" field.
   Now run the above algorithm.  Finally, swap those original 4 bytes
   back in.
-------------------------------------------------------------------- */

void reorder_instantiated_conditions(chunk_cond * top_cc, condition ** dest_inst_top, condition ** dest_inst_bottom)
{
    chunk_cond *cc;

    /* --- Step 1:  swap prev pointers out of variablized conds into chunk_conds,
       and swap pointer to the corresponding instantiated conds into the
       variablized conds' prev pointers --- */
    for (cc = top_cc; cc != NIL; cc = cc->next) {
        cc->saved_prev_pointer_of_variablized_cond = cc->variablized_cond->prev;
        cc->variablized_cond->prev = cc->instantiated_cond;
    }

    /* --- Step 2:  do the reordering of the instantiated conds --- */
    for (cc = top_cc; cc != NIL; cc = cc->next) {
        if (cc->variablized_cond->next) {
            cc->instantiated_cond->next = cc->variablized_cond->next->prev;
        } else {
            cc->instantiated_cond->next = NIL;
            *dest_inst_bottom = cc->instantiated_cond;
        }

        if (cc->saved_prev_pointer_of_variablized_cond) {
            cc->instantiated_cond->prev = cc->saved_prev_pointer_of_variablized_cond->prev;
        } else {
            cc->instantiated_cond->prev = NIL;
            *dest_inst_top = cc->instantiated_cond;
        }
    }

    /* --- Step 3:  restore the prev pointers on variablized conds --- */
    for (cc = top_cc; cc != NIL; cc = cc->next) {
        cc->variablized_cond->prev = cc->saved_prev_pointer_of_variablized_cond;
    }
}

/* --------------------------------------------------------------------
                       Make Clones of Results

   When we build the initial instantiation of the new chunk, we have
   to fill in preferences_generated with *copies* of all the result
   preferences.  These copies are clones of the results.  This routine
   makes these clones and fills in chunk_inst->preferences_generated.
-------------------------------------------------------------------- */

void make_clones_of_results(preference * results, instantiation * chunk_inst)
{
    preference *p, *result_p;

    chunk_inst->preferences_generated = NIL;
    for (result_p = results; result_p != NIL; result_p = result_p->next_result) {
        /* --- copy the preference --- */
        p = make_preference(result_p->type, result_p->id, result_p->attr, result_p->value, result_p->referent);
        symbol_add_ref(p->id);
        symbol_add_ref(p->attr);
        symbol_add_ref(p->value);
        if (preference_is_binary(p->type))
            symbol_add_ref(p->referent);
        /* --- put it onto the list for chunk_inst --- */
        p->inst = chunk_inst;
        insert_at_head_of_dll(chunk_inst->preferences_generated, p, inst_next, inst_prev);
        /* --- insert it into the list of clones for this preference --- */
        p->next_clone = result_p;
        p->prev_clone = result_p->prev_clone;
        result_p->prev_clone = p;
        if (p->prev_clone)
            p->prev_clone->next_clone = p;
    }
}

/* kjh (B14) begin */
Symbol *find_goal_at_goal_stack_level(goal_stack_level level)
{
    Symbol *g;

    for (g = current_agent(top_goal); g != NIL; g = g->id.lower_goal)
        if (g->id.level == level)
            return (g);
    return (NIL);
}

Symbol *find_impasse_wme_value(Symbol * id, Symbol * attr)
{
    wme *w;

    for (w = id->id.impasse_wmes; w != NIL; w = w->next)
        if (w->attr == attr)
            return w->value;
    return NIL;
}

#define NAME_SIZE 512
#define IMPASS_NAME_SIZE 32
Symbol *generate_chunk_name_sym_constant(instantiation * inst)
{
    char name[NAME_SIZE];
    char impass_name[IMPASS_NAME_SIZE];
    Symbol *generated_name;
    Symbol *goal;
    byte impasse_type;
    preference *p;
    goal_stack_level lowest_result_level;

    if (!current_agent(sysparams)[USE_LONG_CHUNK_NAMES])
        return (generate_new_sym_constant(current_agent(chunk_name_prefix), &current_agent(chunk_count)));

    lowest_result_level = current_agent(top_goal)->id.level;
    for (p = inst->preferences_generated; p != NIL; p = p->inst_next)
        if (p->id->id.level > lowest_result_level)
            lowest_result_level = p->id->id.level;

    goal = find_goal_at_goal_stack_level(lowest_result_level);

    if (goal) {
        impasse_type = type_of_existing_impasse(goal);

        /* Note that in this switch statement we set the impasse_name variable "safely" using strncpy.
           strncpy automatically null terminates the copy unless it was truncated.  To be safe, then,
           we always put a NULL character at the end of the array. In this case, we can do this after
           the if/else statements instead of right after every strncpy since the result of every
           strncpy gets funneled past there.
         */
        switch (impasse_type) {
        case NONE_IMPASSE_TYPE:
            print("Internal error: impasse_type is NONE_IMPASSE_TYPE during chunk creation.\n");
            strncpy(impass_name, "unknownimpasse", IMPASS_NAME_SIZE);
            break;
        case CONSTRAINT_FAILURE_IMPASSE_TYPE:
            strncpy(impass_name, "cfailure", IMPASS_NAME_SIZE);
            break;
        case CONFLICT_IMPASSE_TYPE:
            strncpy(impass_name, "conflict", IMPASS_NAME_SIZE);
            break;
        case TIE_IMPASSE_TYPE:
            strncpy(impass_name, "tie", IMPASS_NAME_SIZE);
            break;
        case NO_CHANGE_IMPASSE_TYPE:
            {
                Symbol *sym;

                if ((sym = find_impasse_wme_value(goal->id.lower_goal, current_agent(attribute_symbol))) == NIL) {
#ifdef DEBUG_CHUNK_NAMES
                    print("Internal error: Failed to find ^attribute impasse wme.\n");
                    do_print_for_identifier(goal->id.lower_goal, 1, 0);
#endif
                    strncpy(impass_name, "unknownimpasse", IMPASS_NAME_SIZE);
                } else if (sym == current_agent(operator_symbol)) {
                    strncpy(impass_name, "opnochange", IMPASS_NAME_SIZE);
                } else if (sym == current_agent(state_symbol)) {
                    strncpy(impass_name, "snochange", IMPASS_NAME_SIZE);
                } else {
#ifdef DEBUG_CHUNK_NAMES
                    print("Internal error: ^attribute impasse wme has unexpected value.\n");
#endif
                    strncpy(impass_name, "unknownimpasse", IMPASS_NAME_SIZE);
                }
            }
            break;
        default:
            print("Internal error: encountered unknown impasse_type: %d.\n", impasse_type);
            strncpy(impass_name, "unknownimpasse", IMPASS_NAME_SIZE);
            break;
        }
    } else {
        print("Internal error: Failed to determine impasse type.\n");
        strncpy(impass_name, "unknownimpasse", IMPASS_NAME_SIZE);
    }
    impass_name[IMPASS_NAME_SIZE - 1] = 0;

    snprintf(name, NAME_SIZE, "%s-%lu*d%lu*%s*%lu",
             current_agent(chunk_name_prefix),
             current_agent(chunk_count++), current_agent(d_cycle_count), impass_name, current_agent(chunks_this_d_cycle)
        );
    name[NAME_SIZE - 1] = 0;    /* snprintf doesn't set last char to null if output is truncated */

    /* Any user who named a production like this deserves to be burned, but we'll have mercy: */
    if (find_sym_constant(name)) {
        unsigned long collision_count;

        collision_count = 1;
        print("Warning: generated chunk name already exists.  Will find unique name.\n");
        do {
            snprintf(name, NAME_SIZE, "%s-%lu*d%lu*%s*%lu*%lu",
                     current_agent(chunk_name_prefix),
                     current_agent(chunk_count++),
                     current_agent(d_cycle_count), impass_name, current_agent(chunks_this_d_cycle), collision_count++);
            name[NAME_SIZE - 1] = 0;    /* snprintf doesn't set last char to null if output is truncated */
        } while (find_sym_constant(name));
    }

    generated_name = make_sym_constant(name);
    return generated_name;
}

/* kjh (B14) end */

/* ====================================================================

                        Chunk Instantiation

   This the main chunking routine.  It takes an instantiation, and a
   flag "allow_variablization"--if FALSE, the chunk will not be
   variablized.  (If TRUE, it may still not be variablized, due to
   chunk-free-problem-spaces, ^quiescence t, etc.)
==================================================================== */

void chunk_instantiation(instantiation * inst, bool allow_variablization)
{
    goal_stack_level grounds_level;
    preference *results, *pref;
    instantiation *chunk_inst;
    Symbol *prod_name;
    byte prod_type;
    bool print_name, print_prod;

    condition *lhs_top, *lhs_bottom;
    not *nots;
    chunk_cond *top_cc, *bottom_cc;
    explain_chunk_str temp_explain_chunk;

#if !defined(THIN_JUSTIFICATIONS) || defined(MAKE_PRODUCTION_FOR_THIN_JUSTS)
    production *prod;
    action *rhs;
#endif

#ifndef THIN_JUSTIFICATIONS
    byte rete_addition_result;
#endif

#if !defined(NO_TIMING_STUFF) && defined(DETAILED_TIMING_STATS)
    struct timeval saved_start_tv;
#endif

    /* These two lines quell compiler warnings */
    temp_explain_chunk.conds = NULL;
    temp_explain_chunk.actions = NULL;

    /* --- if it only matched an attribute impasse, don't chunk --- */
    if (!inst->match_goal)
        return;

#ifdef WATCH_PREFS_GENERATED
    print("\nPreferences Generated for instantiation at top of ci\n");
    print_instantiation_with_wmes(inst, TIMETAG_WME_TRACE);
    print("\n   Instantiation Match Goal Level = %d", inst->match_goal_level);
    print("\n---------------------------------------------\n");
    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
        watchful_print_preference(pref);
        print("Reference count = %d\n", pref->reference_count);
    }
    print("\n");
#endif

    /* --- if no preference is above the match goal level, exit --- */
    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
        if (pref->id->id.level < inst->match_goal_level)
            break;
    }
    if (!pref)
        return;

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    start_timer(&saved_start_tv);
#endif
#endif

/* REW: begin 09.15.96 */

    /*

       in OPERAND, we only wanna built a chunk for the top goal; i.e. no
       intermediate chunks are build.  we're essentially creating
       "top-down chunking"; just the opposite of the "bottom-up chunking"
       that is available through a soar prompt-level comand.

       (why do we do this??  i don't remember...)

       we accomplish this by forcing only justifications to be built for
       subgoal chunks.  and when we're about to build the top level
       result, we make a chunk.  a cheat, but it appears to work.  of
       course, this only kicks in if learning is turned on.

       i get the behavior i want by twiddling the allow_variablization
       flag.  i set it to FALSE for intermediate results.  then for the
       last (top-most) result, i set it to whatever it was when the
       function was called.

       by the way, i need the lower level justificiations because they
       support the upper level justifications; i.e. a justification at
       level i supports the justification at level i-1.  i'm talkin' outa
       my butt here since i don't know that for a fact.  it's just that
       i tried building only the top level chunk and ignored the
       intermediate justifications and the system complained.  my
       explanation seemed reasonable, at the moment.

     */

#ifndef DONT_ALLOW_VARIABLIZATION

#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode) == TRUE) {
#endif

        if (current_agent(sysparams)[LEARNING_ON_SYSPARAM] == TRUE) {
            if (pref->id->id.level < (inst->match_goal_level - 1)) {
                allow_variablization = FALSE;
                inst->okay_to_variablize = FALSE;

                if (current_agent(soar_verbose_flag) == TRUE)
                    print("\n   in chunk_instantiation: making justification only");
            }

            else {
                allow_variablization = (bool) current_agent(sysparams)[LEARNING_ON_SYSPARAM];
                inst->okay_to_variablize = (byte) current_agent(sysparams)[LEARNING_ON_SYSPARAM];

                if (current_agent(soar_verbose_flag) == TRUE)
                    print("\n   in chunk_instantiation: resetting allow_variablization to %s",
                          ((allow_variablization) ? "TRUE" : "FALSE"));
            }
        }

#ifndef SOAR_8_ONLY
    }
#endif

#else                           /* DONT_ALLOW_VARIABLIZATION */

    inst->okay_to_variablize = FALSE;

#endif                          /* DONT_ALLOW_VARIABLIZATION */

/* REW: end   09.15.96 */

    results = get_results_for_instantiation(inst);

    if (!results)
        goto chunking_done;

#ifdef OPTIMIZE_TOP_LEVEL_RESULTS
    {
        preference *the_temp_pref;
        bool optimize = TRUE;
        Symbol *top_level_goal = NIL;

        for (the_temp_pref = inst->preferences_generated; the_temp_pref != NIL;
             the_temp_pref = the_temp_pref->inst_next) {
            if (the_temp_pref->id->id.level != 1) {
                optimize = FALSE;
            } else if (!top_level_goal && the_temp_pref->id->id.isa_goal) {
                top_level_goal = the_temp_pref->id;
            }

        }

        if (optimize) {

            /*
               print( "Optimizing this result.\n" );
               printf( "Optimizing this result.\n" );
             */

            allocate_with_pool(&current_agent(instantiation_pool), &chunk_inst);
            chunk_inst->prod = NIL;
            chunk_inst->top_of_instantiated_conditions = NIL;
            chunk_inst->bottom_of_instantiated_conditions = NIL;
            chunk_inst->nots = NIL;
            chunk_inst->GDS_evaluated_already = FALSE;
            chunk_inst->okay_to_variablize = FALSE;

            make_clones_of_results(results, chunk_inst);
            fill_in_new_instantiation_stuff(chunk_inst, FALSE);

            for (the_temp_pref = chunk_inst->preferences_generated;
                 the_temp_pref != NIL; the_temp_pref = the_temp_pref->inst_next) {
                /*
                   if ( the_temp_pref->o_supported == FALSE ) {
                   printf( "Warning: Optimizing a result with I-supported results.\n" );
                   }
                 */
                the_temp_pref->o_supported = TRUE;
                the_temp_pref->match_goal_level = 1;
                the_temp_pref->match_goal = top_level_goal;

            }
            /*
               printf( "Inst = %p\n", inst );
               printf( "Setting Chunk_inst (%p) match goal to 1\n", chunk_inst );
             */
            chunk_inst->match_goal = top_level_goal;
            chunk_inst->match_goal_level = 1;
            chunk_inst->in_ms = TRUE;
            chunk_inst->next = current_agent(newly_created_instantiations);
            current_agent(newly_created_instantiations) = chunk_inst;

            goto chunking_done;

        }
    }
#endif

#ifdef WATCH_RESULTS
    {
        preference *the_temp_pref;

        print("\nResults for instantiaition:\n");
        print_instantiation_with_wmes(inst, TIMETAG_WME_TRACE);
        print("\n---------------------------\n");
        for (the_temp_pref = results; the_temp_pref != NIL; the_temp_pref = the_temp_pref->next_result) {
            watchful_print_preference(the_temp_pref);
            print("References for id at top of chunk_inst = %d\n", the_temp_pref->id->common.reference_count);

        }
        print("\n");
    }
#endif

#ifdef WATCH_INST_CONDS
    print_with_symbols("\nIn chunk_instantiation.\nInst which will be chunked is: %y -- conditions:\n",
                       inst->prod->name);
    print("Address %p\n", inst);
    print_condition_list(inst->top_of_instantiated_conditions, 2, TRUE);
    if (inst->top_of_instantiated_conditions == NIL) {
        print("There are no pointers here...\n");
    }
#endif

    /* --- update flags on goal stack for bottom-up chunking --- */
    {
        Symbol *g;
        for (g = inst->match_goal->id.higher_goal; g && g->id.allow_bottom_up_chunks; g = g->id.higher_goal)
            g->id.allow_bottom_up_chunks = FALSE;
    }

    grounds_level = (short) (inst->match_goal_level - 1);

    current_agent(backtrace_number)++;
    if (current_agent(backtrace_number) == 0)
        current_agent(backtrace_number) = 1;
    current_agent(grounds_tc)++;
    if (current_agent(grounds_tc) == 0)
        current_agent(grounds_tc) = 1;
    current_agent(potentials_tc)++;
    if (current_agent(potentials_tc) == 0)
        current_agent(potentials_tc) = 1;
    current_agent(locals_tc)++;
    if (current_agent(locals_tc) == 0)
        current_agent(locals_tc) = 1;
    current_agent(grounds) = NIL;
    current_agent(positive_potentials) = NIL;
    current_agent(locals) = NIL;
    current_agent(instantiations_with_nots) = NIL;

#ifndef DONT_ALLOW_VARIABLIZATION

    if (allow_variablization && (!current_agent(sysparams)[LEARNING_ALL_GOALS_SYSPARAM]))
        allow_variablization = inst->match_goal->id.allow_bottom_up_chunks;

#endif                          /* DONT_ALLOW_VARIABLIZATION */

    /* DJP : Need to initialize chunk_free_flag to be FALSE, as default before
       looking for problem spaces and setting the chunk_free_flag below  */

    current_agent(chunk_free_flag) = FALSE;
    /* DJP : Noticed this also isn't set if no ps_name */
    current_agent(chunky_flag) = FALSE;

#ifndef DONT_ALLOW_VARIABLIZATION

    /* --- check whether ps name is in chunk_free_problem_spaces --- */
    if (allow_variablization) {

        /* KJC new implementation of learn cmd:  old SPECIFY ==> ONLY,
         * old ON ==> EXCEPT,  now ON is just ON always
         * checking if state is chunky or chunk-free...
         */
        if (current_agent(sysparams)[LEARNING_EXCEPT_SYSPARAM]) {
            if (member_of_list(inst->match_goal, current_agent(chunk_free_problem_spaces))) {
                allow_variablization = FALSE;
                current_agent(chunk_free_flag) = TRUE;
            }
        } else if (current_agent(sysparams)[LEARNING_ONLY_SYSPARAM]) {
            if (member_of_list(inst->match_goal, current_agent(chunky_problem_spaces))) {
                allow_variablization = TRUE;
                current_agent(chunky_flag) = TRUE;
            } else {
                allow_variablization = FALSE;
                current_agent(chunky_flag) = FALSE;
            }
        }
    }
    /* end KJC mods */
    current_agent(variablize_this_chunk) = allow_variablization;

#else                           /* DONT_ALLOW_VARIABLIZATION */

    current_agent(variablize_this_chunk) = FALSE;
#endif                          /* DONT_ALLOW_VARIABLIZATION */

#ifndef THIN_JUSTIFICATIONS

    /* Start a new structure for this potential chunk */

    if (current_agent(sysparams)[EXPLAIN_SYSPARAM]) {
        temp_explain_chunk.conds = NULL;
        temp_explain_chunk.actions = NULL;
        temp_explain_chunk.backtrace = NULL;
        temp_explain_chunk.name[0] = '\0';
        temp_explain_chunk.all_grounds = NIL;
        temp_explain_chunk.next_chunk = NULL;
        reset_backtrace_list();
    }
#endif                          /* !THIN_JUSTIFICATIONS */

#ifndef NO_BACKTRACING
#ifndef DONT_CALC_GDS_OR_BT

    /* --- backtrace through the instantiation that produced each result --- */
    for (pref = results; pref != NIL; pref = pref->next_result) {

#ifndef TRACE_CONTEXT_DECISIONS_ONLY
        if (current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM]) {
            print_string("\nFor result preference ");
            print_preference(pref);
            print_string(" ");
        }
#endif

#ifdef NO_TOP_JUST
        if (pref->inst)
            backtrace_through_instantiation(pref->inst, grounds_level, NULL, 0);

#else
        backtrace_through_instantiation(pref->inst, grounds_level, NULL, 0);

#endif

    }

#else
    add_named_superstate_attribute_to_grounds(inst, "superstate");
#endif

#endif                          /* NO_BACKTRACING */

    current_agent(quiescence_t_flag) = FALSE;

    for (;;) {
        trace_locals(grounds_level);
        trace_grounded_potentials();
        if (!trace_ungrounded_potentials(grounds_level))
            break;
    }
    free_list(current_agent(positive_potentials));

    /* --- backtracing done; collect the grounds into the chunk --- */
    {
        tc_number tc_for_grounds;
        tc_for_grounds = get_new_tc_number();
        build_chunk_conds_for_grounds_and_add_negateds(&top_cc, &bottom_cc, tc_for_grounds);
        nots = get_nots_for_instantiated_conditions(current_agent(instantiations_with_nots), tc_for_grounds);
    }

    /* --- get symbol for name of new chunk or justification --- */

#ifndef DONT_ALLOW_VARIABLIZATION

    if (current_agent(variablize_this_chunk)) {
        /* kjh (B14) begin */
        current_agent(chunks_this_d_cycle)++;
        prod_name = generate_chunk_name_sym_constant(inst);
        /* kjh (B14) end */

        prod_type = CHUNK_PRODUCTION_TYPE;
        print_name = (bool) current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM];
        print_prod = (bool) current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM];

    } else {

#endif                          /* DONT_ALLOW_VARIABLIZATION */

#ifdef THIN_JUSTIFICATIONS
        prod_name = generate_new_sym_constant("temp-justification-", &current_agent(justification_count));
#else
        prod_name = generate_new_sym_constant("justification-", &current_agent(justification_count));
#endif                          /* THIN_JUSTIFICATIONS */

        prod_type = JUSTIFICATION_PRODUCTION_TYPE;
        print_name = (bool) current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM];
        print_prod = (bool) current_agent(sysparams)[TRACE_JUSTIFICATIONS_SYSPARAM];

#ifndef DONT_ALLOW_VARIABLIZATION
    }
#endif                          /* DONT_ALLOW_VARIABLIZATION */

    /* AGR 617/634 begin */
    if (print_name) {
        if (get_printer_output_column() != 1)
            print("\n");
        print_with_symbols("Building %y", prod_name);

#if defined(THIN_JUSTIFICATIONS) && !defined(MAKE_PRODUCTION_FOR_THIN_JUSTS)
        print("...thin justification, no prod");
#endif

    }
    /* AGR 617/634 end */

    /* --- if there aren't any grounds, exit --- */
    if (!top_cc) {
        if (current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM])
            print_string(" Warning: chunk has no grounds, ignoring it.");
        goto chunking_done;
    }
#ifndef DONT_ALLOW_VARIABLIZATION

    /* MVP 6-8-94 */
    if (current_agent(chunks_this_d_cycle) > (unsigned long) current_agent(sysparams)[MAX_CHUNKS_SYSPARAM]) {
        if (current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM])
            print("\nWarning: reached max-chunks! Halting system.");
        current_agent(max_chunks_reached) = TRUE;
        goto chunking_done;
    }
#endif                          /* DONT_ALLOW_VARIABLIZATION */

    /* --- variablize it --- */
    lhs_top = top_cc->variablized_cond;
    lhs_bottom = bottom_cc->variablized_cond;
    reset_variable_generator(lhs_top, NIL);
    current_agent(variablization_tc) = get_new_tc_number();
    variablize_condition_list(lhs_top);
    variablize_nots_and_insert_into_conditions(nots, lhs_top);
#if !defined(THIN_JUSTIFICATIONS) || defined(MAKE_PRODUCTION_FOR_THIN_JUSTS)
    rhs = copy_and_variablize_result_list(results);
#endif

    /* --- add goal/impasse tests to it --- */
    add_goal_or_impasse_tests(top_cc);

    /* --- reorder lhs and make the production --- */
#if !defined(THIN_JUSTIFICATIONS) || defined(MAKE_PRODUCTION_FOR_THIN_JUSTS)

    prod = make_production(prod_type, prod_name, &lhs_top, &lhs_bottom, &rhs, FALSE);

    if (!prod) {
        print("\nUnable to reorder this chunk:\n  ");
        print_condition_list(lhs_top, 2, FALSE);
        print("\n  -->\n   ");
        print_action_list(rhs, 3, FALSE);
        print("\n\n(Ignoring this chunk.  Weird things could happen from now on...)\n");
        goto chunking_done;     /* this leaks memory but who cares */
    }
#endif                          /* !THIN_JUSTIFICATIONS || MAKE_PRODUCTION_FOR_THIN_JUSTS */

    {
        condition *inst_lhs_top, *inst_lhs_bottom;

        reorder_instantiated_conditions(top_cc, &inst_lhs_top, &inst_lhs_bottom);

        /* Record the list of grounds in the order they will appear in the chunk */
        if (current_agent(sysparams)[EXPLAIN_SYSPARAM])
            temp_explain_chunk.all_grounds = inst_lhs_top;      /* Not a copy yet */

        allocate_with_pool(&current_agent(instantiation_pool), &chunk_inst);

#if !defined(THIN_JUSTIFICATIONS) || defined(MAKE_PRODUCTION_FOR_THIN_JUSTS)
        chunk_inst->prod = prod;
#else
        chunk_inst->prod = NIL;
#endif                          /* !THIN_JUSTIFICATIONS || MAKE_PRODUCTION_FOR_THIN_JUSTS */

        chunk_inst->top_of_instantiated_conditions = inst_lhs_top;

#ifdef WATCH_SSCI_CONDS

        print("\nCreating temp-justification: ");

        if (chunk_inst->prod)
            print_with_symbols("%y...conditions:\n", chunk_inst->prod->name);
        else
            print("(nil) ... conditions:\n");

        print("Address %p\n", chunk_inst);
        print_condition_list(chunk_inst->top_of_instantiated_conditions, 2, TRUE);
        if (chunk_inst->top_of_instantiated_conditions == NIL) {
            print("There are no pointers here...\n");
        }
#endif

        chunk_inst->bottom_of_instantiated_conditions = inst_lhs_bottom;
        chunk_inst->nots = nots;
        chunk_inst->GDS_evaluated_already = FALSE;      /* REW:  09.15.96 */

        /* If:
           - you don't want to variablize this chunk, and
           - the reason is ONLY that it's chunk free, and
           - NOT that it's also quiescence, then
           it's okay to variablize through this instantiation later.
         */

        /* AGR MVL1 begin */
        if (!current_agent(sysparams)[LEARNING_ONLY_SYSPARAM]) {
            if ((!current_agent(variablize_this_chunk))
                && (current_agent(chunk_free_flag))
                && (!current_agent(quiescence_t_flag)))
                chunk_inst->okay_to_variablize = TRUE;
            else
                chunk_inst->okay_to_variablize = current_agent(variablize_this_chunk);
        } else {
            if ((!current_agent(variablize_this_chunk))
                && (!current_agent(chunky_flag))
                && (!current_agent(quiescence_t_flag)))
                chunk_inst->okay_to_variablize = TRUE;
            else
                chunk_inst->okay_to_variablize = current_agent(variablize_this_chunk);
        }
        /* AGR MVL1 end */

        chunk_inst->in_ms = TRUE;       /* set TRUE for now, we'll find out later... */

        /* Increments symbol reference counts */
        make_clones_of_results(results, chunk_inst);

        fill_in_new_instantiation_stuff(chunk_inst, TRUE);

    }                           /* matches { condition *inst_lhs_top, *inst_lhs_bottom ...  */

#ifdef WARN_IF_RESULT_IS_I_SUPPORTED
    /* 
     * SW 110499
     *
     * When we use the Soar-lite feature of not adding justifications
     * to the rete, we bump up against a potential problem:
     * Without a production in the rete, there is no way to know
     * when an instantiation should be retracted (I think!).  As a result,
     * we can end up with a huge memory leak.  We get around this by assuming
     * that subgoal results are o-supported.  If this is the case, we
     * can safely remove the instantiations once the preferences have been
     * asserted.  However, this could influence the way certain Soar programs 
     * run.  As a result, it may or may not be desireable to actually force
     * subgoal results to be o-supported without telling anyone...
     */

    for (pref = chunk_inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
        if (pref->o_supported == FALSE) {
            print("Warning: Result is not natively o-supported. ");
#ifdef THIN_JUSTIFICATIONS
#ifdef ALLOW_I_SUPPORTED_SUBGOAL_RESULTS_WITH_THIN_JUSTS
            print("This may produce memory leaks.\n");
#else
            print("O-support will be forced.\n");
#endif
#endif                          /* THIN_JUSTIFICATIONS */

            watchful_print_preference(pref);
        }
    }
#endif                          /* WARN_IF_RESULT_IS_I_SUPPORTED */

#if defined(DONT_CALC_GDS_OR_BT) || (defined(THIN_JUSTIFICATIONS) && !defined(ALLOW_I_SUPPORTED_SUBGOAL_RESULTS_WITH_THIN_JUSTS))

    /* DJP Now force all results to be o-supported */
    /* Have to wait till the clone preferences are created */
    for (pref = chunk_inst->preferences_generated; pref != NIL; pref = pref->inst_next)

        pref->o_supported = TRUE;       /* These preferences are all results */

#endif

#ifndef THIN_JUSTIFICATIONS

    /* RBD 4/6/95 Need to copy cond's and actions for the production here,
       otherwise some of the variables might get deallocated by the call to
       add_production_to_rete() when it throws away chunk variable names. */
    if (current_agent(sysparams)[EXPLAIN_SYSPARAM]) {
        condition *new_top, *new_bottom;
        copy_condition_list(lhs_top, &new_top, &new_bottom);
        temp_explain_chunk.conds = new_top;
        temp_explain_chunk.actions = copy_and_variablize_result_list(results);
    }

    rete_addition_result = add_production_to_rete(prod, lhs_top, chunk_inst, print_name);

#ifdef WATCH_CHUNK_INST
    if (rete_addition_result == REFRACTED_INST_MATCHED)
        print("\nAdded chunk/justification to rete -- result: MATCHED\n");
    else if (rete_addition_result == REFRACTED_INST_DID_NOT_MATCH)
        print("\nAdded chunk/justification to rete -- result: DID NOT MATCH\n");
    else
        print("\nAdded chunk/justification to rete -- result: DUPLICATE\n");
#endif

    /* If didn't immediately excise the chunk from the rete net   
       then record the temporary structure in the list of explained chunks. */

    if (current_agent(sysparams)[EXPLAIN_SYSPARAM]) {
        if ((rete_addition_result != DUPLICATE_PRODUCTION) &&
            ((prod_type != JUSTIFICATION_PRODUCTION_TYPE) || (rete_addition_result != REFRACTED_INST_DID_NOT_MATCH))) {
            strncpy(temp_explain_chunk.name, prod_name->sc.name, PROD_NAME_SIZE);
            temp_explain_chunk.name[PROD_NAME_SIZE - 1] = 0;
            explain_add_temp_to_chunk_list(&temp_explain_chunk);
        } else {
            /* RBD 4/6/95 if excised the chunk, discard previously-copied stuff */
            deallocate_condition_list(temp_explain_chunk.conds);
            deallocate_action_list(temp_explain_chunk.actions);
        }
    }
    /* --- deallocate chunks conds and variablized conditions --- */
    deallocate_condition_list(lhs_top);
    {
        chunk_cond *cc;
        while (top_cc) {
            cc = top_cc;
            top_cc = cc->next;
            free_with_pool(&current_agent(chunk_cond_pool), cc);
        }
    }

    if (print_prod && (rete_addition_result != DUPLICATE_PRODUCTION)) {
        print_string("\n");
        print_production(prod, FALSE);
    }

    if (rete_addition_result == DUPLICATE_PRODUCTION) {
        excise_production(prod, FALSE);
    } else if ((prod_type == JUSTIFICATION_PRODUCTION_TYPE) && (rete_addition_result == REFRACTED_INST_DID_NOT_MATCH)) {
        excise_production(prod, FALSE);
    }

    if (rete_addition_result != REFRACTED_INST_MATCHED) {
        /* --- it didn't match, or it was a duplicate production --- */
        /* --- tell the firer it didn't match, so it'll only assert the
           o-supported preferences --- */
        chunk_inst->in_ms = FALSE;
    }
#else                           /* THIN_JUSTIFICATIONS */

    /* I think this will take care of the 102099 Memory Leak */
    /* --- deallocate chunks conds and variablized conditions --- */
    deallocate_condition_list(lhs_top);
#ifdef DONT_CALC_GDS_OR_BT
    /* SW NOTE See the note in add_named_superstate_attribute_to_grounds */
    deallocate_condition_list(top_cc->cond);
#endif
    {
        chunk_cond *cc;
        while (top_cc) {
            cc = top_cc;
            top_cc = cc->next;
            free_with_pool(&current_agent(chunk_cond_pool), cc);
        }
    }

    if (print_prod) {
        print_string("\n -- Justification (not added to rete) -- \n");
        {
            preference *the_temp_pref;

            print("\nResults for instantiaition:\n");
            print_instantiation_with_wmes(chunk_inst, FULL_WME_TRACE);
            print("\n---------------------------\n");
            for (the_temp_pref = chunk_inst->preferences_generated;
                 the_temp_pref != NIL; the_temp_pref = the_temp_pref->inst_next) {
                watchful_print_preference(the_temp_pref);
            }
            print("\n");
        }

    }
#endif                          /* !THIN_JUSTIFICATIONS */

    /* SW 090799 These two lines of code are necessary for the GDS... */
    /* --- assert the preferences --- */
    chunk_inst->next = current_agent(newly_created_instantiations);
    current_agent(newly_created_instantiations) = chunk_inst;

    /* MVP 6-8-94 */
    if (!current_agent(max_chunks_reached))
#ifndef THIN_JUSTIFICATIONS
        chunk_instantiation(chunk_inst, current_agent(variablize_this_chunk));

#else                           /* THIN_JUSTIFICATIONS */

        if (chunk_inst->isa_ssci_inst != TRUE) {
            chunk_inst->isa_ssci_inst = TRUE;
        }
#ifdef WATCH_SSCI_INSTS

    print("\nCreating an SSCI instantiation: ");
    if (chunk_inst->prod)
        print_with_symbols("%y\n", chunk_inst->prod->name);
    else
        print("(nil)\n");

#endif                          /* WATCH_SSCI_INSTS */

#ifdef SINGLE_THIN_JUSTIFICATION
    second_stage_chunk_instantiation(chunk_inst);

#else                           /* !SINGLE_THIN_JUSTIFICATION --> THIN_JUSTIFICATIONS */

    /*
     * 102699 
     * we can use a recursive call to chunk_inst as opposed to ssci
     * to avoid the tr.soar memory leak.  Today, I will try to fix 
     * ssci
     */
    chunk_instantiation(chunk_inst, FALSE);

#endif                          /* SINGLE_THIN_JUSTIFICATION */

#endif                          /* !THIN_JUSTIFICATION */

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    stop_timer(&saved_start_tv, &current_agent(chunking_cpu_time[current_agent(current_phase)]));
#endif
#endif

    return;

  chunking_done:{
    }

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    stop_timer(&saved_start_tv, &current_agent(chunking_cpu_time[current_agent(current_phase)]));
#endif
#endif

}

#ifdef SINGLE_THIN_JUSTIFICATION

void second_stage_chunk_instantiation(instantiation * inst)
{
    goal_stack_level grounds_level;
    preference *results, *pref;
    bool print_name, print_prod;
    condition *lhs_top, *lhs_bottom;
    not *nots;
    chunk_cond *top_cc, *bottom_cc;

    /* --- if it only matched an attribute impasse, don't chunk --- */
    if (!inst->match_goal)
        return;

#ifdef WATCH_PREFS_GENERATED
    print("\nPreferences Generated for instantiation at top of sscii\n");
    print_instantiation_with_wmes(inst, TIMETAG_WME_TRACE);
    print("\n   Instantiation Match Goal Level = %d", inst->match_goal_level);
    print("\n---------------------------------------------\n");
    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
        watchful_print_preference(pref);
        print("Reference count = %d\n", pref->reference_count);
    }
    print("\n");
#endif

    /* --- if no preference is above the match goal level, exit --- */
    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
        if (pref->id->id.level < inst->match_goal_level)
            break;
    }
    if (!pref)
        return;
    inst->okay_to_variablize = FALSE;

    results = get_results_for_instantiation(inst);

#ifdef WATCH_RESULTS
    {
        preference *the_temp_pref;

        print("\nResults for instantiaition:\n");
        print_instantiation_with_wmes(inst, TIMETAG_WME_TRACE);
        print("\n---------------------------\n");
        for (the_temp_pref = results; the_temp_pref != NIL; the_temp_pref = the_temp_pref->next_result) {
            watchful_print_preference(the_temp_pref);

        }
        print("\n");
    }
#endif

    if (!results)
        goto chunking_done;

    /* --- update flags on goal stack for bottom-up chunking --- */
    {
        Symbol *g;
        for (g = inst->match_goal->id.higher_goal; g && g->id.allow_bottom_up_chunks; g = g->id.higher_goal)
            g->id.allow_bottom_up_chunks = FALSE;
    }

    grounds_level = inst->match_goal_level - 1;

    current_agent(backtrace_number)++;
    if (current_agent(backtrace_number) == 0)
        current_agent(backtrace_number) = 1;
    current_agent(grounds_tc)++;
    if (current_agent(grounds_tc) == 0)
        current_agent(grounds_tc) = 1;
    current_agent(potentials_tc)++;
    if (current_agent(potentials_tc) == 0)
        current_agent(potentials_tc) = 1;
    current_agent(locals_tc)++;
    if (current_agent(locals_tc) == 0)
        current_agent(locals_tc) = 1;
    current_agent(grounds) = NIL;
    current_agent(positive_potentials) = NIL;
    current_agent(locals) = NIL;
    current_agent(instantiations_with_nots) = NIL;

    /* DJP : Need to initialize chunk_free_flag to be FALSE, as default before
       looking for problem spaces and setting the chunk_free_flag below  */

    current_agent(chunk_free_flag) = FALSE;
    /* DJP : Noticed this also isn't set if no ps_name */
    current_agent(chunky_flag) = FALSE;

    current_agent(variablize_this_chunk) = FALSE;

#ifndef NO_BACKTRACING
#ifndef DONT_CALC_GDS_OR_BT

    /* --- backtrace through the instantiation that produced each result --- */
    for (pref = results; pref != NIL; pref = pref->next_result) {

#ifndef TRACE_CONTEXT_DECISIONS_ONLY
        if (current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM]) {
            print_string("\nFor result preference ");
            print_preference(pref);
            print_string(" ");
        }
#endif

        if (pref->inst)
            backtrace_through_instantiation(pref->inst, grounds_level, NULL, 0);

    }

#else
    add_named_superstate_attribute_to_grounds(inst, "superstate");
#endif

#endif                          /* NO_BACKTRACING */

    current_agent(quiescence_t_flag) = FALSE;

    while (TRUE) {
        trace_locals(grounds_level);
        trace_grounded_potentials();
        if (!trace_ungrounded_potentials(grounds_level))
            break;
    }
    free_list(current_agent(positive_potentials));

    /* --- backtracing done; collect the grounds into the chunk --- */
    {
        tc_number tc_for_grounds;
        tc_for_grounds = get_new_tc_number();
        build_chunk_conds_for_grounds_and_add_negateds(&top_cc, &bottom_cc, tc_for_grounds);
        nots = get_nots_for_instantiated_conditions(current_agent(instantiations_with_nots), tc_for_grounds);
    }

    /* --- get symbol for name of new chunk or justification --- */
#ifdef MAKE_PRODUCTION_FOR_THIN_JUSTS
    if (inst->prod->type != JUSTIFICATION_PRODUCTION_TYPE) {
        print("Warning (1)\n");
    }
#endif

    print_name = current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM];
    print_prod = current_agent(sysparams)[TRACE_JUSTIFICATIONS_SYSPARAM];

#ifdef WATCH_SSCI_INSTS

    print("Rebuiding ");
    if (inst->prod)
        print_with_symbols("%y\n", inst->prod->name);
    else
        print("an SSCI inst.\n");

#endif

    /* AGR 617/634 begin */
    if (print_name) {
        if (get_printer_output_column() != 1)
            print("\n");
        print("Rebuiding ");
        if (inst->prod)
            print_with_symbols("%y\n", inst->prod->name);
        else
            print("an SSCI inst.\n");

    }
    /* AGR 617/634 end */

    /* --- if there aren't any grounds, exit --- */
    if (!top_cc) {
        if (current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM])
            print_string(" Warning: chunk has no grounds, ignoring it.");

    }

    /* --- variablize it --- */
    lhs_top = top_cc->variablized_cond;
    lhs_bottom = bottom_cc->variablized_cond;
    reset_variable_generator(lhs_top, NIL);
    current_agent(variablization_tc) = get_new_tc_number();
    variablize_condition_list(lhs_top);
    variablize_nots_and_insert_into_conditions(nots, lhs_top);
    /*
     * no need to make a rhs, since there's no justification being
     * built
     */

    /* --- add goal/impasse tests to it --- */
    add_goal_or_impasse_tests(top_cc);

    /* --- reorder lhs and make the production --- */

    {
        condition *inst_lhs_top, *inst_lhs_bottom;

        reorder_instantiated_conditions(top_cc, &inst_lhs_top, &inst_lhs_bottom);
        deallocate_inst_members_to_be_rewritten(inst);

        /* 
         * now fill in the new conditions for this level
         */
        inst->top_of_instantiated_conditions = inst_lhs_top;
        inst->bottom_of_instantiated_conditions = inst_lhs_bottom;
        inst->nots = nots;

#ifdef WATCH_SSCI_CONDS

        print("Rebuiding temp-just: ");
        if (inst->prod)
            print_with_symbols("%y ... conditions:\n", inst->prod->name);
        else
            print(" (nil) ... conditions: \n");
        print("Address %p\n", inst);
        print_condition_list(inst->top_of_instantiated_conditions, 2, TRUE);
        if (inst->top_of_instantiated_conditions == NIL) {
            print("There are no pointers here...\n");
        }
#endif

        re_fill_in_instantiation_stuff_for_modified_lhs(inst, TRUE);

    }                           /* matches { condition *inst_lhs_top, *inst_lhs_bottom ...  */

#ifdef WARN_IF_RESULT_IS_I_SUPPORTED
    /* 
     * SW 110499
     * See the note with the same date in chunk_instantiation
     */
    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
        if (pref->o_supported == FALSE) {
            print("Warning: Result is not natively o-supported. ");

#ifdef ALLOW_I_SUPPORTED_SUBGOAL_RESULTS_WITH_THIN_JUSTS
            print("This may produce memory leaks.\n");
#else
            print("O-support will be forced.\n");
#endif

            watchful_print_preference(pref);
        }
    }
#endif                          /* WARN_IF_RESULT_IS_I_SUPPORTED */

    /* --- deallocate chunks conds and variablized conditions --- */
    deallocate_condition_list(lhs_top);
#ifdef DONT_CALC_GDS_OR_BT
    /* SW NOTE See the note in add_named_superstate_attribute_to_ground  */
    deallocate_condition_list(top_cc->cond);
#endif

    {
        chunk_cond *cc;
        while (top_cc) {
            cc = top_cc;
            top_cc = cc->next;
            free_with_pool(&current_agent(chunk_cond_pool), cc);
        }
    }

    /* finally, recurse... */
    second_stage_chunk_instantiation(inst);

  chunking_done:{
    }

}

void deallocate_inst_members_to_be_rewritten(instantiation * inst)
{

    goal_stack_level level;
    condition *cond;

    level = inst->match_goal_level;

    /* SW 102799 -- memory leak fix
     * We need to do some freeing of old conditions and such.
     * moreover, we need to free up some references to wmes and
     * preferences in the backtracing structures.  That's 
     * what's going on here (I stole this code from fill_in_new_inst_stuff)
     */

    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION) {

#ifdef NO_TOP_LEVEL_REFS
            if (level > 1) {
                wme_remove_ref(cond->bt.wme);
            }
#ifdef DEBUG_NO_TOP_LEVEL_REFS
            else {
                print("NO_TOP_LEVEL_REFS(4): Not removing reference to tt =%lu ref =%lu\n",
                      cond->bt.wme->timetag, cond->bt.wme->reference_count);
            }
#endif

#else
            wme_remove_ref(cond->bt.wme);
#endif
            if (cond->bt.trace) {

#ifdef NO_TOP_JUST
                if (cond->bt.trace->match_goal_level > level)
                    cond->bt.trace = find_clone_for_level(cond->bt.trace, level);
#else

                if (cond->bt.trace->inst->match_goal_level > level)
                    cond->bt.trace = find_clone_for_level(cond->bt.trace, level);
#endif

                /* begin SW 7.7.99 */
#ifdef NO_TOP_LEVEL_REFS
                if ((cond->bt.trace) && (level > 1)) {
                    preference_remove_ref(cond->bt.trace);
                }
#ifdef DEBUG_NO_TOP_LEVEL_REFS
                else {
                    print("NO_TOP_LEVEL_REFS (5): Not removing reference to  rf = %lu\n",
                          cond->bt.trace->reference_count);
                    print("NO_TOP_LEVEL_REFS (5): cond->bt.trace = \n");
                    print_preference(cond->bt.trace);
                }
#endif

#else
                if (cond->bt.trace)
                    preference_remove_ref(cond->bt.trace);
#endif
            }

        }

    {
        preference *pr, *p, *np;
        /*
           printf( "Trying to remove old preferences generated from match goal\n" );
         */
        if (inst->match_goal) {
            for (p = inst->match_goal->id.preferences_from_goal; p != NIL; p = np) {

                np = p->all_of_goal_next;
                for (pr = inst->preferences_generated; pr != NIL; pr = pr->inst_next) {
                    if (p == pr) {
                        /*
                           printf ("Found prefernces in old match goal, removing\n");
                         */
                        remove_from_dll(inst->match_goal->id.preferences_from_goal,
                                        pr, all_of_goal_next, all_of_goal_prev);
                        break;
                    }
                }
            }
        }
    }

    /* SW 102799
     * now those icky backtracing structures are all been ready 
     * to be deallocated (we've decremented reference counts)
     * and we can free the old conditions (which were valid at
     * the previous depth in the goal stack, but probably aren't 
     * here)
     */
    deallocate_condition_list(inst->top_of_instantiated_conditions);
    deallocate_list_of_nots(inst->nots);

}

/* 
 * Currently, this is copied directy from 
 * fill_in_new_instantiation_stuff  This can be incorporated into the orig.
 */
void re_fill_in_instantiation_stuff_for_modified_lhs(instantiation * inst, bool need_to_do_support_calculations)
{
    condition *cond;
    preference *p;
    goal_stack_level level;

#if 0
#ifdef TRY_SSCI_PROD_NIL
    if (inst->prod)
#endif
        production_add_ref(inst->prod);
#endif

    find_match_goal(inst);

    level = inst->match_goal_level;

#ifdef NO_TOP_JUST
    /* Record goal information as we may discard pref->inst later      */
    /* This list of preferences will catch the productions results and */
    /* clone preferences (I believe).                                  */

    for (p = inst->preferences_generated; p != NIL; p = p->inst_next) {
        p->match_goal = inst->match_goal;       /* NIL if from attribute impasse */
        p->match_goal_level = inst->match_goal_level;
    }

    /* This may be a SWBUG.  I removed a '}' here. */
#endif

    /* 

       Note: since we'll never backtrace through instantiations at the top
       level, it might make sense to not increment the reference counts
       on the wmes and preferences here if the instantiation is at the top
       level.  As it stands now, we could gradually accumulate garbage at
       the top level if we have a never-ending sequence of production
       firings at the top level that chain on each other's results.  (E.g.,
       incrementing a counter on every decision cycle.)  I'm leaving it this
       way for now, because if we go to S-Support, we'll (I think) need to
       save these around (maybe). 

     */

    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION) {

            /* begin SW 7.7.99 */

#ifdef NO_TOP_LEVEL_REFS
            if (level > 1) {
                wme_add_ref(cond->bt.wme);
            }
#ifdef DEBUG_NO_TOP_LEVEL_REFS
            else {
                print("NO_TOP_LEVEL_REFS (1): Not adding reference to tt =%lu  ref =%lu\n",
                      cond->bt.wme->timetag, cond->bt.wme->reference_count);
            }
#endif

#else
            wme_add_ref(cond->bt.wme);
#endif

            /* --- if trace is for a lower level, find one for this level --- */
            if (cond->bt.trace) {

#ifdef NO_TOP_JUST
                if (cond->bt.trace->match_goal_level > level)
                    cond->bt.trace = find_clone_for_level(cond->bt.trace, level);
#else

                if (cond->bt.trace->inst->match_goal_level > level)
                    cond->bt.trace = find_clone_for_level(cond->bt.trace, level);
#endif

                /* begin SW 7.7.99 */
#ifdef NO_TOP_LEVEL_REFS
                if ((cond->bt.trace) && (level > 1)) {
                    preference_add_ref(cond->bt.trace);
                }
#ifdef DEBUG_NO_TOP_LEVEL_REFS
                else {
                    print("NO_TOP_LEVEL_REFS (2): Not adding reference to  rf = %lu\n",
                          cond->bt.trace->reference_count);
                    print("NO_TOP_LEVEL_REFS (2): cond->bt.trace = \n");
                    print_preference(cond->bt.trace);
                }
#endif

#else
                if (cond->bt.trace)
                    preference_add_ref(cond->bt.trace);
#endif
            }

        }

    /* endif SW 7.7.99 */

    if (inst->match_goal) {
        for (p = inst->preferences_generated; p != NIL; p = p->inst_next) {
            insert_at_head_of_dll(inst->match_goal->id.preferences_from_goal, p, all_of_goal_next, all_of_goal_prev);
            p->on_goal_list = TRUE;
        }
    }

    inst->backtrace_number = 0;

    if (current_agent(o_support_calculation_type) == 0) {
        /* --- do calc's the normal Soar 6 way --- */
        if (need_to_do_support_calculations)
            calculate_support_for_instantiation_preferences(inst);
    } else if (current_agent(o_support_calculation_type) == 1) {
        if (need_to_do_support_calculations)
            calculate_support_for_instantiation_preferences(inst);
        /* --- do calc's both ways, warn on differences --- */
        if ((inst->prod->declared_support != DECLARED_O_SUPPORT) &&
            (inst->prod->declared_support != DECLARED_I_SUPPORT)) {
            /* --- At this point, we've done them the normal way.  To look for
               differences, save o-support flags on a list, then do Doug's
               calculations, then compare and restore saved flags. --- */
            list *saved_flags;
            preference *pref;
            bool difference_found;
            saved_flags = NIL;
            for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
                push((pref->o_supported ? pref : NIL), saved_flags);
            saved_flags = destructively_reverse_list(saved_flags);
            dougs_calculate_support_for_instantiation_preferences(inst);
            difference_found = FALSE;
            for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
                cons *c;
                bool b;
                c = saved_flags;
                saved_flags = c->rest;
                b = (c->first ? TRUE : FALSE);
                free_cons(c);
                if (pref->o_supported != b)
                    difference_found = TRUE;
                pref->o_supported = b;
            }
            if (difference_found) {
                print_with_symbols("\n*** O-support difference found in production %y", inst->prod->name);
            }
        }
    } else {
        /* --- do calc's Doug's way --- */
        if ((inst->prod->declared_support != DECLARED_O_SUPPORT) &&
            (inst->prod->declared_support != DECLARED_I_SUPPORT)) {
            dougs_calculate_support_for_instantiation_preferences(inst);
        }
    }
}

#endif                          /* SINGLE_THIN_JUSTIFICATION */

/* --------------------------------------------------------------------

                        Chunker Initialization

   Init_chunker() is called at startup time to do initialization here.
-------------------------------------------------------------------- */

void init_chunker(void)
{
    init_memory_pool(&current_agent(chunk_cond_pool), sizeof(chunk_cond), "chunk condition");
    init_chunk_cond_set(&current_agent(negated_set));
}

#ifdef DONT_CALC_GDS_OR_BT

/* DJP */
/* We're going to create a dummy justification with just one condition.   */
/* The condition will be the named attribute (usually the problem-space)  */
/* passed to this routine.  This function finds the WME in the superstate */
/* and then builds a condition to match it, as if that condition had come */
/* through backtracing for a chunk.                                       */
/* Yes, it's really ugly.                                                 */
/* SW NOTE
 * 
 * This function works by finding the superstate attribute and adding
 * it to the grounds of the new instantiation (justification/chunk) 
 * By doing this, it avoids backtracing altogether, however in this
 * routine, unlike within backtrace_through_instantiation, the grounds
 * consisits of conditions (actually only one) that are fabricated on
 * the spot, as opposed to conditions from other real instantiations
 * Therefore, in a normal, backtracing version the grounds are just pointers
 * to conditions that some other inst owns, in this case no one else owns
 * the conds so we need to do some extra trickery to avoid a memory leak.
 * this is done in chunk_instantiation (and second_stage_chunk_instantiation)
 */
static void add_named_superstate_attribute_to_grounds(instantiation * inst, char *name)
{
    Symbol *target;
    wme *ps;
    slot *the_slot;
    condition *ps_cond;

    /* Find the target WME */
    target = find_sym_constant(name);

    if (!target)
        target = current_agent(superstate_symbol);

    the_slot = find_slot(inst->match_goal->id.higher_goal, target);

    if (the_slot) {
        ps = the_slot->wmes;
    } else {
        ps = find_impasse_wme(inst->match_goal->id.higher_goal, target);
    }

    if (ps == NIL) {
        print("\nNo Chunks Hack: Whoops, couldn't find %s in superstate.\n", name);
        return;
    }

    /* Now try to build a condition for this WME */

    /* This will be de-alloced when free the grounds list */
    allocate_with_pool(&current_agent(condition_pool), &ps_cond);

    ps_cond->prev = NIL;
    ps_cond->next = NIL;
    ps_cond->type = POSITIVE_CONDITION;

    ps_cond->data.tests.id_test = make_equality_test(ps->id);
    ps_cond->data.tests.attr_test = make_equality_test(ps->attr);
    ps_cond->data.tests.value_test = make_equality_test(ps->value);
    ps_cond->test_for_acceptable_preference = ps->acceptable;

    ps_cond->bt.wme = ps;
    ps_cond->bt.level = inst->match_goal_level - 1;     /* Set to superstate level */

    ps_cond->bt.trace = NIL;    /* Maybe this needs to be a preference for the PS slot ? */
    ps_cond->bt.prohibits = NIL;

    push((ps_cond), current_agent(grounds));    /* Add it to the grounds list */

}

#endif
/* 031799 SW End */
