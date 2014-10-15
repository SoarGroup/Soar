#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  chunk.cpp
 *
 * =======================================================================
 *  Supports the learning mechanism in Soar.  Learning can be set
 *  on | off | only | except (for other choices see soarCommands.c: learn).
 *  If set to "only" | "except" users must specify rhs functions in
 *  productions: dont-learn | force-learn.   See rhsfun.c
 * =======================================================================
 */


/* ====================================================================

                          Chunking Routines

   ==================================================================== */

#include <stdlib.h>
#include <cstring>
#include <ctype.h>

#include "kernel.h"
#include "chunk.h"
#include "symtab.h"
#include "wmem.h"
#include "agent.h"
#include "instantiations.h"
#include "production.h"
#include "rhs.h"
#include "print.h"
#include "init_soar.h"
#include "prefmem.h"
#include "decide.h"
#include "explain.h"
#include "backtrace.h"
#include "recmem.h"
#include "rete.h"
#include "xml.h"
#include "soar_TraceNames.h"
#include "soar_instance.h"
#include "wma.h"
#include "test.h"
#include "debug.h"
#include "variablization_manager.h"


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
void add_results_for_id(agent* thisAgent, Symbol* id);

inline void add_results_if_needed(agent* thisAgent, Symbol* sym)
{
    if ((sym)->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        if (((sym)->id->level >= thisAgent->results_match_goal_level) &&
                ((sym)->id->tc_num != thisAgent->results_tc_number))
        {
            add_results_for_id(thisAgent, sym);
        }
}

extern void add_pref_to_results(agent* thisAgent, preference* pref)
{
    preference* p;

    /* --- if an equivalent pref is already a result, don't add this one --- */
    for (p = thisAgent->results; p != NIL; p = p->next_result)
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
    if (pref->inst->match_goal_level != thisAgent->results_match_goal_level)
    {
        for (p = pref->next_clone; p != NIL; p = p->next_clone)
            if (p->inst->match_goal_level == thisAgent->results_match_goal_level)
            {
                break;
            }
        if (!p)
            for (p = pref->prev_clone; p != NIL; p = p->prev_clone)
                if (p->inst->match_goal_level == thisAgent->results_match_goal_level)
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
    pref->next_result = thisAgent->results;
    thisAgent->results = pref;

    /* --- follow transitive closure through value, referent links --- */
    add_results_if_needed(thisAgent, pref->value);
    if (preference_is_binary(pref->type))
    {
        add_results_if_needed(thisAgent, pref->referent);
    }
}

void add_results_for_id(agent* thisAgent, Symbol* id)
{
    slot* s;
    preference* pref;
    wme* w;

    id->tc_num = thisAgent->results_tc_number;

    /* --- scan through all preferences and wmes for all slots for this id --- */
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        add_results_if_needed(thisAgent, w->value);
    }
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
        {
            add_pref_to_results(thisAgent, pref);
        }
        for (w = s->wmes; w != NIL; w = w->next)
        {
            add_results_if_needed(thisAgent, w->value);
        }
    } /* end of for slots loop */
    /* --- now scan through extra prefs and look for any with this id --- */
    for (pref = thisAgent->extra_result_prefs_from_instantiation; pref != NIL;
            pref = pref->inst_next)
    {
        if (pref->id == id)
        {
            add_pref_to_results(thisAgent, pref);
        }
    }
}

preference* get_results_for_instantiation(agent* thisAgent, instantiation* inst)
{
    preference* pref;

    thisAgent->results = NIL;
    thisAgent->results_match_goal_level = inst->match_goal_level;
    thisAgent->results_tc_number = get_new_tc_number(thisAgent);
    thisAgent->extra_result_prefs_from_instantiation = inst->preferences_generated;
    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
        if ((pref->id->id->level < thisAgent->results_match_goal_level) &&
                (pref->id->tc_num != thisAgent->results_tc_number))
        {
            add_pref_to_results(thisAgent, pref);
        }
    return thisAgent->results;
}

action* copy_action_list(agent* thisAgent, action* actions)
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

void init_chunk_cond_set(chunk_cond_set* set)
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

chunk_cond* make_chunk_cond_for_negated_condition(agent* thisAgent, condition* cond)
{
    chunk_cond* cc;
    uint32_t remainder, hv;

    allocate_with_pool(thisAgent, &thisAgent->chunk_cond_pool, &cc);
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

bool add_to_chunk_cond_set(agent* thisAgent, chunk_cond_set* set, chunk_cond* new_cc)
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
        free_with_pool(&thisAgent->chunk_cond_pool, new_cc);
        return false;
    }
    /* --- add new_cc to the table --- */
    insert_at_head_of_dll(set->all, new_cc, next, prev);
    insert_at_head_of_dll(set->table[new_cc->compressed_hash_value], new_cc,
                          next_in_bucket, prev_in_bucket);
    return true;
}

void remove_from_chunk_cond_set(chunk_cond_set* set, chunk_cond* cc)
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

void build_chunk_conds_for_grounds_and_add_negateds(agent* thisAgent,
        chunk_cond** dest_top,
        chunk_cond** dest_bottom,
        tc_number tc_to_use,
        bool* reliable)
{
    cons* c;
    condition* ground;
    chunk_cond* cc, *first_cc, *prev_cc;

    first_cc = NIL; /* unnecessary, but gcc -Wall warns without it */

    dprint(DT_BACKTRACE, "Building chunk conditions from final ground set.\n");
    /* --- build instantiated conds for grounds and setup their TC --- */
    prev_cc = NIL;
    while (thisAgent->grounds)
    {
        c = thisAgent->grounds;
        thisAgent->grounds = thisAgent->grounds->rest;
        ground = static_cast<condition_struct*>(c->first);
        free_cons(thisAgent, c);
        /* --- make the instantiated condition --- */
        allocate_with_pool(thisAgent, &thisAgent->chunk_cond_pool, &cc);
        dprint(DT_BACKTRACE, "Building chunk condition from ground condition...\n");
        dprint_condition(DT_BACKTRACE, ground);
        cc->cond = ground;
        cc->instantiated_cond = copy_condition(thisAgent, cc->cond);
        cc->variablized_cond = copy_condition(thisAgent, cc->cond);
//    cc->variablized_cond = copy_condition_without_relational_constraints (thisAgent, cc->cond);
        if (prev_cc)
        {
            prev_cc->next = cc;
            cc->prev = prev_cc;
            cc->variablized_cond->prev = prev_cc->variablized_cond;
            prev_cc->variablized_cond->next = cc->variablized_cond;
            cc->instantiated_cond->prev = prev_cc->instantiated_cond;
            prev_cc->instantiated_cond->next = cc->instantiated_cond;
        }
        else
        {
            first_cc = cc;
            cc->prev = NIL;
            cc->variablized_cond->prev = NIL;
            cc->instantiated_cond->prev = NIL;
        }
        prev_cc = cc;
        /* --- add this in to the TC --- */
        add_cond_to_tc(thisAgent, ground, tc_to_use, NIL, NIL);
    }

    /* --- scan through negated conditions and check which ones are connected
       to the grounds --- */
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
    {
        print(thisAgent,  "\n\n*** Adding Grounded Negated Conditions ***\n");
    }

    while (thisAgent->negated_set.all)
    {
        cc = thisAgent->negated_set.all;
        remove_from_chunk_cond_set(&thisAgent->negated_set, cc);
        if (cond_is_in_tc(thisAgent, cc->cond, tc_to_use))
        {
            /* --- negated cond is in the TC, so add it to the grounds --- */
            if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
            {
                print(thisAgent,  "\n-->Moving to grounds: ");
                print_condition(thisAgent, cc->cond);
            }
            cc->instantiated_cond = copy_condition(thisAgent, cc->cond);
            /* MToDo | If this condition is negated one, then we don't want to strip relationals out,
             *         so I changed to just copy_condition... */
//      cc->variablized_cond = copy_condition_without_relational_constraints (thisAgent, cc->cond);
            cc->variablized_cond = copy_condition(thisAgent, cc->cond);
            if (prev_cc)
            {
                prev_cc->next = cc;
                cc->prev = prev_cc;
                cc->variablized_cond->prev = prev_cc->variablized_cond;
                prev_cc->variablized_cond->next = cc->variablized_cond;
                cc->instantiated_cond->prev = prev_cc->instantiated_cond;
                prev_cc->instantiated_cond->next = cc->instantiated_cond;
            }
            else
            {
                first_cc = cc;
                cc->prev = NIL;
                cc->variablized_cond->prev = NIL;
                cc->instantiated_cond->prev = NIL;
            }
            prev_cc = cc;
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
                report_local_negation(thisAgent, cc->cond);    // in backtrace.cpp
                *reliable = false;
            }

            free_with_pool(&thisAgent->chunk_cond_pool, cc);
        }
    }

    if (prev_cc)
    {
        prev_cc->next = NIL;
        prev_cc->variablized_cond->next = NIL;
        prev_cc->instantiated_cond->next = NIL;
    }
    else
    {
        first_cc = NIL;
    }

    *dest_top = first_cc;
    *dest_bottom = prev_cc;

    dprint(DT_CONSTRAINTS, "Chunk conditions: \n");
    dprint_condition_list(DT_CONSTRAINTS, first_cc->cond, "");
    dprint(DT_CONSTRAINTS, "Instantiated Conditions: \n");
    dprint_condition_list(DT_CONSTRAINTS, first_cc->instantiated_cond, "");
    dprint(DT_CONSTRAINTS, "Variablized conditions: \n");
    dprint_condition_list(DT_CONSTRAINTS, first_cc->variablized_cond, "");
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

void add_goal_or_impasse_tests(agent* thisAgent, chunk_cond* all_ccs)
{
    chunk_cond* cc;
    tc_number tc;   /* mark each id as we add a test for it, so we don't add
                     a test for the same id in two different places */
    Symbol* id;
    test t;

    tc = get_new_tc_number(thisAgent);
    for (cc = all_ccs; cc != NIL; cc = cc->next)
    {
        if (cc->instantiated_cond->type != POSITIVE_CONDITION)
        {
            continue;
        }
        id = cc->instantiated_cond->data.tests.id_test->data.referent;
        if ((id->id->isa_goal || id->id->isa_impasse) &&
                (id->tc_num != tc))
        {
            t = make_test(thisAgent, NULL, ((id->id->isa_goal) ? GOAL_ID_TEST : IMPASSE_ID_TEST));
            add_test(thisAgent, &(cc->variablized_cond->data.tests.id_test), t);
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

void reorder_instantiated_conditions(chunk_cond* top_cc,
                                     condition** dest_inst_top,
                                     condition** dest_inst_bottom)
{
    chunk_cond* cc;

    /* --- Step 1:  swap prev pointers out of variablized conds into chunk_conds,
       and swap pointer to the corresponding instantiated conds into the
       variablized conds' prev pointers --- */
    for (cc = top_cc; cc != NIL; cc = cc->next)
    {
        cc->saved_prev_pointer_of_variablized_cond = cc->variablized_cond->prev;
        cc->variablized_cond->prev = cc->instantiated_cond;
    }

    /* --- Step 2:  do the reordering of the instantiated conds --- */
    for (cc = top_cc; cc != NIL; cc = cc->next)
    {
        if (cc->variablized_cond->next)
        {
            cc->instantiated_cond->next = cc->variablized_cond->next->prev;
        }
        else
        {
            cc->instantiated_cond->next = NIL;
            *dest_inst_bottom = cc->instantiated_cond;
        }

        if (cc->saved_prev_pointer_of_variablized_cond)
        {
            cc->instantiated_cond->prev =
                cc->saved_prev_pointer_of_variablized_cond->prev;
        }
        else
        {
            cc->instantiated_cond->prev = NIL;
            *dest_inst_top = cc->instantiated_cond;
        }
    }

    /* --- Step 3:  restore the prev pointers on variablized conds --- */
    for (cc = top_cc; cc != NIL; cc = cc->next)
    {
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

void make_clones_of_results(agent* thisAgent, preference* results,
                            instantiation* chunk_inst)
{
    preference* p, *result_p;

    chunk_inst->preferences_generated = NIL;
    for (result_p = results; result_p != NIL; result_p = result_p->next_result)
    {
        /* --- copy the preference --- */
        p = make_preference(thisAgent, result_p->type, result_p->id, result_p->attr,
                            result_p->value, result_p->referent,
                            soar_module::symbol_triple(result_p->original_symbols.id, result_p->original_symbols.attr, result_p->original_symbols.value),
                            soar_module::g_id_triple(result_p->g_ids.id, result_p->g_ids.attr, result_p->g_ids.value));
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

Symbol* find_goal_at_goal_stack_level(agent* thisAgent, goal_stack_level level)
{
    Symbol* g;

    for (g = thisAgent->top_goal; g != NIL; g = g->id->lower_goal)
        if (g->id->level == level)
        {
            return (g);
        }
    return (NIL);
}

Symbol* find_impasse_wme_value(Symbol* id, Symbol* attr)
{
    wme* w;

    for (w = id->id->impasse_wmes; w != NIL; w = w->next)
        if (w->attr == attr)
        {
            return w->value;
        }
    return NIL;
}

Symbol* generate_chunk_name_str_constant(agent* thisAgent, instantiation* inst)
{
    Symbol* generated_name;
    Symbol* goal;
    byte impasse_type;
    preference* p;
    goal_stack_level lowest_result_level;
    std::string lImpasseName;
    std::stringstream lName;

    chunkNameFormats chunkNameFormat = Soar_Instance::Get_Soar_Instance().Get_Chunk_Name_Format();

    lowest_result_level = thisAgent->top_goal->id->level;
    for (p = inst->preferences_generated; p != NIL; p = p->inst_next)
        if (p->id->id->level > lowest_result_level)
        {
            lowest_result_level = p->id->id->level;
        }

    goal = find_goal_at_goal_stack_level(thisAgent, lowest_result_level);

    switch (chunkNameFormat)
    {
        case numberedFormat:
        {
            return (generate_new_str_constant(
                        thisAgent,
                        thisAgent->chunk_name_prefix,
                        &thisAgent->chunk_count));
        }
        case longFormat:
        {
            if (goal)
            {
                impasse_type = type_of_existing_impasse(thisAgent, goal);
                switch (impasse_type)
                {
                    case NONE_IMPASSE_TYPE:
                        lImpasseName = "unknownimpasse";
                        break;
                    case CONSTRAINT_FAILURE_IMPASSE_TYPE:
                        lImpasseName = "cfailure";
                        break;
                    case CONFLICT_IMPASSE_TYPE:
                        lImpasseName = "conflict";
                        break;
                    case TIE_IMPASSE_TYPE:
                        lImpasseName = "tie";
                        break;
                    case NO_CHANGE_IMPASSE_TYPE:
                    {
                        Symbol* sym;

                        if ((sym = find_impasse_wme_value(goal->id->lower_goal, thisAgent->attribute_symbol)) == NIL)
                        {
                            lImpasseName = "unknownimpasse";
                        }
                        else if (sym == thisAgent->operator_symbol)
                        {
                            lImpasseName = "opnochange";
                        }
                        else if (sym == thisAgent->state_symbol)
                        {
                            lImpasseName = "snochange";
                        }
                        else
                        {
                            lImpasseName = "unknownimpasse";
                        }
                    }
                    break;
                    default:
                        lImpasseName = "unknownimpasse";
                        break;
                }
            }
            else
            {
                lImpasseName = "unknownimpasse";
            }
            lName << thisAgent->chunk_name_prefix << "-" << thisAgent->chunk_count++ << "*" <<
                  thisAgent->d_cycle_count << "*" << lImpasseName << "*" << thisAgent->chunks_this_d_cycle;

            break;
        }
        case ruleFormat:
        {
            std::string real_prod_name;

            lImpasseName.erase();
            lName << thisAgent->chunk_name_prefix;

            thisAgent->chunk_count++;
            if (goal)
            {
                impasse_type = type_of_existing_impasse(thisAgent, goal);
                switch (impasse_type)
                {
                    case CONSTRAINT_FAILURE_IMPASSE_TYPE:
                        lImpasseName = "cfo";
                        break;
                    case CONFLICT_IMPASSE_TYPE:
                        lImpasseName = "con";
                        break;
                    case TIE_IMPASSE_TYPE:
                        lImpasseName = "tie";
                        break;
                    case NO_CHANGE_IMPASSE_TYPE:
                    {
                        Symbol* sym;
                        sym = find_impasse_wme_value(goal->id->lower_goal, thisAgent->attribute_symbol);
                        if (sym)
                        {
                            if (sym == thisAgent->operator_symbol)
                            {
                                lImpasseName = "onc";
                            }
                            else if (sym == thisAgent->state_symbol)
                            {
                                lImpasseName = "snc";
                            }
                        }
                    }
                    break;
                    default:
                        break;
                }
            }

            if (inst->prod)
            {
                if (strstr(inst->prod->name->sc->name, thisAgent->chunk_name_prefix) != inst->prod->name->sc->name)
                {
                    /*-- This is a chunk based on a chunk, so annotate name to indicate --*/
                    lName << "-multi";
                }
                lName << "*" << inst->prod->original_rule_name;
            }
            if (!lImpasseName.empty())
            {
                lName << "*" << lImpasseName;
            }
            lName << "*t" << thisAgent->d_cycle_count << "-" << thisAgent->chunks_this_d_cycle;
        }
    }
    lImpasseName.erase();
    if (lName.str().empty()) { return NULL; }

    /* Any user who named a production like this deserves to be burned, but we'll have mercy: */
    if (find_str_constant(thisAgent, lName.str().c_str()))
    {
        uint64_t collision_count = 1;
        std::stringstream newLName;

        print(thisAgent, "Warning: generated chunk name already exists.  Will find unique name.\n");
        xml_generate_warning(thisAgent, "Warning: generated chunk name already exists.  Will find unique name.");
        do
        {
            newLName.str(lName.str().c_str());
            newLName << "-" << collision_count++;
        }
        while (find_str_constant(thisAgent, newLName.str().c_str()));
        lName.str(newLName.str());
        newLName.str("");
    }

    generated_name = make_str_constant(thisAgent, lName.str().c_str());
    return generated_name;
}

bool should_variablize(agent* thisAgent, instantiation* inst)
{
    preference* p;

    if (thisAgent->sysparams[LEARNING_ON_SYSPARAM] == 0)
    {
        return false;
    }

    if (thisAgent->sysparams[LEARNING_EXCEPT_SYSPARAM] &&
            member_of_list(inst->match_goal, thisAgent->chunk_free_problem_spaces))
    {
        if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_CHUNKS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\nnot chunking due to chunk-free state " << inst->match_goal->to_string();
            print(thisAgent,  message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
        }
        return false;
    }

    if (thisAgent->sysparams[LEARNING_ONLY_SYSPARAM] &&
            !member_of_list(inst->match_goal, thisAgent->chunky_problem_spaces))
    {
        if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_CHUNKS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\nnot chunking due to non-chunky state " << inst->match_goal->to_string();
            print(thisAgent,  message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
        }
        return false;
    }

    /* allow_bottom_up_chunks will be false if a chunk was already
       learned in a lower goal
     */
    if (!thisAgent->sysparams[LEARNING_ALL_GOALS_SYSPARAM] &&
            !inst->match_goal->id->allow_bottom_up_chunks)
    {
        return false;
    }

    return true;
}

/* ====================================================================

                        Chunk Instantiation

   This the main chunking routine.  It takes an instantiation, and a
   flag "variablize"--if false, the chunk will not be
   variablized.  (If true, it may still not be variablized, due to
   chunk-free-problem-spaces, ^quiescence t, etc.)
==================================================================== */


void chunk_instantiation(agent* thisAgent, instantiation* inst, bool dont_variablize, instantiation** custom_inst_list)
{
    goal_stack_level grounds_level;
    preference* results, *pref;
    action* rhs;
    production* prod;
    instantiation* chunk_inst;
    Symbol* prod_name;
    byte prod_type;
    bool print_name, print_prod;
    byte rete_addition_result;
    condition* lhs_top, *lhs_bottom;
    chunk_cond* top_cc, *bottom_cc;
    bool reliable = true;
    bool variablize;

    explain_chunk_str temp_explain_chunk;
    memset(temp_explain_chunk.name, 0, EXPLAIN_CHUNK_STRUCT_NAME_BUFFER_SIZE);

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    soar_process_timer local_timer;
    local_timer.set_enabled(&(thisAgent->sysparams[ TIMERS_ENABLED ]));
#endif
#endif

    /* --- if it only matched an attribute impasse, don't chunk --- */
    if (! inst->match_goal)
    {
        return;
    }

    /* --- if no preference is above the match goal level, exit --- */
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

    results = get_results_for_instantiation(thisAgent, inst);
    if (!results)
    {
        goto chunking_done;
    }

    dprint(DT_FUNC_PRODUCTIONS, "=========================================================\n");
    dprint(DT_FUNC_PRODUCTIONS, "chunk_instantiation() called...\n");
    dprint(DT_FUNC_PRODUCTIONS, "=========================================================\n");

    /* set allow_bottom_up_chunks to false for all higher goals to prevent chunking */
    {
        Symbol* g;
        for (g = inst->match_goal->id->higher_goal; g && g->id->allow_bottom_up_chunks; g = g->id->higher_goal)
        {
            g->id->allow_bottom_up_chunks = false;
        }
    }

    grounds_level = inst->match_goal_level - 1;

    thisAgent->backtrace_number++;
    if (thisAgent->backtrace_number == 0)
    {
        thisAgent->backtrace_number = 1;
    }

    thisAgent->grounds_tc++;
    if (thisAgent->grounds_tc == 0)
    {
        thisAgent->grounds_tc = 1;
    }

    thisAgent->potentials_tc++;
    if (thisAgent->potentials_tc == 0)
    {
        thisAgent->potentials_tc = 1;
    }

    thisAgent->locals_tc++;
    if (thisAgent->locals_tc == 0)
    {
        thisAgent->locals_tc = 1;
    }

    thisAgent->grounds = NIL;
    thisAgent->positive_potentials = NIL;
    thisAgent->locals = NIL;

    /* Start a new structure for this potential chunk */

    if (thisAgent->sysparams[EXPLAIN_SYSPARAM])
    {
        temp_explain_chunk.conds       = NULL;
        temp_explain_chunk.actions     = NULL;
        temp_explain_chunk.backtrace   = NULL;
        temp_explain_chunk.name[0]     = '\0';
        temp_explain_chunk.all_grounds = NIL;
        temp_explain_chunk.next_chunk  = NULL;
        reset_backtrace_list(thisAgent);
    }

    dprint(DT_CONSTRAINTS,  "Backtracing through instantiation...\n");
    /* --- backtrace through the instantiation that produced each result --- */
    for (pref = results; pref != NIL; pref = pref->next_result)
    {
        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
        {
            print(thisAgent,  "\nFor result preference ");
            xml_begin_tag(thisAgent, kTagBacktraceResult);
            print_preference(thisAgent, pref);
            print(thisAgent,  " ");
        }
        backtrace_through_instantiation(thisAgent, pref->inst, grounds_level, NULL, &reliable, 0);

        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
        {
            xml_end_tag(thisAgent, kTagBacktraceResult);
        }
    }

    dprint(DT_CONSTRAINTS,  "Backtracing through instantiation DONE.\n");

    dprint(DT_BACKTRACE, "Grounds after backtracing:\n");
    dprint_condition_cons(DT_BACKTRACE, thisAgent->grounds);

    while (true)
    {
        trace_locals(thisAgent, grounds_level, &reliable);
        trace_grounded_potentials(thisAgent);
        if (! trace_ungrounded_potentials(thisAgent, grounds_level, &reliable))
        {
            break;
        }
    }

    dprint(DT_BACKTRACE, "Grounds after tracing:\n");
    dprint_condition_cons(DT_BACKTRACE, thisAgent->grounds);

    thisAgent->variablizationManager->print_cached_constraints(DT_CONSTRAINTS);

    free_list(thisAgent, thisAgent->positive_potentials);

    /* --- backtracing done; collect the grounds into the chunk --- */
    {
        tc_number tc_for_grounds;
        tc_for_grounds = get_new_tc_number(thisAgent);
        build_chunk_conds_for_grounds_and_add_negateds(thisAgent, &top_cc, &bottom_cc, tc_for_grounds, &reliable);
    }

    variablize = !dont_variablize && reliable && should_variablize(thisAgent, inst);

    /* --- get symbol for name of new chunk or justification --- */
    if (variablize)
    {
        thisAgent->chunks_this_d_cycle++;
        prod_name = generate_chunk_name_str_constant(thisAgent, inst);
        prod_type = CHUNK_PRODUCTION_TYPE;
        print_name = (thisAgent->sysparams[TRACE_CHUNK_NAMES_SYSPARAM] != 0);
        print_prod = (thisAgent->sysparams[TRACE_CHUNKS_SYSPARAM] != 0);
    }
    else
    {
        prod_name = generate_new_str_constant(thisAgent, "justification-", &thisAgent->justification_count);
        prod_type = JUSTIFICATION_PRODUCTION_TYPE;
        print_name = (thisAgent->sysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] != 0);
        print_prod = (thisAgent->sysparams[TRACE_JUSTIFICATIONS_SYSPARAM] != 0);
    }

    if (print_name)
    {
        if (get_printer_output_column(thisAgent) != 1)
        {
            print(thisAgent,  "\n");
        }
//      print_with_symbols (thisAgent, "Building %y\n", prod_name);

        xml_begin_tag(thisAgent, kTagLearning);
        xml_begin_tag(thisAgent, kTagProduction);
        xml_att_val(thisAgent, kProduction_Name, prod_name);
        xml_end_tag(thisAgent, kTagProduction);
        xml_end_tag(thisAgent, kTagLearning);
    }
    dprint(DT_FUNC_PRODUCTIONS, "Backtracing done.  Building chunk %s\n", prod_name->to_string());

    /* --- if there aren't any grounds, exit --- */
    if (! top_cc)
    {
        if (thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM])
        {
            print(thisAgent,  "Warning: chunk has no grounds, ignoring it.\n");
            xml_generate_warning(thisAgent, "Warning: chunk has no grounds, ignoring it.");
        }

        goto chunking_done;
    }

    if (thisAgent->chunks_this_d_cycle > static_cast<uint64_t>(thisAgent->sysparams[MAX_CHUNKS_SYSPARAM]))
    {
        if (thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM])
        {
            print(thisAgent,  "Warning: reached max-chunks! Halting system.\n");
            xml_generate_warning(thisAgent, "Warning: reached max-chunks! Halting system.");
        }
        thisAgent->max_chunks_reached = true;

        goto chunking_done;
    }

    lhs_top = top_cc->variablized_cond;
    lhs_bottom = bottom_cc->variablized_cond;

    dprint(DT_PRINT_INSTANTIATIONS,  "chunk_instantiation variablizing following chunk instantiation: \n");
    dprint_cond_results(DT_PRINT_INSTANTIATIONS, lhs_top, results);

    if (variablize)
    {
        reset_variable_generator(thisAgent, lhs_top, NIL);
        thisAgent->variablizationManager->variablize_condition_list(lhs_top);
        thisAgent->variablizationManager->variablize_relational_constraints();

        thisAgent->variablizationManager->install_cached_constraints(lhs_top);

        dprint(DT_MERGE, "Polishing variablized conditions: \n");

        thisAgent->variablizationManager->fix_conditions(lhs_top);
#ifndef MERGE_CONDITIONS_EARLY
        thisAgent->variablizationManager->merge_conditions(lhs_top);
#endif
        dprint(DT_CONSTRAINTS, "Merged variablized conditions with relational constraints: \n");
        dprint_condition_list(DT_CONSTRAINTS, lhs_top, "");

    }

    dprint(DT_RHS_VARIABLIZATION, "==========================================\n");
    dprint(DT_RHS_VARIABLIZATION, "Variablizing RHS action list:\n");
    dprint(DT_RHS_VARIABLIZATION, "==========================================\n");
    rhs = thisAgent->variablizationManager->variablize_results(results, variablize);
    dprint(DT_RHS_VARIABLIZATION, "==========================================\n");
    dprint(DT_RHS_VARIABLIZATION, "Done variablizing RHS action list.\n");
    dprint(DT_RHS_VARIABLIZATION, "==========================================\n");

    dprint(DT_CONSTRAINTS, "- Instantiated conds before add_goal_test\n");
    dprint_condition_list(DT_CONSTRAINTS, top_cc->instantiated_cond);
    dprint(DT_CONSTRAINTS, "- Variablized conds before add_goal_test\n");
    dprint_condition_list(DT_CONSTRAINTS, top_cc->variablized_cond);

    add_goal_or_impasse_tests(thisAgent, top_cc);

    dprint(DT_PRINT_INSTANTIATIONS,  "chunk instantiation created variablized rule: \n");
    dprint_cond_actions(DT_PRINT_INSTANTIATIONS, lhs_top, rhs);

    prod = make_production(thisAgent, prod_type, prod_name, (inst->prod ? inst->prod->name->sc->name : prod_name->sc->name), &lhs_top, &lhs_bottom, &rhs, false);

    dprint(DT_PRINT_INSTANTIATIONS,  "chunk instantiation created reordered rule: \n");
    dprint_cond_actions(DT_PRINT_INSTANTIATIONS, lhs_top, rhs);

    if (!prod)
    {
        print(thisAgent,  "\nUnable to reorder this chunk:\n  ");
        print_condition_list(thisAgent, lhs_top, 2, false);
        print(thisAgent,  "\n  -->\n   ");
        print_action_list(thisAgent, rhs, 3, false);
        print(thisAgent,  "\n\nThis error is likely caused by the reasons outlined section 4 of the Soar\n");
        print(thisAgent,  "manual, subsection \"revising the substructure of a previous result\".\n\n");
        print(thisAgent,  "Check that the rules are not revising substructure of a result matched only\n");
        print(thisAgent,  "through the local state.\n");

        deallocate_condition_list(thisAgent, top_cc->variablized_cond);
        deallocate_condition_list(thisAgent, top_cc->instantiated_cond);
        {
            chunk_cond* cc;
            while (top_cc)
            {
                cc = top_cc;
                top_cc = cc->next;
                free_with_pool(&thisAgent->chunk_cond_pool, cc);
            }
        }

        deallocate_action_list(thisAgent, rhs);

        // We cannot proceed, the GDS will crash in decide.cpp:decide_non_context_slot
        thisAgent->stop_soar = true;
        thisAgent->system_halted = true;

        goto chunking_done;
    }

    {
        condition* inst_lhs_top = 0, *inst_lhs_bottom = 0;

        reorder_instantiated_conditions(top_cc, &inst_lhs_top, &inst_lhs_bottom);

        /* Record the list of grounds in the order they will appear in the chunk. */
        if (thisAgent->sysparams[EXPLAIN_SYSPARAM])
        {
            temp_explain_chunk.all_grounds = inst_lhs_top;    /* Not a copy yet */
        }

        allocate_with_pool(thisAgent, &thisAgent->instantiation_pool, &chunk_inst);
        chunk_inst->prod = prod;
        chunk_inst->top_of_instantiated_conditions = inst_lhs_top;
        chunk_inst->bottom_of_instantiated_conditions = inst_lhs_bottom;

        chunk_inst->GDS_evaluated_already = false;  /* REW:  09.15.96 */
        chunk_inst->reliable = reliable;

        chunk_inst->in_ms = true;  /* set true for now, we'll find out later... */
        make_clones_of_results(thisAgent, results, chunk_inst);
        fill_in_new_instantiation_stuff(thisAgent, chunk_inst, true, inst);
    }

    dprint(DT_PRINT_INSTANTIATIONS, "chunk instantiation created reordered instantiation: \n");
    dprint_cond_prefs(DT_PRINT_INSTANTIATIONS, chunk_inst->top_of_instantiated_conditions, chunk_inst->preferences_generated);

    /* RBD 4/6/95 Need to copy cond's and actions for the production here,
    otherwise some of the variables might get deallocated by the call to
    add_production_to_rete() when it throws away chunk variable names. */
    if (thisAgent->sysparams[EXPLAIN_SYSPARAM])
    {
        condition* new_top = 0;
        condition* new_bottom = 0;
        copy_condition_list(thisAgent, lhs_top, &new_top, &new_bottom);
        temp_explain_chunk.conds = new_top;
        //temp_explain_chunk.actions = copy_and_variablize_result_list (thisAgent, results, variablize);
        temp_explain_chunk.actions = copy_action_list(thisAgent, rhs);
    }
    /* MToDo | Remove the print_name parameter disabling here. */
    rete_addition_result = add_production_to_rete(thisAgent, prod, lhs_top, chunk_inst, print_name || true);

    dprint(DT_PRINT_INSTANTIATIONS, "Add production to rete result: %s\n",
           ((rete_addition_result == DUPLICATE_PRODUCTION) ? "Duplicate production!" :
            (rete_addition_result == REFRACTED_INST_DID_NOT_MATCH) ? "Refracted instantiation did not match!" :
            (rete_addition_result == REFRACTED_INST_MATCHED) ? "Refracted instantiation matched." :
            (rete_addition_result == NO_REFRACTED_INST) ? "No refracted instantiation given." : "INVALID RETE RETURN TYPE!"));

    /* If didn't immediately excise the chunk from the rete net
    then record the temporary structure in the list of explained chunks. */

    if (thisAgent->sysparams[EXPLAIN_SYSPARAM])
    {
        if ((rete_addition_result != DUPLICATE_PRODUCTION)
                && ((prod_type != JUSTIFICATION_PRODUCTION_TYPE)
                    || (rete_addition_result != REFRACTED_INST_DID_NOT_MATCH)))
        {
            strncpy(temp_explain_chunk.name, prod_name->sc->name, EXPLAIN_CHUNK_STRUCT_NAME_BUFFER_SIZE);
            temp_explain_chunk.name[EXPLAIN_CHUNK_STRUCT_NAME_BUFFER_SIZE - 1] = 0;
            explain_add_temp_to_chunk_list(thisAgent, &temp_explain_chunk);
        }
        else
        {
            /* RBD 4/6/95 if excised the chunk, discard previously-copied stuff */
            deallocate_condition_list(thisAgent, temp_explain_chunk.conds);
            deallocate_action_list(thisAgent, temp_explain_chunk.actions);
        }
    }

    dprint(DT_PRINT_INSTANTIATIONS, "chunk instantiation created final production: \n");
    dprint_production(DT_PRINT_INSTANTIATIONS, (rete_addition_result != DUPLICATE_PRODUCTION) ? prod : NIL);
    dprint(DT_PRINT_INSTANTIATIONS, "=========================================================\n");

    /* --- deallocate chunks conds and variablized conditions --- */
    deallocate_condition_list(thisAgent, lhs_top);
    {
        chunk_cond* cc;
        while (top_cc)
        {
            cc = top_cc;
            top_cc = cc->next;
            free_with_pool(&thisAgent->chunk_cond_pool, cc);
        }
    }
    /* MToDo | Do we need to deallocate the rhs here? It doesn't seem to be done anywhere.*/

    if (print_prod && (rete_addition_result != DUPLICATE_PRODUCTION))
    {
        print(thisAgent,  "\n");
        xml_begin_tag(thisAgent, kTagLearning);
        print_production(thisAgent, prod, false);
        xml_end_tag(thisAgent, kTagLearning);
    }

    if (rete_addition_result == DUPLICATE_PRODUCTION)
    {
        excise_production(thisAgent, prod, false);
    }
    else if ((prod_type == JUSTIFICATION_PRODUCTION_TYPE)
             && (rete_addition_result == REFRACTED_INST_DID_NOT_MATCH))
    {
        excise_production(thisAgent, prod, false);
    }

    /* MToDo | Possible fix for GDS bug.  Before, it changed in_ms to false for duplicate
     *         productions.  Don't fully understand the repercussions of this. */

#ifdef OLD_DUPLICATE_CHUNK_METHOD
    if (rete_addition_result != REFRACTED_INST_MATCHED)
#else
    if (rete_addition_result == REFRACTED_INST_DID_NOT_MATCH)
#endif
    {
        /* --- It didn't match or was a duplicate production, so tell the firer it didn't
         *     match, so it'll only assert the o-supported preferences --- */

        chunk_inst->in_ms = false;
    }

    /* --- assert the preferences --- */
    chunk_inst->next = (*custom_inst_list);
    (*custom_inst_list) = chunk_inst;

    thisAgent->variablizationManager->clear_variablization_tables();
    thisAgent->variablizationManager->clear_cached_constraints();

    /* MVP 6-8-94 */
    if (!thisAgent->max_chunks_reached)
    {
        dprint(DT_FUNC_PRODUCTIONS, "Calling chunk instantiation from chunk instantation...\n");
        dprint(DT_FUNC_PRODUCTIONS, "=========================================================\n");
        chunk_instantiation(thisAgent, chunk_inst, dont_variablize, custom_inst_list);
    }

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    local_timer.stop();
    thisAgent->timers_chunking_cpu_time[thisAgent->current_phase].update(local_timer);
#endif
#endif

    dprint(DT_FUNC_PRODUCTIONS, "chunk_instantiation() done building chunk %s\n", prod_name->to_string());
    dprint(DT_FUNC_PRODUCTIONS, "=========================================================\n");
    return;

chunking_done:
    {}
    thisAgent->variablizationManager->clear_variablization_tables();
    thisAgent->variablizationManager->clear_cached_constraints();

    dprint(DT_FUNC_PRODUCTIONS, "chunk_instantiation() done building chunk %s\n", prod_name->to_string());
    dprint(DT_FUNC_PRODUCTIONS, "=========================================================\n");
    symbol_remove_ref(thisAgent, prod_name);

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    local_timer.stop();
    thisAgent->timers_chunking_cpu_time[thisAgent->current_phase].update(local_timer);
#endif
#endif
}

/* --------------------------------------------------------------------

  Chunker Initialization

    Init_chunker() is called at startup time to do initialization here.
    -------------------------------------------------------------------- */

void init_chunker(agent* thisAgent)
{
    init_memory_pool(thisAgent, &thisAgent->chunk_cond_pool, sizeof(chunk_cond), "chunk condition");
    init_chunk_cond_set(&thisAgent->negated_set);
}

