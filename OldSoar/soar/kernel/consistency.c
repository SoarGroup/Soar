/*************************************************************************
 *
 *  file:  consistency.c
 *
 * =======================================================================
 *
 * Source code for Operand2/Waterfall specific functions in the kernel.
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
 *
 * Revision history:
 * 
 * 05 May 97: Created for version 2.0 of Operand2
 * REW
 *
 * 20 Aug 97: Version 2.1 of Operand2/Waterfall
 * Reimplemented the Waterfall functions with a more efficient algorithm
 * REW
 *
 */

/* For information on the consistency check routines */
/* #define DEBUG_CONSISTENCY_CHECK    */

/* For information on aspects of determining the active level */
/* #define DEBUG_DETERMINE_LEVEL_PHASE   */

#include "soarkernel.h"

/* REW: begin 08.20.97 */
#define NEW_DECISION         0
#define SAME_LEVEL           1
#define HIGHER_LEVEL         2
#define LOWER_LEVEL          3
#define NIL_GOAL_RETRACTIONS 4
/* REW: end   08.20.97 */

/* REW: begin 09.15.96 */
extern void remove_wmes_for_context_slot(slot * s);
extern void remove_existing_context_and_descendents(Symbol * goal);
extern byte type_of_existing_impasse(Symbol * goal);
extern Symbol *attribute_of_existing_impasse(Symbol * goal);
extern byte run_preference_semantics_for_consistency_check(slot * s, preference ** result_candidates);

void remove_operator_if_necessary(slot * s, wme * w);
bool decision_consistent_with_current_preferences(Symbol * goal, slot * s);
void remove_current_decision(slot * s);
bool check_context_slot_decisions(goal_stack_level level);
/* REW: end   09.15.96 */

                           /* REW: begin 08.20.97 *//* To implement the Waterfall part of Operand2 */
extern void print_assertion(ms_change * msc);
extern void print_retraction(ms_change * msc);
void initialize_consistency_calculations_for_new_decision();
void determine_highest_active_production_level_in_stack_apply();
void determine_highest_active_production_level_in_stack_propose();
bool goal_stack_consistent_through_goal(Symbol * goal);
bool i_activity_at_goal(Symbol * goal);
bool minor_quiescence_at_goal(Symbol * goal);
int active_production_type_at_goal(Symbol * goal);
Symbol *highest_active_goal_propose();
Symbol *highest_active_goal_apply();
/* REW: end   08.20.97 */

void remove_operator_if_necessary(slot * s, wme * w)
{

    /* REW: begin 11.25.96 */
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    start_timer(&current_agent(start_gds_tv));
#endif
#endif
    /* REW: end   11.25.96 */

    /*         print("Examining slot (next)\n");
       for (next = s; next; next=next->next){
       print_with_symbols("Slot ID:   [%y]\n", next->id);
       print_with_symbols("Slot Attr: [%y]\n", next->attr);
       }

       print("Examining slot (prev)\n");
       for (prev = s->prev; prev; prev=prev->prev){
       print_with_symbols("Slot ID:   [%y]\n", prev->id);
       print_with_symbols("Slot Attr: [%y]\n", prev->attr);
       }

       print("Examining slot WMEs\n");
       for (slot_wmes=s->wmes; slot_wmes; slot_wmes=slot_wmes->next){
       print_wme(slot_wmes);
       }

       print("Examining acceptable preference WMEs\n");
       for (slot_wmes=s->acceptable_preference_wmes; slot_wmes; slot_wmes=slot_wmes->next){
       print_wme(slot_wmes);
       }

       if (current_agent(highest_goal_whose_context_changed)) print_with_symbols("Highest goal with changed context: [%y]\n", current_agent(highest_goal_whose_context_changed));

       print_with_symbols("Slot ID:   [%y]\n", s->id);
       print_with_symbols("Slot Attr: [%y]\n", s->attr);
       if (s->isa_context_slot) print("this is a context slot.\n");
       if (s->impasse_id) print_with_symbols("Impasse: [%y]\n", s->impasse_id);
       if (s->acceptable_preference_changed) print("Acceptable pref changed\n");

       print_with_symbols("WME ID:    [%y]\n", w->id);
       print_with_symbols("WME Attr:  [%y]\n", w->attr);
       print_with_symbols("WME Value: [%y]\n", w->value);
       if (w->value->id.isa_operator) print("This is an operator\n");

       print_with_symbols("s->id->id.operator_slot->id: [%y]\n", s->id->id.operator_slot->id); */

    if (s->wmes) {              /* If there is something in the context slot */
        if (s->wmes->value == w->value) {       /* The WME in the context slot is WME whose pref changed */

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

            if (current_agent(sysparams)[TRACE_OPERAND2_REMOVALS_SYSPARAM]) {
                print("\n        REMOVING: Operator from context slot (proposal no longer matches): ");
                print_wme(w);
            }
#endif
            remove_wmes_for_context_slot(s);
            if (s->id->id.lower_goal)
                remove_existing_context_and_descendents(s->id->id.lower_goal);
        }
    }

    /* REW: begin 11.25.96 */
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    stop_timer(&current_agent(start_gds_tv), &current_agent(gds_cpu_time[current_agent(current_phase)]));
#endif
#endif
    /* REW: end   11.25.96 */
}

/* This code concerns the implementation of a 'consistency check' following
   each IE phase.  The basic idea is that we want context decisions to 
   remain consistent with the current preferences, even if the proposal
   for some operator is still acceptable */

bool decision_consistent_with_current_preferences(Symbol * goal, slot * s)
{
    byte current_impasse_type, new_impasse_type;
    Symbol *current_impasse_attribute;
    wme *current_operator;
    preference *candidates, *cand;
    bool operator_in_slot, goal_is_impassed;

#ifdef DEBUG_CONSISTENCY_CHECK
    if (s->isa_context_slot) {
        print("    slot (s)  isa context slot: ");
        print_with_symbols("    Slot Identifier [%y] and attribute [%y]\n", s->id, s->attr);
    }
    /* print("    Address of s: %x\n", s); */
    print("    s->impasse_type: %d\n", s->impasse_type);
    if (s->impasse_id)
        print("    Impasse ID is set (non-NIL)\n");
#endif

    /* Determine the current operator/impasse in the slot */
    if (goal->id.operator_slot->wmes) {
        /* There is an operator in the slot */
        current_operator = goal->id.operator_slot->wmes;
        operator_in_slot = TRUE;
    } else {
        /* There is not an operator in the slot */
        current_operator = NIL;
        operator_in_slot = FALSE;
    }

    if (goal->id.lower_goal) {
        /* the goal is impassed */
        goal_is_impassed = TRUE;
        current_impasse_type = type_of_existing_impasse(goal);
        current_impasse_attribute = attribute_of_existing_impasse(goal);
#ifdef DEBUG_CONSISTENCY_CHECK
        print("    Goal is impassed:  Impasse type: %d: ", current_impasse_type);
        print_with_symbols("    Impasse attribute: [%y]\n", current_impasse_attribute);
#endif
        /* Special case for an operator no-change */
        if ((operator_in_slot) && (current_impasse_type == NO_CHANGE_IMPASSE_TYPE)) {
            /* Operator no-change impasse: run_preference_semantics will return 0
               and we only want to blow away this operator if another is better
               than it (checked in NONE_IMPASSE_TYPE switch) or if another kind
               of impasse would be generated (e.g., OPERATOR_TIE). So, we set
               the impasse type here to 0; that way we'll know that we should be
               comparing a previous decision for a unique operator against the
               current preference semantics. */
#ifdef DEBUG_CONSISTENCY_CHECK
            print("    This is an operator no-change  impasse.\n");
#endif
            current_impasse_type = NONE_IMPASSE_TYPE;
        }
    } else {
        goal_is_impassed = FALSE;
        current_impasse_type = NONE_IMPASSE_TYPE;
        current_impasse_attribute = NIL;
#ifdef DEBUG_CONSISTENCY_CHECK
        print("    Goal is not impassed: ");
#endif
    }

    /* Determine the new impasse type, based on the preferences that exist now */
    new_impasse_type = run_preference_semantics_for_consistency_check(s, &candidates);

#ifdef DEBUG_CONSISTENCY_CHECK
    print("    Impasse Type returned by run preference semantics: %d\n", new_impasse_type);

    for (cand = candidates; cand; cand = cand->next) {
        print("    Preference for slot:");
        print_preference(cand);
    }

    for (cand = candidates; cand; cand = cand->next_candidate) {
        print("\n    Candidate  for slot:");
        print_preference(cand);
    }
#endif

    if (current_impasse_type != new_impasse_type) {
        /* Then there is an inconsistency: no more work necessary */
#ifdef DEBUG_CONSISTENCY_CHECK
        print
            ("    Impasse types are different: Returning FALSE, preferences are not consistent with prior decision.\n");
#endif
        return FALSE;
    }

    /* in these cases, we know that the new impasse and the old impasse *TYPES* are the same.  We
       just want to check and make the actual impasses/decisions are the same. */
    switch (new_impasse_type) {

    case NONE_IMPASSE_TYPE:
        /* There are four cases to consider when NONE_IMPASSE_TYPE is returned: */
        /* 1.  Previous operator and operator returned by run_pref_sem are the same.
           In this case, return TRUE (decision remains consistent) */

        /* This next if is meant to test that there actually is something in the slot but
           I'm nut quite certain that it will not always be true? */
        if (operator_in_slot) {
#ifdef DEBUG_CONSISTENCY_CHECK
            print("    There is a WME in the operator slot:");
            print_wme(current_operator);
#endif

            /* Because of indifferent preferences, we need to compare all possible candidates
               with the current decision */
            for (cand = candidates; cand; cand = cand->next_candidate) {
                if (current_operator->value == cand->value) {
#ifdef DEBUG_CONSISTENCY_CHECK
                    print_with_symbols("       Operator slot ID [%y] and candidate ID [%y] are the same.\n",
                                       current_operator->value, cand->value);
#endif
                    return TRUE;
                }
            }

            /* 2.  A different operator is indicated for the slot than the one that is
               currently installed.  In this case, we return FALSE (the decision is
               not consistent with the preferences). */

            /* Now we know that the decision is inconsistent */
            return FALSE;

            /* 3.  A single operator is suggested when an impasse existed previously.
               In this case, return FALSE so that the impasse can be removed. */

        } else {                /* There is no operator in the slot */
            if (goal->id.lower_goal) {  /* But there is an impasse */
                if (goal->id.lower_goal->id.isa_impasse)
                    print("This goal is an impasse\n");
                print("      No Impasse Needed but Impasse exists: remove impasse now\n");
                print("\n\n   *************This should never be executed*******************\n\n");
                return FALSE;
            }
        }

        /* 4.  This is the bottom goal in the stack and there is no operator or
           impasse for the operator slot created yet.  We shouldn't call this
           routine in this case (this condition is checked before  
           decision_consistent_with_current_preferences is called) but, for
           completeness' sake, we check this condition and return TRUE
           (because no decision has been made at this level, there is no 
           need to remove anything). */
        print("\n\n   *************This should never be executed*******************\n\n");
        return TRUE;
        break;

    case CONSTRAINT_FAILURE_IMPASSE_TYPE:
#ifdef DEBUG_CONSISTENCY_CHECK
        print("    Constraint Failure Impasse: Returning TRUE\n");
#endif
        return TRUE;
        break;

    case CONFLICT_IMPASSE_TYPE:
#ifdef DEBUG_CONSISTENCY_CHECK
        print("    Conflict Impasse: Returning TRUE\n");
#endif
        return TRUE;
        break;

    case TIE_IMPASSE_TYPE:
#ifdef DEBUG_CONSISTENCY_CHECK
        print("    Tie Impasse: Returning TRUE\n");
#endif
        return TRUE;
        break;

    case NO_CHANGE_IMPASSE_TYPE:
#ifdef DEBUG_CONSISTENCY_CHECK
        print("    No change Impasse: Returning TRUE\n");
#endif
        return TRUE;
        break;
    }

    print("\n   After switch................");
    print("\n\n   *************This should never be executed*******************\n\n");
    return TRUE;

}

void remove_current_decision(slot * s)
{

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

    if (!s->wmes)
        if (current_agent(sysparams)[TRACE_OPERAND2_REMOVALS_SYSPARAM])
            print_with_symbols("\n       REMOVING CONTEXT SLOT: Slot Identifier [%y] and attribute [%y]\n", s->id,
                               s->attr);

    if (s->id)
        if (current_agent(sysparams)[TRACE_OPERAND2_REMOVALS_SYSPARAM])
            print_with_symbols("\n          Decision for goal [%y] is inconsistent.  Replacing it with....\n", s->id);

#endif

    /* If there is an operator in the slot, remove it */
    remove_wmes_for_context_slot(s);

    /* If there are any subgoals, remove those */
    if (s->id->id.lower_goal)
        remove_existing_context_and_descendents(s->id->id.lower_goal);

    do_buffered_wm_and_ownership_changes();

}

/* ------------------------------------------------------------------
                       Check Context Slot Decisions

   This scans down the goal stack and checks the consistency of the current
   decision versus the current preferences for the slot, if the preferences
   have changed.
------------------------------------------------------------------ */

bool check_context_slot_decisions(goal_stack_level level)
{
    Symbol *goal;
    slot *s;

#ifdef DEBUG_CONSISTENCY_CHECK
    if (current_agent(highest_goal_whose_context_changed))
        print_with_symbols("    Highest goal with changed context: [%y]\n",
                           current_agent(highest_goal_whose_context_changed));
#endif

/* REW: begin 05.05.97 */
    /* Check only those goals where preferences have changes that are at or above the level 
       of the consistency check */
    for (goal = current_agent(highest_goal_whose_context_changed); goal && goal->id.level <= level;
         goal = goal->id.lower_goal) {
/* REW: end   05.05.97 */
#ifdef DEBUG_CONSISTENCY_CHECK
        print_with_symbols("    Looking at goal [%y] to see if its preferences have changed\n", goal);
#endif
        s = goal->id.operator_slot;

        if ((goal->id.lower_goal) || (s->wmes)) {       /* If we are not at the bottom goal or if there is an operator in the
                                                           bottom goal's operator slot */
#ifdef DEBUG_CONSISTENCY_CHECK
            print
                ("      This is a goal that either has subgoals or, if the bottom goal, has an operator in the slot\n");
#endif
            if (s->changed) {   /* Only need to check a goal if its prefs have changed */
#ifdef DEBUG_CONSISTENCY_CHECK
                print("      This goal's preferences have changed.\n");
#endif
                if (!decision_consistent_with_current_preferences(goal, s)) {
#ifdef DEBUG_CONSISTENCY_CHECK
                    print_with_symbols
                        ("   The current preferences indicate that the decision at [%y] needs to be removed.\n", goal);
#endif
                    /* This doesn;t seem like it should be necessary but evidently it is: see 2.008 */
                    remove_current_decision(s);
                    return FALSE;
                    break;      /* No need to continue once a decision is removed */
                }
            }
        }
#ifdef DEBUG_CONSISTENCY_CHECK
        else {
            printf("   This is a bottom goal with no operator in the slot\n");
        }
#endif
    }

    return TRUE;
}

/* REW: begin 08.20.97 */

bool i_activity_at_goal(Symbol * goal)
{

    /* print_with_symbols("\nLooking for I-activity at goal: %y\n", goal); */

    if (goal->id.ms_i_assertions)
        return TRUE;

    if (goal->id.ms_retractions)
        return TRUE;

    /* print("\nNo instantiation found.  Returning FALSE\n");  */
    return FALSE;
}

/*   Minor Quiescence at GOAL

     This procedure returns TRUE if the current firing type is IE_PRODS and
     there are no i-assertions (or any retractions) ready to fire in the
     current GOAL.  Else it returns FALSE.  */

bool minor_quiescence_at_goal(Symbol * goal)
{

    if ((current_agent(FIRING_TYPE) == IE_PRODS) && (!i_activity_at_goal(goal)))
        /* firing IEs but no more to fire == minor quiescence */
        return TRUE;
    else
        return FALSE;
}

/* ---------------------------------------------------------------------- */
/* Find the highest goal of activity among the current assertions and
 * retractions */

/* We have to start at the top of the goal stack and go down because *any*
 * goal in the goal stack could be active (and we want to highest one).
 * However, we terminate as soon as a goal with assertions or retractions
 * is found.  Propose cares only about ms_i_assertions & retractions
 */

Symbol *highest_active_goal_propose()
{

    Symbol *goal;

    for (goal = current_agent(top_goal); goal; goal = goal->id.lower_goal) {

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        /* Debugging only */
        print("In highest_active_goal_propose:\n");
        if (goal->id.ms_i_assertions)
            print_assertion(goal->id.ms_i_assertions);
        if (goal->id.ms_retractions)
            print_retraction(goal->id.ms_retractions);
#endif

        /* If there are any active productions at this goal, return the goal */
        if ((goal->id.ms_i_assertions) || (goal->id.ms_retractions))
            return goal;
    }

    /* This routine should only be called when !quiescence.  However, there is
       still the possibility that the only active productions are retractions
       that matched in a NIL goal.  If so, then we just return the bottom goal.
       If not, then all possibilities have been exausted and we have encounted
       an unrecoverable error. */

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
    print("WARNING: Returning NIL active goal because only NIL goal retractions are active.");
#endif
    if (current_agent(nil_goal_retractions))
        return NIL;
    {
        char msg[MESSAGE_SIZE];
        strncpy(msg, "\n consistency.c: Error: Unable to find an active goal when not at quiescence.\n", MESSAGE_SIZE);
        msg[MESSAGE_SIZE - 1] = 0;
        abort_with_fatal_error(msg);
    }
    return NIL;                 /* unneeded, but avoids gcc -Wall warning */
}

Symbol *highest_active_goal_apply()
{

    Symbol *goal;

    for (goal = current_agent(top_goal); goal; goal = goal->id.lower_goal) {

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        /* Debugging only */
        print("In highest_active_goal_apply :\n");
        if (goal->id.ms_i_assertions)
            print_assertion(goal->id.ms_i_assertions);
        if (goal->id.ms_o_assertions)
            print_assertion(goal->id.ms_o_assertions);
        if (goal->id.ms_retractions)
            print_retraction(goal->id.ms_retractions);
#endif

        /* If there are any active productions at this goal, return the goal */
        if ((goal->id.ms_i_assertions) || (goal->id.ms_o_assertions)
            || (goal->id.ms_retractions))
            return goal;
    }

    /* This routine should only be called when !quiescence.  However, there is
       still the possibility that the only active productions are retractions
       that matched in a NIL goal.  If so, then we just return the bottom goal.
       If not, then all possibilities have been exausted and we have encounted
       an unrecoverable error. */

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
    print("WARNING: Returning NIL active goal because only NIL goal retractions are active.");
#endif
    if (current_agent(nil_goal_retractions))
        return NIL;
    {
        char msg[MESSAGE_SIZE];
        strncpy(msg, "\nconsistency.c: Error: Unable to find an active goal when not at quiescence.\n", MESSAGE_SIZE);
        msg[MESSAGE_SIZE - 1] = 0;
        abort_with_fatal_error(msg);
    }
    return NIL;                 /* unneeded, but avoids gcc -Wall warning */
}

/* ---------------------------------------------------------------------- */

/* active_production_type_at_goal
   
   Determines type of productions active at some  active level.  If
   IE PRODS are active, this value is returned (regardless of whether there
   are PEs active or not). Note that this procedure will return erroneous
   values if there is no activity at the current level.  It should only be
   called when activity at the active_level has been determined. */

int active_production_type_at_goal(Symbol * goal)
{

    if (i_activity_at_goal(goal))
        return IE_PRODS;
    else
        return PE_PRODS;
}

/* ---------------------------------------------------------------------- */

bool goal_stack_consistent_through_goal(Symbol * goal)
{
    bool test;

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    start_timer(&current_agent(start_gds_tv));
#endif
#endif

#ifdef DEBUG_CONSISTENCY_CHECK
    print("\nStart: CONSISTENCY CHECK at level %d\n", goal->id.level);

    /* Just a bunch of debug stuff for now */
    if (current_agent(highest_goal_whose_context_changed)) {
        print_with_symbols("current_agent(highest_goal_whose_context_changed) = [%y]\n",
                           current_agent(highest_goal_whose_context_changed));
    } else {
        print("Evidently, nothing has changed: not checking slots\n");
    }
#endif

    test = check_context_slot_decisions(goal->id.level);

#ifdef DEBUG_CONSISTENCY_CHECK
    print("\nEnd:   CONSISTENCY CHECK\n");
#endif

#ifdef DETAILED_TIMING_STATS
    stop_timer(&current_agent(start_gds_tv), &current_agent(gds_cpu_time[current_agent(current_phase)]));
#endif

    return test;
}

/* REW: end   08.20.97 */

/* ---------------------------------------------------------------------- */

/* REW: begin 05.05.97 */

void initialize_consistency_calculations_for_new_decision()
{

    Symbol *goal;

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
    print("\nInitialize consistency calculations for new decision.\n");
#endif

    /* No current activity level */
    current_agent(active_level) = 0;
    current_agent(active_goal) = NIL;

    /* Clear any interruption flags on the goals.... */
    for (goal = current_agent(top_goal); goal; goal = goal->id.lower_goal)
        goal->id.saved_firing_type = NO_SAVED_PRODS;
}

/* ---------------------------------------------------------------------- */

   /* determine_highest_active_production_level_in_stack_apply()

      This routine is responsible for implementing the DETERMINE_LEVEL_PHASE.
      In the Waterfall version of Soar, the DETERMINE_LEVEL_PHASE makes the
      determination of what goal level is active in the stack.  Activity
      proceeds from top goal to bottom goal so the active goal is the goal
      highest in the stack with productions waiting to fire.  This procedure
      also recognizes quiescence (no productions active anywhere) and
      mini-quiescence (no more IE_PRODS are waiting to fire in some goal for a
      goal that fired IE_PRODS in the previous elaboration).  Mini-quiescence is
      followed by a consistency check. */

void determine_highest_active_production_level_in_stack_apply()
{

    Symbol *goal;
    int level_change_type, diff;

#ifndef TRACE_CONTEXT_DECISIONS_ONLY
    if (current_agent(sysparams)[TRACE_PHASES_SYSPARAM])
        print("\n--- Application Phase ---\n");
#endif

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
    print("\nDetermining the highest active level in the stack....\n");
#endif

    if (!any_assertions_or_retractions_ready()) {
        /* This is quiescence */
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\n(Full) Quiescence has been reached...going to decision\n");
#endif

        /* Need to determine if this quiescence is also a minor quiescence,
           otherwise, an inconsistent decision could get retained here (because
           the consistency check was never run). (2.008).  Therefore, if
           in the previous preference phase, IE_PRODS fired, then force a 
           consistency check over the entire stack (by checking at the
           bottom goal). */

        if (minor_quiescence_at_goal(current_agent(bottom_goal))) {
            goal_stack_consistent_through_goal(current_agent(bottom_goal));
        }

        /* regardless of the outcome, we go to the decision phase */

        current_agent(current_phase) = OUTPUT_PHASE;
        return;
    }

    /* Not Quiescence */

    /* Check for Max ELABORATIONS EXCEEDED */

    if (current_agent(e_cycles_this_d_cycle) >= (unsigned long) (current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM])) {
        if (current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM])
            print("\nWarning: reached max-elaborations; proceeding to decision phase.");
        current_agent(current_phase) = OUTPUT_PHASE;
        return;
    }

    /* Save the old goal and level (must save level explicitly in case goal is NIL) */
    current_agent(previous_active_goal) = current_agent(active_goal);
    current_agent(previous_active_level) = current_agent(active_level);

    /* Determine the new highest level of activity */
    current_agent(active_goal) = highest_active_goal_apply();
    if (current_agent(active_goal))
        current_agent(active_level) = current_agent(active_goal)->id.level;
    else
        current_agent(active_level) = 0;        /* Necessary for get_next_retraction */

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
    print("\nHighest level of activity is....%d", current_agent(active_level));
    print("\n   Previous level of activity is....%d", current_agent(previous_active_level));
#endif

    if (!current_agent(active_goal))
        /* Only NIL goal retractions */
        level_change_type = NIL_GOAL_RETRACTIONS;
    else if (current_agent(previous_active_level) == 0)
        level_change_type = NEW_DECISION;
    else {
        diff = current_agent(active_level) - current_agent(previous_active_level);
        if (diff == 0)
            level_change_type = SAME_LEVEL;
        else if (diff > 0)
            level_change_type = LOWER_LEVEL;
        else
            level_change_type = HIGHER_LEVEL;
    }

    switch (level_change_type) {
    case NIL_GOAL_RETRACTIONS:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nOnly NIL goal retractions are active");
#endif
        current_agent(FIRING_TYPE) = IE_PRODS;
        current_agent(current_phase) = PREFERENCE_PHASE;
        break;

    case NEW_DECISION:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nThis is a new decision....");
#endif
        current_agent(FIRING_TYPE) = active_production_type_at_goal(current_agent(active_goal));

        /* in APPLY phase, we can test for ONC here, check ms_o_assertions */

        current_agent(current_phase) = PREFERENCE_PHASE;
        break;

    case LOWER_LEVEL:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nThe level is lower than the previous level....");
#endif
        /* Is there a minor quiescence at the previous level? */
        if (minor_quiescence_at_goal(current_agent(previous_active_goal))) {
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
            print("\nMinor quiescence at level %d", current_agent(previous_active_level));
#endif
            if (!goal_stack_consistent_through_goal(current_agent(previous_active_goal))) {
                current_agent(current_phase) = OUTPUT_PHASE;
                break;
            }
        }

        /* else: check if return to interrupted level */

        goal = current_agent(active_goal);

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        if (goal->id.saved_firing_type == IE_PRODS)
            print("\nSaved production type: IE _PRODS");
        if (goal->id.saved_firing_type == PE_PRODS)
            print("\nSaved production type: PE _PRODS");
        if (goal->id.saved_firing_type == NO_SAVED_PRODS)
            print("\nSaved production type: NONE");
#endif

        if (goal->id.saved_firing_type != NO_SAVED_PRODS) {
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
            print("\nRestoring production type from previous processing at this level");
#endif
            current_agent(FIRING_TYPE) = goal->id.saved_firing_type;
            current_agent(current_phase) = DETERMINE_LEVEL_PHASE;
            break;
        }

        /* else: just do a preference phase */
        current_agent(FIRING_TYPE) = active_production_type_at_goal(current_agent(active_goal));
        current_agent(current_phase) = PREFERENCE_PHASE;
        break;

    case SAME_LEVEL:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nThe level is the same as the previous level....");
#endif
        if (minor_quiescence_at_goal(current_agent(active_goal))) {
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
            print("\nMinor quiescence at level %d", current_agent(active_level));
#endif
            if (!goal_stack_consistent_through_goal(current_agent(active_goal))) {
                current_agent(current_phase) = OUTPUT_PHASE;
                break;
            }
        }

        current_agent(FIRING_TYPE) = active_production_type_at_goal(current_agent(active_goal));
        current_agent(current_phase) = PREFERENCE_PHASE;
        break;

    case HIGHER_LEVEL:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nThe level is higher than the previous level....");
#endif

        goal = current_agent(previous_active_goal);
        goal->id.saved_firing_type = current_agent(FIRING_TYPE);

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        if (goal->id.saved_firing_type == IE_PRODS)
            print("\n Saving current firing type as IE_PRODS");
        else if (goal->id.saved_firing_type == PE_PRODS)
            print("\n Saving current firing type as PE_PRODS");
        else if (goal->id.saved_firing_type == NO_SAVED_PRODS)
            print("\n Saving current firing type as NO_SAVED_PRODS");
        else
            print("\n Unknown SAVED firing type???????");
#endif

        /* run consistency check at new active level *before* firing any
           productions there */

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nMinor quiescence at level %d", current_agent(active_level));
#endif
        if (!goal_stack_consistent_through_goal(current_agent(active_goal))) {
            current_agent(current_phase) = OUTPUT_PHASE;
            break;
        }

        /* If the decision is consistent, then just start processing at this level */

        current_agent(FIRING_TYPE) = active_production_type_at_goal(current_agent(active_goal));
        current_agent(current_phase) = PREFERENCE_PHASE;
        break;
    }

}

/* REW: end   05.05.97 */

                          /* KJC: begin 10.04.98 *//* swiped from REW's determine_highest_active... */
/* ---------------------------------------------------------------------- */

   /* determine_highest_active_production_level_in_stack_propose()

      This routine is responsible for implementing the DETERMINE_LEVEL_PHASE
      for the Propose Phase under the new reordering of the Decision Cycle.
      In the Waterfall version of Soar, the DETERMINE_LEVEL_PHASE makes the
      determination of what goal level is active in the stack.  Activity
      proceeds from top goal to bottom goal so the active goal is the goal
      highest in the stack with productions waiting to fire.  This procedure
      also recognizes quiescence (no productions active anywhere) and
      mini-quiescence (no more IE_PRODS are waiting to fire in some goal for a
      goal that fired IE_PRODS in the previous elaboration).  Mini-quiescence is
      followed by a consistency check. */

void determine_highest_active_production_level_in_stack_propose()
{

    Symbol *goal;
    int level_change_type, diff;

#ifndef TRACE_CONTEXT_DECISIONS_ONLY
    if (current_agent(sysparams)[TRACE_PHASES_SYSPARAM])
        print("\n--- Proposal Phase ---\n");
#endif

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
    print("\n(Propose) Determining the highest active level in the stack....\n");
#endif

    /* We are only checking for i_assertions, not o_assertions, since we don't
       want operators to fire in the proposal phase
     */
    if (!(current_agent(ms_retractions) || current_agent(ms_i_assertions))) {
        /*if (minor_quiescence_at_goal(current_agent(bottom_goal))) { */
        /* This is minor quiescence */
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\n Propose Phase Quiescence has been reached...going to decision\n");
#endif

        /* Force a consistency check over the entire stack (by checking at
           the bottom goal). */
        goal_stack_consistent_through_goal(current_agent(bottom_goal));

        /* Decision phase is always next */
        current_agent(current_phase) = DECISION_PHASE;
        return;
    }

    /* Not Quiescence */

    /* Check for Max ELABORATIONS EXCEEDED */

    if (current_agent(e_cycles_this_d_cycle) >= (unsigned long) (current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM])) {
        if (current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM])
            print("\nWarning: reached max-elaborations; proceeding to decision phase.");
        current_agent(current_phase) = DECISION_PHASE;
        return;
    }

    /* not Max Elaborations */

    /* Save the old goal and level (must save level explicitly in case
       goal is NIL) */
    current_agent(previous_active_goal) = current_agent(active_goal);
    current_agent(previous_active_level) = current_agent(active_level);

    /* Determine the new highest level of activity */
    current_agent(active_goal) = highest_active_goal_propose();
    if (current_agent(active_goal))
        current_agent(active_level) = current_agent(active_goal)->id.level;
    else
        current_agent(active_level) = 0;        /* Necessary for get_next_retraction */

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
    print("\nHighest level of activity is....%d", current_agent(active_level));
    print("\n   Previous level of activity is....%d", current_agent(previous_active_level));
#endif

    if (!current_agent(active_goal))
        /* Only NIL goal retractions */
        level_change_type = NIL_GOAL_RETRACTIONS;
    else if (current_agent(previous_active_level) == 0)
        level_change_type = NEW_DECISION;
    else {
        diff = current_agent(active_level) - current_agent(previous_active_level);
        if (diff == 0)
            level_change_type = SAME_LEVEL;
        else if (diff > 0)
            level_change_type = LOWER_LEVEL;
        else
            level_change_type = HIGHER_LEVEL;
    }

    switch (level_change_type) {
    case NIL_GOAL_RETRACTIONS:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nOnly NIL goal retractions are active");
#endif
        current_agent(FIRING_TYPE) = IE_PRODS;
        current_agent(current_phase) = PREFERENCE_PHASE;
        break;

    case NEW_DECISION:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nThis is a new decision....");
#endif
        current_agent(FIRING_TYPE) = IE_PRODS;
        current_agent(current_phase) = PREFERENCE_PHASE;
        break;

    case LOWER_LEVEL:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nThe level is lower than the previous level....");
#endif
        /* There is always a minor quiescence at the previous level
           in the propose phase, so check for consistency. */
        if (!goal_stack_consistent_through_goal(current_agent(previous_active_goal))) {
            current_agent(current_phase) = DECISION_PHASE;
            break;
        }

        /* else: just do a preference phase */
        current_agent(FIRING_TYPE) = IE_PRODS;
        current_agent(current_phase) = PREFERENCE_PHASE;
        break;

    case SAME_LEVEL:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nThe level is the same as the previous level....");
#endif
        current_agent(FIRING_TYPE) = IE_PRODS;
        current_agent(current_phase) = PREFERENCE_PHASE;
        break;

    case HIGHER_LEVEL:
#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nThe level is higher than the previous level....");
#endif

        goal = current_agent(previous_active_goal);
        goal->id.saved_firing_type = current_agent(FIRING_TYPE);

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        if (goal->id.saved_firing_type == IE_PRODS)
            print("\n Saving current firing type as IE_PRODS");
        else if (goal->id.saved_firing_type == PE_PRODS)
            print("\n Saving current firing type as PE_PRODS");
        else if (goal->id.saved_firing_type == NO_SAVED_PRODS)
            print("\n Saving current firing type as NO_SAVED_PRODS");
        else
            print("\n Unknown SAVED firing type???????");
#endif

        /* run consistency check at new active level *before* firing any
           productions there */

#ifdef DEBUG_DETERMINE_LEVEL_PHASE
        print("\nMinor quiescence at level %d", current_agent(active_level));
#endif
        if (!goal_stack_consistent_through_goal(current_agent(active_goal))) {
            current_agent(current_phase) = DECISION_PHASE;
            break;
        }

        /* If the decision is consistent, then just start processing
           at this level */

        current_agent(FIRING_TYPE) = IE_PRODS;
        current_agent(current_phase) = PREFERENCE_PHASE;
        break;
    }

}

/* KJC: end   10.04.98 */
