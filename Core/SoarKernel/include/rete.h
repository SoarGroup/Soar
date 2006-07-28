/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#ifdef __cplusplus
extern "C"
{
#endif

struct not_struct;

typedef char Bool;
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
typedef struct kernel_struct Kernel;
typedef union symbol_union Symbol;

extern void init_rete (agent* thisAgent);

extern Bool save_rete_net (agent* thisAgent, FILE *dest_file);
extern Bool any_assertions_or_retractions_ready (agent* thisAgent);
extern Bool get_next_assertion (agent* thisAgent, production **prod,
                                struct token_struct **tok,
                                wme **w);
extern Bool get_next_retraction (agent* thisAgent, struct instantiation_struct **inst);
/* REW: begin 08.20.97 */
/* Special routine for retractions in removed goals.  See note in rete.cpp */
extern Bool get_next_nil_goal_retraction (agent* thisAgent, struct instantiation_struct **inst);
/* REW: end   08.20.97 */

#define NO_REFRACTED_INST 0              /* no refracted inst. was given */
#define REFRACTED_INST_MATCHED 1         /* there was a match for the inst. */
#define REFRACTED_INST_DID_NOT_MATCH 2   /* there was no match for it */
#define DUPLICATE_PRODUCTION 3           /* the prod. was a duplicate */
extern byte add_production_to_rete (agent* thisAgent, production *p, condition *lhs_top,
                                    instantiation *refracted_inst,
                                    Bool warn_on_duplicates);
extern void excise_production_from_rete (agent* thisAgent, production *p);

extern void add_wme_to_rete (agent* thisAgent, wme *w);
extern void remove_wme_from_rete (agent* thisAgent, wme *w);

extern void p_node_to_conditions_and_nots (agent* thisAgent, 
                                           struct rete_node_struct *p_node,
                                           struct token_struct *tok,
                                           wme *w,
                                           condition **dest_top_cond,
                                           condition **dest_bottom_cond,
                                           not_struct **dest_nots,
                                           action **dest_rhs);
extern Symbol *get_symbol_from_rete_loc (unsigned short levels_up,
                                         byte field_num,
                                         struct token_struct *tok, wme *w);

extern unsigned long count_rete_tokens_for_production (agent* thisAgent, production *prod);
extern void print_partial_match_information (agent* thisAgent, struct rete_node_struct *p_node,
                                             wme_trace_type wtt);

extern void print_match_set (agent* thisAgent, wme_trace_type wtt, ms_trace_type  mst);
extern void xml_match_set (agent* thisAgent, wme_trace_type wtt, ms_trace_type  mst);
extern void print_rete_statistics (agent* thisAgent);
extern void get_all_node_count_stats (agent* thisAgent);
extern void print_node_count_statistics (agent* thisAgent);
extern int get_node_count_statistic (agent* thisAgent, char * node_type_name, 
				     char * column_name, 
				     unsigned long * result);

extern Bool save_rete_net (agent* thisAgent, FILE *dest_file);
extern Bool load_rete_net (Kernel* thisKernel, agent* thisAgent, FILE *source_file);

#ifdef __cplusplus
}
#endif

#endif
