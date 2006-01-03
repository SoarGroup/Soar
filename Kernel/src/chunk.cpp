#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#include "chunk.h"
#include "symtab.h"
#include "wmem.h"
#include "gdatastructs.h"
#include "kernel.h"
#include "agent.h"
#include "instantiations.h"
#include "production.h"
#include "rhsfun.h"
#include "print.h"
#include "init_soar.h"
#include "prefmem.h"
#include "decide.h"
#include "explain.h"
#include "backtrace.h"
#include "recmem.h"
#include "rete.h"
#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes
#include "gski_event_system_functions.h" // support for triggering XML events

using namespace xmlTraceNames;

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

#ifdef USE_MACROS
#define add_results_if_needed(thisAgent, sym) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      if ( ((sym)->id.level >= thisAgent->results_match_goal_level) && \
           ((sym)->id.tc_num != thisAgent->results_tc_number) ) \
        add_results_for_id(thisAgent, sym); }

#else
inline void add_results_if_needed(agent* thisAgent, Symbol * sym)
{ 
  if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
      if ( ((sym)->id.level >= thisAgent->results_match_goal_level) &&
           ((sym)->id.tc_num != thisAgent->results_tc_number) )
        add_results_for_id(thisAgent, sym);
}
#endif /* USE_MACROS */

extern void add_pref_to_results (agent* thisAgent, preference *pref) {
  preference *p;

  /* --- if an equivalent pref is already a result, don't add this one --- */
  for (p=thisAgent->results; p!=NIL; p=p->next_result) {
    if (p->id!=pref->id) continue;
    if (p->attr!=pref->attr) continue;
    if (p->value!=pref->value) continue;
    if (p->type!=pref->type) continue;
    if (preference_is_unary(pref->type)) return;
    if (p->referent!=pref->referent) continue;
    return;
  }

  /* --- if pref isn't at the right level, find a clone that is --- */
  if (pref->inst->match_goal_level != thisAgent->results_match_goal_level) {
    for (p=pref->next_clone; p!=NIL; p=p->next_clone)
      if (p->inst->match_goal_level == thisAgent->results_match_goal_level) break;
    if (!p)
      for (p=pref->prev_clone; p!=NIL; p=p->prev_clone)
        if (p->inst->match_goal_level == thisAgent->results_match_goal_level) break;
    if (!p) return;  /* if can't find one, it isn't a result */
    pref = p;
  }

  /* --- add this preference to the result list --- */
  pref->next_result = thisAgent->results; 
  thisAgent->results = pref;

  /* --- follow transitive closuse through value, referent links --- */
  add_results_if_needed (thisAgent, pref->value);
  if (preference_is_binary(pref->type))
    add_results_if_needed (thisAgent, pref->referent);
}

void add_results_for_id (agent* thisAgent, Symbol *id) {
  slot *s;
  preference *pref;
  wme *w;

  id->id.tc_num = thisAgent->results_tc_number;

  /* --- scan through all preferences and wmes for all slots for this id --- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    add_results_if_needed (thisAgent, w->value);
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (pref=s->all_preferences; pref!=NIL; pref=pref->all_of_slot_next)
      add_pref_to_results(thisAgent, pref);
    for (w=s->wmes; w!=NIL; w=w->next)
      add_results_if_needed (thisAgent, w->value);
  } /* end of for slots loop */
  /* --- now scan through extra prefs and look for any with this id --- */
  for (pref=thisAgent->extra_result_prefs_from_instantiation; pref!=NIL;
       pref=pref->inst_next) {
    if (pref->id==id) add_pref_to_results(thisAgent, pref);
  }
}

preference *get_results_for_instantiation (agent* thisAgent, instantiation *inst) {
  preference *pref;

  thisAgent->results = NIL;
  thisAgent->results_match_goal_level = inst->match_goal_level;
  thisAgent->results_tc_number = get_new_tc_number(thisAgent);
  thisAgent->extra_result_prefs_from_instantiation = inst->preferences_generated;
  for (pref=inst->preferences_generated; pref!=NIL; pref=pref->inst_next)
    if ( (pref->id->id.level < thisAgent->results_match_goal_level) &&
         (pref->id->id.tc_num != thisAgent->results_tc_number) ) {
      add_pref_to_results(thisAgent, pref);
    }
  return thisAgent->results;
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

void variablize_symbol (agent* thisAgent, Symbol **sym) {
  char prefix[2];
  Symbol *var;
  
  if ((*sym)->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) return;
  if (! thisAgent->variablize_this_chunk) return;
  
  if ((*sym)->id.tc_num == thisAgent->variablization_tc) {
    /* --- it's already been variablized, so use the existing variable --- */
    var = (*sym)->id.variablization;
    symbol_remove_ref (thisAgent, *sym);
    *sym = var;
    symbol_add_ref (var);
    return;
  }

  /* --- need to create a new variable --- */
  (*sym)->id.tc_num = thisAgent->variablization_tc;
  prefix[0] = tolower((*sym)->id.name_letter);
  prefix[1] = 0;
  var = generate_new_variable (thisAgent, prefix);
  (*sym)->id.variablization = var;
  symbol_remove_ref (thisAgent, *sym);
  *sym = var;
}

void variablize_test (agent* thisAgent, test *t) {
  cons *c;
  complex_test *ct;

  if (test_is_blank_test(*t)) return;
  if (test_is_blank_or_equality_test(*t)) {
    variablize_symbol (thisAgent, (Symbol **) t);
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
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      variablize_test (thisAgent, (test *)(&(c->first)));
    return;
  default:  /* relational tests other than equality */
    variablize_symbol (thisAgent, &(ct->data.referent));
    return;
  }
}

void variablize_condition_list (agent* thisAgent, condition *cond) {
  for (; cond!=NIL; cond=cond->next) {
    switch (cond->type) {
    case POSITIVE_CONDITION:
    case NEGATIVE_CONDITION:
      variablize_test (thisAgent, &(cond->data.tests.id_test));
      variablize_test (thisAgent, &(cond->data.tests.attr_test));
      variablize_test (thisAgent, &(cond->data.tests.value_test));
      break;
    case CONJUNCTIVE_NEGATION_CONDITION:
      variablize_condition_list (thisAgent, cond->data.ncc.top);
      break;
    }
  }
}

action *copy_and_variablize_result_list (agent* thisAgent, preference *pref) {
  action *a;
  Symbol *temp;
  
  if (!pref) return NIL;
  allocate_with_pool (thisAgent, &thisAgent->action_pool, &a);
  a->type = MAKE_ACTION;

  temp = pref->id;
  symbol_add_ref (temp);
  variablize_symbol (thisAgent, &temp);
  a->id = symbol_to_rhs_value (temp);

  temp = pref->attr;
  symbol_add_ref (temp);
  variablize_symbol (thisAgent, &temp);
  a->attr = symbol_to_rhs_value (temp);

  temp = pref->value;
  symbol_add_ref (temp);
  variablize_symbol (thisAgent, &temp);
  a->value = symbol_to_rhs_value (temp);

  a->preference_type = pref->type;

  if (preference_is_binary(pref->type)) {
    temp = pref->referent;
    symbol_add_ref (temp);
    variablize_symbol (thisAgent, &temp);
    a->referent = symbol_to_rhs_value (temp);
  }
  
  a->next = copy_and_variablize_result_list (thisAgent, pref->next_result);
  return a;  
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

void init_chunk_cond_set (chunk_cond_set *set) {
  int i;
  
  set->all = NIL;
  for (i=0; i<CHUNK_COND_HASH_TABLE_SIZE; i++) set->table[i] = NIL;
}

chunk_cond *make_chunk_cond_for_condition (agent* thisAgent, condition *cond) {
  chunk_cond *cc;
  unsigned long remainder, hv;
  
  allocate_with_pool (thisAgent, &thisAgent->chunk_cond_pool, &cc);
  cc->cond = cond;
  cc->hash_value = hash_condition (thisAgent, cond);
  remainder = cc->hash_value;
  hv = 0;
  while (remainder) {
    hv ^= (remainder &
           masks_for_n_low_order_bits[LOG_2_CHUNK_COND_HASH_TABLE_SIZE]);
    remainder = remainder >> LOG_2_CHUNK_COND_HASH_TABLE_SIZE;
  }
  cc->compressed_hash_value = hv;
  return cc;
}

Bool add_to_chunk_cond_set (agent* thisAgent, chunk_cond_set *set, chunk_cond *new_cc) {
  chunk_cond *old;
  
  for (old=set->table[new_cc->compressed_hash_value]; old!=NIL;
       old=old->next_in_bucket)
    if (old->hash_value==new_cc->hash_value)
      if (conditions_are_equal (old->cond, new_cc->cond))
        break;
  if (old) {
    /* --- the new condition was already in the set; so don't add it --- */
    free_with_pool (&thisAgent->chunk_cond_pool, new_cc);
    return FALSE;
  }
  /* --- add new_cc to the table --- */
  insert_at_head_of_dll (set->all, new_cc, next, prev);
  insert_at_head_of_dll (set->table[new_cc->compressed_hash_value], new_cc,
                         next_in_bucket, prev_in_bucket);
  return TRUE;
}

void remove_from_chunk_cond_set (chunk_cond_set *set, chunk_cond *cc) {
  remove_from_dll (set->all, cc, next, prev);
  remove_from_dll (set->table[cc->compressed_hash_value],
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

void build_chunk_conds_for_grounds_and_add_negateds (agent* thisAgent, 
													 chunk_cond **dest_top,
                                                     chunk_cond **dest_bottom,
                                                     tc_number tc_to_use) {
  cons *c;
  condition *ground;
  chunk_cond *cc, *first_cc, *prev_cc;

  first_cc = NIL; /* unnecessary, but gcc -Wall warns without it */

  /* --- build instantiated conds for grounds and setup their TC --- */
  prev_cc = NIL;
  while (thisAgent->grounds) {
    c = thisAgent->grounds;
    thisAgent->grounds = thisAgent->grounds->rest;
    ground = static_cast<condition_struct *>(c->first);
    free_cons (thisAgent, c);
    /* --- make the instantiated condition --- */
    allocate_with_pool (thisAgent, &thisAgent->chunk_cond_pool, &cc);
    cc->cond = ground;
    cc->instantiated_cond = copy_condition (thisAgent, cc->cond);
    cc->variablized_cond = copy_condition (thisAgent, cc->cond);
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
    add_cond_to_tc (thisAgent, ground, tc_to_use, NIL, NIL);
  }

  /* --- scan through negated conditions and check which ones are connected
     to the grounds --- */
  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
    print_string (thisAgent, "\n\n*** Adding Grounded Negated Conditions ***\n");
  
  while (thisAgent->negated_set.all) {
    cc = thisAgent->negated_set.all;
    remove_from_chunk_cond_set (&thisAgent->negated_set, cc);
    if (cond_is_in_tc (thisAgent, cc->cond, tc_to_use)) {
      /* --- negated cond is in the TC, so add it to the grounds --- */
      if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
        print_string (thisAgent, "\n-->Moving to grounds: ");
        print_condition (thisAgent, cc->cond);
      }
      cc->instantiated_cond = copy_condition (thisAgent, cc->cond);
      cc->variablized_cond = copy_condition (thisAgent, cc->cond);
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
      free_with_pool (&thisAgent->chunk_cond_pool, cc);
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

not_struct *get_nots_for_instantiated_conditions (agent* thisAgent, 
										   list *instantiations_with_nots,
                                           tc_number tc_of_grounds) {
  cons *c;
  instantiation *inst;
  not_struct *n1, *n2, *new_not, *collected_nots;

  /* --- collect nots for which both id's are marked --- */
  collected_nots = NIL;
  while (instantiations_with_nots) {
    c = instantiations_with_nots;
    instantiations_with_nots = c->rest;
    inst = static_cast<instantiation_struct *>(c->first);
    free_cons (thisAgent, c);
    for (n1=inst->nots; n1 != NIL; n1=n1->next) {
      /* --- Are both id's marked? If no, goto next loop iteration --- */
      if (n1->s1->id.tc_num != tc_of_grounds) continue;
      if (n1->s2->id.tc_num != tc_of_grounds) continue;
      /* --- If the pair already in collected_nots, goto next iteration --- */
      for (n2=collected_nots; n2!=NIL; n2=n2->next) {
        if ((n2->s1 == n1->s1) && (n2->s2 == n1->s2)) break;
        if ((n2->s1 == n1->s2) && (n2->s2 == n1->s1)) break;
      }
      if (n2) continue;
      /* --- Add the pair to collected_nots --- */
      allocate_with_pool (thisAgent, &thisAgent->not_pool, &new_not);
      new_not->next = collected_nots;
      collected_nots = new_not;
      new_not->s1 = n1->s1;
      symbol_add_ref (new_not->s1);
      new_not->s2 = n1->s2;
      symbol_add_ref (new_not->s2);
    } /* end of for n1 */
  } /* end of while instantiations_with_nots */

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

void variablize_nots_and_insert_into_conditions (agent* thisAgent, 
												 not_struct *nots,
                                                 condition *conds) {
  not_struct *n;
  Symbol *var1, *var2;
  test t;
  complex_test *ct;
  condition *c;
  Bool added_it;

  /* --- don't bother Not-ifying justifications --- */
  if (! thisAgent->variablize_this_chunk) return;
  
  for (n=nots; n!=NIL; n=n->next) {
    var1 = n->s1->id.variablization;
    var2 = n->s2->id.variablization;
    /* --- find where var1 is bound, and add "<> var2" to that test --- */
    allocate_with_pool (thisAgent, &thisAgent->complex_test_pool, &ct);
    t = make_test_from_complex_test (ct);
    ct->type = NOT_EQUAL_TEST;
    ct->data.referent = var2;
    symbol_add_ref (var2);
    added_it = FALSE;
    for (c=conds; c!=NIL; c=c->next) {
      if (c->type != POSITIVE_CONDITION) continue;
      if (test_includes_equality_test_for_symbol (c->data.tests.id_test,
                                                  var1)) {
        add_new_test_to_test (thisAgent, &(c->data.tests.id_test), t);
        added_it = TRUE;
        break;
      }
      if (test_includes_equality_test_for_symbol (c->data.tests.attr_test,
                                                  var1)) {
        add_new_test_to_test (thisAgent, &(c->data.tests.attr_test), t);
        added_it = TRUE;
        break;
      }
      if (test_includes_equality_test_for_symbol (c->data.tests.value_test,
                                                  var1)) {
        add_new_test_to_test (thisAgent, &(c->data.tests.value_test), t);
        added_it = TRUE;
        break;
      }
    }
    if (!added_it) {
      char msg[BUFFER_MSG_SIZE];
      strncpy (msg,"chunk.c: Internal error: couldn't add Not test to chunk\n", BUFFER_MSG_SIZE);
	  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */

      abort_with_fatal_error(thisAgent, msg);
    }
  } /* end of for n=nots */
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

void add_goal_or_impasse_tests (agent* thisAgent, chunk_cond *all_ccs) {
  chunk_cond *cc;
  tc_number tc;   /* mark each id as we add a test for it, so we don't add
                     a test for the same id in two different places */
  Symbol *id;
  test t;
  complex_test *ct;

  tc = get_new_tc_number(thisAgent);
  for (cc=all_ccs; cc!=NIL; cc=cc->next) {
    if (cc->instantiated_cond->type!=POSITIVE_CONDITION) continue;
    id = referent_of_equality_test (cc->instantiated_cond->data.tests.id_test);
    if ( (id->id.isa_goal || id->id.isa_impasse) &&
         (id->id.tc_num != tc) ) {
      allocate_with_pool (thisAgent, &thisAgent->complex_test_pool, &ct);
      ct->type = (id->id.isa_goal) ? GOAL_ID_TEST : IMPASSE_ID_TEST;
      t = make_test_from_complex_test(ct);
      add_new_test_to_test (thisAgent, &(cc->variablized_cond->data.tests.id_test), t);
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

void reorder_instantiated_conditions (chunk_cond *top_cc,
                                      condition **dest_inst_top,
                                      condition **dest_inst_bottom) {
  chunk_cond *cc;

  /* --- Step 1:  swap prev pointers out of variablized conds into chunk_conds,
     and swap pointer to the corresponding instantiated conds into the
     variablized conds' prev pointers --- */
  for (cc=top_cc; cc!=NIL; cc=cc->next) {
    cc->saved_prev_pointer_of_variablized_cond = cc->variablized_cond->prev;
    cc->variablized_cond->prev = cc->instantiated_cond;
  }

  /* --- Step 2:  do the reordering of the instantiated conds --- */
  for (cc=top_cc; cc!=NIL; cc=cc->next) {
    if (cc->variablized_cond->next) {
      cc->instantiated_cond->next = cc->variablized_cond->next->prev;
    } else {
      cc->instantiated_cond->next = NIL;
      *dest_inst_bottom = cc->instantiated_cond;
    }
    
    if (cc->saved_prev_pointer_of_variablized_cond) {
      cc->instantiated_cond->prev =
        cc->saved_prev_pointer_of_variablized_cond->prev;
    } else {
      cc->instantiated_cond->prev = NIL;
      *dest_inst_top = cc->instantiated_cond;
    }
  }

  /* --- Step 3:  restore the prev pointers on variablized conds --- */
  for (cc=top_cc; cc!=NIL; cc=cc->next) {
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

void make_clones_of_results (agent* thisAgent, preference *results, 
							 instantiation *chunk_inst) {
  preference *p, *result_p;

  chunk_inst->preferences_generated = NIL;
  for (result_p=results; result_p!=NIL; result_p=result_p->next_result) {
    /* --- copy the preference --- */
    p = make_preference (thisAgent, result_p->type, result_p->id, result_p->attr,
                         result_p->value, result_p->referent);
    symbol_add_ref (p->id);
    symbol_add_ref (p->attr);
    symbol_add_ref (p->value);
    if (preference_is_binary(p->type))
      symbol_add_ref (p->referent);
    /* --- put it onto the list for chunk_inst --- */
    p->inst = chunk_inst;
    insert_at_head_of_dll (chunk_inst->preferences_generated, p,
                           inst_next, inst_prev);
    /* --- insert it into the list of clones for this preference --- */
    p->next_clone = result_p;
    p->prev_clone = result_p->prev_clone;
    result_p->prev_clone = p;
    if (p->prev_clone) p->prev_clone->next_clone = p;
  }
}

/* kjh (B14) begin */
Symbol *find_goal_at_goal_stack_level(agent* thisAgent, goal_stack_level level) {
 Symbol *g;
 
 for (g = thisAgent->top_goal; g != NIL; g = g->id.lower_goal)
   if (g->id.level == level)
     return(g);
 return(NIL);
}

Symbol *find_impasse_wme_value(Symbol *id, Symbol *attr) {
  wme *w;

  for (w = id->id.impasse_wmes; w != NIL; w = w->next)
    if (w->attr == attr) return w->value;
  return NIL;
}

Symbol *generate_chunk_name_sym_constant (agent* thisAgent, instantiation *inst) {
#define BUFFER_GEN_CHUNK_NAME_SIZE 512
  char name[BUFFER_GEN_CHUNK_NAME_SIZE];
#define BUFFER_IMPASS_NAME_SIZE 32
  char impass_name[BUFFER_IMPASS_NAME_SIZE];
  Symbol *generated_name;  
  Symbol *goal;
  byte impasse_type;
  preference *p;
  goal_stack_level lowest_result_level;

  if (!thisAgent->sysparams[USE_LONG_CHUNK_NAMES])
    return(generate_new_sym_constant (thisAgent, thisAgent->chunk_name_prefix, 
                                      &thisAgent->chunk_count));

  lowest_result_level = thisAgent->top_goal->id.level;
  for (p=inst->preferences_generated; p!=NIL; p=p->inst_next)
    if (p->id->id.level > lowest_result_level)
      lowest_result_level = p->id->id.level;
  
  goal = find_goal_at_goal_stack_level(thisAgent, lowest_result_level);

  if (goal) {
    impasse_type = type_of_existing_impasse (thisAgent, goal);
    switch (impasse_type) {
      case NONE_IMPASSE_TYPE:
	#ifdef DEBUG_CHUNK_NAMES
        print ("Warning: impasse_type is NONE_IMPASSE_TYPE during chunk creation.\n");
		GenerateWarningXML(thisAgent, "Warning: impasse_type is NONE_IMPASSE_TYPE during chunk creation.");
	#endif
        strncpy(impass_name,"unknownimpasse",BUFFER_IMPASS_NAME_SIZE);
        break;
      case CONSTRAINT_FAILURE_IMPASSE_TYPE:
        strncpy(impass_name,"cfailure",BUFFER_IMPASS_NAME_SIZE);
        break;
      case CONFLICT_IMPASSE_TYPE:
        strncpy(impass_name,"conflict",BUFFER_IMPASS_NAME_SIZE);
        break;
      case TIE_IMPASSE_TYPE:
        strncpy(impass_name,"tie",BUFFER_IMPASS_NAME_SIZE);
        break;
      case NO_CHANGE_IMPASSE_TYPE:
        {           
          Symbol *sym;
          
          if ((sym = find_impasse_wme_value(goal->id.lower_goal,thisAgent->attribute_symbol)) == NIL) {
            #ifdef DEBUG_CHUNK_NAMES
		    // TODO: generate warning XML: I think we need to get a string for "do_print_for_identifier" and append it
		    // but this seems low priority since it's not even included in a normal build
            print ("Warning: Failed to find ^attribute impasse wme.\n");
            do_print_for_identifier(goal->id.lower_goal, 1, 0);
            #endif
            strncpy(impass_name,"unknownimpasse",BUFFER_IMPASS_NAME_SIZE);
          } else if (sym == thisAgent->operator_symbol) {
            strncpy(impass_name,"opnochange",BUFFER_IMPASS_NAME_SIZE);
          } else if (sym == thisAgent->state_symbol) {
            strncpy(impass_name,"snochange",BUFFER_IMPASS_NAME_SIZE);
          } else {
	    #ifdef DEBUG_CHUNK_NAMES
            print ("Warning: ^attribute impasse wme has unexpected value.\n");
			GenerateWarningXML(thisAgent, "Warning: ^attribute impasse wme has unexpected value.");
            #endif
            strncpy(impass_name,"unknownimpasse",BUFFER_IMPASS_NAME_SIZE);
          }
        }
        break;
      default:
	#ifdef DEBUG_CHUNK_NAMES
	    // TODO: generate warning XML: I think we need to create a buffer and snprintf the impasse_type into it (since it's a byte)
		// but this seems low priority since it's not even included in a normal build
        print ("Warning: encountered unknown impasse_type: %d.\n", impasse_type);

	#endif
        strncpy(impass_name,"unknownimpasse",BUFFER_IMPASS_NAME_SIZE);
        break;
    }
  } else {
    #ifdef DEBUG_CHUNK_NAMES
    print ("Warning: Failed to determine impasse type.\n");
	GenerateWarningXML(thisAgent, "Warning: Failed to determine impasse type.");
    #endif
    strncpy(impass_name,"unknownimpasse",BUFFER_IMPASS_NAME_SIZE);
  }
  impass_name[BUFFER_IMPASS_NAME_SIZE - 1] = 0; /* ensure null termination */

  snprintf (name, BUFFER_GEN_CHUNK_NAME_SIZE, "%s-%lu*d%lu*%s*%lu", 
          thisAgent->chunk_name_prefix,
          thisAgent->chunk_count++,
          thisAgent->d_cycle_count,
          impass_name,
          thisAgent->chunks_this_d_cycle
          );
  name[BUFFER_GEN_CHUNK_NAME_SIZE - 1] = 0; /* ensure null termination */

 
  /* Any user who named a production like this deserves to be burned, but we'll have mercy: */
  if (find_sym_constant (thisAgent, name)) {
    unsigned long collision_count;
    
    collision_count = 1;
    print (thisAgent, "Warning: generated chunk name already exists.  Will find unique name.\n");
	GenerateWarningXML(thisAgent, "Warning: generated chunk name already exists.  Will find unique name.");
    do {
      snprintf (name, BUFFER_GEN_CHUNK_NAME_SIZE, "%s-%lu*d%lu*%s*%lu*%lu", 
              thisAgent->chunk_name_prefix,
              thisAgent->chunk_count++,
              thisAgent->d_cycle_count,
              impass_name,
              thisAgent->chunks_this_d_cycle,
              collision_count++
              );
      name[BUFFER_GEN_CHUNK_NAME_SIZE - 1] = 0; /* ensure null termination */

    } while (find_sym_constant (thisAgent, name));
  }

  generated_name = make_sym_constant(thisAgent, name);
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


void chunk_instantiation (agent* thisAgent, 
						  instantiation *inst, 
						  Bool allow_variablization) {
  Bool making_topmost_chunk = FALSE;   /* RCHONG:  10.11 */
  goal_stack_level grounds_level;
  preference *results, *pref;
  action *rhs;
  production *prod;
  instantiation *chunk_inst;
  Symbol *prod_name;
  byte prod_type;
  Bool print_name, print_prod;
  byte rete_addition_result;
  condition *lhs_top, *lhs_bottom;
  not_struct *nots;
  chunk_cond *top_cc, *bottom_cc;
  
  explain_chunk_str temp_explain_chunk;
  
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  struct timeval saved_start_tv;
#endif
#endif
  
  /* --- if it only matched an attribute impasse, don't chunk --- */
  if (! inst->match_goal) return; 
  
  /* --- if no preference is above the match goal level, exit --- */
  for (pref=inst->preferences_generated; pref!=NIL; pref=pref->inst_next) {
	  if (pref->id->id.level < inst->match_goal_level)
		  break;
  }
  if (! pref) return;
  
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  start_timer (&saved_start_tv);
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
  
  
  if (thisAgent->operand2_mode == TRUE) {
	  
      if (thisAgent->sysparams[LEARNING_ON_SYSPARAM] == TRUE) {
		  if (pref->id->id.level < (inst->match_goal_level - 1)) {
			  making_topmost_chunk     = FALSE;
			  allow_variablization     = FALSE;
			  inst->okay_to_variablize = FALSE;
			  
              if (thisAgent->soar_verbose_flag == TRUE) {
				  printf("\n   in chunk_instantiation: making justification only");
                  GenerateVerboseXML(thisAgent, "in chunk_instantiation: making justification only");
              }
		  }
		  
		  else {
			  making_topmost_chunk     = TRUE;
			  allow_variablization     = (thisAgent->sysparams[LEARNING_ON_SYSPARAM] != 0);
			  inst->okay_to_variablize = (byte) thisAgent->sysparams[LEARNING_ON_SYSPARAM];
			  
              if (thisAgent->soar_verbose_flag == TRUE) {
				  printf("\n   in chunk_instantiation: resetting allow_variablization to %s", ((allow_variablization) ? "TRUE" : "FALSE"));
                  if(allow_variablization) {
                      GenerateVerboseXML(thisAgent, "in chunk_instantiation: resetting allow_variablization to TRUE");
                  } else {
                      GenerateVerboseXML(thisAgent, "in chunk_instantiation: resetting allow_variablization to FALSE");
                  }
              }
		  }
      }
	  
      else {
		  making_topmost_chunk = TRUE;
      }
	  
  }
  
  /* REW: end   09.15.96 */
  
  
  results = get_results_for_instantiation (thisAgent, inst);
  if (!results) goto chunking_done;
  
  /* --- update flags on goal stack for bottom-up chunking --- */
  { Symbol *g;
  for (g=inst->match_goal->id.higher_goal;
  g && g->id.allow_bottom_up_chunks;
  g=g->id.higher_goal)
      g->id.allow_bottom_up_chunks = FALSE;
  }
  
  grounds_level = inst->match_goal_level - 1;
  
  thisAgent->backtrace_number++; 
  if (thisAgent->backtrace_number==0) 
	  thisAgent->backtrace_number=1;
  thisAgent->grounds_tc++; 
  if (thisAgent->grounds_tc==0) 
	  thisAgent->grounds_tc=1;
  thisAgent->potentials_tc++; 
  if (thisAgent->potentials_tc==0) 
	  thisAgent->potentials_tc=1;
  thisAgent->locals_tc++; 
  if (thisAgent->locals_tc==0) 
	  thisAgent->locals_tc=1;
  thisAgent->grounds = NIL;
  thisAgent->positive_potentials = NIL;
  thisAgent->locals = NIL;
  thisAgent->instantiations_with_nots = NIL;
  
  if (allow_variablization &&
      (! thisAgent->sysparams[LEARNING_ALL_GOALS_SYSPARAM]))
	  allow_variablization = inst->match_goal->id.allow_bottom_up_chunks;
  
	  /* DJP : Need to initialize chunk_free_flag to be FALSE, as default before
  looking for problem spaces and setting the chunk_free_flag below  */
  
  thisAgent->chunk_free_flag = FALSE ;
  /* DJP : Noticed this also isn't set if no ps_name */
  thisAgent->chunky_flag = FALSE ;   
  
  
  /* --- check whether ps name is in chunk_free_problem_spaces --- */
  if (allow_variablization) {
  /* KJC new implementation of learn cmd:  old SPECIFY ==> ONLY,
  * old ON ==> EXCEPT,  now ON is just ON always
  * checking if state is chunky or chunk-free...
	  */
	  if ( thisAgent->sysparams[LEARNING_EXCEPT_SYSPARAM]) {
		  if (member_of_list(inst->match_goal,thisAgent->chunk_free_problem_spaces))
		  {
			  allow_variablization = FALSE; 
			  thisAgent->chunk_free_flag = TRUE;
		  }
	  }
	  else if (thisAgent->sysparams[LEARNING_ONLY_SYSPARAM]) {
		  if (member_of_list(inst->match_goal,thisAgent->chunky_problem_spaces))
		  {
			  allow_variablization = TRUE; 
			  thisAgent->chunky_flag = TRUE;
		  }
		  else {
			  allow_variablization = FALSE; 
			  thisAgent->chunky_flag = FALSE;
		  }
	  }
  }   /* end KJC mods */
  
  thisAgent->variablize_this_chunk = allow_variablization;
  
  /* Start a new structure for this potential chunk */
  
  if (thisAgent->sysparams[EXPLAIN_SYSPARAM]) {
	  temp_explain_chunk.conds       = NULL;
	  temp_explain_chunk.actions     = NULL;
	  temp_explain_chunk.backtrace   = NULL;
	  temp_explain_chunk.name[0]     = '\0';
	  temp_explain_chunk.all_grounds = NIL;
	  temp_explain_chunk.next_chunk  = NULL;
	  reset_backtrace_list(thisAgent);
  }
  
  /* --- backtrace through the instantiation that produced each result --- */
  for (pref=results; pref!=NIL; pref=pref->next_result) {
	  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
		  print_string (thisAgent, "\nFor result preference ");
          gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagBacktraceResult);
		  print_preference (thisAgent, pref);
		  print_string (thisAgent, " ");
	  }
	  backtrace_through_instantiation (thisAgent, pref->inst, grounds_level, NULL, 0);
      
      if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagBacktraceResult);
      }
  }
  
  thisAgent->quiescence_t_flag = FALSE;
  
  while (TRUE) {
	  trace_locals (thisAgent, grounds_level);
	  trace_grounded_potentials (thisAgent);
	  if (! trace_ungrounded_potentials (thisAgent, grounds_level)) break;
  }

  free_list (thisAgent, thisAgent->positive_potentials);
  
  /* --- backtracing done; collect the grounds into the chunk --- */
  { tc_number tc_for_grounds;
  tc_for_grounds = get_new_tc_number(thisAgent);
  build_chunk_conds_for_grounds_and_add_negateds (thisAgent, &top_cc, &bottom_cc,
	  tc_for_grounds);
  nots = get_nots_for_instantiated_conditions (thisAgent, thisAgent->instantiations_with_nots,
	  tc_for_grounds);
  }
  
  /* --- get symbol for name of new chunk or justification --- */
  if (thisAgent->variablize_this_chunk) {
	  /* kjh (B14) begin */
	  thisAgent->chunks_this_d_cycle++;
	  prod_name = generate_chunk_name_sym_constant(thisAgent, inst); 
	  /* kjh (B14) end */
	  
	  /*   old way of generating chunk names ...
	  prod_name = generate_new_sym_constant ("chunk-",&thisAgent->chunk_count);
	  thisAgent->chunks_this_d_cycle)++;
	  */
	  
	  prod_type = CHUNK_PRODUCTION_TYPE;
	  print_name = (thisAgent->sysparams[TRACE_CHUNK_NAMES_SYSPARAM] != 0);
	  print_prod = (thisAgent->sysparams[TRACE_CHUNKS_SYSPARAM] != 0);
  } else {
	  prod_name = generate_new_sym_constant (thisAgent, "justification-",
		  &thisAgent->justification_count);
	  prod_type = JUSTIFICATION_PRODUCTION_TYPE;
	  print_name = (thisAgent->sysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] != 0);
	  print_prod = (thisAgent->sysparams[TRACE_JUSTIFICATIONS_SYSPARAM] != 0);
  }
  
  /* AGR 617/634 begin */
  if (print_name) {
	  if (get_printer_output_column(thisAgent)!=1) print (thisAgent, "\n");
	  print_with_symbols (thisAgent, "Building %y", prod_name);

	  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagLearning);
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagProduction);
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kProduction_Name, symbol_to_string(thisAgent, prod_name, true, 0, 0));
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagProduction);
	  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagLearning);
  }
  /* AGR 617/634 end */
  
  /* --- if there aren't any grounds, exit --- */
  if (! top_cc) {
	  if (thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) {
		  print_string (thisAgent, " Warning: chunk has no grounds, ignoring it.");
		  GenerateWarningXML(thisAgent, "Warning: chunk has no grounds, ignoring it.");
	  }
	  goto chunking_done;
  }
  
  /* MVP 6-8-94 */
  if (thisAgent->chunks_this_d_cycle >
      (unsigned long) thisAgent->sysparams[MAX_CHUNKS_SYSPARAM]) {
		  if (thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) {
		  print (thisAgent, "\nWarning: reached max-chunks! Halting system.");
		  GenerateWarningXML(thisAgent, "Warning: reached max-chunks! Halting system.");
		  }
	  thisAgent->max_chunks_reached = TRUE;
	  goto chunking_done;
  }
  
  /* --- variablize it --- */
  lhs_top = top_cc->variablized_cond;
  lhs_bottom = bottom_cc->variablized_cond;
  reset_variable_generator (thisAgent, lhs_top, NIL);
  thisAgent->variablization_tc = get_new_tc_number(thisAgent);
  variablize_condition_list (thisAgent, lhs_top);
  variablize_nots_and_insert_into_conditions (thisAgent, nots, lhs_top);
  rhs = copy_and_variablize_result_list (thisAgent, results);
  
  /* --- add goal/impasse tests to it --- */
  add_goal_or_impasse_tests (thisAgent, top_cc);
  
  /* --- reorder lhs and make the production --- */
  
  prod = make_production (thisAgent, prod_type, prod_name, &lhs_top, &lhs_bottom, &rhs,
	  FALSE);
  
  if (!prod) {
	  print (thisAgent, "\nUnable to reorder this chunk:\n  ");
	  print_condition_list (thisAgent, lhs_top, 2, FALSE);
	  print (thisAgent, "\n  -->\n   ");
	  print_action_list (thisAgent, rhs, 3, FALSE);
	  print (thisAgent, "\n\n(Ignoring this chunk.  Weird things could happen from now on...)\n");
	  goto chunking_done; /* this leaks memory but who cares */
  }
  
  { condition *inst_lhs_top, *inst_lhs_bottom;
  
  reorder_instantiated_conditions (top_cc, &inst_lhs_top, &inst_lhs_bottom);
  
  /* Record the list of grounds in the order they will appear in the chunk. */
  if (thisAgent->sysparams[EXPLAIN_SYSPARAM])
      temp_explain_chunk.all_grounds = inst_lhs_top;   /* Not a copy yet */
  
  allocate_with_pool (thisAgent, &thisAgent->instantiation_pool, &chunk_inst);
  chunk_inst->prod = prod;
  chunk_inst->top_of_instantiated_conditions = inst_lhs_top;
  chunk_inst->bottom_of_instantiated_conditions = inst_lhs_bottom;
  chunk_inst->nots = nots;
  
  chunk_inst->GDS_evaluated_already = FALSE;  /* REW:  09.15.96 */
  
  
											  /* If:
											  - you don't want to variablize this chunk, and
											  - the reason is ONLY that it's chunk free, and
											  - NOT that it's also quiescence, then
											  it's okay to variablize through this instantiation later.
  */
  
  /* AGR MVL1 begin */
  if (! thisAgent->sysparams[LEARNING_ONLY_SYSPARAM]) {
      if ((! thisAgent->variablize_this_chunk) 
		  && (thisAgent->chunk_free_flag) 
		  && (! thisAgent->quiescence_t_flag))
		  chunk_inst->okay_to_variablize = TRUE;
      else
		  chunk_inst->okay_to_variablize = thisAgent->variablize_this_chunk;
  }
  else {
      if ((! thisAgent->variablize_this_chunk) 
		  && (! thisAgent->chunky_flag) 
		  && (! thisAgent->quiescence_t_flag))
		  chunk_inst->okay_to_variablize = TRUE;
      else
		  chunk_inst->okay_to_variablize = thisAgent->variablize_this_chunk;
  }
  /* AGR MVL1 end */
  
  chunk_inst->in_ms = TRUE;  /* set TRUE for now, we'll find out later... */
  make_clones_of_results (thisAgent, results, chunk_inst);
  fill_in_new_instantiation_stuff (thisAgent, chunk_inst, TRUE);
  
  }  /* matches { condition *inst_lhs_top, *inst_lhs_bottom ...  */
  
	 /* RBD 4/6/95 Need to copy cond's and actions for the production here,
     otherwise some of the variables might get deallocated by the call to
  add_production_to_rete() when it throws away chunk variable names. */
  if (thisAgent->sysparams[EXPLAIN_SYSPARAM]) {
	  condition *new_top, *new_bottom;
	  copy_condition_list (thisAgent, lhs_top, &new_top, &new_bottom);
	  temp_explain_chunk.conds = new_top;
	  temp_explain_chunk.actions = copy_and_variablize_result_list (thisAgent, results);
  }
  
  rete_addition_result = add_production_to_rete (thisAgent, prod, lhs_top, chunk_inst,
	  print_name);
  
	  /* If didn't immediately excise the chunk from the rete net   
  then record the temporary structure in the list of explained chunks. */
  
  if (thisAgent->sysparams[EXPLAIN_SYSPARAM])
	  if ((rete_addition_result != DUPLICATE_PRODUCTION) &&
		  ((prod_type != JUSTIFICATION_PRODUCTION_TYPE) ||
		  (rete_addition_result != REFRACTED_INST_DID_NOT_MATCH) )) {
		  strncpy(temp_explain_chunk.name,prod_name->sc.name, EXPLAIN_CHUNK_STRUCT_NAME_BUFFER_SIZE);
		  temp_explain_chunk.name[EXPLAIN_CHUNK_STRUCT_NAME_BUFFER_SIZE - 1] = 0;
		  explain_add_temp_to_chunk_list (thisAgent, &temp_explain_chunk);
	  } else {
		  /* RBD 4/6/95 if excised the chunk, discard previously-copied stuff */
		  deallocate_condition_list (thisAgent, temp_explain_chunk.conds);
		  deallocate_action_list (thisAgent, temp_explain_chunk.actions);
	  }
	  
	  /* --- deallocate chunks conds and variablized conditions --- */
	  deallocate_condition_list (thisAgent, lhs_top);
	  { chunk_cond *cc;
	  while (top_cc) {
		  cc = top_cc;
		  top_cc = cc->next;
		  free_with_pool (&thisAgent->chunk_cond_pool, cc);
	  }
	  }
	  
	  if (print_prod && (rete_addition_result!=DUPLICATE_PRODUCTION)) {
		  print_string (thisAgent, "\n");
          gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagLearning);
		  print_production (thisAgent, prod, FALSE);
          gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagLearning);
	  }
	  
	  if (rete_addition_result==DUPLICATE_PRODUCTION) {
		  excise_production (thisAgent, prod, FALSE);
	  } else if ((prod_type==JUSTIFICATION_PRODUCTION_TYPE) &&
		  (rete_addition_result==REFRACTED_INST_DID_NOT_MATCH)) {
		  excise_production (thisAgent, prod, FALSE);
	  }
	  
	  if (rete_addition_result!=REFRACTED_INST_MATCHED) {
		  /* --- it didn't match, or it was a duplicate production --- */
		  /* --- tell the firer it didn't match, so it'll only assert the
		  o-supported preferences --- */
		  chunk_inst->in_ms = FALSE;
	  }
	  
	  /* --- assert the preferences --- */
	  chunk_inst->next = thisAgent->newly_created_instantiations;
	  thisAgent->newly_created_instantiations = chunk_inst;
	  
	  
	  
	  /* MVP 6-8-94 */
	  if (!thisAgent->max_chunks_reached)
		  chunk_instantiation (thisAgent, chunk_inst, thisAgent->variablize_this_chunk);
	  
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
	  stop_timer (&saved_start_tv,
		  &thisAgent->chunking_cpu_time[thisAgent->(current_phase)]);
#endif
#endif
	  
	  return;
	  
chunking_done: {}
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
			   stop_timer (&saved_start_tv, &thisAgent->chunking_cpu_time[thisAgent->current_phase]);
#endif
#endif
}

/* --------------------------------------------------------------------

  Chunker Initialization
  
	Init_chunker() is called at startup time to do initialization here.
	-------------------------------------------------------------------- */
	
	void init_chunker (agent* thisAgent) {
		init_memory_pool (thisAgent, &thisAgent->chunk_cond_pool, sizeof(chunk_cond), "chunk condition");
		init_chunk_cond_set (&thisAgent->negated_set);
	}
	
