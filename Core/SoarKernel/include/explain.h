/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  explain.h
 *
 * =======================================================================
 */

#ifndef EXPLAIN_H
#define EXPLAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef signed short goal_stack_level;
typedef struct condition_struct condition;
typedef struct action_struct action;
typedef struct backtrace_struct backtrace_str;
typedef struct cons_struct cons;
typedef struct wme_struct wme;
typedef struct agent_struct agent;
typedef union symbol_union Symbol;

/*
   For each chunk (or justification) take a copy of its conds and actions,
   and the list of productions which were backtraced through in creating it.
   Also keep a list of all of the grounds (WMEs in the supergoal) which were
   tested as the chunk was formed.
*/

typedef struct explain_chunk_struct {
#define EXPLAIN_CHUNK_STRUCT_NAME_BUFFER_SIZE 256
   char name[EXPLAIN_CHUNK_STRUCT_NAME_BUFFER_SIZE];                      /* Name of this chunk/justification */
   condition *conds;                    /* Variablized list of conditions */
   action *actions;                     /* Variablized list of actions */
   struct backtrace_struct *backtrace;  /* List of back traced productions */
   struct explain_chunk_struct *next_chunk; /* Next chunk in the list */
   condition *all_grounds;             /* All conditions which go to LHS -- 
                                          must be in same order as the chunk's 
                                          conditions. */
} explain_chunk_str;
/* AGR 564 ends */

/* RBD added decl's of these routines because they were called from files
   other than explain.c.  I don't know what they do. */
extern void init_explain (agent* thisAgent);
extern void reset_backtrace_list (agent* thisAgent);
condition * copy_cond_list(agent* thisAgent, condition *top_list);
condition *copy_conds_from_list(agent* thisAgent, cons *top_list);
extern void explain_add_temp_to_backtrace_list (agent* thisAgent, backtrace_str *temp, 
    cons *grounds, cons *pots, cons *locals, cons *negateds);
extern void explain_add_temp_to_chunk_list(agent* thisAgent, explain_chunk_str *temp);
extern void free_explain_chunk(agent* thisAgent, explain_chunk_str *chunk);
extern void reset_explain (agent* thisAgent);
extern explain_chunk_str *find_chunk(agent* thisAgent, explain_chunk_str *chunk, char *name);
extern condition *find_ground(agent* thisAgent, explain_chunk_str *chunk, int number);
extern void explain_trace_chunk(agent* thisAgent, explain_chunk_str *chunk);
extern void explain_trace_named_chunk (agent* thisAgent, char *chunk_name);
extern condition *explain_find_cond(condition *target, condition *cond_list);
extern void explain_trace(agent* thisAgent, char *chunk_name, backtrace_str *prod_list, condition *ground);
extern void explain_chunk (agent* thisAgent, char *chunk_name, int cond_number);
extern void explain_cond_list (agent* thisAgent, char *chunk_name);
extern void explain_list_chunks (agent* thisAgent);
extern void explain_full_trace (agent* thisAgent);
/* REW: begin 08.20.97 */

/* Export ms_change structure to entire code in order to include pointers to 
   assertion and retractions lists directly on goals. */

/* BUGBUG ms changes only really need tok (the tok from the p-node),
   not a tok+wme pair.  Need to change the firer for this though. */

/* --- info about a change to the match set --- */
typedef struct ms_change_struct {
  struct ms_change_struct *next;         /* dll for all p nodes */
  struct ms_change_struct *prev;
  struct ms_change_struct *next_of_node; /* dll for just this p node */
  struct ms_change_struct *prev_of_node;
  struct rete_node_struct *p_node;       /* for retractions, this can be NIL
                                            if the p_node has been excised */
  struct token_struct *tok;            /* for assertions only */

  wme *w;                              /* for assertions only */
  struct instantiation_struct *inst;   /* for retractions only */
/* REW: begin 08.20.97 */
  Symbol *goal;
  goal_stack_level level;              /* Level of the match of the assertion or retraction */
  struct ms_change_struct *next_in_level; /* dll for goal level */
  struct ms_change_struct *prev_in_level;
/* REW: end   08.20.97 */
} ms_change;
/* REW: end 08.20.97 */

 /* we really only needs these for interface.c, so maybe just
 * explicitly include them there and get rid of this file... kjc */

/* About 80 lines of stuff deleted.  AGR 564  2-May-94 */

/* KBS commented this out -- redundant with agent variable */
/* extern Bool explain_flag;   Flag for whether we're explaining or not */

/* added code related to explain.cpp back in (above) -ajc (5/1/02) */

extern Bool explain_interface_routine (void);
extern char *help_on_explain[];

#ifdef __cplusplus
}
#endif

#endif
