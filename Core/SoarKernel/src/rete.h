/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* =======================================================================
                                 rete.h

   All_wmes_in_rete is the header for a dll of all the wmes currently
   in the rete.  (This is normally equal to all of WM, except at times
   when WM changes have been buffered but not yet done.)  The wmes
   are linked via their "rete_next" and "rete_prev" fields.
   Num_wmes_in_rete counts how many wmes there are in the rete.

   Init_rete() initializes the rete.  It should be called at startup time.

   Any_assertions_or_retractions_ready() returns TRUE iff there are any
   pending changes to the match set.  This is used to test for quiescence.
   Get_next_assertion() retrieves a pending assertion (returning TRUE) or
   returns FALSE is no more are available.  Get_next_retraction() is
   similar.

   Add_production_to_rete() adds a given production, with a given LHS,
   to the rete.  If "refracted_inst" is non-NIL, it should point to an
   initial instantiation of the production.  This routine returns one
   of NO_REFRACTED_INST, REFRACTED_INST_MATCHED, etc. (see below).
   Excise_production_from_rete() removes the given production from the
   rete, and enqueues all its existing instantiations as pending
   retractions.

   Add_wme_to_rete() and remove_wme_from_rete() inform the rete of changes
   to WM.

   P_node_to_conditions_and_nots() takes a p_node and (optionally) a
   token/wme pair, and reconstructs the (optionally instantiated) LHS
   for the production.  The firer uses this to build the instantiated
   conditions; the printer uses it to reconstruct the LHS for printing.
   Get_symbol_from_rete_loc() takes a token/wme pair and a location
   specification (levels_up/field_num), examines the match (token/wme),
   and returns the symbol at that location.  The firer uses this for
   resolving references in RHS actions to variables bound on the LHS.

   Count_rete_tokens_for_production() returns a count of the number of
   tokens currently in use for the given production.

   Print_partial_match_information(), print_match_set(), and
   print_rete_statistics() do printouts for various interface routines.

   Save_rete_net() and load_rete_net() are used for the fastsave/load
   commands.  They save/load everything to/from the given (already open)
   files.  They return TRUE if successful, FALSE if any error occurred.
======================================================================= */

#ifndef RETE_H
#define RETE_H

#include <stdio.h>	// Needed for FILE token below
#include "kernel.h"


typedef unsigned char byte;
typedef byte wme_trace_type;
typedef byte ms_trace_type;
typedef struct instantiation_struct instantiation;
typedef struct production_struct production;
typedef struct condition_struct condition;
typedef struct action_struct action;
typedef struct wme_struct wme;
typedef struct rete_node_struct rete_node;
typedef struct agent_struct agent;
typedef struct symbol_struct Symbol;

typedef char varnames;

inline varnames * one_var_to_varnames(Symbol * x) { return reinterpret_cast<varnames *>(x); }
inline varnames * var_list_to_varnames(cons * x) { return reinterpret_cast<varnames *>(reinterpret_cast<char *>(x) + 1); }
inline uint64_t varnames_is_var_list(varnames * x) { return reinterpret_cast<uint64_t>(x) & 1; }
inline bool varnames_is_one_var(varnames * x) { return ! varnames_is_var_list(x); }
inline Symbol * varnames_to_one_var(varnames * x) { return reinterpret_cast<Symbol *>(x); }
inline list * varnames_to_var_list(varnames * x) { return reinterpret_cast<list *>(static_cast<char *>(x) - 1); }

typedef struct three_field_varnames_struct {
  varnames *id_varnames;
  varnames *attr_varnames;
  varnames *value_varnames;
} three_field_varnames;

typedef struct node_varnames_struct {
  struct node_varnames_struct *parent;
  union varname_data_union {
    three_field_varnames fields;
    struct node_varnames_struct *bottom_of_subconditions;
  } data;
} node_varnames;

typedef struct token_struct {
  /* --- Note: "parent" is NIL on negative node negrm (local join result)
     tokens, non-NIL on all other tokens including CN and CN_P stuff.
     I put "parent" at offset 0 in the structure, so that upward scans
     are fast (saves doing an extra integer addition in the inner loop) --- */
  struct token_struct *parent;
  union token_a_union {
    struct token_in_hash_table_data_struct {
      struct token_struct *next_in_bucket, *prev_in_bucket; /*hash bucket dll*/
      Symbol *referent; /* referent of the hash test (thing we hashed on) */
    } ht;
    struct token_from_right_memory_of_negative_or_cn_node_struct {
      struct token_struct *next_negrm, *prev_negrm;/*other local join results*/
      struct token_struct *left_token; /* token this is local join result for*/
    } neg;
  } a;
  rete_node *node;
  wme *w;
  struct token_struct *first_child;  /* first of dll of children */
  struct token_struct *next_sibling, *prev_sibling; /* for dll of children */
  struct token_struct *next_of_node, *prev_of_node; /* dll of tokens at node */
  struct token_struct *next_from_wme, *prev_from_wme; /* tree-based remove */
  struct token_struct *negrm_tokens; /* join results: for Neg, CN nodes only */
} token;

extern void init_rete (agent* thisAgent);

extern bool any_assertions_or_retractions_ready (agent* thisAgent);
extern bool postpone_assertion (agent* thisAgent, production **prod, struct token_struct **tok, wme **w);
extern void consume_last_postponed_assertion(agent* thisAgent);
extern void restore_postponed_assertions (agent* thisAgent);
extern bool get_next_retraction (agent* thisAgent, struct instantiation_struct **inst);
/* REW: begin 08.20.97 */
/* Special routine for retractions in removed goals.  See note in rete.cpp */
extern bool get_next_nil_goal_retraction (agent* thisAgent, struct instantiation_struct **inst);
/* REW: end   08.20.97 */

#define NO_REFRACTED_INST 0              /* no refracted inst. was given */
#define REFRACTED_INST_MATCHED 1         /* there was a match for the inst. */
#define REFRACTED_INST_DID_NOT_MATCH 2   /* there was no match for it */
#define DUPLICATE_PRODUCTION 3           /* the prod. was a duplicate */
extern byte add_production_to_rete (agent* thisAgent, production *p, condition *lhs_top,
                                    instantiation *refracted_inst,
                                    bool warn_on_duplicates, bool ignore_rhs = false);
extern void excise_production_from_rete (agent* thisAgent, production *p);

extern void add_wme_to_rete (agent* thisAgent, wme *w);
extern void remove_wme_from_rete (agent* thisAgent, wme *w);

extern void p_node_to_conditions_and_rhs (agent* thisAgent,
                                           struct rete_node_struct *p_node,
                                           struct token_struct *tok,
                                           wme *w,
                                           condition **dest_top_cond,
                                           condition **dest_bottom_cond,
                                           action **dest_rhs,
                                           bool compile_chunk_test_info);
extern Symbol *get_symbol_from_rete_loc (unsigned short levels_up,
                                         byte field_num,
                                         struct token_struct *tok, wme *w);

extern uint64_t count_rete_tokens_for_production (agent* thisAgent, production *prod);
extern void print_partial_match_information (agent* thisAgent, struct rete_node_struct *p_node,
                                             wme_trace_type wtt);
extern void xml_partial_match_information (agent* thisAgent, rete_node *p_node, wme_trace_type wtt) ;

extern void print_match_set (agent* thisAgent, wme_trace_type wtt, ms_trace_type  mst);
extern void xml_match_set (agent* thisAgent, wme_trace_type wtt, ms_trace_type  mst);
extern void get_all_node_count_stats (agent* thisAgent);
extern int get_node_count_statistic (agent* thisAgent, char * node_type_name,
				     char * column_name,
				     uint64_t * result);

extern bool save_rete_net (agent* thisAgent, FILE *dest_file, bool use_rete_net_64);
extern bool load_rete_net (agent* thisAgent, FILE *source_file);

/* ---------------------------------------------------------------------

       Test Type <---> Relational (Rete) Test Type Conversion

   These functions convert from xxx_TEST's (defined in soarkernel.h for various
   kinds of complex_test's) to xxx_RETE_TEST's (defined in rete.cpp for
   the different kinds of Rete tests), and vice-versa.  We might just
   use the same set of constants for both purposes, but we want to be
   able to do bit-twiddling on the RETE_TEST types.

--------------------------------------------------------------------- */

//
// 255 == ERROR_TEST_TYPE.  I use 255 here for brevity.
//
/* --- for the last two (i.e., the relational tests), we add in one of
       the following, to specifiy the kind of relation --- */
#define RELATIONAL_EQUAL_RETE_TEST            0x00
#define RELATIONAL_NOT_EQUAL_RETE_TEST        0x01
#define RELATIONAL_LESS_RETE_TEST             0x02
#define RELATIONAL_GREATER_RETE_TEST          0x03
#define RELATIONAL_LESS_OR_EQUAL_RETE_TEST    0x04
#define RELATIONAL_GREATER_OR_EQUAL_RETE_TEST 0x05
#define RELATIONAL_SAME_TYPE_RETE_TEST        0x06

inline TestType relational_test_type_to_test_type(byte test_type)
{
  /* we don't need ...[equal test] */
  switch (test_type) {
    case RELATIONAL_EQUAL_RETE_TEST:
      return EQUALITY_TEST;
      break;
    case RELATIONAL_NOT_EQUAL_RETE_TEST:
      return NOT_EQUAL_TEST;
      break;
    case RELATIONAL_LESS_RETE_TEST:
      return LESS_TEST;
      break;
    case RELATIONAL_GREATER_RETE_TEST:
      return GREATER_TEST;
      break;
    case RELATIONAL_LESS_OR_EQUAL_RETE_TEST:
      return LESS_OR_EQUAL_TEST;
      break;
    case RELATIONAL_GREATER_OR_EQUAL_RETE_TEST:
      return GREATER_OR_EQUAL_TEST;
      break;
    case RELATIONAL_SAME_TYPE_RETE_TEST:
      return SAME_TYPE_TEST;
      break;
    default:
      break;
  }
  char msg[BUFFER_MSG_SIZE];
  abort_with_fatal_error_noprint("Bad test_type in add_rete_test_to_test!!!\n");
  return EQUALITY_TEST;
}
inline byte test_type_to_relational_test_type(byte test_type)
{
  /* we don't need ...[equal test] */
  switch (test_type) {
    case NOT_EQUAL_TEST:
      return RELATIONAL_NOT_EQUAL_RETE_TEST;
      break;
    case LESS_TEST:
      return RELATIONAL_LESS_RETE_TEST;
      break;
    case GREATER_TEST:
      return RELATIONAL_GREATER_RETE_TEST;
      break;
    case LESS_OR_EQUAL_TEST:
      return RELATIONAL_LESS_OR_EQUAL_RETE_TEST;
      break;
    case GREATER_OR_EQUAL_TEST:
      return RELATIONAL_GREATER_OR_EQUAL_RETE_TEST;
      break;
    case SAME_TYPE_TEST:
      return RELATIONAL_SAME_TYPE_RETE_TEST;
      break;
    default:
      break;
  }
  return 255;
}

#endif
