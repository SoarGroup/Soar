#include "rete.h"

#include "agent.h"
#include "callback.h"
#include "condition.h"
#include "decide.h"
#include "dprint.h"
#include "ebc.h"
#include "episodic_memory.h"
#include "instantiation.h"
#include "lexer.h"
#include "mem.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "reinforcement_learning.h"
#include "rhs_functions.h"
#include "rhs.h"
#include "run_soar.h"
#include "semantic_memory.h"
#include "smem_settings.h"
#include "slot.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "test.h"
#include "working_memory.h"
#include "xml.h"

#include <assert.h>
#include <sstream>
#include <stdlib.h>

/*************************************************************************
 *
 *  file:  rete.cpp
 *
 * =======================================================================
 *
 * All_wmes_in_rete is the header for a dll of all the wmes currently
 * in the rete.  (This is normally equal to all of WM, except at times
 * when WM changes have been buffered but not yet done.)  The wmes
 * are linked via their "rete_next" and "rete_prev" fields.
 * Num_wmes_in_rete counts how many wmes there are in the rete.
 *
 * Init_rete() initializes the rete.  It should be called at startup time.
 *
 * Any_assertions_or_retractions_ready() returns true iff there are any
 * pending changes to the match set.  This is used to test for quiescence.
 * Get_next_assertion() retrieves a pending assertion (returning true) or
 * returns false is no more are available.  Get_next_retraction() is
 * similar.
 *
 * Add_production_to_rete() adds a given production, with a given LHS,
 * to the rete.  If "refracted_inst" is non-NIL, it should point to an
 * initial instantiation of the production.  This routine returns one
 * of NO_REFRACTED_INST, REFRACTED_INST_MATCHED, etc. (see below).
 * Excise_production_from_rete() removes the given production from the
 * rete, and enqueues all its existing instantiations as pending
 * retractions.
 *
 * Add_wme_to_rete() and remove_wme_from_rete() inform the rete of changes
 * to WM.
 *
 * P_node_to_conditions_and_nots() takes a p_node and (optionally) a
 * token/wme pair, and reconstructs the (optionally instantiated) LHS
 * for the production.  The firer uses this to build the instantiated
 * conditions; the printer uses it to reconstruct the LHS for printing.
 * Get_symbol_from_rete_loc() takes a token/wme pair and a location
 * specification (levels_up/field_num), examines the match (token/wme),
 * and returns the symbol at that location.  The firer uses this for
 * resolving references in RHS actions to variables bound on the LHS.
 *
 * Count_rete_tokens_for_production() returns a count of the number of
 * tokens currently in use for the given production.
 *
 * Print_partial_match_information(), print_match_set(), and
 * print_rete_statistics() do printouts for various interface routines.
 *
 * Save_rete_net() and load_rete_net() are used for the fastsave/load
 * commands.  They save/load everything to/from the given (already open)
 * files.  They return true if successful, false if any error occurred.
 *
 * =======================================================================
 */

/* ======================================================================

                      Rete Net Routines

   TABLE OF CONTENTS (each part is labeled "SECTION" in the code)

    1:  Rete Net Structures and Declarations
    2:  Match Set Changes
    3:  Alpha Portion of the Rete Net
    4:  Beta Net Initialization and Primitive Construction Routines
    5:  Beta Net Primitive Destruction Routines
    6:  Variable Bindings and Locations
    7:  Varnames and Node_Varnames
    8:  Building the Rete Net:  Condition-To-Node Converstion
    9:  Production Addition and Excising
   10:  Building Conditions (instantiated or not) from the Rete Net
   11:  Rete Test Evaluation Routines
   12:  Beta Node Interpreter Routines: Mem, Pos, and MP Nodes
   13:  Beta Node Interpreter Routines: Negative Nodes
   14:  Beta Node Interpreter Routines: CN and CN_PARTNER Nodes
   15:  Beta Node Interpreter Routines: Production Nodes
   16:  Beta Node Interpreter Routines: Tree-Based Removal
   17:  Fast, Compact Save/Reload of the Whole Rete Net
   18:  Statistics and User Interface Utilities
   19:  Rete Initialization

====================================================================== */

/* ----------- handle inter-switch dependencies ----------- */

/* --- TOKEN_SHARING_STATS requires SHARING_FACTORS --- */
#ifdef TOKEN_SHARING_STATS
#ifndef SHARING_FACTORS
#define SHARING_FACTORS
#endif
#endif

/* --- Calculate DO_ACTIVATION_STATS_ON_REMOVALS --- */
#ifdef NULL_ACTIVATION_STATS
#ifndef DO_ACTIVATION_STATS_ON_REMOVALS
#define DO_ACTIVATION_STATS_ON_REMOVALS
#endif
#endif

using namespace soar_TraceNames;

/* **********************************************************************

   SECTION 1:  Rete Net Structures and Declarations

********************************************************************** */

/* ----------------------------------------------------------------------

       Structures and Declarations:  Beta Portion of the Rete Net

---------------------------------------------------------------------- */
inline bool var_locations_equal(var_location v1, var_location v2)
{
    return (((v1).levels_up == (v2).levels_up) && ((v1).field_num == (v2).field_num));
}

/* --- extract field (id/attr/value) from wme --- */
/* WARNING: this relies on the id/attr/value fields being consecutive in
   the wme structure */
/*#define field_from_wme(wme,field_num) \
  ( (&((wme)->id))[(field_num)] )*/

/* The semantics of this function is the same as
 * inline Symbol * field_from_wme(wme * _wme, byte field_num) {
 *   switch (field_num) {
 *     case 0:
 *       return _wme->id;
 *     case 1:
 *       return _wme->attr;
 *     case 2:
 *       return _wme->value;
 *   }
 * }
 */
inline Symbol* field_from_wme(wme* _wme, byte field_num)
{
    return ((&((_wme)->id))[(field_num)]);
}

/*
#define bnode_is_hashed(x)   ((x) & 0x01)
#define bnode_is_memory(x)   ((x) & 0x02)
#define bnode_is_positive(x) ((x) & 0x04)
#define bnode_is_negative(x) ((x) & 0x08)
#define bnode_is_posneg(x)   ((x) & 0x0C)
#define bnode_is_bottom_of_split_mp(x) ((x) & 0x10)
#define real_parent_node(x) ( bnode_is_bottom_of_split_mp((x)->node_type) ? (x)->parent->parent : (x)->parent )
*/

inline byte bnode_is_hashed(byte x)
{
    return ((x) & 0x01);
}
inline byte bnode_is_memory(byte x)
{
    return ((x) & 0x02);
}
inline byte bnode_is_positive(byte x)
{
    return ((x) & 0x04);
}
inline byte bnode_is_negative(byte x)
{
    return ((x) & 0x08);
}
inline byte bnode_is_posneg(byte x)
{
    return ((x) & 0x0C);
}
inline byte bnode_is_bottom_of_split_mp(byte x)
{
    return ((x) & 0x10);
}

/* This function cannot be defined before struct rete_node_struct */
inline rete_node* real_parent_node(rete_node* x);

/*
 * Initialize the list with all empty strings.
 */
const char* bnode_type_names[256] =
{
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
};

/* Now this function can safely be defined. */
inline rete_node* real_parent_node(rete_node* x)
{
    return (bnode_is_bottom_of_split_mp((x)->node_type) ? (x)->parent->parent : (x)->parent);
}

/* ----------------------------------------------------------------------

             Structures and Declarations:  Right Unlinking

---------------------------------------------------------------------- */

/* Note: a node is right unlinked iff the low-order bit of
   node->b.posneg.next_from_alpha_mem is 1 */

/*#define node_is_right_unlinked(node) \
  (((uint64_t)((node)->b.posneg.next_from_alpha_mem)) & 1)*/
inline uint64_t node_is_right_unlinked(rete_node* node)
{
    return reinterpret_cast<uint64_t>(node->b.posneg.next_from_alpha_mem) & 1;
}

/*#define mark_node_as_right_unlinked(node) { \
  (node)->b.posneg.next_from_alpha_mem = static_cast<rete_node_struct *>((void *)1); }*/
inline void mark_node_as_right_unlinked(rete_node* node)
{
    node->b.posneg.next_from_alpha_mem = reinterpret_cast<rete_node_struct*>(1);
}

//#define relink_to_right_mem(node) {
//  rete_node *rtrm_ancestor, *rtrm_prev;
//  /* find first ancestor that's linked */
//  rtrm_ancestor = (node)->b.posneg.nearest_ancestor_with_same_am;
//  while (rtrm_ancestor && node_is_right_unlinked(rtrm_ancestor))
//    rtrm_ancestor = rtrm_ancestor->b.posneg.nearest_ancestor_with_same_am;
//  if (rtrm_ancestor) {
//    /* insert just before that ancestor */
//    rtrm_prev = rtrm_ancestor->b.posneg.prev_from_alpha_mem;
//    (node)->b.posneg.next_from_alpha_mem = rtrm_ancestor;
//    (node)->b.posneg.prev_from_alpha_mem = rtrm_prev;
//    rtrm_ancestor->b.posneg.prev_from_alpha_mem = (node);
//    if (rtrm_prev) rtrm_prev->b.posneg.next_from_alpha_mem = (node);
//   else (node)->b.posneg.alpha_mem_->beta_nodes = (node);
//  } else {
//   /* no such ancestor, insert at tail of list */
//  rtrm_prev = (node)->b.posneg.alpha_mem_->last_beta_node;
// (node)->b.posneg.next_from_alpha_mem = NIL;
//   (node)->b.posneg.prev_from_alpha_mem = rtrm_prev;
//   (node)->b.posneg.alpha_mem_->last_beta_node = (node);
//   if (rtrm_prev) rtrm_prev->b.posneg.next_from_alpha_mem = (node);
//   else (node)->b.posneg.alpha_mem_->beta_nodes = (node);
// } }
inline void relink_to_right_mem(rete_node* node)
{
    rete_node* rtrm_ancestor, *rtrm_prev;
    /* find first ancestor that's linked */
    rtrm_ancestor = (node)->b.posneg.nearest_ancestor_with_same_am;
    while (rtrm_ancestor && node_is_right_unlinked(rtrm_ancestor))
    {
        rtrm_ancestor = rtrm_ancestor->b.posneg.nearest_ancestor_with_same_am;
    }
    if (rtrm_ancestor)
    {
        /* insert just before that ancestor */
        rtrm_prev = rtrm_ancestor->b.posneg.prev_from_alpha_mem;
        (node)->b.posneg.next_from_alpha_mem = rtrm_ancestor;
        (node)->b.posneg.prev_from_alpha_mem = rtrm_prev;
        rtrm_ancestor->b.posneg.prev_from_alpha_mem = (node);
        if (rtrm_prev)
        {
            rtrm_prev->b.posneg.next_from_alpha_mem = (node);
        }
        else
        {
            (node)->b.posneg.alpha_mem_->beta_nodes = (node);
        }
    }
    else
    {
        /* no such ancestor, insert at tail of list */
        rtrm_prev = (node)->b.posneg.alpha_mem_->last_beta_node;
        (node)->b.posneg.next_from_alpha_mem = NIL;
        (node)->b.posneg.prev_from_alpha_mem = rtrm_prev;
        (node)->b.posneg.alpha_mem_->last_beta_node = (node);
        if (rtrm_prev)
        {
            rtrm_prev->b.posneg.next_from_alpha_mem = (node);
        }
        else
        {
            (node)->b.posneg.alpha_mem_->beta_nodes = (node);
        }
    }
}

/* This macro cannot be easily converted to an inline function.
   Some additional changes are required.
*/
#define unlink_from_right_mem(node) { \
        if ((node)->b.posneg.next_from_alpha_mem == NIL) \
            (node)->b.posneg.alpha_mem_->last_beta_node = \
                    (node)->b.posneg.prev_from_alpha_mem; \
        remove_from_dll ((node)->b.posneg.alpha_mem_->beta_nodes, (node), \
                         b.posneg.next_from_alpha_mem, \
                         b.posneg.prev_from_alpha_mem); \
        mark_node_as_right_unlinked (node); }

/* ----------------------------------------------------------------------

             Structures and Declarations:  Left Unlinking

---------------------------------------------------------------------- */

/* Note: an unmerged positive node is left unlinked iff the low-order bit of
   node->a.pos.next_from_beta_mem is 1 */

/*#define node_is_left_unlinked(node) \
  (((uint64_t)((node)->a.pos.next_from_beta_mem)) & 1)*/
inline uint64_t node_is_left_unlinked(rete_node* node)
{
    return reinterpret_cast<uint64_t>(node->a.pos.next_from_beta_mem) & 1;
}

/*#define mark_node_as_left_unlinked(node) { \
  (node)->a.pos.next_from_beta_mem = static_cast<rete_node_struct *>((void *)1); }*/
inline void mark_node_as_left_unlinked(rete_node* node)
{
    node->a.pos.next_from_beta_mem = reinterpret_cast<rete_node_struct*>(1);
}

/* This macro cannot be easily converted to an inline function.
   Some additional changes are required.
*/
#define relink_to_left_mem(node) { \
        insert_at_head_of_dll ((node)->parent->b.mem.first_linked_child, (node), \
                               a.pos.next_from_beta_mem, \
                               a.pos.prev_from_beta_mem); }

/* This macro cannot be easily converted to an inline function.
   Some additional changes are required.
*/
#define unlink_from_left_mem(node) { \
        remove_from_dll ((node)->parent->b.mem.first_linked_child, (node), \
                         a.pos.next_from_beta_mem, \
                         a.pos.prev_from_beta_mem); \
        mark_node_as_left_unlinked(node); }

/* Note: for merged nodes, we still mark them as left-unlinked, just for
   uniformity.  This probably makes little difference in efficiency. */

/*
#define make_mp_bnode_left_unlinked(node) {(node)->a.np.is_left_unlinked = 1;}
#define make_mp_bnode_left_linked(node) {(node)->a.np.is_left_unlinked = 0;}
#define mp_bnode_is_left_unlinked(node) ((node)->a.np.is_left_unlinked)
*/

inline void make_mp_bnode_left_unlinked(rete_node* node)
{
    (node)->a.np.is_left_unlinked = 1;
}

inline void make_mp_bnode_left_linked(rete_node* node)
{
    (node)->a.np.is_left_unlinked = 0;
}

inline unsigned mp_bnode_is_left_unlinked(rete_node* node)
{
    return ((node)->a.np.is_left_unlinked);
}

/* ----------------------------------------------------------------------

                 Structures and Declarations:  Tokens

---------------------------------------------------------------------- */

/*#define new_left_token(New,current_node,parent_tok,parent_wme) { \
  (New)->node = (current_node); \
  insert_at_head_of_dll ((current_node)->a.np.tokens, (New), \
                         next_of_node, prev_of_node); \
  (New)->first_child = NIL; \
  (New)->parent = (parent_tok); \
  insert_at_head_of_dll ((parent_tok)->first_child, (New), \
                         next_sibling, prev_sibling); \
  (New)->w = (parent_wme); \
  if (parent_wme) insert_at_head_of_dll ((parent_wme)->tokens, (New), \
                                         next_from_wme, prev_from_wme); }*/
inline void new_left_token(token* New, rete_node* current_node,
                           token* parent_tok, wme* parent_wme)
{
    (New)->node = (current_node);
    insert_at_head_of_dll((current_node)->a.np.tokens, (New),
                          next_of_node, prev_of_node);
    (New)->first_child = NIL;
    (New)->parent = (parent_tok);
    insert_at_head_of_dll((parent_tok)->first_child, (New),
                          next_sibling, prev_sibling);
    (New)->w = (parent_wme);
    if (parent_wme) insert_at_head_of_dll((parent_wme)->tokens, (New),
                                              next_from_wme, prev_from_wme);

}

/* Note: (most) tokens are stored in hash table thisAgent->left_ht */

/* ----------------------------------------------------------------------

            Structures and Declarations:  Memory Hash Tables

   Tokens and alpha memory entries (right memory's) as stored in two
   global hash tables.  Unlike most hash tables in Soar, these two tables
   are not dynamically resized -- their size is fixed at compile-time.
---------------------------------------------------------------------- */

/* --- Hash table sizes (actual sizes are powers of 2) --- */
#define LOG2_LEFT_HT_SIZE 14
#define LOG2_RIGHT_HT_SIZE 14

#define LEFT_HT_SIZE (1 << LOG2_LEFT_HT_SIZE)
#define RIGHT_HT_SIZE (1 << LOG2_RIGHT_HT_SIZE)

#define LEFT_HT_MASK (LEFT_HT_SIZE - 1)
#define RIGHT_HT_MASK (RIGHT_HT_SIZE - 1)


/* --- Given the hash value (hv), get contents of bucket header cell ---
#define left_ht_bucket(hv) \
  (* ( ((token **) thisAgent->left_ht) + ((hv) & LEFT_HT_MASK)))
#define right_ht_bucket(hv) \
  (* ( ((right_mem **) thisAgent->right_ht) + ((hv) & RIGHT_HT_MASK)))
*/

/* The return value is modified by the calling function,
   hence the call by reference, */
inline token*& left_ht_bucket(agent* thisAgent, uint32_t hv)
{
    return * (reinterpret_cast<token**>(thisAgent->left_ht) + (hv & LEFT_HT_MASK));
}

inline right_mem* right_ht_bucket(agent* thisAgent, uint32_t hv)
{
    return * (reinterpret_cast<right_mem**>(thisAgent->right_ht) + (hv & RIGHT_HT_MASK));
}

/*#define insert_token_into_left_ht(tok,hv) { \
  token **header_zy37; \
  header_zy37 = ((token **) thisAgent->left_ht) + ((hv) & LEFT_HT_MASK); \
  insert_at_head_of_dll (*header_zy37, (tok), \
                         a.ht.next_in_bucket, a.ht.prev_in_bucket); }*/
inline void insert_token_into_left_ht(agent* thisAgent, token* tok, uint32_t hv)
{
    token** header_zy37;
    header_zy37 = reinterpret_cast<token**>(thisAgent->left_ht) + (hv & LEFT_HT_MASK);
    insert_at_head_of_dll(*header_zy37, tok,
                          a.ht.next_in_bucket, a.ht.prev_in_bucket);
}

/*#define remove_token_from_left_ht(tok,hv) { \
  fast_remove_from_dll (left_ht_bucket(hv), tok, token, \
                        a.ht.next_in_bucket, a.ht.prev_in_bucket); }*/
inline void remove_token_from_left_ht(agent* thisAgent, token* tok, uint32_t hv)
{
    fast_remove_from_dll(left_ht_bucket(thisAgent, hv), tok, token,
                         a.ht.next_in_bucket, a.ht.prev_in_bucket);
}

/* ----------------------------------------------------------------------

       Structures and Declarations:  Beta Net Interpreter Routines

---------------------------------------------------------------------- */

void (*(left_addition_routines[256]))(agent* thisAgent, rete_node* node, token* tok, wme* w) =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
void (*(right_addition_routines[256]))(agent* thisAgent, rete_node* node, wme* w) =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


void remove_token_and_subtree(agent* thisAgent, token* tok);

/* ----------------------------------------------------------------------

             Structures and Declarations:  Debugging Stuff

   These get invoked at the entry and exit points of all node activation
   procedures.  Good place to put debugging checks.
---------------------------------------------------------------------- */

/* Since these do nothing, I will not convert them to inline functions
   for the time being. -ajc (5/6/02 */
#define activation_entry_sanity_check() {}
#define activation_exit_sanity_check() {}

/* ----------------------------------------------------------------------

         Structures and Declarations:  Null Activation Statistics

   Counts the number of null and non-null left activations.  Note that
   this only tallies activations of join nodes for positive conditions;
   negative nodes and CN stuff is ignored.
---------------------------------------------------------------------- */

#ifdef NULL_ACTIVATION_STATS

void null_activation_stats_for_right_activation(rete_node* node, rete_node* node_to_ignore_for_null_activation_stats)
{
    if (node == node_to_ignore_for_activation_stats)
    {
        return;
    }
    switch (node->node_type)
    {
        case POSITIVE_BNODE:
        case UNHASHED_POSITIVE_BNODE:
            thisAgent->num_right_activations++;
            if (! node->parent->a.np.tokens)
            {
                thisAgent->num_null_right_activations++;
            }
            break;
        case MP_BNODE:
        case UNHASHED_MP_BNODE:
            thisAgent->num_right_activations++;
            if (! node->a.np.tokens)
            {
                thisAgent->num_null_right_activations++;
            }
            break;
    }
}

void null_activation_stats_for_left_activation(rete_node* node)
{
    switch (node->node_type)
    {
        case POSITIVE_BNODE:
        case UNHASHED_POSITIVE_BNODE:
            thisAgent->num_left_activations++;
            if (node->b.posneg.alpha_mem_->right_mems == NIL)
            {
                thisAgent->num_null_left_activations++;
            }
            break;
        case MP_BNODE:
        case UNHASHED_MP_BNODE:
            if (mp_bnode_is_left_unlinked(node))
            {
                return;
            }
            thisAgent->num_left_activations++;
            if (node->b.posneg.alpha_mem_->right_mems == NIL)
            {
                thisAgent->num_null_left_activations++;
            }
            break;
    }
}

void print_null_activation_stats()
{
    Output_Manager::Get_OM().print_sf("\nActivations: %u right (%u null), %u left (%u null)\n",
          thisAgent->num_right_activations,
          thisAgent->num_null_right_activations,
          thisAgent->num_left_activations,
          thisAgent->num_null_left_activations);
}

#else

/* Since these do nothing, I will not convert them to inline functions
   for the time being. -ajc (5/6/02 */
#define null_activation_stats_for_right_activation(node) {}
#define null_activation_stats_for_left_activation(node) {}
#define print_null_activation_stats() {}

#endif

/* ----------------------------------------------------------------------

             Structures and Declarations:  Sharing Factors

   Sharing factors are computed/updated using two simple rules:
     (1)  Any time we add a new production to the net, when we get all
     done and have created the p-node, etc., we increment the sharing
     factor on every node the production uses.
     (2) Any time we make a brand new node, we initialize its sharing
     factor to 0.  (This will get incremented shortly thereafter, due
     to rule #1.)

   Note that there are fancy ways to compute/update sharing factors,
   not requiring extra scanning-up-the-net all the time as rule 1 does.
   I went with the ablve way to keep the code small and simple.
---------------------------------------------------------------------- */

#ifdef SHARING_FACTORS

//#define init_sharing_stats_for_new_node(node) { (node)->sharing_factor = 0; }
inline void init_sharing_stats_for_new_node(node* node)
{
    (node)->sharing_factor = 0;
}

/*#define set_sharing_factor(node,sf) { \
  int64_t ssf_237; \
  ssf_237 = (sf) - ((node)->sharing_factor); \
  (node)->sharing_factor = (sf); \
  thisAgent->rete_node_counts_if_no_sharing[(node)->node_type]+=ssf_237; }*/
inline void set_sharing_factor(rete_node* node, uint64_t sf)
{
    int64_t ssf_237;
    ssf_237 = (sf) - ((node)->sharing_factor);
    (node)->sharing_factor = (sf);
    thisAgent->rete_node_counts_if_no_sharing[(node)->node_type] += ssf_237;
}

/* Scans from "node" up to the top node, adds "delta" to sharing factors. */
void adjust_sharing_factors_from_here_to_top(rete_node* node, int delta)
{
    while (node != NIL)
    {
        thisAgent->rete_node_counts_if_no_sharing[node->node_type] += delta;
        node->sharing_factor += delta;
        if (node->node_type == CN_BNODE)
        {
            node = node->b.cn.partner;
        }
        else
        {
            node = node->parent;
        }
    }
}

#else

/* Since these do nothing, I will not convert them to inline functions
   for the time being. -ajc (5/6/02) */
#define init_sharing_stats_for_new_node(node) {}
#define set_sharing_factor(node,sf) {}
#define adjust_sharing_factors_from_here_to_top(node,delta) {}

#endif

/* ----------------------------------------------------------------------

           Structures and Declarations:  (Extra) Rete Statistics

---------------------------------------------------------------------- */

#ifdef TOKEN_SHARING_STATS

/* gets real sharing factor -- converts "0" (temporary sharing factor on
   newly created nodes while we're adding a production to the net) to 1 */
/*#define real_sharing_factor(node) \
  ((node)->sharing_factor ? (node)->sharing_factor : 1)*/
inline uint64_t real_sharing_factor(rete_node* node)
{
    return ((node)->sharing_factor ? (node)->sharing_factor : 1);
}

/*#define token_added(node) { \
  thisAgent->token_additions++; \
  thisAgent->token_additions_without_sharing += real_sharing_factor(node);}*/
inline void token_added(rete_node* node)
{
    thisAgent->token_additions++;
    thisAgent->token_additions_without_sharing += real_sharing_factor(node);
}

#else

#define token_added(node) {}

#endif

/* --- Invoked on every right activation; add=true means right addition --- */
/* NOT invoked on removals unless DO_ACTIVATION_STATS_ON_REMOVALS is set */
/*#define right_node_activation(node,add) { \
  null_activation_stats_for_right_activation(node); }*/
inline void right_node_activation(rete_node* node, bool/*add*/)
{
    (void)node;
    null_activation_stats_for_right_activation(node);
}

/* --- Invoked on every left activation; add=true means left addition --- */
/* NOT invoked on removals unless DO_ACTIVATION_STATS_ON_REMOVALS is set */
/*#define left_node_activation(node,add) { \
  null_activation_stats_for_left_activation(node); }*/
inline void left_node_activation(rete_node* node, bool/*add*/)
{
    (void)node;
    null_activation_stats_for_left_activation(node);
}

/* --- The following two macros are used when creating/destroying nodes --- */

/*#define init_new_rete_node_with_type(node,type) { \
  (node)->node_type = (type); \
  thisAgent->rete_node_counts[(type)]++; \
  init_sharing_stats_for_new_node(node); }*/
inline void init_new_rete_node_with_type(agent* thisAgent, rete_node* node, byte type)
{
    (node)->node_type = (type);
    thisAgent->rete_node_counts[(type)]++;
    init_sharing_stats_for_new_node(node);
}

/*#define update_stats_for_destroying_node(node) { \
  set_sharing_factor(node,0); \
  thisAgent->rete_node_counts[(node)->node_type]--; }*/
inline void update_stats_for_destroying_node(agent* thisAgent, rete_node* node)
{
    set_sharing_factor(node, 0);
    thisAgent->rete_node_counts[(node)->node_type]--;
}

















/* **********************************************************************

   SECTION 2:  Match Set Changes

   Match set changes (i.e., additions or deletions of complete production
   matches) are stored on two lists.  There is one global list of all
   pending ms changes.  Each ms change is also stored on a local list
   for its p-node, containing just the ms changes for that production.
   The second list is needed for when a match is only temporarily
   present during one elaboration cycle -- e.g., we make one change to
   working memory which triggers an addition/retraction, but then make
   another change to working memory which reverses the previous
   addition/retraction.  After the second change, the p-node gets activated
   and has to quickly find the thing being reversed.  The small local
   list makes this possible.

   EXTERNAL INTERFACE:
   Any_assertions_or_retractions_ready() returns true iff there are any
   pending changes to the match set.  This is used to test for quiescence.
   Get_next_assertion() retrieves a pending assertion (returning true) or
   returns false is no more are available.  Get_next_retraction() is
   similar.
********************************************************************** */




Symbol* find_goal_for_match_set_change_assertion(agent* thisAgent, ms_change* msc)
{

    wme* lowest_goal_wme;
    goal_stack_level lowest_level_so_far;
    token* tok;

    dprint(DT_WATERFALL, "Match goal for assertion: %y", msc->p_node->b.p.prod->name);

    lowest_goal_wme = NIL;
    lowest_level_so_far = -1;

    if (msc->w)
    {
        if (msc->w->id->id->isa_goal == true)
        {
            lowest_goal_wme = msc->w;
            lowest_level_so_far = msc->w->id->id->level;
        }
    }

    for (tok = msc->tok; tok != thisAgent->dummy_top_token; tok = tok->parent)
    {
        if (tok->w != NIL)
        {
            /* print_wme(tok->w); */
            if (tok->w->id->id->isa_goal == true)
            {

                if (lowest_goal_wme == NIL)
                {
                    lowest_goal_wme = tok->w;
                }

                else
                {
                    if (tok->w->id->id->level > lowest_goal_wme->id->id->level)
                    {
                        lowest_goal_wme = tok->w;
                    }
                }
            }

        }
    }

    if (lowest_goal_wme)
    {
        dprint_noprefix(DT_WATERFALL, " is [%y]\n", lowest_goal_wme->id);
        return lowest_goal_wme->id;
    }
    {
        dprint_noprefix(DT_WATERFALL, " has no lowest_goal_wme!\n");
        char msg[BUFFER_MSG_SIZE];
        thisAgent->outputManager->printa_sf(thisAgent, "\nError: Did not find goal for ms_change assertion: %y\n", msc->p_node->b.p.prod->name);
        SNPRINTF(msg, BUFFER_MSG_SIZE, "\nError: Did not find goal for ms_change assertion: %s\n",
                 msc->p_node->b.p.prod->name->to_string(true));
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }
    return 0;
}

Symbol* find_goal_for_match_set_change_retraction(ms_change* msc)
{

    dprint(DT_WATERFALL, "Match goal level for retraction: %y", msc->inst->prod_name);

    if (msc->inst->match_goal)
    {
        /* If there is a goal, just return the goal */
        dprint_noprefix(DT_WATERFALL, " is [%y]", msc->inst->match_goal);
        return  msc->inst->match_goal;

    }
    else
    {

        dprint_noprefix(DT_WATERFALL, " is NIL (nil goal retraction)");
        return NIL;

    }
}

bool any_assertions_or_retractions_ready(agent* thisAgent)
{

    Symbol* goal;

    /* Determining if assertions or retractions are ready require looping over
    all goals in Waterfall/Operand2 */

    if (thisAgent->nil_goal_retractions)
    {
        return true;
    }

    /* Loop from bottom to top because we expect activity at
    the bottom usually */

    for (goal = thisAgent->bottom_goal; goal; goal = goal->id->higher_goal)
    {
        /* if there are any assertions or retrctions for this goal,
        return true */
        if (goal->id->ms_o_assertions || goal->id->ms_i_assertions ||
                goal->id->ms_retractions)
        {
            return true;
        }
    }

    /* if there are no nil_goal_retractions and no assertions or retractions
    for any  goal then return false -- there aren't any productions
    ready to fire or retract */

    return false;
}


/* RCHONG: begin 10.11 */

bool any_i_assertions_or_retractions_ready(agent* thisAgent)
{
    return (thisAgent->ms_i_assertions || thisAgent->ms_retractions);
}

/* RCHONG: end 10.11 */

/* New waterfall model:
 *
 * postpone_assertion: formerly get_next_assertion. Removes the first
 * assertion from the assertion lists and adds it to the postponed
 * assertions list. Returns false if there are no assertions.
 *
 * consume_last_postponed_assertion: removes the first assertion from the
 * postponed assertions list, making it go away permenantly.
 *
 * restore_postponed_assertions: replaces the postponed assertions back on
 * the assertion lists.
 */
bool postpone_assertion(agent* thisAgent, production** prod, struct token_struct** tok, wme** w)
{
    ms_change* msc = NIL;


    /* In Waterfall, we return only assertions that match in the
    currently active goal */

    if (thisAgent->active_goal)   /* Just do asserts for current goal */
    {
        if (thisAgent->FIRING_TYPE == PE_PRODS)
        {
            if (! thisAgent->active_goal->id->ms_o_assertions)
            {
                return false;
            }

            msc = thisAgent->active_goal->id->ms_o_assertions;
            remove_from_dll(thisAgent->ms_o_assertions, msc, next, prev);
            remove_from_dll(thisAgent->active_goal->id->ms_o_assertions,
                            msc, next_in_level, prev_in_level);

        }
        else
        {
            /* IE PRODS */
            if (! thisAgent->active_goal->id->ms_i_assertions)
            {
                return false;
            }

            msc = thisAgent->active_goal->id->ms_i_assertions;
            remove_from_dll(thisAgent->ms_i_assertions, msc, next, prev);
            remove_from_dll(thisAgent->active_goal->id->ms_i_assertions,
                            msc, next_in_level, prev_in_level);
        }

    }
    else
    {

        /* If there is not an active goal, then there should not be any
        assertions.  If there are, then we generate and error message
        and abort. */

        if ((thisAgent->ms_i_assertions) ||
                (thisAgent->ms_o_assertions))
        {

            // Commented out 11/2007
            // laird: I would like us to remove that error message that happens
            // in Obscurebot. It just freaks people out and we have yet to see an error in Soar because of it.

            //char msg[BUFFER_MSG_SIZE];
            //strncpy(msg,"\nrete.c: Error: No active goal, but assertions are on the assertion list.", BUFFER_MSG_SIZE);
            //msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            //abort_with_fatal_error(thisAgent, msg);

        }

        return false; /* if we are in an initiazation and there are no
                      assertions, just retrurn false to terminate
                      the procedure. */

    }

    remove_from_dll(msc->p_node->b.p.tentative_assertions, msc,
                    next_of_node, prev_of_node);
    *prod = msc->p_node->b.p.prod;
    *tok = msc->tok;
    *w = msc->w;

    // save the assertion on the postponed list
    insert_at_head_of_dll(thisAgent->postponed_assertions, msc, next, prev);

//    assert(msc->tok && *tok);

    return true;
}

void consume_last_postponed_assertion(agent* thisAgent)
{
    assert(thisAgent->postponed_assertions);

    ms_change* msc = thisAgent->postponed_assertions;

    // get the most recently postponed assertion
    remove_from_dll(thisAgent->postponed_assertions, msc, next, prev);

    // kill it
    thisAgent->memoryManager->free_with_pool(MP_ms_change, msc);
}

void restore_postponed_assertions(agent* thisAgent)
{

    while (thisAgent->postponed_assertions)
    {
        ms_change* msc = thisAgent->postponed_assertions;

        // get the most recently postponed assertion
        remove_from_dll(thisAgent->postponed_assertions, msc, next, prev);

        assert(msc != NIL);

        // do the reverse of postpone_assertion
        insert_at_head_of_dll(msc->p_node->b.p.tentative_assertions,
                              msc, next_of_node, prev_of_node);

        assert(thisAgent->active_goal);

        if (thisAgent->FIRING_TYPE == PE_PRODS)
        {
            insert_at_head_of_dll(thisAgent->active_goal->id->ms_o_assertions,
                                  msc, next_in_level, prev_in_level);
            insert_at_head_of_dll(thisAgent->ms_o_assertions, msc, next, prev);
        }
        else
        {
            // IE
            insert_at_head_of_dll(thisAgent->active_goal->id->ms_i_assertions,
                                  msc, next_in_level, prev_in_level);
            insert_at_head_of_dll(thisAgent->ms_i_assertions, msc, next, prev);
        }
    }
}

bool get_next_retraction(agent* thisAgent, instantiation** inst)
{
    ms_change* msc;

    /* just do the retractions for the current level */

    /* initialization condition (2.107/2.111) */
    if (thisAgent->active_level == 0)
    {
        return false;
    }

    if (! thisAgent->active_goal->id->ms_retractions)
    {
        return false;
    }

    msc = thisAgent->active_goal->id->ms_retractions;

    /* remove from the complete retraction list */
    remove_from_dll(thisAgent->ms_retractions, msc, next, prev);
    /* and remove from the Waterfall-specific list */
    remove_from_dll(thisAgent->active_goal->id->ms_retractions,
                    msc, next_in_level, prev_in_level);
    if (msc->p_node)
        remove_from_dll(msc->p_node->b.p.tentative_retractions, msc,
                        next_of_node, prev_of_node);
    *inst = msc->inst;
    thisAgent->memoryManager->free_with_pool(MP_ms_change, msc);
    return true;
}






/* Retract an instantiation on the nil goal list.  If there are no
   retractions on the nil goal retraction list, return false.  This
   procedure is only called in Operand2 mode, so there is no need for
   any checks for Operand2-specific processing. */

bool get_next_nil_goal_retraction(agent* thisAgent, instantiation** inst)
{
    ms_change* msc;

    if (! thisAgent->nil_goal_retractions)
    {
        return false;
    }
    msc = thisAgent->nil_goal_retractions;

    /* Remove this retraction from the NIL goal list */
    remove_from_dll(thisAgent->nil_goal_retractions, msc,
                    next_in_level, prev_in_level);

    /* next and prev set and used in Operand2 exactly as used in Soar 7 --
       so we have to make sure and delete this retraction from the regular
       list */
    remove_from_dll(thisAgent->ms_retractions, msc, next, prev);

    if (msc->p_node)
    {
        remove_from_dll(msc->p_node->b.p.tentative_retractions, msc,
                        next_of_node, prev_of_node);
    }
    *inst = msc->inst;
    thisAgent->memoryManager->free_with_pool(MP_ms_change, msc);
    return true;

}












/* **********************************************************************

   SECTION 3:  Alpha Portion of the Rete Net

   The alpha (top) part of the rete net consists of the alpha memories.
   Each of these memories is stored in one of 16 hash tables, depending
   on which fields it tests:

      bit 0 (value 1) indicates it tests the id slot
      bit 1 (value 2) indicates it tests the attr slot
      bit 2 (value 4) indicates it tests the value slot
      bit 3 (value 8) indicates it tests for an acceptable preference

   The hash tables are dynamically resized hash tables.

   Find_or_make_alpha_mem() either shares an existing alpha memory or
   creates a new one, adjusting reference counts accordingly.
   Remove_ref_to_alpha_mem() decrements the reference count and
   deallocates the alpha memory if it's no longer used.

   EXTERNAL INTERFACE:
   Add_wme_to_rete() and remove_wme_from_rete() do just what they say.
********************************************************************** */

/* --- Returns true iff the given wme goes into the given alpha memory --- */
/*#define wme_matches_alpha_mem(w,am) ( \
  (((am)->id==NIL) || ((am)->id==(w)->id)) && \
  (((am)->attr==NIL) || ((am)->attr==(w)->attr)) && \
  (((am)->value==NIL) || ((am)->value==(w)->value)) && \
  ((am)->acceptable==(w)->acceptable))*/
inline bool wme_matches_alpha_mem(wme* w, alpha_mem* am)
{
    return ((am->id == NIL) || (am->id == w->id)) &&
           ((am->attr == NIL) || (am->attr == w->attr)) &&
           ((am->value == NIL) || (am->value == w->value)) &&
           (am->acceptable == w->acceptable);
}

/* --- Returns hash value for the given id/attr/value symbols --- */
/*#define alpha_hash_value(i,a,v,num_bits) \
 ( ( ((i) ? ((Symbol *)(i))->hash_id : 0) ^ \
     ((a) ? ((Symbol *)(a))->hash_id : 0) ^ \
     ((v) ? ((Symbol *)(v))->hash_id : 0) ) & \
   masks_for_n_low_order_bits[(num_bits)] )*/
inline uint32_t alpha_hash_value(Symbol* i, Symbol* a, Symbol* v, short num_bits)
{
    return
        (((i ? i->hash_id : 0) ^
          (a ? a->hash_id : 0) ^
          (v ? v->hash_id : 0)) &
         masks_for_n_low_order_bits[(num_bits)]);
}

/* --- rehash funciton for resizable hash table routines --- */
uint32_t hash_alpha_mem(void* item, short num_bits)
{
    alpha_mem* am;

    am = static_cast<alpha_mem_struct*>(item);
    return alpha_hash_value(am->id, am->attr, am->value, num_bits);
}

/* --- Which of the 16 hash tables to use? --- */
/*#define table_for_tests(id,attr,value,acceptable) \
  thisAgent->alpha_hash_tables [ ((id) ? 1 : 0) + ((attr) ? 2 : 0) + \
                                     ((value) ? 4 : 0) + \
                                     ((acceptable) ? 8 : 0) ]*/
inline hash_table* table_for_tests(agent* thisAgent,
                                   Symbol* id, Symbol* attr, Symbol* value,
                                   bool acceptable)
{
    return thisAgent->alpha_hash_tables [(id ? 1 : 0) + (attr ? 2 : 0) +
                                         (value ? 4 : 0) +
                                         (acceptable ? 8 : 0) ];
}

//#define get_next_alpha_mem_id() (thisAgent->alpha_mem_id_counter++)
inline uint32_t get_next_alpha_mem_id(agent* thisAgent)
{
    return thisAgent->alpha_mem_id_counter++;
}

/* --- Adds a WME to an alpha memory (create a right_mem for it), but doesn't
   inform any successors --- */
void add_wme_to_alpha_mem(agent* thisAgent, wme* w, alpha_mem* am)
{
    right_mem** header, *rm;
    uint32_t hv;

    /* --- allocate new right_mem, fill it fields --- */
    thisAgent->memoryManager->allocate_with_pool(MP_right_mem, &rm);
    rm->w = w;
    rm->am = am;

    /* --- add it to dll's for the hash bucket, alpha mem, and wme --- */
    hv = am->am_id ^ w->id->hash_id;
    header = reinterpret_cast<right_mem**>(thisAgent->right_ht) + (hv & RIGHT_HT_MASK);
    insert_at_head_of_dll(*header, rm, next_in_bucket, prev_in_bucket);
    insert_at_head_of_dll(am->right_mems, rm, next_in_am, prev_in_am);
    insert_at_head_of_dll(w->right_mems, rm, next_from_wme, prev_from_wme);
}

/* --- Removes a WME (right_mem) from its alpha memory, but doesn't inform
   any successors --- */
void remove_wme_from_alpha_mem(agent* thisAgent, right_mem* rm)
{
    wme* w;
    alpha_mem* am;
    uint32_t hv;
    right_mem** header;

    w = rm->w;
    am = rm->am;

    /* --- remove it from dll's for the hash bucket, alpha mem, and wme --- */
    hv = am->am_id ^ w->id->hash_id;
    header = reinterpret_cast<right_mem**>(thisAgent->right_ht) + (hv & RIGHT_HT_MASK);
    remove_from_dll(*header, rm, next_in_bucket, prev_in_bucket);
    remove_from_dll(am->right_mems, rm, next_in_am, prev_in_am);
    remove_from_dll(w->right_mems, rm, next_from_wme, prev_from_wme);

    /* --- deallocate it --- */
    thisAgent->memoryManager->free_with_pool(MP_right_mem, rm);
}

/* --- Looks for an existing alpha mem, returns it or NIL if not found --- */
alpha_mem* find_alpha_mem(agent* thisAgent, Symbol* id, Symbol* attr,
                          Symbol* value, bool acceptable)
{
    hash_table* ht;
    alpha_mem* am;
    uint32_t hash_value;

    ht = table_for_tests(thisAgent, id, attr, value, acceptable);
    hash_value = alpha_hash_value(id, attr, value, ht->log2size);

    for (am = reinterpret_cast<alpha_mem*>(*(ht->buckets + hash_value)); am != NIL;
            am = am->next_in_hash_table)
        if ((am->id == id) && (am->attr == attr) &&
                (am->value == value) && (am->acceptable == acceptable))
        {
            return am;
        }
    return NIL;
}

/* --- Find and share existing alpha memory, or create new one.  Adjusts
   the reference count on the alpha memory accordingly. --- */
alpha_mem* find_or_make_alpha_mem(agent* thisAgent, Symbol* id, Symbol* attr,
                                  Symbol* value, bool acceptable)
{
    hash_table* ht;
    alpha_mem* am, *more_general_am;
    wme* w;
    right_mem* rm;

    /* --- look for an existing alpha mem --- */
    am = find_alpha_mem(thisAgent, id, attr, value, acceptable);
    if (am)
    {
        am->reference_count++;
        return am;
    }

    /* --- no existing alpha_mem found, so create a new one --- */
    thisAgent->memoryManager->allocate_with_pool(MP_alpha_mem, &am);
    am->next_in_hash_table = NIL;
    am->right_mems = NIL;
    am->beta_nodes = NIL;
    am->last_beta_node = NIL;
    am->reference_count = 1;
    am->id = id;
    if (id)
    {
        thisAgent->symbolManager->symbol_add_ref(id);
    }
    am->attr = attr;
    if (attr)
    {
        thisAgent->symbolManager->symbol_add_ref(attr);
    }
    am->value = value;
    if (value)
    {
        thisAgent->symbolManager->symbol_add_ref(value);
    }
    am->acceptable = acceptable;
    am->am_id = get_next_alpha_mem_id(thisAgent);
    ht = table_for_tests(thisAgent, id, attr, value, acceptable);
    add_to_hash_table(thisAgent, ht, am);

    /* --- fill new mem with any existing matching WME's --- */
    more_general_am = NIL;
    if (id)
    {
        more_general_am = find_alpha_mem(thisAgent, NIL, attr, value, acceptable);
    }
    if (!more_general_am && value)
    {
        more_general_am = find_alpha_mem(thisAgent, NIL, attr, NIL, acceptable);
    }
    if (more_general_am)
    {
        /* --- fill new mem using the existing more general one --- */
        for (rm = more_general_am->right_mems; rm != NIL; rm = rm->next_in_am)
            if (wme_matches_alpha_mem(rm->w, am))
            {
                add_wme_to_alpha_mem(thisAgent, rm->w, am);
            }
    }
    else
    {
        /* --- couldn't find such an existing mem, so do it the hard way --- */
        for (w = thisAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
            if (wme_matches_alpha_mem(w, am))
            {
                add_wme_to_alpha_mem(thisAgent, w, am);
            }
    }

    return am;
}

/* --- Using the given hash table and hash value, try to find a
   matching alpha memory in the indicated hash bucket.  If we find one,
   we add the wme to it and inform successor nodes. --- */
void add_wme_to_aht(agent* thisAgent, hash_table* ht, uint32_t hash_value, wme* w)
{
    alpha_mem* am;
    rete_node* node, *next;

    hash_value = hash_value & masks_for_n_low_order_bits[ht->log2size];
    am = reinterpret_cast<alpha_mem*>(*(ht->buckets + hash_value));
    while (am != NIL)
    {
        if (wme_matches_alpha_mem(w, am))
        {
            /* --- found the right alpha memory, first add the wme --- */
            add_wme_to_alpha_mem(thisAgent, w, am);

            /* --- now call the beta nodes --- */
            for (node = am->beta_nodes; node != NIL; node = next)
            {
                next = node->b.posneg.next_from_alpha_mem;
                (*(right_addition_routines[node->node_type]))(thisAgent, node, w);
            }
            return; /* only one possible alpha memory per table could match */
        }
        am = am->next_in_hash_table;
    }
}

/* We cannot use 'xor' as the name of a function because it is defined in UNIX. */
//#define xor_op(i,a,v) ((i) ^ (a) ^ (v))
inline uint32_t xor_op(uint32_t i, uint32_t a, uint32_t v)
{
    return ((i) ^ (a) ^ (v));
}

/* --- Adds a WME to the Rete. --- */
void add_wme_to_rete(agent* thisAgent, wme* w)
{
    uint32_t hi, ha, hv;

    /* --- add w to all_wmes_in_rete --- */
    insert_at_head_of_dll(thisAgent->all_wmes_in_rete, w, rete_next, rete_prev);
    thisAgent->num_wmes_in_rete++;

    /* --- it's not in any right memories or tokens yet --- */
    w->right_mems = NIL;
    w->tokens = NIL;

    /* --- add w to the appropriate alpha_mem in each of 8 possible tables --- */
    hi = w->id->hash_id;
    ha = w->attr->hash_id;
    hv = w->value->hash_id;

    if (w->acceptable)
    {
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[8],  xor_op(0, 0, 0), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[9],  xor_op(hi, 0, 0), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[10], xor_op(0, ha, 0), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[11], xor_op(hi, ha, 0), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[12], xor_op(0, 0, hv), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[13], xor_op(hi, 0, hv), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[14], xor_op(0, ha, hv), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[15], xor_op(hi, ha, hv), w);
    }
    else
    {
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[0],  xor_op(0, 0, 0), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[1],  xor_op(hi, 0, 0), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[2],  xor_op(0, ha, 0), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[3],  xor_op(hi, ha, 0), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[4],  xor_op(0, 0, hv), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[5],  xor_op(hi, 0, hv), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[6],  xor_op(0, ha, hv), w);
        add_wme_to_aht(thisAgent, thisAgent->alpha_hash_tables[7],  xor_op(hi, ha, hv), w);
    }
    w->epmem_id = EPMEM_NODEID_BAD;
    w->epmem_valid = NIL;
    {
        if (thisAgent->EpMem->epmem_db->get_status() == soar_module::connected)
        {
            // if identifier-valued and short-term, known value
            if ((w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
                    (w->value->id->epmem_id != EPMEM_NODEID_BAD) &&
                    (w->value->id->epmem_valid == thisAgent->EpMem->epmem_validation))
            {
                // add id ref count
                (*thisAgent->EpMem->epmem_id_ref_counts)[ w->value->id->epmem_id ]->insert(w);
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   increasing ref_count of value in %d %d %d; new ref_count is %d\n",
                        (unsigned int) w->id->id->epmem_id, (unsigned int) epmem_temporal_hash(thisAgent, w->attr), (unsigned int) w->value->id->epmem_id, (unsigned int)(*thisAgent->epmem_id_ref_counts)[ w->value->id->epmem_id ]->size());
#endif
            }

            // if known id
            if ((w->id->id->epmem_id != EPMEM_NODEID_BAD) && (w->id->id->epmem_valid == thisAgent->EpMem->epmem_validation))
            {
                // add to add set
                thisAgent->EpMem->epmem_wme_adds->insert(w->id);
            }
        }
    }
}

inline void _epmem_remove_wme(agent* thisAgent, wme* w)
{
    bool was_encoded = false;

    if (w->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        bool lti = (w->value->id->LTI_ID != NIL);

        if ((w->epmem_id != EPMEM_NODEID_BAD) && (w->epmem_valid == thisAgent->EpMem->epmem_validation))
        {
            was_encoded = true;

            (*thisAgent->EpMem->epmem_edge_removals)[ w->epmem_id ] = true;

#ifdef DEBUG_EPMEM_WME_ADD
            fprintf(stderr, "   wme destroyed: %d %d %d\n",
                    (unsigned int) w->id->id->epmem_id, (unsigned int) epmem_temporal_hash(thisAgent, w->attr), (unsigned int) w->value->id->epmem_id);
#endif

            // return to the id pool
            //if (!lti)
            {
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   returning WME to pool: %d %d %d\n",
                        (unsigned int) w->id->id->epmem_id, (unsigned int) epmem_temporal_hash(thisAgent, w->attr), (unsigned int) w->value->id->epmem_id);
#endif
                epmem_return_id_pool::iterator p = thisAgent->EpMem->epmem_id_replacement->find(w->epmem_id);
                (*p->second).push_front(std::make_pair(w->value->id->epmem_id, w->epmem_id));
                thisAgent->EpMem->epmem_id_replacement->erase(p);
            }
        }

        // reduce the ref count on the value
        if ((w->value->id->epmem_id != EPMEM_NODEID_BAD) && (w->value->id->epmem_valid == thisAgent->EpMem->epmem_validation)) //!lti &&
        {
            epmem_wme_set* my_refs = (*thisAgent->EpMem->epmem_id_ref_counts)[ w->value->id->epmem_id ];

            epmem_wme_set::iterator rc_it = my_refs->find(w);
            if (rc_it != my_refs->end())
            {
                my_refs->erase(rc_it);
#ifdef DEBUG_EPMEM_WME_ADD
                fprintf(stderr, "   reducing ref_count of value in %d %d %d; new ref_count is %d\n",
                        (unsigned int) w->id->id->epmem_id, (unsigned int) epmem_temporal_hash(thisAgent, w->attr), (unsigned int) w->value->id->epmem_id, (unsigned int) my_refs->size());
#endif

                if (my_refs->size() == 0)
                {
#ifdef DEBUG_EPMEM_WME_ADD
                    fprintf(stderr, "   recursing; clearing ref_count of value in %d %d %d\n",
                            (unsigned int) w->id->id->epmem_id, (unsigned int) epmem_temporal_hash(thisAgent, w->attr), (unsigned int) w->value->id->epmem_id);
#endif
                    my_refs->clear();
                    thisAgent->EpMem->epmem_id_removes->push_front(w->value);
                }
            }
        }
    }
    else if ((w->epmem_id != EPMEM_NODEID_BAD) && (w->epmem_valid == thisAgent->EpMem->epmem_validation))
    {
        was_encoded = true;

        (*thisAgent->EpMem->epmem_node_removals)[ w->epmem_id ] = true;
    }

    if (was_encoded)
    {
        w->epmem_id = EPMEM_NODEID_BAD;
        w->epmem_valid = NIL;
    }
}

/*------------------------------------------------------------------
                       epmem_process_ids

   @brief This functions calles _empem_remove_wme for all valid
          id's in the agent's epmem_id_removes list.

   @detailed Also removes for all wmes related to that id's
             impasse, and all wme's related to that slot.

------------------------------------------------------------------ */

inline void _epmem_process_ids(agent* thisAgent)
{
    Symbol* id;
    slot* s;
    wme* w;

    while (!thisAgent->EpMem->epmem_id_removes->empty())
    {
        id = thisAgent->EpMem->epmem_id_removes->front();
        thisAgent->EpMem->epmem_id_removes->pop_front();

        assert(id->is_sti());

        if ((id->id->epmem_id != EPMEM_NODEID_BAD) && (id->id->epmem_valid == thisAgent->EpMem->epmem_validation))
        {
            // invalidate identifier encoding
            id->id->epmem_id = EPMEM_NODEID_BAD;
            id->id->epmem_valid = NIL;

            // impasse wmes
            for (w = id->id->impasse_wmes; w != NIL; w = w->next)
            {
                _epmem_remove_wme(thisAgent, w);
            }

            // input wmes
            for (w = id->id->input_wmes; w != NIL; w = w->next)
            {
                _epmem_remove_wme(thisAgent, w);
            }

            // regular wmes
            for (s = id->id->slots; s != NIL; s = s->next)
            {
                for (w = s->wmes; w != NIL; w = w->next)
                {
                    _epmem_remove_wme(thisAgent, w);
                }

                for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
                {
                    _epmem_remove_wme(thisAgent, w);
                }
            }
        }
    }
}

/* --- Removes a WME from the Rete. --- */
void remove_wme_from_rete(agent* thisAgent, wme* w)
{
    right_mem* rm;
    alpha_mem* am;
    rete_node* node, *next, *child;
    token* tok, *left;

    {
        if (thisAgent->EpMem->epmem_db->get_status() == soar_module::connected)
        {
            _epmem_remove_wme(thisAgent, w);
            _epmem_process_ids(thisAgent);
        }
    }

    dprint(DT_RETE_PNODE_ADD, "Removing WME from RETE: %w\n", w);

    /* --- remove w from all_wmes_in_rete --- */
    remove_from_dll(thisAgent->all_wmes_in_rete, w, rete_next, rete_prev);
    thisAgent->num_wmes_in_rete--;

    /* --- remove w from each alpha_mem it's in --- */
    while (w->right_mems)
    {
        rm = w->right_mems;
        am = rm->am;
        /* --- found the alpha memory, first remove the wme from it --- */
        remove_wme_from_alpha_mem(thisAgent, rm);

#ifdef DO_ACTIVATION_STATS_ON_REMOVALS
        /* --- if doing statistics stuff, then activate each attached node --- */
        for (node = am->beta_nodes; node != NIL; node = next)
        {
            next = node->b.posneg.next_from_alpha_mem;
            right_node_activation(node, false);
        }
#endif

        /* --- for left unlinking, then if the alpha memory just went to
           zero, left unlink any attached Pos or MP nodes --- */
        if (am->right_mems == NIL)
        {
            for (node = am->beta_nodes; node != NIL; node = next)
            {
                next = node->b.posneg.next_from_alpha_mem;
                switch (node->node_type)
                {
                    case POSITIVE_BNODE:
                    case UNHASHED_POSITIVE_BNODE:
                        unlink_from_left_mem(node);
                        break;
                    case MP_BNODE:
                    case UNHASHED_MP_BNODE:
                        make_mp_bnode_left_unlinked(node);
                        break;
                } /* end of switch (node->node_type) */
            }
        }
    }

    /* --- tree-based removal of all tokens that involve w --- */
    while (w->tokens)
    {
        tok = w->tokens;
        node = tok->node;
        if (! tok->parent)
        {
            /* Note: parent pointer is NIL only on negative node negrm tokens */
            left = tok->a.neg.left_token;
            remove_from_dll(w->tokens, tok, next_from_wme, prev_from_wme);
            remove_from_dll(left->negrm_tokens, tok,
                            a.neg.next_negrm, a.neg.prev_negrm);
            thisAgent->memoryManager->free_with_pool(MP_token, tok);
            if (! left->negrm_tokens)   /* just went to 0, so call children */
            {
                for (child = node->first_child; child != NIL; child = child->next_sibling)
                {
                    (*(left_addition_routines[child->node_type]))(thisAgent, child, left, NIL);
                }
            }
        }
        else
        {
            remove_token_and_subtree(thisAgent, w->tokens);
        }
    }
}

/* --- Decrements reference count, deallocates alpha memory if unused. --- */
void remove_ref_to_alpha_mem(agent* thisAgent, alpha_mem* am)
{
    hash_table* ht;

    am->reference_count--;
    if (am->reference_count != 0)
    {
        return;
    }
    /* --- remove from hash table, and deallocate the alpha_mem --- */
    ht = table_for_tests(thisAgent, am->id, am->attr, am->value, am->acceptable);
    remove_from_hash_table(thisAgent, ht, am);
    if (am->id)
    {
        thisAgent->symbolManager->symbol_remove_ref(&am->id);
    }
    if (am->attr)
    {
        thisAgent->symbolManager->symbol_remove_ref(&am->attr);
    }
    if (am->value)
    {
        thisAgent->symbolManager->symbol_remove_ref(&am->value);
    }
    while (am->right_mems)
    {
        remove_wme_from_alpha_mem(thisAgent, am->right_mems);
    }
    thisAgent->memoryManager->free_with_pool(MP_alpha_mem, am);
}













/* **********************************************************************

   SECTION 4: Beta Net Initialization and Primitive Construction Routines

   The following routines are the basic Rete net building routines.
   Init_dummy_top_node() creates the dummy top node (for the current
   agent).  Make_new_mem_node(), make_new_positive_node(),
   make_new_mp_node(), make_new_negative_node(), make_new_cn_node(), and
   make_new_production_node() are the basic node creators.  Split_mp_node()
   and merge_into_mp_node() do the dynamic merging/splitting of memory
   and positive nodes.
********************************************************************** */

//#define get_next_beta_node_id() (thisAgent->beta_node_id_counter++)
inline uint32_t get_next_beta_node_id(agent* thisAgent)
{
    return (thisAgent->beta_node_id_counter++);
}

/* ------------------------------------------------------------------------
                          Init Dummy Top Node

   The dummy top node always has one token in it (WME=NIL).  This is
   just there so that (real) root nodes in the beta net can be handled
   the same as non-root nodes.
------------------------------------------------------------------------ */

void init_dummy_top_node(agent* thisAgent)
{
    /* --- create the dummy top node --- */
    thisAgent->memoryManager->allocate_with_pool(MP_rete_node, &thisAgent->dummy_top_node);
    init_new_rete_node_with_type(thisAgent, thisAgent->dummy_top_node, DUMMY_TOP_BNODE);
    thisAgent->dummy_top_node->parent = NIL;
    thisAgent->dummy_top_node->first_child = NIL;
    thisAgent->dummy_top_node->next_sibling = NIL;

    /* --- create the dummy top token --- */
    thisAgent->memoryManager->allocate_with_pool(MP_token, &thisAgent->dummy_top_token);
    thisAgent->dummy_top_token->parent = NIL;
    thisAgent->dummy_top_token->node = thisAgent->dummy_top_node;
    thisAgent->dummy_top_token->w = NIL;
    thisAgent->dummy_top_token->first_child = NIL;
    thisAgent->dummy_top_token->next_sibling = NIL;
    thisAgent->dummy_top_token->prev_sibling = NIL;
    thisAgent->dummy_top_token->next_from_wme = NIL;
    thisAgent->dummy_top_token->prev_from_wme = NIL;
    thisAgent->dummy_top_token->next_of_node = NIL;
    thisAgent->dummy_top_token->prev_of_node = NIL;
    thisAgent->dummy_top_node->a.np.tokens = thisAgent->dummy_top_token;
}

/* ------------------------------------------------------------------------
                  Remove Node From Parents List of Children

   Splices a given node out of its parent's list of children.  This would
   be a lot easier if the children lists were doubly-linked, but that
   would take up a lot of extra space.
------------------------------------------------------------------------ */

void remove_node_from_parents_list_of_children(rete_node* node)
{
    rete_node* prev_sibling;

    prev_sibling = node->parent->first_child;
    if (prev_sibling == node)
    {
        node->parent->first_child = node->next_sibling;
        return;
    }
    while (prev_sibling->next_sibling != node)
    {
        prev_sibling = prev_sibling->next_sibling;
    }
    prev_sibling->next_sibling = node->next_sibling;
}

/* ------------------------------------------------------------------------
                 Update Node With Matches From Above

   Calls a node's left-addition routine with each match (token) from
   the node's parent.  DO NOT call this routine on (positive, unmerged)
   join nodes.
------------------------------------------------------------------------ */

void update_node_with_matches_from_above(agent* thisAgent, rete_node* child)
{
    rete_node* parent;
    rete_node* saved_parents_first_child, *saved_childs_next_sibling;
    right_mem* rm;
    token* tok;

    //dprint(DT_RETE_PNODE_ADD, "update_node_with_matches_from_above called with child node %d\n", child->node_id);
    if (bnode_is_bottom_of_split_mp(child->node_type))
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "\nrete.c: Internal error: update_node_with_matches_from_above called on split node", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }

    parent = child->parent;

    /* --- if parent is dummy top node, tell child about dummy top token --- */
    if (parent->node_type == DUMMY_TOP_BNODE)
    {
        (*(left_addition_routines[child->node_type]))(thisAgent, child, thisAgent->dummy_top_token, NIL);
        return;
    }

    /* --- if parent is positive: first do surgery on parent's child list,
           to replace the list with "child"; then call parent's add_right
           routine with each wme in the parent's alpha mem; then do surgery
           to restore previous child list of parent. --- */
    if (bnode_is_positive(parent->node_type))
    {
        /* --- If the node is right unlinked, then don't activate it.  This is
           important because some interpreter routines rely on the node
           being right linked whenever it gets right activated. */
        if (node_is_right_unlinked(parent))
        {
            return;
        }
        saved_parents_first_child = parent->first_child;
        saved_childs_next_sibling = child->next_sibling;
        parent->first_child = child;
        child->next_sibling = NIL;
        /* to avoid double-counting these right adds */
        rete_node* node_to_ignore_for_activation_stats = parent;
        for (rm = parent->b.posneg.alpha_mem_->right_mems; rm != NIL; rm = rm->next_in_am)
        {
            (*(right_addition_routines[parent->node_type]))(thisAgent, parent, rm->w);
        }
        node_to_ignore_for_activation_stats = NIL;
        parent->first_child = saved_parents_first_child;
        child->next_sibling = saved_childs_next_sibling;
        return;
    }

    /* --- if parent is negative or cn: easy, just look at the list of tokens
           on the parent node. --- */
    for (tok = parent->a.np.tokens; tok != NIL; tok = tok->next_of_node)
        if (! tok->negrm_tokens)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, tok, NIL);
        }
}

/* ------------------------------------------------------------------------
                     Nearest Ancestor With Same AM

   Scans up the net and finds the first (i.e., nearest) ancestor node
   that uses a given alpha_mem.  Returns that node, or NIL if none exists.
------------------------------------------------------------------------ */

rete_node* nearest_ancestor_with_same_am(rete_node* node, alpha_mem* am)
{
    while (node->node_type != DUMMY_TOP_BNODE)
    {
        if (node->node_type == CN_BNODE)
        {
            node = node->b.cn.partner->parent;
        }
        else
        {
            node = real_parent_node(node);
        }
        if (bnode_is_posneg(node->node_type) && (node->b.posneg.alpha_mem_ == am))
        {
            return node;
        }
    }
    return NIL;
}

/* --------------------------------------------------------------------
                         Make New Mem Node

   Make a new beta memory node, return a pointer to it.
-------------------------------------------------------------------- */

rete_node* make_new_mem_node(agent* thisAgent,
                             rete_node* parent, byte node_type,
                             var_location left_hash_loc)
{
    rete_node* node;

    /* --- create the node data structure, fill in fields --- */
    thisAgent->memoryManager->allocate_with_pool(MP_rete_node, &node);
    init_new_rete_node_with_type(thisAgent, node, node_type);
    node->parent = parent;
    node->next_sibling = parent->first_child;
    parent->first_child = node;
    node->first_child = NIL;
    node->b.mem.first_linked_child = NIL;

    /* These hash fields are not used for unhashed node types */
    node->left_hash_loc_field_num = left_hash_loc.field_num;
    node->left_hash_loc_levels_up = left_hash_loc.levels_up;

    node->node_id = get_next_beta_node_id(thisAgent);
    node->a.np.tokens = NIL;

    /* --- call new node's add_left routine with all the parent's tokens --- */
    update_node_with_matches_from_above(thisAgent, node);

    return node;
}

/* --------------------------------------------------------------------
                         Make New Positive Node

   Make a new positive join node, return a pointer to it.
-------------------------------------------------------------------- */

rete_node* make_new_positive_node(agent* thisAgent,
                                  rete_node* parent_mem, byte node_type,
                                  alpha_mem* am, rete_test* rt,
                                  bool prefer_left_unlinking)
{
    rete_node* node;

    /* --- create the node data structure, fill in fields --- */
    thisAgent->memoryManager->allocate_with_pool(MP_rete_node, &node);
    init_new_rete_node_with_type(thisAgent, node, node_type);
    node->parent = parent_mem;
    node->next_sibling = parent_mem->first_child;
    parent_mem->first_child = node;
    node->first_child = NIL;
    relink_to_left_mem(node);
    node->b.posneg.other_tests = rt;
    node->b.posneg.alpha_mem_ = am;
    node->b.posneg.nearest_ancestor_with_same_am =
        nearest_ancestor_with_same_am(node, am);
    relink_to_right_mem(node);

    /* --- don't need to force WM through new node yet, as it's just a
       join node with no children --- */

    /* --- unlink the join node from one side if possible --- */
    if (! parent_mem->a.np.tokens)
    {
        unlink_from_right_mem(node);
    }
    if ((! am->right_mems) && ! node_is_right_unlinked(node))
    {
        unlink_from_left_mem(node);
    }
    if (prefer_left_unlinking && (! parent_mem->a.np.tokens) &&
            (! am->right_mems))
    {
        relink_to_right_mem(node);
        unlink_from_left_mem(node);
    }

    return node;
}

/* --------------------------------------------------------------------
                             Split MP Node

   Split a given MP node into separate M and P nodes, return a pointer
   to the new Memory node.
-------------------------------------------------------------------- */

rete_node* split_mp_node(agent* thisAgent, rete_node* mp_node)
{
    rete_node mp_copy;
    rete_node* pos_node, *mem_node, *parent;
    byte mem_node_type, node_type;
    token* t;

    /* --- determine appropriate node types for new M and P nodes --- */
    if (mp_node->node_type == MP_BNODE)
    {
        node_type = POSITIVE_BNODE;
        mem_node_type = MEMORY_BNODE;
    }
    else
    {
        node_type = UNHASHED_POSITIVE_BNODE;
        mem_node_type = UNHASHED_MEMORY_BNODE;
    }

    /* --- save a copy of the MP data, then kill the MP node --- */
    mp_copy = *mp_node;
    parent = mp_node->parent;
    remove_node_from_parents_list_of_children(mp_node);
    update_stats_for_destroying_node(thisAgent, mp_node);   /* clean up rete stats stuff */

    /* --- the old MP node will get transmogrified into the new Pos node --- */
    pos_node = mp_node;

    /* --- create the new M node, transfer the MP node's tokens to it --- */
    thisAgent->memoryManager->allocate_with_pool(MP_rete_node, &mem_node);
    init_new_rete_node_with_type(thisAgent, mem_node, mem_node_type);
    set_sharing_factor(mem_node, mp_copy.sharing_factor);

    mem_node->parent = parent;
    mem_node->next_sibling = parent->first_child;
    parent->first_child = mem_node;
    mem_node->first_child = pos_node;
    mem_node->b.mem.first_linked_child = NIL;
    mem_node->left_hash_loc_field_num = mp_copy.left_hash_loc_field_num;
    mem_node->left_hash_loc_levels_up = mp_copy.left_hash_loc_levels_up;
    mem_node->node_id = mp_copy.node_id;

    mem_node->a.np.tokens = mp_node->a.np.tokens;
    for (t = mp_node->a.np.tokens; t != NIL; t = t->next_of_node)
    {
        t->node = mem_node;
    }

    /* --- transmogrify the old MP node into the new Pos node --- */
    init_new_rete_node_with_type(thisAgent, pos_node, node_type);
    pos_node->parent = mem_node;
    pos_node->first_child = mp_copy.first_child;
    pos_node->next_sibling = NIL;
    pos_node->b.posneg = mp_copy.b.posneg;
    relink_to_left_mem(pos_node);    /* for now, but might undo this below */
    set_sharing_factor(pos_node, mp_copy.sharing_factor);

    /* --- set join node's unlinking status according to mp_copy's --- */
    if (mp_bnode_is_left_unlinked(&mp_copy))
    {
        unlink_from_left_mem(pos_node);
    }

    return mem_node;
}

/* --------------------------------------------------------------------
                           Merge Into MP Node

   Merge a given Memory node and its one positive join child into an
   MP node, returning a pointer to the MP node.
-------------------------------------------------------------------- */

rete_node* merge_into_mp_node(agent* thisAgent, rete_node* mem_node)
{
    rete_node* pos_node, *mp_node, *parent;
    rete_node pos_copy;
    byte node_type;
    token* t;

    pos_node = mem_node->first_child;
    parent = mem_node->parent;

    /* --- sanity check: Mem node must have exactly one child --- */
    if ((! pos_node) || pos_node->next_sibling)
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "\nrete.c: Internal error: tried to merge_into_mp_node, but <>1 child\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }

    /* --- determine appropriate node type for new MP node --- */
    if (mem_node->node_type == MEMORY_BNODE)
    {
        node_type = MP_BNODE;
    }
    else
    {
        node_type = UNHASHED_MP_BNODE;
    }

    /* --- save a copy of the Pos data, then kill the Pos node --- */
    pos_copy = *pos_node;
    update_stats_for_destroying_node(thisAgent, pos_node);   /* clean up rete stats stuff */

    /* --- the old Pos node gets transmogrified into the new MP node --- */
    mp_node = pos_node;
    init_new_rete_node_with_type(thisAgent, mp_node, node_type);
    set_sharing_factor(mp_node, pos_copy.sharing_factor);
    mp_node->b.posneg = pos_copy.b.posneg;

    /* --- transfer the Mem node's tokens to the MP node --- */
    mp_node->a.np.tokens = mem_node->a.np.tokens;
    for (t = mem_node->a.np.tokens; t != NIL; t = t->next_of_node)
    {
        t->node = mp_node;
    }
    mp_node->left_hash_loc_field_num = mem_node->left_hash_loc_field_num;
    mp_node->left_hash_loc_levels_up = mem_node->left_hash_loc_levels_up;
    mp_node->node_id = mem_node->node_id;

    /* --- replace the Mem node with the new MP node --- */
    mp_node->parent = parent;
    mp_node->next_sibling = parent->first_child;
    parent->first_child = mp_node;
    mp_node->first_child = pos_copy.first_child;

    remove_node_from_parents_list_of_children(mem_node);
    update_stats_for_destroying_node(thisAgent, mem_node);   /* clean up rete stats stuff */
    thisAgent->memoryManager->free_with_pool(MP_rete_node, mem_node);

    /* --- set MP node's unlinking status according to pos_copy's --- */
    make_mp_bnode_left_linked(mp_node);
    if (node_is_left_unlinked(&pos_copy))
    {
        make_mp_bnode_left_unlinked(mp_node);
    }

    return mp_node;
}

/* --------------------------------------------------------------------
                           Make New MP Node

   Make a new MP node, return a pointer to it.
-------------------------------------------------------------------- */

rete_node* make_new_mp_node(agent* thisAgent,
                            rete_node* parent, byte node_type,
                            var_location left_hash_loc, alpha_mem* am,
                            rete_test* rt, bool prefer_left_unlinking)
{
    rete_node* mem_node, *pos_node;
    byte mem_node_type, pos_node_type;

    if (node_type == MP_BNODE)
    {
        pos_node_type = POSITIVE_BNODE;
        mem_node_type = MEMORY_BNODE;
    }
    else
    {
        pos_node_type = UNHASHED_POSITIVE_BNODE;
        mem_node_type = UNHASHED_MEMORY_BNODE;
    }
    mem_node = make_new_mem_node(thisAgent, parent, mem_node_type, left_hash_loc);
    pos_node = make_new_positive_node(thisAgent, mem_node, pos_node_type, am, rt,
                                      prefer_left_unlinking);
    return merge_into_mp_node(thisAgent, mem_node);
}

/* --------------------------------------------------------------------
                         Make New Negative Node

   Make a new negative node, return a pointer to it.
-------------------------------------------------------------------- */

rete_node* make_new_negative_node(agent* thisAgent,
                                  rete_node* parent, byte node_type,
                                  var_location left_hash_loc,
                                  alpha_mem* am, rete_test* rt)
{
    rete_node* node;

    thisAgent->memoryManager->allocate_with_pool(MP_rete_node, &node);
    init_new_rete_node_with_type(thisAgent, node, node_type);
    node->parent = parent;
    node->next_sibling = parent->first_child;
    parent->first_child = node;
    node->first_child = NIL;
    node->left_hash_loc_field_num = left_hash_loc.field_num;
    node->left_hash_loc_levels_up = left_hash_loc.levels_up;
    node->b.posneg.other_tests = rt;
    node->b.posneg.alpha_mem_ = am;
    node->a.np.tokens = NIL;
    node->b.posneg.nearest_ancestor_with_same_am =
        nearest_ancestor_with_same_am(node, am);
    relink_to_right_mem(node);

    node->node_id = get_next_beta_node_id(thisAgent);

    /* --- call new node's add_left routine with all the parent's tokens --- */
    update_node_with_matches_from_above(thisAgent, node);

    /* --- if no tokens arrived from parent, unlink the node --- */
    if (! node->a.np.tokens)
    {
        unlink_from_right_mem(node);
    }

    return node;
}

/* --------------------------------------------------------------------
                          Make New CN Node

   Make new CN and CN_PARTNER nodes, return a pointer to the CN node.
-------------------------------------------------------------------- */

rete_node* make_new_cn_node(agent* thisAgent,
                            rete_node* parent,
                            rete_node* bottom_of_subconditions)
{
    rete_node* node, *partner, *ncc_subconditions_top_node;

    /* --- Find top node in the subconditions branch --- */
    ncc_subconditions_top_node = NIL; /* unneeded, but avoids gcc -Wall warn */
    for (node = bottom_of_subconditions; node != parent; node = node->parent)
    {
        ncc_subconditions_top_node = node;
    }

    thisAgent->memoryManager->allocate_with_pool(MP_rete_node, &node);
    init_new_rete_node_with_type(thisAgent, node, CN_BNODE);
    thisAgent->memoryManager->allocate_with_pool(MP_rete_node, &partner);
    init_new_rete_node_with_type(thisAgent, partner, CN_PARTNER_BNODE);

    /* NOTE: for improved efficiency, <node> should be on the parent's
       children list *after* the ncc subcontitions top node */
    remove_node_from_parents_list_of_children(ncc_subconditions_top_node);
    node->parent = parent;
    node->next_sibling = parent->first_child;
    ncc_subconditions_top_node->next_sibling = node;
    parent->first_child = ncc_subconditions_top_node;
    node->first_child = NIL;

    node->a.np.tokens = NIL;
    node->b.cn.partner = partner;
    node->node_id = get_next_beta_node_id(thisAgent);

    partner->parent = bottom_of_subconditions;
    partner->next_sibling = bottom_of_subconditions->first_child;
    bottom_of_subconditions->first_child = partner;
    partner->first_child = NIL;
    partner->a.np.tokens = NIL;
    partner->b.cn.partner = node;

    /* --- call partner's add_left routine with all the parent's tokens --- */
    update_node_with_matches_from_above(thisAgent, partner);
    /* --- call new node's add_left routine with all the parent's tokens --- */
    update_node_with_matches_from_above(thisAgent, node);

    return node;
}

/* --------------------------------------------------------------------
                        Make New Production Node

   Make a new production node, return a pointer to it.

   Does not handle the following tasks:
     - filling in p_node->b.p.parents_nvn or discarding chunk variable names
     - filling in stuff on new_prod (except does fill in new_prod->p_node)
     - using update_node_with_matches_from_above (p_node) or handling
       an initial refracted instantiation
-------------------------------------------------------------------- */

rete_node* make_new_production_node(agent* thisAgent,
                                    rete_node* parent, production* new_prod)
{
    rete_node* p_node;

    thisAgent->memoryManager->allocate_with_pool(MP_rete_node, &p_node);
    init_new_rete_node_with_type(thisAgent, p_node, P_BNODE);
    new_prod->p_node = p_node;
    p_node->parent = parent;
    p_node->next_sibling = parent->first_child;
    parent->first_child = p_node;
    p_node->first_child = NIL;
    p_node->b.p.prod = new_prod;
    p_node->a.np.tokens = NIL;
    p_node->b.p.tentative_assertions = NIL;
    p_node->b.p.tentative_retractions = NIL;
    return p_node;
}









/* **********************************************************************

   SECTION 5:  Beta Net Primitive Destruction Routines

   Deallocate_rete_test_list() deallocates a list of rete test structures,
   removing references to symbols within them.

   Deallocate_rete_node() deallocates a given beta node (which must
   not be a p_node), cleaning up any tokens it contains, removing
   references (to symbols and alpha memories).  It also continues
   deallocating nodes up the net if they are no longer used.
********************************************************************** */

void deallocate_rete_test_list(agent* thisAgent, rete_test* rt)
{
    rete_test* next_rt;

    while (rt)
    {
        next_rt = rt->next;

        if (test_is_constant_relational_test(rt->type))
        {
            thisAgent->symbolManager->symbol_remove_ref(&rt->data.constant_referent);
        } else if (rt->type == DISJUNCTION_RETE_TEST)
        {
            thisAgent->symbolManager->deallocate_symbol_list_removing_references(rt->data.disjunction_list);
        }

        thisAgent->memoryManager->free_with_pool(MP_rete_test, rt);
        rt = next_rt;
    }
}

void deallocate_rete_node(agent* thisAgent, rete_node* node)
{
    rete_node* parent;

    /* --- don't deallocate the dummy top node --- */
    if (node == thisAgent->dummy_top_node)
    {
        return;
    }

    /* --- sanity check --- */
    if (node->node_type == P_BNODE)
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "Internal error: deallocate_rete_node() called on p-node.\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }

    parent = node->parent;

    /* --- if a cn node, deallocate its partner first --- */
    if (node->node_type == CN_BNODE)
    {
        deallocate_rete_node(thisAgent, node->b.cn.partner);
    }

    /* --- clean up any tokens at the node --- */
    if (! bnode_is_bottom_of_split_mp(node->node_type))
        while (node->a.np.tokens)
        {
            remove_token_and_subtree(thisAgent, node->a.np.tokens);
        }

    /* --- stuff for posneg nodes only --- */
    if (bnode_is_posneg(node->node_type))
    {
        deallocate_rete_test_list(thisAgent, node->b.posneg.other_tests);
        /* --- right unlink the node, cleanup alpha memory --- */
        if (! node_is_right_unlinked(node))
        {
            unlink_from_right_mem(node);
        }
        remove_ref_to_alpha_mem(thisAgent, node->b.posneg.alpha_mem_);
    }

    /* --- remove the node from its parent's list --- */
    remove_node_from_parents_list_of_children(node);

    /* --- for unmerged pos. nodes: unlink, maybe merge its parent --- */
    if (bnode_is_bottom_of_split_mp(node->node_type))
    {
        if (! node_is_left_unlinked(node))
        {
            unlink_from_left_mem(node);
        }
        /* --- if parent is mem node with just one child, merge them --- */
        if (parent->first_child && (! parent->first_child->next_sibling))
        {
            merge_into_mp_node(thisAgent, parent);
            parent = NIL;
        }
    }

    update_stats_for_destroying_node(thisAgent, node);   /* clean up rete stats stuff */
    thisAgent->memoryManager->free_with_pool(MP_rete_node, node);

    /* --- if parent has no other children, deallocate it, and recurse  --- */
    /* Added check to make sure that parent wasn't deallocated in previous merge */
    if (parent && !parent->first_child)
    {
        deallocate_rete_node(thisAgent, parent);
    }
}












/* **********************************************************************

   SECTION 6:  Variable Bindings and Locations

   As we build the network for a production, we have to keep track of
   where variables are bound -- i.e., at what earlier conditions/fields
   (if any) did a given variable occur?  We could do this by scanning
   upwards -- look at all the earlier conditions to try to find an
   occurrence of the variable -- but that would take O(C) time, where
   C is the number of conditions.  Instead, we store binding location
   information directly on the variables in the symbol table.  Each
   variable has a field var.rete_binding_locations, which holds a
   stack (yes, a stack) of binding locations, with the most recent (i.e.,
   lowest in the Rete) binding on top of the stack.  (It has to be a stack
   so we can push and pop bindings during the handling of conjunctive
   negations.)

   Whenever a variable is created, the symbol table routines initialize
   var.rete_binding_locations to NIL.  It is important for the stack to
   get completely popped after we're done with each production addition,
   so it gets properly reset to NIL.

   The basic operations on these binding stacks are done with a few
   macros below.  A binding location is represented by the CAR of a
   CONS -- the level and field numbers are crammed into the CAR.
   Var_is_bound() returns true iff the given variable has been bound.
   Push_var_binding() pushes a new binding of the given variable.
   Pop_var_binding() pops the top binding.
********************************************************************** */

//#define var_is_bound(v) (((Symbol *)(v))->var->rete_binding_locations != NIL)
inline bool var_is_bound(Symbol* v)
{
    return v->var->rete_binding_locations != NIL;
}

//#define varloc_to_dummy(depth,field_num) ((void *)(((depth)<<2) + (field_num)))
inline void* varloc_to_dummy(rete_node_level depth, byte field_num)
{
    return reinterpret_cast<void*>((depth << 2) + field_num);
}

//#define dummy_to_varloc_depth(d)     (((uint64_t)(d))>>2)
inline rete_node_level dummy_to_varloc_depth(void* d)
{
    return static_cast<rete_node_level>(reinterpret_cast<uintptr_t>(d) >> 2);
}

//#define dummy_to_varloc_field_num(d) (((uint64_t)(d)) & 3)
inline byte dummy_to_varloc_field_num(void* d)
{
    return static_cast<byte>(reinterpret_cast<uintptr_t>(d) & 3);
}

/*#define push_var_binding(v,depth,field_num) { \
  void *dummy_xy312; \
  dummy_xy312 = varloc_to_dummy ((depth), (field_num)); \
  push(thisAgent, dummy_xy312, ((Symbol *)(v))->var->rete_binding_locations); }*/
inline void push_var_binding(agent* thisAgent, Symbol* v, rete_node_level depth, byte field_num)
{
    void* dummy_xy312;
    dummy_xy312 = varloc_to_dummy(depth, field_num);
    push(thisAgent, dummy_xy312, v->var->rete_binding_locations);
}

/*#define pop_var_binding(v) { \
  cons *c_xy312; \
  c_xy312 = ((Symbol *)(v))->var->rete_binding_locations; \
  ((Symbol *)(v))->var->rete_binding_locations = c_xy312->rest; \
  free_cons (c_xy312); }*/
inline void pop_var_binding(agent* thisAgent, void* v)
{
    cons* c_xy312;
    c_xy312 = static_cast<Symbol*>(v)->var->rete_binding_locations;
    static_cast<Symbol*>(v)->var->rete_binding_locations = c_xy312->rest;
    free_cons(thisAgent, c_xy312);
}

/* -------------------------------------------------------------------
                          Find Var Location

   This routine finds the most recent place a variable was bound.
   It does this simply by looking at the top of the binding stack
   for that variable.  If there is any binding, its location is stored
   in the parameter *result, and the function returns true.  If no
   binding is found, the function returns false.
------------------------------------------------------------------- */

bool find_var_location(Symbol* var, rete_node_level current_depth,
                       var_location* result)
{
    void* dummy;
    if (! var->var->rete_binding_locations)
    {
        return false;
    }
    dummy = var->var->rete_binding_locations->first;
    result->levels_up = current_depth - dummy_to_varloc_depth(dummy);
    result->field_num = dummy_to_varloc_field_num(dummy);
    //dprint(DT_DEBUG, "find_var_location returning %d %d", result->levels_up, result->field_num);
    return true;
}

/* -------------------------------------------------------------------
                      Bind Variables in Test

   This routine pushes bindings for variables occurring (i.e., being
   equality-tested) in a given test.  It can do this in DENSE fashion
   (push a new binding for ANY variable) or SPARSE fashion (push a new
   binding only for previously-unbound variables), depending on the
   boolean "dense" parameter.  Any variables receiving new bindings
   are also pushed onto the given "varlist".
------------------------------------------------------------------- */

void bind_variables_in_test(agent* thisAgent,
                            test t,
                            rete_node_level depth,
                            byte field_num,
                            bool dense,
                            cons** varlist)
{
    Symbol* referent;

    assert(t && t->eq_test);
    referent = t->eq_test->data.referent;
    if (!referent->is_variable()) return;
    if (!dense && var_is_bound(referent)) return;
    push_var_binding(thisAgent, referent, depth, field_num);
    push(thisAgent, referent, *varlist);
}

/* -------------------------------------------------------------------
             Pop Bindings and Deallocate List of Variables

   This routine takes a list of variables; for each item <v> on the
   list, it pops a binding of <v>.  It also deallocates the list.
   This is often used for un-binding a group of variables which got
   bound in some procedure.
------------------------------------------------------------------- */

void pop_bindings_and_deallocate_list_of_variables(agent* thisAgent, cons* vars)
{
    while (vars)
    {
        cons* c;
        c = vars;
        vars = vars->rest;
        pop_var_binding(thisAgent, c->first);
        free_cons(thisAgent, c);
    }
}

/* **********************************************************************

   SECTION 7:  Varnames and Node_Varnames

   Varnames and Node_Varnames (NVN) structures are used to record the names
   of variables bound (i.e., equality tested) at rete nodes.  The only
   purpose of saving this information is so we can reconstruct the
   original source code for a production when we want to print it.  For
   chunks, we don't save any of this information -- we just re-gensym
   the variable names on each printing (unless DISCARD_CHUNK_VARNAMES
   is set to false).

   For each production, a chain of node_varnames structures is built,
   paralleling the structure of the rete net (i.e., the portion of the rete
   used for that production).  There is a node_varnames structure for
   each Mem, Neg, or NCC node in that part, giving the names of variables
   bound in the id, attr, and value fields of the condition at that node.

   At each field, we could bind zero, one, or more variables.  To
   save space, we use some bit-twiddling here.  A "varnames" represents
   zero or more variables:   NIL means zero; a pointer (with the low-order
   bit being 0) to a variable means just that one variable; and any
   other pointer (with the low-order bit set to 1) points (minus 1, of
   course) to a consed list of variables.

   Add_var_to_varnames() takes an existing varnames object (which can
   be NIL, for no variable names) and returns a new varnames object
   which adds (destructively!) a given variable to the previous one.
   Deallocate_varnames() deallocates a varnames object, removing references
   to symbols, etc.  Deallocate_node_varnames() deallocates a whole
   chain of node_varnames structures, scanning up the net, etc.
********************************************************************** */

varnames* add_var_to_varnames(agent* thisAgent, Symbol* var,
                              varnames* old_varnames)
{
    cons* c1, *c2;

    thisAgent->symbolManager->symbol_add_ref(var);
    if (old_varnames == NIL)
    {
        return one_var_to_varnames(var);
    }
    if (varnames_is_one_var(old_varnames))
    {
        allocate_cons(thisAgent, &c1);
        allocate_cons(thisAgent, &c2);
        c1->first = var;
        c1->rest = c2;
        c2->first = varnames_to_one_var(old_varnames);
        c2->rest = NIL;
        return var_list_to_varnames(c1);
    }
    /* --- otherwise old_varnames is a list --- */
    allocate_cons(thisAgent, &c1);
    c1->first = var;
    c1->rest = varnames_to_var_list(old_varnames);
    return var_list_to_varnames(c1);
}

void deallocate_varnames(agent* thisAgent, varnames* vn)
{
    Symbol* sym;
    cons* symlist;

    if (vn == NIL)
    {
        return;
    }
    if (varnames_is_one_var(vn))
    {
        sym = varnames_to_one_var(vn);
        thisAgent->symbolManager->symbol_remove_ref(&sym);
    }
    else
    {
        symlist = varnames_to_var_list(vn);
        thisAgent->symbolManager->deallocate_symbol_list_removing_references(symlist);
    }
}

void deallocate_node_varnames(agent* thisAgent,
                              rete_node* node, rete_node* cutoff,
                              node_varnames* nvn)
{
    node_varnames* temp;

    while (node != cutoff)
    {
        if (node->node_type == CN_BNODE)
        {
            deallocate_node_varnames(thisAgent, node->b.cn.partner->parent, node->parent,
                                     nvn->data.bottom_of_subconditions);
        }
        else
        {
            deallocate_varnames(thisAgent, nvn->data.fields.id_varnames);
            deallocate_varnames(thisAgent, nvn->data.fields.attr_varnames);
            deallocate_varnames(thisAgent, nvn->data.fields.value_varnames);
        }
        node = real_parent_node(node);
        temp = nvn;
        nvn = nvn->parent;
        thisAgent->memoryManager->free_with_pool(MP_node_varnames, temp);
    }
}

/* ----------------------------------------------------------------------
                          Add Varnames to Test

   This routine adds (an equality test for) each variable in "vn" to
   the given test "t", destructively modifying t.  This is used for
   restoring the original variables to test in a hand-coded production
   when we reconstruct its conditions.
---------------------------------------------------------------------- */

void add_varnames_to_test(agent* thisAgent, varnames* vn, test* t)
{
    test New;
    cons* c;
    Symbol* temp;

    if (vn == NIL)
    {
        return;
    }
    if (varnames_is_one_var(vn))
    {
        temp = varnames_to_one_var(vn);
        dprint(DT_ADD_EXPLANATION_TRACE, "add_varnames_to_test adding varname %s from one_var.\n", temp->var->name);
        New = make_test(thisAgent, temp, EQUALITY_TEST);
        add_test(thisAgent, t, New);
    }
    else
    {
        for (c = varnames_to_var_list(vn); c != NIL; c = c->rest)
        {
            temp = static_cast<Symbol*>(c->first);
            dprint(DT_ADD_EXPLANATION_TRACE, "add_varnames_to_test adding varname %s from varlist.\n", temp->var->name);
            New =  make_test(thisAgent, temp, EQUALITY_TEST);
            add_test(thisAgent, t, New);
        }
    }
}


void add_varname_identity_to_test(agent* thisAgent, varnames* vn, test t, uint64_t pI_id, bool pNoConstantIdentities)
{
//    test New;
    cons* c;
    Symbol* temp;

    if (vn == NIL) return;
    if (pNoConstantIdentities && !t->data.referent->is_sti()) return;

    assert (varnames_is_one_var(vn));
    temp = varnames_to_one_var(vn);
    if (!t->data.referent->is_variable())
    {
        t->identity = thisAgent->explanationBasedChunker->get_or_create_identity(temp, pI_id);
        dprint(DT_ADD_EXPLANATION_TRACE, "add_varname_identity_to_test adding identity o%u for varname %y from one_var in inst %u.\n", t->identity, temp, pI_id);
    } else {
        dprint(DT_ADD_EXPLANATION_TRACE, "add_varname_identity_to_test did not add identity for varname %y because ungrounded NCC var in inst %u.\n", temp, pI_id);
    }

}
/* -------------------------------------------------------------------
     Creating the Node Varnames Structures for a List of Conditions

   Add_unbound_varnames_in_test() adds to an existing varnames object
   the names of any currently-unbound variables equality-tested in
   a given test.  Make_nvn_for_posneg_cond() creates and returns the
   node_varnames structure for a single given (simple) positive or
   negative condition.  Get_nvn_for_condition_list() creates the
   whole chain of NVN structures for a list of conditions, returning
   a pointer to the bottom structure in the chain.
------------------------------------------------------------------- */

varnames* add_unbound_varnames_in_test(agent* thisAgent, test t,
                                       varnames* starting_vn)
{
    assert(t && t->eq_test);

    Symbol* referent = t->eq_test->data.referent;
    if (referent->is_variable() && !var_is_bound(referent))
    {
        starting_vn = add_var_to_varnames(thisAgent, referent, starting_vn);
    }
    return starting_vn;
}

node_varnames* make_nvn_for_posneg_cond(agent* thisAgent,
                                        condition* cond,
                                        node_varnames* parent_nvn)
{
    node_varnames* New;
    cons* vars_bound;

    vars_bound = NIL;

    thisAgent->memoryManager->allocate_with_pool(MP_node_varnames, &New);
    New->parent = parent_nvn;

    /* --- fill in varnames for id test --- */
    New->data.fields.id_varnames =
        add_unbound_varnames_in_test(thisAgent, cond->data.tests.id_test, NIL);

    /* --- add sparse bindings for id, then get attr field varnames --- */
    bind_variables_in_test(thisAgent, cond->data.tests.id_test, 0, 0, false, &vars_bound);
    New->data.fields.attr_varnames =
        add_unbound_varnames_in_test(thisAgent, cond->data.tests.attr_test, NIL);

    /* --- add sparse bindings for attr, then get value field varnames --- */
    bind_variables_in_test(thisAgent, cond->data.tests.attr_test, 0, 0, false, &vars_bound);
    New->data.fields.value_varnames =
        add_unbound_varnames_in_test(thisAgent, cond->data.tests.value_test, NIL);

    /* --- Pop the variable bindings for these conditions --- */
    pop_bindings_and_deallocate_list_of_variables(thisAgent, vars_bound);

    return New;
}

node_varnames* get_nvn_for_condition_list(agent* thisAgent,
        condition* cond_list,
        node_varnames* parent_nvn)
{
    node_varnames* New = 0;
    condition* cond;
    cons* vars;

    vars = NIL;

    for (cond = cond_list; cond != NIL; cond = cond->next)
    {

        switch (cond->type)
        {
            case POSITIVE_CONDITION:
                New = make_nvn_for_posneg_cond(thisAgent, cond, parent_nvn);

                /* --- Add sparse variable bindings for this condition --- */
                bind_variables_in_test(thisAgent, cond->data.tests.id_test, 0, 0, false, &vars);
                bind_variables_in_test(thisAgent, cond->data.tests.attr_test, 0, 0, false, &vars);
                bind_variables_in_test(thisAgent, cond->data.tests.value_test, 0, 0, false, &vars);
                break;
            case NEGATIVE_CONDITION:
                New = make_nvn_for_posneg_cond(thisAgent, cond, parent_nvn);
                break;
            case CONJUNCTIVE_NEGATION_CONDITION:
                thisAgent->memoryManager->allocate_with_pool(MP_node_varnames, &New);
                New->parent = parent_nvn;
                New->data.bottom_of_subconditions =
                    get_nvn_for_condition_list(thisAgent, cond->data.ncc.top, parent_nvn);
                break;
        }

        parent_nvn = New;
    }

    /* --- Pop the variable bindings for these conditions --- */
    pop_bindings_and_deallocate_list_of_variables(thisAgent, vars);

    return parent_nvn;
}

/* **********************************************************************

   SECTION 8:  Building the Rete Net:  Condition-To-Node Converstion

   Build_network_for_condition_list() is the key routine here. (See
   description below.)
********************************************************************** */


/* ---------------------------------------------------------------------
                         Add Rete Tests for Test

   This is used for converting tests (from conditions) into the appropriate
   rete_test's and/or constant-to-be-tested-by-the-alpha-network.  It takes
   all sub-tests from a given test, converts them into the necessary Rete
   tests (if any -- note that an equality test with a previously-unbound
   variable can be ignored), and destructively adds the Rete tests to
   the given "rt" parameter.  The "current_depth" and "field_num" params
   tell where the current test originated.

   For any field, we can handle one equality-with-a-constant test in the
   alpha net.  If the "*alpha_constant" parameter is initially NIL, this
   routine may also set *alpha_constant to point to the constant symbol
   for the alpha net to test (rather than creating the corresponding
   rete_test).

   Before calling this routine, variables should be bound densely for
   parent and higher conditions, and sparsely for the current condition.
------------------------------------------------------------------------ */
void add_rete_tests_for_test(agent* thisAgent, test t,
                             rete_node_level current_depth,
                             byte field_num,
                             rete_test** rt,
                             Symbol** alpha_constant)
{
    var_location where;
    where.var_location_struct::field_num = 0;
    where.var_location_struct::levels_up = 0;
    cons* c;
    rete_test* new_rt;
    Symbol* referent;

    if (!t)
    {
        return;
    }

    switch (t->type)
    {
        case EQUALITY_TEST:
            referent = t->data.referent;

        /* --- if constant test and alpha=NIL, install alpha test --- */
        if ((referent->symbol_type != VARIABLE_SYMBOL_TYPE) &&
                (*alpha_constant == NIL))
        {
            *alpha_constant = referent;
            return;
        }

        /* --- if constant, make = constant test --- */
        if (referent->symbol_type != VARIABLE_SYMBOL_TYPE)
        {
                thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &new_rt);
            new_rt->right_field_num = field_num;
            new_rt->type = CONSTANT_RELATIONAL_RETE_TEST + RELATIONAL_EQUAL_RETE_TEST;
            new_rt->data.constant_referent = referent;
            thisAgent->symbolManager->symbol_add_ref(referent);
            new_rt->next = *rt;
            *rt = new_rt;
            return;
        }

        /* --- variable: if binding is for current field, do nothing --- */
        if (! find_var_location(referent, current_depth, &where))
        {
            char msg[BUFFER_MSG_SIZE];
            thisAgent->outputManager->printa_sf(thisAgent, "Error: Rete build found test of unbound var: %y\n",
                               referent);
            SNPRINTF(msg, BUFFER_MSG_SIZE, "Error: Rete build found test of unbound var: %s\n",
                         referent->to_string(true));
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }
        if ((where.levels_up == 0) && (where.field_num == field_num))
        {
            return;
        }

        /* --- else make variable equality test --- */
            thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &new_rt);
        new_rt->right_field_num = field_num;
        new_rt->type = VARIABLE_RELATIONAL_RETE_TEST + RELATIONAL_EQUAL_RETE_TEST;
        new_rt->data.variable_referent = where;
        new_rt->next = *rt;
        *rt = new_rt;
        return;
            break;
        case SMEM_LINK_TEST:
        case SMEM_LINK_NOT_TEST:
        case NOT_EQUAL_TEST:
        case LESS_TEST:
        case GREATER_TEST:
        case LESS_OR_EQUAL_TEST:
        case GREATER_OR_EQUAL_TEST:
        case SAME_TYPE_TEST:
            /* --- if constant, make constant test --- */
            if (t->data.referent->symbol_type != VARIABLE_SYMBOL_TYPE)
            {
                thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &new_rt);
                new_rt->right_field_num = field_num;
                new_rt->type = CONSTANT_RELATIONAL_RETE_TEST +
                               test_type_to_relational_test_type(t->type);
                new_rt->data.constant_referent = t->data.referent;
                thisAgent->symbolManager->symbol_add_ref(t->data.referent);
                new_rt->next = *rt;
                *rt = new_rt;
                return;
            }
            /* --- else make variable test --- */
            if (! find_var_location(t->data.referent, current_depth, &where))
            {
                char msg[BUFFER_MSG_SIZE];
                thisAgent->outputManager->printa_sf(thisAgent, "Error: Rete build found test of unbound var: %y\n",
                                   t->data.referent);
                SNPRINTF(msg, BUFFER_MSG_SIZE, "Error: Rete build found test of unbound var: %s\n",
                         t->data.referent->to_string(true));
                msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
                abort_with_fatal_error(thisAgent, msg);
            }
            thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &new_rt);
            new_rt->right_field_num = field_num;
            new_rt->type = VARIABLE_RELATIONAL_RETE_TEST +
                           test_type_to_relational_test_type(t->type);
            new_rt->data.variable_referent = where;
            new_rt->next = *rt;
            *rt = new_rt;
            return;

        case DISJUNCTION_TEST:
            thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &new_rt);
            new_rt->right_field_num = field_num;
            new_rt->type = DISJUNCTION_RETE_TEST;
            new_rt->data.disjunction_list =
                thisAgent->symbolManager->copy_symbol_list_adding_references(t->data.disjunction_list);
            new_rt->next = *rt;
            *rt = new_rt;
            return;

        case CONJUNCTIVE_TEST:
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                add_rete_tests_for_test(thisAgent, static_cast<test>(c->first),
                                        current_depth, field_num, rt, alpha_constant);
            }
            return;

        case GOAL_ID_TEST:
            thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &new_rt);
            new_rt->type = ID_IS_GOAL_RETE_TEST;
            new_rt->right_field_num = 0;
            new_rt->next = *rt;
            *rt = new_rt;
            return;

        case IMPASSE_ID_TEST:
            thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &new_rt);
            new_rt->type = ID_IS_IMPASSE_RETE_TEST;
            new_rt->right_field_num = 0;
            new_rt->next = *rt;
            *rt = new_rt;
            return;

        case SMEM_LINK_UNARY_TEST:
            thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &new_rt);
            new_rt->type = UNARY_SMEM_LINK_RETE_TEST;
            new_rt->right_field_num = field_num;
            new_rt->next = *rt;
            *rt = new_rt;
            return;

        case SMEM_LINK_UNARY_NOT_TEST:
            thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &new_rt);
            new_rt->type = UNARY_SMEM_LINK_NOT_RETE_TEST;
            new_rt->right_field_num = field_num;
            new_rt->next = *rt;
            *rt = new_rt;
            return;

        default:
        {
            char msg[BUFFER_MSG_SIZE];
            SNPRINTF(msg, BUFFER_MSG_SIZE, "Error: found bad test type %d while building rete\n",
                     t->type);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
            break;
        }
    } /* end of switch statement */
} /* end of function add_rete_tests_for_test() */



/* ------------------------------------------------------------------------
                      Rete Test Lists are Identical

   This is used for checking whether an existing Rete node can be
   shared, instead of building a new one.

   Single_rete_tests_are_identical() checks whether two (non-conjunctive)
   Rete tests are the same.  (Note that in the case of disjunction tests,
   the symbols in the disjunction have to be in the same order; this
   simplifies and speeds up the code here, but unnecessarily reduces
   sharing.)

   Rete_test_lists_are_identical() checks whether two lists of Rete tests
   are identical.  (Note that the lists have to be in the order; the code
   here doesn't check all possible orderings.)
------------------------------------------------------------------------ */

bool single_rete_tests_are_identical(agent* thisAgent, rete_test* rt1, rete_test* rt2)
{
    cons* c1, *c2;

    if (rt1->type != rt2->type)
    {
        return false;
    }

    if (rt1->right_field_num != rt2->right_field_num)
    {
        return false;
    }
    if (test_is_variable_relational_test(rt1->type))
    {
        return (var_locations_equal(rt1->data.variable_referent, rt2->data.variable_referent));
    }

    if (test_is_constant_relational_test(rt1->type))
    {
        return (rt1->data.constant_referent == rt2->data.constant_referent);
    }

    if ((rt1->type == ID_IS_GOAL_RETE_TEST) || (rt1->type == ID_IS_IMPASSE_RETE_TEST) ||
        (rt1->type == UNARY_SMEM_LINK_RETE_TEST) || (rt1->type == UNARY_SMEM_LINK_NOT_RETE_TEST))
    {
        return true;
    }

    if (rt1->type == DISJUNCTION_RETE_TEST)
    {
        c1 = rt1->data.disjunction_list;
        c2 = rt2->data.disjunction_list;
        while ((c1 != NIL) && (c2 != NIL))
        {
            if (c1->first != c2->first)
            {
                return false;
            }
            c1 = c1->rest;
            c2 = c2->rest;
        }
        if (c1 == c2)
        {
            return true;
        }
        return false;
    }
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "Internal error: bad rete test type in single_rete_tests_are_identical\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }
    return false; /* unreachable, but without it, gcc -Wall warns here */
}

bool rete_test_lists_are_identical(agent* thisAgent, rete_test* rt1, rete_test* rt2)
{
    while (rt1 && rt2)
    {
        if (! single_rete_tests_are_identical(thisAgent, rt1, rt2))
        {
            return false;
        }
        rt1 = rt1->next;
        rt2 = rt2->next;
    }
    if (rt1 == rt2)
    {
        return true;    /* make sure they both hit end-of-list */
    }
    return false;
}

/* ------------------------------------------------------------------------
                      Extract Rete Test to Hash With

   Extracts from a Rete test list the variable equality test to use for
   hashing.  Returns true if successful, or false if there was no such
   test to use for hashing.  The Rete test list ("rt") is destructively
   modified to splice out the extracted test.
------------------------------------------------------------------------ */

bool extract_rete_test_to_hash_with(agent* thisAgent,
                                    rete_test** rt,
                                    var_location* dest_hash_loc)
{
    rete_test* prev, *current;

    /* --- look through rt list, find the first variable equality test --- */
    prev = NIL;
    for (current = *rt; current != NIL; prev = current, current = current->next)
        if (current->type == VARIABLE_RELATIONAL_RETE_TEST +
                RELATIONAL_EQUAL_RETE_TEST)
        {
            break;
        }

    if (!current)
    {
        return false;    /* no variable equality test was found */
    }

    /* --- unlink it from rt --- */
    if (prev)
    {
        prev->next = current->next;
    }
    else
    {
        *rt = current->next;
    }

    /* --- extract info, and deallocate that single test --- */
    *dest_hash_loc = current->data.variable_referent;
    current->next = NIL;
    deallocate_rete_test_list(thisAgent, current);
    return true;
}

/* ------------------------------------------------------------------------
                       Make Node for Positive Cond

   Finds or creates a node for the given single condition <cond>, which
   must be a simple positive condition.  The node is made a child of the
   given <parent> node.  Variables for earlier conditions should be bound
   densely before this routine is called.  The routine returns a pointer
   to the (newly-created or shared) node.
------------------------------------------------------------------------ */

rete_node* make_node_for_positive_cond(agent* thisAgent,
                                       condition* cond,
                                       rete_node_level current_depth,
                                       rete_node* parent)
{
    byte pos_node_type, mem_node_type, mp_node_type;
    Symbol* alpha_id, *alpha_attr, *alpha_value;
    rete_node* node, *mem_node, *mp_node;
    alpha_mem* am;
    rete_test* rt;
    bool hash_this_node;
    var_location left_hash_loc;
    left_hash_loc.var_location_struct::field_num = 0;
    left_hash_loc.var_location_struct::levels_up = 0;
    cons* vars_bound_here;

    alpha_id = alpha_attr = alpha_value = NIL;
    rt = NIL;
    vars_bound_here = NIL;

    /* --- Add sparse variable bindings for this condition --- */
    bind_variables_in_test(thisAgent, cond->data.tests.id_test, current_depth, 0,
                           false, &vars_bound_here);
    bind_variables_in_test(thisAgent, cond->data.tests.attr_test, current_depth, 1,
                           false, &vars_bound_here);
    bind_variables_in_test(thisAgent, cond->data.tests.value_test, current_depth, 2,
                           false, &vars_bound_here);

    /* --- Get Rete tests, alpha constants, and hash location --- */
    add_rete_tests_for_test(thisAgent, cond->data.tests.id_test, current_depth, 0,
                            &rt, &alpha_id);
    hash_this_node = extract_rete_test_to_hash_with(thisAgent, &rt, &left_hash_loc);
    add_rete_tests_for_test(thisAgent, cond->data.tests.attr_test, current_depth, 1,
                            &rt, &alpha_attr);
    add_rete_tests_for_test(thisAgent, cond->data.tests.value_test, current_depth, 2,
                            &rt, &alpha_value);

    /* --- Pop sparse variable bindings for this condition --- */
    pop_bindings_and_deallocate_list_of_variables(thisAgent, vars_bound_here);

    /* --- Get alpha memory --- */
    am = find_or_make_alpha_mem(thisAgent, alpha_id, alpha_attr, alpha_value,
                                cond->test_for_acceptable_preference);

    /* --- Algorithm for adding node:
            1.  look for matching mem node; if found then
                  look for matching join node; create new one if no match
            2.  no matching mem node:  look for mp node with matching mem
                  if found, if join part matches too, then done
                            else delete mp node, create mem node and 2 joins
                  if not matching mem node, create new mp node. */

    /* --- determine desired node types --- */
    if (hash_this_node)
    {
        pos_node_type = POSITIVE_BNODE;
        mem_node_type = MEMORY_BNODE;
        mp_node_type = MP_BNODE;
    }
    else
    {
        pos_node_type = UNHASHED_POSITIVE_BNODE;
        mem_node_type = UNHASHED_MEMORY_BNODE;
        mp_node_type = UNHASHED_MP_BNODE;
    }

    /* --- look for a matching existing memory node --- */
    for (mem_node = parent->first_child; mem_node != NIL;
            mem_node = mem_node->next_sibling)
        if ((mem_node->node_type == mem_node_type) &&
                ((!hash_this_node) ||
                 ((mem_node->left_hash_loc_field_num == left_hash_loc.field_num) &&
                  (mem_node->left_hash_loc_levels_up == left_hash_loc.levels_up))))
        {
            break;
        }

    if (mem_node)     /* -- A matching memory node was found --- */
    {
        /* --- look for a matching existing join node --- */
        for (node = mem_node->first_child; node != NIL; node = node->next_sibling)
            if ((node->node_type == pos_node_type) &&
                    (am == node->b.posneg.alpha_mem_) &&
                    rete_test_lists_are_identical(thisAgent, node->b.posneg.other_tests, rt))
            {
                break;
            }

        if (node)      /* --- A matching join node was found --- */
        {
            deallocate_rete_test_list(thisAgent, rt);
            remove_ref_to_alpha_mem(thisAgent, am);
            return node;
        }
        else           /* --- No match was found, so create a new node --- */
        {
            node = make_new_positive_node(thisAgent, mem_node, pos_node_type, am , rt, false);
            return node;
        }
    }

    /* --- No matching memory node was found; look for MP with matching M --- */
    for (mp_node = parent->first_child; mp_node != NIL;
            mp_node = mp_node->next_sibling)
        if ((mp_node->node_type == mp_node_type) &&
                ((!hash_this_node) ||
                 ((mp_node->left_hash_loc_field_num == left_hash_loc.field_num) &&
                  (mp_node->left_hash_loc_levels_up == left_hash_loc.levels_up))))
        {
            break;
        }

    if (mp_node)    /* --- Found matching M part of MP --- */
    {
        if ((am == mp_node->b.posneg.alpha_mem_) &&
                rete_test_lists_are_identical(thisAgent, mp_node->b.posneg.other_tests, rt))
        {
            /* --- Complete MP match was found --- */
            deallocate_rete_test_list(thisAgent, rt);
            remove_ref_to_alpha_mem(thisAgent, am);
            return mp_node;
        }

        /* --- Delete MP node, replace it with M and two positive joins --- */
        mem_node = split_mp_node(thisAgent, mp_node);
        node = make_new_positive_node(thisAgent, mem_node, pos_node_type, am, rt, false);
        return node;
    }

    /* --- Didn't even find a matching M part of MP, so make a new MP node --- */
    return make_new_mp_node(thisAgent, parent, mp_node_type, left_hash_loc, am, rt, false);
}

/* ------------------------------------------------------------------------
                       Make Node for Negative Cond

   Finds or creates a node for the given single condition <cond>, which
   must be a simple negative (not ncc) condition.  The node is made a
   child of the given <parent> node.  Variables for earlier conditions
   should be bound densely before this routine is called.  The routine
   returns a pointer to the (newly-created or shared) node.
------------------------------------------------------------------------ */

rete_node* make_node_for_negative_cond(agent* thisAgent,
                                       condition* cond,
                                       rete_node_level current_depth,
                                       rete_node* parent)
{
    byte node_type;
    Symbol* alpha_id, *alpha_attr, *alpha_value;
    rete_node* node;
    alpha_mem* am;
    rete_test* rt;
    bool hash_this_node;
    var_location left_hash_loc;
    left_hash_loc.var_location_struct::field_num = 0;
    left_hash_loc.var_location_struct::levels_up = 0;
    cons* vars_bound_here;

    alpha_id = alpha_attr = alpha_value = NIL;
    rt = NIL;
    vars_bound_here = NIL;

    /* --- Add sparse variable bindings for this condition --- */
    bind_variables_in_test(thisAgent, cond->data.tests.id_test, current_depth, 0,
                           false, &vars_bound_here);
    bind_variables_in_test(thisAgent, cond->data.tests.attr_test, current_depth, 1,
                           false, &vars_bound_here);
    bind_variables_in_test(thisAgent, cond->data.tests.value_test, current_depth, 2,
                           false, &vars_bound_here);

    /* --- Get Rete tests, alpha constants, and hash location --- */
    add_rete_tests_for_test(thisAgent, cond->data.tests.id_test, current_depth, 0,
                            &rt, &alpha_id);
    hash_this_node = extract_rete_test_to_hash_with(thisAgent, &rt, &left_hash_loc);
    add_rete_tests_for_test(thisAgent, cond->data.tests.attr_test, current_depth, 1,
                            &rt, &alpha_attr);
    add_rete_tests_for_test(thisAgent, cond->data.tests.value_test, current_depth, 2,
                            &rt, &alpha_value);

    /* --- Pop sparse variable bindings for this condition --- */
    pop_bindings_and_deallocate_list_of_variables(thisAgent, vars_bound_here);

    /* --- Get alpha memory --- */
    am = find_or_make_alpha_mem(thisAgent, alpha_id, alpha_attr, alpha_value,
                                cond->test_for_acceptable_preference);

    /* --- determine desired node type --- */
    node_type = hash_this_node ? NEGATIVE_BNODE : UNHASHED_NEGATIVE_BNODE;

    /* --- look for a matching existing node --- */
    for (node = parent->first_child; node != NIL; node = node->next_sibling)
        if ((node->node_type == node_type) &&
                (am == node->b.posneg.alpha_mem_) &&
                ((!hash_this_node) ||
                 ((node->left_hash_loc_field_num == left_hash_loc.field_num) &&
                  (node->left_hash_loc_levels_up == left_hash_loc.levels_up))) &&
                rete_test_lists_are_identical(thisAgent, node->b.posneg.other_tests, rt))
        {
            break;
        }

    if (node)      /* --- A matching node was found --- */
    {
        deallocate_rete_test_list(thisAgent, rt);
        remove_ref_to_alpha_mem(thisAgent, am);
        return node;
    }
    else           /* --- No match was found, so create a new node --- */
    {
        node = make_new_negative_node(thisAgent, parent, node_type, left_hash_loc, am, rt);
        return node;
    }
}

/* ------------------------------------------------------------------------
                      Build Network for Condition List

    This routine builds or shares the Rete network for the conditions in
    the given <cond_list>.  <Depth_of_first_cond> tells the depth of the
    first condition/node; <parent> gives the parent node under which the
    network should be built or shared.

    Three "dest" parameters may be used for returing results from this
    routine.  If <dest_bottom_node> is given as non-NIL, this routine
    fills it in with a pointer to the lowermost node in the resulting
    network.  If <dest_bottom_depth> is non-NIL, this routine fills it
    in with the depth of the lowermost node.  If <dest_vars_bound> is
    non_NIL, this routine fills it in with a list of variables bound
    in the given <cond_list>, and does not pop the bindings for those
    variables, in which case the caller is responsible for popping theose
    bindings.  If <dest_vars_bound> is given as NIL, then this routine
    pops the bindings, and the caller does not have to do the cleanup.
------------------------------------------------------------------------ */

void build_network_for_condition_list(agent* thisAgent,
                                      condition* cond_list,
                                      rete_node_level depth_of_first_cond,
                                      rete_node* parent,
                                      rete_node** dest_bottom_node,
                                      rete_node_level* dest_bottom_depth,
                                      cons** dest_vars_bound)
{
    rete_node* node, *new_node, *child, *subconditions_bottom_node;
    condition* cond;
    rete_node_level current_depth;
    cons* vars_bound;

    node = parent;
    current_depth = depth_of_first_cond;
    vars_bound = NIL;

    for (cond = cond_list; cond != NIL; cond = cond->next)
    {
        switch (cond->type)
        {

            case POSITIVE_CONDITION:
                new_node = make_node_for_positive_cond(thisAgent, cond, current_depth, node);
                /* --- Add dense variable bindings for this condition --- */
                bind_variables_in_test(thisAgent, cond->data.tests.id_test, current_depth, 0,
                                       true, &vars_bound);
                bind_variables_in_test(thisAgent, cond->data.tests.attr_test, current_depth, 1,
                                       true, &vars_bound);
                bind_variables_in_test(thisAgent, cond->data.tests.value_test, current_depth, 2,
                                       true, &vars_bound);
                break;

            case NEGATIVE_CONDITION:
                new_node = make_node_for_negative_cond(thisAgent, cond, current_depth, node);
                break;

            case CONJUNCTIVE_NEGATION_CONDITION:
                /* --- first, make the subconditions part of the rete --- */
                build_network_for_condition_list(thisAgent, cond->data.ncc.top, current_depth,
                                                 node, &subconditions_bottom_node, NIL, NIL);
                /* --- look for an existing CN node --- */
                for (child = node->first_child; child != NIL; child = child->next_sibling)
                    if (child->node_type == CN_BNODE)
                        if (child->b.cn.partner->parent == subconditions_bottom_node)
                        {
                            break;
                        }
                /* --- share existing node or build new one --- */
                if (child)
                {
                    new_node = child;
                }
                else
                {
                    new_node = make_new_cn_node(thisAgent, node, subconditions_bottom_node);
                }
                break;

            default:
                new_node = NIL; /* unreachable, but without it gcc -Wall warns here */
        }

        node = new_node;
        current_depth++;
    }

    /* --- return results to caller --- */
    if (dest_bottom_node)
    {
        *dest_bottom_node = node;
    }
    if (dest_bottom_depth)
    {
        *dest_bottom_depth = current_depth - 1;
    }
    if (dest_vars_bound)
    {
        *dest_vars_bound = vars_bound;
    }
    else
    {
        pop_bindings_and_deallocate_list_of_variables(thisAgent, vars_bound);
    }
}













/* ************************************************************************

   SECTION 9:  Production Addition and Excising

   EXTERNAL INTERFACE:
   Add_production_to_rete() adds a given production, with a given LHS,
   to the Rete.  Excise_production_from_rete() removes a given production
   from the Rete.
************************************************************************ */

/* ---------------------------------------------------------------------
                             Same RHS

   Tests whether two RHS's (i.e., action lists) are the same (except
   for function calls).  This is used for finding duplicate productions.
--------------------------------------------------------------------- */

bool same_rhs(action* rhs1, action* rhs2, bool rl_chunk_stop)
{
    action* a1, *a2;

    /* --- Scan through the two RHS's; make sure there's no function calls,
       and make sure the actions are all the same. --- */
    /* --- Warning: this relies on the representation of rhs_value's:
       two of the same funcall will not be equal (==), but two of the
       same symbol, reteloc, or unboundvar will be equal (==). --- */

    a1 = rhs1;
    a2 = rhs2;

    while (a1 && a2)
    {
        if (a1->type == FUNCALL_ACTION)
        {
            if (a2->type == FUNCALL_ACTION)
            {
                return rhs_values_equal(a1->value, a2->value);
            }
            return false;
        }
        if (a1->preference_type != a2->preference_type)
        {
            return false;
        }
        if (!rhs_values_equal(a1->id, a2->id))
        {
            return false;
        }
        if (!rhs_values_equal(a1->attr, a2->attr))
        {
            return false;
        }
        if (!rhs_values_equal(a1->value, a2->value))
        {
            return false;
        }
        if (preference_is_binary(a1->preference_type))
            if (!rhs_values_equal(a1->referent, a2->referent))
            {
                bool stop = true;
                if (rl_chunk_stop)
                {
                    if (rhs_value_is_symbol(a1->referent) && rhs_value_is_symbol(a2->referent))
                    {
                        Symbol* a1r = rhs_value_to_symbol(a1->referent);
                        Symbol* a2r = rhs_value_to_symbol(a2->referent);

                        if (((a1r->symbol_type == INT_CONSTANT_SYMBOL_TYPE) || (a1r->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)) &&
                                ((a2r->symbol_type == INT_CONSTANT_SYMBOL_TYPE) || (a2r->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)))
                        {
                            if (((a1 == rhs1) && (!a1->next)) && ((a2 == rhs2) && (!a2->next)))
                            {
                                stop = false;
                            }
                        }
                    }
                }
                if (stop)
                {
                    return false;
                }
            }
        a1 = a1->next;
        a2 = a2->next;
    }

    /* --- If we reached the end of one RHS but not the other, then
       they must be different --- */
    if (a1 != a2)
    {
        return false;
    }

    /* --- If we got this far, the RHS's must be identical. --- */
    return true;
}

/* ---------------------------------------------------------------------
                    Fixup RHS-Value Variable References

   After we've built the network for a production, we go through its
   RHS and replace all the variables with reteloc's and unboundvar indices.
   For each variable <v> on the RHS, if <v> is bound on the LHS, then
   we replace RHS references to it with a specification of where its
   LHS binding can be found, e.g., "the value field four levels up".
   Each RHS variable <v> not bound on the LHS is replaced with an index,
   e.g., "unbound varible number 6".  As we're doing this, we keep track
   of the names of all the unbound variables.

   When this routine is called, variables should be bound (densely) for
   the entire LHS.
--------------------------------------------------------------------- */


void fixup_rhs_value_variable_references(agent* thisAgent, rhs_value* rv,
        rete_node_level bottom_depth,
        cons*& rhs_unbound_vars_for_new_prod,
        uint64_t& num_rhs_unbound_vars_for_new_prod,
        tc_number rhs_unbound_vars_tc)
{
    cons* c;
    Symbol* sym;
    var_location var_loc;
    var_loc.var_location_struct::levels_up = 0;
    var_loc.var_location_struct::field_num = 0;
    uint64_t index;

    if (rhs_value_is_symbol(*rv))
    {
        sym = rhs_value_to_symbol(*rv);
        if (sym->symbol_type != VARIABLE_SYMBOL_TYPE)
        {
            return;
        }
        /* --- Found a variable.  Is is bound on the LHS? --- */
        if (find_var_location(sym, static_cast<rete_node_level>(bottom_depth + 1), &var_loc))
        {
            /* --- Yes, replace it with reteloc --- */
            thisAgent->symbolManager->symbol_remove_ref(&sym);
            *rv = reteloc_to_rhs_value(var_loc.field_num, var_loc.levels_up - 1);
        }
        else
        {
            /* --- No, replace it with rhs_unboundvar --- */
            if (sym->tc_num != rhs_unbound_vars_tc)
            {
                thisAgent->symbolManager->symbol_add_ref(sym);
                push(thisAgent, sym, rhs_unbound_vars_for_new_prod);
                sym->tc_num = rhs_unbound_vars_tc;
                index = num_rhs_unbound_vars_for_new_prod++;
                sym->var->current_binding_value = reinterpret_cast<Symbol*>(index);
            }
            else
            {
                index = reinterpret_cast<uint64_t>(sym->var->current_binding_value);
            }
            *rv = unboundvar_to_rhs_value(index);
            thisAgent->symbolManager->symbol_remove_ref(&sym);
        }
        return;
    }

    if (rhs_value_is_funcall(*rv))
    {
        for (c = rhs_value_to_funcall_list(*rv)->rest; c != NIL; c = c->rest)
            fixup_rhs_value_variable_references(thisAgent, reinterpret_cast<rhs_value*>(&(c->first)),
                                                bottom_depth, rhs_unbound_vars_for_new_prod,
                                                num_rhs_unbound_vars_for_new_prod,
                                                rhs_unbound_vars_tc);
    }
}

/* ---------------------------------------------------------------------
                    Update Max RHS Unbound Variables

   When a production is fired, we use an array of gensyms to store
   the bindings for the RHS unbound variables.  We have to grow the
   memory block allocated for this array any time a production comes
   along with more RHS unbound variables than we've ever seen before.
   This procedure checks the number of RHS unbound variables for a new
   production, and grows the array if necessary.
--------------------------------------------------------------------- */

void update_max_rhs_unbound_variables(agent* thisAgent, uint64_t num_for_new_production)
{
    if (num_for_new_production > thisAgent->max_rhs_unbound_variables)
    {
        thisAgent->memoryManager->free_memory(thisAgent->rhs_variable_bindings, MISCELLANEOUS_MEM_USAGE);
        thisAgent->max_rhs_unbound_variables = num_for_new_production;
        thisAgent->rhs_variable_bindings = (Symbol**)
                                           thisAgent->memoryManager->allocate_memory_and_zerofill(thisAgent->max_rhs_unbound_variables *
                                                   sizeof(Symbol*), MISCELLANEOUS_MEM_USAGE);
    }
}

/* ---------------------------------------------------------------------
                       Add Production to Rete

   Add_production_to_rete() adds a given production, with a given LHS,
   to the rete.  If "refracted_inst" is non-NIL, it should point to an
   initial instantiation of the production.  This routine returns
   DUPLICATE_PRODUCTION if the production was a duplicate; else
   NO_REFRACTED_INST if no refracted inst. was given; else either
   REFRACTED_INST_MATCHED or REFRACTED_INST_DID_NOT_MATCH.

   The initial refracted instantiation is provided so the initial
   instantiation of a newly-build chunk doesn't get fired.  We handle
   this as follows.  We store the initial instantiation as a "tentative
   retraction" on the new p-node.  Then we inform the p-node of any
   matches (tokens from above).  If any of them is the same as the
   refracted instantiation, then that instantiation will get removed
   from "tentative_retractions".  When the p-node has been informed of
   all matches, we just check whether the instantiation is still on
   tentative_retractions.  If not, there was a match (and the p-node's
   activation routine filled in the token info on the instantiation for
   us).  If so, there was no match for the refracted instantiation.

   BUGBUG should we check for duplicate justifications?
--------------------------------------------------------------------- */

byte add_production_to_rete(agent* thisAgent, production* p, condition* lhs_top, instantiation* refracted_inst, bool warn_on_duplicates, production* &duplicate_rule, bool ignore_rhs)
{
    rete_node* bottom_node, *p_node;
    rete_node_level bottom_depth;
    cons* vars_bound;
    ms_change* msc;
    action* a;
    byte production_addition_result;

    dprint(DT_RETE_PNODE_ADD, "add_production_to_rete called for production %y:\n", p->name);
    dprint(DT_RETE_PNODE_ADD, "instantiation:\n%7", refracted_inst);
    dprint(DT_RETE_PNODE_ADD, "lhs:\n%1", lhs_top);

    /* --- build the network for all the conditions --- */
    build_network_for_condition_list(thisAgent, lhs_top, 1, thisAgent->dummy_top_node,
                                     &bottom_node, &bottom_depth, &vars_bound);

    /* --- change variable names in RHS to Rete location references or
    unbound variable indices --- */
    cons* rhs_unbound_vars_for_new_prod = NIL;
    uint64_t num_rhs_unbound_vars_for_new_prod = 0;
    tc_number rhs_unbound_vars_tc = get_new_tc_number(thisAgent);
    for (a = p->action_list; a != NIL; a = a->next)
    {
        fixup_rhs_value_variable_references(thisAgent, &(a->value), bottom_depth,
                                            rhs_unbound_vars_for_new_prod, num_rhs_unbound_vars_for_new_prod, rhs_unbound_vars_tc);
        if (a->type == MAKE_ACTION)
        {
            fixup_rhs_value_variable_references(thisAgent, &(a->id), bottom_depth,
                                                rhs_unbound_vars_for_new_prod, num_rhs_unbound_vars_for_new_prod, rhs_unbound_vars_tc);
            fixup_rhs_value_variable_references(thisAgent, &(a->attr), bottom_depth,
                                                rhs_unbound_vars_for_new_prod, num_rhs_unbound_vars_for_new_prod, rhs_unbound_vars_tc);
            if (preference_is_binary(a->preference_type))
                fixup_rhs_value_variable_references(thisAgent, &(a->referent), bottom_depth,
                                                    rhs_unbound_vars_for_new_prod, num_rhs_unbound_vars_for_new_prod, rhs_unbound_vars_tc);
        }
    }

    /* --- clean up variable bindings created by build_network...() --- */
    pop_bindings_and_deallocate_list_of_variables(thisAgent, vars_bound);

    update_max_rhs_unbound_variables(thisAgent, num_rhs_unbound_vars_for_new_prod);

    /* --- look for an existing p node that matches --- */
    for (p_node = bottom_node->first_child; p_node != NIL;
            p_node = p_node->next_sibling)
    {
        if (p_node->node_type != P_BNODE)
        {
            continue;
        }
        if (!ignore_rhs && !same_rhs(p_node->b.p.prod->action_list, p->action_list, thisAgent->RL->rl_params->chunk_stop->get_value() == on))
        {
            continue;
        }
        /* Note:   This is a hack to get around an RL template bug that surfaced
         *         after we added identity-based STI variablization. For some
         *         reason, the RETE will now say that the original template that
         *         created the instantiation is a duplicate of the instance, if that
         *         instance had no conditions specialized by the match that created it.
         *         Previously, bottom_node->first_child was null, indicating that
         *         it was not a duplicate.  Not sure why, but this seems to work
         *         for now, though it hasn't been well-tested.  (very limited RL
         *         unit tests as of 3/2016)
         *
         *         Note that we check ignore_rhs because that is false when the
         *         parser calls this function.  By checking its value, Soar should
         *         still detect duplicate templates that are added.  */
        if (ignore_rhs && (p_node->b.p.prod->type == TEMPLATE_PRODUCTION_TYPE))
        {
            continue;
        }
        /* --- duplicate production found --- */
        duplicate_rule = p_node->b.p.prod;
        if (warn_on_duplicates)
        {
            std::stringstream output;
            output << "\nIgnoring "
                   << p->name->to_string(true)
                   << " because it is a duplicate of "
                   << p_node->b.p.prod->name->to_string(true)
                   << " ";
            xml_generate_warning(thisAgent, output.str().c_str());

            thisAgent->outputManager->printa_sf(thisAgent, "Ignoring %y because it is a duplicate of %y\n",
                               p->name, p_node->b.p.prod->name);
        }
        thisAgent->symbolManager->deallocate_symbol_list_removing_references(rhs_unbound_vars_for_new_prod);
//        if (refracted_inst)
//        {
//            insert_at_head_of_dll(p->instantiations, refracted_inst, next, prev);
//            refracted_inst->rete_token = NIL;
//            refracted_inst->rete_wme = NIL;
//            thisAgent->memoryManager->allocate_with_pool(MP_ms_change, &msc);
//            msc->inst = refracted_inst;
//            msc->p_node = p_node;
//            /* Because the RETE 'artificially' refracts this instantiation (ie, it is
//            not actually firing -- the original instantiation fires but not the
//            chunk), we make the refracted instantiation of the chunk a nil_goal
//            retraction, rather than associating it with the activity of its match
//            goal. In p_node_left_addition, where the tentative assertion will be
//            generated, we make it a point to look at the goal value and extract
//            from the appropriate list; here we just make a a simplifying
//            assumption that the goal is NIL (although, in reality), it never will
//            be.  */
//
//            /* This initialization is necessary (for at least safety reasons, for all
//            msc's, regardless of the mode */
//            msc->level = NO_WME_LEVEL;
//            msc->goal = NIL;
//
//            dprint(DT_WATERFALL, " %y is a refracted instantiation\n", refracted_inst->prod_name);
//
//            insert_at_head_of_dll(thisAgent->nil_goal_retractions,
//                                  msc, next_in_level, prev_in_level);
//
//    #ifdef BUG_139_WORKAROUND
//            msc->p_node->b.p.prod->already_fired = 0;       /* RPM workaround for bug #139; mark prod as not fired yet */
//    #endif
//
//            insert_at_head_of_dll(thisAgent->ms_retractions, msc, next, prev);
//            insert_at_head_of_dll(p_node->b.p.tentative_retractions, msc, next_of_node, prev_of_node);
//        }
        return DUPLICATE_PRODUCTION;
    }

    /* --- build a new p node --- */
    p_node = make_new_production_node(thisAgent, bottom_node, p);
    adjust_sharing_factors_from_here_to_top(p_node, 1);


    /* KJC 1/28/98  left these comments in to support REW comments below
    but commented out the operand_mode code  */
    /*

    in operand, we don't want to refract the instantiation.  consider
    this situation: a PE chunk was created during the IE phase.  that
    instantiation shouldn't be applied and we prevent this from
    happening (see chunk_instantiation() in chunk.c).  we eventually get
    to the OUTPUT_PHASE, then the QUIESCENCE_PHASE.  up to this point,
    the chunk hasn't done it's thing.  we start the PE_PHASE.  now, it
    is at this time that the just-built PE chunk should match and fire.
    if we were to refract the chunk, it wouldn't fire it at this point
    and it's actions would never occur.  by not refracting it, we allow
    the chunk to match and fire.

    caveat: we must refract justifications, otherwise they would fire
    and in doing so would produce more chunks/justifications.

    if ((thisAgent->operand_mode == true) && 1)
    if (refracted_inst != NIL) {
    if (refracted_inst->prod->type != JUSTIFICATION_PRODUCTION_TYPE)
    refracted_inst = NIL;
    }
    */
    /* In Operand2, for now, we want both chunks and justifications to be
    treated as refracted instantiations, at least for now.  At some point,
    this issue needs to be re-visited for chunks that immediately match with
    a different instantiation and a different type of support than the
    original, chunk-creating instantiation. */


    /* --- handle initial refraction by adding it to tentative_retractions --- */
    if (refracted_inst)
    {
        insert_at_head_of_dll(p->instantiations, refracted_inst, next, prev);
        refracted_inst->rete_token = NIL;
        refracted_inst->rete_wme = NIL;
        thisAgent->memoryManager->allocate_with_pool(MP_ms_change, &msc);
        msc->inst = refracted_inst;
        msc->p_node = p_node;
        /* Because the RETE 'artificially' refracts this instantiation (ie, it is
        not actually firing -- the original instantiation fires but not the
        chunk), we make the refracted instantiation of the chunk a nil_goal
        retraction, rather than associating it with the activity of its match
        goal. In p_node_left_addition, where the tentative assertion will be
        generated, we make it a point to look at the goal value and extract
        from the appropriate list; here we just make a a simplifying
        assumption that the goal is NIL (although, in reality), it never will
        be.  */

        /* This initialization is necessary (for at least safety reasons, for all
        msc's, regardless of the mode */
        msc->level = NO_WME_LEVEL;
        msc->goal = NIL;

        dprint(DT_WATERFALL, " %y is a refracted instantiation\n", refracted_inst->prod_name);

        insert_at_head_of_dll(thisAgent->nil_goal_retractions,
                              msc, next_in_level, prev_in_level);

#ifdef BUG_139_WORKAROUND
        msc->p_node->b.p.prod->already_fired = 0;       /* RPM workaround for bug #139; mark prod as not fired yet */
#endif

        insert_at_head_of_dll(thisAgent->ms_retractions, msc, next, prev);
        insert_at_head_of_dll(p_node->b.p.tentative_retractions, msc, next_of_node, prev_of_node);
    }

    /* --- call new node's add_left routine with all the parent's tokens --- */
    update_node_with_matches_from_above(thisAgent, p_node);

    /* --- store result indicator --- */
    if (! refracted_inst)
    {
        production_addition_result = NO_REFRACTED_INST;
    }
    else
    {
        remove_from_dll(p->instantiations, refracted_inst, next, prev);
        if (p_node->b.p.tentative_retractions)
        {
            dprint(DT_VARIABLIZATION_MANAGER, "Refracted instantiation did not match!  Printing partial matches...\n");
            dprint_partial_matches(DT_VARIABLIZATION_MANAGER, p_node);

            production_addition_result = REFRACTED_INST_DID_NOT_MATCH;
            msc = p_node->b.p.tentative_retractions;
            p_node->b.p.tentative_retractions = NIL;
            remove_from_dll(thisAgent->ms_retractions, msc, next, prev);
            if (msc->goal)
            {
                remove_from_dll(msc->goal->id->ms_retractions, msc, next_in_level, prev_in_level);
            }
            else
            {
                remove_from_dll(thisAgent->nil_goal_retractions, msc, next_in_level, prev_in_level);
            }


            thisAgent->memoryManager->free_with_pool(MP_ms_change, msc);

        }
        else
        {
            production_addition_result = REFRACTED_INST_MATCHED;
        }
    }
    
    /* The following was used to discard var names.  Not currently compatible with EBC.  If we want to
     * save memory and throw the chunk names out, we'll need to change explanation trace
     * generation to handle a null nvn.  It might just be a matter of gensymm'ing symbols, but
     * we might have to keep track of stuff to get the identities of the gensymmed vars consistent 
     * across conditions and actions. */
    //if ((p->type==CHUNK_PRODUCTION_TYPE) && DISCARD_CHUNK_VARNAMES) {
    //   p->p_node->b.p.parents_nvn = NIL;
    //   p->rhs_unbound_variables = NIL;
    //   thisAgent->symbolManager->deallocate_symbol_list_removing_references (rhs_unbound_vars_for_new_prod);
    //} else {
    //}
    
    /* --- Store variable name information --- */
    p->p_node->b.p.parents_nvn = get_nvn_for_condition_list(thisAgent, lhs_top, NIL);
    p->rhs_unbound_variables =
        destructively_reverse_list(rhs_unbound_vars_for_new_prod);

    /* --- invoke callback functions --- */
    soar_invoke_callbacks(thisAgent, PRODUCTION_JUST_ADDED_CALLBACK, static_cast<soar_call_data>(p));

    return production_addition_result;
}

/* ---------------------------------------------------------------------
                      Excise Production from Rete

   This removes a given production from the Rete net, and enqueues all
   its existing instantiations as pending retractions.
--------------------------------------------------------------------- */

void excise_production_from_rete(agent* thisAgent, production* pProd)
{
    rete_node* p_node, *parent;
    ms_change* msc;

    soar_invoke_callbacks(thisAgent, PRODUCTION_JUST_ABOUT_TO_BE_EXCISED_CALLBACK, static_cast<soar_call_data>(pProd));

    p_node = pProd->p_node;
    pProd->p_node = NIL;      /* mark production as not being in the rete anymore */
    parent = p_node->parent;

    /* --- deallocate the variable name information --- */
    if (p_node->b.p.parents_nvn)
        deallocate_node_varnames(thisAgent, parent, thisAgent->dummy_top_node,
                                 p_node->b.p.parents_nvn);

    /* --- cause all existing instantiations to retract, by removing any
       tokens at the node --- */
    while (p_node->a.np.tokens)
    {
        remove_token_and_subtree(thisAgent, p_node->a.np.tokens);
    }

    /* --- At this point, there are no tentative_assertion's.  Now set
       the p_node field of all tentative_retractions to NIL, to indicate
       that the p_node is being excised  --- */
    for (msc = p_node->b.p.tentative_retractions; msc != NIL; msc = msc->next_of_node)
    {
        msc->p_node = NIL;
    }

    /* --- finally, excise the p_node --- */
    remove_node_from_parents_list_of_children(p_node);
    update_stats_for_destroying_node(thisAgent, p_node);    /* clean up rete stats stuff */
    thisAgent->memoryManager->free_with_pool(MP_rete_node, p_node);

    /* --- update sharing factors on the path from here to the top node --- */
    adjust_sharing_factors_from_here_to_top(parent, -1);

    /* --- and propogate up the net --- */
    if (! parent->first_child)
    {
        deallocate_rete_node(thisAgent, parent);
    }
}













/* **********************************************************************

   SECTION 10:  Building Conditions (instantiated or not) from the Rete Net

   These routines are used for two things.  First, when we want to print
   out the source code for a production, we need to reconstruct its
   conditions and actions.  Second, when we fire a production, we need to
   build its instantiated conditions.  (These are used for run-time
   o-support calculations and for backtracing.)

   Conceptually, we do this all top-down, by starting at the top Rete
   node and walking down to the p-node for the desired production.
   (The actual implementation starts at the p-node, of course, and
   walks its way up the net recursively.)  As we work our way down, at
   each level:
      For instantiating a top-level positive condition:
          Just build a simple instantiated condition by looking at the
          WME it matched.  Also record any "<>" tests.
      For instantiating anything else, or for rebuilding the LHS:
          Look at the Rete node and use it to figure out what the
          LHS condition looked like.

   EXTERNAL INTERFACE:
   P_node_to_conditions_and_nots() takes a p_node and (optionally) a
   token/wme pair, and reconstructs the (optionally instantiated) LHS
   for the production.  It also reconstructs the RHS actions.
   Get_symbol_from_rete_loc() takes a token/wme pair and a location
   specification (levels_up/field_num), examines the match (token/wme),
   and returns the symbol at that location.
********************************************************************** */

/* ----------------------------------------------------------------------
                     Var Bound in Reconstructed Conds

   We're reconstructing the conditions for a production in top-down
   fashion.  Suppose we come to a Rete test checking for equality with
   the "value" field 3 levels up.  In that case, for the current condition,
   we want to include an equality test for whatever variable got bound
   in the value field 3 levels up.  This function scans up the list
   of conditions reconstructed so far, and finds the appropriate variable.
---------------------------------------------------------------------- */

Symbol* var_bound_in_reconstructed_conds(agent* thisAgent,
        condition* cond, /* current cond */
        byte where_field_num,
        rete_node_level where_levels_up)
{
    test t;

    while (where_levels_up)
    {
        where_levels_up--;
        cond = cond->prev;
    }

    if (where_field_num == 0)       t = cond->data.tests.id_test;
    else if (where_field_num == 1)  t = cond->data.tests.attr_test;
    else                            t = cond->data.tests.value_test;

    if (!t) goto abort_var_bound_in_reconstructed_conds;
    t = t->eq_test;

    return t->data.referent;

abort_var_bound_in_reconstructed_conds:
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "Internal error in var_bound_in_reconstructed_conds\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }
    return 0; /* unreachable, but without it, gcc -Wall warns here */
}

test var_test_bound_in_reconstructed_conds(
    agent* thisAgent,
    condition* cond,
    byte where_field_num,
    rete_node_level where_levels_up)
{
    test t;

    while (where_levels_up)
    {
        where_levels_up--;
        cond = cond->prev;
    }

    if (where_field_num == 0)       t = cond->data.tests.id_test;
    else if (where_field_num == 1)  t = cond->data.tests.attr_test;
    else                            t = cond->data.tests.value_test;

    if (!t) goto abort_var_test_bound_in_reconstructed_conds;

    return t->eq_test;

abort_var_test_bound_in_reconstructed_conds:
        {
            char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "Internal error in var_test_bound_in_reconstructed_conds\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }
    return 0; /* unreachable, but without it, gcc -Wall warns here */
}

/* ----------------------------------------------------------------------
                          Rete Node To Conditions

   This is the main routine for reconstructing the LHS source code, and
   for building instantiated conditions when a production is fired.
   It builds the conditions corresponding to the given rete node ("node")
   and all its ancestors, up to the given "cutoff" node.  The given
   node_varnames structure "nvn", if non-NIL, should be the node_varnames
   corresponding to "node".  <tok,w> (if they are non-NIL) specifies the
   token/wme pair that emerged from "node" -- these are used only when
   firing, not when reconstructing.  "conds_for_cutoff_and_up" should be
   the lowermost cond in the already-constructed chain of conditions
   for the "cutoff" node and higher.  "Dest_top_cond" and "dest_bottom_cond"
   get filled in with the highest and lowest conditions built by this
   procedure.

   additional_tests indicates whether we want to add original variables
   and tests to the condition generated (for chunk creation) and false if
   we just want the bound equality tests.

---------------------------------------------------------------------- */

void rete_node_to_conditions(agent* thisAgent,
                             rete_node* node,
                             node_varnames* nvn,
                             rete_node* cutoff,
                             token* tok,
                             wme* w,
                             condition* conds_for_cutoff_and_up,
                             condition** dest_top_cond,
                             condition** dest_bottom_cond,
                             uint64_t pI_id,
                             AddAdditionalTestsMode additional_tests)
{
    condition* cond;
    alpha_mem* am;

    cond = make_condition(thisAgent);
    if (real_parent_node(node) == cutoff)
    {
        cond->prev = conds_for_cutoff_and_up; /* if this is the top of an NCC, this will get replaced by NIL later */
        *dest_top_cond = cond;
    }
    else
    {
        rete_node_to_conditions(thisAgent, real_parent_node(node),
                                nvn ? nvn->parent : NIL,
                                cutoff,
                                tok ? tok->parent : NIL,
                                tok ? tok->w : NIL,
                                conds_for_cutoff_and_up,
                                dest_top_cond, &(cond->prev),
                                pI_id,
                                additional_tests);
        cond->prev->next = cond;
    }
    cond->next = NIL;
    *dest_bottom_cond = cond;

    if (node->node_type == CN_BNODE)
    {
        cond->type = CONJUNCTIVE_NEGATION_CONDITION;
//        dprint(DT_NCC_VARIABLIZATION, "CONJUNCTIVE_NEGATION_CONDITION encountered.  Making recursive call.\n");
        rete_node_to_conditions(thisAgent, node->b.cn.partner->parent,
                                nvn ? nvn->data.bottom_of_subconditions : NIL,
                                node->parent,
                                NIL,
                                NIL,
                                cond->prev,
                                &(cond->data.ncc.top),
                                &(cond->data.ncc.bottom),
                                pI_id,
                                additional_tests);
        cond->data.ncc.top->prev = NIL;
    }
    else
    {
//        dprint(DT_NCC_VARIABLIZATION, "RETE Non-recursive call to rete_node_to_conditions.\n");
        if (bnode_is_positive(node->node_type))
        {
            cond->type = POSITIVE_CONDITION;
//            dprint(DT_NCC_VARIABLIZATION, "POSITIVE_CONDITION encountered:\n");
        }
        else
        {
            cond->type = NEGATIVE_CONDITION;
//            dprint(DT_NCC_VARIABLIZATION, "NEGATIVE_CONDITION encountered.\n");
        }

        if (w && (cond->type == POSITIVE_CONDITION))
        {
            /* --- make equality test for bound symbols.  then add additional tests  --- */

            cond->data.tests.id_test = make_test(thisAgent, w->id, EQUALITY_TEST);
            cond->data.tests.attr_test = make_test(thisAgent, w->attr, EQUALITY_TEST);
            cond->data.tests.value_test = make_test(thisAgent, w->value, EQUALITY_TEST);

            cond->test_for_acceptable_preference = w->acceptable;
            cond->bt.wme_ = w;
            if (additional_tests != DONT_EXPLAIN)
            {
                thisAgent->explanationBasedChunker->add_explanation_to_condition(node, cond, nvn, pI_id, additional_tests);
            }
//            dprint(DT_NCC_VARIABLIZATION, "%l", cond);
        }
        else
        {
            /* -- Note: This portion normally used for printing productions but if this is a simple
             *    negative condition (but not NCC) and w exists, the same code is used to print
             *    out those conditions (since they don't require bindings and use original variables
             *    just like when printing a production). */


            am = node->b.posneg.alpha_mem_;
            if (am->id)
            {
                cond->data.tests.id_test = make_test(thisAgent, am->id, EQUALITY_TEST);
            }
            if (am->attr)
            {
                cond->data.tests.attr_test = make_test(thisAgent, am->attr, EQUALITY_TEST);
            }
            if (am->value)
            {
                cond->data.tests.value_test = make_test(thisAgent, am->value, EQUALITY_TEST);
            }
            cond->test_for_acceptable_preference = am->acceptable;

            if (nvn)
            {
                add_varnames_to_test(thisAgent, nvn->data.fields.id_varnames,
                                     &(cond->data.tests.id_test));
                add_varnames_to_test(thisAgent, nvn->data.fields.attr_varnames,
                                     &(cond->data.tests.attr_test));
                add_varnames_to_test(thisAgent, nvn->data.fields.value_varnames,
                                     &(cond->data.tests.value_test));
            }

            /* --- on hashed nodes, add equality test for the hash function --- */
            if ((node->node_type == MP_BNODE) || (node->node_type == NEGATIVE_BNODE))
            {
                add_hash_info_to_id_test(thisAgent, cond,
                                         node->left_hash_loc_field_num,
                                         node->left_hash_loc_levels_up);
            }
            else if (node->node_type == POSITIVE_BNODE)
            {
                add_hash_info_to_id_test(thisAgent, cond,
                                         node->parent->left_hash_loc_field_num,
                                         node->parent->left_hash_loc_levels_up);
            }

            if (additional_tests != DONT_EXPLAIN)
            {
                thisAgent->explanationBasedChunker->add_explanation_to_condition(node, cond, nvn, pI_id, additional_tests);
            }
            else
            {
            /* --- if there are other tests, add them too --- */
            if (node->b.posneg.other_tests)
            {
                add_rete_test_list_to_tests(thisAgent, cond, node->b.posneg.other_tests);
            }
            }
            /* --- if we threw away the variable names, make sure there's some
               equality test in each of the three fields --- */
            if (! nvn)
            {
                if (!cond->data.tests.id_test || !cond->data.tests.id_test->eq_test)
                {
                    add_gensymmed_equality_test(thisAgent, &(cond->data.tests.id_test), 's');
                }
                if (!cond->data.tests.attr_test || !cond->data.tests.attr_test->eq_test)
                {
                    add_gensymmed_equality_test(thisAgent, &(cond->data.tests.attr_test), 'a');
                }
                if (!cond->data.tests.value_test || !cond->data.tests.value_test->eq_test)
                    add_gensymmed_equality_test(thisAgent, &(cond->data.tests.value_test),
                                                first_letter_from_test(cond->data.tests.attr_test));
            }
        }
    }
}

/* -----------------------------------------------------------------------
                          P Node to Conditions
                       Get Symbol From Rete Loc

   P_node_to_conditions_and_nots() takes a p_node and (optionally) a
   token/wme pair, and reconstructs the (optionally instantiated) LHS
   for the production.  If "dest_rhs" is non-NIL, it also reconstructs
   the RHS actions, and fills in dest_rhs with the action list.
   Note: if tok!=NIL, this routine also returns (in dest_nots) the
   top-level positive "<>" tests.  If tok==NIL, dest_nots is not used.

   Get_symbol_from_rete_loc() takes a token/wme pair and a location
   specification (levels_up/field_num), examines the match (token/wme),
   and returns the symbol at that location.  The firer uses this for
   resolving references in RHS actions to variables bound on the LHS.
----------------------------------------------------------------------- */

void p_node_to_conditions_and_rhs(agent* thisAgent,
                                   rete_node* p_node,
                                   token* tok,
                                   wme* w,
                                   condition** dest_top_cond,
                                   condition** dest_bottom_cond,
                                  action** dest_rhs,
                                  uint64_t pI_id,
                                  AddAdditionalTestsMode additional_tests)
{
    cons* c;
    Symbol** cell;
    int64_t index;
    production* prod;
//	goal_stack_level lowest_level_so_far, match_level;
//    condition* cond;

    prod = p_node->b.p.prod;

    if (tok == NIL)
    {
        w = NIL;    /* just for safety */
    }
    thisAgent->symbolManager->reset_variable_generator(NIL, NIL);  /* we'll be gensymming new vars */
    rete_node_to_conditions(thisAgent,
                            p_node->parent,
                            p_node->b.p.parents_nvn,
                            thisAgent->dummy_top_node,
                            tok, w, NIL,
                            dest_top_cond,
                            dest_bottom_cond,
                            pI_id,
                            additional_tests);


    if (dest_rhs)
    {
        thisAgent->highest_rhs_unboundvar_index = -1;
        if (prod->rhs_unbound_variables)
        {
            cell = thisAgent->rhs_variable_bindings;
            for (c = prod->rhs_unbound_variables; c != NIL; c = c->rest)
            {
                *(cell++) = static_cast<symbol_struct*>(c->first);
                thisAgent->highest_rhs_unboundvar_index++;
            }
        }
        *dest_rhs = create_RHS_action_list(thisAgent, prod->action_list, *dest_bottom_cond, pI_id, additional_tests);
        index = 0;
        cell = thisAgent->rhs_variable_bindings;
        while (index++ <= thisAgent->highest_rhs_unboundvar_index)
        {
            *(cell++) = NIL;
        }
    }
}

Symbol* get_symbol_from_rete_loc(unsigned short levels_up,
                                 byte field_num,
                                 token* tok, wme* w)
{
    while (levels_up)
    {
        levels_up--;
        w = tok->w;
        tok = tok->parent;
    }
    if (field_num == 0)
    {
        return w->id;
    }
    if (field_num == 1)
    {
        return w->attr;
    }
    return w->value;
}


/* **********************************************************************

   SECTION 11:  Rete Test Evaluation Routines

   These routines perform the "other tests" stored at positive and
   negative join nodes.  Each is passed parameters: the rete_test
   to be performed, and the <token,wme> pair on which to perform the
   test.
********************************************************************** */

bool error_rete_test_routine(agent* thisAgent, rete_test* rt, token* left, wme* w);
#define ertr error_rete_test_routine
bool((*(rete_test_routines[256]))
     (agent* thisAgent, rete_test* rt, token* left, wme* w)) =
{
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr,
    ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr, ertr
};

inline bool match_left_and_right(agent* thisAgent, rete_test* _rete_test,
                                 token* left, wme* w)
{
    return ((*(rete_test_routines[(_rete_test)->type])) \
            (thisAgent, (_rete_test), (left), (w)));
}

/* Note:  "=" and "<>" tests always return false when one argument is
   an integer and the other is a floating point number */

#define numcmp(x,y) (((x) < (y)) ? -1 : (((x) > (y)) ? 1 : 0))

/* return -1, 0, or 1 if s1 is less than, equal to, or greater than s2,
 * respectively
 */
inline int64_t compare_symbols(Symbol* s1, Symbol* s2)
{
    switch (s1->symbol_type)
    {
        case INT_CONSTANT_SYMBOL_TYPE:
            switch (s2->symbol_type)
            {
                case INT_CONSTANT_SYMBOL_TYPE:
                    return s1->ic->value - s2->ic->value;
                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    return numcmp(s1->ic->value, s2->fc->value);
                default:
                    return -1;
            }
        case FLOAT_CONSTANT_SYMBOL_TYPE:
            switch (s2->symbol_type)
            {
                case INT_CONSTANT_SYMBOL_TYPE:
                    return numcmp(s1->fc->value, s2->ic->value);
                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    return numcmp(s1->fc->value, s2->fc->value);
                default:
                    return -1;
            }
        case STR_CONSTANT_SYMBOL_TYPE:
            switch (s2->symbol_type)
            {
                case STR_CONSTANT_SYMBOL_TYPE:
                    return strcmp(s1->sc->name, s2->sc->name);
                default:
                    return -1;
            }
        case IDENTIFIER_SYMBOL_TYPE:
            switch (s2->symbol_type)
            {
                case IDENTIFIER_SYMBOL_TYPE:
                    if (s1->id->name_letter == s2->id->name_letter)
                    {
                        return static_cast<int64_t>(s1->id->name_number - s2->id->name_number);
                    }
                    else
                    {
                        return numcmp(s1->id->name_letter, s2->id->name_letter);
                    }
                default:
                    return -1;
            }
        default:
            return -1;
    }
}

bool error_rete_test_routine(agent* thisAgent, rete_test* /*rt*/, token* /*left*/, wme* /*w*/)
{
    char msg[BUFFER_MSG_SIZE];
    strncpy(msg, "Internal error: bad rete test type, hit error_rete_test_routine\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
    return false; /* unreachable, but without it, gcc -Wall warns here */
}

bool id_is_goal_rete_test_routine(agent* /*thisAgent*/, rete_test* /*rt*/, token* /*left*/, wme* w)
{
    return w->id->id->isa_goal;
}

bool id_is_impasse_rete_test_routine(agent* /*thisAgent*/, rete_test* /*rt*/, token* /*left*/, wme* w)
{
    return w->id->id->isa_impasse;
}

bool unary_smem_link_not_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/,
        wme* w)
{
    Symbol* s1;

    s1 = field_from_wme(w, rt->right_field_num);
    return static_cast<bool>(!s1->is_lti());
}

bool unary_smem_link_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/,
        wme* w)
{
    Symbol* s1;

    s1 = field_from_wme(w, rt->right_field_num);
    return static_cast<bool>(s1->is_lti());
}

bool disjunction_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/, wme* w)
{
    Symbol* sym;
    cons* c;

    sym = field_from_wme(w, rt->right_field_num);
    for (c = rt->data.disjunction_list; c != NIL; c = c->rest)
        if (c->first == sym)
        {
            return true;
        }
    return false;
}

bool constant_equal_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/, wme* w)
{
    Symbol* s1, *s2;

    s1 = field_from_wme(w, rt->right_field_num);
    s2 = rt->data.constant_referent;
    return (s1 == s2);
}

bool constant_not_equal_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/,
        wme* w)
{
    Symbol* s1, *s2;

    s1 = field_from_wme(w, rt->right_field_num);
    s2 = rt->data.constant_referent;
    return (s1 != s2);
}

bool constant_less_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/, wme* w)
{
    Symbol* s1, *s2;

    s1 = field_from_wme(w, rt->right_field_num);
    s2 = rt->data.constant_referent;
    return static_cast<bool>(compare_symbols(s1, s2) < 0);
}

bool constant_greater_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/, wme* w)
{
    Symbol* s1, *s2;

    s1 = field_from_wme(w, rt->right_field_num);
    s2 = rt->data.constant_referent;
    return static_cast<bool>(compare_symbols(s1, s2) > 0);
}

bool constant_less_or_equal_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/,
        wme* w)
{
    Symbol* s1, *s2;

    s1 = field_from_wme(w, rt->right_field_num);
    s2 = rt->data.constant_referent;
    return static_cast<bool>(compare_symbols(s1, s2) <= 0);
}

bool constant_greater_or_equal_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/,
        wme* w)
{
    Symbol* s1, *s2;

    s1 = field_from_wme(w, rt->right_field_num);
    s2 = rt->data.constant_referent;
    return static_cast<bool>(compare_symbols(s1, s2) >= 0);
}

bool constant_same_type_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/,
        wme* w)
{
    Symbol* s1, *s2;

    s1 = field_from_wme(w, rt->right_field_num);
    s2 = rt->data.constant_referent;
    return static_cast<bool>(s1->symbol_type == s2->symbol_type);
}

bool constant_smem_link_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/,
        wme* w)
{
    Symbol* s1, *s2;

    s1 = field_from_wme(w, rt->right_field_num);
    s2 = rt->data.constant_referent;
    return static_cast<bool>(s1->is_lti() &&  s2->is_int() && s1->id->LTI_ID == s2->ic->value);
}

bool constant_smem_link_not_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* /*left*/,
        wme* w)
{
    Symbol* s1, *s2;

    s1 = field_from_wme(w, rt->right_field_num);
    s2 = rt->data.constant_referent;
    return static_cast<bool>(!s1->is_lti() ||  !s2->is_int() || (s1->id->LTI_ID != s2->ic->value));
}

bool variable_equal_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* left, wme* w)
{
    Symbol* s1, *s2;
    int i;

    s1 = field_from_wme(w, rt->right_field_num);

    if (rt->data.variable_referent.levels_up != 0)
    {
        i = rt->data.variable_referent.levels_up - 1;
        while (i != 0)
        {
            left = left->parent;
            i--;
        }
        w = left->w;
    }
    s2 = field_from_wme(w, rt->data.variable_referent.field_num);

    return (s1 == s2);
}

bool variable_not_equal_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* left,
        wme* w)
{
    Symbol* s1, *s2;
    int i;

    s1 = field_from_wme(w, rt->right_field_num);

    if (rt->data.variable_referent.levels_up != 0)
    {
        i = rt->data.variable_referent.levels_up - 1;
        while (i != 0)
        {
            left = left->parent;
            i--;
        }
        w = left->w;
    }
    s2 = field_from_wme(w, rt->data.variable_referent.field_num);

    return (s1 != s2);
}

bool variable_less_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* left, wme* w)
{
    Symbol* s1, *s2;
    int i;

    s1 = field_from_wme(w, rt->right_field_num);

    if (rt->data.variable_referent.levels_up != 0)
    {
        i = rt->data.variable_referent.levels_up - 1;
        while (i != 0)
        {
            left = left->parent;
            i--;
        }
        w = left->w;
    }
    s2 = field_from_wme(w, rt->data.variable_referent.field_num);

    return static_cast<bool>(compare_symbols(s1, s2) < 0);
}

bool variable_greater_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* left, wme* w)
{
    Symbol* s1, *s2;
    int i;

    s1 = field_from_wme(w, rt->right_field_num);

    if (rt->data.variable_referent.levels_up != 0)
    {
        i = rt->data.variable_referent.levels_up - 1;
        while (i != 0)
        {
            left = left->parent;
            i--;
        }
        w = left->w;
    }
    s2 = field_from_wme(w, rt->data.variable_referent.field_num);

    return static_cast<bool>(compare_symbols(s1, s2) > 0);
}

bool variable_less_or_equal_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* left,
        wme* w)
{
    Symbol* s1, *s2;
    int i;

    s1 = field_from_wme(w, rt->right_field_num);

    if (rt->data.variable_referent.levels_up != 0)
    {
        i = rt->data.variable_referent.levels_up - 1;
        while (i != 0)
        {
            left = left->parent;
            i--;
        }
        w = left->w;
    }
    s2 = field_from_wme(w, rt->data.variable_referent.field_num);

    return static_cast<bool>(compare_symbols(s1, s2) <= 0);
}

bool variable_greater_or_equal_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* left,
        wme* w)
{
    Symbol* s1, *s2;
    int i;

    s1 = field_from_wme(w, rt->right_field_num);

    if (rt->data.variable_referent.levels_up != 0)
    {
        i = rt->data.variable_referent.levels_up - 1;
        while (i != 0)
        {
            left = left->parent;
            i--;
        }
        w = left->w;
    }
    s2 = field_from_wme(w, rt->data.variable_referent.field_num);

    return static_cast<bool>(compare_symbols(s1, s2) >= 0);
}

bool variable_same_type_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* left,
        wme* w)
{
    Symbol* s1, *s2;
    int i;

    s1 = field_from_wme(w, rt->right_field_num);

    if (rt->data.variable_referent.levels_up != 0)
    {
        i = rt->data.variable_referent.levels_up - 1;
        while (i != 0)
        {
            left = left->parent;
            i--;
        }
        w = left->w;
    }
    s2 = field_from_wme(w, rt->data.variable_referent.field_num);
    return (s1->symbol_type == s2->symbol_type);
}


bool variable_smem_link_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* left,
        wme* w)
{
    Symbol* s1, *s2;
    int i;

    s1 = field_from_wme(w, rt->right_field_num);

    if (rt->data.variable_referent.levels_up != 0)
    {
        i = rt->data.variable_referent.levels_up - 1;
        while (i != 0)
        {
            left = left->parent;
            i--;
        }
        w = left->w;
    }
    s2 = field_from_wme(w, rt->data.variable_referent.field_num);
    return (s1->is_lti() && s2->is_lti() && (s1->id->LTI_ID == s2->id->LTI_ID));
}

bool variable_smem_link_not_rete_test_routine(agent* /*thisAgent*/, rete_test* rt, token* left,
        wme* w)
{
    Symbol* s1, *s2;
    int i;
    bool return_val = false;

    s1 = field_from_wme(w, rt->right_field_num);

    if (rt->data.variable_referent.levels_up != 0)
    {
        i = rt->data.variable_referent.levels_up - 1;
        while (i != 0)
        {
            left = left->parent;
            i--;
        }
        w = left->w;
    }
    s2 = field_from_wme(w, rt->data.variable_referent.field_num);
    if (!s1->is_lti() || !s2->is_lti())
    {
        return_val = true;
    } else if (s1->id->LTI_ID != s2->id->LTI_ID) {
        return_val = true;
    }
    return return_val;
}


/* ************************************************************************

   SECTION 12:  Beta Node Interpreter Routines: Mem, Pos, and MP Nodes

************************************************************************ */

void positive_node_left_addition(agent* thisAgent, rete_node* node, token* New,
                                 Symbol* hash_referent);
void unhashed_positive_node_left_addition(agent* thisAgent, rete_node* node, token* New);

void rete_error_left(agent* thisAgent, rete_node* node, token* /*t*/, wme* /*w*/)
{
    char msg[BUFFER_MSG_SIZE];
    SNPRINTF(msg, BUFFER_MSG_SIZE, "Rete net error:  tried to left-activate node of type %d\n",
             node->node_type);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
}

void rete_error_right(agent* thisAgent, rete_node* node, wme* /*w*/)
{
    char msg[BUFFER_MSG_SIZE];
    SNPRINTF(msg, BUFFER_MSG_SIZE, "Rete net error:  tried to right-activate node of type %d\n",
             node->node_type);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
}

void beta_memory_node_left_addition(agent* thisAgent, rete_node* node,
                                    token* tok, wme* w)
{
    uint32_t hv;
    Symbol* referent;
    rete_node* child, *next;
    token* New;

    activation_entry_sanity_check();
    left_node_activation(node, true);

    {
        int levels_up;
        token* t;

        levels_up = node->left_hash_loc_levels_up;
        if (levels_up == 1)
        {
            referent = field_from_wme(w, node->left_hash_loc_field_num);
        }
        else     /* --- levels_up > 1 --- */
        {
            for (t = tok, levels_up -= 2; levels_up != 0; levels_up--)
            {
                t = t->parent;
            }
            referent = field_from_wme(t->w, node->left_hash_loc_field_num);
        }
    }

    hv = node->node_id ^ referent->hash_id;

    /* --- build new left token, add it to the hash table --- */
    token_added(node);
    thisAgent->memoryManager->allocate_with_pool(MP_token, &New);
    new_left_token(New, node, tok, w);
    insert_token_into_left_ht(thisAgent, New, hv);
    New->a.ht.referent = referent;

    /* --- inform each linked child (positive join) node --- */
    for (child = node->b.mem.first_linked_child; child != NIL; child = next)
    {
        next = child->a.pos.next_from_beta_mem;
        positive_node_left_addition(thisAgent, child, New, referent);
    }
    activation_exit_sanity_check();
}

void unhashed_beta_memory_node_left_addition(agent* thisAgent,
        rete_node* node, token* tok,
        wme* w)
{
    uint32_t hv;
    rete_node* child, *next;
    token* New;

    activation_entry_sanity_check();
    left_node_activation(node, true);

    hv = node->node_id;

    /* --- build new left token, add it to the hash table --- */
    token_added(node);
    thisAgent->memoryManager->allocate_with_pool(MP_token, &New);
    new_left_token(New, node, tok, w);
    insert_token_into_left_ht(thisAgent, New, hv);
    New->a.ht.referent = NIL;

    /* --- inform each linked child (positive join) node --- */
    for (child = node->b.mem.first_linked_child; child != NIL; child = next)
    {
        next = child->a.pos.next_from_beta_mem;
        unhashed_positive_node_left_addition(thisAgent, child, New);
    }
    activation_exit_sanity_check();
}

void positive_node_left_addition(agent* thisAgent,
                                 rete_node* node, token* New,
                                 Symbol* hash_referent)
{
    uint32_t right_hv;
    right_mem* rm;
    alpha_mem* am;
    rete_test* rt;
    bool failed_a_test;
    rete_node* child;

    activation_entry_sanity_check();
    left_node_activation(node, true);

    am = node->b.posneg.alpha_mem_;

    if (node_is_right_unlinked(node))
    {
        relink_to_right_mem(node);
        if (am->right_mems == NIL)
        {
            unlink_from_left_mem(node);
            activation_exit_sanity_check();
            return;
        }
    }

    /* --- look through right memory for matches --- */
    right_hv = am->am_id ^ hash_referent->hash_id;
    for (rm = right_ht_bucket(thisAgent, right_hv); rm != NIL; rm = rm->next_in_bucket)
    {
        if (rm->am != am)
        {
            continue;
        }
        /* --- does rm->w match New? --- */
        if (hash_referent != rm->w->id)
        {
            continue;
        }
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, New, rm->w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found, so call each child node --- */
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, New, rm->w);
        }
    }
    activation_exit_sanity_check();
}

void unhashed_positive_node_left_addition(agent* thisAgent, rete_node* node, token* New)
{
    right_mem* rm;
    rete_test* rt;
    bool failed_a_test;
    rete_node* child;

    activation_entry_sanity_check();
    left_node_activation(node, true);

    if (node_is_right_unlinked(node))
    {
        relink_to_right_mem(node);
        if (node->b.posneg.alpha_mem_->right_mems == NIL)
        {
            unlink_from_left_mem(node);
            activation_exit_sanity_check();
            return;
        }
    }

    /* --- look through right memory for matches --- */
    for (rm = node->b.posneg.alpha_mem_->right_mems; rm != NIL;
            rm = rm->next_in_am)
    {
        /* --- does rm->w match new? --- */
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, New, rm->w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found, so call each child node --- */
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, New, rm->w);
        }
    }
    activation_exit_sanity_check();
}

void mp_node_left_addition(agent* thisAgent, rete_node* node, token* tok, wme* w)
{
    uint32_t hv;
    Symbol* referent;
    rete_node* child;
    token* New;
    uint32_t right_hv;
    right_mem* rm;
    alpha_mem* am;
    rete_test* rt;
    bool failed_a_test;

    //dprint(DT_RETE_PNODE_ADD, "mp_node_left_addition called with node %d, token %u, and wme %w\n", node->node_id, tok, w);
    activation_entry_sanity_check();
    left_node_activation(node, true);

    {
        int levels_up;
        token* t;

        levels_up = node->left_hash_loc_levels_up;
        if (levels_up == 1)
        {
            referent = field_from_wme(w, node->left_hash_loc_field_num);
        }
        else     /* --- levels_up > 1 --- */
        {
            for (t = tok, levels_up -= 2; levels_up != 0; levels_up--)
            {
                t = t->parent;
            }
            referent = field_from_wme(t->w, node->left_hash_loc_field_num);
        }
    }

    hv = node->node_id ^ referent->hash_id;

    /* --- build new left token, add it to the hash table --- */
    token_added(node);
    thisAgent->memoryManager->allocate_with_pool(MP_token, &New);
    new_left_token(New, node, tok, w);
    insert_token_into_left_ht(thisAgent, New, hv);
    New->a.ht.referent = referent;

    if (mp_bnode_is_left_unlinked(node))
    {
        activation_exit_sanity_check();
        return;
    }

    am = node->b.posneg.alpha_mem_;

    if (node_is_right_unlinked(node))
    {
        relink_to_right_mem(node);
        if (am->right_mems == NIL)
        {
            make_mp_bnode_left_unlinked(node);
            activation_exit_sanity_check();
            return;
        }
    }

    /* --- look through right memory for matches --- */
    right_hv = am->am_id ^ referent->hash_id;
    for (rm = right_ht_bucket(thisAgent, right_hv); rm != NIL; rm = rm->next_in_bucket)
    {
        if (rm->am != am)
        {
            continue;
        }
        /* --- does rm->w match new? --- */
        if (referent != rm->w->id)
        {
            continue;
        }
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
        {
            if (! match_left_and_right(thisAgent, rt, New, rm->w))
            {
                failed_a_test = true;
                break;
            }
        }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found, so call each child node --- */
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, New, rm->w);
        }
    }
    activation_exit_sanity_check();
}


void unhashed_mp_node_left_addition(agent* thisAgent, rete_node* node,
                                    token* tok, wme* w)
{
    uint32_t hv;
    rete_node* child;
    token* New;
    right_mem* rm;
    rete_test* rt;
    bool failed_a_test;

    activation_entry_sanity_check();
    left_node_activation(node, true);

    hv = node->node_id;

    /* --- build new left token, add it to the hash table --- */
    token_added(node);
    thisAgent->memoryManager->allocate_with_pool(MP_token, &New);
    new_left_token(New, node, tok, w);
    insert_token_into_left_ht(thisAgent, New, hv);
    New->a.ht.referent = NIL;

    if (mp_bnode_is_left_unlinked(node))
    {
        return;
    }

    if (node_is_right_unlinked(node))
    {
        relink_to_right_mem(node);
        if (node->b.posneg.alpha_mem_->right_mems == NIL)
        {
            make_mp_bnode_left_unlinked(node);
            activation_exit_sanity_check();
            return;
        }
    }

    /* --- look through right memory for matches --- */
    for (rm = node->b.posneg.alpha_mem_->right_mems; rm != NIL;
            rm = rm->next_in_am)
    {
        /* --- does rm->w match new? --- */
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, New, rm->w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found, so call each child node --- */
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, New, rm->w);
        }
    }
    activation_exit_sanity_check();
}

void positive_node_right_addition(agent* thisAgent, rete_node* node, wme* w)
{
    uint32_t hv;
    token* tok;
    Symbol* referent;
    rete_test* rt;
    bool failed_a_test;
    rete_node* child;

    activation_entry_sanity_check();
    right_node_activation(node, true);

    if (node_is_left_unlinked(node))
    {
        relink_to_left_mem(node);
        if (! node->parent->a.np.tokens)
        {
            unlink_from_right_mem(node);
            activation_exit_sanity_check();
            return;
        }
    }

    referent = w->id;
    hv = node->parent->node_id ^ referent->hash_id;

    for (tok = left_ht_bucket(thisAgent, hv); tok != NIL; tok = tok->a.ht.next_in_bucket)
    {
        if (tok->node != node->parent)
        {
            continue;
        }
        /* --- does tok match w? --- */
        if (tok->a.ht.referent != referent)
        {
            continue;
        }
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, tok, w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found, so call each child node --- */
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, tok, w);
        }
    }
    activation_exit_sanity_check();
}

void unhashed_positive_node_right_addition(agent* thisAgent, rete_node* node, wme* w)
{
    uint32_t hv;
    token* tok;
    rete_test* rt;
    bool failed_a_test;
    rete_node* child;

    activation_entry_sanity_check();
    right_node_activation(node, true);

    if (node_is_left_unlinked(node))
    {
        relink_to_left_mem(node);
        if (! node->parent->a.np.tokens)
        {
            unlink_from_right_mem(node);
            activation_exit_sanity_check();
            return;
        }
    }

    hv = node->parent->node_id;

    for (tok = left_ht_bucket(thisAgent, hv); tok != NIL; tok = tok->a.ht.next_in_bucket)
    {
        if (tok->node != node->parent)
        {
            continue;
        }
        /* --- does tok match w? --- */
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, tok, w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found, so call each child node --- */
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, tok, w);
        }
    }
    activation_exit_sanity_check();
}

void mp_node_right_addition(agent* thisAgent, rete_node* node, wme* w)
{
    uint32_t hv;
    token* tok;
    Symbol* referent;
    rete_test* rt;
    bool failed_a_test;
    rete_node* child;
    //static uint64_t callcount = 0;
    //uint64_t lastcallcount = 0;

    //dprint(DT_RETE_PNODE_ADD, "mp_node_right_addition called for %u time with node %d and wme %w\n", ++callcount, node->node_id, w);
    //lastcallcount = callcount;

    activation_entry_sanity_check();
    right_node_activation(node, true);

    if (mp_bnode_is_left_unlinked(node))
    {
        make_mp_bnode_left_linked(node);
        if (! node->a.np.tokens)
        {
            unlink_from_right_mem(node);
            activation_exit_sanity_check();
            return;
        }
    }

    referent = w->id;
    hv = node->node_id ^ referent->hash_id;
    //uint64_t childcount = 0;

    //dprint(DT_RETE_PNODE_ADD, "Starting token list we're iterating through: ");
    //for (tok = left_ht_bucket(thisAgent, hv); tok != NIL; tok = tok->a.ht.next_in_bucket)
    //{
    //    childcount++;
    //    dprint(DT_RETE_PNODE_ADD, "%u: %w", tok, tok->w);
    //}
    //dprint(DT_RETE_PNODE_ADD, "\nnum children: %u\n", childcount);
    //childcount = 0;

    for (tok = left_ht_bucket(thisAgent, hv); tok != NIL; tok = tok->a.ht.next_in_bucket)
    {
        //dprint(DT_RETE_PNODE_ADD, "mp_node_right_addition checking token %u, (#%u) for node %d \n", tok, ++childcount, node->node_id);
        //dprint(DT_RETE_PNODE_ADD, "tok: %u, next: %u\n", tok, tok->a.ht.next_in_bucket);

        if (tok->node != node)
        {
            continue;
        }
        /* --- does tok match w? --- */
        if (tok->a.ht.referent != referent)
        {
            continue;
        }
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
        {
            if (! match_left_and_right(thisAgent, rt, tok, w))
            {
                failed_a_test = true;
                break;
            }
        }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found, so call each child node --- */
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, tok, w);
        }
        //dprint(DT_RETE_PNODE_ADD, "left_addition done.  end of loop.  tok: %u, next: %u\n", tok, tok->a.ht.next_in_bucket);
    }
    activation_exit_sanity_check();
    //dprint(DT_RETE_PNODE_ADD, "mp_node_right_addition finished for %u time with node %d and wme %w\n", ++callcount, node->node_id, w);
}

void unhashed_mp_node_right_addition(agent* thisAgent, rete_node* node, wme* w)
{
    uint32_t hv;
    token* tok;
    rete_test* rt;
    bool failed_a_test;
    rete_node* child;

    activation_entry_sanity_check();
    right_node_activation(node, true);

    if (mp_bnode_is_left_unlinked(node))
    {
        make_mp_bnode_left_linked(node);
        if (! node->a.np.tokens)
        {
            unlink_from_right_mem(node);
            activation_exit_sanity_check();
            return;
        }
    }

    hv = node->node_id;

    for (tok = left_ht_bucket(thisAgent, hv); tok != NIL; tok = tok->a.ht.next_in_bucket)
    {
        if (tok->node != node)
        {
            continue;
        }
        /* --- does tok match w? --- */
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, tok, w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found, so call each child node --- */
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, tok, w);
        }
    }
    activation_exit_sanity_check();
}

/* ************************************************************************

   SECTION 13:  Beta Node Interpreter Routines: Negative Nodes

************************************************************************ */

void negative_node_left_addition(agent* thisAgent, rete_node* node,
                                 token* tok, wme* w)
{
    uint32_t hv, right_hv;
    Symbol* referent;
    right_mem* rm;
    alpha_mem* am;
    rete_test* rt;
    bool failed_a_test;
    rete_node* child;
    token* New;

    activation_entry_sanity_check();
    left_node_activation(node, true);

    if (node_is_right_unlinked(node))
    {
        relink_to_right_mem(node);
    }

    {
        int levels_up;
        token* t;

        levels_up = node->left_hash_loc_levels_up;
        if (levels_up == 1)
        {
            referent = field_from_wme(w, node->left_hash_loc_field_num);
        }
        else     /* --- levels_up > 1 --- */
        {
            for (t = tok, levels_up -= 2; levels_up != 0; levels_up--)
            {
                t = t->parent;
            }
            referent = field_from_wme(t->w, node->left_hash_loc_field_num);
        }
    }

    hv = node->node_id ^ referent->hash_id;

    /* --- build new token, add it to the hash table --- */
    token_added(node);
    thisAgent->memoryManager->allocate_with_pool(MP_token, &New);
    new_left_token(New, node, tok, w);
    insert_token_into_left_ht(thisAgent, New, hv);
    New->a.ht.referent = referent;
    New->negrm_tokens = NIL;

    /* --- look through right memory for matches --- */
    am = node->b.posneg.alpha_mem_;
    right_hv = am->am_id ^ referent->hash_id;
    for (rm = right_ht_bucket(thisAgent, right_hv); rm != NIL; rm = rm->next_in_bucket)
    {
        if (rm->am != am)
        {
            continue;
        }
        /* --- does rm->w match new? --- */
        if (referent != rm->w->id)
        {
            continue;
        }
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, New, rm->w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        {
            token* t;
            thisAgent->memoryManager->allocate_with_pool(MP_token, &t);
            t->node = node;
            t->parent = NIL;
            t->w = rm->w;
            t->a.neg.left_token = New;
            insert_at_head_of_dll(rm->w->tokens, t, next_from_wme, prev_from_wme);
            t->first_child = NIL;
            insert_at_head_of_dll(New->negrm_tokens, t,
                                  a.neg.next_negrm, a.neg.prev_negrm);
        }
    }

    /* --- if no matches were found, call each child node --- */
    if (! New->negrm_tokens)
    {
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, New, NIL);
        }
    }
    activation_exit_sanity_check();
}

void unhashed_negative_node_left_addition(agent* thisAgent, rete_node* node,
        token* tok, wme* w)
{
    uint32_t hv;
    rete_test* rt;
    bool failed_a_test;
    right_mem* rm;
    rete_node* child;
    token* New;

    activation_entry_sanity_check();
    left_node_activation(node, true);

    if (node_is_right_unlinked(node))
    {
        relink_to_right_mem(node);
    }

    hv = node->node_id;

    /* --- build new token, add it to the hash table --- */
    token_added(node);
    thisAgent->memoryManager->allocate_with_pool(MP_token, &New);
    new_left_token(New, node, tok, w);
    insert_token_into_left_ht(thisAgent, New, hv);
    New->a.ht.referent = NIL;
    New->negrm_tokens = NIL;

    /* --- look through right memory for matches --- */
    for (rm = node->b.posneg.alpha_mem_->right_mems; rm != NIL; rm = rm->next_in_am)
    {
        /* --- does rm->w match new? --- */
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, New, rm->w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        {
            token* t;
            thisAgent->memoryManager->allocate_with_pool(MP_token, &t);
            t->node = node;
            t->parent = NIL;
            t->w = rm->w;
            t->a.neg.left_token = New;
            insert_at_head_of_dll(rm->w->tokens, t, next_from_wme, prev_from_wme);
            t->first_child = NIL;
            insert_at_head_of_dll(New->negrm_tokens, t,
                                  a.neg.next_negrm, a.neg.prev_negrm);
        }
    }

    /* --- if no matches were found, call each child node --- */
    if (! New->negrm_tokens)
    {
        for (child = node->first_child; child != NIL; child = child->next_sibling)
        {
            (*(left_addition_routines[child->node_type]))(thisAgent, child, New, NIL);
        }
    }
    activation_exit_sanity_check();
}

void negative_node_right_addition(agent* thisAgent, rete_node* node, wme* w)
{
    uint32_t hv;
    token* tok;
    Symbol* referent;
    rete_test* rt;
    bool failed_a_test;

    activation_entry_sanity_check();
    right_node_activation(node, true);

    referent = w->id;
    hv = node->node_id ^ referent->hash_id;

    for (tok = left_ht_bucket(thisAgent, hv); tok != NIL; tok = tok->a.ht.next_in_bucket)
    {
        if (tok->node != node)
        {
            continue;
        }
        /* --- does tok match w? --- */
        if (tok->a.ht.referent != referent)
        {
            continue;
        }
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, tok, w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found: build new negrm token, remove descendent tokens --- */
        {
            token* t;
            thisAgent->memoryManager->allocate_with_pool(MP_token, &t);
            t->node = node;
            t->parent = NIL;
            t->w = w;
            t->a.neg.left_token = tok;
            insert_at_head_of_dll(w->tokens, t, next_from_wme, prev_from_wme);
            t->first_child = NIL;
            insert_at_head_of_dll(tok->negrm_tokens, t,
                                  a.neg.next_negrm, a.neg.prev_negrm);
        }
        while (tok->first_child)
        {
            remove_token_and_subtree(thisAgent, tok->first_child);
        }
    }
    activation_exit_sanity_check();
}

void unhashed_negative_node_right_addition(agent* thisAgent, rete_node* node, wme* w)
{
    uint32_t hv;
    token* tok;
    rete_test* rt;
    bool failed_a_test;

    activation_entry_sanity_check();
    right_node_activation(node, true);

    hv = node->node_id;

    for (tok = left_ht_bucket(thisAgent, hv); tok != NIL; tok = tok->a.ht.next_in_bucket)
    {
        if (tok->node != node)
        {
            continue;
        }
        /* --- does tok match w? --- */
        failed_a_test = false;
        for (rt = node->b.posneg.other_tests; rt != NIL; rt = rt->next)
            if (! match_left_and_right(thisAgent, rt, tok, w))
            {
                failed_a_test = true;
                break;
            }
        if (failed_a_test)
        {
            continue;
        }
        /* --- match found: build new negrm token, remove descendent tokens --- */
        {
            token* t;
            thisAgent->memoryManager->allocate_with_pool(MP_token, &t);
            t->node = node;
            t->parent = NIL;
            t->w = w;
            t->a.neg.left_token = tok;
            insert_at_head_of_dll(w->tokens, t, next_from_wme, prev_from_wme);
            t->first_child = NIL;
            insert_at_head_of_dll(tok->negrm_tokens, t,
                                  a.neg.next_negrm, a.neg.prev_negrm);
        }
        while (tok->first_child)
        {
            remove_token_and_subtree(thisAgent, tok->first_child);
        }
    }
    activation_exit_sanity_check();
}

/* ************************************************************************

   SECTION 14:  Beta Node Interpreter Routines: CN and CN_PARTNER Nodes

   These routines can support either the CN node hearing about new left
   tokens before the CN_PARTNER, or vice-versa.  This makes them a bit
   more complex than they would be otherwise.
************************************************************************ */

void cn_node_left_addition(agent* thisAgent, rete_node* node, token* tok, wme* w)
{
    uint32_t hv;
    token* t, *New;
    rete_node* child;

    activation_entry_sanity_check();
    left_node_activation(node, true);

    hv = node->node_id ^ cast_and_possibly_truncate<uint32_t>(tok) ^ cast_and_possibly_truncate<uint32_t>(w);

    /* --- look for a matching left token (since the partner node might have
       heard about this new token already, in which case it would have done
       the CN node's work already); if found, exit --- */
    for (t = left_ht_bucket(thisAgent, hv); t != NIL; t = t->a.ht.next_in_bucket)
        if ((t->node == node) && (t->parent == tok) && (t->w == w))
        {
            return;
        }

    /* --- build left token, add it to the hash table --- */
    token_added(node);
    thisAgent->memoryManager->allocate_with_pool(MP_token, &New);
    new_left_token(New, node, tok, w);
    insert_token_into_left_ht(thisAgent, New, hv);
    New->negrm_tokens = NIL;

    /* --- pass the new token on to each child node --- */
    for (child = node->first_child; child != NIL; child = child->next_sibling)
    {
        (*(left_addition_routines[child->node_type]))(thisAgent, child, New, NIL);
    }

    activation_exit_sanity_check();
}

void cn_partner_node_left_addition(agent* thisAgent, rete_node* node,
                                   token* tok, wme* w)
{
    rete_node* partner, *temp;
    uint32_t hv;
    token* left, *negrm_tok;

    activation_entry_sanity_check();
    left_node_activation(node, true);

    partner = node->b.cn.partner;

    /* --- build new negrm token --- */
    token_added(node);
    thisAgent->memoryManager->allocate_with_pool(MP_token, &negrm_tok);
    new_left_token(negrm_tok, node, tok, w);

    /* --- advance (tok,w) up to the token from the top of the branch --- */
    temp = node->parent;
    while (temp != partner->parent)
    {
        temp = real_parent_node(temp);
        w = tok->w;
        tok = tok->parent;
    }

    /* --- look for the matching left token --- */
    hv = partner->node_id ^ cast_and_possibly_truncate<uint32_t>(tok) ^ cast_and_possibly_truncate<uint32_t>(w);
    for (left = left_ht_bucket(thisAgent, hv); left != NIL; left = left->a.ht.next_in_bucket)
        if ((left->node == partner) && (left->parent == tok) && (left->w == w))
        {
            break;
        }

    /* --- if not found, create a new left token --- */
    if (!left)
    {
        token_added(partner);
        thisAgent->memoryManager->allocate_with_pool(MP_token, &left);
        new_left_token(left, partner, tok, w);
        insert_token_into_left_ht(thisAgent, left, hv);
        left->negrm_tokens = NIL;
    }

    /* --- add new negrm token to the left token --- */
    negrm_tok->a.neg.left_token = left;
    insert_at_head_of_dll(left->negrm_tokens, negrm_tok,
                          a.neg.next_negrm, a.neg.prev_negrm);

    /* --- remove any descendent tokens of the left token --- */
    while (left->first_child)
    {
        remove_token_and_subtree(thisAgent, left->first_child);
    }

    activation_exit_sanity_check();
}

/* ************************************************************************

   SECTION 15:  Beta Node Interpreter Routines: Production Nodes

   During each elaboration cycle, we buffer the assertions (new matches)
   and retractions (old no-longer-present matches) in "tentative_assertions"
   and "tentative_retractions" on each p-node.  We have to buffer them
   because a match could appear and then disappear during one e-cycle
   (e.g., add one WME, this creates a match, then remove another WME,
   and the match goes away).  A match can also disappear then re-appear
   (example case involves an NCC -- create a match fot the NCC by adding
   a WME inside it, then remove another WME for a different condition
   inside the NCC).  When one of these "strobe" situations occurs,
   we don't want to actually fire the production or retract the
   instantiation -- hence the buffering.
************************************************************************ */

/* ----------------------------------------------------------------------
                         P Node Left Addition

   Algorithm:

   Does this token match (wme's equal) one of tentative_retractions?
     (We have to check instantiation structure for this--when an
     instantiation retracts then re-asserts in one e-cycle, the
     token itself will be different, but all the wme's tested positively
     will be the same.)
   If so, remove that tentative_retraction.
   If not, store this new token in tentative_assertions.
---------------------------------------------------------------------- */

void p_node_left_addition(agent* thisAgent, rete_node* node, token* tok, wme* w)
{
    ms_change* msc;
    condition* cond;
    token* current_token, *New;
    wme* current_wme;
    rete_node* current_node;
    bool match_found;


    /* RCHONG: begin 10.11 */

    int prod_type;
    token* OPERAND_curr_tok, *temp_tok;

    action*    act;
    bool      operator_proposal, op_elab;

    int pass;
    wme* lowest_goal_wme;

    /* RCHONG: end 10.11 */

    //dprint(DT_RETE_PNODE_ADD, "p_node_left_addition called with node %d, token %u, and wme %w\n", node->node_id, tok, w);
    activation_entry_sanity_check();
    left_node_activation(node, true);

    /* --- build new left token (used only for tree-based remove) --- */
    token_added(node);
    thisAgent->memoryManager->allocate_with_pool(MP_token, &New);
    new_left_token(New, node, tok, w);

    /* --- check for match in tentative_retractions --- */
    match_found = false;
    for (msc = node->b.p.tentative_retractions; msc != NIL; msc = msc->next_of_node)
    {
        match_found = true;
        cond = msc->inst->bottom_of_instantiated_conditions;
        current_token = tok;
        current_wme = w;
        current_node = node->parent;
        while (current_node->node_type != DUMMY_TOP_BNODE)
        {
            if (bnode_is_positive(current_node->node_type))
                if (current_wme != cond->bt.wme_)
                {
                    match_found = false;
                    break;
                }
            current_node = real_parent_node(current_node);
            current_wme = current_token->w;
            current_token = current_token->parent;
            cond = cond->prev;
        }
        if (match_found)
        {
            break;
        }
    }

#ifdef BUG_139_WORKAROUND
    /* --- test workaround for bug #139: don't rematch justifications; let them be removed --- */
    /* note that the justification is added to the retraction list when it is first created, so
    we let it match the first time, but not after that */
    if (match_found && node->b.p.prod->type == JUSTIFICATION_PRODUCTION_TYPE)
    {
        if (node->b.p.prod->already_fired)
        {
            return;
        }
        else
        {
            node->b.p.prod->already_fired = 1;
        }
    }
#endif

    /* --- if match found tentative_retractions, remove it --- */
    if (match_found)
    {
        msc->inst->rete_token = tok;
        msc->inst->rete_wme = w;
        remove_from_dll(node->b.p.tentative_retractions, msc, next_of_node, prev_of_node);
        remove_from_dll(thisAgent->ms_retractions, msc, next, prev);
        if (msc->goal)
        {
            remove_from_dll(msc->goal->id->ms_retractions, msc,
                            next_in_level, prev_in_level);
        }
        else
        {
            // RPM 6/05
            // This if statement is to avoid a crash we get on most platforms in Soar 7 mode
            // It's unknown what consequences it has, but the Soar 7 demos seem to work
            // To return things to how they were, simply remove the if statement (but leave
            //  the remove_from_dll line).

            // voigtjr 2009: returning things to how they were now that soar7 is removed
            //if(thisAgent->nil_goal_retractions)
            {
                remove_from_dll(thisAgent->nil_goal_retractions,  msc, next_in_level, prev_in_level);
            }
        }

        thisAgent->memoryManager->free_with_pool(MP_ms_change, msc);
        dprint(DT_RETE_PNODE_ADD, "Removing tentative retraction: %y\n", node->b.p.prod->name);
        activation_exit_sanity_check();
        return;
    }

    /* --- no match found, so add new assertion --- */
    dprint(DT_RETE_PNODE_ADD, "Adding tentative assertion: %y\n", node->b.p.prod->name);

    thisAgent->memoryManager->allocate_with_pool(MP_ms_change, &msc);
    msc->tok = tok;
    msc->w = w;
    msc->p_node = node;
    msc->inst = NIL;  /* just for safety */
    /* initialize goal regardless of run mode */
    msc->level = NO_WME_LEVEL;
    msc->goal = NIL;
    
    //assert(tok);
    /*  (this is a RCHONG comment, but might also apply to Operand2...?)

    what we have to do now is to, essentially, determine the kind of
    support this production would get based on its present complete
    matches.  once i know the support, i can then know into which match
    set list to put "msc".

    this code is used to make separate PE productions from IE
    productions by putting them into different match set lists.  in
    non-OPERAND, these matches would all go into one list.

    BUGBUG i haven't tested this with a production that has more than
    one match where the matches could have different support.  is that
    even possible???

    */

    /* operand code removed 1/22/99 - kjc */

    /* Find the goal and level for this ms change */
    msc->goal = find_goal_for_match_set_change_assertion(thisAgent, msc);
    msc->level = msc->goal->id->level;

    dprint(DT_WATERFALL, "    Level of goal is  %d\n", static_cast<int64_t>(msc->level));

    prod_type = IE_PRODS;

    if (node->b.p.prod->declared_support == DECLARED_O_SUPPORT)
    {
        prod_type = PE_PRODS;
    }

    else if (node->b.p.prod->declared_support == DECLARED_I_SUPPORT)
    {
        prod_type = IE_PRODS;
    }

    else if (node->b.p.prod->declared_support == UNDECLARED_SUPPORT)
    {
        /* check if the instantiation is proposing an operator.  if it
           is, then this instantiation is i-supported. */

        operator_proposal = false;

        for (act = node->b.p.prod->action_list; act != NIL ; act = act->next)
        {
            if ((act->type == MAKE_ACTION) && (rhs_value_is_symbol(act->attr)))
            {
                if ((rhs_value_to_rhs_symbol(act->attr)->referent == thisAgent->symbolManager->soarSymbols.operator_symbol) &&
                        (act->preference_type == ACCEPTABLE_PREFERENCE_TYPE))
                {
                    Symbol* lSym = NULL;
                    if (tok && w)
                    {
                        lSym = get_symbol_from_rete_loc(rhs_value_to_reteloc_levels_up(act->id),
                                                       rhs_value_to_reteloc_field_num(act->id), tok, w);
                    }
                    if (lSym && lSym->id->isa_goal)
                    {
                        operator_proposal = true;
                        prod_type = !PE_PRODS;
                        break;
                    }
                }
            }
        }

        if (operator_proposal == false)
        {

            /*
            examine all the different matches for this productions
            */

            for (OPERAND_curr_tok = node->a.np.tokens;
                    OPERAND_curr_tok != NIL;
                    OPERAND_curr_tok = OPERAND_curr_tok->next_of_node)
            {

                /* i'll need to make two passes over each set of wmes that
                match this production.  the first pass looks for the lowest
                goal identifier.  the second pass looks for a wme of the form:

                (<lowest-goal-id> ^operator ...)

                if such a wme is found, then this production is a PE_PROD.
                otherwise, it's a IE_PROD.

                admittedly, this implementation is kinda sloppy.  i need to
                clean it up some.

                BUGBUG this check only looks at positive conditions.  we
                haven't really decided what testing the absence of the
                operator will do.  this code assumes that such a productions
                (instantiation) would get i-support.

                Modified 1/00 by KJC for o-support-mode == 3:  prods that have ONLY operator
                elaborations (<o> ^attr ^value) are IE_PROD.  If prod has
                both operator applications and <o> elabs, then it's PE_PROD
                and the user is warned that <o> elabs will be o-supported. */
                
                op_elab = false;
                lowest_goal_wme = NIL;

                for (pass = 0; pass != 2; pass++)
                {

                    temp_tok = OPERAND_curr_tok;
                    while (temp_tok != NIL)
                    {
                        while (temp_tok->w == NIL)
                        {
                            temp_tok = temp_tok->parent;
                            if (temp_tok == NIL) break;
                        }
                        if (temp_tok == NIL) break;
                        if (temp_tok->w == NIL) break;

                        if (pass == 0)
                        {
                            if (temp_tok->w->id->id->isa_goal == true)
                            {
                                if (lowest_goal_wme == NIL)
                                {
                                    lowest_goal_wme = temp_tok->w;
                                }
                                else
                                {
                                    if (temp_tok->w->id->id->level >
                                            lowest_goal_wme->id->id->level)
                                    {
                                        lowest_goal_wme = temp_tok->w;
                                    }
                                }
                            }
                        }
                        else
                        {
                            if ((temp_tok->w->attr == thisAgent->symbolManager->soarSymbols.operator_symbol) &&
                                (temp_tok->w->acceptable == false) && (temp_tok->w->id == lowest_goal_wme->id))
                            {
                                /* iff RHS has only operator elaborations
                                    then it's IE_PROD, otherwise PE_PROD, so
                                    look for non-op-elabs in the actions  KJC 1/00 */


                                /* We also need to check reteloc's to see if they
                                    are referring to operator augmentations before determining
                                    if this is an operator elaboration
                                 */

                                for (act = node->b.p.prod->action_list; act != NIL ; act = act->next)
                                {
                                    if (act->type == MAKE_ACTION)
                                    {
                                        if ((rhs_value_is_symbol(act->id)) && (rhs_value_to_symbol(act->id) == temp_tok->w->value))
                                        {
                                            op_elab = true;
                                        }
                                        else if (rhs_value_is_reteloc(act->id) &&
                                            (temp_tok->w->value == get_symbol_from_rete_loc(rhs_value_to_reteloc_levels_up(act->id),
                                                                                            rhs_value_to_reteloc_field_num(act->id), tok, w)))
                                        {
                                            op_elab = true;
                                        }
                                        else
                                        {
                                            /* this is not an operator elaboration */
                                            prod_type = PE_PRODS;
                                        }
                                    } // act->type == MAKE_ACTION
                                } // for
                            }
                        } /* end if (pass == 0) ... */
                        temp_tok = temp_tok->parent;
                    }  /* end while (temp_tok != NIL) ... */

                    if (prod_type == PE_PRODS)
                    {
                        if (op_elab == true)
                        {

                            /* warn user about mixed actions */

                            if (thisAgent->outputManager->settings[OM_WARNINGS])
                            {
                                thisAgent->outputManager->printa_sf(thisAgent, "WARNING:  Operator elaborations mixed with operator applications\nAssigning i_support to prod %y",  node->b.p.prod->name);

                                // XML generation
                                growable_string gs = make_blank_growable_string(thisAgent);
                                add_to_growable_string(thisAgent, &gs, "WARNING:  Operator elaborations mixed with operator applications\nAssigning i_support to prod ");
                                add_to_growable_string(thisAgent, &gs, node->b.p.prod->name->to_string(true));
                                xml_generate_warning(thisAgent, text_of_growable_string(gs));
                                free_growable_string(thisAgent, gs);

                                prod_type = IE_PRODS;
                                break;
                            }
                        }
                    }
                }  /* end for pass =  */
            }        /* end for loop checking all matches */

            /* BUG:  IF you print lowest_goal_wme here, you don't get what
            you'd expect.  Instead of the lowest goal WME, it looks like
            you get the lowest goal WME in the first/highest assertion of
            all the matches for this production.  So, if there is a single
            match, you get the right number.  If there are multiple matches
            for the same production, you get the lowest goal of the
            highest match goal production (or maybe just the first to
            fire?).  I don;t know for certain if this is the behavior
            Ron C. wanted or if it's a bug --
            i need to talk to him about it. */

        }  /* end if (operator_proposal == false) */

    }        /* end UNDECLARED_SUPPORT */

    if (prod_type == PE_PRODS)
    {
        insert_at_head_of_dll(thisAgent->ms_o_assertions, msc, next, prev);
        insert_at_head_of_dll(msc->goal->id->ms_o_assertions, msc, next_in_level, prev_in_level);
        node->b.p.prod->OPERAND_which_assert_list = O_LIST;

        if (thisAgent->trace_settings[TRACE_ASSERTIONS_SYSPARAM])
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%e   RETE: putting [%y] into ms_o_assertions",  node->b.p.prod->name);
            char buf[256];
            SNPRINTF(buf, 254, "RETE: putting [%s] into ms_o_assertions", node->b.p.prod->name->to_string(true));
            xml_generate_verbose(thisAgent, buf);
        }
    }

    else
    {
        insert_at_head_of_dll(thisAgent->ms_i_assertions, msc, next, prev);
        insert_at_head_of_dll(msc->goal->id->ms_i_assertions, msc, next_in_level, prev_in_level);
        node->b.p.prod->OPERAND_which_assert_list = I_LIST;

        if (thisAgent->trace_settings[TRACE_ASSERTIONS_SYSPARAM])
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%e   RETE: putting [%y] into ms_i_assertions",  node->b.p.prod->name);
            char buf[256];
            SNPRINTF(buf, 254, "RETE: putting [%s] into ms_i_assertions", node->b.p.prod->name->to_string(true));
            xml_generate_verbose(thisAgent, buf);
        }
    }

    // :interrupt
    if (node->b.p.prod->interrupt)
    {
        node->b.p.prod->interrupt++;
        thisAgent->stop_soar = true;

        // Note that this production name might not be completely accurate.
        // If two productions match, the last matched production name will be
        // saved, but if this production then gets retracted on the same
        // elaboration cycle, while the first matching production remains
        // on the assertion list, Soar will still halt, but the production
        // named will be inaccurate.
        thisAgent->outputManager->printa_sf(thisAgent, "\n*** Production match-time interrupt (:interrupt), probably from %y\n", node->b.p.prod->name);
        thisAgent->outputManager->printa_sf(thisAgent, "    [Phase] (Interrupt, Stop) is [%d] (%d,%d)\n", thisAgent->current_phase, node->b.p.prod->interrupt, thisAgent->stop_soar);

        thisAgent->reason_for_stopping = ":interrupt";
    }

    /* RCHONG: end 10.11 */

    insert_at_head_of_dll(node->b.p.tentative_assertions, msc, next_of_node, prev_of_node);
    activation_exit_sanity_check();
    //dprint(DT_RETE_PNODE_ADD, "p_node_left_addition finished for node %d, token %u, and wme %w\n", node->node_id, tok, w);

}

/* ----------------------------------------------------------------------
                         P Node Left Removal

   Algorithm:

   Does this token match (eq) one of the tentative_assertions?
   If so, just remove that tentative_assertion.
   If not, find the instantiation corresponding to this token
     and add it to tentative_retractions.
---------------------------------------------------------------------- */

/* BUGBUG shouldn't need to pass in both tok and w -- should have the
   p-node's token get passed in instead, and have it point to the
   corresponding instantiation structure. */

void p_node_left_removal(agent* thisAgent, rete_node* node, token* tok, wme* w)
{
    ms_change* msc;
    instantiation* inst;

    activation_entry_sanity_check();

    /* --- check for match in tentative_assertions --- */
    for (msc = node->b.p.tentative_assertions; msc != NIL; msc = msc->next_of_node)
    {
        if ((msc->tok == tok) && (msc->w == w))
        {
            dprint(DT_RETE_PNODE_ADD, "Removing tentative assertion: %y", node->b.p.prod->name);

            /* --- match found in tentative_assertions, so remove it --- */
            remove_from_dll(node->b.p.tentative_assertions, msc, next_of_node, prev_of_node);

            // :interrupt
            if (node->b.p.prod->interrupt > 1)
            {
                node->b.p.prod->interrupt--;
                thisAgent->stop_soar = false;
                if (thisAgent->trace_settings[TRACE_ASSERTIONS_SYSPARAM])
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "RETRACTION (1) reset interrupt to READY -- (Interrupt, Stop) to (%d, %d)\n", node->b.p.prod->interrupt, thisAgent->stop_soar);
                }
            }

            if (node->b.p.prod->OPERAND_which_assert_list == O_LIST)
            {
                dprint(DT_RETE_PNODE_ADD, "...also removing from ms_o_assertions\n");
                remove_from_dll(thisAgent->ms_o_assertions, msc, next, prev);
                /* msc already defined for the assertion so the goal should be defined
                as well. */
                remove_from_dll(msc->goal->id->ms_o_assertions, msc, next_in_level, prev_in_level);
            }
            else if (node->b.p.prod->OPERAND_which_assert_list == I_LIST)
            {
                dprint(DT_RETE_PNODE_ADD, "...also removing from ms_i_assertions\n");
                remove_from_dll(thisAgent->ms_i_assertions, msc, next, prev);
                remove_from_dll(msc->goal->id->ms_i_assertions, msc, next_in_level, prev_in_level);
            }

            thisAgent->memoryManager->free_with_pool(MP_ms_change, msc);
            activation_exit_sanity_check();
            return;
        }
    } /* end of for loop */

    /* --- find the instantiation corresponding to this token --- */
    for (inst = node->b.p.prod->instantiations; inst != NIL; inst = inst->next)
        if ((inst->rete_token == tok) && (inst->rete_wme == w))
        {
            break;
        }

    if (inst)
    {
        /* --- add that instantiation to tentative_retractions --- */
        dprint(DT_RETE_PNODE_ADD, "Adding tentative retraction: %y", node->b.p.prod->name);
        inst->rete_token = NIL;
        inst->rete_wme = NIL;
        thisAgent->memoryManager->allocate_with_pool(MP_ms_change, &msc);
        msc->inst = inst;
        msc->p_node = node;
        msc->tok = NIL;     /* just for safety */
        msc->w = NIL;       /* just for safety */
        msc->level = NO_WME_LEVEL;      /* just for safety */
        msc->goal = NIL;    /* just for safety */
        insert_at_head_of_dll(node->b.p.tentative_retractions, msc,  next_of_node, prev_of_node);

        /* Determine what the goal of the msc is and add it to that
        goal's list of retractions */
        msc->goal = find_goal_for_match_set_change_retraction(msc);
        msc->level = msc->goal->id->level;

        dprint(DT_WATERFALL, "    Level of retraction is: %d\n", msc->level);

        if (msc->goal->id->link_count == 0)
        {
            /* BUG (potential) (Operand2/Waterfall: 2.101)
            When a goal is removed in the stack, it is not immediately garbage
            collected, meaning that the goal pointer is still valid when the
            retraction is created.  So the goal for a retraction will always be
            valid, even though, for retractions caused by goal removals, the
            goal will be removed at the next WM phase. (You can see this by
            printing the identifier for the goal in the elaboration cycle
            after goal removal.  It's still there, although nothing is attached
            to it.  One elab later, the identifier itself is removed.)  Because
            Waterfall needs to know if the goal is valid or not, I look at the
            link_count on the symbol.  A link_count of 0 is the trigger for the
            garbage collection so this solution should work -- I just make the
            pointer NIL to ensure that the retractions get added to the
            NIL_goal_retraction list.  However, if the link_count is never
            *not* zero for an already removed goal, this solution will fail,
            resulting in both the retraction never being able to fire and a
            memory leak (because the items on the ms_change list on the symbol
            will never be freed). */
            /* print("\nThis goal is being removed.  Changing msc goal pointer to NIL.");  */
            msc->goal = NIL;
        }

        /* Put on the original retraction list */
        insert_at_head_of_dll(thisAgent->ms_retractions, msc, next, prev);
        if (msc->goal)   /* Goal exists */
        {
            insert_at_head_of_dll(msc->goal->id->ms_retractions, msc, next_in_level, prev_in_level);
        }
        else   /* NIL Goal; put on the NIL Goal list */
        {
            insert_at_head_of_dll(thisAgent->nil_goal_retractions,  msc, next_in_level, prev_in_level);
        }

        dprint(DT_WATERFALL, "Retraction: %y is active at level %d.  Enable DEBUG_WATERFALL for retraction lists.\n", msc->inst->prod_name, msc->level);

        #ifdef DEBUG_WATERFALL

        {
            ms_change* assertion;
            thisAgent->outputManager->printa_sf(thisAgent, "%e Retractions list:\n");
            for (assertion = thisAgent->ms_retractions;  assertion; assertion = assertion->next)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "     Retraction: %y ", assertion->p_node->b.p.prod->name);
                thisAgent->outputManager->printa_sf(thisAgent, " at level %d\n", assertion->level);
            }

            if (thisAgent->nil_goal_retractions)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "%eCurrent NIL Goal list:\n");
                assertion = NIL;
                for (assertion = thisAgent->nil_goal_retractions; assertion; assertion = assertion->next_in_level)
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "     Retraction: %y ", assertion->p_node->b.p.prod->name);
                    thisAgent->outputManager->printa_sf(thisAgent, " at level %d\n", assertion->level);
                    if (assertion->goal)
                    {
                        thisAgent->outputManager->printa_sf(thisAgent, "This assertion has non-NIL goal pointer.\n");
                    }
                }
            }
        }
        #endif

        activation_exit_sanity_check();
        return;
    }

    if (thisAgent->trace_settings[TRACE_ASSERTIONS_SYSPARAM])
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%e%y: ", node->b.p.prod->name);
        char buf[256];
        SNPRINTF(buf, 254, "%s: ", node->b.p.prod->name->to_string(true));
        xml_generate_verbose(thisAgent, buf);
    }

    #ifdef BUG_139_WORKAROUND
    if (node->b.p.prod->type == JUSTIFICATION_PRODUCTION_TYPE)
    {
        #ifdef BUG_139_WORKAROUND_WARNING
        thisAgent->outputManager->printa_sf(thisAgent, "%eWarning: can't find instantiation of justification %y to retract (BUG 139 WORKAROUND)\n",
            node->b.p.prod ? node->b.p.prod->name : NULL);
        xml_generate_warning(thisAgent, "Warning: can't find an existing justification to retract (BUG 139 WORKAROUND)");
        #endif
        return;
    }
    #endif

    thisAgent->outputManager->printa_sf(thisAgent, "%eWarning: Soar can't find an existing instantiation of %y to retract.  Soar memory may be corrupt.\n",
        node->b.p.prod ? node->b.p.prod->name : NULL);
    xml_generate_warning(thisAgent, "Warning: Soar can't find an existing instantiation to retract.  Soar memory may be corrupt.");

    /* This can occur when an agent reaches max-chunks (even before EBC). Bug 139 workaround just continues execution when it can't
     * find an instantiation it expects.  Perhaps that's ok because justifications are different.  I'm not sure.
     * I'm going to try doing the same thing here and just print a warning, instead of aborting.  It could be useful to allow
     * execution, for debugging if nothing else.  */
//    {
//        char msg[BUFFER_MSG_SIZE];
//        strncpy(msg,
//                "Internal error: can't find existing instantiation to retract\n", BUFFER_MSG_SIZE);
//        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
//        abort_with_fatal_error(thisAgent, msg);
//    }
}

/* ************************************************************************

   SECTION 16:  Beta Node Interpreter Routines: Tree-Based Removal

   This routine does tree-based removal of a token and its descendents.
   Note that it uses a nonrecursive tree traversal; each iteration, the
   leaf being deleted is the leftmost leaf in the tree.
************************************************************************ */

void remove_token_and_subtree(agent* thisAgent, token* root)
{
    rete_node* node, *child, *next;
    token* tok, *next_value_for_tok, *left, *t, *next_t;
    byte node_type;

    tok = root;

    while (true)
    {
        /* --- move down to the leftmost leaf --- */
        while (tok->first_child)
        {
            tok = tok->first_child;
        }
        next_value_for_tok = tok->next_sibling ? tok->next_sibling : tok->parent;

        /* --- cleanup stuff common to all types of nodes --- */
        node = tok->node;
        left_node_activation(node, false);
        fast_remove_from_dll(node->a.np.tokens, tok, token, next_of_node,
                             prev_of_node);
        fast_remove_from_dll(tok->parent->first_child, tok, token,
                             next_sibling, prev_sibling);
        if (tok->w) fast_remove_from_dll(tok->w->tokens, tok, token,
                                             next_from_wme, prev_from_wme);
        node_type = node->node_type;

        /* --- for merged Mem/Pos nodes --- */
        if ((node_type == MP_BNODE) || (node_type == UNHASHED_MP_BNODE))
        {
            remove_token_from_left_ht(thisAgent, tok, node->node_id ^
                                      (tok->a.ht.referent ?
                                       tok->a.ht.referent->hash_id : 0));
            if (! mp_bnode_is_left_unlinked(node))
            {
                if (! node->a.np.tokens)
                {
                    unlink_from_right_mem(node);
                }
            }

            /* --- for P nodes --- */
        }
        else if (node_type == P_BNODE)
        {
            p_node_left_removal(thisAgent, node, tok->parent, tok->w);

            /* --- for Negative nodes --- */
        }
        else if ((node_type == NEGATIVE_BNODE) ||
                 (node_type == UNHASHED_NEGATIVE_BNODE))
        {
            remove_token_from_left_ht(thisAgent, tok, node->node_id ^
                                      (tok->a.ht.referent ?
                                       tok->a.ht.referent->hash_id : 0));
            if (! node->a.np.tokens)
            {
                unlink_from_right_mem(node);
            }
            for (t = tok->negrm_tokens; t != NIL; t = next_t)
            {
                next_t = t->a.neg.next_negrm;
                fast_remove_from_dll(t->w->tokens, t, token, next_from_wme, prev_from_wme);
                thisAgent->memoryManager->free_with_pool(MP_token, t);
            }

            /* --- for Memory nodes --- */
        }
        else if ((node_type == MEMORY_BNODE) || (node_type == UNHASHED_MEMORY_BNODE))
        {
            remove_token_from_left_ht(thisAgent, tok, node->node_id ^
                                      (tok->a.ht.referent ?
                                       tok->a.ht.referent->hash_id : 0));
#ifdef DO_ACTIVATION_STATS_ON_REMOVALS
            /* --- if doing statistics stuff, then activate each attached node --- */
            for (child = node->b.mem.first_linked_child; child != NIL; child = next)
            {
                next = child->a.pos.next_from_beta_mem;
                left_node_activation(child, false);
            }
#endif
            /* --- for right unlinking, then if the beta memory just went to
               zero, right unlink any attached Pos nodes --- */
            if (! node->a.np.tokens)
            {
                for (child = node->b.mem.first_linked_child; child != NIL; child = next)
                {
                    next = child->a.pos.next_from_beta_mem;
                    unlink_from_right_mem(child);
                }
            }

            /* --- for CN nodes --- */
        }
        else if (node_type == CN_BNODE)
        {
            remove_token_from_left_ht(thisAgent, tok, node->node_id ^
                                      static_cast<uint32_t>(reinterpret_cast<uint64_t>(tok->parent)) ^
                                      static_cast<uint32_t>(reinterpret_cast<uint64_t>(tok->w))); // double cast necessary for avoiding precision loss warning
            for (t = tok->negrm_tokens; t != NIL; t = next_t)
            {
                next_t = t->a.neg.next_negrm;
                if (t->w) fast_remove_from_dll(t->w->tokens, t, token,
                                                   next_from_wme, prev_from_wme);
                fast_remove_from_dll(t->node->a.np.tokens, t, token,
                                     next_of_node, prev_of_node);
                fast_remove_from_dll(t->parent->first_child, t, token,
                                     next_sibling, prev_sibling);
                thisAgent->memoryManager->free_with_pool(MP_token, t);
            }

            /* --- for CN Partner nodes --- */
        }
        else if (node_type == CN_PARTNER_BNODE)
        {
            left = tok->a.neg.left_token;
            fast_remove_from_dll(left->negrm_tokens, tok, token,
                                 a.neg.next_negrm, a.neg.prev_negrm);
            if (! left->negrm_tokens)   /* just went to 0, so call children */
            {
                for (child = left->node->first_child; child != NIL;
                        child = child->next_sibling)
                {
                    (*(left_addition_routines[child->node_type]))(thisAgent, child, left, NIL);
                }
            }

        }
        else
        {
            char msg[BUFFER_MSG_SIZE];
            SNPRINTF(msg, BUFFER_MSG_SIZE,
                     "Internal error: bad node type %d in remove_token_and_subtree\n",
                     node->node_type);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }

        thisAgent->memoryManager->free_with_pool(MP_token, tok);
        if (tok == root)
        {
            break;    /* if leftmost leaf was the root, we're done */
        }
        tok = next_value_for_tok; /* else go get the leftmost leaf again */
    }
}




/* **********************************************************************

   SECTION 17:  Fast, Compact Save/Reload of the Whole Rete Net

   These routines handle the fastsave/load of the Rete net.  The basic
   format of the file is as follows.  We first write out an initial
   "magic number" string; this is just used during reload to make sure
   the file we're trying to load actually *is* a fastsave file.  Next
   comes the version number.  IF YOU CHANGE THE FILE FORMAT, CHANGE THE
   VERSION NUMBER.  PROVIDING BACKWARD COMPATIBILITY OR A CONVERSION
   UTILITY IS STRONGLY RECOMMENDED.

   After that, we just dump out all the symbols (except for identifiers)
   in the system.  Next, we write out all the alpha memories (just the
   id/attr/value form they take, not the WMEs they contain).  Then,
   the actual Rete net.  This is written out as a preorder traversal of
   the Rete tree -- the record for each node consists of some data for
   that particular node, followed by the records for each of its children.
   NCC's are handled by ignoring the CN node during the preorder traversal,
   but writing out the data for the CN_PARTNER node and pretending the CN
   node's children actually belong to the CN_PARTNER.  (This is done so that
   when we reload the net, the whole NCC subnetwork and CN/CNP stuff gets
   reloaded and reconstructed *before* any nodes underneath the CN node.

   File format (version 3):
     [Note: all 16-bit or 32-bit words are written LSB first]

     magic number sequence: "SoarCompactReteNet\n"
     1 byte: 0 (null termination for the above string)
     1 byte: format version number (current version is version 3)

     4 bytes: number of str_constants
     4 bytes: number of variables
     4 bytes: number of int_constants
     4 bytes: number of float_constants
       names of all str_constants (each a null-terminated string)
       names of all variables (each a null-terminated string)
       values of all int_constants (each as a null-terminated ASCII string)
       values of all float_constants (each as a null-terminated ASCII string)

     4 bytes: number of alpha memories
       definitions of all alpha memories, each of the form:
         12 bytes: indices of the symbols in the id, attr, and value fields
                   (0 if the field has no symbol in it)
         1 byte: 0-->normal, 1-->acceptable preference test

     4 bytes: number of children of the root node
     node records for each child of the root node

  Node record:
    1 byte: node type
    data for node:
      posneg nodes: if hashed (and not P): 3 bytes -- hash field num, levels up
                    4 bytes -- index of alpha memory
                    record for rete test list (for other tests)
                    pos and mp nodes: 1 byte -- 1 if left unlinked, 0 else
      mem nodes: if hashed: 3 bytes -- hash field num, levels up
      cn nodes: no record at all (not even node type) -- handled with cn_p
                node record instead.  Basically, we ignore the cn node when
                writing the net, and pretend the cn/cn_p pair is one big
                node underneath the subnetwork.
      cn_p nodes: number of conjuncts in the NCC
      p_nodes: 4 bytes: name of production (symindex)
               1 byte (0 or 1): flag -- is there a documentation string
                 if yes: documentation string (null-terminated string)
               1 byte: type
               1 byte: declared support
               record for the list of RHS actions
               4 bytes: number of RHS unbound variables
                 RHS unbound variables (symindices for each one)
               1 byte (0 or 1): flag -- is there node_varnames info?
                 if yes:  node_varnames records for this production
    4 bytes: number of children
    node records for each child

  EXTERNAL INTERFACE:
  Save_rete_net() and load_rete_net() save and load everything to and
  from the given (already open) files.  They return true if successful,
  false if any error occurred.
********************************************************************** */

FILE* rete_fs_file;  /* File handle we're using -- "fs" for "fast-save" */
bool rete_net_64; // used by reteload_eight_bytes, retesave_eight_bytes, BADBAD global, fix with rete_fs_file above

/* ----------------------------------------------------------------------
                Save/Load Bytes, Short and Long Integers

   These are the lowest-level routines for accessing the FS file.  Note
   that all 16-bit or 32-bit words are written LSB first.  We do this
   carefully, so that fastsave files will be portable across machine
   types (big-endian vs. little-endian).
---------------------------------------------------------------------- */

void retesave_one_byte(uint8_t b, FILE* /*f*/)
{
    fputc(b, rete_fs_file);
}

uint8_t reteload_one_byte(FILE* f)
{
    return static_cast<uint8_t>(fgetc(f));
}

void retesave_two_bytes(uint16_t w, FILE* f)
{
    retesave_one_byte(static_cast<uint8_t>(w & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 8) & 0xFF), f);
}

uint16_t reteload_two_bytes(FILE* f)
{
    uint16_t i;
    i = reteload_one_byte(f);
    i += (reteload_one_byte(f) << 8);
    return i;
}

void retesave_four_bytes(uint32_t w, FILE* f)
{
    retesave_one_byte(static_cast<uint8_t>(w & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 8) & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 16) & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 24) & 0xFF), f);
}

uint32_t reteload_four_bytes(FILE* f)
{
    uint32_t i;
    i = reteload_one_byte(f);
    i += (reteload_one_byte(f) << 8);
    i += (reteload_one_byte(f) << 16);
    i += (reteload_one_byte(f) << 24);
    return i;
}

void retesave_eight_bytes(uint64_t w, FILE* f)
{
    if (!rete_net_64)
    {
        retesave_four_bytes(static_cast<uint32_t>(w), f);
        return;
    }
    retesave_one_byte(static_cast<uint8_t>(w & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 8) & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 16) & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 24) & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 32) & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 40) & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 48) & 0xFF), f);
    retesave_one_byte(static_cast<uint8_t>((w >> 56) & 0xFF), f);
}

uint64_t reteload_eight_bytes(FILE* f)
{
    if (!rete_net_64)
    {
        return reteload_four_bytes(f);
    }

    uint64_t i;
    uint64_t tmp;
    i = reteload_one_byte(f);
    tmp = reteload_one_byte(f);
    i += (tmp << 8);
    tmp = reteload_one_byte(f);
    i += (tmp << 16);
    tmp = reteload_one_byte(f);
    i += (tmp << 24);
    tmp = reteload_one_byte(f);
    i += (tmp << 32);
    tmp = reteload_one_byte(f);
    i += (tmp << 40);
    tmp = reteload_one_byte(f);
    i += (tmp << 48);
    tmp = reteload_one_byte(f);
    i += (tmp << 56);
    return i;
}

/* ----------------------------------------------------------------------
                            Save/Load Strings

   Strings are written as null-terminated sequences of characters, just
   like the usual C format.  Reteload_string() leaves the result in
   reteload_string_buf[].
---------------------------------------------------------------------- */

char reteload_string_buf[4 * MAX_LEXEME_LENGTH];

void retesave_string(const char* s, FILE* f)
{
    while (*s)
    {
        retesave_one_byte(*s, f);
        s++;
    }
    retesave_one_byte(0, f);
}

void reteload_string(FILE* f)
{
    int i, ch;
    i = 0;
    do
    {
        ch = reteload_one_byte(f);
        reteload_string_buf[i++] = static_cast<char>(ch);
    }
    while (ch);
}

/* ----------------------------------------------------------------------
                            Save/Load Symbols

   We write out symbol names once at the beginning of the file, and
   thereafter refer to symbols using 32-bit index numbers instead of their
   full names.  Retesave_symbol_and_assign_index() writes out one symbol
   and assigns it an index (stored in sym->retesave_symindex).
   Index numbers are assigned sequentially -- the first symbol in the file
   has index number 1, the second has number 2, etc.  Retesave_symbol_table()
   saves the whole symbol table, using the following format:

       4 bytes: number of str_constants
       4 bytes: number of variables
       4 bytes: number of int_constants
       4 bytes: number of float_constants
         names of all str_constants (each a null-terminated string)
         names of all variables (each a null-terminated string)
         values of all int_constants (each as a null-term. ASCII string)
         values of all float_constants (each as a null-term. ASCII string)

   To reload symbols, we read the records and make new symbols, and
   also create an array (reteload_symbol_table) that maps from the
   index numbers to the Symbol structures.  Reteload_all_symbols() does
   this.  Reteload_symbol_from_index() reads an index number and returns
   the appropriate Symbol (without incrementing its reference count).
   Reteload_free_symbol_table() frees up the symbol table when we're done.
---------------------------------------------------------------------- */

void retesave_symbol_table(agent* thisAgent, FILE* f)
{
    thisAgent->current_retesave_symindex = 0;
    thisAgent->symbolManager->retesave(f);
}

void reteload_all_symbols(agent* thisAgent, FILE* f)
{
    uint64_t num_str_constants, num_variables;
    uint64_t num_int_constants, num_float_constants;
    Symbol** current_place_in_symtab;
    uint64_t i;

    num_str_constants = reteload_eight_bytes(f);
    num_variables = reteload_eight_bytes(f);
    num_int_constants = reteload_eight_bytes(f);
    num_float_constants = reteload_eight_bytes(f);

    thisAgent->reteload_num_syms = num_str_constants + num_variables + num_int_constants
                                   + num_float_constants;

    /* --- allocate memory for the symbol table --- */
    thisAgent->reteload_symbol_table = (Symbol**)
                                       thisAgent->memoryManager->allocate_memory(thisAgent->reteload_num_syms * sizeof(char*), MISCELLANEOUS_MEM_USAGE);

    /* --- read in all the symbols from the file --- */
    current_place_in_symtab = thisAgent->reteload_symbol_table;
    for (i = 0; i < num_str_constants; i++)
    {
        reteload_string(f);
        *(current_place_in_symtab++) = thisAgent->symbolManager->make_str_constant(reteload_string_buf);
    }
    for (i = 0; i < num_variables; i++)
    {
        reteload_string(f);
        *(current_place_in_symtab++) = thisAgent->symbolManager->make_variable(reteload_string_buf);
    }
    for (i = 0; i < num_int_constants; i++)
    {
        reteload_string(f);
        *(current_place_in_symtab++) =
            thisAgent->symbolManager->make_int_constant(strtol(reteload_string_buf, NULL, 10));
    }
    for (i = 0; i < num_float_constants; i++)
    {
        reteload_string(f);
        *(current_place_in_symtab++) =
            thisAgent->symbolManager->make_float_constant(strtod(reteload_string_buf, NULL));
    }
}

Symbol* reteload_symbol_from_index(agent* thisAgent, FILE* f)
{
    uint64_t index;

    index = reteload_eight_bytes(f);
    if (index == 0)
    {
        return NIL;
    }
    index--;
    if (index >= thisAgent->reteload_num_syms)
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "Internal error (file corrupted?): symbol count too small\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }
    return *(thisAgent->reteload_symbol_table + index);
}

void reteload_free_symbol_table(agent* thisAgent)
{
    uint64_t i;

    for (i = 0; i < thisAgent->reteload_num_syms; i++)
    {
        thisAgent->symbolManager->symbol_remove_ref(&(*(thisAgent->reteload_symbol_table + i)));
    }
    thisAgent->memoryManager->free_memory(thisAgent->reteload_symbol_table, MISCELLANEOUS_MEM_USAGE);
}

/* ----------------------------------------------------------------------
                        Save/Load Alpha Memories

   We write out alpha memories once, near the beginning of the file, and
   thereafter refer to them using 32-bit index numbers (just like symbols).
   Retesave_alpha_mem_and_assign_index() writes out one alpha memory
   and assigns it an index (stored in am->retesave_amindex).  Index numbers
   are assigned sequentially -- the first alpha memory in the file has
   index number 1, the second has number 2, etc.   Retesave_alpha_memories()
   writes out all the alpha memories, in the following format:

       4 bytes: number of alpha memories
         definitions of all alpha memories, each of the form:
           12 bytes: indices of the symbols in the id, attr, and value fields
                     (0 if the field has no symbol in it)
           1 byte: 0-->normal, 1-->acceptable preference test

   To reload alpha memories, we read the records and make new AM's, and
   also create an array (reteload_am_table) that maps from the
   index numbers to the alpha_mem structures.  Reteload_alpha_memories()
   does this.  Reteload_am_from_index() reads an index number and returns
   the appropriate alpha_mem (without incrementing its reference count).
   Reteload_free_am_table() frees up the table when we're done.
---------------------------------------------------------------------- */

bool retesave_alpha_mem_and_assign_index(agent* thisAgent, void* item, void* userdata)
{
    alpha_mem* am;
    FILE* f = reinterpret_cast<FILE*>(userdata);

    am = static_cast<alpha_mem_struct*>(item);
    thisAgent->current_retesave_amindex++;
    am->retesave_amindex = thisAgent->current_retesave_amindex;
    retesave_eight_bytes(am->id ? am->id->retesave_symindex : 0, f);
    retesave_eight_bytes(am->attr ? am->attr->retesave_symindex : 0, f);
    retesave_eight_bytes(am->value ? am->value->retesave_symindex : 0, f);
    retesave_one_byte(static_cast<byte>(am->acceptable ? 1 : 0), f);
    return false;
}

void retesave_alpha_memories(agent* thisAgent, FILE* f)
{
    uint64_t i, num_ams;

    thisAgent->current_retesave_amindex = 0;
    num_ams = 0;
    for (i = 0; i < 16; i++)
    {
        num_ams += thisAgent->alpha_hash_tables[i]->count;
    }
    retesave_eight_bytes(num_ams, f);
    for (i = 0; i < 16; i++)
        do_for_all_items_in_hash_table(thisAgent, thisAgent->alpha_hash_tables[i],
                                       retesave_alpha_mem_and_assign_index, f);
}

void reteload_alpha_memories(agent* thisAgent, FILE* f)
{
    uint64_t i;
    Symbol* id, *attr, *value;
    bool acceptable;

    thisAgent->reteload_num_ams = reteload_eight_bytes(f);
    thisAgent->reteload_am_table = (alpha_mem**)
                                   thisAgent->memoryManager->allocate_memory(thisAgent->reteload_num_ams * sizeof(char*), MISCELLANEOUS_MEM_USAGE);
    for (i = 0; i < thisAgent->reteload_num_ams; i++)
    {
        id = reteload_symbol_from_index(thisAgent, f);
        attr = reteload_symbol_from_index(thisAgent, f);
        value = reteload_symbol_from_index(thisAgent, f);
        acceptable = reteload_one_byte(f) ? true : false;
        *(thisAgent->reteload_am_table + i) = find_or_make_alpha_mem(thisAgent, id, attr, value, acceptable);
    }
}

alpha_mem* reteload_am_from_index(agent* thisAgent, FILE* f)
{
    uint64_t amindex;

    amindex = reteload_eight_bytes(f) - 1;
    if (amindex >= thisAgent->reteload_num_ams)
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg,
                "Internal error (file corrupted?): alpha mem count too small\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }
    return *(thisAgent->reteload_am_table + amindex);
}

void reteload_free_am_table(agent* thisAgent)
{
    uint64_t i;

    for (i = 0; i < thisAgent->reteload_num_ams; i++)
    {
        remove_ref_to_alpha_mem(thisAgent, *(thisAgent->reteload_am_table + i));
    }
    thisAgent->memoryManager->free_memory(thisAgent->reteload_am_table, MISCELLANEOUS_MEM_USAGE);
}

/* ----------------------------------------------------------------------
                  Save/Load Varnames and Node_Varnames

  These routines write out and read in node varnames records.

  Node_varnames record:
    records (in bottom-up order) -- start at bottom, walk up net,
      into NCC's as we go along; for each node, write three field varnames

  varnames record:
    type (1 byte): 0=null, 1=one var, 2=list
    if one var: 4 bytes (symindex)
    if list: 4 bytes (number of items) + list of symindices
---------------------------------------------------------------------- */

void retesave_varnames(varnames* names, FILE* f)
{
    cons* c;
    uint64_t i;
    Symbol* sym;

    if (! names)
    {
        retesave_one_byte(0, f);
    }
    else if (varnames_is_one_var(names))
    {
        retesave_one_byte(1, f);
        sym = varnames_to_one_var(names);
        retesave_eight_bytes(sym->retesave_symindex, f);
    }
    else
    {
        retesave_one_byte(2, f);
        for (i = 0, c = varnames_to_var_list(names); c != NIL; i++, c = c->rest);
        retesave_eight_bytes(i, f);
        for (c = varnames_to_var_list(names); c != NIL; c = c->rest)
        {
            retesave_eight_bytes(static_cast<Symbol*>(c->first)->retesave_symindex, f);
        }
    }
}

varnames* reteload_varnames(agent* thisAgent, FILE* f)
{
    cons* c;
    uint64_t i, count;
    Symbol* sym;

    i = reteload_one_byte(f);
    if (i == 0)
    {
        return NIL;
    }
    if (i == 1)
    {
        sym = reteload_symbol_from_index(thisAgent, f);
        thisAgent->symbolManager->symbol_add_ref(sym);
        return one_var_to_varnames(sym);
    }
    else
    {
        count = reteload_eight_bytes(f);
        c = NIL;
        while (count--)
        {
            sym = reteload_symbol_from_index(thisAgent, f);
            thisAgent->symbolManager->symbol_add_ref(sym);
            push(thisAgent, sym, c);
        }
        c = destructively_reverse_list(c);
        return var_list_to_varnames(c);
    }
}

void retesave_node_varnames(node_varnames* nvn, rete_node* node, FILE* f)
{
    while (true)
    {
        if (node->node_type == DUMMY_TOP_BNODE)
        {
            return;
        }
        if (node->node_type == CN_BNODE)
        {
            node = node->b.cn.partner->parent;
            nvn = nvn->data.bottom_of_subconditions;
            continue;
        }
        retesave_varnames(nvn->data.fields.id_varnames, f);
        retesave_varnames(nvn->data.fields.attr_varnames, f);
        retesave_varnames(nvn->data.fields.value_varnames, f);
        nvn = nvn->parent;
        node = real_parent_node(node);
    }
}

node_varnames* reteload_node_varnames(agent* thisAgent, rete_node* node, FILE* f)
{
    node_varnames* nvn, *nvn_for_ncc;
    rete_node* temp;

    if (node->node_type == DUMMY_TOP_BNODE)
    {
        return NIL;
    }
    thisAgent->memoryManager->allocate_with_pool(MP_node_varnames, &nvn);
    if (node->node_type == CN_BNODE)
    {
        temp = node->b.cn.partner->parent;
        nvn_for_ncc = reteload_node_varnames(thisAgent, temp, f);
        nvn->data.bottom_of_subconditions = nvn_for_ncc;
        while (temp != node->parent)
        {
            temp = real_parent_node(temp);
            nvn_for_ncc = nvn_for_ncc->parent;
        }
        nvn->parent = nvn_for_ncc;
    }
    else
    {
        nvn->data.fields.id_varnames = reteload_varnames(thisAgent, f);
        nvn->data.fields.attr_varnames = reteload_varnames(thisAgent, f);
        nvn->data.fields.value_varnames = reteload_varnames(thisAgent, f);
        nvn->parent = reteload_node_varnames(thisAgent, real_parent_node(node), f);
    }
    return nvn;
}

/* ----------------------------------------------------------------------
                            Save/Load RHS Values

  RHS value record:
    1 byte: type (0=symbol, 1=funcall, 2=reteloc, 3=rhs_unbound_var)
    for symbols: 4 bytes (symindex)
    for funcalls: symindex of function name, 4 bytes (# of args),
       rhs value record for each arg
    for retelocs: 1 byte (field num) + 2 bytes (levels up)
    for rhs_unbound_vars: 4 bytes (symindex)
---------------------------------------------------------------------- */

void retesave_rhs_value(rhs_value rv, FILE* f)
{
    uint64_t i;
    Symbol* sym;
    cons* c;

    if (rhs_value_is_symbol(rv))
    {
        retesave_one_byte(0, f);
        sym = rhs_value_to_symbol(rv);
        retesave_eight_bytes(sym->retesave_symindex, f);
    }
    else if (rhs_value_is_funcall(rv))
    {
        retesave_one_byte(1, f);
        c = rhs_value_to_funcall_list(rv);
        sym = static_cast<rhs_function*>(c->first)->name;
        retesave_eight_bytes(sym->retesave_symindex, f);
        c = c->rest;
        for (i = 0; c != NIL; i++, c = c->rest);
        retesave_eight_bytes(i, f);
        for (c = rhs_value_to_funcall_list(rv)->rest; c != NIL; c = c->rest)
        {
            retesave_rhs_value(static_cast<rhs_value>(c->first), f);
        }
    }
    else if (rhs_value_is_reteloc(rv))
    {
        retesave_one_byte(2, f);
        retesave_one_byte(rhs_value_to_reteloc_field_num(rv), f);
        retesave_two_bytes(rhs_value_to_reteloc_levels_up(rv), f);
    }
    else
    {
        retesave_one_byte(3, f);
        retesave_eight_bytes(rhs_value_to_unboundvar(rv), f);
    }
}

rhs_value reteload_rhs_value(agent* thisAgent, FILE* f)
{
    rhs_value rv, temp;
    uint64_t i, count;
    Symbol* sym;
    byte type, field_num;
    int levels_up;
    cons* funcall_list;
    rhs_function* rf;

    type = reteload_one_byte(f);
    switch (type)
    {
        case 0:
            sym = reteload_symbol_from_index(thisAgent, f);
            rv = allocate_rhs_value_for_symbol(thisAgent, sym, 0);
            break;
        case 1:
            funcall_list = NIL;
            sym = reteload_symbol_from_index(thisAgent, f);

            /* I traced through production parsing and the RHS function name is not kept around there. Instead, it "finds" the symbol
             * (as opposed to "make", which adds a ref) and uses that to hash to the existing RHS function structure (which keeps a
             * ref on the symbol name). The initial symbol ref comes from init_built_in_rhs_functions (+1 ref) and then is removed
             * later via remove_built_in_rhs_functions (-1 ref).
             *
             * The parallel in rete-net loading is the symbol table that is loaded in via reteload_all_symbols (+1 ref) and then freed
             * in reteload_free_symbol_table (-1 ref).
                 *
                 * - NLD: 4/30/2011
             */
            // thisAgent->symbolManager->symbol_add_ref(sym);

            rf = lookup_rhs_function(thisAgent, sym);
            if (!rf)
            {
                char msg[BUFFER_MSG_SIZE];
                thisAgent->outputManager->printa_sf(thisAgent, "Error: can't load this file because it uses an undefined RHS function %y\n", sym);
                SNPRINTF(msg, BUFFER_MSG_SIZE, "Error: can't load this file because it uses an undefined RHS function %s\n", sym->to_string(true));
                msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
                abort_with_fatal_error(thisAgent, msg);
            }
            push(thisAgent, rf, funcall_list);
            count = reteload_eight_bytes(f);
            while (count--)
            {
                temp = reteload_rhs_value(thisAgent, f);
                push(thisAgent, temp, funcall_list);
            }
            funcall_list = destructively_reverse_list(funcall_list);
            rv = funcall_list_to_rhs_value(funcall_list);
            break;
        case 2:
            field_num = reteload_one_byte(f);
            levels_up = reteload_two_bytes(f);
            rv = reteloc_to_rhs_value(field_num, static_cast<rete_node_level>(levels_up));
            break;
        case 3:
            i = reteload_eight_bytes(f);
            update_max_rhs_unbound_variables(thisAgent, i + 1);
            rv = unboundvar_to_rhs_value(i);
            break;
        default:
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg, "Internal error (file corrupted?): bad rhs_value type\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }
            rv = NIL;  /* unreachable, but without it gcc -Wall warns */
    }
    return rv;
}

/* ----------------------------------------------------------------------
                          Save/Load RHS Actions

  Record for a single RHS action:
    1 byte: type
    1 byte: preference type
    1 byte: support
    for FUNCALL_ACTION's: rhs value record for value
    for MAKE_ACTION's: rhs value records for id, attr, value,
       and referent if binary

  Record for a list of RHS actions:
    4 bytes: number of RHS actions in the list
    record for each one (as above)
---------------------------------------------------------------------- */

void retesave_rhs_action(action* a, FILE* f)
{
    retesave_one_byte(a->type, f);
    retesave_one_byte(a->preference_type, f);
    retesave_one_byte(a->support, f);
    if (a->type == FUNCALL_ACTION)
    {
        retesave_rhs_value(a->value, f);
    }
    else     /* MAKE_ACTION's */
    {
        retesave_rhs_value(a->id, f);
        retesave_rhs_value(a->attr, f);
        retesave_rhs_value(a->value, f);
        if (preference_is_binary(a->preference_type))
        {
            retesave_rhs_value(a->referent, f);
        }
    }
}

action* reteload_rhs_action(agent* thisAgent, FILE* f)
{
    action* a;

    a = make_action(thisAgent);
    a->type = static_cast<ActionType>(reteload_one_byte(f));
    a->preference_type = static_cast<PreferenceType>(reteload_one_byte(f));
    a->support = static_cast<SupportType>(reteload_one_byte(f));
    if (a->type == FUNCALL_ACTION)
    {
        a->value = reteload_rhs_value(thisAgent, f);
    }
    else     /* MAKE_ACTION's */
    {
        a->id = reteload_rhs_value(thisAgent, f);
        a->attr = reteload_rhs_value(thisAgent, f);
        a->value = reteload_rhs_value(thisAgent, f);
        if (preference_is_binary(a->preference_type))
        {
            a->referent = reteload_rhs_value(thisAgent, f);
        }
        else
        {
            a->referent = NIL;
        }
    }
    return a;
}

void retesave_action_list(action* first_a, FILE* f)
{
    uint64_t i;
    action* a;

    for (i = 0, a = first_a; a != NIL; i++, a = a->next);
    retesave_eight_bytes(i, f);
    for (a = first_a; a != NIL; a = a->next)
    {
        retesave_rhs_action(a, f);
    }
}

action* reteload_action_list(agent* thisAgent, FILE* f)
{
    action* a, *prev_a, *first_a;
    uint64_t count;

    count = reteload_eight_bytes(f);
    prev_a = NIL;
    first_a = NIL;  /* unneeded, but without it gcc -Wall warns here */
    while (count--)
    {
        a = reteload_rhs_action(thisAgent, f);
        if (prev_a)
        {
            prev_a->next = a;
        }
        else
        {
            first_a = a;
        }
        prev_a = a;
    }
    if (prev_a)
    {
        prev_a->next = NIL;
    }
    else
    {
        first_a = NIL;
    }
    return first_a;
}

/* ----------------------------------------------------------------------
                            Save/Load Rete Tests

  Record for a single Rete test:
    1 byte: test type
    1 byte: right_field_num
    other data:
      for relational test to variable: 3 bytes -- field num (1), levels up (2)
      for relational test to constant: 4 bytes -- symindex of the constant
      for disjunctions: 4 bytes (number of disjuncts) then list of symindices

  Record for a list of Rete tests:
    2 bytes -- number of tests in the list
    Rete test records (as above) for each one
---------------------------------------------------------------------- */

void retesave_rete_test(rete_test* rt, FILE* f)
{
    int i;
    cons* c;

    retesave_one_byte(rt->type, f);
    retesave_one_byte(rt->right_field_num, f);
    if (test_is_constant_relational_test(rt->type))
    {
        retesave_eight_bytes(rt->data.constant_referent->retesave_symindex, f);
    }
    else if (test_is_variable_relational_test(rt->type))
    {
        retesave_one_byte(rt->data.variable_referent.field_num, f);
        retesave_two_bytes(rt->data.variable_referent.levels_up, f);
    }
    else if (rt->type == DISJUNCTION_RETE_TEST)
    {
        for (i = 0, c = rt->data.disjunction_list; c != NIL; i++, c = c->rest);
        retesave_two_bytes(static_cast<uint16_t>(i), f);
        for (c = rt->data.disjunction_list; c != NIL; c = c->rest)
        {
            retesave_eight_bytes(static_cast<Symbol*>(c->first)->retesave_symindex, f);
        }
    }
}

rete_test* reteload_rete_test(agent* thisAgent, FILE* f)
{
    rete_test* rt;
    Symbol* sym;
    uint64_t count;
    cons* temp;

    thisAgent->memoryManager->allocate_with_pool(MP_rete_test, &rt);
    rt->type = reteload_one_byte(f);
    rt->right_field_num = reteload_one_byte(f);

    if (test_is_constant_relational_test(rt->type))
    {
        rt->data.constant_referent = reteload_symbol_from_index(thisAgent, f);
        thisAgent->symbolManager->symbol_add_ref(rt->data.constant_referent);
    }
    else if (test_is_variable_relational_test(rt->type))
    {
        rt->data.variable_referent.field_num = reteload_one_byte(f);
        rt->data.variable_referent.levels_up = static_cast<rete_node_level>(reteload_two_bytes(f));

    }
    else if (rt->type == DISJUNCTION_RETE_TEST)
    {
        count = reteload_two_bytes(f);
        temp = NIL;
        while (count--)
        {
            sym = reteload_symbol_from_index(thisAgent, f);
            thisAgent->symbolManager->symbol_add_ref(sym);
            push(thisAgent, sym, temp);
        }
        rt->data.disjunction_list = destructively_reverse_list(temp);
    }
    return rt;
}

void retesave_rete_test_list(rete_test* first_rt, FILE* f)
{
    uint64_t i;
    rete_test* rt;

    for (i = 0, rt = first_rt; rt != NIL; i++, rt = rt->next);
    retesave_two_bytes(static_cast<uint16_t>(i), f);
    for (rt = first_rt; rt != NIL; rt = rt->next)
    {
        retesave_rete_test(rt, f);
    }
}

rete_test* reteload_rete_test_list(agent* thisAgent, FILE* f)
{
    rete_test* rt, *prev_rt, *first;
    uint64_t count;

    prev_rt = NIL;
    first = NIL;  /* unneeded, but without it gcc -Wall warns here */
    count = reteload_two_bytes(f);
    while (count--)
    {
        rt = reteload_rete_test(thisAgent, f);
        if (prev_rt)
        {
            prev_rt->next = rt;
        }
        else
        {
            first = rt;
        }
        prev_rt = rt;
    }
    if (prev_rt)
    {
        prev_rt->next = NIL;
    }
    else
    {
        first = NIL;
    }
    return first;
}

/* ----------------------------------------------------------------------
                         Save/Load Rete Nodes

   These routines save/reload data for Rete nodes (and their descendents).
   Retesave_children_of_node() writes out the records for the children
   of a given node (and their descendents).  Retesave_rete_node_and_children()
   writes out the record for a given node (which includes the records for all
   its descendents).  The records have the following format:

   Node record:
     1 byte: node type
     data for node:
       posneg nodes: if hashed (and not P): 3 bytes: hash field num, levels up
                     4 bytes -- index of alpha memory
                     record for rete test list (for other tests)
                     pos and mp nodes: 1 byte -- 1 if left unlinked, 0 else
       mem nodes: if hashed: 3 bytes -- hash field num, levels up
       cn nodes: no record at all (not even node type) -- handled with cn_p
                 node record instead.  Basically, we ignore the cn node when
                 writing the net, and pretend the cn/cn_p pair is one big
                 node underneath the subnetwork.
       cn_p nodes: number of conjuncts in the NCC
       p_nodes: 4 bytes: name of production (symindex)
                1 byte (0 or 1): flag -- is there a documentation string
                  if yes: documentation string (null-terminated string)
                1 byte: type
                1 byte: declared support
                record for the list of RHS actions
                4 bytes: number of RHS unbound variables
                  RHS unbound variables (symindices for each one)
                1 byte (0 or 1): flag -- is there node_varnames info?
                  if yes:  node_varnames records for this production
     4 bytes: number of children
     node records for each child

   Note that we write out a flag indicating whether join nodes are
   currently left-unlinked or not.  This is for the join nodes underneath
   a huge fan-out from a beta memory -- most of these will be left-unlinked.
   Since by default we right-unlink newly-created nodes rather than
   left-unlinking them, without special handling these nodes would be
   right-unlinked when we reload the network.  This would lead to a large
   startup penalty due to a large number of initial null left activations.

   Reteload_node_and_children() reads in the record for a given node and
   all its descendents, and reconstructs the Rete network structures.
---------------------------------------------------------------------- */

void retesave_rete_node_and_children(agent* thisAgent, rete_node* node, FILE* f);

void retesave_children_of_node(agent* thisAgent, rete_node* node, FILE* f)
{
    rete_node* child;
    std::stack<rete_node*> nodeStack;

    /* --- Count number of non-CN-node children. --- */
    for (child = node->first_child; child; child = child->next_sibling)
    {
        if (child->node_type != CN_BNODE)
        {
            nodeStack.push(child);
        }
    }
    retesave_eight_bytes(nodeStack.size(), f);

    /* --- Write out records for all the node's children except CN's. --- */
    while (!nodeStack.empty())
    {
        retesave_rete_node_and_children(thisAgent, nodeStack.top(), f);
        nodeStack.pop();
    }
}

void retesave_rete_node_and_children(agent* thisAgent, rete_node* node, FILE* f)
{
    uint64_t i;
    production* prod;
    cons* c;
    rete_node* temp;

    if (node->node_type == CN_BNODE)
    {
        return;    /* ignore CN nodes */
    }

    retesave_one_byte(node->node_type, f);

    switch (node->node_type)
    {
        case MEMORY_BNODE:
            retesave_one_byte(node->left_hash_loc_field_num, f);
            retesave_two_bytes(node->left_hash_loc_levels_up, f);
        /* ... and fall through to the next case below ... */
        case UNHASHED_MEMORY_BNODE:
            break;

        case MP_BNODE:
            retesave_one_byte(node->left_hash_loc_field_num, f);
            retesave_two_bytes(node->left_hash_loc_levels_up, f);
        /* ... and fall through to the next case below ... */
        case UNHASHED_MP_BNODE:
            retesave_eight_bytes(node->b.posneg.alpha_mem_->retesave_amindex, f);
            retesave_rete_test_list(node->b.posneg.other_tests, f);
            retesave_one_byte(static_cast<byte>(node->a.np.is_left_unlinked ? 1 : 0), f);
            break;

        case POSITIVE_BNODE:
        case UNHASHED_POSITIVE_BNODE:
            retesave_eight_bytes(node->b.posneg.alpha_mem_->retesave_amindex, f);
            retesave_rete_test_list(node->b.posneg.other_tests, f);
            retesave_one_byte(static_cast<byte>(node_is_left_unlinked(node) ? 1 : 0), f);
            break;

        case NEGATIVE_BNODE:
            retesave_one_byte(node->left_hash_loc_field_num, f);
            retesave_two_bytes(node->left_hash_loc_levels_up, f);
        /* ... and fall through to the next case below ... */
        case UNHASHED_NEGATIVE_BNODE:
            retesave_eight_bytes(node->b.posneg.alpha_mem_->retesave_amindex, f);
            retesave_rete_test_list(node->b.posneg.other_tests, f);
            break;

        case CN_PARTNER_BNODE:
            i = 0;
            temp = real_parent_node(node);
            while (temp != node->b.cn.partner->parent)
            {
                temp = real_parent_node(temp);
                i++;
            }
            retesave_eight_bytes(i, f);
            break;

        case P_BNODE:
            prod = node->b.p.prod;
            retesave_eight_bytes(prod->name->retesave_symindex, f);
            if (prod->documentation)
            {
                retesave_one_byte(1, f);
                retesave_string(prod->documentation, f);
            }
            else
            {
                retesave_one_byte(0, f);
            }
            retesave_one_byte(prod->type, f);
            retesave_one_byte(prod->declared_support, f);
            retesave_action_list(prod->action_list, f);
            for (i = 0, c = prod->rhs_unbound_variables; c != NIL; i++, c = c->rest);
            retesave_eight_bytes(i, f);
            for (c = prod->rhs_unbound_variables; c != NIL; c = c->rest)
            {
                retesave_eight_bytes(static_cast<Symbol*>(c->first)->retesave_symindex, f);
            }
            if (node->b.p.parents_nvn)
            {
                retesave_one_byte(1, f);
                retesave_node_varnames(node->b.p.parents_nvn, node->parent, f);
            }
            else
            {
                retesave_one_byte(0, f);
            }
            break;

        default:
        {
            char msg[BUFFER_MSG_SIZE];
            SNPRINTF(msg, BUFFER_MSG_SIZE,
                     "Internal error: fastsave found node type %d\n", node->node_type);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }
    } /* end of switch statement */

    /* --- For cn_p nodes, write out the CN node's children instead --- */
    if (node->node_type == CN_PARTNER_BNODE)
    {
        node = node->b.cn.partner;
    }
    /* --- Write out records for all the node's children. --- */
    retesave_children_of_node(thisAgent, node, f);
}

void reteload_node_and_children(agent* thisAgent, rete_node* parent, FILE* f)
{
    byte type, left_unlinked_flag;
    rete_node* New, *ncc_top;
    uint64_t count;
    alpha_mem* am;
    production* prod;
    Symbol* sym;
    cons* ubv_list;
    var_location left_hash_loc;
    rete_test* other_tests;

    type = reteload_one_byte(f);

    /*
       Initializing the left_hash_loc structure to flag values.
       It gets passed into some of the various make_new_??? functions
       below but is never used (hopefully) for UNHASHED node types.
    */
    left_hash_loc.field_num = static_cast<byte>(-1);
    left_hash_loc.levels_up = static_cast<rete_node_level>(-1);

    switch (type)
    {
        case MEMORY_BNODE:
            left_hash_loc.field_num = reteload_one_byte(f);
            left_hash_loc.levels_up = static_cast<rete_node_level>(reteload_two_bytes(f));
        /* ... and fall through to the next case below ... */
        case UNHASHED_MEMORY_BNODE:
            New = make_new_mem_node(thisAgent, parent, type, left_hash_loc);
            break;

        case MP_BNODE:
            left_hash_loc.field_num = reteload_one_byte(f);
            left_hash_loc.levels_up = static_cast<rete_node_level>(reteload_two_bytes(f));
        /* ... and fall through to the next case below ... */
        case UNHASHED_MP_BNODE:
            am = reteload_am_from_index(thisAgent, f);
            am->reference_count++;
            other_tests = reteload_rete_test_list(thisAgent, f);
            left_unlinked_flag = reteload_one_byte(f);
            New = make_new_mp_node(thisAgent, parent, type, left_hash_loc, am, other_tests,
                                   left_unlinked_flag != 0);
            break;

        case POSITIVE_BNODE:
        case UNHASHED_POSITIVE_BNODE:
            am = reteload_am_from_index(thisAgent, f);
            am->reference_count++;
            other_tests = reteload_rete_test_list(thisAgent, f);
            left_unlinked_flag = reteload_one_byte(f);
            New = make_new_positive_node(thisAgent, parent, type, am, other_tests,
                                         left_unlinked_flag != 0);
            break;

        case NEGATIVE_BNODE:
            left_hash_loc.field_num = reteload_one_byte(f);
            left_hash_loc.levels_up = static_cast<rete_node_level>(reteload_two_bytes(f));
        /* ... and fall through to the next case below ... */
        case UNHASHED_NEGATIVE_BNODE:
            am = reteload_am_from_index(thisAgent, f);
            am->reference_count++;
            other_tests = reteload_rete_test_list(thisAgent, f);
            New = make_new_negative_node(thisAgent, parent, type, left_hash_loc, am, other_tests);
            break;

        case CN_PARTNER_BNODE:
            count = reteload_eight_bytes(f);
            ncc_top = parent;
            while (count--)
            {
                ncc_top = real_parent_node(ncc_top);
            }
            New = make_new_cn_node(thisAgent, ncc_top, parent);
            break;

        case P_BNODE:
            thisAgent->memoryManager->allocate_with_pool(MP_production, &prod);
            prod->reference_count = 1;
            prod->firing_count = 0;
            prod->trace_firings = false;
            prod->instantiations = NIL;
            prod->filename = NIL;
            prod->p_node = NIL;
            prod->interrupt = false;
            prod->interrupt_break = false;
            prod->duplicate_chunks_this_cycle = 0;
            prod->last_duplicate_dc = 0;
            prod->explain_its_chunks = false;
            prod->save_for_justification_explanation = false;
            prod->p_id = thisAgent->explanationBasedChunker->get_new_prod_id();

            sym = reteload_symbol_from_index(thisAgent, f);
            thisAgent->symbolManager->symbol_add_ref(sym);
            prod->name = sym;
            /* If this rule was a chunk, then original rule name might be different.  To
             * avoid having to alter rete saving, we'll just make the original name after
             * a rete load the same thing as a the saved name.  Side effect is minimal:
             * longer chunk name on chunks based on chunks loaded from rete net, for
             * example chunk*chunk-x2*apply-d12 instead of chunk-x3*apply*d12 */
            prod->original_rule_name = make_memory_block_for_string(thisAgent, prod->name->sc->name);
            sym->sc->production = prod;
            if (reteload_one_byte(f))
            {
                reteload_string(f);
                prod->documentation = make_memory_block_for_string(thisAgent, reteload_string_buf);
            }
            else
            {
                prod->documentation = NIL;
            }
            prod->type = static_cast<ProductionType>(reteload_one_byte(f));
            prod->declared_support = static_cast<SupportType>(reteload_one_byte(f));
            prod->action_list = reteload_action_list(thisAgent, f);

            count = reteload_eight_bytes(f);
            update_max_rhs_unbound_variables(thisAgent, count);
            ubv_list = NIL;
            while (count--)
            {
                sym = reteload_symbol_from_index(thisAgent, f);
                thisAgent->symbolManager->symbol_add_ref(sym);
                push(thisAgent, sym, ubv_list);
            }
            prod->rhs_unbound_variables = destructively_reverse_list(ubv_list);

            insert_at_head_of_dll(thisAgent->all_productions_of_type[prod->type],
                                  prod, next, prev);
            thisAgent->num_productions_of_type[prod->type]++;

            // Soar-RL stuff
            prod->rl_update_count = 0.0;
            prod->rl_delta_bar_delta_beta = -3.0;
            prod->rl_delta_bar_delta_h = 0.0;
            prod->rl_update_count = 0;
            prod->rl_rule = false;
            prod->rl_ecr = 0.0;
            prod->rl_efr = 0.0;
            prod->rl_gql = 0.0;
            if ((prod->type != JUSTIFICATION_PRODUCTION_TYPE) && (prod->type != TEMPLATE_PRODUCTION_TYPE))
            {
                prod->rl_rule = rl_valid_rule(prod);
                if (prod->rl_rule)
                {
                    prod->rl_efr = get_number_from_symbol(rhs_value_to_symbol(prod->action_list->referent));

                    if (prod->documentation)
                    {
                        rl_rule_meta(thisAgent, prod);
                    }
                }
            }
            prod->rl_template_conds = NIL;

            New = make_new_production_node(thisAgent, parent, prod);
            adjust_sharing_factors_from_here_to_top(New, 1);
            if (reteload_one_byte(f))
            {
                New->b.p.parents_nvn = reteload_node_varnames(thisAgent, parent, f);
            }
            else
            {
                New->b.p.parents_nvn = NIL;
            }

            /* --- call new node's add_left routine with all the parent's tokens --- */
            update_node_with_matches_from_above(thisAgent, New);

            /* --- invoke callback on the production --- */
            soar_invoke_callbacks(thisAgent, PRODUCTION_JUST_ADDED_CALLBACK, static_cast<soar_call_data>(prod));

            break;

        default:
        {
            char msg[BUFFER_MSG_SIZE];
            SNPRINTF(msg, BUFFER_MSG_SIZE, "Internal error: fastload found node type %d\n", type);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
            New = NIL; /* unreachable, but without it gcc -Wall warns here */
        }
    } /* end of switch statement */

    /* --- read in the children of the node --- */
    count = reteload_eight_bytes(f);
    while (count--)
    {
        reteload_node_and_children(thisAgent, New, f);
    }
}

/* ----------------------------------------------------------------------
                        Save/Load The Whole Net

  Save_rete_net() and load_rete_net() save and load everything to and
  from the given (already open) files.  They return true if successful,
  false if any error occurred.
---------------------------------------------------------------------- */

bool save_rete_net(agent* thisAgent, FILE* dest_file, bool use_rete_net_64)
{

    /* --- make sure there are no justifications present --- */
    if (thisAgent->all_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE])
    {
        thisAgent->outputManager->printa_sf(thisAgent, "Internal error: save_rete_net() with justifications present.\n");
        return false;
    }

    rete_fs_file = dest_file;
    rete_net_64 = use_rete_net_64;
    uint8_t version = use_rete_net_64 ? 4 : 3;

    retesave_string("SoarCompactReteNet\n", dest_file);
    retesave_one_byte(version, dest_file);  /* format version number */
    retesave_symbol_table(thisAgent, dest_file);
    retesave_alpha_memories(thisAgent, dest_file);
    retesave_children_of_node(thisAgent, thisAgent->dummy_top_node, dest_file);
    return true;
}

bool load_rete_net(agent* thisAgent, FILE* source_file)
{
    int format_version_num;
    uint64_t i, count;

    /* RDF: 20020814 RDF Cleaning up the agent working memory and production
       memory to avoid unnecessary errors in this function. */
    reinitialize_soar(thisAgent);
    excise_all_productions(thisAgent, true);

    /* DONE clearing old productions */

    /* --- check for empty system --- */
    if (thisAgent->all_wmes_in_rete)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "Internal error: load_rete_net() called with nonempty WM.\n");
        return false;
    }
    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
        if (thisAgent->num_productions_of_type[i])
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Internal error: load_rete_net() called with nonempty PM.\n");
            return false;
        }

    // BADBAD: this is global, used in retesave_one_byte
    rete_fs_file = source_file;

    /* --- read file header, make sure it's a valid file --- */
    reteload_string(source_file);
    if (strcmp(reteload_string_buf, "SoarCompactReteNet\n"))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "This file isn't a Soar fastsave file.\n");
        return false;
    }
    format_version_num = reteload_one_byte(source_file);
    switch (format_version_num)
    {
        case 3:
            // Since there's already a global, I'm putting the 32- or 64-bit switch out there globally
            rete_net_64 = false; // used by reteload_eight_bytes
            break;
        case 4:
            // Since there's already a global, I'm putting the 32- or 64-bit switch out there globally
            rete_net_64 = true; // used by reteload_eight_bytes
            break;
        default:
            thisAgent->outputManager->printa_sf(thisAgent, "This file is in a format (version %d) I don't understand.\n", format_version_num);
            return false;
    }

    reteload_all_symbols(thisAgent, source_file);
    reteload_alpha_memories(thisAgent, source_file);
    count = reteload_eight_bytes(source_file);
    while (count--)
    {
        reteload_node_and_children(thisAgent, thisAgent->dummy_top_node, source_file);
    }

    /* --- clean up auxilliary tables --- */
    reteload_free_am_table(thisAgent);
    reteload_free_symbol_table(thisAgent);

    /* RDF: 20020814 Now adding the top state and io symbols and wmes */
    init_agent_memory(thisAgent);

    return true;
}














/* **********************************************************************

   SECTION 18:  Statistics and User Interface Utilities

   EXTERNAL INTERFACE:
   Count_rete_tokens_for_production() returns a count of the number of
   tokens currently in use for the given production.
   Print_partial_match_information(), print_match_set(), and
   print_rete_statistics() do printouts for various interface routines.
   Get_node_count_statistic() is for TclSoar to get an individual stat.
********************************************************************** */

/* ----------------------------------------------------------------------
                    Count Rete Tokens For Production

   Returns a count of the number of tokens currently in use for the given
   production.  The count does not include:
     tokens in the p_node (i.e., tokens representing complete matches)
     local join result tokens on (real) tokens in negative/NCC nodes
---------------------------------------------------------------------- */

uint64_t count_rete_tokens_for_production(agent* thisAgent, production* prod)
{
    uint64_t count;
    rete_node* node;
    token* tok;

    if (! prod->p_node)
    {
        return 0;
    }
    node = prod->p_node->parent;
    count = 0;
    while (node != thisAgent->dummy_top_node)
    {
        if ((node->node_type != POSITIVE_BNODE) &&
                (node->node_type != UNHASHED_POSITIVE_BNODE))
        {
            for (tok = node->a.np.tokens; tok != NIL; tok = tok->next_of_node)
            {
                count++;
            }
        }
        if (node->node_type == CN_BNODE)
        {
            node = node->b.cn.partner->parent;
        }
        else
        {
            node = node->parent;
        }
    }
    return count;
}

/* --------------------------------------------------------------------
                          Rete Statistics

   Get_all_node_count_stats() sets up the three arrays actual[],
   if_no_merging[], and if_no_sharing[] to contain the current node
   counts of each type of node.  Actual[] gives the actual count.
   If_no_merging[] tells what the count would be if we never merged
   Mem and Pos nodes into MP nodes.  If_no_sharing[] tells what the
   count would be if we didn't share beta nodes across productions AND
   didn't merge Mem+Pos into MP nodes.  (I did it this way so we can
   tell what the static sharing factor is *without* having to worry
   about the merging stuff, which is not a standard Rete technique.)

   Print_node_count_statistics() prints everything out.
   Get_node_count_statistic() is the main routine for TclSoar.
   Print_rete_statistics() is the main routine for non-TclSoar.
-------------------------------------------------------------------- */
void init_bnode_type_names(agent* /*thisAgent*/)
{
    static bool bnode_initialzied = false;

    //
    // This should be properly locked.
    //
    if (!bnode_initialzied)
    {
        bnode_type_names[UNHASHED_MEMORY_BNODE]   = "unhashed memory";
        bnode_type_names[MEMORY_BNODE]            = "memory";
        bnode_type_names[UNHASHED_MP_BNODE]       = "unhashed mem-pos";
        bnode_type_names[MP_BNODE]                = "mem-pos";
        bnode_type_names[UNHASHED_POSITIVE_BNODE] = "unhashed positive";
        bnode_type_names[POSITIVE_BNODE]          = "positive";
        bnode_type_names[NEGATIVE_BNODE]          = "negative";
        bnode_type_names[UNHASHED_NEGATIVE_BNODE] = "unhashed negative";
        bnode_type_names[DUMMY_TOP_BNODE]         = "dummy top";
        bnode_type_names[DUMMY_MATCHES_BNODE]     = "dummy matches";
        bnode_type_names[CN_BNODE]                = "conj. neg.";
        bnode_type_names[CN_PARTNER_BNODE]        = "conj. neg. partner";
        bnode_type_names[P_BNODE]                 = "production";

        bnode_initialzied = true;
    }
}



void get_all_node_count_stats(agent* thisAgent)
{
    int i;

    //
    // This sanity check should no longer be neccessary.
    //
    /* --- sanity check: make sure we've got names for all the bnode types --- */
    //for (i=0; i<256; i++)
    //  if (thisAgent->rete_node_counts[i] &&
    //      (*bnode_type_names[i] == 0)) {
    //    thisAgent->OutputManager->print( "Internal eror: unknown node type [%d] has nonzero count.\n",i);
    //  }
    init_bnode_type_names(thisAgent);

    /* --- calculate the three arrays --- */
    for (i = 0; i < 256; i++)
    {
        thisAgent->actual[i] = thisAgent->rete_node_counts[i];
        thisAgent->if_no_merging[i] = thisAgent->rete_node_counts[i];
        thisAgent->if_no_sharing[i] = thisAgent->rete_node_counts_if_no_sharing[i];
    }

    /* --- don't want the dummy matches node to show up as a real node --- */
    thisAgent->actual[DUMMY_MATCHES_BNODE] = 0;
    thisAgent->if_no_merging[DUMMY_MATCHES_BNODE] = 0;
    thisAgent->if_no_sharing[DUMMY_MATCHES_BNODE] = 0;

    /* --- If no merging or sharing, each MP node would be 1 Mem + 1 Pos --- */
    thisAgent->if_no_merging[MEMORY_BNODE] += thisAgent->if_no_merging[MP_BNODE];
    thisAgent->if_no_merging[POSITIVE_BNODE] += thisAgent->if_no_merging[MP_BNODE];
    thisAgent->if_no_merging[MP_BNODE] = 0;
    thisAgent->if_no_merging[UNHASHED_MEMORY_BNODE] += thisAgent->if_no_merging[UNHASHED_MP_BNODE];
    thisAgent->if_no_merging[UNHASHED_POSITIVE_BNODE] += thisAgent->if_no_merging[UNHASHED_MP_BNODE];
    thisAgent->if_no_merging[UNHASHED_MP_BNODE] = 0;
    thisAgent->if_no_sharing[MEMORY_BNODE] += thisAgent->if_no_sharing[MP_BNODE];
    thisAgent->if_no_sharing[POSITIVE_BNODE] += thisAgent->if_no_sharing[MP_BNODE];
    thisAgent->if_no_sharing[MP_BNODE] = 0;
    thisAgent->if_no_sharing[UNHASHED_MEMORY_BNODE] += thisAgent->if_no_sharing[UNHASHED_MP_BNODE];
    thisAgent->if_no_sharing[UNHASHED_POSITIVE_BNODE] += thisAgent->if_no_sharing[UNHASHED_MP_BNODE];
    thisAgent->if_no_sharing[UNHASHED_MP_BNODE] = 0;
}

/* Returns 0 if result invalid, 1 if result valid */
int get_node_count_statistic(agent* thisAgent,
                             char* node_type_name,
                             char* column_name,
                             uint64_t* result)
{
    int i;
    uint64_t tot;

    get_all_node_count_stats(thisAgent);

    if (!strcmp("total", node_type_name))
    {
        if (!strcmp("actual", column_name))
        {
            for (tot = 0, i = 0; i < 256; i++)
            {
                tot += thisAgent->actual[i];
            }
            *result = tot;
        }
        else if (!strcmp("if-no-merging", column_name))
        {
            for (tot = 0, i = 0; i < 256; i++)
            {
                tot += thisAgent->if_no_merging[i];
            }
            *result = tot;
        }
#ifdef SHARING_FACTORS
        else if (!strcmp("if-no-sharing", column_name))
        {
            for (tot = 0, i = 0; i < 256; i++)
            {
                tot += thisAgent->if_no_sharing[i];
            }
            *result = tot;
        }
#endif
        else
        {
            return 0;
        }
    }
    else
    {
        for (i = 0; i < 256; i++)
            if (!strcmp(bnode_type_names[i], node_type_name))
            {
                if (!strcmp("actual", column_name))
                {
                    *result = thisAgent->actual[i];
                }
                else if (!strcmp("if-no-merging", column_name))
                {
                    *result = thisAgent->if_no_merging[i];
                }
#ifdef SHARING_FACTORS
                else if (!strcmp("if-no-sharing", column_name))
                {
                    *result = thisAgent->if_no_sharing[i];
                }
#endif
                else
                {
                    return 0;
                }
                return 1;
            }
        return 0;
    }

    return 1;
}

/* ----------------------------------------------------------------------

                Partial Match Information:  Utilities

   To get info on partial matches for a given production, we use several
   helper routines.  Get_all_left_tokens_emerging_from_node() returns
   the tokens (chained via their next_of_node links) that are currently
   the output resulting from a given node.  (I'm not sure, but I think
   that with the new tree-based removal, this routine is no longer needed,
   as the tokens are always available on a list on some child node, but
   I didn't bother rewriting these routines.)  The routine obtains these
   tokens by temporarily making the "dummy_matches_node" a child of the
   given node, and then calling update_node_with_matches_from_above().
   The dummy_matches_node_left_addition() routine then gets activated
   for each token, and it builds up the list.  When the caller is done,
   it should call deallocate_token_list() to free up this list.

   Print_whole_token() prints out a given token in the format appropriate
   for the given wme_trace_type: either a list of timetags, a list of
   WMEs, or no printout at all.
---------------------------------------------------------------------- */

void dummy_matches_node_left_addition(agent* thisAgent, rete_node* /*node*/, token* tok, wme* w)
{
    token* New;

    /* --- just add a token record to dummy_matches_node_tokens --- */
    thisAgent->memoryManager->allocate_with_pool(MP_token, &New);
    New->node = NIL;
    New->parent = tok;
    New->w = w;
    New->next_of_node = thisAgent->dummy_matches_node_tokens;
    thisAgent->dummy_matches_node_tokens = New;
}

token* get_all_left_tokens_emerging_from_node(agent* thisAgent, rete_node* node)
{
    token* result;
    rete_node dummy_matches_node;

    thisAgent->dummy_matches_node_tokens = NIL;
    dummy_matches_node.node_type = DUMMY_MATCHES_BNODE;
    dummy_matches_node.parent = node;
    dummy_matches_node.first_child = NIL;
    dummy_matches_node.next_sibling = NIL;
    update_node_with_matches_from_above(thisAgent, &dummy_matches_node);
    result = thisAgent->dummy_matches_node_tokens;
    return result;
}

void deallocate_token_list(agent* thisAgent, token* t)
{
    token* next;

    while (t)
    {
        next = t->next_of_node;
        thisAgent->memoryManager->free_with_pool(MP_token, t);
        t = next;
    }
}

void print_whole_token(agent* thisAgent, token* t, wme_trace_type wtt)
{
    if (t == thisAgent->dummy_top_token)
    {
        return;
    }
    print_whole_token(thisAgent, t->parent, wtt);
    if (t->w)
    {
        if (wtt == TIMETAG_WME_TRACE)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%u", t->w->timetag);
        }
        else if (wtt == FULL_WME_TRACE)
        {
            print_wme(thisAgent, t->w);
        }
        if (wtt != NONE_WME_TRACE)
        {
            thisAgent->outputManager->printa_sf(thisAgent, " ");
        }
    }
}

/* ----------------------------------------------------------------------

                   Printing Partial Match Information

   This is for the "matches" command.  Print_partial_match_information()
   is called from the interface routine; ppmi_aux() is a helper function.
   We first call p_node_to_conditions_and_nots() to get the condition
   list for the LHS.  We then (conceptually) start at the top of the
   net, with the first condition; for each condition, we collect the
   tokens output by the previous node, to find the number of matches here.
   We print the # of matches here; print this condition.  If this is
   the first cond that didn't have any match, then we also print its
   matches-for-left and matches-for-right.

   Of course, we can't actually start at the top of the net and work our
   way down, since we'd have no way to find our way the the correct
   p-node.  So instead, we use a recursive procedure that basically does
   the same thing.
---------------------------------------------------------------------- */

/* --- Print stuff for given node and higher, up to but not including the
       cutoff node.  Return number of matches at the given node/cond. --- */
int64_t ppmi_aux(agent* thisAgent,    /* current agent */
                 rete_node* node,    /* current node */
                 rete_node* cutoff,  /* don't print cutoff node or any higher */
                 condition* cond,    /* cond for current node */
                 wme_trace_type wtt, /* what type of printout to use */
                 int indent)         /* number of spaces indent */
{
    token* tokens, *t, *parent_tokens;
    right_mem* rm;
    int64_t matches_one_level_up;
    int64_t matches_at_this_level;
#define MATCH_COUNT_STRING_BUFFER_SIZE 20
    char match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE];
    rete_node* parent;

    /* --- find the number of matches for this condition --- */
    tokens = get_all_left_tokens_emerging_from_node(thisAgent, node);
    matches_at_this_level = 0;
    for (t = tokens; t != NIL; t = t->next_of_node)
    {
        matches_at_this_level++;
    }
    deallocate_token_list(thisAgent, tokens);

    /* --- if we're at the cutoff node, we're done --- */
    if (node == cutoff)
    {
        return matches_at_this_level;
    }

    /* --- do stuff higher up --- */
    parent = real_parent_node(node);
    matches_one_level_up = ppmi_aux(thisAgent, parent, cutoff,
                                    cond->prev, wtt, indent);

    /* --- Form string for current match count:  If an earlier cond had no
       matches, just leave it blank; if this is the first 0, use ">>>>" --- */
    if (! matches_one_level_up)
    {
        strncpy(match_count_string, "    ", MATCH_COUNT_STRING_BUFFER_SIZE);
        match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
    }
    else if (! matches_at_this_level)
    {
        strncpy(match_count_string, ">>>>", MATCH_COUNT_STRING_BUFFER_SIZE);
        match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
    }
    else
    {
        SNPRINTF(match_count_string, MATCH_COUNT_STRING_BUFFER_SIZE, "%4ld", static_cast<long int>(matches_at_this_level));
        match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
    }

    /* --- print extra indentation spaces --- */
    thisAgent->outputManager->print_spaces(thisAgent, indent);

    if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
    {
        /* --- recursively print match counts for the NCC subconditions --- */
        thisAgent->outputManager->printa_sf(thisAgent, "    -{\n");
        ppmi_aux(thisAgent, real_parent_node(node->b.cn.partner),
                 parent,
                 cond->data.ncc.bottom,
                 wtt,
                 indent + 5);
        thisAgent->outputManager->print_spaces(thisAgent, indent);
        thisAgent->outputManager->printa_sf(thisAgent, "%s }\n", match_count_string);
    }
    else
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%s", match_count_string);
        print_condition(thisAgent, cond);
        thisAgent->outputManager->printa_sf(thisAgent, "\n");
        /* --- if this is the first match-failure (0 matches), print info on
           matches for left and right --- */
        if (matches_one_level_up && (!matches_at_this_level))
        {
            if (wtt != NONE_WME_TRACE)
            {
                thisAgent->outputManager->print_spaces(thisAgent, indent);
                thisAgent->outputManager->printa_sf(thisAgent, "*** Matches For Left ***\n");
                parent_tokens = get_all_left_tokens_emerging_from_node(thisAgent, parent);
                for (t = parent_tokens; t != NIL; t = t->next_of_node)
                {
                    thisAgent->outputManager->print_spaces(thisAgent, indent);
                    print_whole_token(thisAgent, t, wtt);
                    thisAgent->outputManager->printa_sf(thisAgent, "\n");
                }
                deallocate_token_list(thisAgent, parent_tokens);
                thisAgent->outputManager->print_spaces(thisAgent, indent);
                thisAgent->outputManager->printa_sf(thisAgent, "*** Matches for Right ***\n");
                thisAgent->outputManager->print_spaces(thisAgent, indent);
                for (rm = node->b.posneg.alpha_mem_->right_mems; rm != NIL;
                        rm = rm->next_in_am)
                {
                    if (wtt == TIMETAG_WME_TRACE)
                    {
                        thisAgent->outputManager->printa_sf(thisAgent, "%u", rm->w->timetag);
                    }
                    else if (wtt == FULL_WME_TRACE)
                    {
                        print_wme(thisAgent, rm->w);
                    }
                    thisAgent->outputManager->printa_sf(thisAgent, " ");
                }
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
        } /* end of if (matches_one_level_up ...) */
    }

    /* --- return result --- */
    return matches_at_this_level;
}

void print_partial_match_information(agent* thisAgent, rete_node* p_node,
                                     wme_trace_type wtt)
{
    condition* top_cond, *bottom_cond;
    int64_t n;
    token* tokens, *t;
    p_node_to_conditions_and_rhs(thisAgent, p_node, NIL, NIL, &top_cond, &bottom_cond,
                                 NIL);
    n = ppmi_aux(thisAgent, p_node->parent, thisAgent->dummy_top_node, bottom_cond,
                 wtt, 0);
    thisAgent->outputManager->printa_sf(thisAgent, "\n%d complete matches.\n", n);
    if (n && (wtt != NONE_WME_TRACE))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "*** Complete Matches ***\n");
        tokens = get_all_left_tokens_emerging_from_node(thisAgent, p_node->parent);
        for (t = tokens; t != NIL; t = t->next_of_node)
        {
            print_whole_token(thisAgent, t, wtt);
            thisAgent->outputManager->printa_sf(thisAgent, "\n");
        }
        deallocate_token_list(thisAgent, tokens);
    }
    deallocate_condition_list(thisAgent, top_cond);
}

/* ----------------------------------------------------------------------

   Used by the "ms" command -- prints out the current match set.
---------------------------------------------------------------------- */

typedef struct match_set_trace
{
    Symbol* sym;
    int        count;
    struct match_set_trace* next;
    Symbol* goal;
} MS_trace;

MS_trace* in_ms_trace(Symbol* sym, MS_trace* trace)
{
    MS_trace* tmp;
    for (tmp = trace; tmp; tmp = tmp->next)
    {
        if (tmp->sym == sym)
        {
            return tmp;
        }
    }
    return 0;
}

MS_trace* in_ms_trace_same_goal(Symbol* sym, MS_trace* trace, Symbol* goal)
{
    MS_trace* tmp;
    for (tmp = trace; tmp; tmp = tmp->next)
    {
        if ((tmp->sym == sym) && (goal == tmp->goal))
        {
            return tmp;
        }
    }
    return 0;
}

void print_match_set(agent* thisAgent, wme_trace_type wtt, ms_trace_type mst)
{
    ms_change* msc;
    token temp_token;
    MS_trace* ms_trace = NIL, *tmp;

    /* --- Print assertions --- */


    if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "O Assertions:\n");
        for (msc = thisAgent->ms_o_assertions; msc != NIL; msc = msc->next)
        {

            if (wtt != NONE_WME_TRACE)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "  %y ", msc->p_node->b.p.prod->name);
                /* Add match goal to the print of the matching production */
                thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", msc->goal);
                temp_token.parent = msc->tok;
                temp_token.w = msc->w;
                print_whole_token(thisAgent, &temp_token, wtt);
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
            else
            {
                if ((tmp = in_ms_trace_same_goal(msc->p_node->b.p.prod->name,
                                                 ms_trace, msc->goal)) != NIL)
                {
                    tmp->count++;
                }
                else
                {
                    tmp = static_cast<match_set_trace*>(thisAgent->memoryManager->allocate_memory(sizeof(MS_trace), MISCELLANEOUS_MEM_USAGE));
                    tmp->sym = msc->p_node->b.p.prod->name;
                    tmp->count = 1;
                    tmp->next = ms_trace;
                    /* Add match goal to the print of the matching production */
                    tmp->goal = msc->goal;
                    ms_trace = tmp;
                }
            }
        }

        if (wtt == NONE_WME_TRACE)
        {
            while (ms_trace)
            {
                tmp = ms_trace;
                ms_trace = tmp->next;
                thisAgent->outputManager->printa_sf(thisAgent, "  %y ", tmp->sym);
                /*  BUG: for now this will print the goal of the first
                assertion inspected, even though there can be multiple
                assertions at different levels.
                See 2.110 in the OPERAND-CHANGE-LOG. */
                thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", tmp->goal);
                if (tmp->count > 1)
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "(%d)\n", tmp->count);
                }
                else
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "\n");
                }
                thisAgent->memoryManager->free_memory(tmp, MISCELLANEOUS_MEM_USAGE);
            }
        }
    }

    if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "I Assertions:\n");
        for (msc = thisAgent->ms_i_assertions; msc != NIL; msc = msc->next)
        {

            if (wtt != NONE_WME_TRACE)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "  %y ", msc->p_node->b.p.prod->name);
                /* Add match goal to the print of the matching production */
                thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", msc->goal);
                temp_token.parent = msc->tok;
                temp_token.w = msc->w;
                print_whole_token(thisAgent, &temp_token, wtt);
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
            else
            {
                if ((tmp = in_ms_trace_same_goal(msc->p_node->b.p.prod->name,
                                                 ms_trace, msc->goal)) != NIL)
                {
                    tmp->count++;
                }
                else
                {
                    tmp = static_cast<match_set_trace*>(thisAgent->memoryManager->allocate_memory(sizeof(MS_trace),
                                                        MISCELLANEOUS_MEM_USAGE));
                    tmp->sym = msc->p_node->b.p.prod->name;
                    tmp->count = 1;
                    tmp->next = ms_trace;
                    /* Add match goal to the print of the matching production */
                    tmp->goal = msc->goal;
                    ms_trace = tmp;
                }
            }
        }

        if (wtt == NONE_WME_TRACE)
        {
            while (ms_trace)
            {
                tmp = ms_trace;
                ms_trace = tmp->next;
                thisAgent->outputManager->printa_sf(thisAgent, "  %y ", tmp->sym);
                /*  BUG: for now this will print the goal of the first
                assertion inspected, even though there can be multiple
                assertions at different levels.
                See 2.110 in the OPERAND-CHANGE-LOG. */
                thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", tmp->goal);
                if (tmp->count > 1)
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "(%d)\n", tmp->count);
                }
                else
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "\n");
                }
                thisAgent->memoryManager->free_memory(tmp, MISCELLANEOUS_MEM_USAGE);
            }
        }
    }

    /* --- Print retractions --- */
    if (mst == MS_ASSERT_RETRACT || mst == MS_RETRACT)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "Retractions:\n");
        for (msc = thisAgent->ms_retractions; msc != NIL; msc = msc->next)
        {
            if (wtt != NONE_WME_TRACE)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "  ");
                print_instantiation_with_wmes(thisAgent, msc->inst, wtt, -1);
                thisAgent->outputManager->printa_sf(thisAgent, "\n");
            }
            else
            {
                if (msc->inst->prod)
                {
                    if ((tmp = in_ms_trace_same_goal(msc->inst->prod_name,
                                                     ms_trace, msc->goal)) != NIL)
                    {
                        tmp->count++;
                    }
                    else
                    {
                        tmp = static_cast<match_set_trace*>(thisAgent->memoryManager->allocate_memory(sizeof(MS_trace),
                                                            MISCELLANEOUS_MEM_USAGE));
                        tmp->sym = msc->inst->prod_name;
                        tmp->count = 1;
                        tmp->next = ms_trace;
                        /* Add match goal to the print of the matching production */
                        tmp->goal = msc->goal;
                        ms_trace = tmp;
                    }
                }
            }
        }
        if (wtt == NONE_WME_TRACE)
        {
            while (ms_trace)
            {
                tmp = ms_trace;
                ms_trace = tmp->next;
                thisAgent->outputManager->printa_sf(thisAgent, "  %y ", tmp->sym);
                /*  BUG: for now this will print the goal of the first assertion
                inspected, even though there can be multiple assertions at

                different levels.
                See 2.110 in the OPERAND-CHANGE-LOG. */
                if (tmp->goal)
                {
                    thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", tmp->goal);
                }
                else
                {
                    thisAgent->outputManager->printa_sf(thisAgent, " [NIL] ");
                }
                if (tmp->count > 1)
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "(%d)\n", tmp->count);
                }
                else
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "\n");
                }
                thisAgent->memoryManager->free_memory(tmp, MISCELLANEOUS_MEM_USAGE);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////
//
// XML Generation functions.
//
// These are currently local to rete while I'm working on matches.
// They should eventually move to their own file with a new header.
//
/////////////////////////////////////////////////////////////////

void xml_whole_token(agent* thisAgent, token* t, wme_trace_type wtt)
{
    if (t == thisAgent->dummy_top_token)
    {
        return;
    }
    xml_whole_token(thisAgent, t->parent, wtt);
    if (t->w)
    {
        if (wtt == TIMETAG_WME_TRACE)
        {
            xml_att_val(thisAgent, kWME_TimeTag, t->w->timetag);
        }
        else if (wtt == FULL_WME_TRACE)
        {
            xml_object(thisAgent, t->w);
        }
        //if (wtt!=NONE_WME_TRACE) print (thisAgent, " ");
    }
}

bool xml_pick_conds_with_matching_id_test(dl_cons* dc, agent* thisAgent)
{
    condition* cond;
    cond = static_cast<condition_struct*>(dc->item);
    if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
    {
        return false;
    }
    return tests_are_equal(thisAgent->id_test_to_match, cond->data.tests.id_test, false);
}


#define XML_CONDITION_LIST_TEMP_SIZE 10000
void xml_condition_list(agent* thisAgent, condition* conds,
                        int indent, bool internal)
{
    dl_list* conds_not_yet_printed, *tail_of_conds_not_yet_printed;
    dl_list* conds_for_this_id;
    dl_cons* dc;
    condition* c;
    bool removed_goal_test, removed_impasse_test;
    test id_test;
    std::string id_test_str;

    if (!conds)
    {
        return;
    }

    /* --- build dl_list of all the actions --- */
    conds_not_yet_printed = NIL;
    tail_of_conds_not_yet_printed = NIL;

    for (c = conds; c != NIL; c = c->next)
    {
        thisAgent->memoryManager->allocate_with_pool(MP_dl_cons, &dc);
        dc->item = c;
        if (conds_not_yet_printed)
        {
            tail_of_conds_not_yet_printed->next = dc;
        }
        else
        {
            conds_not_yet_printed = dc;
        }
        dc->prev = tail_of_conds_not_yet_printed;
        tail_of_conds_not_yet_printed = dc;
    }
    tail_of_conds_not_yet_printed->next = NIL;

    /* --- main loop: find all conds for first id, print them together --- */
    bool did_one_line_already = false;
    while (conds_not_yet_printed)
    {
        if (did_one_line_already)
        {
            //print (thisAgent, "\n");
            //print_spaces (thisAgent, indent);
        }
        else
        {
            did_one_line_already = true;
        }

        dc = conds_not_yet_printed;
        remove_from_dll(conds_not_yet_printed, dc, next, prev);
        c = static_cast<condition_struct*>(dc->item);
        if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);
            //print_string (thisAgent, "-{");
            xml_begin_tag(thisAgent, kTagConjunctive_Negation_Condition);
            xml_condition_list(thisAgent, c->data.ncc.top, indent + 2, internal);
            xml_end_tag(thisAgent, kTagConjunctive_Negation_Condition);
            //print_string (thisAgent, "}");
            continue;
        }

        /* --- normal pos/neg conditions --- */
        removed_goal_test = removed_impasse_test = false;
        id_test = copy_test(thisAgent, c->data.tests.id_test, false, false, true, &removed_goal_test, &removed_impasse_test);
        thisAgent->id_test_to_match = copy_test(thisAgent, id_test->eq_test);

        /* --- collect all cond's whose id test matches this one --- */
        conds_for_this_id = dc;
        dc->prev = NIL;
        if (internal)
        {
            dc->next = NIL;
        }
        else
        {
            dc->next = extract_dl_list_elements(thisAgent, &conds_not_yet_printed,
                                                xml_pick_conds_with_matching_id_test);
        }

        // DJP: Moved this loop out so we get a condition tag per condition on this id
        // rather than an id with a series of conditions.
        while (conds_for_this_id)
        {
            /* --- print the collected cond's all together --- */
            //print_string (thisAgent, " (");
            xml_begin_tag(thisAgent, kTagCondition);

            if (removed_goal_test)
            {
                //print_string (thisAgent, "state ");
                xml_att_val(thisAgent, kConditionTest, kConditionTestState);

            }

            if (removed_impasse_test)
            {
                //print_string (thisAgent, "impasse ");
                xml_att_val(thisAgent, kConditionTest, kConditionTestImpasse);
            }
            id_test_str.clear();
            thisAgent->outputManager->sprinta_sf(thisAgent, id_test_str, "%t", id_test);
            xml_att_val(thisAgent, kConditionId, id_test_str.c_str());
            deallocate_test(thisAgent, thisAgent->id_test_to_match);
            deallocate_test(thisAgent, id_test);

            //growable_string gs = make_blank_growable_string(thisAgent);
            dc = conds_for_this_id;
            conds_for_this_id = conds_for_this_id->next;
            c = static_cast<condition_struct*>(dc->item);
            thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);

            {
                /* --- build and print attr/value test for condition c --- */
                char temp[XML_CONDITION_LIST_TEMP_SIZE], *ch;

                memset(temp, 0, XML_CONDITION_LIST_TEMP_SIZE);
                ch = temp;
                //strncpy (ch, " ", XML_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                if (c->type == NEGATIVE_CONDITION)
                {
                    strncat(ch, "-", XML_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                }

                //strncat (ch, "^", XML_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                while (*ch)
                {
                    ch++;
                }
                Output_Manager::Get_OM().sprinta_sf_cstr(thisAgent, ch, XML_CONDITION_LIST_TEMP_SIZE - (ch - temp), "%t", c->data.tests.attr_test);
                while (*ch)
                {
                    ch++;
                }

                *ch = 0 ; // Terminate
                xml_att_val(thisAgent, kAttribute, temp) ;

                // Reset the ch pointer
                ch = temp ;
                if (c->data.tests.value_test)
                {
                    *(ch++) = ' ';
                    Output_Manager::Get_OM().sprinta_sf_cstr(thisAgent, ch, XML_CONDITION_LIST_TEMP_SIZE - (ch - temp), "%t", c->data.tests.value_test);
                    while (*ch)
                    {
                        ch++;
                    }
                    if (c->test_for_acceptable_preference)
                    {
                        strncpy(ch, " +", XML_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                        while (*ch)
                        {
                            ch++;
                        }
                    }
                }
                *ch = 0;
                if (thisAgent->outputManager->get_printer_output_column(thisAgent) + (ch - temp) >= COLUMNS_PER_LINE)
                {
                    //print_string (thisAgent, "\n");
                    //print_spaces (thisAgent, indent+6);
                }
                //print_string (thisAgent, temp);
                //add_to_growable_string(thisAgent, &gs, temp);
                xml_att_val(thisAgent, kValue, temp);
            }
            //free_growable_string(thisAgent, gs);
        }
        //print_string (thisAgent, ")");
        xml_end_tag(thisAgent, kTagCondition);
    } /* end of while (conds_not_yet_printed) */
}

void xml_condition(agent* thisAgent, condition* cond)
{
    condition* old_next, *old_prev;

    old_next = cond->next;
    old_prev = cond->prev;
    cond->next = NIL;
    cond->prev = NIL;
    xml_condition_list(thisAgent, cond, 0, true);
    cond->next = old_next;
    cond->prev = old_prev;
}

void xml_instantiation_with_wmes(agent* thisAgent, instantiation* inst,
                                 wme_trace_type wtt, int action)
{
    int PRINTING = -1;
    int FIRING = 0;
    int RETRACTING = 1;
    condition* cond;


    if (action == PRINTING)
    {
        xml_begin_tag(thisAgent, kTagProduction);
    }
    else if (action == FIRING)
    {
        xml_begin_tag(thisAgent, kTagProduction_Firing);
        xml_begin_tag(thisAgent, kTagProduction);
    }
    else if (action == RETRACTING)
    {
        xml_begin_tag(thisAgent, kTagProduction_Retracting);
        xml_begin_tag(thisAgent, kTagProduction);
    }

    if (inst->prod)
    {
        //print_with_symbols  (thisAgent, "%y", inst->prod_name);
        xml_att_val(thisAgent, kProduction_Name, inst->prod_name);
    }
    else
    {
        //print (thisAgent, "[dummy production]");
        xml_att_val(thisAgent, kProduction_Name, "[dummy_production]");

    }

    //print (thisAgent, "\n");

    if (wtt == NONE_WME_TRACE)
    {
        if (action == PRINTING)
        {
            xml_end_tag(thisAgent, kTagProduction);
        }
        else if (action == FIRING)
        {
            xml_end_tag(thisAgent, kTagProduction);
            xml_end_tag(thisAgent, kTagProduction_Firing);
        }
        else if (action == RETRACTING)
        {
            xml_end_tag(thisAgent, kTagProduction);
            xml_end_tag(thisAgent, kTagProduction_Retracting);
        }
        return;
    }

    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION)
        {
            switch (wtt)
            {
                case TIMETAG_WME_TRACE:
                    //print (thisAgent, " %u", cond->bt.wme_->timetag);

                    xml_begin_tag(thisAgent, kTagWME);
                    xml_att_val(thisAgent, kWME_TimeTag, cond->bt.wme_->timetag);
                    xml_end_tag(thisAgent, kTagWME);

                    break;
                case FULL_WME_TRACE:
                    if (action != RETRACTING)
                    {
                        //print (thisAgent, " ");
                        xml_object(thisAgent, cond->bt.wme_);
                    }
                    else
                    {
                        // Not all conds available when retracting, depending on DO_TOP_LEVEL_REF_CTS
#ifdef DO_TOP_LEVEL_REF_CTS
                        //print (thisAgent, " ");
                        xml_object(thisAgent, cond->bt.wme_);
#else

                        // Wmes that matched the LHS of a retraction may already be free'd; just print tt.
                        //print (thisAgent, " %u", cond->bt.wme_->timetag);

                        xml_begin_tag(thisAgent, kTagWME);
                        xml_att_val(thisAgent, kWME_TimeTag, cond->bt.wme_->timetag);
                        xml_end_tag(thisAgent, kTagWME);

#endif
                    }
                    break;
            }
        }

    if (action == PRINTING)
    {
        xml_end_tag(thisAgent, kTagProduction);
    }
    else if (action == FIRING)
    {
        xml_end_tag(thisAgent, kTagProduction);
        xml_end_tag(thisAgent, kTagProduction_Firing);
    }
    else if (action == RETRACTING)
    {
        xml_end_tag(thisAgent, kTagProduction);
        xml_end_tag(thisAgent, kTagProduction_Retracting);
    }
}

/////////////////////////////////////////////////////////////////
//
// XML version of print_match_set().
//
// Based on the print logic but generates XML directly.
//
/////////////////////////////////////////////////////////////////
void xml_match_set(agent* thisAgent, wme_trace_type wtt, ms_trace_type mst)
{
    ms_change* msc;
    token temp_token;
    MS_trace* ms_trace = NIL, *tmp;

    /* --- Print assertions --- */

    if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT)
    {
        //print (thisAgent, "O Assertions:\n");
        xml_begin_tag(thisAgent, kOAssertions) ;

        for (msc = thisAgent->ms_o_assertions; msc != NIL; msc = msc->next)
        {

            if (wtt != NONE_WME_TRACE)
            {
                xml_begin_tag(thisAgent, kTagProduction) ;
                xml_att_val(thisAgent, kName, msc->p_node->b.p.prod->name) ;
                xml_att_val(thisAgent, kGoal, msc->goal) ;
                //print_with_symbols (thisAgent, "  %y ", msc->p_node->b.p.prod->name);
                /* Add match goal to the print of the matching production */
                //thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", msc->goal);

                temp_token.parent = msc->tok;
                temp_token.w = msc->w;
                xml_whole_token(thisAgent, &temp_token, wtt);
                //print (thisAgent, "\n");
                xml_end_tag(thisAgent, kTagProduction) ;
            }
            else
            {
                if ((tmp = in_ms_trace_same_goal(msc->p_node->b.p.prod->name,
                                                 ms_trace, msc->goal)) != NIL)
                {
                    tmp->count++;
                }
                else
                {
                    tmp = static_cast<match_set_trace*>(thisAgent->memoryManager->allocate_memory(sizeof(MS_trace), MISCELLANEOUS_MEM_USAGE));
                    tmp->sym = msc->p_node->b.p.prod->name;
                    tmp->count = 1;
                    tmp->next = ms_trace;
                    /* Add match goal to the print of the matching production */
                    tmp->goal = msc->goal;
                    ms_trace = tmp;
                }
            }
        }

        if (wtt == NONE_WME_TRACE)
        {
            while (ms_trace)
            {
                xml_begin_tag(thisAgent, kTagProduction) ;
                tmp = ms_trace;
                ms_trace = tmp->next;
                xml_att_val(thisAgent, kName, tmp->sym) ;
                xml_att_val(thisAgent, kGoal, tmp->goal) ;
                if (tmp->count > 1)
                {
                    xml_att_val(thisAgent, kCount, tmp->count) ;    // DJP -- No idea what this count is
                }
                //print_with_symbols (thisAgent, "  %y ", tmp->sym);
                /*  BUG: for now this will print the goal of the first
                assertion inspected, even though there can be multiple
                assertions at different levels.
                See 2.110 in the OPERAND-CHANGE-LOG. */
                //thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", tmp->goal);
                //if (tmp->count > 1)
                //  print(thisAgent, "(%d)\n", tmp->count);
                //else
                //  print(thisAgent, "\n");
                thisAgent->memoryManager->free_memory(tmp, MISCELLANEOUS_MEM_USAGE);
                xml_end_tag(thisAgent, kTagProduction) ;
            }
        }
        xml_end_tag(thisAgent, kOAssertions) ;
    }

    if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT)
    {
        //print (thisAgent, "I Assertions:\n");
        xml_begin_tag(thisAgent, kIAssertions) ;
        for (msc = thisAgent->ms_i_assertions; msc != NIL; msc = msc->next)
        {

            if (wtt != NONE_WME_TRACE)
            {
                //print_with_symbols (thisAgent, "  %y ", msc->p_node->b.p.prod->name);
                /* Add match goal to the print of the matching production */
                //thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", msc->goal);
                xml_begin_tag(thisAgent, kTagProduction) ;
                xml_att_val(thisAgent, kName, msc->p_node->b.p.prod->name) ;
                xml_att_val(thisAgent, kGoal, msc->goal) ;

                temp_token.parent = msc->tok;
                temp_token.w = msc->w;
                xml_whole_token(thisAgent, &temp_token, wtt);
                //print (thisAgent, "\n");
                xml_end_tag(thisAgent, kTagProduction) ;
            }
            else
            {
                if ((tmp = in_ms_trace_same_goal(msc->p_node->b.p.prod->name,
                                                 ms_trace, msc->goal)) != NIL)
                {
                    tmp->count++;
                }
                else
                {
                    tmp = static_cast<match_set_trace*>(thisAgent->memoryManager->allocate_memory(sizeof(MS_trace),
                                                        MISCELLANEOUS_MEM_USAGE));
                    tmp->sym = msc->p_node->b.p.prod->name;
                    tmp->count = 1;
                    tmp->next = ms_trace;
                    /* Add match goal to the print of the matching production */
                    tmp->goal = msc->goal;
                    ms_trace = tmp;
                }
            }
        }

        if (wtt == NONE_WME_TRACE)
        {
            while (ms_trace)
            {
                tmp = ms_trace;
                ms_trace = tmp->next;
                xml_begin_tag(thisAgent, kTagProduction) ;
                xml_att_val(thisAgent, kName, tmp->sym) ;
                xml_att_val(thisAgent, kGoal, tmp->goal) ;
                if (tmp->count > 1)
                {
                    xml_att_val(thisAgent, kCount, tmp->count) ;    // DJP -- No idea what this count is
                }
                //print_with_symbols (thisAgent, "  %y ", tmp->sym);
                /*  BUG: for now this will print the goal of the first
                assertion inspected, even though there can be multiple
                assertions at different levels.
                See 2.110 in the OPERAND-CHANGE-LOG. */
                //thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", tmp->goal);
                //if (tmp->count > 1)
                //  print(thisAgent, "(%d)\n", tmp->count);
                //else
                //  print(thisAgent, "\n");

                thisAgent->memoryManager->free_memory(tmp, MISCELLANEOUS_MEM_USAGE);
                xml_end_tag(thisAgent, kTagProduction) ;
            }
        }
    }
    xml_end_tag(thisAgent, kIAssertions) ;

    if (mst == MS_ASSERT_RETRACT || mst == MS_RETRACT)
    {
        xml_begin_tag(thisAgent, kRetractions) ;
        //print (thisAgent, "Retractions:\n");
        for (msc = thisAgent->ms_retractions; msc != NIL; msc = msc->next)
        {
            if (wtt != NONE_WME_TRACE)
            {
                //print (thisAgent, "  ");
                xml_instantiation_with_wmes(thisAgent, msc->inst, wtt, -1);
                //print (thisAgent, "\n");
            }
            else
            {
                if (msc->inst->prod)
                {
                    if ((tmp = in_ms_trace_same_goal(msc->inst->prod_name,
                                                     ms_trace, msc->goal)) != NIL)
                    {
                        tmp->count++;
                    }
                    else
                    {
                        tmp = static_cast<match_set_trace*>(thisAgent->memoryManager->allocate_memory(sizeof(MS_trace),
                                                            MISCELLANEOUS_MEM_USAGE));
                        tmp->sym = msc->inst->prod_name;
                        tmp->count = 1;
                        tmp->next = ms_trace;
                        /* Add match goal to the print of the matching production */
                        tmp->goal = msc->goal;
                        ms_trace = tmp;
                    }
                }
            }
        }
        if (wtt == NONE_WME_TRACE)
        {
            while (ms_trace)
            {
                tmp = ms_trace;
                ms_trace = tmp->next;
                xml_begin_tag(thisAgent, kTagProduction) ;
                xml_att_val(thisAgent, kName, tmp->sym) ;
                if (tmp->goal)
                {
                    xml_att_val(thisAgent, kGoal, tmp->goal) ;
                }
                else
                {
                    xml_att_val(thisAgent, kGoal, "NIL") ;
                }
                if (tmp->count > 1)
                {
                    xml_att_val(thisAgent, kCount, tmp->count) ;    // DJP -- No idea what this count is
                }
                //print_with_symbols (thisAgent, "  %y ", tmp->sym);
                /*  BUG: for now this will print the goal of the first assertion
                inspected, even though there can be multiple assertions at

                different levels.
                See 2.110 in the OPERAND-CHANGE-LOG. */
                //if (tmp->goal)
                //  thisAgent->outputManager->printa_sf(thisAgent, " [%y] ", tmp->goal);
                //else
                //  print(thisAgent, " [NIL] ");
                //if(tmp->count > 1)
                //  print(thisAgent, "(%d)\n", tmp->count);
                //else
                //  print(thisAgent, "\n");
                thisAgent->memoryManager->free_memory(tmp, MISCELLANEOUS_MEM_USAGE);
                xml_end_tag(thisAgent, kTagProduction) ;
            }
        }
    }
}

/* --- Print stuff for given node and higher, up to but not including the
       cutoff node.  Return number of matches at the given node/cond. --- */
int64_t xml_aux(agent* thisAgent,    /* current agent */
                rete_node* node,    /* current node */
                rete_node* cutoff,  /* don't print cutoff node or any higher */
                condition* cond,    /* cond for current node */
                wme_trace_type wtt, /* what type of printout to use */
                int indent)         /* number of spaces indent */
{
    token* tokens, *t, *parent_tokens;
    right_mem* rm;
    int64_t matches_one_level_up;
    int64_t matches_at_this_level;
    //#define MATCH_COUNT_STRING_BUFFER_SIZE 20
    //char match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE];
    rete_node* parent;

    /* --- find the number of matches for this condition --- */
    tokens = get_all_left_tokens_emerging_from_node(thisAgent, node);
    matches_at_this_level = 0;
    for (t = tokens; t != NIL; t = t->next_of_node)
    {
        matches_at_this_level++;
    }
    deallocate_token_list(thisAgent, tokens);

    /* --- if we're at the cutoff node, we're done --- */
    if (node == cutoff)
    {
        return matches_at_this_level;
    }

    /* --- do stuff higher up --- */
    parent = real_parent_node(node);
    matches_one_level_up = xml_aux(thisAgent, parent, cutoff,
                                   cond->prev, wtt, indent);

    /* --- Form string for current match count:  If an earlier cond had no
       matches, just leave it blank; if this is the first 0, use ">>>>" --- */
    if (! matches_one_level_up)
    {
        //xml_att_val(thisAgent, kMatchCount, 0) ;
        //strncpy (match_count_string, "    ", MATCH_COUNT_STRING_BUFFER_SIZE);
        //match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
    }
    else if (! matches_at_this_level)
    {
        //xml_att_val(thisAgent, kMatchCount, 0) ;
        //strncpy (match_count_string, ">>>>", MATCH_COUNT_STRING_BUFFER_SIZE);
        //match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
    }
    else
    {
        //xml_att_val(thisAgent, kMatchCount, matches_at_this_level) ;
        //SNPRINTF (match_count_string, MATCH_COUNT_STRING_BUFFER_SIZE, "%4ld", matches_at_this_level);
        //match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
    }

    /* --- print extra indentation spaces --- */
    //print_spaces (thisAgent, indent);

    if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
    {
        /* --- recursively print match counts for the NCC subconditions --- */
        xml_begin_tag(thisAgent, kTagConjunctive_Negation_Condition) ;
        //print (thisAgent, "    -{\n");
        xml_aux(thisAgent, real_parent_node(node->b.cn.partner),
                parent,
                cond->data.ncc.bottom,
                wtt,
                indent + 5);
        //print_spaces (thisAgent, indent);
        //print (thisAgent, "%s }\n", match_count_string);
        xml_end_tag(thisAgent, kTagConjunctive_Negation_Condition) ;
    }
    else
    {
        //print (thisAgent, "%s", match_count_string);
        xml_condition(thisAgent, cond);

        // DJP: This is a trick to let us insert more attributes into xml_condition().
        xml_move_current_to_last_child(thisAgent) ;
        // DJP: Moved this test from earlier down to here as no longer building match_count_string
        if (!matches_one_level_up)
        {
            xml_att_val(thisAgent, kMatchCount, 0) ;
        }
        else
        {
            xml_att_val(thisAgent, kMatchCount, matches_at_this_level) ;
        }
        xml_move_current_to_parent(thisAgent) ;

        //print (thisAgent, "\n");
        /* --- if this is the first match-failure (0 matches), print info on
           matches for left and right --- */
        if (matches_one_level_up && (!matches_at_this_level))
        {
            if (wtt != NONE_WME_TRACE)
            {
                //print_spaces (thisAgent, indent);
                xml_begin_tag(thisAgent, kTagLeftMatches) ;
                //print (thisAgent, "*** Matches For Left ***\n");
                parent_tokens = get_all_left_tokens_emerging_from_node(thisAgent, parent);
                for (t = parent_tokens; t != NIL; t = t->next_of_node)
                {
                    //print_spaces (thisAgent, indent);
                    xml_begin_tag(thisAgent, kTagToken) ;
                    xml_whole_token(thisAgent, t, wtt);
                    xml_end_tag(thisAgent, kTagToken) ;
                    //print (thisAgent, "\n");
                }
                deallocate_token_list(thisAgent, parent_tokens);
                xml_end_tag(thisAgent, kTagLeftMatches) ;
                //print_spaces (thisAgent, indent);
                //print (thisAgent, "*** Matches for Right ***\n");
                xml_begin_tag(thisAgent, kTagRightMatches) ;
                //print_spaces (thisAgent, indent);
                for (rm = node->b.posneg.alpha_mem_->right_mems; rm != NIL;
                        rm = rm->next_in_am)
                {
                    //if (wtt==TIMETAG_WME_TRACE) print (thisAgent, "%lu", rm->w->timetag);
                    //else if (wtt==FULL_WME_TRACE) print_wme (thisAgent, rm->w);
                    //print (thisAgent, " ");
                    if (wtt == TIMETAG_WME_TRACE)
                    {
                        xml_att_val(thisAgent, kWME_TimeTag, rm->w->timetag);
                    }
                    else if (wtt == FULL_WME_TRACE)
                    {
                        xml_object(thisAgent, rm->w);
                    }
                }
                xml_end_tag(thisAgent, kTagRightMatches) ;
                //print (thisAgent, "\n");
            }
        } /* end of if (matches_one_level_up ...) */
    }

    /* --- return result --- */
    return matches_at_this_level;
}

void xml_partial_match_information(agent* thisAgent, rete_node* p_node, wme_trace_type wtt)
{
    condition* top_cond, *bottom_cond;
    int64_t n;
    token* tokens, *t;

    xml_begin_tag(thisAgent, kTagProduction) ;
    p_node_to_conditions_and_rhs(thisAgent, p_node, NIL, NIL, &top_cond, &bottom_cond,
                                 NIL);
    n = xml_aux(thisAgent, p_node->parent, thisAgent->dummy_top_node, bottom_cond,
                wtt, 0);
    xml_att_val(thisAgent, kMatches, n) ;
    //print (thisAgent, "\n%d complete matches.\n", n);
    if (n && (wtt != NONE_WME_TRACE))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "*** Complete Matches ***\n");
        tokens = get_all_left_tokens_emerging_from_node(thisAgent, p_node->parent);
        for (t = tokens; t != NIL; t = t->next_of_node)
        {
            xml_whole_token(thisAgent, t, wtt);
            //print (thisAgent, "\n");
        }
        deallocate_token_list(thisAgent, tokens);
    }
    deallocate_condition_list(thisAgent, top_cond);
    xml_end_tag(thisAgent, kTagProduction) ;
}


/* **********************************************************************

   SECTION 19:  Rete Initialization

   EXTERNAL INTERFACE:
   Init_rete() initializes everything.
********************************************************************** */
void init_left_and_right_addition_routines()
{
    static bool is_initialized = false;
    if (!is_initialized)
    {
        left_addition_routines[DUMMY_MATCHES_BNODE]      = dummy_matches_node_left_addition;
        left_addition_routines[MEMORY_BNODE]             = beta_memory_node_left_addition;
        left_addition_routines[UNHASHED_MEMORY_BNODE]    = unhashed_beta_memory_node_left_addition;
        left_addition_routines[MP_BNODE]                 = mp_node_left_addition;
        left_addition_routines[UNHASHED_MP_BNODE]        = unhashed_mp_node_left_addition;
        left_addition_routines[CN_BNODE]                 = cn_node_left_addition;
        left_addition_routines[CN_PARTNER_BNODE]         = cn_partner_node_left_addition;
        left_addition_routines[P_BNODE]                  = p_node_left_addition;
        left_addition_routines[NEGATIVE_BNODE]           = negative_node_left_addition;
        left_addition_routines[UNHASHED_NEGATIVE_BNODE]  = unhashed_negative_node_left_addition;

        right_addition_routines[POSITIVE_BNODE]          = positive_node_right_addition;
        right_addition_routines[UNHASHED_POSITIVE_BNODE] = unhashed_positive_node_right_addition;
        right_addition_routines[MP_BNODE]                = mp_node_right_addition;
        right_addition_routines[UNHASHED_MP_BNODE]       = unhashed_mp_node_right_addition;
        right_addition_routines[NEGATIVE_BNODE]          = negative_node_right_addition;
        right_addition_routines[UNHASHED_NEGATIVE_BNODE] = unhashed_negative_node_right_addition;

        is_initialized = true;
    }
}


void init_rete(agent* thisAgent)
{

    /*
           This function consists of two parts. The first initializes variables
           pertaining to a particular agent. The second initializes some important
           globals (bnode type names, addition routines, and test routines).
           Originally, these two parts were ordered the other way.

           The globals should only be initialized once (when the rete for the first
           agent is initialized), whereas everything else should be initialized on
           every call to the function (i.e. whenever the rete for a new agent is
           initialized).

           Therefore, the order has been switched so that the agent-specific
           variables are initialized first. Once this is done, a simple test of a
           static boolean variable indicates whether or not the globals have already
           been initialized. If they have, then the function exits prematurely.

           As far as I can see, this switch has no undesired effects, since the
           agent-specific function calls in the first part do not depend upon the
           global variables defined in the second part.

           -AJC (8/9/02)
    */

    int i;

    thisAgent->memoryManager->init_memory_pool(MP_alpha_mem, sizeof(alpha_mem),
                     "alpha mem");
    thisAgent->memoryManager->init_memory_pool(MP_rete_test, sizeof(rete_test),
                     "rete test");
    thisAgent->memoryManager->init_memory_pool(MP_rete_node, sizeof(rete_node),
                     "rete node");
    thisAgent->memoryManager->init_memory_pool(MP_node_varnames, sizeof(node_varnames),
                     "node varnames");
    thisAgent->memoryManager->init_memory_pool(MP_token, sizeof(token), "token");
    thisAgent->memoryManager->init_memory_pool(MP_right_mem, sizeof(right_mem),
                     "right mem");
    thisAgent->memoryManager->init_memory_pool(MP_ms_change, sizeof(ms_change),
                     "ms change");

    for (i = 0; i < 16; i++)
    {
        thisAgent->alpha_hash_tables[i] = make_hash_table(thisAgent, 0, hash_alpha_mem);
    }

    thisAgent->left_ht = thisAgent->memoryManager->allocate_memory_and_zerofill(sizeof(char*) * LEFT_HT_SIZE, HASH_TABLE_MEM_USAGE);
    thisAgent->right_ht = thisAgent->memoryManager->allocate_memory_and_zerofill(sizeof(char*) * RIGHT_HT_SIZE, HASH_TABLE_MEM_USAGE);

    init_dummy_top_node(thisAgent);

    thisAgent->max_rhs_unbound_variables = 1;
    thisAgent->rhs_variable_bindings = (Symbol**)
                                       thisAgent->memoryManager->allocate_memory_and_zerofill(sizeof(Symbol*), MISCELLANEOUS_MEM_USAGE);

    /* This is still not thread-safe. -AJC (8/9/02) */
    static bool bInit = false;
    if (bInit)
    {
        return;
    }

    bInit = true;

    init_bnode_type_names(thisAgent);

    init_left_and_right_addition_routines();

    //
    // rete_test_routines is now statically initialized.
    //
    //for (i=0; i<256; i++) rete_test_routines[i] = error_rete_test_routine;
    rete_test_routines[DISJUNCTION_RETE_TEST] = disjunction_rete_test_routine;
    rete_test_routines[ID_IS_GOAL_RETE_TEST] = id_is_goal_rete_test_routine;
    rete_test_routines[ID_IS_IMPASSE_RETE_TEST] = id_is_impasse_rete_test_routine;
    rete_test_routines[UNARY_SMEM_LINK_RETE_TEST] = unary_smem_link_rete_test_routine;
    rete_test_routines[UNARY_SMEM_LINK_NOT_RETE_TEST] = unary_smem_link_not_rete_test_routine;
    rete_test_routines[CONSTANT_RELATIONAL_RETE_TEST +
                       RELATIONAL_EQUAL_RETE_TEST] =
                           constant_equal_rete_test_routine;
    rete_test_routines[CONSTANT_RELATIONAL_RETE_TEST +
                       RELATIONAL_NOT_EQUAL_RETE_TEST] =
                           constant_not_equal_rete_test_routine;
    rete_test_routines[CONSTANT_RELATIONAL_RETE_TEST +
                       RELATIONAL_LESS_RETE_TEST] =
                           constant_less_rete_test_routine;
    rete_test_routines[CONSTANT_RELATIONAL_RETE_TEST +
                       RELATIONAL_GREATER_RETE_TEST] =
                           constant_greater_rete_test_routine;
    rete_test_routines[CONSTANT_RELATIONAL_RETE_TEST +
                       RELATIONAL_LESS_OR_EQUAL_RETE_TEST] =
                           constant_less_or_equal_rete_test_routine;
    rete_test_routines[CONSTANT_RELATIONAL_RETE_TEST +
                       RELATIONAL_GREATER_OR_EQUAL_RETE_TEST] =
                           constant_greater_or_equal_rete_test_routine;
    rete_test_routines[CONSTANT_RELATIONAL_RETE_TEST +
                       RELATIONAL_SAME_TYPE_RETE_TEST] =
                           constant_same_type_rete_test_routine;
    rete_test_routines[CONSTANT_RELATIONAL_RETE_TEST +
                       RELATIONAL_SMEM_LINK_TEST] =
                           constant_smem_link_rete_test_routine;
    rete_test_routines[CONSTANT_RELATIONAL_RETE_TEST +
                       RELATIONAL_SMEM_LINK_NOT_TEST] =
                           constant_smem_link_not_rete_test_routine;
    rete_test_routines[VARIABLE_RELATIONAL_RETE_TEST +
                       RELATIONAL_EQUAL_RETE_TEST] =
                           variable_equal_rete_test_routine;
    rete_test_routines[VARIABLE_RELATIONAL_RETE_TEST +
                       RELATIONAL_NOT_EQUAL_RETE_TEST] =
                           variable_not_equal_rete_test_routine;
    rete_test_routines[VARIABLE_RELATIONAL_RETE_TEST +
                       RELATIONAL_LESS_RETE_TEST] =
                           variable_less_rete_test_routine;
    rete_test_routines[VARIABLE_RELATIONAL_RETE_TEST +
                       RELATIONAL_GREATER_RETE_TEST] =
                           variable_greater_rete_test_routine;
    rete_test_routines[VARIABLE_RELATIONAL_RETE_TEST +
                       RELATIONAL_LESS_OR_EQUAL_RETE_TEST] =
                           variable_less_or_equal_rete_test_routine;
    rete_test_routines[VARIABLE_RELATIONAL_RETE_TEST +
                       RELATIONAL_GREATER_OR_EQUAL_RETE_TEST] =
                           variable_greater_or_equal_rete_test_routine;
    rete_test_routines[VARIABLE_RELATIONAL_RETE_TEST +
                       RELATIONAL_SAME_TYPE_RETE_TEST] =
                           variable_same_type_rete_test_routine;
    rete_test_routines[VARIABLE_RELATIONAL_RETE_TEST +
                       RELATIONAL_SMEM_LINK_TEST] =
                           variable_smem_link_rete_test_routine;
    rete_test_routines[VARIABLE_RELATIONAL_RETE_TEST +
                           RELATIONAL_SMEM_LINK_NOT_TEST] =
                               variable_smem_link_not_rete_test_routine;
    }
