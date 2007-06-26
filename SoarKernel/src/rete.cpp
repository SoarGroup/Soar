#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

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
 * Any_assertions_or_retractions_ready() returns TRUE iff there are any
 * pending changes to the match set.  This is used to test for quiescence.
 * Get_next_assertion() retrieves a pending assertion (returning TRUE) or
 * returns FALSE is no more are available.  Get_next_retraction() is
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
 * files.  They return TRUE if successful, FALSE if any error occurred.
 *
 * =======================================================================
 */

/* ======================================================================

                      Rete Net Routines for Soar 6
   
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

#include "rete.h"
#include "kernel.h"
#include "mem.h"
#include "wmem.h"
#include "gdatastructs.h"
#include "explain.h"
#include "symtab.h"
#include "agent.h"
#include "print.h"
#include "production.h"
#include "init_soar.h"
#include "instantiations.h"
#include "rhsfun.h"
#include "lexer.h"
#include "xml.h"

/* JC ADDED: for gSKI events */
#include "gski_event_system_functions.h"

// So we can call directly to it to create XML output.
// If you need a kernel w/o any external dependencies, removing this include
// and methods starting with xml_ should do the trick.
#include "cli_CommandLineInterface.h"	

/* ----------- basic functionality switches ----------- */

/* Set to FALSE to preserve variable names in chunks (takes extra space) */
#define discard_chunk_varnames TRUE

/* ----------- debugging switches ----------- */

/* Uncomment the following line to get pnode printouts */
/* #define DEBUG_RETE_PNODES  */

/* REW: begin 08.20.97 */
/* For information on the Waterfall processing in rete.cpp */
/* #define DEBUG_WATERFALL */
/* REW: end   08.20.97 */

/* ----------- statistics switches ----------- */

/* Uncomment the following line to get statistics on token counts with and
   without sharing */
/* #define TOKEN_SHARING_STATS */

/* Uncomment the following line to gather statistics on null activations */
/* #define NULL_ACTIVATION_STATS */

/* Uncomment the following line to gather statistics on beta node sharing */
/* #define SHARING_FACTORS */

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







/* **********************************************************************

   SECTION 1:  Rete Net Structures and Declarations

********************************************************************** */

/* ----------------------------------------------------------------------

       Structures and Declarations:  Alpha Portion of the Rete Net

---------------------------------------------------------------------- */

/* --- dll of all wmes currently in the rete:  this is needed to
       initialize newly created alpha memories --- */
/* wme *all_wmes_in_rete; (moved to glob_vars.h) */

/* --- structure of each alpha memory --- */
typedef struct alpha_mem_struct {
  struct alpha_mem_struct *next_in_hash_table;  /* next mem in hash bucket */
  struct right_mem_struct *right_mems;  /* dll of right_mem structures */
  struct rete_node_struct *beta_nodes;  /* list of attached beta nodes */
  struct rete_node_struct *last_beta_node; /* tail of above dll */
  Symbol *id;                  /* constants tested by this alpha mem */
  Symbol *attr;                /* (NIL if this alpha mem ignores that field) */
  Symbol *value;
  Bool acceptable;             /* does it test for acceptable pref? */
  unsigned long am_id;            /* id for hashing */
  unsigned long reference_count;  /* number of beta nodes using this mem */
  unsigned long retesave_amindex;
} alpha_mem;

/* --- the entry for one WME in one alpha memory --- */
typedef struct right_mem_struct {
  wme *w;                      /* the wme */
  alpha_mem *am;               /* the alpha memory */
  struct right_mem_struct *next_in_bucket, *prev_in_bucket; /*hash bucket dll*/
  struct right_mem_struct *next_in_am, *prev_in_am;       /*rm's in this amem*/
  struct right_mem_struct *next_from_wme, *prev_from_wme; /*tree-based remove*/
} right_mem;

/* Note: right_mem's are stored in hash table thisAgent->right_ht */

/* ----------------------------------------------------------------------

       Structures and Declarations:  Beta Portion of the Rete Net

---------------------------------------------------------------------- */

/* --- types of tests found at beta nodes --- */
#define CONSTANT_RELATIONAL_RETE_TEST 0x00
#define VARIABLE_RELATIONAL_RETE_TEST 0x10
#define DISJUNCTION_RETE_TEST         0x20
#define ID_IS_GOAL_RETE_TEST          0x30
#define ID_IS_IMPASSE_RETE_TEST       0x31
//#define test_is_constant_relational_test(x) (((x) & 0xF0)==0x00)
//#define test_is_variable_relational_test(x) (((x) & 0xF0)==0x10)

inline Bool test_is_constant_relational_test(byte x)
{
  return (((x) & 0xF0)==0x00);
}

inline Bool test_is_variable_relational_test(byte x)
{
  return (((x) & 0xF0)==0x10);
}

/* --- for the last two (i.e., the relational tests), we add in one of
       the following, to specifiy the kind of relation --- */
#define RELATIONAL_EQUAL_RETE_TEST            0x00
#define RELATIONAL_NOT_EQUAL_RETE_TEST        0x01
#define RELATIONAL_LESS_RETE_TEST             0x02
#define RELATIONAL_GREATER_RETE_TEST          0x03
#define RELATIONAL_LESS_OR_EQUAL_RETE_TEST    0x04
#define RELATIONAL_GREATER_OR_EQUAL_RETE_TEST 0x05
#define RELATIONAL_SAME_TYPE_RETE_TEST        0x06
//#define kind_of_relational_test(x) ((x) & 0x0F)
//#define test_is_not_equal_test(x) (((x)==0x01) || ((x)==0x11))

inline byte kind_of_relational_test(byte x)
{
  return ((x) & 0x0F);
}

inline Bool test_is_not_equal_test(byte x)
{
  return (((x)==0x01) || ((x)==0x11));
}

/* --- tells where to find a variable --- */
typedef unsigned short rete_node_level;

typedef struct var_location_struct {
  rete_node_level levels_up; /* 0=current node's alphamem, 1=parent's, etc. */
  byte field_num;            /* 0=id, 1=attr, 2=value */
} var_location;

/* define an equality predicate for var_location structures */
/*#define var_locations_equal(v1,v2) \
  ( ((v1).levels_up==(v2).levels_up) && ((v1).field_num==(v2).field_num) )*/
inline Bool var_locations_equal(var_location v1, var_location v2)
{
  return ( ((v1).levels_up==(v2).levels_up) && ((v1).field_num==(v2).field_num) );
}

/* --- extract field (id/attr/value) from wme --- */
/* WARNING: this relies on the id/attr/value fields being consecutive in
   the wme structure (defined in soarkernel.h) */
/*#define field_from_wme(wme,field_num) \
  ( (&((wme)->id))[(field_num)] )*/
inline Symbol * field_from_wme(wme * _wme, byte field_num)
{
  return ( (&((_wme)->id))[(field_num)] );
}
                                    
/* --- gives data for a test that must be applied at a node --- */
typedef struct rete_test_struct {
  byte right_field_num;          /* field (0, 1, or 2) from wme */
  byte type;                     /* test type (ID_IS_GOAL_RETE_TEST, etc.) */
  union rete_test_data_union {
    var_location variable_referent;   /* for relational tests to a variable */
    Symbol *constant_referent;        /* for relational tests to a constant */
    list *disjunction_list;           /* list of symbols in disjunction test */
  } data;
  struct rete_test_struct *next; /* next in list of tests at the node */
} rete_test;

/* --- types and structure of beta nodes --- */  
/*   key:  bit 0 --> hashed                  */
/*         bit 1 --> memory                  */
/*         bit 2 --> positive join           */
/*         bit 3 --> negative join           */
/*         bit 4 --> split from beta memory  */
/*         bit 6 --> various special types   */

/* Warning: If you change any of these or add ones, be sure to update the
   bit-twiddling macros just below */
#define UNHASHED_MEMORY_BNODE   0x02
#define MEMORY_BNODE            0x03
#define UNHASHED_MP_BNODE       0x06
#define MP_BNODE                0x07
#define UNHASHED_POSITIVE_BNODE 0x14
#define POSITIVE_BNODE          0x15
#define UNHASHED_NEGATIVE_BNODE 0x08
#define NEGATIVE_BNODE          0x09
#define DUMMY_TOP_BNODE         0x40
#define DUMMY_MATCHES_BNODE     0x41
#define CN_BNODE                0x42
#define CN_PARTNER_BNODE        0x43
#define P_BNODE                 0x44

/*
#define bnode_is_hashed(x)   ((x) & 0x01)
#define bnode_is_memory(x)   ((x) & 0x02)
#define bnode_is_positive(x) ((x) & 0x04)
#define bnode_is_negative(x) ((x) & 0x08)
#define bnode_is_posneg(x)   ((x) & 0x0C)
#define bnode_is_bottom_of_split_mp(x) ((x) & 0x10)
#define real_parent_node(x) ( bnode_is_bottom_of_split_mp((x)->node_type) ? (x)->parent->parent : (x)->parent )
*/

inline byte bnode_is_hashed(byte x) { return ((x) & 0x01); }
inline byte bnode_is_memory(byte x) { return ((x) & 0x02); }
inline byte bnode_is_positive(byte x) { return ((x) & 0x04); }
inline byte bnode_is_negative(byte x) { return ((x) & 0x08); }
inline byte bnode_is_posneg(byte x) { return ((x) & 0x0C); }
inline byte bnode_is_bottom_of_split_mp(byte x) { return ((x) & 0x10); }

/* This function cannot be defined before struct rete_node_struct */
inline rete_node * real_parent_node(rete_node * x);

/*
 * Initialize the list with all empty strings.
 */
char *bnode_type_names[256] =
{
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","","",   
   "","","","","","","","","","","","","","","",""  
};

/* --- data for positive nodes only --- */
typedef struct pos_node_data_struct {
  /* --- dll of left-linked pos nodes from the parent beta memory --- */
  struct rete_node_struct *next_from_beta_mem, *prev_from_beta_mem;
} pos_node_data;

/* --- data for both positive and negative nodes --- */
typedef struct posneg_node_data_struct {
  rete_test *other_tests; /* tests other than the hashed test */
  alpha_mem *alpha_mem_;  /* the alpha memory this node uses */
  struct rete_node_struct *next_from_alpha_mem; /* dll of nodes using that */
  struct rete_node_struct *prev_from_alpha_mem; /*   ... alpha memory */
  struct rete_node_struct *nearest_ancestor_with_same_am;
} posneg_node_data;

/* --- data for beta memory nodes only --- */
typedef struct beta_memory_node_data_struct {
  /* --- first pos node child that is left-linked --- */
  struct rete_node_struct *first_linked_child;  
} beta_memory_node_data;

/* --- data for cn and cn_partner nodes only --- */
typedef struct cn_node_data_struct {
  struct rete_node_struct *partner;    /* cn, cn_partner point to each other */
} cn_node_data;

/* --- data for production nodes only --- */
typedef struct p_node_data_struct {
  struct production_struct *prod;                  /* the production */
  struct node_varnames_struct *parents_nvn;        /* records variable names */
  struct ms_change_struct *tentative_assertions;   /* pending MS changes */
  struct ms_change_struct *tentative_retractions;
} p_node_data;

#define O_LIST 0     /* moved here from soarkernel.h.  only used in rete.cpp */
#define I_LIST 1     /*   values for prod->OPERAND_which_assert_list */

/* --- data for all except positive nodes --- */
typedef struct non_pos_node_data_struct {
  struct token_struct *tokens;           /* dll of tokens at this node */
  unsigned is_left_unlinked:1;           /* used on mp nodes only */
} non_pos_node_data;

/* --- structure of a rete beta node --- */
typedef struct rete_node_struct {
  byte node_type;                  /* tells what kind of node this is */

  /* -- used only on hashed nodes -- */
  /* field_num: 0=id, 1=attr, 2=value */
  byte left_hash_loc_field_num;      
  /* left_hash_loc_levels_up: 0=current node's alphamem, 1=parent's, etc. */
  rete_node_level left_hash_loc_levels_up; 
  /* node_id: used for hash function */
  unsigned long node_id;                   

#ifdef SHARING_FACTORS
  unsigned long sharing_factor;
#endif

  struct rete_node_struct *parent;       /* points to parent node */
  struct rete_node_struct *first_child;  /* used for dll of all children, */
  struct rete_node_struct *next_sibling; /*   regardless of unlinking status */
  union rete_node_a_union {
    pos_node_data pos;                   /* for pos. nodes */
    non_pos_node_data np;                /* for all other nodes */
  } a;
  union rete_node_b_union {
    posneg_node_data posneg;            /* for pos, neg, mp nodes */
    beta_memory_node_data mem;          /* for beta memory nodes */
    cn_node_data cn;                    /* for cn, cn_partner nodes */
    p_node_data p;                      /* for p nodes */
  } b;
} rete_node;

/* Now this function can safely be defined. */
inline rete_node * real_parent_node(rete_node * x)
{
  return ( bnode_is_bottom_of_split_mp((x)->node_type) ? (x)->parent->parent : (x)->parent );
}

/* ----------------------------------------------------------------------

             Structures and Declarations:  Right Unlinking

---------------------------------------------------------------------- */

/* Note: a node is right unlinked iff the low-order bit of
   node->b.posneg.next_from_alpha_mem is 1 */

/*#define node_is_right_unlinked(node) \
  (((unsigned long)((node)->b.posneg.next_from_alpha_mem)) & 1)*/
inline unsigned long node_is_right_unlinked(rete_node * node)
{
  return (((unsigned long)((node)->b.posneg.next_from_alpha_mem)) & 1);
}

/*#define mark_node_as_right_unlinked(node) { \
  (node)->b.posneg.next_from_alpha_mem = static_cast<rete_node_struct *>((void *)1); }*/
inline void mark_node_as_right_unlinked(rete_node * node)
{
  (node)->b.posneg.next_from_alpha_mem = static_cast<rete_node_struct *>((void *)1);
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
inline void relink_to_right_mem(rete_node * node)
{
  rete_node *rtrm_ancestor, *rtrm_prev;
  /* find first ancestor that's linked */
  rtrm_ancestor = (node)->b.posneg.nearest_ancestor_with_same_am;
  while (rtrm_ancestor && node_is_right_unlinked(rtrm_ancestor))
    rtrm_ancestor = rtrm_ancestor->b.posneg.nearest_ancestor_with_same_am;
  if (rtrm_ancestor) {
    /* insert just before that ancestor */
    rtrm_prev = rtrm_ancestor->b.posneg.prev_from_alpha_mem;
    (node)->b.posneg.next_from_alpha_mem = rtrm_ancestor;
    (node)->b.posneg.prev_from_alpha_mem = rtrm_prev;
    rtrm_ancestor->b.posneg.prev_from_alpha_mem = (node);
    if (rtrm_prev) rtrm_prev->b.posneg.next_from_alpha_mem = (node);
    else (node)->b.posneg.alpha_mem_->beta_nodes = (node);
  } else {
    /* no such ancestor, insert at tail of list */
    rtrm_prev = (node)->b.posneg.alpha_mem_->last_beta_node;
    (node)->b.posneg.next_from_alpha_mem = NIL;
    (node)->b.posneg.prev_from_alpha_mem = rtrm_prev;
    (node)->b.posneg.alpha_mem_->last_beta_node = (node);
    if (rtrm_prev) rtrm_prev->b.posneg.next_from_alpha_mem = (node);
    else (node)->b.posneg.alpha_mem_->beta_nodes = (node);
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
  (((unsigned long)((node)->a.pos.next_from_beta_mem)) & 1)*/
inline unsigned long node_is_left_unlinked(rete_node * node)
{
  return (((unsigned long)((node)->a.pos.next_from_beta_mem)) & 1);
}

/*#define mark_node_as_left_unlinked(node) { \
  (node)->a.pos.next_from_beta_mem = static_cast<rete_node_struct *>((void *)1); }*/
inline void mark_node_as_left_unlinked(rete_node * node)
{
  (node)->a.pos.next_from_beta_mem = static_cast<rete_node_struct *>((void *)1);
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

inline void make_mp_bnode_left_unlinked(rete_node * node) 
{
  (node)->a.np.is_left_unlinked = 1;
}

inline void make_mp_bnode_left_linked(rete_node * node) 
{
  (node)->a.np.is_left_unlinked = 0;
}

inline unsigned mp_bnode_is_left_unlinked(rete_node * node) 
{ 
  return ((node)->a.np.is_left_unlinked);
}

/* ----------------------------------------------------------------------

                 Structures and Declarations:  Tokens

---------------------------------------------------------------------- */

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
inline void new_left_token(token * New, rete_node * current_node, 
                           token * parent_tok, wme * parent_wme)
{
  (New)->node = (current_node);
  insert_at_head_of_dll ((current_node)->a.np.tokens, (New),
                         next_of_node, prev_of_node);
  (New)->first_child = NIL;
  (New)->parent = (parent_tok);
  insert_at_head_of_dll ((parent_tok)->first_child, (New),
                         next_sibling, prev_sibling);
  (New)->w = (parent_wme);
  if (parent_wme) insert_at_head_of_dll ((parent_wme)->tokens, (New),
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
// FIXME: investigate hash table sizes
//#ifdef _WINDOWS
//#define LOG2_LEFT_HT_SIZE 13
//#define LOG2_RIGHT_HT_SIZE 13
//#else
#define LOG2_LEFT_HT_SIZE 14
#define LOG2_RIGHT_HT_SIZE 14
//#endif

#define LEFT_HT_SIZE (((long) 1) << LOG2_LEFT_HT_SIZE)
#define RIGHT_HT_SIZE (((long) 1) << LOG2_RIGHT_HT_SIZE)

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
inline token * & left_ht_bucket(agent* thisAgent, unsigned long hv)
{
  return (* ( ((token **) thisAgent->left_ht) + ((hv) & LEFT_HT_MASK)));
}

inline right_mem * right_ht_bucket(agent* thisAgent, unsigned long hv)
{
  return (* ( ((right_mem **) thisAgent->right_ht) + ((hv) & RIGHT_HT_MASK)));
}

/*#define insert_token_into_left_ht(tok,hv) { \
  token **header_zy37; \
  header_zy37 = ((token **) thisAgent->left_ht) + ((hv) & LEFT_HT_MASK); \
  insert_at_head_of_dll (*header_zy37, (tok), \
                         a.ht.next_in_bucket, a.ht.prev_in_bucket); }*/
inline void insert_token_into_left_ht(agent* thisAgent, token * tok, unsigned long hv) 
{
  token **header_zy37;
  header_zy37 = ((token **) thisAgent->left_ht) + ((hv) & LEFT_HT_MASK);
  insert_at_head_of_dll (*header_zy37, (tok),
                         a.ht.next_in_bucket, a.ht.prev_in_bucket);
}

/*#define remove_token_from_left_ht(tok,hv) { \
  fast_remove_from_dll (left_ht_bucket(hv), tok, token, \
                        a.ht.next_in_bucket, a.ht.prev_in_bucket); }*/
inline void remove_token_from_left_ht(agent* thisAgent, token * tok, unsigned long hv)
{
  fast_remove_from_dll (left_ht_bucket(thisAgent, hv), tok, token,
                        a.ht.next_in_bucket, a.ht.prev_in_bucket);
}

/* ----------------------------------------------------------------------

       Structures and Declarations:  Beta Net Interpreter Routines

---------------------------------------------------------------------- */

void (*(left_addition_routines[256])) (agent* thisAgent, rete_node *node, token *tok, wme *w) =
{ 
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
void (*(right_addition_routines[256])) (agent* thisAgent, rete_node *node, wme *w) =
{ 
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


void remove_token_and_subtree (agent* thisAgent, token *tok);

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

void null_activation_stats_for_right_activation (rete_node *node, rete_node *node_to_ignore_for_null_activation_stats)
{
  if (node==node_to_ignore_for_activation_stats) return;
  switch (node->node_type) {
  case POSITIVE_BNODE:
  case UNHASHED_POSITIVE_BNODE:
    thisAgent->num_right_activations++;
    if (! node->parent->a.np.tokens)
      thisAgent->num_null_right_activations++;
    break;
  case MP_BNODE:
  case UNHASHED_MP_BNODE:
    thisAgent->num_right_activations++;
    if (! node->a.np.tokens) thisAgent->num_null_right_activations++;
    break;
  }
}

void null_activation_stats_for_left_activation (rete_node *node) {
  switch (node->node_type) {
  case POSITIVE_BNODE:
  case UNHASHED_POSITIVE_BNODE:
    thisAgent->num_left_activations++;
    if (node->b.posneg.alpha_mem_->right_mems==NIL)
      thisAgent->num_null_left_activations++;
    break;
  case MP_BNODE:
  case UNHASHED_MP_BNODE:
    if (mp_bnode_is_left_unlinked(node)) return;
    thisAgent->num_left_activations++;
    if (node->b.posneg.alpha_mem_->right_mems==NIL)
      thisAgent->num_null_left_activations++;
    break;
  }
}

void print_null_activation_stats () {
  print ("\nActivations: %lu right (%lu null), %lu left (%lu null)\n",
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
inline void init_sharing_stats_for_new_node(node * node)
{
  (node)->sharing_factor = 0;
}

/*#define set_sharing_factor(node,sf) { \
  long ssf_237; \
  ssf_237 = (sf) - ((node)->sharing_factor); \
  (node)->sharing_factor = (sf); \
  thisAgent->rete_node_counts_if_no_sharing[(node)->node_type]+=ssf_237; }*/
inline void set_sharing_factor(rete_node * node, unsigned long sf)
{
  long ssf_237;
  ssf_237 = (sf) - ((node)->sharing_factor);
  (node)->sharing_factor = (sf);
  thisAgent->rete_node_counts_if_no_sharing[(node)->node_type]+=ssf_237;
}

/* Scans from "node" up to the top node, adds "delta" to sharing factors. */
void adjust_sharing_factors_from_here_to_top (rete_node *node, int delta) {
  while (node!=NIL) {
    thisAgent->rete_node_counts_if_no_sharing[node->node_type] += delta;
    node->sharing_factor += delta;
    if (node->node_type==CN_BNODE) node = node->b.cn.partner;
    else node = node->parent;
  }
}

#else

/* Since these do nothing, I will not convert them to inline functions
   for the time being. -ajc (5/6/02 */
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
inline unsigned long real_sharing_factor(rete_node * node)
{
  return ((node)->sharing_factor ? (node)->sharing_factor : 1);
}

/*#define token_added(node) { \
  thisAgent->token_additions++; \
  thisAgent->token_additions_without_sharing += real_sharing_factor(node);}*/
inline void token_added(rete_node * node)
{
  thisAgent->token_additions++;
  thisAgent->token_additions_without_sharing += real_sharing_factor(node);
}

#else

#define token_added(node) {}

#endif

/* --- Invoked on every right activation; add=TRUE means right addition --- */
/* NOT invoked on removals unless DO_ACTIVATION_STATS_ON_REMOVALS is set */
/*#define right_node_activation(node,add) { \
  null_activation_stats_for_right_activation(node); }*/
inline void right_node_activation(rete_node * node, Bool add)
{
  null_activation_stats_for_right_activation(node);
}

/* --- Invoked on every left activation; add=TRUE means left addition --- */
/* NOT invoked on removals unless DO_ACTIVATION_STATS_ON_REMOVALS is set */
/*#define left_node_activation(node,add) { \
  null_activation_stats_for_left_activation(node); }*/
inline void left_node_activation(rete_node * node, Bool add)
{
  null_activation_stats_for_left_activation(node);
}

/* --- The following two macros are used when creating/destroying nodes --- */

/*#define init_new_rete_node_with_type(node,type) { \
  (node)->node_type = (type); \
  thisAgent->rete_node_counts[(type)]++; \
  init_sharing_stats_for_new_node(node); }*/
inline void init_new_rete_node_with_type(agent* thisAgent, rete_node * node, byte type)
{
  (node)->node_type = (type);
  thisAgent->rete_node_counts[(type)]++;
  init_sharing_stats_for_new_node(node);
}

/*#define update_stats_for_destroying_node(node) { \
  set_sharing_factor(node,0); \
  thisAgent->rete_node_counts[(node)->node_type]--; }*/
inline void update_stats_for_destroying_node(agent* thisAgent, rete_node * node)
{
  set_sharing_factor(node,0);
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
   Any_assertions_or_retractions_ready() returns TRUE iff there are any
   pending changes to the match set.  This is used to test for quiescence.
   Get_next_assertion() retrieves a pending assertion (returning TRUE) or
   returns FALSE is no more are available.  Get_next_retraction() is
   similar.
********************************************************************** */



/* REW: begin 08.20.97 */

Symbol *find_goal_for_match_set_change_assertion(agent* thisAgent, ms_change *msc) {

  wme *lowest_goal_wme;
  goal_stack_level lowest_level_so_far;
  token *tok;

#ifdef DEBUG_WATERFALL
  print_with_symbols(thisAgent, "\nMatch goal for assertion: %y", msc->p_node->b.p.prod->name); 
#endif


  lowest_goal_wme = NIL;
  lowest_level_so_far = -1;

  if (msc->w) {
      if (msc->w->id->id.isa_goal == TRUE) {
        lowest_goal_wme = msc->w;
        lowest_level_so_far = msc->w->id->id.level;
      }
  }

  for (tok=msc->tok; tok!=thisAgent->dummy_top_token; tok=tok->parent) {
    if (tok->w != NIL) {
      /* print_wme(tok->w); */
      if (tok->w->id->id.isa_goal == TRUE) {

        if (lowest_goal_wme == NIL)
          lowest_goal_wme = tok->w;
        
        else {
          if (tok->w->id->id.level > lowest_goal_wme->id->id.level)
            lowest_goal_wme = tok->w;
        }
      }
       
    }
  } 

  if (lowest_goal_wme) {
#ifdef DEBUG_WATERFALL
    print_with_symbols(thisAgent, " is [%y]", lowest_goal_wme->id);
#endif
       return lowest_goal_wme->id;
  }
  { char msg[BUFFER_MSG_SIZE];
  print_with_symbols(thisAgent, "\nError: Did not find goal for ms_change assertion: %y\n", msc->p_node->b.p.prod->name);
  snprintf(msg, BUFFER_MSG_SIZE,"\nError: Did not find goal for ms_change assertion: %s\n",
         symbol_to_string(thisAgent, msc->p_node->b.p.prod->name,TRUE,NIL, 0));
  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
  abort_with_fatal_error(thisAgent, msg);
  }
  return 0;
}

Symbol *find_goal_for_match_set_change_retraction(ms_change *msc) {

#ifdef DEBUG_WATERFALL
       print_with_symbols(thisAgent, "\nMatch goal level for retraction: %y", msc->inst->prod->name);
#endif

   if (msc->inst->match_goal) {
     /* If there is a goal, just return the goal */
#ifdef DEBUG_WATERFALL
     print_with_symbols(thisAgent, " is [%y]", msc->inst->match_goal);
#endif 
     return  msc->inst->match_goal;

   }  else {

#ifdef DEBUG_WATERFALL
     print(" is NIL (nil goal retraction)");
#endif 
     return NIL;

   }
}   

void print_assertion( agent* thisAgent, ms_change *msc) {

  if (msc->p_node)
   print_with_symbols(thisAgent, "\nAssertion: %y", msc->p_node->b.p.prod->name);
  else
    print(thisAgent, "\nAssertion exists but has no p_node");
}

void print_retraction( agent* thisAgent, ms_change *msc) {

  if (msc->p_node)
    print_with_symbols(thisAgent, "\nRetraction: %y", msc->p_node->b.p.prod->name);
  else 
    print(thisAgent, "\nRetraction exists but has no p_node");
}

/* REW: end   08.20.97 */


Bool any_assertions_or_retractions_ready (agent* thisAgent) {
 
  Symbol *goal;

  /* REW: begin 08.20.97 */
  if (thisAgent->operand2_mode == TRUE) {

  /* Determining if assertions or retractions are ready require looping over
     all goals in Waterfall/Operand2 */

     if (thisAgent->nil_goal_retractions) return TRUE;

     /* Loop from bottom to top because we expect activity at
        the bottom usually */

     for (goal=thisAgent->bottom_goal;goal;goal=goal->id.higher_goal) {
       /* if there are any assertions or retrctions for this goal,
          return TRUE */
       if (goal->id.ms_o_assertions || goal->id.ms_i_assertions ||
           goal->id.ms_retractions)
         return TRUE;
     }

     /* if there are no nil_goal_retractions and no assertions or retractions
        for any  goal then return FALSE -- there aren't any productions
        ready to fire or retract */

     return FALSE;

  }

/* REW: end   08.20.97 */

  else

  return (thisAgent->ms_assertions || thisAgent->ms_retractions);
}


/* RCHONG: begin 10.11 */

Bool any_i_assertions_or_retractions_ready (agent* thisAgent) {
   return (thisAgent->ms_i_assertions || thisAgent->ms_retractions);
}

/* RCHONG: end 10.11 */


Bool get_next_assertion (agent* thisAgent, production **prod,
                         struct token_struct **tok,
                         wme **w) {
  ms_change *msc;
 
  msc = NIL; /* unneeded, but avoids gcc -Wall warn */ 

  
  /* REW: begin 09.15.96 */
  if (thisAgent->operand2_mode == TRUE) {
     
     /* REW: begin 08.20.97 */
     
     /* In Waterfall, we return only assertions that match in the
     currently active goal */
     
     if (thisAgent->active_goal) { /* Just do asserts for current goal */
        if (thisAgent->FIRING_TYPE == PE_PRODS) {
           if (! thisAgent->active_goal->id.ms_o_assertions) return FALSE;
           
           msc = thisAgent->active_goal->id.ms_o_assertions;
           remove_from_dll (thisAgent->ms_o_assertions, msc, next, prev);
           remove_from_dll (thisAgent->active_goal->id.ms_o_assertions,
              msc, next_in_level, prev_in_level);
           
        } else {
           /* IE PRODS */
           if (! thisAgent->active_goal->id.ms_i_assertions) return FALSE;
           
           msc = thisAgent->active_goal->id.ms_i_assertions;
           remove_from_dll (thisAgent->ms_i_assertions, msc, next, prev);
           remove_from_dll (thisAgent->active_goal->id.ms_i_assertions,
              msc, next_in_level, prev_in_level);
        }
        
     } else {
        
     /* If there is not an active goal, then there should not be any
     assertions.  If there are, then we generate and error message
        and abort. */
        
        if ((thisAgent->ms_i_assertions) ||
           (thisAgent->ms_o_assertions)) {
           char msg[BUFFER_MSG_SIZE];
           strncpy(msg,"\nrete.c: Error: No active goal, but assertions are on the assertion list.", BUFFER_MSG_SIZE);
           msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
           abort_with_fatal_error(thisAgent, msg);
           
        }   
        
        return FALSE; /* if we are in an initiazation and there are no
                      assertions, just retrurn FALSE to terminate
        the procedure. */
        
     }
     /* REW: end   08.20.97 */
  }
  /* REW: end   09.15.96 */

   else {
      if (! thisAgent->ms_assertions) return FALSE;
      msc = thisAgent->ms_assertions;

      remove_from_dll (thisAgent->ms_assertions, msc, next, prev);
   }


  remove_from_dll (msc->p_node->b.p.tentative_assertions, msc,
                   next_of_node, prev_of_node);
  *prod = msc->p_node->b.p.prod;
  *tok = msc->tok;
  *w = msc->w;
  free_with_pool (&thisAgent->ms_change_pool, msc);
  return TRUE;
}

Bool get_next_retraction (agent* thisAgent, instantiation **inst) {
  ms_change *msc;

  /* REW: begin 08.20.97 */
  if (!thisAgent->operand2_mode ) { 
    /* for non-Operand2 modes, just remove the head of the retractions list */
    /* REW: end   08.20.97 */
    if (! thisAgent->ms_retractions) return FALSE;
    msc = thisAgent->ms_retractions;
    remove_from_dll (thisAgent->ms_retractions, msc, next, prev);
    if (msc->p_node)
      remove_from_dll (msc->p_node->b.p.tentative_retractions, msc,
                     next_of_node, prev_of_node);
    *inst = msc->inst;
    free_with_pool (&thisAgent->ms_change_pool, msc);
    return TRUE;

  /* REW: begin 08.20.97 */
  } else {
    /* just do the retractions for the current level */

    /* initialization condition (2.107/2.111) */
    if (thisAgent->active_level == 0) return FALSE; 

    if (! thisAgent->active_goal->id.ms_retractions) return FALSE;

    msc = thisAgent->active_goal->id.ms_retractions;

    /* remove from the complete retraction list */
    remove_from_dll (thisAgent->ms_retractions, msc, next, prev);
    /* and remove from the Waterfall-specific list */
    remove_from_dll (thisAgent->active_goal->id.ms_retractions,
                     msc, next_in_level, prev_in_level);
    if (msc->p_node)
      remove_from_dll (msc->p_node->b.p.tentative_retractions, msc,
                     next_of_node, prev_of_node);
    *inst = msc->inst;
    free_with_pool (&thisAgent->ms_change_pool, msc);
    return TRUE;

  }
  /* REW: end   08.20.97 */
}





/* REW: begin 08.20.97 */

/* Retract an instantiation on the nil goal list.  If there are no
   retractions on the nil goal retraction list, return FALSE.  This
   procedure is only called in Operand2 mode, so there is no need for
   any checks for Operand2-specific processing. */

Bool get_next_nil_goal_retraction (agent* thisAgent, instantiation **inst) {
  ms_change *msc;

    if (! thisAgent->nil_goal_retractions) return FALSE;
    msc = thisAgent->nil_goal_retractions;

    /* Remove this retraction from the NIL goal list */
    remove_from_dll (thisAgent->nil_goal_retractions, msc,
                     next_in_level, prev_in_level);

    /* next and prev set and used in Operand2 exactly as used in Soar 7 --
       so we have to make sure and delete this retraction from the regular
       list */
    remove_from_dll (thisAgent->ms_retractions, msc, next, prev);

    if (msc->p_node) {
      remove_from_dll (msc->p_node->b.p.tentative_retractions, msc,
                     next_of_node, prev_of_node);
    }
    *inst = msc->inst;
    free_with_pool (&thisAgent->ms_change_pool, msc);
    return TRUE;

}

/* REW: end   08.20.97 */











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

/* --- Returns TRUE iff the given wme goes into the given alpha memory --- */
/*#define wme_matches_alpha_mem(w,am) ( \
  (((am)->id==NIL) || ((am)->id==(w)->id)) && \
  (((am)->attr==NIL) || ((am)->attr==(w)->attr)) && \
  (((am)->value==NIL) || ((am)->value==(w)->value)) && \
  ((am)->acceptable==(w)->acceptable))*/
inline Bool wme_matches_alpha_mem(wme * w, alpha_mem * am)
{
  return (((am)->id==NIL) || ((am)->id==(w)->id)) &&
    (((am)->attr==NIL) || ((am)->attr==(w)->attr)) &&
    (((am)->value==NIL) || ((am)->value==(w)->value)) &&
    ((am)->acceptable==(w)->acceptable);
}

/* --- Returns hash value for the given id/attr/value symbols --- */
/*#define alpha_hash_value(i,a,v,num_bits) \
 ( ( ((i) ? ((Symbol *)(i))->common.hash_id : 0) ^ \
     ((a) ? ((Symbol *)(a))->common.hash_id : 0) ^ \
     ((v) ? ((Symbol *)(v))->common.hash_id : 0) ) & \
   masks_for_n_low_order_bits[(num_bits)] )*/
inline unsigned long alpha_hash_value(Symbol * i, Symbol * a, Symbol * v, short num_bits)
{
  return ( ( ((i) ? ((Symbol *)(i))->common.hash_id : 0) ^
    ((a) ? ((Symbol *)(a))->common.hash_id : 0) ^
    ((v) ? ((Symbol *)(v))->common.hash_id : 0) ) &
    masks_for_n_low_order_bits[(num_bits)] );
}

/* --- rehash funciton for resizable hash table routines --- */
unsigned long hash_alpha_mem (void *item, short num_bits) {
  alpha_mem *am;

  am = static_cast<alpha_mem_struct *>(item);
  return alpha_hash_value (am->id, am->attr, am->value, num_bits);
}

/* --- Which of the 16 hash tables to use? --- */
/*#define table_for_tests(id,attr,value,acceptable) \
  thisAgent->alpha_hash_tables [ ((id) ? 1 : 0) + ((attr) ? 2 : 0) + \
                                     ((value) ? 4 : 0) + \
                                     ((acceptable) ? 8 : 0) ]*/
inline hash_table * table_for_tests(agent* thisAgent, 
                                                                        Symbol * id, Symbol * attr, Symbol * value, 
                                    Bool acceptable)
{
  return thisAgent->alpha_hash_tables [ ((id) ? 1 : 0) + ((attr) ? 2 : 0) +
    ((value) ? 4 : 0) +
    ((acceptable) ? 8 : 0) ];
}

//#define get_next_alpha_mem_id() (thisAgent->alpha_mem_id_counter++)
inline unsigned long get_next_alpha_mem_id(agent* thisAgent)
{
  return thisAgent->alpha_mem_id_counter++;
}

/* --- Adds a WME to an alpha memory (create a right_mem for it), but doesn't
   inform any successors --- */
void add_wme_to_alpha_mem (agent* thisAgent, wme *w, alpha_mem *am) {
  right_mem **header, *rm;
  unsigned long hv;

  /* --- allocate new right_mem, fill it fields --- */
  allocate_with_pool (thisAgent, &thisAgent->right_mem_pool, &rm);
  rm->w = w;
  rm->am = am;

  /* --- add it to dll's for the hash bucket, alpha mem, and wme --- */
  hv = am->am_id ^ w->id->common.hash_id;
  header = ((right_mem **)thisAgent->right_ht) + (hv & RIGHT_HT_MASK);
  insert_at_head_of_dll (*header, rm, next_in_bucket, prev_in_bucket);
  insert_at_head_of_dll (am->right_mems, rm, next_in_am, prev_in_am);
  insert_at_head_of_dll (w->right_mems, rm, next_from_wme, prev_from_wme);
}

/* --- Removes a WME (right_mem) from its alpha memory, but doesn't inform
   any successors --- */
void remove_wme_from_alpha_mem (agent* thisAgent, right_mem *rm) {
  wme *w;
  alpha_mem *am;
  unsigned long hv;
  right_mem **header;

  w = rm->w;  
  am = rm->am;

  /* --- remove it from dll's for the hash bucket, alpha mem, and wme --- */
  hv = am->am_id ^ w->id->common.hash_id;
  header = ((right_mem **)thisAgent->right_ht) + (hv & RIGHT_HT_MASK);
  remove_from_dll (*header, rm, next_in_bucket, prev_in_bucket);
  remove_from_dll (am->right_mems, rm, next_in_am, prev_in_am);
  remove_from_dll (w->right_mems, rm, next_from_wme, prev_from_wme);

  /* --- deallocate it --- */
  free_with_pool (&thisAgent->right_mem_pool, rm);
}

/* --- Looks for an existing alpha mem, returns it or NIL if not found --- */
alpha_mem *find_alpha_mem (agent* thisAgent, Symbol *id, Symbol *attr,
                           Symbol *value, Bool acceptable) {
  hash_table *ht;
  alpha_mem *am;
  unsigned long hash_value;

  ht = table_for_tests (thisAgent, id, attr, value, acceptable);
  hash_value = alpha_hash_value (id, attr, value, ht->log2size);

  for (am = (alpha_mem *) (*(ht->buckets+hash_value)); am!=NIL;
       am=am->next_in_hash_table)
    if ((am->id==id) && (am->attr==attr) &&
        (am->value==value) && (am->acceptable==acceptable))
      return am;
  return NIL;
}

/* --- Find and share existing alpha memory, or create new one.  Adjusts
   the reference count on the alpha memory accordingly. --- */
alpha_mem *find_or_make_alpha_mem (agent* thisAgent, Symbol *id, Symbol *attr,
                                   Symbol *value, Bool acceptable) {
  hash_table *ht;
  alpha_mem *am, *more_general_am;
  wme *w;
  right_mem *rm;

  /* --- look for an existing alpha mem --- */
  am = find_alpha_mem (thisAgent, id, attr, value, acceptable);
  if (am) {
    am->reference_count++;
    return am;
  }
  
  /* --- no existing alpha_mem found, so create a new one --- */
  allocate_with_pool (thisAgent, &thisAgent->alpha_mem_pool, &am);
  am->next_in_hash_table = NIL;
  am->right_mems = NIL;
  am->beta_nodes = NIL;
  am->last_beta_node = NIL;
  am->reference_count = 1;
  am->id = id;
  if (id) symbol_add_ref (id);
  am->attr = attr;
  if (attr) symbol_add_ref (attr);
  am->value = value;
  if (value) symbol_add_ref (value);
  am->acceptable = acceptable;
  am->am_id = get_next_alpha_mem_id(thisAgent);
  ht = table_for_tests (thisAgent, id, attr, value, acceptable);
  add_to_hash_table (thisAgent, ht, am);
  
  /* --- fill new mem with any existing matching WME's --- */
  more_general_am = NIL;
  if (id)
    more_general_am = find_alpha_mem (thisAgent, NIL, attr, value, acceptable);
  if (!more_general_am && value)
    more_general_am = find_alpha_mem (thisAgent, NIL, attr, NIL, acceptable);
  if (more_general_am) {
    /* --- fill new mem using the existing more general one --- */
    for (rm=more_general_am->right_mems; rm!=NIL; rm=rm->next_in_am)
      if (wme_matches_alpha_mem (rm->w, am))
        add_wme_to_alpha_mem (thisAgent, rm->w, am);
  } else {
    /* --- couldn't find such an existing mem, so do it the hard way --- */
    for (w=thisAgent->all_wmes_in_rete; w!=NIL; w=w->rete_next)
      if (wme_matches_alpha_mem (w,am)) add_wme_to_alpha_mem (thisAgent, w, am);
  }
  
  return am;
}

/* --- Using the given hash table and hash value, try to find a 
   matching alpha memory in the indicated hash bucket.  If we find one,
   we add the wme to it and inform successor nodes. --- */
void add_wme_to_aht (agent* thisAgent, hash_table *ht, unsigned long hash_value, wme *w) {
  alpha_mem *am;
  rete_node *node, *next;
 
  hash_value = hash_value & masks_for_n_low_order_bits[ht->log2size];
  am = (alpha_mem *) (*(ht->buckets+hash_value));
  while (am!=NIL) {
    if (wme_matches_alpha_mem (w,am)) {
      /* --- found the right alpha memory, first add the wme --- */
      add_wme_to_alpha_mem (thisAgent, w, am);

      /* --- now call the beta nodes --- */
      for (node=am->beta_nodes; node!=NIL; node=next) {
        next = node->b.posneg.next_from_alpha_mem;
        (*(right_addition_routines[node->node_type]))(thisAgent,node,w);
      }
      return; /* only one possible alpha memory per table could match */
    } 
    am = am->next_in_hash_table;
  }
}
     
/* We cannot use 'xor' as the name of a function because it is defined in UNIX. */
//#define xor_op(i,a,v) ((i) ^ (a) ^ (v))
inline unsigned long xor_op(unsigned long i, unsigned long a, unsigned long v)
{
  return ((i) ^ (a) ^ (v));
}

/* --- Adds a WME to the Rete. --- */
void add_wme_to_rete (agent* thisAgent, wme *w) {
  unsigned long hi, ha, hv;

  /* --- add w to all_wmes_in_rete --- */
  insert_at_head_of_dll (thisAgent->all_wmes_in_rete, w, rete_next, rete_prev);
  thisAgent->num_wmes_in_rete++;

  /* --- it's not in any right memories or tokens yet --- */
  w->right_mems = NIL;
  w->tokens = NIL;

  /* --- add w to the appropriate alpha_mem in each of 8 possible tables --- */
  hi = w->id->common.hash_id;
  ha = w->attr->common.hash_id;
  hv = w->value->common.hash_id;

  if (w->acceptable) {
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[8],  xor_op( 0, 0, 0), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[9],  xor_op(hi, 0, 0), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[10], xor_op( 0,ha, 0), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[11], xor_op(hi,ha, 0), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[12], xor_op( 0, 0,hv), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[13], xor_op(hi, 0,hv), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[14], xor_op( 0,ha,hv), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[15], xor_op(hi,ha,hv), w);
  } else {
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[0],  xor_op( 0, 0, 0), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[1],  xor_op(hi, 0, 0), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[2],  xor_op( 0,ha, 0), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[3],  xor_op(hi,ha, 0), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[4],  xor_op( 0, 0,hv), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[5],  xor_op(hi, 0,hv), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[6],  xor_op( 0,ha,hv), w);
    add_wme_to_aht (thisAgent, thisAgent->alpha_hash_tables[7],  xor_op(hi,ha,hv), w);
  }
}

/* --- Removes a WME from the Rete. --- */
void remove_wme_from_rete (agent* thisAgent, wme *w) {
  right_mem *rm;
  alpha_mem *am;
  rete_node *node, *next, *child;
  token *tok, *left;
  
  /* --- remove w from all_wmes_in_rete --- */
  remove_from_dll (thisAgent->all_wmes_in_rete, w, rete_next, rete_prev);
  thisAgent->num_wmes_in_rete--;
  
  /* --- remove w from each alpha_mem it's in --- */
  while (w->right_mems) {
    rm = w->right_mems;
    am = rm->am;
    /* --- found the alpha memory, first remove the wme from it --- */
    remove_wme_from_alpha_mem (thisAgent, rm);
    
#ifdef DO_ACTIVATION_STATS_ON_REMOVALS
    /* --- if doing statistics stuff, then activate each attached node --- */
    for (node=am->beta_nodes; node!=NIL; node=next) {
      next = node->b.posneg.next_from_alpha_mem;
      right_node_activation (node,FALSE);
    }
#endif
    
    /* --- for left unlinking, then if the alpha memory just went to
       zero, left unlink any attached Pos or MP nodes --- */
    if (am->right_mems==NIL) {
      for (node=am->beta_nodes; node!=NIL; node=next) {
        next = node->b.posneg.next_from_alpha_mem;
        switch (node->node_type) {
        case POSITIVE_BNODE:
        case UNHASHED_POSITIVE_BNODE:
          unlink_from_left_mem (node);
          break;
        case MP_BNODE:
        case UNHASHED_MP_BNODE:
          make_mp_bnode_left_unlinked (node);
          break;
        } /* end of switch (node->node_type) */
      }
    }
  }
 
  /* --- tree-based removal of all tokens that involve w --- */
  while (w->tokens) {
    tok = w->tokens;
    node = tok->node;
    if (! tok->parent) {
      /* Note: parent pointer is NIL only on negative node negrm tokens */
      left = tok->a.neg.left_token;
      remove_from_dll (w->tokens, tok, next_from_wme, prev_from_wme);
      remove_from_dll (left->negrm_tokens, tok,
                       a.neg.next_negrm, a.neg.prev_negrm);
      free_with_pool (&thisAgent->token_pool, tok);
      if (! left->negrm_tokens) { /* just went to 0, so call children */
        for (child=node->first_child; child!=NIL; child=child->next_sibling)
          (*(left_addition_routines[child->node_type]))(thisAgent,child,left,NIL);
      }
    } else {
      remove_token_and_subtree (thisAgent, w->tokens);
    }
  }
}

/* --- Decrements reference count, deallocates alpha memory if unused. --- */  
void remove_ref_to_alpha_mem (agent* thisAgent, alpha_mem *am) {
  hash_table *ht;
 
  am->reference_count--;
  if (am->reference_count!=0) return;
  /* --- remove from hash table, and deallocate the alpha_mem --- */
  ht = table_for_tests (thisAgent, am->id, am->attr, am->value, am->acceptable);
  remove_from_hash_table (thisAgent, ht, am);
  if (am->id) symbol_remove_ref (thisAgent, am->id);
  if (am->attr) symbol_remove_ref (thisAgent, am->attr);
  if (am->value) symbol_remove_ref (thisAgent, am->value);
  while (am->right_mems) remove_wme_from_alpha_mem (thisAgent, am->right_mems);
  free_with_pool (&thisAgent->alpha_mem_pool, am);
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
inline unsigned long get_next_beta_node_id(agent* thisAgent)
{
  return (thisAgent->beta_node_id_counter++);
}

/* ------------------------------------------------------------------------
                          Init Dummy Top Node

   The dummy top node always has one token in it (WME=NIL).  This is 
   just there so that (real) root nodes in the beta net can be handled 
   the same as non-root nodes.
------------------------------------------------------------------------ */

void init_dummy_top_node (agent* thisAgent) {
  /* --- create the dummy top node --- */
  allocate_with_pool (thisAgent, &thisAgent->rete_node_pool,
                      &thisAgent->dummy_top_node);
  init_new_rete_node_with_type (thisAgent, thisAgent->dummy_top_node,DUMMY_TOP_BNODE);
  thisAgent->dummy_top_node->parent = NIL;
  thisAgent->dummy_top_node->first_child = NIL;
  thisAgent->dummy_top_node->next_sibling = NIL;

  /* --- create the dummy top token --- */
  allocate_with_pool (thisAgent, &thisAgent->token_pool,
                      &thisAgent->dummy_top_token);
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

void remove_node_from_parents_list_of_children (rete_node *node) {
  rete_node *prev_sibling;

  prev_sibling = node->parent->first_child;
  if (prev_sibling == node) {
    node->parent->first_child = node->next_sibling;
    return;
  }
  while (prev_sibling->next_sibling != node)
    prev_sibling = prev_sibling->next_sibling;
  prev_sibling->next_sibling = node->next_sibling;
}

/* ------------------------------------------------------------------------
                 Update Node With Matches From Above

   Calls a node's left-addition routine with each match (token) from 
   the node's parent.  DO NOT call this routine on (positive, unmerged)
   join nodes.
------------------------------------------------------------------------ */

void update_node_with_matches_from_above (agent* thisAgent, rete_node *child)
{
  rete_node *parent;
  rete_node *saved_parents_first_child, *saved_childs_next_sibling;
  right_mem *rm;
  token *tok;
  
  if (bnode_is_bottom_of_split_mp(child->node_type)) {
    char msg[BUFFER_MSG_SIZE];
    strncpy (msg, "\nrete.c: Internal error: update_node_with_matches_from_above called on split node", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error (thisAgent, msg);
  }
  
  parent = child->parent;

  /* --- if parent is dummy top node, tell child about dummy top token --- */ 
  if (parent->node_type==DUMMY_TOP_BNODE) {
    (*(left_addition_routines[child->node_type]))(thisAgent,child,thisAgent->dummy_top_token,NIL);
    return;
  }

  /* --- if parent is positive: first do surgery on parent's child list,
         to replace the list with "child"; then call parent's add_right 
         routine with each wme in the parent's alpha mem; then do surgery 
         to restore previous child list of parent. --- */
  if (bnode_is_positive(parent->node_type)) {
    /* --- If the node is right unlinked, then don't activate it.  This is
       important because some interpreter routines rely on the node
       being right linked whenever it gets right activated. */
    if (node_is_right_unlinked (parent)) return;
    saved_parents_first_child = parent->first_child;
    saved_childs_next_sibling = child->next_sibling;
    parent->first_child = child;
    child->next_sibling = NIL;
    /* to avoid double-counting these right adds */
    rete_node* node_to_ignore_for_activation_stats = parent;
    for (rm=parent->b.posneg.alpha_mem_->right_mems; rm!=NIL; rm=rm->next_in_am)
      (*(right_addition_routines[parent->node_type]))(thisAgent,parent,rm->w);
    node_to_ignore_for_activation_stats = NIL;
    parent->first_child = saved_parents_first_child;
    child->next_sibling = saved_childs_next_sibling;
    return;
  }
    
  /* --- if parent is negative or cn: easy, just look at the list of tokens
         on the parent node. --- */
  for (tok=parent->a.np.tokens; tok!=NIL; tok=tok->next_of_node)
    if (! tok->negrm_tokens)
      (*(left_addition_routines[child->node_type])) (thisAgent,child,tok,NIL);
}

/* ------------------------------------------------------------------------
                     Nearest Ancestor With Same AM

   Scans up the net and finds the first (i.e., nearest) ancestor node
   that uses a given alpha_mem.  Returns that node, or NIL if none exists.
------------------------------------------------------------------------ */

rete_node *nearest_ancestor_with_same_am (rete_node *node, alpha_mem *am) {
  while (node->node_type!=DUMMY_TOP_BNODE) {
    if (node->node_type==CN_BNODE) node = node->b.cn.partner->parent;
    else node = real_parent_node(node);
    if (bnode_is_posneg(node->node_type) && (node->b.posneg.alpha_mem_==am))
      return node;
  }
  return NIL;
}

/* --------------------------------------------------------------------
                         Make New Mem Node
 
   Make a new beta memory node, return a pointer to it.
-------------------------------------------------------------------- */

rete_node *make_new_mem_node (agent* thisAgent, 
                                                          rete_node *parent, byte node_type,
                              var_location left_hash_loc) {
  rete_node *node;

  /* --- create the node data structure, fill in fields --- */
  allocate_with_pool (thisAgent, &thisAgent->rete_node_pool, &node);
  init_new_rete_node_with_type (thisAgent, node, node_type);
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
  update_node_with_matches_from_above (thisAgent, node);
  
  return node;
}

/* --------------------------------------------------------------------
                         Make New Positive Node

   Make a new positive join node, return a pointer to it.
-------------------------------------------------------------------- */

rete_node *make_new_positive_node (agent* thisAgent, 
                                                                   rete_node *parent_mem, byte node_type,
                                   alpha_mem *am, rete_test *rt,
                                   Bool prefer_left_unlinking) {
  rete_node *node;
  
  /* --- create the node data structure, fill in fields --- */
  allocate_with_pool (thisAgent, &thisAgent->rete_node_pool, &node);
  init_new_rete_node_with_type (thisAgent, node, node_type);
  node->parent = parent_mem;
  node->next_sibling = parent_mem->first_child;
  parent_mem->first_child = node;
  node->first_child = NIL;
  relink_to_left_mem (node);
  node->b.posneg.other_tests = rt;
  node->b.posneg.alpha_mem_ = am;
  node->b.posneg.nearest_ancestor_with_same_am =
    nearest_ancestor_with_same_am (node, am);
  relink_to_right_mem (node);

  /* --- don't need to force WM through new node yet, as it's just a
     join node with no children --- */
  
  /* --- unlink the join node from one side if possible --- */
  if (! parent_mem->a.np.tokens) unlink_from_right_mem (node);
  if ((! am->right_mems) && ! node_is_right_unlinked (node))
    unlink_from_left_mem (node);
  if (prefer_left_unlinking && (! parent_mem->a.np.tokens) &&
      (! am->right_mems)) {
    relink_to_right_mem (node);
    unlink_from_left_mem (node);
  }

  return node;
}

/* --------------------------------------------------------------------
                             Split MP Node

   Split a given MP node into separate M and P nodes, return a pointer
   to the new Memory node.
-------------------------------------------------------------------- */

rete_node *split_mp_node (agent* thisAgent, rete_node *mp_node) {
  rete_node mp_copy;
  rete_node *pos_node, *mem_node, *parent;
  byte mem_node_type, node_type;
  token *t;
  
  /* --- determine appropriate node types for new M and P nodes --- */
  if (mp_node->node_type==MP_BNODE) {
    node_type = POSITIVE_BNODE;
    mem_node_type = MEMORY_BNODE;
  } else {
    node_type = UNHASHED_POSITIVE_BNODE;
    mem_node_type = UNHASHED_MEMORY_BNODE;
  }
  
  /* --- save a copy of the MP data, then kill the MP node --- */
  mp_copy = *mp_node;
  parent = mp_node->parent;
  remove_node_from_parents_list_of_children (mp_node);
  update_stats_for_destroying_node (thisAgent, mp_node);  /* clean up rete stats stuff */
  
  /* --- the old MP node will get transmogrified into the new Pos node --- */
  pos_node = mp_node;

  /* --- create the new M node, transfer the MP node's tokens to it --- */
  allocate_with_pool (thisAgent, &thisAgent->rete_node_pool, &mem_node);
  init_new_rete_node_with_type (thisAgent, mem_node, mem_node_type);
  set_sharing_factor (mem_node, mp_copy.sharing_factor);
  
  mem_node->parent = parent;
  mem_node->next_sibling = parent->first_child;
  parent->first_child = mem_node;
  mem_node->first_child = pos_node;
  mem_node->b.mem.first_linked_child = NIL;
  mem_node->left_hash_loc_field_num = mp_copy.left_hash_loc_field_num;
  mem_node->left_hash_loc_levels_up = mp_copy.left_hash_loc_levels_up;
  mem_node->node_id = mp_copy.node_id;

  mem_node->a.np.tokens = mp_node->a.np.tokens;
  for (t=mp_node->a.np.tokens; t!=NIL; t=t->next_of_node) t->node = mem_node;
  
  /* --- transmogrify the old MP node into the new Pos node --- */
  init_new_rete_node_with_type (thisAgent, pos_node, node_type);
  pos_node->parent = mem_node;
  pos_node->first_child = mp_copy.first_child;
  pos_node->next_sibling = NIL;
  pos_node->b.posneg = mp_copy.b.posneg;
  relink_to_left_mem (pos_node);   /* for now, but might undo this below */
  set_sharing_factor (pos_node, mp_copy.sharing_factor);

  /* --- set join node's unlinking status according to mp_copy's --- */
  if (mp_bnode_is_left_unlinked(&mp_copy)) unlink_from_left_mem (pos_node);
  
  return mem_node;
}

/* --------------------------------------------------------------------
                           Merge Into MP Node

   Merge a given Memory node and its one positive join child into an
   MP node, returning a pointer to the MP node.
-------------------------------------------------------------------- */

rete_node *merge_into_mp_node (agent* thisAgent, rete_node *mem_node) {
  rete_node *pos_node, *mp_node, *parent;
  rete_node pos_copy;
  byte node_type;
  token *t;
  
  pos_node = mem_node->first_child;
  parent = mem_node->parent;

  /* --- sanity check: Mem node must have exactly one child --- */
  if ((! pos_node) || pos_node->next_sibling) {
    char msg[BUFFER_MSG_SIZE];
    strncpy (msg, "\nrete.c: Internal error: tried to merge_into_mp_node, but <>1 child\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
  }
 
  /* --- determine appropriate node type for new MP node --- */
  if (mem_node->node_type==MEMORY_BNODE) {
    node_type = MP_BNODE;
  } else {
    node_type = UNHASHED_MP_BNODE;
  }

  /* --- save a copy of the Pos data, then kill the Pos node --- */
  pos_copy = *pos_node;
  update_stats_for_destroying_node (thisAgent, pos_node);  /* clean up rete stats stuff */

  /* --- the old Pos node gets transmogrified into the new MP node --- */
  mp_node = pos_node;
  init_new_rete_node_with_type (thisAgent, mp_node, node_type);
  set_sharing_factor (mp_node, pos_copy.sharing_factor);
  mp_node->b.posneg = pos_copy.b.posneg;

  /* --- transfer the Mem node's tokens to the MP node --- */
  mp_node->a.np.tokens = mem_node->a.np.tokens;
  for (t=mem_node->a.np.tokens; t!=NIL; t=t->next_of_node) t->node = mp_node;
  mp_node->left_hash_loc_field_num = mem_node->left_hash_loc_field_num;
  mp_node->left_hash_loc_levels_up = mem_node->left_hash_loc_levels_up;
  mp_node->node_id = mem_node->node_id;

  /* --- replace the Mem node with the new MP node --- */
  mp_node->parent = parent;
  mp_node->next_sibling = parent->first_child;
  parent->first_child = mp_node;
  mp_node->first_child = pos_copy.first_child;

  remove_node_from_parents_list_of_children (mem_node);
  update_stats_for_destroying_node (thisAgent, mem_node);  /* clean up rete stats stuff */
  free_with_pool (&thisAgent->rete_node_pool, mem_node);

  /* --- set MP node's unlinking status according to pos_copy's --- */
  make_mp_bnode_left_linked (mp_node);
  if (node_is_left_unlinked(&pos_copy)) make_mp_bnode_left_unlinked (mp_node);

  return mp_node;
}

/* --------------------------------------------------------------------
                           Make New MP Node

   Make a new MP node, return a pointer to it.
-------------------------------------------------------------------- */

rete_node *make_new_mp_node (agent* thisAgent, 
                                                         rete_node *parent, byte node_type,
                             var_location left_hash_loc, alpha_mem *am,
                             rete_test *rt, Bool prefer_left_unlinking) {
  rete_node *mem_node, *pos_node;
  byte mem_node_type, pos_node_type;

  if (node_type==MP_BNODE) {
    pos_node_type = POSITIVE_BNODE;
    mem_node_type = MEMORY_BNODE;
  } else {
    pos_node_type = UNHASHED_POSITIVE_BNODE;
    mem_node_type = UNHASHED_MEMORY_BNODE;
  }
  mem_node = make_new_mem_node (thisAgent, parent, mem_node_type, left_hash_loc);
  pos_node = make_new_positive_node (thisAgent, mem_node, pos_node_type, am, rt,
                                     prefer_left_unlinking);
  return merge_into_mp_node (thisAgent, mem_node);
}

/* --------------------------------------------------------------------
                         Make New Negative Node

   Make a new negative node, return a pointer to it.
-------------------------------------------------------------------- */

rete_node *make_new_negative_node (agent* thisAgent, 
                                                                   rete_node *parent, byte node_type,
                                   var_location left_hash_loc,
                                   alpha_mem *am, rete_test *rt) {
  rete_node *node;

  allocate_with_pool (thisAgent, &thisAgent->rete_node_pool, &node);
  init_new_rete_node_with_type (thisAgent, node, node_type);
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
    nearest_ancestor_with_same_am (node, am);
  relink_to_right_mem (node);

  node->node_id = get_next_beta_node_id(thisAgent);

  /* --- call new node's add_left routine with all the parent's tokens --- */
  update_node_with_matches_from_above (thisAgent, node);
    
  /* --- if no tokens arrived from parent, unlink the node --- */
  if (! node->a.np.tokens) unlink_from_right_mem (node);
    
  return node;
}

/* --------------------------------------------------------------------
                          Make New CN Node

   Make new CN and CN_PARTNER nodes, return a pointer to the CN node.
-------------------------------------------------------------------- */

rete_node *make_new_cn_node (agent* thisAgent, 
                                                         rete_node *parent,
                             rete_node *bottom_of_subconditions) {
  rete_node *node, *partner, *ncc_subconditions_top_node;

  /* --- Find top node in the subconditions branch --- */
  ncc_subconditions_top_node = NIL; /* unneeded, but avoids gcc -Wall warn */
  for (node=bottom_of_subconditions; node!=parent; node=node->parent) {
    ncc_subconditions_top_node = node;
  }

  allocate_with_pool (thisAgent, &thisAgent->rete_node_pool, &node);
  init_new_rete_node_with_type (thisAgent, node, CN_BNODE);
  allocate_with_pool (thisAgent, &thisAgent->rete_node_pool, &partner);
  init_new_rete_node_with_type (thisAgent, partner, CN_PARTNER_BNODE);

  /* NOTE: for improved efficiency, <node> should be on the parent's
     children list *after* the ncc subcontitions top node */
  remove_node_from_parents_list_of_children (ncc_subconditions_top_node);
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
  update_node_with_matches_from_above (thisAgent, partner);
  /* --- call new node's add_left routine with all the parent's tokens --- */
  update_node_with_matches_from_above (thisAgent, node);

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

rete_node *make_new_production_node (agent* thisAgent, 
                                                                         rete_node *parent, production *new_prod) {
  rete_node *p_node;

  allocate_with_pool (thisAgent, &thisAgent->rete_node_pool, &p_node);
  init_new_rete_node_with_type (thisAgent, p_node, P_BNODE);
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

void deallocate_rete_test_list (agent* thisAgent, rete_test *rt) {
  rete_test *next_rt;

  while (rt) {
    next_rt = rt->next;

    if (test_is_constant_relational_test(rt->type)) {
      symbol_remove_ref (thisAgent, rt->data.constant_referent);
    } else if (rt->type==DISJUNCTION_RETE_TEST) {
      deallocate_symbol_list_removing_references (thisAgent, rt->data.disjunction_list);
    }

    free_with_pool (&thisAgent->rete_test_pool, rt);
    rt = next_rt;
  }
}
   
void deallocate_rete_node (agent* thisAgent, rete_node *node) {
  rete_node *parent;

  /* --- don't deallocate the dummy top node --- */
  if (node==thisAgent->dummy_top_node) return;

  /* --- sanity check --- */
  if (node->node_type==P_BNODE) {
    char msg[BUFFER_MSG_SIZE];
    strncpy (msg, "Internal error: deallocate_rete_node() called on p-node.\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
  }

  parent = node->parent;
  
  /* --- if a cn node, deallocate its partner first --- */
  if (node->node_type==CN_BNODE) deallocate_rete_node (thisAgent, node->b.cn.partner);

  /* --- clean up any tokens at the node --- */
  if (! bnode_is_bottom_of_split_mp(node->node_type))
    while (node->a.np.tokens) remove_token_and_subtree (thisAgent, node->a.np.tokens);

  /* --- stuff for posneg nodes only --- */
  if (bnode_is_posneg(node->node_type)) {
    deallocate_rete_test_list (thisAgent, node->b.posneg.other_tests);
    /* --- right unlink the node, cleanup alpha memory --- */
    if (! node_is_right_unlinked (node)) unlink_from_right_mem (node);
    remove_ref_to_alpha_mem (thisAgent, node->b.posneg.alpha_mem_);
  }

  /* --- remove the node from its parent's list --- */
  remove_node_from_parents_list_of_children (node);

  /* --- for unmerged pos. nodes: unlink, maybe merge its parent --- */
  if (bnode_is_bottom_of_split_mp(node->node_type)) {
    if (! node_is_left_unlinked(node)) unlink_from_left_mem (node);
    /* --- if parent is mem node with just one child, merge them --- */
    if (parent->first_child && (! parent->first_child->next_sibling)) {
       merge_into_mp_node (thisAgent, parent);
       parent = NIL;
    }
  }

  update_stats_for_destroying_node (thisAgent, node);  /* clean up rete stats stuff */
  free_with_pool (&thisAgent->rete_node_pool, node);

  /* --- if parent has no other children, deallocate it, and recurse  --- */
  /* Added check to make sure that parent wasn't deallocated in previous merge */
  if ( parent && !parent->first_child) deallocate_rete_node (thisAgent, parent);
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
   Var_is_bound() returns TRUE iff the given variable has been bound.
   Push_var_binding() pushes a new binding of the given variable.
   Pop_var_binding() pops the top binding.
********************************************************************** */

//#define var_is_bound(v) (((Symbol *)(v))->var.rete_binding_locations != NIL)
inline Bool var_is_bound(Symbol * v)
{
  return (((Symbol *)(v))->var.rete_binding_locations != NIL);
}

/*
#define varloc_to_dummy(depth,field_num) ((void *)(((depth)<<2) + (field_num)))
#define dummy_to_varloc_depth(d)     (((unsigned long)(d))>>2)
#define dummy_to_varloc_field_num(d) (((unsigned long)(d)) & 3)
*/

inline void * varloc_to_dummy(rete_node_level depth, byte field_num)
{
  return ((void *)(((depth)<<2) + (field_num)));
}

inline unsigned long dummy_to_varloc_depth(void * d)
{
  return (((unsigned long)(d))>>2);
}

inline unsigned long dummy_to_varloc_field_num(void * d)
{
  return (((unsigned long)(d)) & 3);
}

/*#define push_var_binding(v,depth,field_num) { \
  void *dummy_xy312; \
  dummy_xy312 = varloc_to_dummy ((depth), (field_num)); \
  push(thisAgent, dummy_xy312, ((Symbol *)(v))->var.rete_binding_locations); }*/
inline void push_var_binding(agent* thisAgent, Symbol * v, 
                                                         rete_node_level depth, byte field_num)
{
  void *dummy_xy312;
  dummy_xy312 = varloc_to_dummy ((depth), (field_num));
  push(thisAgent, dummy_xy312, ((Symbol *)(v))->var.rete_binding_locations);
}

/*#define pop_var_binding(v) { \
  cons *c_xy312; \
  c_xy312 = ((Symbol *)(v))->var.rete_binding_locations; \
  ((Symbol *)(v))->var.rete_binding_locations = c_xy312->rest; \
  free_cons (c_xy312); }*/
inline void pop_var_binding(agent* thisAgent, void * v)
{
  cons *c_xy312;
  c_xy312 = ((Symbol *)(v))->var.rete_binding_locations;
  ((Symbol *)(v))->var.rete_binding_locations = c_xy312->rest;
  free_cons (thisAgent, c_xy312);
}

/* -------------------------------------------------------------------
                          Find Var Location

   This routine finds the most recent place a variable was bound.
   It does this simply by looking at the top of the binding stack
   for that variable.  If there is any binding, its location is stored 
   in the parameter *result, and the function returns TRUE.  If no 
   binding is found, the function returns FALSE.
------------------------------------------------------------------- */

Bool find_var_location (Symbol *var, rete_node_level current_depth,
                        var_location *result) {
  void *dummy;
  if (! var->var.rete_binding_locations) return FALSE;
  dummy = var->var.rete_binding_locations->first;
  result->levels_up = current_depth - (rete_node_level)dummy_to_varloc_depth (dummy);
  result->field_num = (byte)dummy_to_varloc_field_num (dummy);
  return TRUE;
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

void bind_variables_in_test (agent* thisAgent, 
                                                         test t,
                             rete_node_level depth,
                             byte field_num,
                             Bool dense,
                             list **varlist) {
  Symbol *referent;
  complex_test *ct;
  cons *c;
  
  if (test_is_blank_test(t)) return;
  if (test_is_blank_or_equality_test(t)) {
    referent = referent_of_equality_test(t);
    if (referent->common.symbol_type!=VARIABLE_SYMBOL_TYPE) return;
    if (!dense && var_is_bound (referent)) return;
    push_var_binding (thisAgent, referent, depth, field_num);
    push(thisAgent, referent, *varlist);
    return;
  }

  ct = complex_test_from_test(t);
  if (ct->type==CONJUNCTIVE_TEST)
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      bind_variables_in_test (thisAgent, static_cast<char *>(c->first), 
                                                          depth, field_num, dense, varlist);
}

/* -------------------------------------------------------------------
             Pop Bindings and Deallocate List of Variables

   This routine takes a list of variables; for each item <v> on the
   list, it pops a binding of <v>.  It also deallocates the list.
   This is often used for un-binding a group of variables which got
   bound in some procedure.
------------------------------------------------------------------- */

void pop_bindings_and_deallocate_list_of_variables (agent* thisAgent, list *vars) {
  while (vars) {
    cons *c;
    c = vars;
    vars = vars->rest;
    pop_var_binding (thisAgent, c->first);
    free_cons (thisAgent, c);
  }
}













/* **********************************************************************

   SECTION 7:  Varnames and Node_Varnames

   Varnames and Node_Varnames (NVN) structures are used to record the names
   of variables bound (i.e., equality tested) at rete nodes.  The only
   purpose of saving this information is so we can reconstruct the 
   original source code for a production when we want to print it.  For
   chunks, we don't save any of this information -- we just re-gensym 
   the variable names on each printing (unless discard_chunk_varnames
   is set to FALSE).

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

typedef char varnames;

/*
#define one_var_to_varnames(x) ((varnames *) (x))
#define var_list_to_varnames(x) ((varnames *) (((char *)(x)) + 1))
#define varnames_is_one_var(x) (! (varnames_is_var_list(x)))
#define varnames_is_var_list(x) (((unsigned long)(x)) & 1)
#define varnames_to_one_var(x) ((Symbol *) (x))
#define varnames_to_var_list(x) ((list *) (((char *)(x)) - 1))
*/

inline varnames * one_var_to_varnames(Symbol * x) { return ((varnames *) (x)); }
inline varnames * var_list_to_varnames(cons * x) { return ((varnames *) (((char *)(x)) + 1)); }
inline unsigned long varnames_is_var_list(varnames * x) { return (((unsigned long)(x)) & 1); }
inline Bool varnames_is_one_var(varnames * x) { return (! (varnames_is_var_list(x))); }
inline Symbol * varnames_to_one_var(varnames * x) { return ((Symbol *) (x)); }
inline list * varnames_to_var_list(varnames * x) { return ((list *) (((char *)(x)) - 1)); }

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

varnames *add_var_to_varnames (agent* thisAgent, Symbol *var, 
                                                           varnames *old_varnames) {
  cons *c1, *c2;

  symbol_add_ref (var);
  if (old_varnames == NIL)
    return one_var_to_varnames(var);
  if (varnames_is_one_var(old_varnames)) {
    allocate_cons (thisAgent, &c1);
    allocate_cons (thisAgent, &c2);
    c1->first = var;
    c1->rest = c2;
    c2->first = varnames_to_one_var(old_varnames);
    c2->rest = NIL;
    return var_list_to_varnames(c1);
  }
  /* --- otherwise old_varnames is a list --- */
  allocate_cons (thisAgent, &c1);
  c1->first = var;
  c1->rest = varnames_to_var_list(old_varnames);
  return var_list_to_varnames(c1);
}

void deallocate_varnames (agent* thisAgent, varnames *vn) {
  Symbol *sym;
  list *symlist;

  if (vn == NIL) return;
  if (varnames_is_one_var(vn)) {
    sym = varnames_to_one_var(vn);
    symbol_remove_ref (thisAgent, sym);
  } else {
    symlist = varnames_to_var_list(vn);
    deallocate_symbol_list_removing_references (thisAgent, symlist);
  }
}

void deallocate_node_varnames (agent* thisAgent, 
                                                           rete_node *node, rete_node *cutoff,
                               node_varnames *nvn) {
  node_varnames *temp;

  while (node!=cutoff) {
    if (node->node_type==CN_BNODE) {
      deallocate_node_varnames (thisAgent, node->b.cn.partner->parent, node->parent,
                                nvn->data.bottom_of_subconditions);
    } else {
      deallocate_varnames (thisAgent, nvn->data.fields.id_varnames);
      deallocate_varnames (thisAgent, nvn->data.fields.attr_varnames);
      deallocate_varnames (thisAgent, nvn->data.fields.value_varnames);
    }
    node = real_parent_node (node);
    temp = nvn;
    nvn = nvn->parent;
    free_with_pool (&thisAgent->node_varnames_pool, temp);
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

varnames *add_unbound_varnames_in_test (agent* thisAgent, test t, 
                                                                                varnames *starting_vn) {
  cons *c;
  Symbol *referent;
  complex_test *ct;

  if (test_is_blank_test(t)) return starting_vn;
  if (test_is_blank_or_equality_test(t)) {
    referent = referent_of_equality_test(t);
    if (referent->common.symbol_type==VARIABLE_SYMBOL_TYPE)
      if (! var_is_bound (referent))
        starting_vn = add_var_to_varnames (thisAgent, referent, starting_vn);
    return starting_vn;
  }

  ct = complex_test_from_test(t);
  
  if (ct->type==CONJUNCTIVE_TEST) {
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      starting_vn = add_unbound_varnames_in_test (thisAgent, static_cast<char *>(c->first), 
                                                                                                  starting_vn);
  }
  return starting_vn;
}

node_varnames *make_nvn_for_posneg_cond (agent* thisAgent, 
                                                                                 condition *cond,
                                         node_varnames *parent_nvn) {
  node_varnames *New;
  list *vars_bound;

  vars_bound = NIL;

  allocate_with_pool (thisAgent, &thisAgent->node_varnames_pool, &New);
  New->parent = parent_nvn;

  /* --- fill in varnames for id test --- */
  New->data.fields.id_varnames =
    add_unbound_varnames_in_test (thisAgent, cond->data.tests.id_test, NIL);

  /* --- add sparse bindings for id, then get attr field varnames --- */
  bind_variables_in_test (thisAgent, cond->data.tests.id_test, 0, 0, FALSE, &vars_bound);
  New->data.fields.attr_varnames =
    add_unbound_varnames_in_test (thisAgent, cond->data.tests.attr_test, NIL);

  /* --- add sparse bindings for attr, then get value field varnames --- */
  bind_variables_in_test (thisAgent, cond->data.tests.attr_test, 0, 0, FALSE,&vars_bound);
  New->data.fields.value_varnames =
    add_unbound_varnames_in_test (thisAgent, cond->data.tests.value_test, NIL);

  /* --- Pop the variable bindings for these conditions --- */
  pop_bindings_and_deallocate_list_of_variables (thisAgent, vars_bound);

  return New;
}  

node_varnames *get_nvn_for_condition_list (agent* thisAgent, 
                                                                                   condition *cond_list,
                                           node_varnames *parent_nvn) {
  node_varnames *New;
  condition *cond;
  list *vars;
  
  vars = NIL;
  
  for (cond=cond_list; cond!=NIL; cond=cond->next) {

    switch (cond->type) {
    case POSITIVE_CONDITION:
      New = make_nvn_for_posneg_cond (thisAgent, cond, parent_nvn);

      /* --- Add sparse variable bindings for this condition --- */
      bind_variables_in_test (thisAgent, cond->data.tests.id_test, 0, 0, FALSE, &vars);
      bind_variables_in_test (thisAgent, cond->data.tests.attr_test, 0, 0, FALSE, &vars);
      bind_variables_in_test (thisAgent, cond->data.tests.value_test, 0, 0, FALSE, &vars);
      break;
    case NEGATIVE_CONDITION:
      New = make_nvn_for_posneg_cond (thisAgent, cond, parent_nvn);
      break;
    case CONJUNCTIVE_NEGATION_CONDITION:
      allocate_with_pool (thisAgent, &thisAgent->node_varnames_pool, &New);
      New->parent = parent_nvn;
      New->data.bottom_of_subconditions =
        get_nvn_for_condition_list (thisAgent, cond->data.ncc.top, parent_nvn);
      break;
    }
    
    parent_nvn = New;
  }

  /* --- Pop the variable bindings for these conditions --- */
  pop_bindings_and_deallocate_list_of_variables (thisAgent, vars);

  return parent_nvn;  
}
 


















/* **********************************************************************

   SECTION 8:  Building the Rete Net:  Condition-To-Node Converstion

   Build_network_for_condition_list() is the key routine here. (See
   description below.)
********************************************************************** */


/* ---------------------------------------------------------------------

       Test Type <---> Relational (Rete) Test Type Conversion Tables

   These tables convert from xxx_TEST's (defined in soarkernel.h for various
   kinds of complex_test's) to xxx_RETE_TEST's (defined in rete.cpp for
   the different kinds of Rete tests), and vice-versa.  We might just 
   use the same set of constants for both purposes, but we want to be
   able to do bit-twiddling on the RETE_TEST types.

   (This stuff probably doesn't belong under "Building the Rete Net",
   but I wasn't sure where else to put it.)
--------------------------------------------------------------------- */

//
// 255 == ERROR_TEST_TYPE.  I use 255 here for brevity.
//
byte test_type_to_relational_test_type[256] =
{
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

//
// 255 == ERROR_TEST_TYPE.  I use 255 here for brevity.
//
byte relational_test_type_to_test_type[256] =
{
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

/* Warning: the two items below must not be the same as any xxx_TEST's defined
   in soarkernel.h for the types of complex_test's */
#define EQUAL_TEST_TYPE 254 
#define ERROR_TEST_TYPE 255

void init_test_type_conversion_tables (void) 
{
  /* This is to avoid multiple initializations. (This may not yet thread-safe.) */
  static bool bInit = FALSE;
  if (bInit)
          return;
  bInit = TRUE;

  /* we don't need ...[equal test] */
  test_type_to_relational_test_type[NOT_EQUAL_TEST] =   RELATIONAL_NOT_EQUAL_RETE_TEST;
  test_type_to_relational_test_type[LESS_TEST] =    RELATIONAL_LESS_RETE_TEST;
  test_type_to_relational_test_type[GREATER_TEST] =    RELATIONAL_GREATER_RETE_TEST;
  test_type_to_relational_test_type[LESS_OR_EQUAL_TEST] =    RELATIONAL_LESS_OR_EQUAL_RETE_TEST;
  test_type_to_relational_test_type[GREATER_OR_EQUAL_TEST] =    RELATIONAL_GREATER_OR_EQUAL_RETE_TEST;
  test_type_to_relational_test_type[SAME_TYPE_TEST] =    RELATIONAL_SAME_TYPE_RETE_TEST;

  relational_test_type_to_test_type[RELATIONAL_EQUAL_RETE_TEST] =    EQUAL_TEST_TYPE;
  relational_test_type_to_test_type[RELATIONAL_NOT_EQUAL_RETE_TEST] =    NOT_EQUAL_TEST;
  relational_test_type_to_test_type[RELATIONAL_LESS_RETE_TEST] =    LESS_TEST;
  relational_test_type_to_test_type[RELATIONAL_GREATER_RETE_TEST] =    GREATER_TEST;
  relational_test_type_to_test_type[RELATIONAL_LESS_OR_EQUAL_RETE_TEST] =    LESS_OR_EQUAL_TEST;
  relational_test_type_to_test_type[RELATIONAL_GREATER_OR_EQUAL_RETE_TEST] =    GREATER_OR_EQUAL_TEST;
  relational_test_type_to_test_type[RELATIONAL_SAME_TYPE_RETE_TEST] =    SAME_TYPE_TEST;
}

/* ------------------------------------------------------------------------
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

void add_rete_tests_for_test (agent* thisAgent, test t,
                              rete_node_level current_depth,
                              byte field_num,
                              rete_test **rt,
                              Symbol **alpha_constant) {
  var_location where;
  cons *c;
  rete_test *new_rt;
  complex_test *ct;
  Symbol *referent;

  if (test_is_blank_test(t)) return;

  if (test_is_blank_or_equality_test(t)) {
    referent = referent_of_equality_test(t);
    
    /* --- if constant test and alpha=NIL, install alpha test --- */
    if ((referent->common.symbol_type!=VARIABLE_SYMBOL_TYPE) &&
        (*alpha_constant==NIL)) {
      *alpha_constant = referent;
      return;
    }
   
    /* --- if constant, make = constant test --- */
    if (referent->common.symbol_type!=VARIABLE_SYMBOL_TYPE) {
      allocate_with_pool (thisAgent, &thisAgent->rete_test_pool, &new_rt);
      new_rt->right_field_num = field_num;
      new_rt->type = CONSTANT_RELATIONAL_RETE_TEST+RELATIONAL_EQUAL_RETE_TEST;
      new_rt->data.constant_referent = referent;
      symbol_add_ref (referent);
      new_rt->next = *rt;
      *rt = new_rt;
      return;
    }

    /* --- variable: if binding is for current field, do nothing --- */
    if (! find_var_location (referent, current_depth, &where)) {
      char msg[BUFFER_MSG_SIZE];
      print_with_symbols (thisAgent, "Error: Rete build found test of unbound var: %y\n",
                          referent);
      snprintf (msg, BUFFER_MSG_SIZE, "Error: Rete build found test of unbound var: %s\n",
                          symbol_to_string(thisAgent, referent,TRUE, NIL, 0));
     msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
     abort_with_fatal_error(thisAgent, msg);
    }
    if ((where.levels_up==0) && (where.field_num==field_num)) return;

    /* --- else make variable equality test --- */
    allocate_with_pool (thisAgent, &thisAgent->rete_test_pool, &new_rt);
    new_rt->right_field_num = field_num;
    new_rt->type = VARIABLE_RELATIONAL_RETE_TEST+RELATIONAL_EQUAL_RETE_TEST;
    new_rt->data.variable_referent = where;
    new_rt->next = *rt;
    *rt = new_rt;
    return;
  }

  ct = complex_test_from_test(t);

  switch (ct->type) {    
   
  case NOT_EQUAL_TEST:
  case LESS_TEST:
  case GREATER_TEST:
  case LESS_OR_EQUAL_TEST:
  case GREATER_OR_EQUAL_TEST:
  case SAME_TYPE_TEST:
    /* --- if constant, make constant test --- */
    if (ct->data.referent->common.symbol_type!=VARIABLE_SYMBOL_TYPE) {
      allocate_with_pool (thisAgent, &thisAgent->rete_test_pool, &new_rt);
      new_rt->right_field_num = field_num;
      new_rt->type = CONSTANT_RELATIONAL_RETE_TEST +
                     test_type_to_relational_test_type[ct->type];
      new_rt->data.constant_referent = ct->data.referent;
      symbol_add_ref (ct->data.referent);
      new_rt->next = *rt;
      *rt = new_rt;
      return;
    }
    /* --- else make variable test --- */   
    if (! find_var_location (ct->data.referent, current_depth, &where)) {
      char msg[BUFFER_MSG_SIZE];
      print_with_symbols (thisAgent, "Error: Rete build found test of unbound var: %y\n",
                          ct->data.referent);
      snprintf (msg, BUFFER_MSG_SIZE, "Error: Rete build found test of unbound var: %s\n",
                          symbol_to_string(thisAgent, ct->data.referent,TRUE, NIL, 0));
      msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
      abort_with_fatal_error(thisAgent, msg);
    }
    allocate_with_pool (thisAgent, &thisAgent->rete_test_pool, &new_rt);
    new_rt->right_field_num = field_num;
    new_rt->type = VARIABLE_RELATIONAL_RETE_TEST +
                   test_type_to_relational_test_type[ct->type];
    new_rt->data.variable_referent = where;
    new_rt->next = *rt;
    *rt = new_rt;
    return;

  case DISJUNCTION_TEST:
    allocate_with_pool (thisAgent, &thisAgent->rete_test_pool, &new_rt);
    new_rt->right_field_num = field_num;
    new_rt->type = DISJUNCTION_RETE_TEST;
    new_rt->data.disjunction_list =
      copy_symbol_list_adding_references (thisAgent, ct->data.disjunction_list);
    new_rt->next = *rt;
    *rt = new_rt;
    return;
   
  case CONJUNCTIVE_TEST:
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest) {
      add_rete_tests_for_test (thisAgent, static_cast<char *>(c->first), 
                                                           current_depth, field_num, rt, alpha_constant);
    }
    return;

  case GOAL_ID_TEST:
    allocate_with_pool (thisAgent, &thisAgent->rete_test_pool, &new_rt);
    new_rt->type = ID_IS_GOAL_RETE_TEST;
    new_rt->right_field_num = 0;
    new_rt->next = *rt;
    *rt = new_rt;
    return;

  case IMPASSE_ID_TEST:
    allocate_with_pool (thisAgent, &thisAgent->rete_test_pool, &new_rt);
    new_rt->type = ID_IS_IMPASSE_RETE_TEST;
    new_rt->right_field_num = 0;
    new_rt->next = *rt;
    *rt = new_rt;
    return;
    
  default:
    { char msg[BUFFER_MSG_SIZE];
    snprintf (msg, BUFFER_MSG_SIZE,"Error: found bad test type %d while building rete\n",
           ct->type);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
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

Bool single_rete_tests_are_identical (agent* thisAgent, rete_test *rt1, rete_test *rt2) {
  cons *c1, *c2;
 
  if (rt1->type != rt2->type) return FALSE;

  if (rt1->right_field_num != rt2->right_field_num) return FALSE;

  if (test_is_variable_relational_test(rt1->type))
    return (var_locations_equal (rt1->data.variable_referent,
                                 rt2->data.variable_referent));

  if (test_is_constant_relational_test(rt1->type)) {
    return (rt1->data.constant_referent == rt2->data.constant_referent);
  }

  if (rt1->type==ID_IS_GOAL_RETE_TEST) return TRUE;
  if (rt1->type==ID_IS_IMPASSE_RETE_TEST) return TRUE;

  if (rt1->type == DISJUNCTION_RETE_TEST) {
    c1 = rt1->data.disjunction_list;
    c2 = rt2->data.disjunction_list;
    while ((c1!=NIL)&&(c2!=NIL)) {
      if (c1->first != c2->first) return FALSE;
      c1 = c1->rest;
      c2 = c2->rest;
    }
    if (c1==c2) return TRUE;
    return FALSE;
  }
  { char msg[BUFFER_MSG_SIZE];
  strncpy(msg,"Internal error: bad rete test type in single_rete_tests_are_identical\n", BUFFER_MSG_SIZE);
  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
  abort_with_fatal_error(thisAgent, msg);
  }
  return FALSE; /* unreachable, but without it, gcc -Wall warns here */
}

Bool rete_test_lists_are_identical (agent* thisAgent, rete_test *rt1, rete_test *rt2) {
  while (rt1 && rt2) {
    if (! single_rete_tests_are_identical(thisAgent, rt1,rt2)) 
                return FALSE;
    rt1 = rt1->next;
    rt2 = rt2->next;
  }
  if (rt1==rt2) return TRUE;  /* make sure they both hit end-of-list */
  return FALSE;
}

/* ------------------------------------------------------------------------
                      Extract Rete Test to Hash With

   Extracts from a Rete test list the variable equality test to use for
   hashing.  Returns TRUE if successful, or FALSE if there was no such
   test to use for hashing.  The Rete test list ("rt") is destructively
   modified to splice out the extracted test.
------------------------------------------------------------------------ */

Bool extract_rete_test_to_hash_with (agent* thisAgent, 
                                                                         rete_test **rt,
                                     var_location *dest_hash_loc) {
  rete_test *prev, *current;
 
  /* --- look through rt list, find the first variable equality test --- */
  prev = NIL;
  for (current = *rt; current!=NIL; prev=current, current=current->next)
    if (current->type==VARIABLE_RELATIONAL_RETE_TEST +
                       RELATIONAL_EQUAL_RETE_TEST) break;
 
  if (!current) return FALSE;  /* no variable equality test was found */

  /* --- unlink it from rt --- */
  if (prev) prev->next = current->next; else *rt = current->next;

  /* --- extract info, and deallocate that single test --- */
  *dest_hash_loc = current->data.variable_referent;
  current->next = NIL;
  deallocate_rete_test_list (thisAgent, current);
  return TRUE;
}

/* ------------------------------------------------------------------------
                       Make Node for Positive Cond

   Finds or creates a node for the given single condition <cond>, which
   must be a simple positive condition.  The node is made a child of the
   given <parent> node.  Variables for earlier conditions should be bound
   densely before this routine is called.  The routine returns a pointer 
   to the (newly-created or shared) node.
------------------------------------------------------------------------ */

rete_node *make_node_for_positive_cond (agent* thisAgent, 
                                                                                condition *cond,
                                        rete_node_level current_depth,
                                        rete_node *parent) {
  byte pos_node_type, mem_node_type, mp_node_type;
  Symbol *alpha_id, *alpha_attr, *alpha_value;
  rete_node *node, *mem_node, *mp_node;
  alpha_mem *am;
  rete_test *rt;
  Bool hash_this_node;
  var_location left_hash_loc;
  list *vars_bound_here;
  
  alpha_id = alpha_attr = alpha_value = NIL;
  rt = NIL;
  vars_bound_here = NIL;

  /* --- Add sparse variable bindings for this condition --- */
  bind_variables_in_test (thisAgent, cond->data.tests.id_test, current_depth, 0,
                          FALSE, &vars_bound_here);
  bind_variables_in_test (thisAgent, cond->data.tests.attr_test, current_depth, 1,
                          FALSE, &vars_bound_here);
  bind_variables_in_test (thisAgent, cond->data.tests.value_test, current_depth, 2,
                          FALSE, &vars_bound_here);

  /* --- Get Rete tests, alpha constants, and hash location --- */
  add_rete_tests_for_test (thisAgent, cond->data.tests.id_test, current_depth, 0,
                           &rt, &alpha_id);
  hash_this_node = extract_rete_test_to_hash_with (thisAgent, &rt, &left_hash_loc);
  add_rete_tests_for_test (thisAgent, cond->data.tests.attr_test, current_depth, 1,
                           &rt, &alpha_attr);
  add_rete_tests_for_test (thisAgent, cond->data.tests.value_test, current_depth, 2,
                           &rt, &alpha_value);

  /* --- Pop sparse variable bindings for this condition --- */
  pop_bindings_and_deallocate_list_of_variables (thisAgent, vars_bound_here);

  /* --- Get alpha memory --- */
  am = find_or_make_alpha_mem (thisAgent, alpha_id, alpha_attr, alpha_value,
                     cond->test_for_acceptable_preference);

  /* --- Algorithm for adding node:
          1.  look for matching mem node; if found then
                look for matching join node; create new one if no match
          2.  no matching mem node:  look for mp node with matching mem
                if found, if join part matches too, then done
                          else delete mp node, create mem node and 2 joins
                if not matching mem node, create new mp node. */

  /* --- determine desired node types --- */
  if (hash_this_node) {
    pos_node_type = POSITIVE_BNODE;
    mem_node_type = MEMORY_BNODE;
    mp_node_type = MP_BNODE;
  } else {
    pos_node_type = UNHASHED_POSITIVE_BNODE;
    mem_node_type = UNHASHED_MEMORY_BNODE;
    mp_node_type = UNHASHED_MP_BNODE;
  }

  /* --- look for a matching existing memory node --- */
  for (mem_node=parent->first_child; mem_node!=NIL;
       mem_node=mem_node->next_sibling)
    if ((mem_node->node_type==mem_node_type) &&
        ((!hash_this_node) ||
         ((mem_node->left_hash_loc_field_num==left_hash_loc.field_num) &&
          (mem_node->left_hash_loc_levels_up==left_hash_loc.levels_up))))
      break;

  if (mem_node) {   /* -- A matching memory node was found --- */
    /* --- look for a matching existing join node --- */
    for (node=mem_node->first_child; node!=NIL; node=node->next_sibling)
      if ((node->node_type==pos_node_type) &&
          (am == node->b.posneg.alpha_mem_) &&
          rete_test_lists_are_identical (thisAgent, node->b.posneg.other_tests, rt))
        break;
    
    if (node) {    /* --- A matching join node was found --- */
      deallocate_rete_test_list (thisAgent, rt);
      remove_ref_to_alpha_mem (thisAgent, am);
      return node;
    } else {       /* --- No match was found, so create a new node --- */
      node = make_new_positive_node (thisAgent, mem_node, pos_node_type, am ,rt, FALSE);
      return node;
    }
  }
 
  /* --- No matching memory node was found; look for MP with matching M --- */
  for (mp_node=parent->first_child; mp_node!=NIL;
       mp_node=mp_node->next_sibling)
    if ((mp_node->node_type==mp_node_type) &&
        ((!hash_this_node) ||
         ((mp_node->left_hash_loc_field_num==left_hash_loc.field_num) &&
          (mp_node->left_hash_loc_levels_up==left_hash_loc.levels_up))))
      break;

  if (mp_node) {  /* --- Found matching M part of MP --- */
    if ((am == mp_node->b.posneg.alpha_mem_) &&
        rete_test_lists_are_identical (thisAgent, mp_node->b.posneg.other_tests, rt)) {
      /* --- Complete MP match was found --- */
      deallocate_rete_test_list (thisAgent, rt);
      remove_ref_to_alpha_mem (thisAgent, am);
      return mp_node;
    }

    /* --- Delete MP node, replace it with M and two positive joins --- */
    mem_node = split_mp_node (thisAgent, mp_node);
    node = make_new_positive_node (thisAgent, mem_node, pos_node_type, am, rt, FALSE);
    return node;
  }

  /* --- Didn't even find a matching M part of MP, so make a new MP node --- */
  return make_new_mp_node (thisAgent, parent, mp_node_type, left_hash_loc, am, rt, FALSE);
}  

/* ------------------------------------------------------------------------
                       Make Node for Negative Cond

   Finds or creates a node for the given single condition <cond>, which
   must be a simple negative (not ncc) condition.  The node is made a
   child of the given <parent> node.  Variables for earlier conditions 
   should be bound densely before this routine is called.  The routine 
   returns a pointer to the (newly-created or shared) node.
------------------------------------------------------------------------ */
                                          
rete_node *make_node_for_negative_cond (agent* thisAgent, 
                                                                                condition *cond,
                                        rete_node_level current_depth,
                                        rete_node *parent) {
  byte node_type;
  Symbol *alpha_id, *alpha_attr, *alpha_value;
  rete_node *node;
  alpha_mem *am;
  rete_test *rt;
  Bool hash_this_node;
  var_location left_hash_loc;
  list *vars_bound_here;
  
  alpha_id = alpha_attr = alpha_value = NIL;
  rt = NIL;
  vars_bound_here = NIL;

  /* --- Add sparse variable bindings for this condition --- */
  bind_variables_in_test (thisAgent, cond->data.tests.id_test, current_depth, 0,
                          FALSE, &vars_bound_here);
  bind_variables_in_test (thisAgent, cond->data.tests.attr_test, current_depth, 1,
                          FALSE, &vars_bound_here);
  bind_variables_in_test (thisAgent, cond->data.tests.value_test, current_depth, 2,
                          FALSE, &vars_bound_here);

  /* --- Get Rete tests, alpha constants, and hash location --- */
  add_rete_tests_for_test (thisAgent, cond->data.tests.id_test, current_depth, 0,
                           &rt, &alpha_id);
  hash_this_node = extract_rete_test_to_hash_with (thisAgent, &rt, &left_hash_loc);
  add_rete_tests_for_test (thisAgent, cond->data.tests.attr_test, current_depth, 1,
                           &rt, &alpha_attr);
  add_rete_tests_for_test (thisAgent, cond->data.tests.value_test, current_depth, 2,
                           &rt, &alpha_value);

  /* --- Pop sparse variable bindings for this condition --- */
  pop_bindings_and_deallocate_list_of_variables (thisAgent, vars_bound_here);

  /* --- Get alpha memory --- */
  am = find_or_make_alpha_mem (thisAgent, alpha_id, alpha_attr, alpha_value,
                     cond->test_for_acceptable_preference);

  /* --- determine desired node type --- */
  node_type = hash_this_node ? NEGATIVE_BNODE : UNHASHED_NEGATIVE_BNODE;

  /* --- look for a matching existing node --- */
  for (node=parent->first_child; node!=NIL; node=node->next_sibling)
    if ((node->node_type==node_type) &&
        (am == node->b.posneg.alpha_mem_) &&
        ((!hash_this_node) ||
         ((node->left_hash_loc_field_num==left_hash_loc.field_num) &&
          (node->left_hash_loc_levels_up==left_hash_loc.levels_up))) &&
        rete_test_lists_are_identical (thisAgent, node->b.posneg.other_tests, rt)) break;

  if (node) {    /* --- A matching node was found --- */
    deallocate_rete_test_list (thisAgent, rt);
    remove_ref_to_alpha_mem (thisAgent, am);
    return node;
  } else {       /* --- No match was found, so create a new node --- */
    node = make_new_negative_node (thisAgent, parent, node_type, left_hash_loc, am, rt);
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

void build_network_for_condition_list (agent* thisAgent, 
                                                                           condition *cond_list,
                                       rete_node_level depth_of_first_cond,
                                       rete_node *parent,
                                       rete_node **dest_bottom_node,
                                       rete_node_level *dest_bottom_depth,
                                       list **dest_vars_bound) {
  rete_node *node, *new_node, *child, *subconditions_bottom_node;
  condition *cond;
  rete_node_level current_depth;
  list *vars_bound;
  
  node = parent;
  current_depth = depth_of_first_cond;
  vars_bound = NIL;
  
  for (cond=cond_list; cond!=NIL; cond=cond->next) {
    switch (cond->type) {

    case POSITIVE_CONDITION:
      new_node = make_node_for_positive_cond (thisAgent, cond, current_depth, node);
      /* --- Add dense variable bindings for this condition --- */
      bind_variables_in_test (thisAgent, cond->data.tests.id_test, current_depth, 0,
                              TRUE, &vars_bound);
      bind_variables_in_test (thisAgent, cond->data.tests.attr_test, current_depth, 1,
                              TRUE, &vars_bound);
      bind_variables_in_test (thisAgent, cond->data.tests.value_test, current_depth, 2,
                              TRUE, &vars_bound);
      break;

    case NEGATIVE_CONDITION:
      new_node = make_node_for_negative_cond (thisAgent, cond, current_depth, node);
      break;

    case CONJUNCTIVE_NEGATION_CONDITION:
      /* --- first, make the subconditions part of the rete --- */
      build_network_for_condition_list (thisAgent, cond->data.ncc.top, current_depth,
                   node, &subconditions_bottom_node, NIL, NIL);
      /* --- look for an existing CN node --- */
      for (child=node->first_child; child!=NIL; child=child->next_sibling)
        if (child->node_type==CN_BNODE)
          if (child->b.cn.partner->parent==subconditions_bottom_node) break;
      /* --- share existing node or build new one --- */
      if (child) {
        new_node = child;
      } else {
        new_node = make_new_cn_node (thisAgent, node, subconditions_bottom_node);
      }
      break;
   
    default:
      new_node = NIL; /* unreachable, but without it gcc -Wall warns here */
    }
    
    node = new_node;
    current_depth++;
  }
  
  /* --- return results to caller --- */
  if (dest_bottom_node) *dest_bottom_node = node;
  if (dest_bottom_depth) *dest_bottom_depth = current_depth-1;
  if (dest_vars_bound) {
    *dest_vars_bound = vars_bound;
  } else {
    pop_bindings_and_deallocate_list_of_variables (thisAgent, vars_bound);
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

Bool same_rhs (action *rhs1, action *rhs2) {
  action *a1, *a2;
  
  /* --- Scan through the two RHS's; make sure there's no function calls,
     and make sure the actions are all the same. --- */
  /* --- Warning: this relies on the representation of rhs_value's:
     two of the same funcall will not be equal (==), but two of the
     same symbol, reteloc, or unboundvar will be equal (==). --- */

  a1 = rhs1;
  a2 = rhs2;

  while (a1 && a2) {
    if (a1->type == FUNCALL_ACTION) return FALSE;
    if (a2->type == FUNCALL_ACTION) return FALSE;
    if (a1->preference_type != a2->preference_type) return FALSE;
    if (a1->id != a2->id) return FALSE;
    if (a1->attr != a2->attr) return FALSE;
    if (a1->value != a2->value) return FALSE;
    if (preference_is_binary(a1->preference_type))
      if (a1->referent != a2->referent) return FALSE;
    a1 = a1->next;
    a2 = a2->next;
  }

  /* --- If we reached the end of one RHS but not the other, then
     they must be different --- */
  if (a1 != a2) return FALSE;

  /* --- If we got this far, the RHS's must be identical. --- */
  return TRUE;
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


void fixup_rhs_value_variable_references (agent* thisAgent, rhs_value *rv,
                                          rete_node_level bottom_depth, 
                                          list * & rhs_unbound_vars_for_new_prod, 
                                          unsigned long & num_rhs_unbound_vars_for_new_prod, 
                                          tc_number rhs_unbound_vars_tc)
{
  cons *c;
  Symbol *sym;
  var_location var_loc;
  unsigned long index;
  
  if (rhs_value_is_symbol(*rv)) {
    sym = rhs_value_to_symbol (*rv);
    if (sym->common.symbol_type!=VARIABLE_SYMBOL_TYPE) return;
    /* --- Found a variable.  Is is bound on the LHS? --- */
    if (find_var_location (sym, (rete_node_level)(bottom_depth+1), &var_loc)) {
      /* --- Yes, replace it with reteloc --- */
      symbol_remove_ref (thisAgent, sym);
      *rv = reteloc_to_rhs_value (var_loc.field_num, var_loc.levels_up-1);
    } else {
      /* --- No, replace it with rhs_unboundvar --- */
      if (sym->var.tc_num != rhs_unbound_vars_tc) {
        symbol_add_ref (sym);
        push(thisAgent, sym, rhs_unbound_vars_for_new_prod);
        sym->var.tc_num = rhs_unbound_vars_tc;
        index = num_rhs_unbound_vars_for_new_prod++;
        sym->var.current_binding_value = (Symbol *)index;
      } else {
        index = (unsigned long)(sym->var.current_binding_value);
      }
      *rv = unboundvar_to_rhs_value (index);
      symbol_remove_ref (thisAgent, sym);
    }
    return;
  }

  if (rhs_value_is_funcall(*rv)) {
    for (c=rhs_value_to_funcall_list(*rv)->rest; c!=NIL; c=c->rest)
      fixup_rhs_value_variable_references (thisAgent, (rhs_value *)(&(c->first)),
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

void update_max_rhs_unbound_variables (agent* thisAgent, 
                                                                           unsigned long num_for_new_production) {
  if (num_for_new_production > thisAgent->max_rhs_unbound_variables) {
    free_memory (thisAgent, thisAgent->rhs_variable_bindings,MISCELLANEOUS_MEM_USAGE);
    thisAgent->max_rhs_unbound_variables = num_for_new_production;
    thisAgent->rhs_variable_bindings = (Symbol **)
      allocate_memory_and_zerofill (thisAgent, thisAgent->max_rhs_unbound_variables *
                                   sizeof(Symbol *), MISCELLANEOUS_MEM_USAGE);
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

byte add_production_to_rete (agent* thisAgent, 
                                                         production *p,
                             condition *lhs_top,
                             instantiation *refracted_inst,
                             Bool warn_on_duplicates) 
{
  rete_node *bottom_node, *p_node;
  rete_node_level bottom_depth;
  list *vars_bound;
  ms_change *msc;
  action *a;
  byte production_addition_result;

  /* JC ADDED: tell gSKI we're about to add a production */
  gSKI_MakeAgentCallback(gSKI_K_EVENT_PRODUCTION_ADDED, 0, thisAgent, static_cast<void*>(p));

  /* --- build the network for all the conditions --- */
  build_network_for_condition_list (thisAgent, lhs_top, 1, thisAgent->dummy_top_node, 
                      &bottom_node, &bottom_depth, &vars_bound);

  /* --- change variable names in RHS to Rete location references or
     unbound variable indices --- */
  list* rhs_unbound_vars_for_new_prod = NIL;
  unsigned long num_rhs_unbound_vars_for_new_prod = 0;
  tc_number rhs_unbound_vars_tc = get_new_tc_number(thisAgent);
  for (a=p->action_list; a!=NIL; a=a->next) {
    fixup_rhs_value_variable_references (thisAgent, &(a->value), bottom_depth, 
                rhs_unbound_vars_for_new_prod, num_rhs_unbound_vars_for_new_prod, rhs_unbound_vars_tc);
    if (a->type==MAKE_ACTION) {
      fixup_rhs_value_variable_references (thisAgent, &(a->id), bottom_depth, 
                  rhs_unbound_vars_for_new_prod, num_rhs_unbound_vars_for_new_prod, rhs_unbound_vars_tc);
      fixup_rhs_value_variable_references (thisAgent, &(a->attr), bottom_depth, 
                  rhs_unbound_vars_for_new_prod, num_rhs_unbound_vars_for_new_prod, rhs_unbound_vars_tc);
      if (preference_is_binary(a->preference_type))
        fixup_rhs_value_variable_references (thisAgent, &(a->referent), bottom_depth, 
                        rhs_unbound_vars_for_new_prod, num_rhs_unbound_vars_for_new_prod, rhs_unbound_vars_tc);
    }
  }
  
  /* --- clean up variable bindings created by build_network...() --- */
  pop_bindings_and_deallocate_list_of_variables (thisAgent, vars_bound);

  update_max_rhs_unbound_variables (thisAgent, num_rhs_unbound_vars_for_new_prod);

  /* --- look for an existing p node that matches --- */
  for (p_node=bottom_node->first_child; p_node!=NIL;
       p_node=p_node->next_sibling) {
    if (p_node->node_type != P_BNODE) continue;
    if (! same_rhs (p_node->b.p.prod->action_list, p->action_list)) continue;
    /* --- duplicate production found --- */
    if (warn_on_duplicates)
      print_with_symbols (thisAgent, "\nIgnoring %y because it is a duplicate of %y ",
                          p->name, p_node->b.p.prod->name);
    deallocate_symbol_list_removing_references (thisAgent, rhs_unbound_vars_for_new_prod);
    return DUPLICATE_PRODUCTION;
  }

  /* --- build a new p node --- */
  p_node = make_new_production_node (thisAgent, bottom_node, p);
  adjust_sharing_factors_from_here_to_top (p_node, 1);


  /* KJC 1/28/98  left these comments in to support REW comments below
     but commented out the operand_mode code  */
  /* RCHONG: begin 10.11 */
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

  if ((thisAgent->operand_mode == TRUE) && 1)
     if (refracted_inst != NIL) {
        if (refracted_inst->prod->type != JUSTIFICATION_PRODUCTION_TYPE)
           refracted_inst = NIL;
     }
  */
  /* RCHONG: end 10.11 */

  /* REW: begin 09.15.96 */
  /* In Operand2, for now, we want both chunks and justifications to be
     treated as refracted instantiations, at least for now.  At some point,
     this issue needs to be re-visited for chunks that immediately match with
     a different instantiation and a different type of support than the
     original, chunk-creating instantion. */
  /* REW: end   09.15.96 */


  /* --- handle initial refraction by adding it to tentative_retractions --- */
  if (refracted_inst) {
    insert_at_head_of_dll (p->instantiations, refracted_inst, next, prev);
    refracted_inst->rete_token = NIL;
    refracted_inst->rete_wme = NIL;
    allocate_with_pool (thisAgent, &thisAgent->ms_change_pool, &msc);
    msc->inst = refracted_inst;
    msc->p_node = p_node;
/* REW: begin 08.20.97 */
    /* Because the RETE 'artificially' refracts this instantiation (ie, it is
       not actually firing -- the original instantiation fires but not the
       chunk), we make the refracted instantiation of the chunk a nil_goal
       retraction, rather than associating it with the activity of its match
       goal. In p_node_left_addition, where the tentative assertion will be
       generated, we make it a point to look at the goal value and exrtac
       from the appropriate list; here we just make a a simplifying
       assumption that the goal is NIL (although, in reality), it never will
       be.  */

    /* This initialization is necessary (for at least safety reasons, for all
       msc's, regardless of the mode */
    msc->level = 0;
    msc->goal = NIL;
    if (thisAgent->operand2_mode) {

#ifdef DEBUG_WATERFALL    
       print_with_symbols(thisAgent, "\n %y is a refracted instantiation",
                          refracted_inst->prod->name); 
#endif 

       insert_at_head_of_dll (thisAgent->nil_goal_retractions,
                              msc, next_in_level, prev_in_level);
    }
/* REW: end   08.20.97 */

#ifdef BUG_139_WORKAROUND
        msc->p_node->b.p.prod->already_fired = 0;       /* RPM workaround for bug #139; mark prod as not fired yet */
#endif

    insert_at_head_of_dll (thisAgent->ms_retractions, msc, next, prev);
    insert_at_head_of_dll (p_node->b.p.tentative_retractions, msc,
                           next_of_node, prev_of_node);
  }

  /* --- call new node's add_left routine with all the parent's tokens --- */
  update_node_with_matches_from_above (thisAgent, p_node);

  /* --- store result indicator --- */
  if (! refracted_inst) {
    production_addition_result = NO_REFRACTED_INST;
  } else {
    remove_from_dll (p->instantiations, refracted_inst, next, prev);
    if (p_node->b.p.tentative_retractions) {
      production_addition_result = REFRACTED_INST_DID_NOT_MATCH;
      msc = p_node->b.p.tentative_retractions;
      p_node->b.p.tentative_retractions = NIL;
      remove_from_dll (thisAgent->ms_retractions, msc, next, prev);
      /* REW: begin 10.03.97 */ /* BUGFIX 2.125 */
    if (thisAgent->operand2_mode) {
      if (msc->goal) {
        remove_from_dll(msc->goal->id.ms_retractions, msc,
                        next_in_level, prev_in_level);
      } else {
        remove_from_dll(thisAgent->nil_goal_retractions,
                        msc, next_in_level, prev_in_level);
      }
    }
    /* REW: end   10.03.97 */

      
      free_with_pool (&thisAgent->ms_change_pool, msc);
   
    } else {
      production_addition_result = REFRACTED_INST_MATCHED;
    }
  }

  /* --- if not a chunk, store variable name information --- */
  if ((p->type==CHUNK_PRODUCTION_TYPE) && discard_chunk_varnames) {
    p->p_node->b.p.parents_nvn = NIL;
    p->rhs_unbound_variables = NIL;    
    deallocate_symbol_list_removing_references (thisAgent, rhs_unbound_vars_for_new_prod);
  } else {
    p->p_node->b.p.parents_nvn = get_nvn_for_condition_list (thisAgent, lhs_top, NIL);
    p->rhs_unbound_variables =
      destructively_reverse_list (rhs_unbound_vars_for_new_prod);
  }

   /* --- invoke callback functions --- */
  soar_invoke_callbacks (thisAgent, thisAgent, PRODUCTION_JUST_ADDED_CALLBACK,
                         (soar_call_data) p);

//#ifdef _WINDOWS
//        add_production_to_stat_lists(new_prod);
//#endif

  /* JC ADDED: tell gSKI we're about to add a production */
  gSKI_MakeAgentCallback(gSKI_K_EVENT_PRODUCTION_ADDED, 1, thisAgent, static_cast<void*>(p));

  return production_addition_result;
}

/* ---------------------------------------------------------------------
                      Excise Production from Rete

   This removes a given production from the Rete net, and enqueues all 
   its existing instantiations as pending retractions.
--------------------------------------------------------------------- */

void excise_production_from_rete (agent* thisAgent, production *p) 
{
  rete_node *p_node, *parent;
  ms_change *msc;

  soar_invoke_callbacks (thisAgent, thisAgent, 
                         PRODUCTION_JUST_ABOUT_TO_BE_EXCISED_CALLBACK,
                         (soar_call_data) p);
 
  /* JC ADDED: Tell gSKI we are about to excise a production */
  gSKI_MakeAgentCallback(gSKI_K_EVENT_PRODUCTION_REMOVED, 0, thisAgent, static_cast<void*>(p));
  
//#ifdef _WINDOWS
//        remove_production_from_stat_lists(prod_to_be_excised);
//#endif
  
  p_node = p->p_node;
  p->p_node = NIL;      /* mark production as not being in the rete anymore */
  parent = p_node->parent;

  /* --- deallocate the variable name information --- */
  if (p_node->b.p.parents_nvn)
    deallocate_node_varnames (thisAgent, parent, thisAgent->dummy_top_node,
                              p_node->b.p.parents_nvn);

  /* --- cause all existing instantiations to retract, by removing any
     tokens at the node --- */
  while (p_node->a.np.tokens) remove_token_and_subtree (thisAgent, p_node->a.np.tokens);  

  /* --- At this point, there are no tentative_assertion's.  Now set
     the p_node field of all tentative_retractions to NIL, to indicate
     that the p_node is being excised  --- */
  for (msc=p_node->b.p.tentative_retractions; msc!=NIL; msc=msc->next_of_node)
    msc->p_node = NIL;

  /* --- finally, excise the p_node --- */
  remove_node_from_parents_list_of_children (p_node);
  update_stats_for_destroying_node (thisAgent, p_node);   /* clean up rete stats stuff */
  free_with_pool (&thisAgent->rete_node_pool, p_node);

  /* --- update sharing factors on the path from here to the top node --- */
  adjust_sharing_factors_from_here_to_top (parent, -1);

  /* --- and propogate up the net --- */
  if (! parent->first_child) 
     deallocate_rete_node (thisAgent, parent);
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
                      Add Gensymmed Equality Test

   This routine destructively modifies a given test, adding to it a test
   for equality with a new gensym variable.
---------------------------------------------------------------------- */

void add_gensymmed_equality_test (agent* thisAgent, test *t, char first_letter) {
  Symbol *New;
  test eq_test;
  char prefix[2];
  
  prefix[0] = first_letter;
  prefix[1] = 0;
  New = generate_new_variable (thisAgent, prefix);
  eq_test = make_equality_test (New);
  symbol_remove_ref (thisAgent, New);
  add_new_test_to_test (thisAgent, t, eq_test);
}

/* ----------------------------------------------------------------------
                     Var Bound in Reconstructed Conds

   We're reconstructing the conditions for a production in top-down
   fashion.  Suppose we come to a Rete test checking for equality with 
   the "value" field 3 levels up.  In that case, for the current condition,
   we want to include an equality test for whatever variable got bound
   in the value field 3 levels up.  This function scans up the list
   of conditions reconstructed so far, and finds the appropriate variable.
---------------------------------------------------------------------- */

Symbol *var_bound_in_reconstructed_conds (agent* thisAgent, 
                                                                                  condition *cond, /* current cond */
                                          byte where_field_num,
                                          rete_node_level where_levels_up) {
  test t;
  complex_test *ct;
  cons *c;
  
  while (where_levels_up) { where_levels_up--; cond = cond->prev; }

  if (where_field_num==0) t = cond->data.tests.id_test;
  else if (where_field_num==1) t = cond->data.tests.attr_test;
  else t = cond->data.tests.value_test;

  if (test_is_blank_test(t)) goto abort_var_bound_in_reconstructed_conds;
  if (test_is_blank_or_equality_test(t)) return referent_of_equality_test(t);
      
  ct = complex_test_from_test(t);
  if (ct->type==CONJUNCTIVE_TEST) {
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      if ( (! test_is_blank_test ((test)(c->first))) &&
           (test_is_blank_or_equality_test ((test)(c->first))) )
        return referent_of_equality_test ((test)(c->first));
  }

  abort_var_bound_in_reconstructed_conds:
  { char msg[BUFFER_MSG_SIZE];
  strncpy (msg, "Internal error in var_bound_in_reconstructed_conds\n", BUFFER_MSG_SIZE);
  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
  abort_with_fatal_error(thisAgent, msg);
  }
  return 0; /* unreachable, but without it, gcc -Wall warns here */
}

/* ----------------------------------------------------------------------
                      Add Rete Test List to Tests

   Given the additional Rete tests (besides the hashed equality test) at
   a certain node, we need to convert them into the equivalent tests in
   the conditions being reconstructed.  This procedure does this -- it
   destructively modifies the given currently-being-reconstructed-cond
   by adding any necessary extra tests to its three field tests.
---------------------------------------------------------------------- */

void add_rete_test_list_to_tests (agent* thisAgent, 
                                                                  condition *cond, /* current cond */
                                  rete_test *rt) {
  Symbol *referent;
  test New;
  complex_test *new_ct;
  byte test_type;
  
  for ( ; rt!=NIL; rt=rt->next) {
   
    if (rt->type==ID_IS_GOAL_RETE_TEST) {
      allocate_with_pool (thisAgent, &thisAgent->complex_test_pool, &new_ct);
      New = make_test_from_complex_test(new_ct);
      new_ct->type = GOAL_ID_TEST;
    } else if (rt->type==ID_IS_IMPASSE_RETE_TEST) {
      allocate_with_pool (thisAgent, &thisAgent->complex_test_pool, &new_ct);
      New = make_test_from_complex_test(new_ct);
      new_ct->type = IMPASSE_ID_TEST;
    } else if (rt->type==DISJUNCTION_RETE_TEST) {
      allocate_with_pool (thisAgent, &thisAgent->complex_test_pool, &new_ct);
      New = make_test_from_complex_test(new_ct);
      new_ct->type = DISJUNCTION_TEST;
      new_ct->data.disjunction_list =
        copy_symbol_list_adding_references (thisAgent, rt->data.disjunction_list);
    } else if (test_is_constant_relational_test(rt->type)) {
      test_type =
        relational_test_type_to_test_type[kind_of_relational_test(rt->type)];
      referent = rt->data.constant_referent;
      symbol_add_ref (referent);
      if (test_type==EQUAL_TEST_TYPE) {
        New = make_equality_test_without_adding_reference (referent);
      } else {
        allocate_with_pool (thisAgent, &thisAgent->complex_test_pool, &new_ct);
        New = make_test_from_complex_test(new_ct);
        new_ct->type = test_type;
        new_ct->data.referent = referent;
      }
    } else if (test_is_variable_relational_test(rt->type)) {
      test_type =
        relational_test_type_to_test_type[kind_of_relational_test(rt->type)];
      if (! rt->data.variable_referent.levels_up) {
        /* --- before calling var_bound_in_reconstructed_conds, make sure 
           there's an equality test in the referent location (add one if
           there isn't one already there), otherwise there'd be no variable
           there to test against --- */
        if (rt->data.variable_referent.field_num==0) {
          if (! test_includes_equality_test_for_symbol
                  (cond->data.tests.id_test, NIL))
            add_gensymmed_equality_test (thisAgent, &(cond->data.tests.id_test), 's');
        } else if (rt->data.variable_referent.field_num==1) {
          if (! test_includes_equality_test_for_symbol
                  (cond->data.tests.attr_test, NIL))
            add_gensymmed_equality_test (thisAgent, &(cond->data.tests.attr_test), 'a');
        } else {
          if (! test_includes_equality_test_for_symbol
                  (cond->data.tests.value_test, NIL))
            add_gensymmed_equality_test (thisAgent, &(cond->data.tests.value_test),
                       first_letter_from_test(cond->data.tests.attr_test));
        }
      }
      referent = var_bound_in_reconstructed_conds (thisAgent, cond,
                          rt->data.variable_referent.field_num,
                          rt->data.variable_referent.levels_up);
      symbol_add_ref (referent);
      if (test_type==EQUAL_TEST_TYPE) {
        New = make_equality_test_without_adding_reference (referent);
      } else {
        allocate_with_pool (thisAgent, &thisAgent->complex_test_pool, &new_ct);
        New = make_test_from_complex_test(new_ct);
        new_ct->type = test_type;
        new_ct->data.referent = referent;
      }
    } else {
      char msg[BUFFER_MSG_SIZE];
      strncpy (msg, "Error: bad test_type in add_rete_test_to_test\n", BUFFER_MSG_SIZE);
     msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
     abort_with_fatal_error(thisAgent, msg);
      New = NIL; /* unreachable, but without it gcc -Wall warns here */
    }

    if (rt->right_field_num==0)
      add_new_test_to_test (thisAgent, &(cond->data.tests.id_test), New);
    else if (rt->right_field_num==2)
      add_new_test_to_test (thisAgent, &(cond->data.tests.value_test), New);
    else
      add_new_test_to_test (thisAgent, &(cond->data.tests.attr_test), New);
  }
}

/* ----------------------------------------------------------------------
                               Collect Nots

   When we build the instantiated conditions for a production being
   fired, we also record all the "<>" tests between pairs of identifiers.
   (This information is used during chunking.)  This procedure looks for
   any such <> tests in the given Rete test list (from the "other tests"
   at a Rete node), and adds records of them to the global variable
   nots_found_in_production.  "Right_wme" is the wme that matched
   the current condition; "cond" is the currently-being-reconstructed
   condition.
---------------------------------------------------------------------- */

//not_struct *nots_found_in_production; /* collected <> tests */

void collect_nots (agent* thisAgent, 
                   rete_test *rt,
                   wme *right_wme,
                   condition *cond, 
                   not_struct * & nots_found_in_production) {
  not_struct *new_not;
  Symbol *right_sym;
  Symbol *referent;

  for ( ; rt!=NIL; rt=rt->next) {

    if (! test_is_not_equal_test(rt->type)) continue;

    right_sym = field_from_wme (right_wme, rt->right_field_num);

    if (right_sym->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) continue;
   
    if (rt->type == CONSTANT_RELATIONAL_RETE_TEST +
                    RELATIONAL_NOT_EQUAL_RETE_TEST) {
      referent = rt->data.constant_referent;
      if (referent->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) continue;
      allocate_with_pool (thisAgent, &thisAgent->not_pool, &new_not);
      new_not->next = nots_found_in_production;
      nots_found_in_production = new_not;
      new_not->s1 = right_sym;
      symbol_add_ref (right_sym);
      new_not->s2 = referent;
      symbol_add_ref (referent);
      continue;
    }
   
    if (rt->type == VARIABLE_RELATIONAL_RETE_TEST +
                    RELATIONAL_NOT_EQUAL_RETE_TEST) {
      referent = var_bound_in_reconstructed_conds (thisAgent, cond, 
                              rt->data.variable_referent.field_num,
                              rt->data.variable_referent.levels_up);
      if (referent->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) continue;
      allocate_with_pool (thisAgent, &thisAgent->not_pool, &new_not);
      new_not->next = nots_found_in_production;
      nots_found_in_production = new_not;
      new_not->s1 = right_sym;
      symbol_add_ref (right_sym);
      new_not->s2 = referent;
      symbol_add_ref (referent);
      continue;
    }
  }
}

/* ----------------------------------------------------------------------
                          Add Varnames to Test

   This routine adds (an equality test for) each variable in "vn" to
   the given test "t", destructively modifying t.  This is used for
   restoring the original variables to test in a hand-coded production
   when we reconstruct its conditions.
---------------------------------------------------------------------- */

void add_varnames_to_test (agent* thisAgent, varnames *vn, test *t) {
  test New;
  cons *c;
 
  if (vn == NIL) return;
  if (varnames_is_one_var(vn)) {
    New = make_equality_test (varnames_to_one_var(vn));
    add_new_test_to_test (thisAgent, t, New);
  } else {
    for (c=varnames_to_var_list(vn); c!=NIL; c=c->rest) {
      New = make_equality_test ((Symbol *)(c->first));
      add_new_test_to_test (thisAgent, t, New);
    }
  }
}

/* ----------------------------------------------------------------------
                      Add Hash Info to ID Test

   This routine adds an equality test to the id field test in a given
   condition, destructively modifying that id test.  The equality test
   is the one appropriate for the given hash location (field_num/levels_up).
---------------------------------------------------------------------- */

void add_hash_info_to_id_test (agent* thisAgent, 
                                                           condition *cond,
                               byte field_num,
                               rete_node_level levels_up) {
  Symbol *temp;
  test New;
 
  temp = var_bound_in_reconstructed_conds (thisAgent, cond, field_num, levels_up);
  New = make_equality_test (temp);
  add_new_test_to_test (thisAgent, &(cond->data.tests.id_test), New);
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
---------------------------------------------------------------------- */

/* NOTE: clean this procedure up somehow? */

void rete_node_to_conditions (agent* thisAgent, 
                                                          rete_node *node,
                              node_varnames *nvn,
                              rete_node *cutoff,
                              token *tok,
                              wme *w,
                              condition *conds_for_cutoff_and_up,
                              condition **dest_top_cond,
                              condition **dest_bottom_cond, 
                                                          not_struct * & nots_found_in_production) {
  condition *cond;
  alpha_mem *am;
    
  allocate_with_pool (thisAgent, &thisAgent->condition_pool, &cond);
  if (real_parent_node(node)==cutoff) {
    cond->prev = conds_for_cutoff_and_up; /* if this is the top of an NCC, this
                                             will get replaced by NIL later */
    *dest_top_cond = cond;
  } else {
    rete_node_to_conditions (thisAgent, real_parent_node(node),
                             nvn ? nvn->parent : NIL,
                             cutoff,
                             tok ? tok->parent : NIL,
                             tok ? tok->w : NIL,
                             conds_for_cutoff_and_up,
                             dest_top_cond, &(cond->prev), 
                                                         nots_found_in_production);
    cond->prev->next = cond;
  }
  cond->next = NIL;
  *dest_bottom_cond = cond;
  
  if (node->node_type==CN_BNODE) {
    cond->type = CONJUNCTIVE_NEGATION_CONDITION;
    rete_node_to_conditions (thisAgent, node->b.cn.partner->parent,
                             nvn ? nvn->data.bottom_of_subconditions : NIL,
                             node->parent,
                             NIL,
                             NIL,
                             cond->prev,
                             &(cond->data.ncc.top),
                             &(cond->data.ncc.bottom), 
                                                         nots_found_in_production);
    cond->data.ncc.top->prev = NIL;
  } else {
    if (bnode_is_positive(node->node_type))
      cond->type = POSITIVE_CONDITION;
    else
      cond->type = NEGATIVE_CONDITION;
    
    if (w && (cond->type==POSITIVE_CONDITION)) {
      /* --- make simple tests and collect nots --- */
      cond->data.tests.id_test = make_equality_test (w->id);
      cond->data.tests.attr_test = make_equality_test (w->attr);
      cond->data.tests.value_test = make_equality_test (w->value);
      cond->test_for_acceptable_preference = w->acceptable;
      cond->bt.wme_ = w;
      if (node->b.posneg.other_tests) /* don't bother if there are no tests*/
        collect_nots (thisAgent, node->b.posneg.other_tests, w, cond, 
                              nots_found_in_production);
    } else {
      am = node->b.posneg.alpha_mem_;
      cond->data.tests.id_test = make_blank_or_equality_test (am->id);
      cond->data.tests.attr_test = make_blank_or_equality_test (am->attr);
      cond->data.tests.value_test = make_blank_or_equality_test (am->value);
      cond->test_for_acceptable_preference = am->acceptable;
      
      if (nvn) {
        add_varnames_to_test (thisAgent, nvn->data.fields.id_varnames,
                              &(cond->data.tests.id_test));
        add_varnames_to_test (thisAgent, nvn->data.fields.attr_varnames,
                              &(cond->data.tests.attr_test));
        add_varnames_to_test (thisAgent, nvn->data.fields.value_varnames,
                              &(cond->data.tests.value_test));
      }

      /* --- on hashed nodes, add equality test for the hash function --- */
      if ((node->node_type==MP_BNODE) || (node->node_type==NEGATIVE_BNODE)) {
        add_hash_info_to_id_test (thisAgent, cond,
                                  node->left_hash_loc_field_num,
                                  node->left_hash_loc_levels_up);
      } else if (node->node_type==POSITIVE_BNODE) {
        add_hash_info_to_id_test (thisAgent, cond,
                                  node->parent->left_hash_loc_field_num,
                                  node->parent->left_hash_loc_levels_up);
      }

      /* --- if there are other tests, add them too --- */
      if (node->b.posneg.other_tests)
        add_rete_test_list_to_tests (thisAgent, cond, node->b.posneg.other_tests);

      /* --- if we threw away the variable names, make sure there's some 
         equality test in each of the three fields --- */
      if (! nvn) {
        if (! test_includes_equality_test_for_symbol
                (cond->data.tests.id_test, NIL))
          add_gensymmed_equality_test (thisAgent, &(cond->data.tests.id_test), 's');
        if (! test_includes_equality_test_for_symbol
                (cond->data.tests.attr_test, NIL))
          add_gensymmed_equality_test (thisAgent, &(cond->data.tests.attr_test), 'a');
        if (! test_includes_equality_test_for_symbol 
                (cond->data.tests.value_test, NIL))
          add_gensymmed_equality_test (thisAgent, &(cond->data.tests.value_test),
                    first_letter_from_test (cond->data.tests.attr_test));
      }
    }
  }
}

/* -------------------------------------------------------------------
             Reconstructing the RHS Actions of a Production

   When we print a production (but not when we fire one), we have to 
   reconstruct the RHS actions.  This is because many of the variables
   in the RHS have been replaced by references to Rete locations (i.e.,
   rather than specifying <v>, we specify "value field 3 levels up"
   or "the 7th RHS unbound variable".  The routines below copy rhs_value's
   and actions, and substitute variable names for such references.
   For RHS unbound variables, we gensym new variable names.
------------------------------------------------------------------- */

rhs_value copy_rhs_value_and_substitute_varnames (agent* thisAgent, 
                                                  rhs_value rv,
                                                  condition *cond,
                                                  char first_letter) {
  cons *c, *new_c, *prev_new_c;
  list *fl, *new_fl;
  Symbol *sym;
  long index;
  char prefix[2];
  
  if (rhs_value_is_reteloc(rv)) {
    sym = var_bound_in_reconstructed_conds (thisAgent, cond,
                                 (byte)rhs_value_to_reteloc_field_num(rv),
                                 (rete_node_level)rhs_value_to_reteloc_levels_up(rv));
    symbol_add_ref (sym);
    return symbol_to_rhs_value (sym);
  }

  if (rhs_value_is_unboundvar(rv)) 
  {
    index = rhs_value_to_unboundvar(rv);
    if (! *(thisAgent->rhs_variable_bindings+index)) 
    {
      prefix[0] = first_letter;
      prefix[1] = 0;

      sym = generate_new_variable (thisAgent, prefix);
      *(thisAgent->rhs_variable_bindings+index) = sym;

      if (thisAgent->highest_rhs_unboundvar_index < index)
      {
        thisAgent->highest_rhs_unboundvar_index = index;
      }
    } 
    else 
    {
      sym = *(thisAgent->rhs_variable_bindings+index);
      symbol_add_ref (sym);
    }
    return symbol_to_rhs_value (sym);
  }

  if (rhs_value_is_funcall(rv)) {
    fl = rhs_value_to_funcall_list(rv);
    allocate_cons (thisAgent, &new_fl);
    new_fl->first = fl->first;
    prev_new_c = new_fl;
    for (c=fl->rest; c!=NIL; c=c->rest) {
      allocate_cons (thisAgent, &new_c);
      new_c->first = copy_rhs_value_and_substitute_varnames (thisAgent, 
                                                                                                                         static_cast<char *>(c->first),
                                                                     cond, first_letter);
      prev_new_c->rest = new_c;
      prev_new_c = new_c;
    }
    prev_new_c->rest = NIL;
    return funcall_list_to_rhs_value (new_fl);
  } else {
    symbol_add_ref (rhs_value_to_symbol(rv));
    return rv;
  }
}

action *copy_action_list_and_substitute_varnames (agent* thisAgent, 
                                                                                                  action *actions,
                                                  condition *cond) {
  action *old, *New, *prev, *first;
  char first_letter;
  
  prev = NIL;
  first = NIL;  /* unneeded, but without it gcc -Wall warns here */
  old = actions;
  while (old) {
    allocate_with_pool (thisAgent, &thisAgent->action_pool, &New);
    if (prev) prev->next = New; else first = New;
    prev = New;
    New->type = old->type;
    New->preference_type = old->preference_type;
    New->support = old->support;
    if (old->type==FUNCALL_ACTION) {
      New->value = copy_rhs_value_and_substitute_varnames (thisAgent, 
                                                           old->value, cond,
                                                           'v');
    } else {
      New->id = copy_rhs_value_and_substitute_varnames (thisAgent, old->id, cond, 's');
      New->attr = copy_rhs_value_and_substitute_varnames (thisAgent, old->attr, cond,'a');
      first_letter = first_letter_from_rhs_value (New->attr);
      New->value = copy_rhs_value_and_substitute_varnames (thisAgent, old->value, cond,
                          first_letter);
      if (preference_is_binary(old->preference_type))
        New->referent = copy_rhs_value_and_substitute_varnames (thisAgent, old->referent,
                                              cond, first_letter);
    }
    old = old->next;
  }
  if (prev) prev->next = NIL; else first = NIL;
  return first;     
}

/* -----------------------------------------------------------------------
                     P Node to Conditions and Nots
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

void p_node_to_conditions_and_nots (agent* thisAgent, 
                                                                        rete_node *p_node,
                                    token *tok,
                                    wme *w,
                                    condition **dest_top_cond,
                                    condition **dest_bottom_cond,
                                    not_struct **dest_nots,
                                    action **dest_rhs) {
  cons *c;
  Symbol **cell;
  long index;
  production *prod;

  prod = p_node->b.p.prod;
  
  not_struct *nots_found_in_production = NIL;
  if (tok==NIL) w=NIL;  /* just for safety */
  reset_variable_generator (thisAgent, NIL, NIL); /* we'll be gensymming new vars */
  rete_node_to_conditions (thisAgent, 
                           p_node->parent,
                           p_node->b.p.parents_nvn,
                           thisAgent->dummy_top_node,
                           tok, w, NIL,
                           dest_top_cond, dest_bottom_cond, 
                                                   nots_found_in_production);
  if (tok) *dest_nots = nots_found_in_production;
  nots_found_in_production = NIL; /* just for safety */
  if (dest_rhs) 
  {
     thisAgent->highest_rhs_unboundvar_index = -1;
     if (prod->rhs_unbound_variables) 
     {
        cell = thisAgent->rhs_variable_bindings;
        for (c=prod->rhs_unbound_variables; c!=NIL; c=c->rest) 
        {
           *(cell++) = static_cast<symbol_union *>(c->first);
           thisAgent->highest_rhs_unboundvar_index++;
        }
     }
     *dest_rhs = copy_action_list_and_substitute_varnames (thisAgent, 
                prod->action_list,
        *dest_bottom_cond);
     index = 0;
     cell = thisAgent->rhs_variable_bindings;
     while (index++ <= thisAgent->highest_rhs_unboundvar_index) *(cell++) = NIL;
  }
}

Symbol *get_symbol_from_rete_loc (unsigned short levels_up,
                                  byte field_num,
                                  token *tok, wme *w) {
  while (levels_up) {
    levels_up--;
    w = tok->w;
    tok = tok->parent;
  }
  if (field_num==0) return w->id;
  if (field_num==1) return w->attr;  
  return w->value;
}


/* **********************************************************************

   SECTION 11:  Rete Test Evaluation Routines

   These routines perform the "other tests" stored at positive and
   negative join nodes.  Each is passed parameters: the rete_test
   to be performed, and the <token,wme> pair on which to perform the
   test.
********************************************************************** */

Bool error_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w);
#define ertr error_rete_test_routine
Bool ( (*(rete_test_routines[256]))
       (agent* thisAgent, rete_test *rt, token *left, wme *w)) =
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

/*#define match_left_and_right(rete_test,left,w) \
  ( (*(rete_test_routines[(rete_test)->type])) \
    ((rete_test),(left),(w)) )*/
inline Bool match_left_and_right(agent* thisAgent, rete_test * _rete_test, 
                                   token * left, wme * w)
{
  return ( (*(rete_test_routines[(_rete_test)->type])) \
    (thisAgent,(_rete_test),(left),(w)) );
}

/* This macro cannot be easily converted to an inline function. 
   Some additional changes are required.
*/
#define numeric_comparison_between_symbols(s1,s2,comparator_op) ( \
  ( ((s1)->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) && \
    ((s2)->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) ) ? \
    (((s1)->ic.value) comparator_op ((s2)->ic.value)) : \
  ( ((s1)->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) && \
    ((s2)->common.symbol_type==FLOAT_CONSTANT_SYMBOL_TYPE) ) ? \
    (((s1)->ic.value) comparator_op ((s2)->fc.value)) : \
  ( ((s1)->common.symbol_type==FLOAT_CONSTANT_SYMBOL_TYPE) && \
    ((s2)->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) ) ? \
    (((s1)->fc.value) comparator_op ((s2)->ic.value)) : \
  ( ((s1)->common.symbol_type==FLOAT_CONSTANT_SYMBOL_TYPE) && \
    ((s2)->common.symbol_type==FLOAT_CONSTANT_SYMBOL_TYPE) ) ? \
    (((s1)->fc.value) comparator_op ((s2)->fc.value)) : \
  FALSE )

/* Note:  "=" and "<>" tests always return FALSE when one argument is
   an integer and the other is a floating point number */

Bool error_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  char msg[BUFFER_MSG_SIZE];
  strncpy (msg, "Internal error: bad rete test type, hit error_rete_test_routine\n", BUFFER_MSG_SIZE);
  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
  abort_with_fatal_error(thisAgent, msg);
  return FALSE; /* unreachable, but without it, gcc -Wall warns here */
}

Bool id_is_goal_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  return w->id->id.isa_goal;
}

Bool id_is_impasse_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  return w->id->id.isa_impasse;
}

Bool disjunction_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  Symbol *sym;
  cons *c;

  sym = field_from_wme (w,rt->right_field_num);
  for (c=rt->data.disjunction_list; c!=NIL; c=c->rest)
    if (c->first==sym) return TRUE;
  return FALSE;
}

Bool constant_equal_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  Symbol *s1, *s2;
 
  s1 = field_from_wme (w,rt->right_field_num);
  s2 = rt->data.constant_referent;
  return (s1 == s2);
}

Bool constant_not_equal_rete_test_routine (agent* thisAgent, rete_test *rt, token *left,
                                           wme *w) {
  Symbol *s1, *s2;
 
  s1 = field_from_wme (w,rt->right_field_num);
  s2 = rt->data.constant_referent;
  return (s1 != s2);
}

Bool constant_less_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  Symbol *s1, *s2;
 
  s1 = field_from_wme (w,rt->right_field_num);
  s2 = rt->data.constant_referent;
  return numeric_comparison_between_symbols (s1, s2, < );
}

Bool constant_greater_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  Symbol *s1, *s2;
 
  s1 = field_from_wme (w,rt->right_field_num);
  s2 = rt->data.constant_referent;
  return numeric_comparison_between_symbols (s1, s2, > );
}

Bool constant_less_or_equal_rete_test_routine (agent* thisAgent, rete_test *rt, token *left,
                                               wme *w) {
  Symbol *s1, *s2;
 
  s1 = field_from_wme (w,rt->right_field_num);
  s2 = rt->data.constant_referent;
  return numeric_comparison_between_symbols (s1, s2, <= );
}

Bool constant_greater_or_equal_rete_test_routine (agent* thisAgent, rete_test *rt, token *left,
                                                  wme *w) {
  Symbol *s1, *s2;
 
  s1 = field_from_wme (w,rt->right_field_num);
  s2 = rt->data.constant_referent;
  return numeric_comparison_between_symbols (s1, s2, >= );
}

Bool constant_same_type_rete_test_routine (agent* thisAgent, rete_test *rt, token *left,
                                           wme *w) {
  Symbol *s1, *s2;
 
  s1 = field_from_wme (w,rt->right_field_num);
  s2 = rt->data.constant_referent;
  return (s1->common.symbol_type == s2->common.symbol_type);
}

Bool variable_equal_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  Symbol *s1, *s2;
  int i;
 
  s1 = field_from_wme (w, rt->right_field_num);

  if (rt->data.variable_referent.levels_up!=0) {
    i = rt->data.variable_referent.levels_up - 1;
    while (i!=0) {
      left = left->parent;
      i--;
    }
    w = left->w;
  }
  s2 = field_from_wme (w, rt->data.variable_referent.field_num);

  return (s1 == s2);
}

Bool variable_not_equal_rete_test_routine (agent* thisAgent, rete_test *rt, token *left,
                                           wme *w) {
  Symbol *s1, *s2;
  int i;
 
  s1 = field_from_wme (w, rt->right_field_num);

  if (rt->data.variable_referent.levels_up!=0) {
    i = rt->data.variable_referent.levels_up - 1;
    while (i!=0) {
      left = left->parent;
      i--;
    }
    w = left->w;
  }
  s2 = field_from_wme (w, rt->data.variable_referent.field_num);

  return (s1 != s2);
}

Bool variable_less_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  Symbol *s1, *s2;
  int i;
 
  s1 = field_from_wme (w, rt->right_field_num);

  if (rt->data.variable_referent.levels_up!=0) {
    i = rt->data.variable_referent.levels_up - 1;
    while (i!=0) {
      left = left->parent;
      i--;
    }
    w = left->w;
  }
  s2 = field_from_wme (w, rt->data.variable_referent.field_num);

  return numeric_comparison_between_symbols (s1, s2, < );
}

Bool variable_greater_rete_test_routine (agent* thisAgent, rete_test *rt, token *left, wme *w) {
  Symbol *s1, *s2;
  int i;
 
  s1 = field_from_wme (w, rt->right_field_num);

  if (rt->data.variable_referent.levels_up!=0) {
    i = rt->data.variable_referent.levels_up - 1;
    while (i!=0) {
      left = left->parent;
      i--;
    }
    w = left->w;
  }
  s2 = field_from_wme (w, rt->data.variable_referent.field_num);

  return numeric_comparison_between_symbols (s1, s2, > );
}

Bool variable_less_or_equal_rete_test_routine (agent* thisAgent, rete_test *rt, token *left,
                                               wme *w) {
  Symbol *s1, *s2;
  int i;
 
  s1 = field_from_wme (w, rt->right_field_num);

  if (rt->data.variable_referent.levels_up!=0) {
    i = rt->data.variable_referent.levels_up - 1;
    while (i!=0) {
      left = left->parent;
      i--;
    }
    w = left->w;
  }
  s2 = field_from_wme (w, rt->data.variable_referent.field_num);

  return numeric_comparison_between_symbols (s1, s2, <= );
}

Bool variable_greater_or_equal_rete_test_routine (agent* thisAgent, rete_test *rt, token *left,
                                                  wme *w) {
  Symbol *s1, *s2;
  int i;
 
  s1 = field_from_wme (w, rt->right_field_num);

  if (rt->data.variable_referent.levels_up!=0) {
    i = rt->data.variable_referent.levels_up - 1;
    while (i!=0) {
      left = left->parent;
      i--;
    }
    w = left->w;
  }
  s2 = field_from_wme (w, rt->data.variable_referent.field_num);

  return numeric_comparison_between_symbols (s1, s2, >= );
}

Bool variable_same_type_rete_test_routine (agent* thisAgent, rete_test *rt, token *left,
                                           wme *w) {
  Symbol *s1, *s2;
  int i;
 
  s1 = field_from_wme (w, rt->right_field_num);

  if (rt->data.variable_referent.levels_up!=0) {
    i = rt->data.variable_referent.levels_up - 1;
    while (i!=0) {
      left = left->parent;
      i--;
    }
    w = left->w;
  }
  s2 = field_from_wme (w, rt->data.variable_referent.field_num);
  return (s1->common.symbol_type == s2->common.symbol_type);
}



/* ************************************************************************

   SECTION 12:  Beta Node Interpreter Routines: Mem, Pos, and MP Nodes

************************************************************************ */

void positive_node_left_addition (agent* thisAgent, rete_node *node, token *New,
                                  Symbol *hash_referent);
void unhashed_positive_node_left_addition (agent* thisAgent, rete_node *node, token *New);

void rete_error_left (agent* thisAgent, rete_node *node, token *t, wme *w) {
  char msg[BUFFER_MSG_SIZE];
  snprintf (msg, BUFFER_MSG_SIZE, "Rete net error:  tried to left-activate node of type %d\n",
         node->node_type);
  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
  abort_with_fatal_error(thisAgent, msg);
}

void rete_error_right (agent* thisAgent, rete_node *node, wme *w) {
  char msg[BUFFER_MSG_SIZE];
  snprintf (msg, BUFFER_MSG_SIZE, "Rete net error:  tried to right-activate node of type %d\n",
         node->node_type);
  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
  abort_with_fatal_error(thisAgent, msg);
}

void beta_memory_node_left_addition (agent* thisAgent, rete_node *node, 
                                                                         token *tok, wme *w) {
  unsigned long hv;
  Symbol *referent;
  rete_node *child, *next;
  token *New;

  activation_entry_sanity_check();
  left_node_activation(node,TRUE);

  {
    int levels_up;
    token *t;
    
    levels_up = node->left_hash_loc_levels_up;
    if (levels_up==1) {
      referent = field_from_wme (w, node->left_hash_loc_field_num);
    } else { /* --- levels_up > 1 --- */
      for (t=tok, levels_up -= 2; levels_up!=0; levels_up--) t=t->parent;
      referent = field_from_wme (t->w, node->left_hash_loc_field_num);
    }
  }
  
  hv = node->node_id ^ referent->common.hash_id;

  /* --- build new left token, add it to the hash table --- */
  token_added(node);
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &New);
  new_left_token (New, node, tok, w);
  insert_token_into_left_ht (thisAgent, New, hv);
  New->a.ht.referent = referent;

  /* --- inform each linked child (positive join) node --- */
  for (child=node->b.mem.first_linked_child; child!=NIL; child=next) {
    next = child->a.pos.next_from_beta_mem;
    positive_node_left_addition (thisAgent, child, New, referent);
  }
  activation_exit_sanity_check(); 
}   

void unhashed_beta_memory_node_left_addition (agent* thisAgent, 
                                                                                          rete_node *node, token *tok,
                                              wme *w) {
  unsigned long hv;
  rete_node *child, *next;
  token *New;

  activation_entry_sanity_check();  
  left_node_activation(node,TRUE);

  hv = node->node_id;

  /* --- build new left token, add it to the hash table --- */
  token_added(node);
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &New);
  new_left_token (New, node, tok, w);
  insert_token_into_left_ht (thisAgent, New, hv);
  New->a.ht.referent = NIL;

  /* --- inform each linked child (positive join) node --- */
  for (child=node->b.mem.first_linked_child; child!=NIL; child=next) {
    next = child->a.pos.next_from_beta_mem;
    unhashed_positive_node_left_addition (thisAgent, child, New);
  }
  activation_exit_sanity_check();
}

void positive_node_left_addition (agent* thisAgent, 
                                                                  rete_node *node, token *New,
                                  Symbol *hash_referent) {
  unsigned long right_hv;
  right_mem *rm;
  alpha_mem *am;
  rete_test *rt;
  Bool failed_a_test;
  rete_node *child;

  activation_entry_sanity_check();
  left_node_activation(node,TRUE);

  am = node->b.posneg.alpha_mem_;

  if (node_is_right_unlinked(node)) {
    relink_to_right_mem (node);
    if (am->right_mems==NIL) {
      unlink_from_left_mem (node);
      activation_exit_sanity_check();
      return;
    }
  }

  /* --- look through right memory for matches --- */
  right_hv = am->am_id ^ hash_referent->common.hash_id;
  for (rm=right_ht_bucket(thisAgent, right_hv); rm!=NIL; rm=rm->next_in_bucket) {
    if (rm->am != am) continue;
    /* --- does rm->w match New? --- */
    if (hash_referent != rm->w->id) continue;
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, New, rm->w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found, so call each child node --- */
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,New,rm->w);
  }
  activation_exit_sanity_check();
}   

void unhashed_positive_node_left_addition (agent* thisAgent, rete_node *node, token *New) {
  right_mem *rm;
  rete_test *rt;
  Bool failed_a_test;
  rete_node *child;

  activation_entry_sanity_check();  
  left_node_activation(node,TRUE);

  if (node_is_right_unlinked(node)) {
    relink_to_right_mem (node);
    if (node->b.posneg.alpha_mem_->right_mems==NIL) {
      unlink_from_left_mem (node);
      activation_exit_sanity_check();
      return;
    }
  }

  /* --- look through right memory for matches --- */
  for (rm=node->b.posneg.alpha_mem_->right_mems; rm!=NIL; 
rm=rm->next_in_am) {
    /* --- does rm->w match new? --- */
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, New, rm->w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found, so call each child node --- */
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,New,rm->w);
  }
  activation_exit_sanity_check();
}   

void mp_node_left_addition (agent* thisAgent, rete_node *node, token *tok, wme *w) {
  unsigned long hv;
  Symbol *referent;
  rete_node *child;
  token *New;
  unsigned long right_hv;
  right_mem *rm;
  alpha_mem *am;
  rete_test *rt;
  Bool failed_a_test;

  activation_entry_sanity_check();
  left_node_activation(node,TRUE);

  {
    int levels_up;
    token *t;
    
    levels_up = node->left_hash_loc_levels_up;
    if (levels_up==1) {
      referent = field_from_wme (w, node->left_hash_loc_field_num);
    } else { /* --- levels_up > 1 --- */
      for (t=tok, levels_up -= 2; levels_up!=0; levels_up--) t=t->parent;
      referent = field_from_wme (t->w, node->left_hash_loc_field_num);
    }
  }
  
  hv = node->node_id ^ referent->common.hash_id;

  /* --- build new left token, add it to the hash table --- */
  token_added(node);
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &New);
  new_left_token (New, node, tok, w);
  insert_token_into_left_ht (thisAgent, New, hv);
  New->a.ht.referent = referent;

  if (mp_bnode_is_left_unlinked(node)) {
    activation_exit_sanity_check();
    return;
  }

  am = node->b.posneg.alpha_mem_;

  if (node_is_right_unlinked(node)) {
    relink_to_right_mem (node);
    if (am->right_mems==NIL) {
     make_mp_bnode_left_unlinked (node);
      activation_exit_sanity_check();
      return;
    }
  }

  /* --- look through right memory for matches --- */
  right_hv = am->am_id ^ referent->common.hash_id;
  for (rm=right_ht_bucket(thisAgent, right_hv); rm!=NIL; rm=rm->next_in_bucket) {
    if (rm->am != am) continue;
    /* --- does rm->w match new? --- */
    if (referent != rm->w->id) continue;
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, New, rm->w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found, so call each child node --- */
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,New,rm->w);
  }
  activation_exit_sanity_check();
}   


void unhashed_mp_node_left_addition (agent* thisAgent, rete_node *node, 
                                                                         token *tok, wme *w) {
  unsigned long hv;
  rete_node *child;
  token *New;
  right_mem *rm;
  rete_test *rt;
  Bool failed_a_test;

  activation_entry_sanity_check();
  left_node_activation(node,TRUE);

  hv = node->node_id;

  /* --- build new left token, add it to the hash table --- */
  token_added(node);
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &New);
  new_left_token (New, node, tok, w);
  insert_token_into_left_ht (thisAgent, New, hv);
  New->a.ht.referent = NIL;

  if (mp_bnode_is_left_unlinked(node)) return;

  if (node_is_right_unlinked(node)) {
    relink_to_right_mem (node);
    if (node->b.posneg.alpha_mem_->right_mems==NIL) {
      make_mp_bnode_left_unlinked (node);
      activation_exit_sanity_check();
      return;
    }
  }

  /* --- look through right memory for matches --- */
  for (rm=node->b.posneg.alpha_mem_->right_mems; rm!=NIL; 
rm=rm->next_in_am) {
    /* --- does rm->w match new? --- */
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, New, rm->w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found, so call each child node --- */
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,New,rm->w);
  }
  activation_exit_sanity_check();    
}

void positive_node_right_addition (agent* thisAgent, rete_node *node, wme *w) {
  unsigned long hv;
  token *tok;
  Symbol *referent;
  rete_test *rt;
  Bool failed_a_test;
  rete_node *child;

  activation_entry_sanity_check();
  right_node_activation(node,TRUE);

  if (node_is_left_unlinked(node)) {
    relink_to_left_mem (node);
    if (! node->parent->a.np.tokens) {
      unlink_from_right_mem (node);
      activation_exit_sanity_check();
      return;
    }
  }

  referent = w->id;
  hv = node->parent->node_id ^ referent->common.hash_id;

  for (tok=left_ht_bucket(thisAgent, hv); tok!=NIL; tok=tok->a.ht.next_in_bucket) {
    if (tok->node != node->parent) continue;
    /* --- does tok match w? --- */
    if (tok->a.ht.referent != referent) continue;
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, tok, w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found, so call each child node --- */
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,tok,w);
  }
  activation_exit_sanity_check();
}

void unhashed_positive_node_right_addition (agent* thisAgent, rete_node *node, wme *w) {
  unsigned long hv;
  token *tok;
  rete_test *rt;
  Bool failed_a_test;
  rete_node *child;

  activation_entry_sanity_check();
  right_node_activation(node,TRUE);

  if (node_is_left_unlinked(node)) {
    relink_to_left_mem (node);
    if (! node->parent->a.np.tokens) {
      unlink_from_right_mem (node);
      activation_exit_sanity_check();
      return;
    }
  }

  hv = node->parent->node_id;

  for (tok=left_ht_bucket(thisAgent, hv); tok!=NIL; tok=tok->a.ht.next_in_bucket) {
    if (tok->node != node->parent) continue;
    /* --- does tok match w? --- */
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, tok, w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found, so call each child node --- */
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,tok,w);
  }
  activation_exit_sanity_check();
}

void mp_node_right_addition (agent* thisAgent, rete_node *node, wme *w) {
  unsigned long hv;
  token *tok;
  Symbol *referent;
  rete_test *rt;
  Bool failed_a_test;
  rete_node *child;

  activation_entry_sanity_check();
  right_node_activation(node,TRUE);

  if (mp_bnode_is_left_unlinked(node)) {
    make_mp_bnode_left_linked (node);
    if (! node->a.np.tokens) {
      unlink_from_right_mem (node);
      activation_exit_sanity_check();
      return;
    }
  }

  referent = w->id;
  hv = node->node_id ^ referent->common.hash_id;

  for (tok=left_ht_bucket(thisAgent, hv); tok!=NIL; tok=tok->a.ht.next_in_bucket) {
    if (tok->node != node) continue;
    /* --- does tok match w? --- */
    if (tok->a.ht.referent != referent) continue;
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, tok, w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found, so call each child node --- */
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,tok,w);
  }
  activation_exit_sanity_check();
}

void unhashed_mp_node_right_addition (agent* thisAgent, rete_node *node, wme *w) {
  unsigned long hv;
  token *tok;
  rete_test *rt;
  Bool failed_a_test;
  rete_node *child;

  activation_entry_sanity_check();
  right_node_activation(node,TRUE);

  if (mp_bnode_is_left_unlinked(node)) {
    make_mp_bnode_left_linked (node);
    if (! node->a.np.tokens) {
      unlink_from_right_mem (node);
      activation_exit_sanity_check();
      return;
    }
  }

  hv = node->node_id;

  for (tok=left_ht_bucket(thisAgent, hv); tok!=NIL; tok=tok->a.ht.next_in_bucket) {
    if (tok->node != node) continue;
    /* --- does tok match w? --- */
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, tok, w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found, so call each child node --- */
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,tok,w);
  }
  activation_exit_sanity_check();
}

/* ************************************************************************

   SECTION 13:  Beta Node Interpreter Routines: Negative Nodes

************************************************************************ */

void negative_node_left_addition (agent* thisAgent, rete_node *node, 
                                                                  token *tok, wme *w) {
  unsigned long hv, right_hv;
  Symbol *referent;
  right_mem *rm;
  alpha_mem *am;
  rete_test *rt;
  Bool failed_a_test;
  rete_node *child;
  token *New;

  activation_entry_sanity_check();
  left_node_activation(node,TRUE);

  if (node_is_right_unlinked(node)) relink_to_right_mem (node);

  {
    int levels_up;
    token *t;
    
    levels_up = node->left_hash_loc_levels_up;
    if (levels_up==1) {
      referent = field_from_wme (w, node->left_hash_loc_field_num);
    } else { /* --- levels_up > 1 --- */
      for (t=tok, levels_up -= 2; levels_up!=0; levels_up--) t=t->parent;
      referent = field_from_wme (t->w, node->left_hash_loc_field_num);
    }
  }
  
  hv = node->node_id ^ referent->common.hash_id;

  /* --- build new token, add it to the hash table --- */
  token_added(node);
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &New);
  new_left_token (New, node, tok, w);
  insert_token_into_left_ht (thisAgent, New, hv);
  New->a.ht.referent = referent;
  New->negrm_tokens = NIL;
  
  /* --- look through right memory for matches --- */
  am = node->b.posneg.alpha_mem_;
  right_hv = am->am_id ^ referent->common.hash_id;
  for (rm=right_ht_bucket(thisAgent, right_hv); rm!=NIL; rm=rm->next_in_bucket) {
    if (rm->am != am) continue;
    /* --- does rm->w match new? --- */
    if (referent != rm->w->id) continue;
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, New, rm->w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    { token *t;
      allocate_with_pool (thisAgent, &thisAgent->token_pool, &t);
      t->node = node;
      t->parent = NIL;
      t->w = rm->w;
      t->a.neg.left_token = New;
      insert_at_head_of_dll (rm->w->tokens, t, next_from_wme, prev_from_wme);
      t->first_child = NIL;
      insert_at_head_of_dll (New->negrm_tokens, t,
                             a.neg.next_negrm, a.neg.prev_negrm);
    }
  }

  /* --- if no matches were found, call each child node --- */
  if (! New->negrm_tokens) {
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,New,NIL);
  }
  activation_exit_sanity_check();
}

void unhashed_negative_node_left_addition (agent* thisAgent, rete_node *node, 
                                                                                   token *tok, wme *w) {
  unsigned long hv;
  rete_test *rt;
  Bool failed_a_test;
  right_mem *rm;
  rete_node *child;
  token *New;

  activation_entry_sanity_check();
  left_node_activation(node,TRUE);

  if (node_is_right_unlinked(node)) relink_to_right_mem (node);

  hv = node->node_id;

  /* --- build new token, add it to the hash table --- */
  token_added(node);
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &New);
  new_left_token (New, node, tok, w);
  insert_token_into_left_ht (thisAgent, New, hv);
  New->a.ht.referent = NIL;
  New->negrm_tokens = NIL;

  /* --- look through right memory for matches --- */
  for (rm=node->b.posneg.alpha_mem_->right_mems; rm!=NIL; rm=rm->next_in_am) {
    /* --- does rm->w match new? --- */
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, New, rm->w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    { token *t;
      allocate_with_pool (thisAgent, &thisAgent->token_pool, &t);
      t->node = node;
      t->parent = NIL;
      t->w = rm->w;
      t->a.neg.left_token = New;
      insert_at_head_of_dll (rm->w->tokens, t,next_from_wme,prev_from_wme);
      t->first_child = NIL;
      insert_at_head_of_dll (New->negrm_tokens, t,
                             a.neg.next_negrm, a.neg.prev_negrm);
    }
  }

  /* --- if no matches were found, call each child node --- */
  if (! New->negrm_tokens) {
    for (child=node->first_child; child!=NIL; child=child->next_sibling)
      (*(left_addition_routines[child->node_type]))(thisAgent,child,New,NIL);
  }
  activation_exit_sanity_check();
}

void negative_node_right_addition (agent* thisAgent, rete_node *node, wme *w) {
  unsigned long hv;
  token *tok;
  Symbol *referent;
  rete_test *rt;
  Bool failed_a_test;

  activation_entry_sanity_check();
  right_node_activation(node,TRUE);

  referent = w->id;
  hv = node->node_id ^ referent->common.hash_id;

  for (tok=left_ht_bucket(thisAgent, hv); tok!=NIL; tok=tok->a.ht.next_in_bucket) {
    if (tok->node != node) continue;
    /* --- does tok match w? --- */
    if (tok->a.ht.referent != referent) continue;
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, tok, w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found: build new negrm token, remove descendent tokens --- */
    { token *t;
      allocate_with_pool (thisAgent, &thisAgent->token_pool, &t);
      t->node = node;
      t->parent = NIL;
      t->w = w;
      t->a.neg.left_token = tok;
      insert_at_head_of_dll (w->tokens, t, next_from_wme, prev_from_wme);
      t->first_child = NIL;
      insert_at_head_of_dll (tok->negrm_tokens, t,
                             a.neg.next_negrm, a.neg.prev_negrm);
    }
    while (tok->first_child) remove_token_and_subtree (thisAgent, tok->first_child);
  }
  activation_exit_sanity_check();
}

void unhashed_negative_node_right_addition (agent* thisAgent, rete_node *node, wme *w) {
  unsigned long hv;
  token *tok;
  rete_test *rt;
  Bool failed_a_test;

  activation_entry_sanity_check();
  right_node_activation(node,TRUE);

  hv = node->node_id;

  for (tok=left_ht_bucket(thisAgent, hv); tok!=NIL; tok=tok->a.ht.next_in_bucket) {
    if (tok->node != node) continue;
    /* --- does tok match w? --- */
    failed_a_test = FALSE;
    for (rt=node->b.posneg.other_tests; rt!=NIL; rt=rt->next)
      if (! match_left_and_right (thisAgent, rt, tok, w)) {
        failed_a_test = TRUE;
        break;
      }
    if (failed_a_test) continue;
    /* --- match found: build new negrm token, remove descendent tokens --- */
    { token *t;
      allocate_with_pool (thisAgent, &thisAgent->token_pool, &t);
      t->node = node;
      t->parent = NIL;
      t->w = w;
      t->a.neg.left_token = tok;
      insert_at_head_of_dll (w->tokens, t, next_from_wme, prev_from_wme);
      t->first_child = NIL;
      insert_at_head_of_dll (tok->negrm_tokens, t,
                             a.neg.next_negrm, a.neg.prev_negrm);
    }
    while (tok->first_child) remove_token_and_subtree (thisAgent, tok->first_child);
  }
  activation_exit_sanity_check();
}

/* ************************************************************************

   SECTION 14:  Beta Node Interpreter Routines: CN and CN_PARTNER Nodes

   These routines can support either the CN node hearing about new left
   tokens before the CN_PARTNER, or vice-versa.  This makes them a bit
   more complex than they would be otherwise.
************************************************************************ */

void cn_node_left_addition (agent* thisAgent, rete_node *node, token *tok, wme *w) {
  unsigned long hv;
  token *t, *New;
  rete_node *child;

  activation_entry_sanity_check();
  left_node_activation(node,TRUE);

  hv = node->node_id ^ (unsigned long)tok ^ (unsigned long)w;

  /* --- look for a matching left token (since the partner node might have
     heard about this new token already, in which case it would have done
     the CN node's work already); if found, exit --- */
  for (t=left_ht_bucket(thisAgent, hv); t!=NIL; t=t->a.ht.next_in_bucket)
    if ((t->node==node)&&(t->parent==tok)&&(t->w==w)) return;

  /* --- build left token, add it to the hash table --- */
  token_added(node);
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &New);
  new_left_token (New, node, tok, w);
  insert_token_into_left_ht (thisAgent, New, hv);
  New->negrm_tokens = NIL;
 
  /* --- pass the new token on to each child node --- */
  for (child=node->first_child; child!=NIL; child=child->next_sibling)
    (*(left_addition_routines[child->node_type]))(thisAgent,child,New,NIL);

  activation_exit_sanity_check();
}

void cn_partner_node_left_addition (agent* thisAgent, rete_node *node, 
                                                                        token *tok, wme *w) {
  rete_node *partner, *temp;
  unsigned long hv;
  token *left, *negrm_tok;
  
  activation_entry_sanity_check();
  left_node_activation(node,TRUE);

  partner = node->b.cn.partner;

  /* --- build new negrm token --- */
  token_added(node);
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &negrm_tok);
  new_left_token (negrm_tok, node, tok, w);

  /* --- advance (tok,w) up to the token from the top of the branch --- */
  temp = node->parent;
  while (temp != partner->parent) {
    temp = real_parent_node (temp);
    w = tok->w;
    tok = tok->parent;
  }

  /* --- look for the matching left token --- */
  hv = partner->node_id ^ (unsigned long)tok ^ (unsigned long)w;
  for (left=left_ht_bucket(thisAgent, hv); left!=NIL; left=left->a.ht.next_in_bucket)
    if ((left->node==partner)&&(left->parent==tok)&&(left->w==w)) break;

  /* --- if not found, create a new left token --- */
  if (!left) {
    token_added(partner);
    allocate_with_pool (thisAgent, &thisAgent->token_pool, &left);
    new_left_token (left, partner, tok, w);
    insert_token_into_left_ht (thisAgent, left, hv);
    left->negrm_tokens = NIL;
  }

  /* --- add new negrm token to the left token --- */
  negrm_tok->a.neg.left_token = left;
  insert_at_head_of_dll (left->negrm_tokens, negrm_tok,
                         a.neg.next_negrm, a.neg.prev_negrm);

  /* --- remove any descendent tokens of the left token --- */
  while (left->first_child) remove_token_and_subtree (thisAgent, left->first_child);

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
   inside the NCC).  When one of these "stobe" situations occurs,
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

void p_node_left_addition (agent* thisAgent, rete_node *node, token *tok, wme *w) {
  ms_change *msc;
  condition *cond;
  token *current_token, *New;
  wme *current_wme;
  rete_node *current_node;
  Bool match_found;


  /* RCHONG: begin 10.11 */

  int prod_type;
  token *OPERAND_curr_tok, *temp_tok;

  action    *act;
  Bool      operator_proposal,op_elab;
  char      action_attr[50];

  int pass;
  wme *lowest_goal_wme;

  /* RCHONG: end 10.11 */

  activation_entry_sanity_check();
  left_node_activation(node,TRUE);

  /* --- build new left token (used only for tree-based remove) --- */
  token_added(node);
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &New);
  new_left_token (New, node, tok, w);

  /* --- check for match in tentative_retractions --- */
  match_found = FALSE;
  for (msc=node->b.p.tentative_retractions; msc!=NIL; msc=msc->next_of_node) {
    match_found = TRUE;
    cond = msc->inst->bottom_of_instantiated_conditions;
    current_token = tok;
    current_wme = w;
    current_node = node->parent;
    while (current_node->node_type!=DUMMY_TOP_BNODE) {
      if (bnode_is_positive(current_node->node_type))
        if (current_wme != cond->bt.wme_) {
          match_found=FALSE; break;
        }
      current_node = real_parent_node (current_node);
      current_wme = current_token->w;
      current_token = current_token->parent;
      cond = cond->prev;
    }
    if (match_found) break;
  }

#ifdef BUG_139_WORKAROUND
    /* --- test workaround for bug #139: don't rematch justifications; let them be removed --- */
    /* note that the justification is added to the retraction list when it is first created, so
       we let it match the first time, but not after that */
    if (match_found && node->b.p.prod->type == JUSTIFICATION_PRODUCTION_TYPE) {
        if (node->b.p.prod->already_fired) {
            return;
        } else {
            node->b.p.prod->already_fired = 1;
        }
    }
#endif

  /* --- if match found tentative_retractions, remove it --- */
  if (match_found) {
    msc->inst->rete_token = tok;
    msc->inst->rete_wme = w;
    remove_from_dll (node->b.p.tentative_retractions, msc,
                     next_of_node, prev_of_node);
    remove_from_dll (thisAgent->ms_retractions, msc, next, prev);
    /* REW: begin 08.20.97 */
    if (msc->goal) {
      remove_from_dll (msc->goal->id.ms_retractions, msc,
                       next_in_level, prev_in_level);
    } else {
	  // BUGBUG FIXME BADBAD TODO
	  // RPM 6/05
	  // This if statement is to avoid a crash we get on most platforms in Soar 7 mode
	  // It's unknown what consequences it has, but the Soar 7 demos seem to work
	  // To return things to how they were, simply remove the if statement (but leave
	  //  the remove_from_dll line).
	  if(thisAgent->nil_goal_retractions)
         remove_from_dll (thisAgent->nil_goal_retractions,
                       msc, next_in_level, prev_in_level); 
    }
    /* REW: end   08.20.97 */
    
    free_with_pool (&thisAgent->ms_change_pool, msc);
#ifdef DEBUG_RETE_PNODES
    print_with_symbols (thisAgent, "\nRemoving tentative retraction: %y",
                        node->b.p.prod->name);
#endif
    activation_exit_sanity_check();
    return;
  }

  /* --- no match found, so add new assertion --- */
#ifdef DEBUG_RETE_PNODES
  print_with_symbols (thisAgent, "\nAdding tentative assertion: %y",
                      node->b.p.prod->name);
#endif

  allocate_with_pool (thisAgent, &thisAgent->ms_change_pool, &msc);
  msc->tok = tok;
  msc->w = w;
  msc->p_node = node;
  msc->inst = NIL;  /* just for safety */
  /* REW: begin 08.20.97 */
  /* initialize goal regardless of run mode */
  msc->level = 0;
  msc->goal = NIL;
  /* REW: end   08.20.97 */

/* RCHONG: begin 10.11 */

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

  /* REW: begin 09.15.96 */
  if (thisAgent->operand2_mode == TRUE) {

    /* REW: begin 08.20.97 */
     /* Find the goal and level for this ms change */
     msc->goal = find_goal_for_match_set_change_assertion(thisAgent, msc);
     msc->level = msc->goal->id.level;
#ifdef DEBUG_WATERFALL
     print("\n    Level of goal is  %d", msc->level);
#endif
     /* REW: end 08.20.97 */

     prod_type = IE_PRODS;

     if (node->b.p.prod->declared_support == DECLARED_O_SUPPORT)
        prod_type = PE_PRODS;

     else if (node->b.p.prod->declared_support == DECLARED_I_SUPPORT)
        prod_type = IE_PRODS;

     else if (node->b.p.prod->declared_support == UNDECLARED_SUPPORT) {

        /*
        check if the instantiation is proposing an operator.  if it
        is, then this instantiation is i-supported.
        */

        operator_proposal = FALSE;
        
        for (act = node->b.p.prod->action_list; act != NIL ; act = act->next) {
                        if ((act->type == MAKE_ACTION) &&
                                (rhs_value_is_symbol(act->attr))) {
                                if ((strcmp(rhs_value_to_string (thisAgent, act->attr, action_attr, 50),
                                        "operator") == NIL) &&
                                        (act->preference_type == ACCEPTABLE_PREFERENCE_TYPE)) {
                                        operator_proposal = TRUE;
                                        prod_type = !PE_PRODS;
                                        break;
                                }
                        }
                }

		if (operator_proposal == FALSE) {

			/*
			examine all the different matches for this productions
			*/

			for (OPERAND_curr_tok = node->a.np.tokens;
				OPERAND_curr_tok != NIL;
				OPERAND_curr_tok = OPERAND_curr_tok->next_of_node) {

					/*

					i'll need to make two passes over each set of wmes that
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

					Modified 1/00 by KJC for operand2_mode == TRUE  AND
					o-support-mode == 3:  prods that have ONLY operator 
					elaborations (<o> ^attr ^value) are IE_PROD.  If prod has
					both operator applications and <o> elabs, then it's PE_PROD 
					and the user is warned that <o> elabs will be o-supported.

					*/
					op_elab = FALSE;
					lowest_goal_wme = NIL;

					for (pass = 0; pass != 2; pass++) {

						temp_tok = OPERAND_curr_tok;
						while (temp_tok != NIL) {
							while (temp_tok->w == NIL) {
								temp_tok = temp_tok->parent;
								if (temp_tok == NIL) break;
							}
							if (temp_tok == NIL)  break;
							if (temp_tok->w == NIL) break;

							if (pass == 0) {
								if (temp_tok->w->id->id.isa_goal == TRUE) {
									if (lowest_goal_wme == NIL)
										lowest_goal_wme = temp_tok->w;
									else {
										if (temp_tok->w->id->id.level >
											lowest_goal_wme->id->id.level)
											lowest_goal_wme = temp_tok->w;
									}
								}
							} else {
								if ((temp_tok->w->attr == thisAgent->operator_symbol) && (temp_tok->w->acceptable == FALSE) && (temp_tok->w->id == lowest_goal_wme->id)) {
										if ((thisAgent->o_support_calculation_type == 3) || (thisAgent->o_support_calculation_type == 4)) {
											/* iff RHS has only operator elaborations 
											then it's IE_PROD, otherwise PE_PROD, so
											look for non-op-elabs in the actions  KJC 1/00 */


											/* We also need to check reteloc's to see if they
											are referring to operator augmentations before determining
											if this is an operator elaboration
											*/

											for (act = node->b.p.prod->action_list; act != NIL ; act = act->next) {
												if (act->type == MAKE_ACTION) {
													if ((rhs_value_is_symbol(act->id)) &&

														/** shouldn't this be either 
														symbol_to_rhs_value (act->id) ==  or
														act->id == rhs_value_to_symbol(temp..)**/
														(rhs_value_to_symbol(act->id) == 
														temp_tok->w->value)) {
															op_elab = TRUE;
														} else if ( (thisAgent->o_support_calculation_type == 4) 
															&& (rhs_value_is_reteloc(act->id)) 
															&& (temp_tok->w->value == get_symbol_from_rete_loc( (byte)rhs_value_to_reteloc_levels_up(act->id),(byte)rhs_value_to_reteloc_field_num(act->id), tok, w ))) {
															op_elab = TRUE;
														} else {
															/* this is not an operator elaboration */
															prod_type = PE_PRODS;
														}
												} // act->type == MAKE_ACTION
											} // for
										} else {                                        
											prod_type = PE_PRODS;
											break;
										}
									}
							} /* end if (pass == 0) ... */
							temp_tok = temp_tok->parent;
						}  /* end while (temp_tok != NIL) ... */

						if (prod_type == PE_PRODS)
							if ((thisAgent->o_support_calculation_type != 3) && (thisAgent->o_support_calculation_type != 4)) break;
							else if (op_elab == TRUE) {

								/* warn user about mixed actions */

								if ((thisAgent->o_support_calculation_type == 3) && thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) {
									print_with_symbols(thisAgent, "\nWARNING:  operator elaborations mixed with operator applications\nget o_support in prod %y",
																												node->b.p.prod->name);
									
                                    // XML generation
                                    growable_string gs = make_blank_growable_string(thisAgent);
                                    add_to_growable_string(thisAgent, &gs, "WARNING:  operator elaborations mixed with operator applications\nget o_support in prod ");
                                    add_to_growable_string(thisAgent, &gs, symbol_to_string(thisAgent, node->b.p.prod->name, true, 0, 0));
                                    GenerateWarningXML(thisAgent, text_of_growable_string(gs));
                                    free_growable_string(thisAgent, gs);

									prod_type = PE_PRODS;
									break;
								}
								else if ((thisAgent->o_support_calculation_type == 4)  && thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) {
									print_with_symbols(thisAgent, "\nWARNING:  operator elaborations mixed with operator applications\nget i_support in prod %y",
																												node->b.p.prod->name);

                                    // XML generation
                                    growable_string gs = make_blank_growable_string(thisAgent);
                                    add_to_growable_string(thisAgent, &gs, "WARNING:  operator elaborations mixed with operator applications\nget i_support in prod ");
                                    add_to_growable_string(thisAgent, &gs, symbol_to_string(thisAgent, node->b.p.prod->name, true, 0, 0));
                                    GenerateWarningXML(thisAgent, text_of_growable_string(gs));
                                    free_growable_string(thisAgent, gs);

									prod_type = IE_PRODS;
									break;
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

		}  /* end if (operator_proposal == FALSE) */
        
     }        /* end UNDECLARED_SUPPORT */
         
     if (prod_type == PE_PRODS) {
       insert_at_head_of_dll (thisAgent->ms_o_assertions, msc, next, prev);

       /* REW: begin 08.20.97 */
       insert_at_head_of_dll (msc->goal->id.ms_o_assertions,
                              msc, next_in_level, prev_in_level);
       /* REW: end   08.20.97 */


        node->b.p.prod->OPERAND_which_assert_list = O_LIST;

        if (thisAgent->soar_verbose_flag == TRUE) {
           print_with_symbols(thisAgent, "\n   RETE: putting [%y] into ms_o_assertions",
                              node->b.p.prod->name);
           char buf[256];
           snprintf(buf, 254, "RETE: putting [%s] into ms_o_assertions", symbol_to_string(thisAgent, node->b.p.prod->name, true, 0, 0));
           GenerateVerboseXML(thisAgent, buf);
        }
     }

     else { 
       insert_at_head_of_dll (thisAgent->ms_i_assertions,
                              msc, next, prev); 

       /* REW: end 08.20.97 */
       insert_at_head_of_dll (msc->goal->id.ms_i_assertions,
                              msc, next_in_level, prev_in_level);
       /* REW: end 08.20.97 */

        node->b.p.prod->OPERAND_which_assert_list = I_LIST;

        if (thisAgent->soar_verbose_flag == TRUE) {
           print_with_symbols(thisAgent, "\n   RETE: putting [%y] into ms_i_assertions",
                              node->b.p.prod->name);
           char buf[256];
           snprintf(buf, 254, "RETE: putting [%s] into ms_i_assertions", symbol_to_string(thisAgent, node->b.p.prod->name, true, 0, 0));
           GenerateVerboseXML(thisAgent, buf);
        }
     }
  }
  /* REW: end   09.15.96 */

  else /* non-Operand* flavor Soar */

     insert_at_head_of_dll (thisAgent->ms_assertions, msc, next, prev);

     ///
     // Location for Match Interrupt

  /* RCHONG: end 10.11 */

  insert_at_head_of_dll (node->b.p.tentative_assertions, msc,
                         next_of_node, prev_of_node);
  activation_exit_sanity_check();
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

void p_node_left_removal (agent* thisAgent, rete_node *node, token *tok, wme *w) {
  ms_change *msc;
  instantiation *inst;

  activation_entry_sanity_check();
  
  /* --- check for match in tentative_assertions --- */
  for (msc=node->b.p.tentative_assertions; msc!=NIL; msc=msc->next_of_node) {
    if ((msc->tok==tok) && (msc->w==w)) {
      /* --- match found in tentative_assertions, so remove it --- */
      remove_from_dll (node->b.p.tentative_assertions, msc,
                       next_of_node, prev_of_node);


      /* REW: begin 09.15.96 */
      if (thisAgent->operand2_mode == TRUE) {
         if (node->b.p.prod->OPERAND_which_assert_list == O_LIST) {
            remove_from_dll (thisAgent->ms_o_assertions, msc, next, prev);
            /* REW: begin 08.20.97 */
            /* msc already defined for the assertion so the goal should be defined
               as well. */
            remove_from_dll (msc->goal->id.ms_o_assertions, msc,
                             next_in_level, prev_in_level);
            /* REW: end   08.20.97 */
         }
         else if (node->b.p.prod->OPERAND_which_assert_list == I_LIST) {
            remove_from_dll (thisAgent->ms_i_assertions, msc, next, prev);
            /* REW: begin 08.20.97 */
            remove_from_dll (msc->goal->id.ms_i_assertions, msc,
                             next_in_level, prev_in_level);
            /* REW: end   08.20.97 */
         }
      }
      /* REW: end   09.15.96 */

      else


      remove_from_dll (thisAgent->ms_assertions, msc, next, prev);
      free_with_pool (&thisAgent->ms_change_pool, msc);
#ifdef DEBUG_RETE_PNODES
      print_with_symbols (thisAgent, "\nRemoving tentative assertion: %y",
                          node->b.p.prod->name);
#endif
      activation_exit_sanity_check();
      return;
    }
  } /* end of for loop */

  /* --- find the instantiation corresponding to this token --- */
  for (inst=node->b.p.prod->instantiations; inst!=NIL; inst=inst->next)
    if ((inst->rete_token==tok)&&(inst->rete_wme==w)) break;

  if (inst) {
    /* --- add that instantiation to tentative_retractions --- */
#ifdef DEBUG_RETE_PNODES
    print_with_symbols (thisAgent, "\nAdding tentative retraction: %y",
                        node->b.p.prod->name);
#endif

    inst->rete_token = NIL;
    inst->rete_wme = NIL;
    allocate_with_pool (thisAgent, &thisAgent->ms_change_pool, &msc);
    msc->inst = inst;
    msc->p_node = node;
    msc->tok = NIL;     /* just for safety */
    msc->w = NIL;       /* just for safety */
    /* REW: begin 08.20.97 */
    msc->level = 0;     /* just for safety */
    msc->goal = NIL;    /* just for safety */
    /* REW: end   08.20.97 */
    insert_at_head_of_dll (node->b.p.tentative_retractions, msc,
                           next_of_node, prev_of_node);

    /* REW: begin 08.20.97 */
    
    if (thisAgent->operand2_mode) {
      /* Determine what the goal of the msc is and add it to that
         goal's list of retractions */
      msc->goal = find_goal_for_match_set_change_retraction(msc);
      msc->level = msc->goal->id.level;

#ifdef DEBUG_WATERFALL
      print("\n    Level of retraction is: %d", msc->level);
#endif

      if (msc->goal->id.link_count == 0) {
        /* BUG (potential) (Operand2/Waterfall: 2.101)
           When a goal is removed in the stack, it is not immediately garbage
           collected, meaning that the goal pointer is still valid when the 
           retraction is created.  So the goal for a retraction will always be
           valid, even though, for retractions caused by goal removals, the 
           goal will be removed at the next WM phase. (You can see this by
           printing the identifier for the goal in the elaboration cycle
           after goal removal.  It's still there, although nothing is attacjed
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
       insert_at_head_of_dll (thisAgent->ms_retractions, msc, next, prev);
       if (msc->goal) { /* Goal exists */
         insert_at_head_of_dll (msc->goal->id.ms_retractions, msc,
                                next_in_level, prev_in_level);
       }
       else { /* NIL Goal; put on the NIL Goal list */
         insert_at_head_of_dll (thisAgent->nil_goal_retractions,
                                msc, next_in_level, prev_in_level);
       }

#ifdef DEBUG_WATERFALL
        print_with_symbols(thisAgent, "\nRetraction: %y", msc->inst->prod->name); 
        print(" is active at level %d\n", msc->level); 

        { ms_change *assertion;
        print("\n Retractions list:\n");
        for (assertion=thisAgent->ms_retractions;
             assertion;
             assertion=assertion->next) {
          print_with_symbols(thisAgent, "     Retraction: %y ",
                             assertion->p_node->b.p.prod->name);
          print(" at level %d\n", assertion->level);
        } 

        if (thisAgent->nil_goal_retractions) {
          print("\nCurrent NIL Goal list:\n");
          assertion = NIL;
          for (assertion=thisAgent->nil_goal_retractions;
               assertion;
               assertion=assertion->next_in_level) {
            print_with_symbols(thisAgent, "     Retraction: %y ",
                               assertion->p_node->b.p.prod->name);
            print(" at level %d\n", assertion->level);
            if (assertion->goal) print("This assertion has non-NIL goal pointer.\n");
          } 
        }
        }
#endif 
    /* REW: end   08.20.97 */

    } else { /* For Reg. Soar just add it to the list */
          insert_at_head_of_dll (thisAgent->ms_retractions,
                                 msc, next, prev);
    }
    activation_exit_sanity_check();
    return;
  }

  /* REW: begin 09.15.96 */

  if ((thisAgent->operand2_mode == TRUE) &&
      (thisAgent->soar_verbose_flag == TRUE)) {
          print_with_symbols (thisAgent, "\n%y: ",node->b.p.prod->name);
          char buf[256];
          snprintf(buf, 254, "%s: ", symbol_to_string(thisAgent, node->b.p.prod->name, true, 0, 0));
          GenerateVerboseXML(thisAgent, buf);
      }

  /* REW: end   09.15.96 */

#ifdef BUG_139_WORKAROUND
    if (node->b.p.prod->type == JUSTIFICATION_PRODUCTION_TYPE) {
#ifdef BUG_139_WORKAROUND_WARNING
        print(thisAgent, "\nWarning: can't find an existing inst to retract (BUG 139 WORKAROUND)\n");
		GenerateWarningXML(thisAgent, "Warning: can't find an existing inst to retract (BUG 139 WORKAROUND)");
#endif
        return;
    }
#endif

  {      char msg[BUFFER_MSG_SIZE];
  strncpy (msg,
          "Internal error: can't find existing instantiation to retract\n", BUFFER_MSG_SIZE);
  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
  abort_with_fatal_error(thisAgent, msg);
  }
}

/* ************************************************************************

   SECTION 16:  Beta Node Interpreter Routines: Tree-Based Removal

   This routine does tree-based removal of a token and its descendents.
   Note that it uses a nonrecursive tree traversal; each iteration, the
   leaf being deleted is the leftmost leaf in the tree.
************************************************************************ */

void remove_token_and_subtree (agent* thisAgent, token *root) {
  rete_node *node, *child, *next;
  token *tok, *next_value_for_tok, *left, *t, *next_t;
  byte node_type;
  
  tok = root;
  
  while (TRUE) {
    /* --- move down to the leftmost leaf --- */
    while (tok->first_child) tok = tok->first_child;
    next_value_for_tok = tok->next_sibling ? tok->next_sibling : tok->parent;

    /* --- cleanup stuff common to all types of nodes --- */
    node = tok->node;
    left_node_activation(node,FALSE);
    fast_remove_from_dll (node->a.np.tokens, tok, token, next_of_node,
                          prev_of_node);
    fast_remove_from_dll (tok->parent->first_child, tok, token,
                          next_sibling,prev_sibling);
    if (tok->w) fast_remove_from_dll (tok->w->tokens, tok, token,
                                 next_from_wme, prev_from_wme);
    node_type = node->node_type;

    /* --- for merged Mem/Pos nodes --- */
    if ((node_type==MP_BNODE)||(node_type==UNHASHED_MP_BNODE)) {
      remove_token_from_left_ht (thisAgent, tok, node->node_id ^
                                 (tok->a.ht.referent ?
                                  tok->a.ht.referent->common.hash_id : 0));
      if (! mp_bnode_is_left_unlinked(node)) {
        if (! node->a.np.tokens) unlink_from_right_mem (node);
      }

    /* --- for P nodes --- */
    } else if (node_type==P_BNODE) {
      p_node_left_removal (thisAgent, node, tok->parent, tok->w);

    /* --- for Negative nodes --- */
    } else if ((node_type==NEGATIVE_BNODE) ||
               (node_type==UNHASHED_NEGATIVE_BNODE)) {
      remove_token_from_left_ht (thisAgent, tok, node->node_id ^
                                 (tok->a.ht.referent ?
                                  tok->a.ht.referent->common.hash_id : 0));
      if (! node->a.np.tokens) unlink_from_right_mem (node);
      for (t=tok->negrm_tokens; t!=NIL; t=next_t) {
        next_t = t->a.neg.next_negrm;
        fast_remove_from_dll(t->w->tokens,t,token,next_from_wme,prev_from_wme);
        free_with_pool (&thisAgent->token_pool, t);
      }

    /* --- for Memory nodes --- */
    } else if ((node_type==MEMORY_BNODE)||(node_type==UNHASHED_MEMORY_BNODE)) {
      remove_token_from_left_ht (thisAgent, tok, node->node_id ^
                                 (tok->a.ht.referent ?
                                  tok->a.ht.referent->common.hash_id : 0));
#ifdef DO_ACTIVATION_STATS_ON_REMOVALS
      /* --- if doing statistics stuff, then activate each attached node --- */
      for (child=node->b.mem.first_linked_child; child!=NIL; child=next) {
        next = child->a.pos.next_from_beta_mem;
        left_node_activation (child,FALSE);
      }
#endif
      /* --- for right unlinking, then if the beta memory just went to
         zero, right unlink any attached Pos nodes --- */
      if (! node->a.np.tokens) {
        for (child=node->b.mem.first_linked_child; child!=NIL; child=next) {
          next = child->a.pos.next_from_beta_mem;
          unlink_from_right_mem (child);
        }
      }

    /* --- for CN nodes --- */
    } else if (node_type==CN_BNODE) {
      remove_token_from_left_ht (thisAgent, tok, node->node_id ^
                                      (unsigned long)(tok->parent) ^
                                      (unsigned long)(tok->w));
      for (t=tok->negrm_tokens; t!=NIL; t=next_t) {
        next_t = t->a.neg.next_negrm;
        if (t->w) fast_remove_from_dll (t->w->tokens, t, token,
                                        next_from_wme, prev_from_wme);
        fast_remove_from_dll (t->node->a.np.tokens, t, token,
                              next_of_node, prev_of_node);
        fast_remove_from_dll (t->parent->first_child, t, token,
                              next_sibling, prev_sibling);
        free_with_pool (&thisAgent->token_pool, t);
      }

    /* --- for CN Partner nodes --- */
    } else if (node_type==CN_PARTNER_BNODE) {
      left = tok->a.neg.left_token;
      fast_remove_from_dll (left->negrm_tokens, tok, token,
                            a.neg.next_negrm, a.neg.prev_negrm);
      if (! left->negrm_tokens) { /* just went to 0, so call children */
        for (child=left->node->first_child; child!=NIL;
             child=child->next_sibling)
          (*(left_addition_routines[child->node_type]))(thisAgent,child,left,NIL);
      }

    } else {
      char msg[BUFFER_MSG_SIZE];
      snprintf (msg, BUFFER_MSG_SIZE,
              "Internal error: bad node type %d in remove_token_and_subtree\n",
              node->node_type);
     msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
     abort_with_fatal_error(thisAgent, msg);
    }
    
    free_with_pool (&thisAgent->token_pool, tok);
    if (tok==root) break; /* if leftmost leaf was the root, we're done */
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

     4 bytes: number of sym_constants
     4 bytes: number of variables
     4 bytes: number of int_constants
     4 bytes: number of float_constants
       names of all sym_constants (each a null-terminated string)
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
  from the given (already open) files.  They return TRUE if successful, 
  FALSE if any error occurred.
********************************************************************** */

FILE *rete_fs_file;  /* File handle we're using -- "fs" for "fast-save" */

/* ----------------------------------------------------------------------
                Save/Load Bytes, Short and Long Integers

   These are the lowest-level routines for accessing the FS file.  Note
   that all 16-bit or 32-bit words are written LSB first.  We do this 
   carefully, so that fastsave files will be portable across machine
   types (big-endian vs. little-endian).
---------------------------------------------------------------------- */

void retesave_one_byte (byte b, FILE* f) {
  fputc (b, rete_fs_file); 
}

byte reteload_one_byte (FILE* f) {
  return fgetc (f);
}

void retesave_two_bytes (unsigned long w, FILE* f) {
  retesave_one_byte ((byte)(w & 0xFF),f );
  retesave_one_byte ((byte)((w >> 8) & 0xFF),f );
}

unsigned long reteload_two_bytes (FILE* f) {
  unsigned long i;
  i = reteload_one_byte (f);
  i += (reteload_one_byte(f) << 8);
  return i;
}

void retesave_four_bytes (unsigned long w, FILE* f) {
  retesave_one_byte ((byte)(w & 0xFF),f);
  retesave_one_byte ((byte)((w >> 8) & 0xFF),f);
  retesave_one_byte ((byte)((w >> 16) & 0xFF),f);
  retesave_one_byte ((byte)((w >> 24) & 0xFF),f);
}

unsigned long reteload_four_bytes (FILE* f) {
  unsigned long i;
  i = reteload_one_byte (f);
  i += (reteload_one_byte(f) << 8);
  i += (reteload_one_byte(f) << 16);
  i += (reteload_one_byte(f) << 24);
  return i;
}

/* ----------------------------------------------------------------------
                            Save/Load Strings

   Strings are written as null-terminated sequences of characters, just
   like the usual C format.  Reteload_string() leaves the result in
   reteload_string_buf[].
---------------------------------------------------------------------- */

char reteload_string_buf[4*MAX_LEXEME_LENGTH];

void retesave_string (char *s, FILE* f) {
  while (*s) {
    retesave_one_byte (*s,f);
    s++;
  }
  retesave_one_byte (0,f);
}

void reteload_string (FILE* f) {
  int i, ch;
  i = 0;
  do {
    ch = reteload_one_byte(f);
    reteload_string_buf[i++] = ch;
  } while (ch);
}

/* ----------------------------------------------------------------------
                            Save/Load Symbols

   We write out symbol names once at the beginning of the file, and 
   thereafter refer to symbols using 32-bit index numbers instead of their
   full names.  Retesave_symbol_and_assign_index() writes out one symbol
   and assigns it an index (stored in sym->common.a.retesave_symindex).
   Index numbers are assigned sequentially -- the first symbol in the file
   has index number 1, the second has number 2, etc.  Retesave_symbol_table()
   saves the whole symbol table, using the following format:

       4 bytes: number of sym_constants
       4 bytes: number of variables
       4 bytes: number of int_constants
       4 bytes: number of float_constants
         names of all sym_constants (each a null-terminated string)
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

Bool retesave_symbol_and_assign_index (agent* thisAgent, void *item, FILE* f) {
  Symbol *sym;

  sym = static_cast<symbol_union *>(item);
  thisAgent->current_retesave_symindex++;
  sym->common.a.retesave_symindex = thisAgent->current_retesave_symindex;
  retesave_string (symbol_to_string (thisAgent, sym, FALSE, NIL, 0), f);
  return FALSE;
}

void retesave_symbol_table (agent* thisAgent, FILE* f) {
  thisAgent->current_retesave_symindex = 0;

  retesave_four_bytes (thisAgent->sym_constant_hash_table->count,f);
  retesave_four_bytes (thisAgent->variable_hash_table->count,f);
  retesave_four_bytes (thisAgent->int_constant_hash_table->count,f);
  retesave_four_bytes (thisAgent->float_constant_hash_table->count,f);

  do_for_all_items_in_hash_table (thisAgent, thisAgent->sym_constant_hash_table,
                                  retesave_symbol_and_assign_index,f);
  do_for_all_items_in_hash_table (thisAgent, thisAgent->variable_hash_table,
                                  retesave_symbol_and_assign_index,f);
  do_for_all_items_in_hash_table (thisAgent, thisAgent->int_constant_hash_table,
                                  retesave_symbol_and_assign_index,f);
  do_for_all_items_in_hash_table (thisAgent, thisAgent->float_constant_hash_table,
                                  retesave_symbol_and_assign_index,f);
}

void reteload_all_symbols (agent* thisAgent, FILE* f) {
  unsigned long num_sym_constants, num_variables;
  unsigned long num_int_constants, num_float_constants;
  Symbol **current_place_in_symtab;
  unsigned long i;

  num_sym_constants = reteload_four_bytes(f);
  num_variables = reteload_four_bytes(f);
  num_int_constants = reteload_four_bytes(f);
  num_float_constants = reteload_four_bytes(f);

  thisAgent->reteload_num_syms = num_sym_constants + num_variables + num_int_constants
    + num_float_constants;

  /* --- allocate memory for the symbol table --- */
  thisAgent->reteload_symbol_table = (Symbol **)
    allocate_memory (thisAgent, thisAgent->reteload_num_syms*sizeof(char *),MISCELLANEOUS_MEM_USAGE);
  
  /* --- read in all the symbols from the file --- */
  current_place_in_symtab = thisAgent->reteload_symbol_table;
  for (i=0; i<num_sym_constants; i++) {
    reteload_string(f);
    *(current_place_in_symtab++) = make_sym_constant (thisAgent, reteload_string_buf);
  }
  for (i=0; i<num_variables; i++) {
    reteload_string(f);
    *(current_place_in_symtab++) = make_variable (thisAgent, reteload_string_buf);
  }
  for (i=0; i<num_int_constants; i++) {
    reteload_string(f);
    *(current_place_in_symtab++) =
      make_int_constant (thisAgent, strtol(reteload_string_buf,NULL,10));
  }
  for (i=0; i<num_float_constants; i++) {
    reteload_string(f);
    *(current_place_in_symtab++) = 
      make_float_constant (thisAgent, (float)my_strtod(reteload_string_buf,NULL,10));
  }
}

Symbol *reteload_symbol_from_index (agent* thisAgent, FILE* f) {
  unsigned long index;
  
  index = reteload_four_bytes(f);
  if (index==0) return NIL;
  index--;
  if (index >= thisAgent->reteload_num_syms) {
    char msg[BUFFER_MSG_SIZE];
    strncpy (msg, "Internal error (file corrupted?): symbol count too small\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
  }
  return *(thisAgent->reteload_symbol_table+index);
}

void reteload_free_symbol_table (agent* thisAgent) {
  unsigned long i;

  for (i=0; i<thisAgent->reteload_num_syms; i++)
    symbol_remove_ref (thisAgent, *(thisAgent->reteload_symbol_table+i));
  free_memory (thisAgent, thisAgent->reteload_symbol_table, MISCELLANEOUS_MEM_USAGE);
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

Bool retesave_alpha_mem_and_assign_index (agent* thisAgent, void *item, FILE* f) {
  alpha_mem *am;

  am = static_cast<alpha_mem_struct *>(item);
  thisAgent->current_retesave_amindex++;
  am->retesave_amindex = thisAgent->current_retesave_amindex;
  retesave_four_bytes (am->id ? am->id->common.a.retesave_symindex : 0,f);
  retesave_four_bytes (am->attr ? am->attr->common.a.retesave_symindex : 0,f);
  retesave_four_bytes (am->value ? am->value->common.a.retesave_symindex : 0,f);
  retesave_one_byte ((byte)(am->acceptable ? 1 : 0),f);
  return FALSE;
}

void retesave_alpha_memories (agent* thisAgent, FILE* f) {
  unsigned long i, num_ams;

  thisAgent->current_retesave_amindex = 0;
  num_ams = 0;
  for (i=0; i<16; i++) num_ams += thisAgent->alpha_hash_tables[i]->count;
  retesave_four_bytes (num_ams,f);
  for (i=0; i<16; i++)
    do_for_all_items_in_hash_table (thisAgent, thisAgent->alpha_hash_tables[i],
                                    retesave_alpha_mem_and_assign_index,f);
}

void reteload_alpha_memories (agent* thisAgent, FILE* f) {
  unsigned long i;
  Symbol *id, *attr, *value;
  Bool acceptable;

  thisAgent->reteload_num_ams = reteload_four_bytes(f);
  thisAgent->reteload_am_table = (alpha_mem **)
    allocate_memory (thisAgent, thisAgent->reteload_num_ams*sizeof(char *),MISCELLANEOUS_MEM_USAGE);
  for (i=0; i<thisAgent->reteload_num_ams; i++) {
    id = reteload_symbol_from_index(thisAgent,f);
    attr = reteload_symbol_from_index(thisAgent,f);
    value = reteload_symbol_from_index(thisAgent,f);
    acceptable = reteload_one_byte(f) ? TRUE : FALSE;
    *(thisAgent->reteload_am_table+i) = find_or_make_alpha_mem (thisAgent,id,attr,value,acceptable);
  }
}

alpha_mem *reteload_am_from_index (agent* thisAgent, FILE* f) {
  unsigned long amindex;

  amindex = reteload_four_bytes(f) - 1;
  if (amindex >= thisAgent->reteload_num_ams) {
    char msg[BUFFER_MSG_SIZE];
    strncpy (msg,
            "Internal error (file corrupted?): alpha mem count too small\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
  }
  return *(thisAgent->reteload_am_table+amindex);
}

void reteload_free_am_table (agent* thisAgent) {
  unsigned long i;

  for (i=0; i<thisAgent->reteload_num_ams; i++)
    remove_ref_to_alpha_mem (thisAgent, *(thisAgent->reteload_am_table+i));
  free_memory (thisAgent, thisAgent->reteload_am_table, MISCELLANEOUS_MEM_USAGE);
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

void retesave_varnames (varnames *names, FILE* f) {
  list *c;
  unsigned long i;
  Symbol *sym;

  if (! names) {
    retesave_one_byte (0,f);
  } else if (varnames_is_one_var(names)) {
    retesave_one_byte (1,f);
    sym = varnames_to_one_var (names);
    retesave_four_bytes (sym->common.a.retesave_symindex,f);
  } else {
    retesave_one_byte (2,f);
    for (i=0, c=varnames_to_var_list(names); c!=NIL; i++, c=c->rest);
    retesave_four_bytes (i,f);
    for (c=varnames_to_var_list(names); c!=NIL; c=c->rest)
      retesave_four_bytes (((Symbol *)(c->first))->common.a.retesave_symindex,f);
  }
}

varnames *reteload_varnames (agent* thisAgent, FILE* f) {
  list *c;
  unsigned long i, count;
  Symbol *sym;

  i = reteload_one_byte (f);
  if (i==0) return NIL;
  if (i==1) {
    sym = reteload_symbol_from_index (thisAgent,f);
    symbol_add_ref (sym);
    return one_var_to_varnames (sym);
  } else {
    count = reteload_four_bytes (f);
    c = NIL;
    while (count--) {
      sym = reteload_symbol_from_index (thisAgent,f);
      symbol_add_ref (sym);
      push(thisAgent, sym, c);
    }
    c = destructively_reverse_list (c);
    return var_list_to_varnames (c);
  }
}

void retesave_node_varnames (node_varnames *nvn, rete_node *node, FILE* f) {
  while (TRUE) {
    if (node->node_type == DUMMY_TOP_BNODE) return;
    if (node->node_type == CN_BNODE) {
      node=node->b.cn.partner->parent;
      nvn = nvn->data.bottom_of_subconditions;
      continue;
    }
    retesave_varnames (nvn->data.fields.id_varnames,f);
    retesave_varnames (nvn->data.fields.attr_varnames,f);
    retesave_varnames (nvn->data.fields.value_varnames,f);
    nvn = nvn->parent;
    node = real_parent_node (node);
  }
}

node_varnames *reteload_node_varnames (agent* thisAgent, rete_node *node, FILE* f) {
  node_varnames *nvn, *nvn_for_ncc;
  rete_node *temp;

  if (node->node_type == DUMMY_TOP_BNODE) return NIL;
  allocate_with_pool (thisAgent, &thisAgent->node_varnames_pool, &nvn);
  if (node->node_type == CN_BNODE) {
    temp = node->b.cn.partner->parent;
    nvn_for_ncc = reteload_node_varnames (thisAgent, temp,f);
    nvn->data.bottom_of_subconditions = nvn_for_ncc;
    while (temp!=node->parent) {
      temp = real_parent_node(temp);
      nvn_for_ncc = nvn_for_ncc->parent;
    }
    nvn->parent = nvn_for_ncc;
  } else {
    nvn->data.fields.id_varnames = reteload_varnames (thisAgent,f);
    nvn->data.fields.attr_varnames = reteload_varnames (thisAgent,f);
    nvn->data.fields.value_varnames = reteload_varnames (thisAgent,f);
    nvn->parent = reteload_node_varnames (thisAgent, real_parent_node(node),f);
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

void retesave_rhs_value (rhs_value rv, FILE* f) {
  unsigned long i;
  Symbol *sym;
  cons *c;
  
  if (rhs_value_is_symbol(rv)) {
    retesave_one_byte (0,f);
    sym = rhs_value_to_symbol (rv);
    retesave_four_bytes (sym->common.a.retesave_symindex,f);
  } else if (rhs_value_is_funcall(rv)) {
    retesave_one_byte (1,f);
    c = rhs_value_to_funcall_list (rv);
    sym = ((rhs_function *)(c->first))->name;
    retesave_four_bytes (sym->common.a.retesave_symindex,f);
    c=c->rest;
    for (i=0; c!=NIL; i++, c=c->rest);
    retesave_four_bytes (i,f);
    for (c=rhs_value_to_funcall_list(rv)->rest; c!=NIL; c=c->rest)
      retesave_rhs_value ((rhs_value)(c->first),f);
  } else if (rhs_value_is_reteloc(rv)) {
    retesave_one_byte (2,f);
    retesave_one_byte ((byte)rhs_value_to_reteloc_field_num(rv),f);
    retesave_two_bytes (rhs_value_to_reteloc_levels_up(rv),f);
  } else {
    retesave_one_byte (3,f);
    retesave_four_bytes (rhs_value_to_unboundvar(rv),f);
  }
}

rhs_value reteload_rhs_value (Kernel* thisKernel, agent* thisAgent, FILE* f) {
  rhs_value rv, temp;
  unsigned long i, count;
  Symbol *sym;
  byte type, field_num;
  int levels_up;
  list *funcall_list;
  rhs_function *rf;

  type = reteload_one_byte(f);
  switch (type) {
  case 0:
    sym = reteload_symbol_from_index(thisAgent,f);
    symbol_add_ref (sym);
    rv = symbol_to_rhs_value (sym);
    break;
  case 1:
    funcall_list = NIL;
    sym = reteload_symbol_from_index(thisAgent,f);
    symbol_add_ref (sym);
    rf = lookup_rhs_function (thisAgent, sym);
    if (!rf) {
      char msg[BUFFER_MSG_SIZE];
      print_with_symbols (thisAgent, "Error: can't load this file because it uses an undefined RHS function %y\n", sym);
      snprintf (msg, BUFFER_MSG_SIZE, "Error: can't load this file because it uses an undefined RHS function %s\n", symbol_to_string(thisAgent, sym,TRUE,NIL, 0));
      msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
      abort_with_fatal_error(thisAgent, msg);
    }
    push(thisAgent, rf, funcall_list);
    count = reteload_four_bytes(f);    
    while (count--) {
      temp = reteload_rhs_value(thisKernel, thisAgent,f);
      push(thisAgent, temp, funcall_list);
    }
    funcall_list = destructively_reverse_list (funcall_list);
    rv = funcall_list_to_rhs_value (funcall_list);
    break;
  case 2:
    field_num = reteload_one_byte(f);
    levels_up = reteload_two_bytes(f);
    rv = reteloc_to_rhs_value (field_num,levels_up);
    break;
  case 3:
    i = reteload_four_bytes(f);
    update_max_rhs_unbound_variables (thisAgent, i+1);
    rv = unboundvar_to_rhs_value (i);
    break;
  default:
    { char msg[BUFFER_MSG_SIZE];
    strncpy (msg, "Internal error (file corrupted?): bad rhs_value type\n", BUFFER_MSG_SIZE);
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

void retesave_rhs_action (action *a, FILE* f) {
  retesave_one_byte (a->type,f);
  retesave_one_byte (a->preference_type,f);
  retesave_one_byte (a->support,f);
  if (a->type==FUNCALL_ACTION) {
    retesave_rhs_value (a->value,f);   
  } else { /* MAKE_ACTION's */
    retesave_rhs_value (a->id,f);
    retesave_rhs_value (a->attr,f);
    retesave_rhs_value (a->value,f);
    if (preference_is_binary (a->preference_type))
      retesave_rhs_value (a->referent,f);
  }
}
    
action *reteload_rhs_action (Kernel* thisKernel, agent* thisAgent, FILE* f) {
  action *a;

  allocate_with_pool (thisAgent, &thisAgent->action_pool, &a);
  a->type = reteload_one_byte(f);
  a->preference_type = reteload_one_byte(f);
  a->support = reteload_one_byte(f);
  if (a->type==FUNCALL_ACTION) {
    a->id = NIL; a->attr = NIL; a->referent = NIL;
    a->value = reteload_rhs_value(thisKernel, thisAgent,f);
  } else { /* MAKE_ACTION's */
    a->id = reteload_rhs_value(thisKernel, thisAgent,f);
    a->attr = reteload_rhs_value(thisKernel, thisAgent,f);
    a->value = reteload_rhs_value(thisKernel, thisAgent,f);
    if (preference_is_binary (a->preference_type))
      a->referent = reteload_rhs_value(thisKernel, thisAgent,f);
    else
      a->referent = NIL;
  }
  return a;
}

void retesave_action_list (action *first_a, FILE* f) {
  unsigned long i;
  action *a;

  for (i=0, a=first_a; a!=NIL; i++, a=a->next);
  retesave_four_bytes (i,f);
  for (a=first_a; a!=NIL; a=a->next) retesave_rhs_action (a,f);
}

action *reteload_action_list (Kernel* thisKernel, agent* thisAgent, FILE* f) {
  action *a, *prev_a, *first_a;
  unsigned long count;
  
  count = reteload_four_bytes (f);
  prev_a = NIL;
  first_a = NIL;  /* unneeded, but without it gcc -Wall warns here */
  while (count--) {
    a = reteload_rhs_action (thisKernel, thisAgent,f );
    if (prev_a) prev_a->next = a; else first_a = a;
    prev_a = a;
  }
  if (prev_a) prev_a->next = NIL; else first_a = NIL;
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

void retesave_rete_test (rete_test *rt, FILE* f) {
  int i; cons *c;
  
  retesave_one_byte (rt->type,f);
  retesave_one_byte (rt->right_field_num,f);
  if (test_is_constant_relational_test(rt->type)) {
    retesave_four_bytes(rt->data.constant_referent->common.a.retesave_symindex, f);
  } else if (test_is_variable_relational_test(rt->type)) {
    retesave_one_byte (rt->data.variable_referent.field_num,f);
    retesave_two_bytes (rt->data.variable_referent.levels_up,f);
  } else if (rt->type==DISJUNCTION_RETE_TEST) {
    for (i=0, c=rt->data.disjunction_list; c!=NIL; i++, c=c->rest,f);
    retesave_two_bytes (i,f);
    for (c=rt->data.disjunction_list; c!=NIL; c=c->rest)
      retesave_four_bytes (((Symbol *)(c->first))->common.a.retesave_symindex,f);
  }
}

rete_test *reteload_rete_test (agent* thisAgent, FILE* f) {
  rete_test *rt;
  Symbol *sym;
  unsigned long count;
  list *temp;

  allocate_with_pool (thisAgent, &thisAgent->rete_test_pool, &rt);
  rt->type = reteload_one_byte(f);
  rt->right_field_num = reteload_one_byte(f);
  
  if (test_is_constant_relational_test(rt->type)) {
    rt->data.constant_referent = reteload_symbol_from_index(thisAgent,f);
    symbol_add_ref (rt->data.constant_referent);  
  } else if (test_is_variable_relational_test(rt->type)) {
    rt->data.variable_referent.field_num = reteload_one_byte(f);
    rt->data.variable_referent.levels_up = (unsigned short)reteload_two_bytes(f);
  } else if (rt->type==DISJUNCTION_RETE_TEST) {
    count = reteload_two_bytes(f);
    temp = NIL;
    while (count--) {
      sym = reteload_symbol_from_index(thisAgent,f);
      symbol_add_ref (sym);
      push(thisAgent, sym, temp);
    }
    rt->data.disjunction_list = destructively_reverse_list (temp);
  }
  return rt;
}

void retesave_rete_test_list (rete_test *first_rt, FILE* f) {
  unsigned long i;
  rete_test *rt;

  for (i=0, rt=first_rt; rt!=NIL; i++, rt=rt->next);
  retesave_two_bytes (i,f);
  for (rt=first_rt; rt!=NIL; rt=rt->next)
    retesave_rete_test (rt,f);
}

rete_test *reteload_rete_test_list (agent* thisAgent, FILE* f) {
  rete_test *rt, *prev_rt, *first;
  unsigned long count;
  
  prev_rt = NIL;
  first = NIL;  /* unneeded, but without it gcc -Wall warns here */
  count = reteload_two_bytes(f);
  while (count--) {
    rt = reteload_rete_test(thisAgent,f);
    if (prev_rt) prev_rt->next = rt; else first = rt;
    prev_rt = rt;
  }
  if (prev_rt) prev_rt->next = NIL; else first = NIL;
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

void retesave_rete_node_and_children (agent* thisAgent, rete_node *node, FILE* f);

void retesave_children_of_node (agent* thisAgent, rete_node *node, FILE* f) {
  unsigned long i; rete_node *child;

  /* --- Count number of non-CN-node children. --- */
  for (i=0, child=node->first_child; child; child=child->next_sibling)
    if (child->node_type != CN_BNODE) i++;
  retesave_four_bytes (i,f);

  /* --- Write out records for all the node's children except CN's. --- */
  for (child=node->first_child; child; child=child->next_sibling)
    if (child->node_type != CN_BNODE)
      retesave_rete_node_and_children (thisAgent, child, f);
}

void retesave_rete_node_and_children (agent* thisAgent, rete_node *node, FILE* f) {
  unsigned long i;
  production *prod;
  cons *c;
  rete_node *temp;

  if (node->node_type == CN_BNODE) return; /* ignore CN nodes */
  
  retesave_one_byte (node->node_type,f);

  switch (node->node_type) {
  case MEMORY_BNODE:
    retesave_one_byte (node->left_hash_loc_field_num,f);
    retesave_two_bytes (node->left_hash_loc_levels_up,f);
    /* ... and fall through to the next case below ... */
  case UNHASHED_MEMORY_BNODE:
    break;

  case MP_BNODE:
    retesave_one_byte (node->left_hash_loc_field_num,f);
    retesave_two_bytes (node->left_hash_loc_levels_up,f);
    /* ... and fall through to the next case below ... */
  case UNHASHED_MP_BNODE:
    retesave_four_bytes (node->b.posneg.alpha_mem_->retesave_amindex,f);
    retesave_rete_test_list (node->b.posneg.other_tests,f);
    retesave_one_byte ((byte)(node->a.np.is_left_unlinked ? 1 : 0),f);
    break;

  case POSITIVE_BNODE:
  case UNHASHED_POSITIVE_BNODE:
    retesave_four_bytes (node->b.posneg.alpha_mem_->retesave_amindex,f);
    retesave_rete_test_list (node->b.posneg.other_tests,f);
    retesave_one_byte ((byte)(node_is_left_unlinked(node) ? 1 : 0),f);
    break;

  case NEGATIVE_BNODE:
    retesave_one_byte (node->left_hash_loc_field_num,f);
    retesave_two_bytes (node->left_hash_loc_levels_up,f );
    /* ... and fall through to the next case below ... */
  case UNHASHED_NEGATIVE_BNODE:
    retesave_four_bytes (node->b.posneg.alpha_mem_->retesave_amindex,f);
    retesave_rete_test_list (node->b.posneg.other_tests,f);
    break;

  case CN_PARTNER_BNODE:
    i=0;
    temp = real_parent_node (node);
    while (temp != node->b.cn.partner->parent) {
      temp = real_parent_node (temp);
      i++;
    }
    retesave_four_bytes (i,f);
    break;

  case P_BNODE:
    prod = node->b.p.prod;
    retesave_four_bytes (prod->name->common.a.retesave_symindex,f);
    if (prod->documentation) {
      retesave_one_byte (1,f);
      retesave_string (prod->documentation,f);
    } else {
      retesave_one_byte (0,f);
    }
    retesave_one_byte (prod->type,f);
    retesave_one_byte (prod->declared_support,f);
    retesave_action_list (prod->action_list,f );
    for (i=0, c=prod->rhs_unbound_variables; c!=NIL; i++, c=c->rest);
    retesave_four_bytes (i,f);
    for (c=prod->rhs_unbound_variables; c!=NIL; c=c->rest)
      retesave_four_bytes (((Symbol *)(c->first))->common.a.retesave_symindex,f);
    if (node->b.p.parents_nvn) {
      retesave_one_byte (1,f);
      retesave_node_varnames (node->b.p.parents_nvn, node->parent, f);
    } else {
      retesave_one_byte (0,f);
    }
    break;

  default:
    {char msg[BUFFER_MSG_SIZE];
    snprintf (msg, BUFFER_MSG_SIZE,
           "Internal error: fastsave found node type %d\n", node->node_type);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
    }
  } /* end of switch statement */

  /* --- For cn_p nodes, write out the CN node's children instead --- */
  if (node->node_type == CN_PARTNER_BNODE) node = node->b.cn.partner;
  /* --- Write out records for all the node's children. --- */
  retesave_children_of_node (thisAgent, node,f);
}

void reteload_node_and_children (Kernel* thisKernel, agent* thisAgent, rete_node *parent, FILE* f) {
  byte type, left_unlinked_flag;
  rete_node *New, *ncc_top;
  unsigned long count;
  alpha_mem *am;
  production *prod;
  Symbol *sym;
  list *ubv_list;
  var_location left_hash_loc;
  rete_test *other_tests;
  
  type = reteload_one_byte(f);

  /* 
     Initializing the left_hash_loc structure to flag values.
     It gets passed into some of the various make_new_??? functions
     below but is never used (hopefully) for UNHASHED node types.
  */
  left_hash_loc.field_num = (byte) -1;
  left_hash_loc.levels_up = (rete_node_level) -1;

  switch (type) {
  case MEMORY_BNODE:
    left_hash_loc.field_num = reteload_one_byte(f);
    left_hash_loc.levels_up = (unsigned short)reteload_two_bytes(f);
    /* ... and fall through to the next case below ... */
  case UNHASHED_MEMORY_BNODE:
    New = make_new_mem_node (thisAgent, parent, type, left_hash_loc);
    break;

  case MP_BNODE:
    left_hash_loc.field_num = reteload_one_byte(f);
    left_hash_loc.levels_up = (unsigned short)reteload_two_bytes(f);
    /* ... and fall through to the next case below ... */
  case UNHASHED_MP_BNODE:
    am = reteload_am_from_index(thisAgent,f);
    am->reference_count++;
    other_tests = reteload_rete_test_list(thisAgent,f);
    left_unlinked_flag = reteload_one_byte(f);
    New = make_new_mp_node (thisAgent, parent, type, left_hash_loc, am, other_tests,
                            left_unlinked_flag != 0);
    break;

  case POSITIVE_BNODE:
  case UNHASHED_POSITIVE_BNODE:
    am = reteload_am_from_index(thisAgent,f);
    am->reference_count++;
    other_tests = reteload_rete_test_list(thisAgent,f);
    left_unlinked_flag = reteload_one_byte(f);
    New = make_new_positive_node (thisAgent, parent, type, am, other_tests,
                                  left_unlinked_flag != 0);
    break;

  case NEGATIVE_BNODE:
    left_hash_loc.field_num = reteload_one_byte(f);
    left_hash_loc.levels_up = (unsigned short)reteload_two_bytes(f);
    /* ... and fall through to the next case below ... */
  case UNHASHED_NEGATIVE_BNODE:
    am = reteload_am_from_index(thisAgent,f);
    am->reference_count++;
    other_tests = reteload_rete_test_list(thisAgent,f);
    New = make_new_negative_node (thisAgent, parent, type, left_hash_loc, am,other_tests);
    break;

  case CN_PARTNER_BNODE:
    count = reteload_four_bytes(f);
    ncc_top = parent;
    while (count--) ncc_top = real_parent_node (ncc_top);
    New = make_new_cn_node (thisAgent, ncc_top, parent);
    break;

  case P_BNODE:
    allocate_with_pool (thisAgent, &thisAgent->production_pool, &prod);
    prod->reference_count = 1;
    prod->firing_count = 0;
    prod->trace_firings = FALSE;
    prod->instantiations = NIL;
    prod->filename = NIL;
    prod->p_node = NIL;
    
    sym = reteload_symbol_from_index (thisAgent,f);
    symbol_add_ref (sym);
    prod->name = sym;
    sym->sc.production = prod;
    if (reteload_one_byte(f)) {
      reteload_string(f);
      prod->documentation = make_memory_block_for_string (thisAgent, reteload_string_buf);
    } else {
      prod->documentation = NIL;
    }
    prod->type = reteload_one_byte (f);
    prod->declared_support = reteload_one_byte (f);
    prod->action_list = reteload_action_list (thisKernel, thisAgent,f);

    count = reteload_four_bytes (f);
    update_max_rhs_unbound_variables (thisAgent, count);
    ubv_list = NIL;
    while (count--) {
      sym = reteload_symbol_from_index(thisAgent,f);
      symbol_add_ref (sym);
      push(thisAgent, sym, ubv_list);
    }
    prod->rhs_unbound_variables = destructively_reverse_list (ubv_list);

    insert_at_head_of_dll (thisAgent->all_productions_of_type[prod->type],
                           prod, next, prev);
    thisAgent->num_productions_of_type[prod->type]++;

    New = make_new_production_node (thisAgent, parent, prod);
    adjust_sharing_factors_from_here_to_top (New, 1);
    if (reteload_one_byte(f)) {
      New->b.p.parents_nvn = reteload_node_varnames (thisAgent, parent,f);
    } else {
      New->b.p.parents_nvn = NIL;
    }

    /* --- call new node's add_left routine with all the parent's tokens --- */
    update_node_with_matches_from_above (thisAgent, New);

     /* --- invoke callback on the production --- */
    soar_invoke_callbacks (thisAgent, thisAgent, PRODUCTION_JUST_ADDED_CALLBACK,
                          (soar_call_data) prod);
 
    break;

  default:
    { char msg[BUFFER_MSG_SIZE];
    snprintf (msg, BUFFER_MSG_SIZE,"Internal error: fastload found node type %d\n", type);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
    New = NIL; /* unreachable, but without it gcc -Wall warns here */
    }
  } /* end of switch statement */

  /* --- read in the children of the node --- */
  count = reteload_four_bytes(f);
  while (count--) reteload_node_and_children (thisKernel, thisAgent, New,f);
}

/* ----------------------------------------------------------------------
                        Save/Load The Whole Net

  Save_rete_net() and load_rete_net() save and load everything to and 
  from the given (already open) files.  They return TRUE if successful, 
  FALSE if any error occurred.
---------------------------------------------------------------------- */

Bool save_rete_net (agent* thisAgent, FILE *dest_file) {

  /* --- make sure there are no justifications present --- */
  if (thisAgent->all_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE]) {
    print (thisAgent, "Internal error: save_rete_net() with justifications present.\n");
    return FALSE;
  }
  
  rete_fs_file = dest_file;

  retesave_string ("SoarCompactReteNet\n",dest_file);
  retesave_one_byte (3,dest_file);  /* format version number */
  retesave_symbol_table(thisAgent, dest_file);
  retesave_alpha_memories(thisAgent,dest_file);
  retesave_children_of_node (thisAgent, thisAgent->dummy_top_node,dest_file);
  return TRUE;
}

Bool load_rete_net (Kernel* thisKernel, agent* thisAgent, FILE *source_file) {
  int format_version_num;
  unsigned long i, count;

  /* RDF: 20020814 RDF Cleaning up the agent working memory and production
     memory to avoid unecessary errors in this function. */
  reinitialize_soar(thisAgent);
  excise_all_productions(thisAgent, TRUE);

  /* DONE clearing old productions */
  

  /* --- check for empty system --- */
  if (thisAgent->all_wmes_in_rete) {
    print (thisAgent, "Internal error: load_rete_net() called with nonempty WM.\n");
    return FALSE;
  }
  for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    if (thisAgent->num_productions_of_type[i]) {
      print (thisAgent, "Internal error: load_rete_net() called with nonempty PM.\n");
      return FALSE;
    }
  
  rete_fs_file = source_file;

  /* --- read file header, make sure it's a valid file --- */
  reteload_string(source_file);
  if (strcmp(reteload_string_buf,"SoarCompactReteNet\n")) {
    print (thisAgent, "This file isn't a Soar fastsave file.\n");
    return FALSE;
  }
  format_version_num = reteload_one_byte(source_file);
  if (format_version_num!=3) {
    print (thisAgent, "This file is in a format (version %d) I don't understand.\n",
           format_version_num);
    return FALSE;
  }

  reteload_all_symbols(thisAgent,source_file);
  reteload_alpha_memories(thisAgent,source_file);
  count = reteload_four_bytes(source_file);
  while (count--) reteload_node_and_children (thisKernel, thisAgent, thisAgent->dummy_top_node,source_file);

  /* --- clean up auxilliary tables --- */
  reteload_free_am_table(thisAgent);
  reteload_free_symbol_table(thisAgent);

  /* RDF: 20020814 Now adding the top state and io symbols and wmes */
  init_agent_memory(thisAgent);

  return TRUE;
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

unsigned long count_rete_tokens_for_production (agent* thisAgent, 
                                                                                                production *prod) {
  unsigned long count;
  rete_node *node;
  token *tok;

  if (! prod->p_node) return 0;
  node = prod->p_node->parent;
  count = 0;
  while (node!=thisAgent->dummy_top_node) {
    if ((node->node_type != POSITIVE_BNODE) &&
        (node->node_type != UNHASHED_POSITIVE_BNODE)) {
      for (tok=node->a.np.tokens; tok!=NIL; tok=tok->next_of_node) count++;
    }
    if (node->node_type==CN_BNODE) node = node->b.cn.partner->parent;
    else node = node->parent;
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
void init_bnode_type_names(agent* thisAgent)
{
   static bool bnode_initialzied = false;

   //
   // This should be properly locked.
   //
   if(!bnode_initialzied)
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



void get_all_node_count_stats (agent* thisAgent) {
  int i;

  //
  // This sanity check should no longer be neccessary.
  //
  /* --- sanity check: make sure we've got names for all the bnode types --- */
  //for (i=0; i<256; i++)
  //  if (thisAgent->rete_node_counts[i] &&
  //      (*bnode_type_names[i] == 0)) {
  //    print (thisAgent, "Internal eror: unknown node type [%d] has nonzero count.\n",i);
  //  }
  init_bnode_type_names(thisAgent);

  /* --- calculate the three arrays --- */
  for (i=0; i<256; i++) {
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

void print_node_count_statistics (agent* thisAgent) {
  int i;
  unsigned long tot;

  get_all_node_count_stats(thisAgent);

  /* --- print table headers --- */
#ifdef SHARING_FACTORS
  print (thisAgent, "      Node Type           Actual  If no merging  If no sharing\n");
  print (thisAgent, "---------------------  ---------  -------------  -------------\n");
#else
  print (thisAgent, "      Node Type           Actual  If no merging\n");
  print (thisAgent, "---------------------  ---------  -------------\n");
#endif

  /* --- print main table --- */
  for (i=0; i<256; i++) if (*bnode_type_names[i]) {
    print (thisAgent, "%21s  %9lu  %13lu", bnode_type_names[i],
           thisAgent->actual[i], thisAgent->if_no_merging[i]);
#ifdef SHARING_FACTORS
    print (thisAgent, "  %13lu", thisAgent->if_no_sharing[i]);
#endif
    print (thisAgent, "\n");
  }

  /* --- print table end (totals) --- */
#ifdef SHARING_FACTORS
  print (thisAgent, "---------------------  ---------  -------------  -------------\n");
#else
  print (thisAgent, "---------------------  ---------  -------------\n");
#endif
  print (thisAgent, "                Total");
  for (tot=0, i=0; i<256; i++) tot+=thisAgent->actual[i];
  print (thisAgent, "  %9lu", tot);
  for (tot=0, i=0; i<256; i++) tot+=thisAgent->if_no_merging[i];
  print (thisAgent, "  %13lu", tot);
#ifdef SHARING_FACTORS
  for (tot=0, i=0; i<256; i++) tot+=thisAgent->if_no_sharing[i];
  print (thisAgent, "  %13lu", tot);
#endif
  print (thisAgent, "\n");
}

/* Returns 0 if result invalid, 1 if result valid */
int get_node_count_statistic (agent* thisAgent, 
                                  char * node_type_name, 
                              char * column_name,
                              unsigned long * result) 
{
  int i;
  unsigned long tot;

  get_all_node_count_stats(thisAgent);

  if (!strcmp("total", node_type_name))
    {
      if (!strcmp("actual", column_name))
        {
          for (tot=0, i=0; i<256; i++) tot+=thisAgent->actual[i];
          *result = tot;
        }
      else if (!strcmp("if-no-merging", column_name))
        {
          for (tot=0, i=0; i<256; i++) tot+=thisAgent->if_no_merging[i];
          *result = tot;
        }
#ifdef SHARING_FACTORS
      else if (!strcmp("if-no-sharing", column_name))
        {
          for (tot=0, i=0; i<256; i++) tot+=thisAgent->if_no_sharing[i];
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
      for (i=0; i<256; i++) 
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

void print_rete_statistics (agent* thisAgent) {

#ifdef TOKEN_SHARING_STATS
  print ("Token additions: %lu   If no sharing: %lu\n",
         thisAgent->token_additions,
         thisAgent->token_additions_without_sharing);
#endif

  print_node_count_statistics(thisAgent);
  print_null_activation_stats();
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

void dummy_matches_node_left_addition (agent* thisAgent, rete_node *node, token *tok, wme *w) 
{
  token *New;
  
  /* --- just add a token record to dummy_matches_node_tokens --- */
  allocate_with_pool (thisAgent, &thisAgent->token_pool, &New);
  New->node = NIL;
  New->parent = tok;
  New->w = w;
  New->next_of_node = thisAgent->dummy_matches_node_tokens;
  thisAgent->dummy_matches_node_tokens = New;
}

token *get_all_left_tokens_emerging_from_node (agent* thisAgent,rete_node *node) 
{
  token *result;
  rete_node dummy_matches_node;  
  
  thisAgent->dummy_matches_node_tokens = NIL;
  dummy_matches_node.node_type = DUMMY_MATCHES_BNODE;
  dummy_matches_node.parent = node;
  dummy_matches_node.first_child = NIL;
  dummy_matches_node.next_sibling = NIL;
  update_node_with_matches_from_above (thisAgent, &dummy_matches_node);
  result = thisAgent->dummy_matches_node_tokens;
  return result;
}

void deallocate_token_list (agent* thisAgent, token *t) {
  token *next;

  while (t) {
    next = t->next_of_node;
    free_with_pool (&thisAgent->token_pool, t);
    t = next;
  }
}

void print_whole_token (agent* thisAgent, token *t, wme_trace_type wtt) {
  if (t==thisAgent->dummy_top_token) return;
  print_whole_token (thisAgent, t->parent, wtt);
  if (t->w) {
    if (wtt==TIMETAG_WME_TRACE) print (thisAgent, "%lu", t->w->timetag);
    else if (wtt==FULL_WME_TRACE) print_wme (thisAgent, t->w);
    if (wtt!=NONE_WME_TRACE) print (thisAgent, " ");
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
long ppmi_aux (agent* thisAgent,   /* current agent */
                           rete_node *node,    /* current node */
               rete_node *cutoff,  /* don't print cutoff node or any higher */
               condition *cond,    /* cond for current node */
               wme_trace_type wtt, /* what type of printout to use */
               int indent) {       /* number of spaces indent */
  token *tokens, *t, *parent_tokens;
  right_mem *rm;
  long matches_one_level_up;
  long matches_at_this_level;
#define MATCH_COUNT_STRING_BUFFER_SIZE 20
  char match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE];
  rete_node *parent;

  /* --- find the number of matches for this condition --- */
  tokens = get_all_left_tokens_emerging_from_node (thisAgent, node);
  matches_at_this_level = 0;
  for (t=tokens; t!=NIL; t=t->next_of_node) matches_at_this_level++;
  deallocate_token_list (thisAgent, tokens);

  /* --- if we're at the cutoff node, we're done --- */
  if (node==cutoff) return matches_at_this_level;

  /* --- do stuff higher up --- */
  parent = real_parent_node(node);
  matches_one_level_up = ppmi_aux (thisAgent, parent, cutoff,
                                   cond->prev, wtt, indent);

  /* --- Form string for current match count:  If an earlier cond had no
     matches, just leave it blank; if this is the first 0, use ">>>>" --- */
  if (! matches_one_level_up) {
    strncpy (match_count_string, "    ", MATCH_COUNT_STRING_BUFFER_SIZE);
	match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
  } else if (! matches_at_this_level) {
    strncpy (match_count_string, ">>>>", MATCH_COUNT_STRING_BUFFER_SIZE);
	match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
  } else {
    snprintf (match_count_string, MATCH_COUNT_STRING_BUFFER_SIZE, "%4ld", matches_at_this_level);
	match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
  }

  /* --- print extra indentation spaces --- */
  print_spaces (thisAgent, indent);

  if (cond->type==CONJUNCTIVE_NEGATION_CONDITION) {
    /* --- recursively print match counts for the NCC subconditions --- */
    print (thisAgent, "    -{\n");
    ppmi_aux (thisAgent, real_parent_node(node->b.cn.partner),
              parent,
              cond->data.ncc.bottom,
              wtt,
              indent+5);
    print_spaces (thisAgent, indent);
    print (thisAgent, "%s }\n", match_count_string);
  } else {
    print (thisAgent, "%s", match_count_string);
    print_condition (thisAgent, cond);
    print (thisAgent, "\n");
    /* --- if this is the first match-failure (0 matches), print info on
       matches for left and right --- */
    if (matches_one_level_up && (!matches_at_this_level)) {
      if (wtt!=NONE_WME_TRACE) {
        print_spaces (thisAgent, indent);
        print (thisAgent, "*** Matches For Left ***\n");
        parent_tokens = get_all_left_tokens_emerging_from_node (thisAgent, parent);
        for (t=parent_tokens; t!=NIL; t=t->next_of_node) {
          print_spaces (thisAgent, indent);
          print_whole_token (thisAgent, t, wtt);
          print (thisAgent, "\n");
        }
        deallocate_token_list (thisAgent, parent_tokens);
        print_spaces (thisAgent, indent);
        print (thisAgent, "*** Matches for Right ***\n");
        print_spaces (thisAgent, indent);
        for (rm=node->b.posneg.alpha_mem_->right_mems; rm!=NIL;
             rm=rm->next_in_am) {
          if (wtt==TIMETAG_WME_TRACE) print (thisAgent, "%lu", rm->w->timetag);
          else if (wtt==FULL_WME_TRACE) print_wme (thisAgent, rm->w);
          print (thisAgent, " ");
        }
        print (thisAgent, "\n");
      }
    } /* end of if (matches_one_level_up ...) */
  }

  /* --- return result --- */  
  return matches_at_this_level;
}

void print_partial_match_information (agent* thisAgent, rete_node *p_node, 
                                                                          wme_trace_type wtt) {
  condition *top_cond, *bottom_cond;
  long n;
  token *tokens, *t;

  p_node_to_conditions_and_nots (thisAgent, p_node, NIL, NIL, &top_cond, &bottom_cond,
                                 NIL, NIL);
  n = ppmi_aux (thisAgent, p_node->parent, thisAgent->dummy_top_node, bottom_cond,
                wtt, 0);
  print (thisAgent, "\n%d complete matches.\n", n);
  if (n && (wtt!=NONE_WME_TRACE)) {
    print (thisAgent, "*** Complete Matches ***\n");
    tokens = get_all_left_tokens_emerging_from_node (thisAgent, p_node->parent);
    for (t=tokens; t!=NIL; t=t->next_of_node) {
      print_whole_token (thisAgent, t, wtt);
      print (thisAgent, "\n");
    }
    deallocate_token_list (thisAgent, tokens);
  }
  deallocate_condition_list (thisAgent, top_cond);
}

/* ----------------------------------------------------------------------

   Used by the "ms" command -- prints out the current match set.
---------------------------------------------------------------------- */

typedef struct match_set_trace {
        Symbol *sym;
        int        count;
        struct match_set_trace *next;
  /* REW: begin 08.20.97 */
  /* Add match goal to the print of the matching production */
        Symbol *goal;
  /* REW: end   08.20.97 */
} MS_trace;

MS_trace *in_ms_trace(Symbol *sym, MS_trace *trace) {
  MS_trace *tmp;
  for(tmp = trace; tmp; tmp=tmp->next) {
    if(tmp->sym == sym) return tmp;
  }
  return (MS_trace *)0;
}

/* REW: begin 10.22.97 */
MS_trace *in_ms_trace_same_goal(Symbol *sym, MS_trace *trace, Symbol *goal) {
  MS_trace *tmp;
  for(tmp = trace; tmp; tmp=tmp->next) {
    if((tmp->sym == sym) && (goal == tmp->goal)) return tmp;
  }
  return (MS_trace *)0;
}
/* REW: end   10.22.97 */

void print_match_set (agent* thisAgent, wme_trace_type wtt, ms_trace_type mst) {
  ms_change *msc;
  token temp_token;
  MS_trace *ms_trace = NIL, *tmp;

  /* --- Print assertions --- */  


  /* REW: begin 09.15.96 */
  if (thisAgent->operand2_mode == TRUE) {

    if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT) {
       print (thisAgent, "O Assertions:\n");
       for (msc=thisAgent->ms_o_assertions; msc!=NIL; msc=msc->next) {

         if(wtt != NONE_WME_TRACE) {
           print_with_symbols (thisAgent, "  %y ", msc->p_node->b.p.prod->name);
           /* REW: begin 08.20.97 */
           /* Add match goal to the print of the matching production */
           print_with_symbols(thisAgent, " [%y] ", msc->goal);
           /* REW: end   08.20.97 */
           temp_token.parent = msc->tok;
           temp_token.w = msc->w;
           print_whole_token (thisAgent, &temp_token, wtt);
           print (thisAgent, "\n");
         }
         else {
           /* REW: begin 10.22.97 */
           if((tmp = in_ms_trace_same_goal(msc->p_node->b.p.prod->name,
                                           ms_trace, msc->goal))!=NIL) {
             /* REW: end   10.22.97 */
             tmp->count++;
           }
           else {
             tmp = static_cast<match_set_trace *>(allocate_memory(thisAgent, sizeof(MS_trace), MISCELLANEOUS_MEM_USAGE));
             tmp->sym = msc->p_node->b.p.prod->name;
             tmp->count = 1;
             tmp->next = ms_trace;
             /* REW: begin 08.20.97 */
             /* Add match goal to the print of the matching production */
             tmp->goal = msc->goal;
             /* REW: end   08.20.97 */
             ms_trace = tmp;
          }
        }
      }

      if (wtt == NONE_WME_TRACE) {
         while (ms_trace) {
           tmp = ms_trace; ms_trace = tmp->next;
           print_with_symbols (thisAgent, "  %y ", tmp->sym);
           /* REW: begin 08.20.97 */
           /*  BUG: for now this will print the goal of the first
               assertion inspected, even though there can be multiple
               assertions at different levels. 
               See 2.110 in the OPERAND-CHANGE-LOG. */
           print_with_symbols(thisAgent, " [%y] ", tmp->goal);
           /* REW: end  08.20.97 */
           if (tmp->count > 1)
             print(thisAgent, "(%d)\n", tmp->count);
           else
             print(thisAgent, "\n");
           free_memory(thisAgent, (void *)tmp, MISCELLANEOUS_MEM_USAGE);
        }
      }
    }

     if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT) {
       print (thisAgent, "I Assertions:\n");
       for (msc=thisAgent->ms_i_assertions; msc!=NIL; msc=msc->next) {

         if(wtt != NONE_WME_TRACE) {
           print_with_symbols (thisAgent, "  %y ", msc->p_node->b.p.prod->name);
           /* REW: begin 08.20.97 */
           /* Add match goal to the print of the matching production */
           print_with_symbols(thisAgent, " [%y] ", msc->goal);
           /* REW: end   08.20.97 */
           temp_token.parent = msc->tok;
           temp_token.w = msc->w;
           print_whole_token (thisAgent, &temp_token, wtt);
           print (thisAgent, "\n");
         }
         else {
           /* REW: begin 10.22.97 */
           if((tmp = in_ms_trace_same_goal(msc->p_node->b.p.prod->name,
                                           ms_trace, msc->goal))!=NIL) {
             /* REW: end   10.22.97 */
             tmp->count++;
           }
           else {
             tmp = static_cast<match_set_trace *>(allocate_memory(thisAgent, sizeof(MS_trace), 
                                                                                                                          MISCELLANEOUS_MEM_USAGE));
             tmp->sym = msc->p_node->b.p.prod->name;
             tmp->count = 1;
             tmp->next = ms_trace;
             /* REW: begin 08.20.97 */
             /* Add match goal to the print of the matching production */
             tmp->goal = msc->goal;
             /* REW: end   08.20.97 */
             ms_trace = tmp;
          }
        }
      }

      if (wtt == NONE_WME_TRACE) {
         while (ms_trace) {
           tmp = ms_trace; ms_trace = tmp->next;
           print_with_symbols (thisAgent, "  %y ", tmp->sym);
           /* REW: begin 08.20.97 */
           /*  BUG: for now this will print the goal of the first
               assertion inspected, even though there can be multiple
               assertions at different levels. 
               See 2.110 in the OPERAND-CHANGE-LOG. */
           print_with_symbols(thisAgent, " [%y] ", tmp->goal);
           /* REW: end  08.20.97 */
           if (tmp->count > 1)
             print(thisAgent, "(%d)\n", tmp->count);
           else
             print(thisAgent, "\n");
           free_memory(thisAgent, (void *)tmp, MISCELLANEOUS_MEM_USAGE);
        }
      }
    }

  }
  /* REW: end   09.15.96 */

  else


  if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT) {
    print (thisAgent, "Assertions:\n");
    for (msc=thisAgent->ms_assertions; msc!=NIL; msc=msc->next) {
      if(wtt != NONE_WME_TRACE) {
        print_with_symbols (thisAgent, "  %y\n ", msc->p_node->b.p.prod->name);
        temp_token.parent = msc->tok;
        temp_token.w = msc->w;
        print_whole_token (thisAgent, &temp_token, wtt);
        print (thisAgent, "\n");
      } else {
        if((tmp = in_ms_trace(msc->p_node->b.p.prod->name, ms_trace))!=NIL) {
          tmp->count++;
        } else {
          tmp = static_cast<match_set_trace *>(allocate_memory(thisAgent, sizeof(MS_trace), 
                                                                                                                           MISCELLANEOUS_MEM_USAGE));
          tmp->sym = msc->p_node->b.p.prod->name;
          tmp->count = 1;
          tmp->next = ms_trace;
          ms_trace = tmp;
        }
      }
    }
    if (wtt == NONE_WME_TRACE) {
      while (ms_trace) {
        tmp = ms_trace; ms_trace = tmp->next;
        print_with_symbols (thisAgent, "  %y ", tmp->sym);
        if (tmp->count > 1)
          print(thisAgent, "(%d)\n", tmp->count);
        else
          print(thisAgent, "\n");
        free_memory(thisAgent, (void *)tmp, MISCELLANEOUS_MEM_USAGE);
      }
    }
  }

  /* --- Print retractions --- */  
  if (mst == MS_ASSERT_RETRACT || mst == MS_RETRACT) {
    print (thisAgent, "Retractions:\n");
    for (msc=thisAgent->ms_retractions; msc!=NIL; msc=msc->next) {
      if(wtt != NONE_WME_TRACE) {
        print (thisAgent, "  ");
        print_instantiation_with_wmes (thisAgent, msc->inst, wtt, -1);
        print (thisAgent, "\n");
      } else {
        if(msc->inst->prod) {
          /* REW: begin 10.22.97 */
          if((tmp = in_ms_trace_same_goal(msc->inst->prod->name,
                                          ms_trace, msc->goal))!=NIL) {
            /* REW: end   10.22.97 */
            tmp->count++;
          } else {
            tmp = static_cast<match_set_trace *>(allocate_memory(thisAgent, sizeof(MS_trace), 
                                                                                                                         MISCELLANEOUS_MEM_USAGE));
            tmp->sym = msc->inst->prod->name;
            tmp->count = 1;
            tmp->next = ms_trace;
             /* REW: begin 08.20.97 */
             /* Add match goal to the print of the matching production */
             tmp->goal = msc->goal;
             /* REW: end   08.20.97 */
            ms_trace = tmp;
          }
        }
      }
    }
    if(wtt == NONE_WME_TRACE) {
      while (ms_trace) {
        tmp = ms_trace; ms_trace = tmp->next;
        print_with_symbols (thisAgent, "  %y ", tmp->sym);
           /* REW: begin 08.20.97 */
           /*  BUG: for now this will print the goal of the first assertion
               inspected, even though there can be multiple assertions at

               different levels. 
               See 2.110 in the OPERAND-CHANGE-LOG. */
        if (tmp->goal)
          print_with_symbols(thisAgent, " [%y] ", tmp->goal);
        else
          print(thisAgent, " [NIL] ");
           /* REW: end  08.20.97 */
        if(tmp->count > 1)
          print(thisAgent, "(%d)\n", tmp->count);
        else
          print(thisAgent, "\n");
        free_memory(thisAgent, (void *)tmp, MISCELLANEOUS_MEM_USAGE);
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

void xml_whole_token (agent* thisAgent, token *t, wme_trace_type wtt) {
  if (t==thisAgent->dummy_top_token) return;
  xml_whole_token (thisAgent, t->parent, wtt);
  if (t->w) {
    if (wtt==TIMETAG_WME_TRACE) xmlULong(kWME_TimeTag, t->w->timetag);
    else if (wtt==FULL_WME_TRACE) xml_wme (thisAgent, t->w);
    //if (wtt!=NONE_WME_TRACE) print (thisAgent, " ");
  }
}

Bool xml_pick_conds_with_matching_id_test (dl_cons *dc, agent* thisAgent) {
  condition *cond;
  cond = static_cast<condition_struct *>(dc->item);
  if (cond->type==CONJUNCTIVE_NEGATION_CONDITION) return FALSE;
  return tests_are_equal (thisAgent->id_test_to_match, cond->data.tests.id_test);
}

#if 0
// Not currently using
// xml_test is based on test_to_string.
void xml_test (agent* thisAgent, char const* pTag, test t) {
	char *dest = 0 ;
	size_t dest_size = 0 ;
	cons *c;
  complex_test *ct;
  char *ch;

  if (test_is_blank_test(t)) {
    //if (!dest) dest=thisAgent->printed_output_string;
    xmlString(pTag, "[BLANK TEST]") ;	// Using tag as attribute name
    //strncpy (dest, "[BLANK TEST]", dest_size);  /* this should never get executed */
	//dest[dest_size - 1] = 0; /* ensure null termination */
    //return dest;
	return ;
  }

  if (test_is_blank_or_equality_test(t)) {
    xmlSymbol(thisAgent, pTag, referent_of_equality_test(t)) ;	// Using tag as attribute name
	return ;
    //return symbol_to_string (thisAgent, referent_of_equality_test(t), TRUE, dest, dest_size);
  }

  if (!dest) {
 	dest=thisAgent->printed_output_string;
	dest_size = MAX_LEXEME_LENGTH*2+10; /* from agent.h */
  }
  ch = dest;
  ct = complex_test_from_test(t);
  
  switch (ct->type) {
  case NOT_EQUAL_TEST:
    strncpy (ch, "<> ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) 
		ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case LESS_TEST:
    strncpy (ch, "< ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case GREATER_TEST:
    strncpy (ch, "> ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case LESS_OR_EQUAL_TEST:
    strncpy (ch, "<= ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case GREATER_OR_EQUAL_TEST:
    strncpy (ch, ">= ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case SAME_TYPE_TEST:
    strncpy (ch, "<=> ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    symbol_to_string (thisAgent, ct->data.referent, TRUE, ch, dest_size - (ch - dest));
    break;
  case DISJUNCTION_TEST:
    // BUGBUG: Need to think this through more carefully
    xmlString(pTag, "BUGBUG--Adding disjunction in XML--not done yet") ;
    strncpy (ch, "<< ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    for (c=ct->data.disjunction_list; c!=NIL; c=c->rest) {
      symbol_to_string (thisAgent, static_cast<symbol_union *>(c->first), TRUE, ch, dest_size - (ch - dest)); 
	  while (*ch) ch++;
      *(ch++) = ' ';
    }
    strncpy (ch, ">>", dest_size - (ch - dest));
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
    break;
  case CONJUNCTIVE_TEST:
    // BUGBUG: Need to think this through more carefully
    xmlString(pTag, "BUGBUG--Adding conjunction in XML--not done yet") ;
    strncpy (ch, "{ ", dest_size - (ch - dest)); 
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
	while (*ch) ch++;
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest) {
      xml_test (thisAgent, pTag, static_cast<char *>(c->first)) ; //, ch, dest_size - (ch - dest)); 
	  while (*ch) ch++;
      *(ch++) = ' ';
    }
    strncpy (ch, "}", dest_size - (ch - dest));
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
    break;
  case GOAL_ID_TEST:
    strncpy (dest, "[GOAL ID TEST]", dest_size - (ch - dest)); /* this should never get executed */
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
    break;
  case IMPASSE_ID_TEST:
    strncpy (dest, "[IMPASSE ID TEST]", dest_size - (ch - dest)); /* this should never get executed */
    ch[dest_size - (ch - dest) - 1] = 0; /* ensure null termination */
    break;
  }

  xmlString(pTag, dest) ;
  return ;
}
#endif //0

#define XML_CONDITION_LIST_TEMP_SIZE 10000
void xml_condition_list (agent* thisAgent, condition *conds, 
						   int indent, Bool internal) {
   dl_list *conds_not_yet_printed, *tail_of_conds_not_yet_printed;
   dl_list *conds_for_this_id;
   dl_cons *dc;
   condition *c;
   Bool removed_goal_test, removed_impasse_test;
   test id_test;  

   if (!conds) return;

   /* --- build dl_list of all the actions --- */
   conds_not_yet_printed = NIL;
   tail_of_conds_not_yet_printed = NIL;

   for (c=conds; c!=NIL; c=c->next) 
   {
      allocate_with_pool (thisAgent, &thisAgent->dl_cons_pool, &dc);
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
   Bool did_one_line_already = FALSE;
   while (conds_not_yet_printed) 
   {
      if (did_one_line_already) 
      {
         //print (thisAgent, "\n");
         //print_spaces (thisAgent, indent);
      } 
      else 
      {
         did_one_line_already = TRUE;
      }

      dc = conds_not_yet_printed;
      remove_from_dll (conds_not_yet_printed, dc, next, prev);
      c = static_cast<condition_struct *>(dc->item);
      if (c->type==CONJUNCTIVE_NEGATION_CONDITION) 
      {
         free_with_pool (&thisAgent->dl_cons_pool, dc);
         //print_string (thisAgent, "-{");
         xmlBeginTag(kTagConjunctive_Negation_Condition);
         xml_condition_list (thisAgent, c->data.ncc.top, indent+2, internal);
         xmlEndTag(kTagConjunctive_Negation_Condition);
         //print_string (thisAgent, "}");
         continue;
      }

      /* --- normal pos/neg conditions --- */
      removed_goal_test = removed_impasse_test = FALSE;
      id_test = copy_test_removing_goal_impasse_tests(thisAgent, c->data.tests.id_test, 
         &removed_goal_test, 
         &removed_impasse_test);
      thisAgent->id_test_to_match = copy_of_equality_test_found_in_test (thisAgent, id_test);

      /* --- collect all cond's whose id test matches this one --- */
      conds_for_this_id = dc;
      dc->prev = NIL;
      if (internal) 
      {
         dc->next = NIL;
      } 
      else 
      {
         dc->next = extract_dl_list_elements (thisAgent, &conds_not_yet_printed,
                                               xml_pick_conds_with_matching_id_test);
      }

	  // DJP: Moved this loop out so we get a condition tag per condition on this id
	  // rather than an id with a series of conditions.
      while (conds_for_this_id) 
      {
		  /* --- print the collected cond's all together --- */
		  //print_string (thisAgent, " (");
		  xmlBeginTag(kTagCondition);

		  if (removed_goal_test) 
		  {
			 //print_string (thisAgent, "state ");
			 xmlString(kConditionTest, kConditionTestState);

		  }

		  if (removed_impasse_test) 
		  {
			 //print_string (thisAgent, "impasse ");
			 xmlString(kConditionTest, kConditionTestImpasse);
		  }

		  //print_string (thisAgent, test_to_string (thisAgent, id_test, NULL, 0));
		  //xml_test(thisAgent, kConditionId, id_test) ;
		  xmlString(kConditionId, test_to_string(thisAgent, id_test, NULL, 0)) ;
		  deallocate_test (thisAgent, thisAgent->id_test_to_match);
		  deallocate_test (thisAgent, id_test);

	      //growable_string gs = make_blank_growable_string(thisAgent);
         dc = conds_for_this_id;
         conds_for_this_id = conds_for_this_id->next;
         c = static_cast<condition_struct *>(dc->item);
         free_with_pool (&thisAgent->dl_cons_pool, dc);

         { /* --- build and print attr/value test for condition c --- */
            char temp[XML_CONDITION_LIST_TEMP_SIZE], *ch;

			memset(temp,0,XML_CONDITION_LIST_TEMP_SIZE);
            ch = temp;
            //strncpy (ch, " ", XML_CONDITION_LIST_TEMP_SIZE - (ch - temp));
            if (c->type==NEGATIVE_CONDITION) 
            {
               strncat (ch, "-", XML_CONDITION_LIST_TEMP_SIZE - (ch - temp));
            }

            //strncat (ch, "^", XML_CONDITION_LIST_TEMP_SIZE - (ch - temp));
			while (*ch) ch++;
            test_to_string (thisAgent, c->data.tests.attr_test, ch, XML_CONDITION_LIST_TEMP_SIZE - (ch - temp)); 
			while (*ch) ch++;

			*ch = 0 ; // Terminate
			xmlString(kAttribute, temp) ;

			// Reset the ch pointer
			ch = temp ;
            if (! test_is_blank_test(c->data.tests.value_test)) 
            {
               *(ch++) = ' ';
               test_to_string (thisAgent, c->data.tests.value_test, ch, XML_CONDITION_LIST_TEMP_SIZE - (ch - temp)); 
			   while (*ch) ch++;
               if (c->test_for_acceptable_preference) 
               {
                  strncpy (ch, " +", XML_CONDITION_LIST_TEMP_SIZE - (ch - temp)); while (*ch) ch++;
               }
            }
            *ch = 0;
            if (thisAgent->printer_output_column + (ch - temp) >= COLUMNS_PER_LINE) 
            {
               //print_string (thisAgent, "\n");
               //print_spaces (thisAgent, indent+6);
            }
            //print_string (thisAgent, temp);
            //add_to_growable_string(thisAgent, &gs, temp);
		    xmlString(kValue, temp);
         }
		 //free_growable_string(thisAgent, gs);
      }
      //print_string (thisAgent, ")");
	  xmlEndTag(kTagCondition);
   } /* end of while (conds_not_yet_printed) */
}

void xml_condition (agent* thisAgent, condition *cond) {
  condition *old_next, *old_prev;

  old_next = cond->next;
  old_prev = cond->prev;
  cond->next = NIL;
  cond->prev = NIL;
  xml_condition_list (thisAgent, cond, 0, TRUE);
  cond->next = old_next;
  cond->prev = old_prev;
}

void xml_instantiation_with_wmes (agent* thisAgent, instantiation *inst, 
									wme_trace_type wtt, int action) 
{
  int PRINTING = -1;
  int FIRING = 0;
  int RETRACTING = 1;
  condition *cond;


  if (action == PRINTING) {
	  xmlBeginTag(kTagProduction);
  } else if (action == FIRING) {
	  xmlBeginTag(kTagProduction_Firing);
	  xmlBeginTag(kTagProduction);
  } else if (action == RETRACTING) {
	  xmlBeginTag(kTagProduction_Retracting);
	  xmlBeginTag(kTagProduction);
  }

  if (inst->prod) {
      //print_with_symbols  (thisAgent, "%y", inst->prod->name);
      xmlSymbol(thisAgent, kProduction_Name, inst->prod->name);
  } else {
      //print (thisAgent, "[dummy production]");
	  xmlString(kProduction_Name, "[dummy_production]");

  }

  //print (thisAgent, "\n");

  if (wtt==NONE_WME_TRACE) {
	  if (action == PRINTING) {
		  xmlEndTag(kTagProduction);
	  } else if (action == FIRING) {
		  xmlEndTag(kTagProduction);
		  xmlEndTag(kTagProduction_Firing);
	  } else if (action == RETRACTING) {
		  xmlEndTag(kTagProduction);
		  xmlEndTag(kTagProduction_Retracting);
	  }
	  return;
  }

  for (cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next)
    if (cond->type==POSITIVE_CONDITION) {
      switch (wtt) {
      case TIMETAG_WME_TRACE:
        //print (thisAgent, " %lu", cond->bt.wme_->timetag);

		xmlBeginTag(kTagWME);
		xmlULong(kWME_TimeTag, cond->bt.wme_->timetag);
		xmlEndTag(kTagWME);

        break;
      case FULL_WME_TRACE:	
		  if (action != RETRACTING) {
			  //print (thisAgent, " ");
			  xml_wme (thisAgent, cond->bt.wme_);
		  } else {
			  // Not all conds available when retracting, depending on DO_TOP_LEVEL_REF_CTS
			  #ifdef DO_TOP_LEVEL_REF_CTS
			  //print (thisAgent, " ");
			  xml_wme (thisAgent, cond->bt.wme_);
              #else

			  // Wmes that matched the LHS of a retraction may already be free'd; just print tt.
			  //print (thisAgent, " %lu", cond->bt.wme_->timetag);

			  xmlBeginTag(kTagWME);
			  xmlULong(kWME_TimeTag, cond->bt.wme_->timetag);
			  xmlEndTag(kTagWME);
 
              #endif
		  }
        break;
      }
    }
	
	if (action == PRINTING) {
		xmlEndTag(kTagProduction);
	} else if (action == FIRING) {
		xmlEndTag(kTagProduction);
		xmlEndTag(kTagProduction_Firing);
	} else if (action == RETRACTING) {
		xmlEndTag(kTagProduction);
		xmlEndTag(kTagProduction_Retracting);
	}
}

/////////////////////////////////////////////////////////////////
//
// XML version of print_match_set().
//
// Based on the print logic but generates XML directly.
//
/////////////////////////////////////////////////////////////////
void xml_match_set (agent* thisAgent, wme_trace_type wtt, ms_trace_type mst) {
  ms_change *msc;
  token temp_token;
  MS_trace *ms_trace = NIL, *tmp;

  /* --- Print assertions --- */  

  /* REW: begin 09.15.96 */
  if (thisAgent->operand2_mode == TRUE) {

    if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT) {
       //print (thisAgent, "O Assertions:\n");
		xmlBeginTag(kOAssertions) ;

       for (msc=thisAgent->ms_o_assertions; msc!=NIL; msc=msc->next) {

         if(wtt != NONE_WME_TRACE) {
		   xmlBeginTag(kTagProduction) ;
		   xmlSymbol(thisAgent, kName, msc->p_node->b.p.prod->name) ;
		   xmlSymbol(thisAgent, kGoal, msc->goal) ;
           //print_with_symbols (thisAgent, "  %y ", msc->p_node->b.p.prod->name);
           /* REW: begin 08.20.97 */
           /* Add match goal to the print of the matching production */
           //print_with_symbols(thisAgent, " [%y] ", msc->goal);
		   
           /* REW: end   08.20.97 */
           temp_token.parent = msc->tok;
           temp_token.w = msc->w;
           xml_whole_token (thisAgent, &temp_token, wtt);
           //print (thisAgent, "\n");
		   xmlEndTag(kTagProduction) ;
         }
         else {
           /* REW: begin 10.22.97 */
           if((tmp = in_ms_trace_same_goal(msc->p_node->b.p.prod->name,
                                           ms_trace, msc->goal))!=NIL) {
             /* REW: end   10.22.97 */
             tmp->count++;
           }
           else {
             tmp = static_cast<match_set_trace *>(allocate_memory(thisAgent, sizeof(MS_trace), MISCELLANEOUS_MEM_USAGE));
             tmp->sym = msc->p_node->b.p.prod->name;
             tmp->count = 1;
             tmp->next = ms_trace;
             /* REW: begin 08.20.97 */
             /* Add match goal to the print of the matching production */
             tmp->goal = msc->goal;
             /* REW: end   08.20.97 */
             ms_trace = tmp;
          }
        }
      }

      if (wtt == NONE_WME_TRACE) {
         while (ms_trace) {
		   xmlBeginTag(kTagProduction) ;
           tmp = ms_trace; ms_trace = tmp->next;
		   xmlSymbol(thisAgent, kName, tmp->sym) ;
		   xmlSymbol(thisAgent, kGoal, tmp->goal) ;
           if (tmp->count > 1)
			   xmlInt(kCount, tmp->count) ;	// DJP -- No idea what this count is
           //print_with_symbols (thisAgent, "  %y ", tmp->sym);
           /* REW: begin 08.20.97 */
           /*  BUG: for now this will print the goal of the first
               assertion inspected, even though there can be multiple
               assertions at different levels. 
               See 2.110 in the OPERAND-CHANGE-LOG. */
           //print_with_symbols(thisAgent, " [%y] ", tmp->goal);
           /* REW: end  08.20.97 */
           //if (tmp->count > 1)
           //  print(thisAgent, "(%d)\n", tmp->count);
           //else
           //  print(thisAgent, "\n");
           free_memory(thisAgent, (void *)tmp, MISCELLANEOUS_MEM_USAGE);
	       xmlEndTag(kTagProduction) ;
        }
      }
	  xmlEndTag(kOAssertions) ;
	}

     if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT) {
       //print (thisAgent, "I Assertions:\n");
	   xmlBeginTag(kIAssertions) ;
       for (msc=thisAgent->ms_i_assertions; msc!=NIL; msc=msc->next) {

         if(wtt != NONE_WME_TRACE) {
           //print_with_symbols (thisAgent, "  %y ", msc->p_node->b.p.prod->name);
           /* REW: begin 08.20.97 */
           /* Add match goal to the print of the matching production */
           //print_with_symbols(thisAgent, " [%y] ", msc->goal);
		   xmlBeginTag(kTagProduction) ;
		   xmlSymbol(thisAgent, kName, msc->p_node->b.p.prod->name) ;
		   xmlSymbol(thisAgent, kGoal, msc->goal) ;

           /* REW: end   08.20.97 */
           temp_token.parent = msc->tok;
           temp_token.w = msc->w;
           xml_whole_token (thisAgent, &temp_token, wtt);
           //print (thisAgent, "\n");
	       xmlEndTag(kTagProduction) ;
         }
         else {
           /* REW: begin 10.22.97 */
           if((tmp = in_ms_trace_same_goal(msc->p_node->b.p.prod->name,
                                           ms_trace, msc->goal))!=NIL) {
             /* REW: end   10.22.97 */
             tmp->count++;
           }
           else {
             tmp = static_cast<match_set_trace *>(allocate_memory(thisAgent, sizeof(MS_trace), 
                                                                                                                          MISCELLANEOUS_MEM_USAGE));
             tmp->sym = msc->p_node->b.p.prod->name;
             tmp->count = 1;
             tmp->next = ms_trace;
             /* REW: begin 08.20.97 */
             /* Add match goal to the print of the matching production */
             tmp->goal = msc->goal;
             /* REW: end   08.20.97 */
             ms_trace = tmp;
          }
        }
      }

      if (wtt == NONE_WME_TRACE) {
         while (ms_trace) {
           tmp = ms_trace; ms_trace = tmp->next;
		   xmlBeginTag(kTagProduction) ;
		   xmlSymbol(thisAgent, kName, tmp->sym) ;
		   xmlSymbol(thisAgent, kGoal, tmp->goal) ;
           if (tmp->count > 1)
			   xmlInt(kCount, tmp->count) ;	// DJP -- No idea what this count is
           //print_with_symbols (thisAgent, "  %y ", tmp->sym);
           /* REW: begin 08.20.97 */
           /*  BUG: for now this will print the goal of the first
               assertion inspected, even though there can be multiple
               assertions at different levels. 
               See 2.110 in the OPERAND-CHANGE-LOG. */
           //print_with_symbols(thisAgent, " [%y] ", tmp->goal);
           /* REW: end  08.20.97 */
           //if (tmp->count > 1)
           //  print(thisAgent, "(%d)\n", tmp->count);
           //else
           //  print(thisAgent, "\n");

           free_memory(thisAgent, (void *)tmp, MISCELLANEOUS_MEM_USAGE);
	       xmlEndTag(kTagProduction) ;
        }
      }
    }
	xmlEndTag(kIAssertions) ;
  }
  /* REW: end   09.15.96 */

  else


  if (mst == MS_ASSERT_RETRACT || mst == MS_ASSERT) {
    xmlBeginTag(kAssertions) ;
    //print (thisAgent, "Assertions:\n");
    for (msc=thisAgent->ms_assertions; msc!=NIL; msc=msc->next) {
      if(wtt != NONE_WME_TRACE) {
	    xmlBeginTag(kTagProduction) ;
	    xmlSymbol(thisAgent, kName, msc->p_node->b.p.prod->name) ;
        //print_with_symbols (thisAgent, "  %y\n ", msc->p_node->b.p.prod->name);
        temp_token.parent = msc->tok;
        temp_token.w = msc->w;
        xml_whole_token (thisAgent, &temp_token, wtt);
		xmlEndTag(kTagProduction) ;
        //print (thisAgent, "\n");
      } else {
        if((tmp = in_ms_trace(msc->p_node->b.p.prod->name, ms_trace))!=NIL) {
          tmp->count++;
        } else {
          tmp = static_cast<match_set_trace *>(allocate_memory(thisAgent, sizeof(MS_trace), 
                                                                                                                           MISCELLANEOUS_MEM_USAGE));
          tmp->sym = msc->p_node->b.p.prod->name;
          tmp->count = 1;
          tmp->next = ms_trace;
          ms_trace = tmp;
        }
      }
    }
    if (wtt == NONE_WME_TRACE) {
      while (ms_trace) {
        tmp = ms_trace; ms_trace = tmp->next;
	    xmlBeginTag(kTagProduction) ;
	    xmlSymbol(thisAgent, kName, tmp->sym) ;
	    xmlSymbol(thisAgent, kGoal, tmp->goal) ;
        if (tmp->count > 1)
		   xmlInt(kCount, tmp->count) ;	// DJP -- No idea what this count is
        //print_with_symbols (thisAgent, "  %y ", tmp->sym);
        //if (tmp->count > 1)
        //  print(thisAgent, "(%d)\n", tmp->count);
        //else
        //  print(thisAgent, "\n");
        free_memory(thisAgent, (void *)tmp, MISCELLANEOUS_MEM_USAGE);
	    xmlEndTag(kTagProduction) ;
      }
    }
	xmlEndTag(kAssertions) ;
  }

  /* --- Print retractions --- */  
  if (mst == MS_ASSERT_RETRACT || mst == MS_RETRACT) {
    xmlBeginTag(kRetractions) ;
    //print (thisAgent, "Retractions:\n");
    for (msc=thisAgent->ms_retractions; msc!=NIL; msc=msc->next) {
      if(wtt != NONE_WME_TRACE) {
        //print (thisAgent, "  ");
        xml_instantiation_with_wmes (thisAgent, msc->inst, wtt, -1);
        //print (thisAgent, "\n");
      } else {
        if(msc->inst->prod) {
          /* REW: begin 10.22.97 */
          if((tmp = in_ms_trace_same_goal(msc->inst->prod->name,
                                          ms_trace, msc->goal))!=NIL) {
            /* REW: end   10.22.97 */
            tmp->count++;
          } else {
            tmp = static_cast<match_set_trace *>(allocate_memory(thisAgent, sizeof(MS_trace), 
                                                                                                                         MISCELLANEOUS_MEM_USAGE));
            tmp->sym = msc->inst->prod->name;
            tmp->count = 1;
            tmp->next = ms_trace;
             /* REW: begin 08.20.97 */
             /* Add match goal to the print of the matching production */
             tmp->goal = msc->goal;
             /* REW: end   08.20.97 */
            ms_trace = tmp;
          }
        }
      }
    }
    if(wtt == NONE_WME_TRACE) {
      while (ms_trace) {
        tmp = ms_trace; ms_trace = tmp->next;
	    xmlBeginTag(kTagProduction) ;
	    xmlSymbol(thisAgent, kName, tmp->sym) ;
        if (tmp->goal)
			xmlSymbol(thisAgent, kGoal, tmp->goal) ;
		else
			xmlString(kGoal, "NIL") ;
        if (tmp->count > 1)
		   xmlInt(kCount, tmp->count) ;	// DJP -- No idea what this count is
        //print_with_symbols (thisAgent, "  %y ", tmp->sym);
           /* REW: begin 08.20.97 */
           /*  BUG: for now this will print the goal of the first assertion
               inspected, even though there can be multiple assertions at

               different levels. 
               See 2.110 in the OPERAND-CHANGE-LOG. */
        //if (tmp->goal)
        //  print_with_symbols(thisAgent, " [%y] ", tmp->goal);
        //else
        //  print(thisAgent, " [NIL] ");
           /* REW: end  08.20.97 */
        //if(tmp->count > 1)
        //  print(thisAgent, "(%d)\n", tmp->count);
        //else
        //  print(thisAgent, "\n");
        free_memory(thisAgent, (void *)tmp, MISCELLANEOUS_MEM_USAGE);
	    xmlEndTag(kTagProduction) ;
      }
    }
  }
}

/* --- Print stuff for given node and higher, up to but not including the
       cutoff node.  Return number of matches at the given node/cond. --- */
long xml_aux (agent* thisAgent,   /* current agent */
                           rete_node *node,    /* current node */
               rete_node *cutoff,  /* don't print cutoff node or any higher */
               condition *cond,    /* cond for current node */
               wme_trace_type wtt, /* what type of printout to use */
               int indent) {       /* number of spaces indent */
  token *tokens, *t, *parent_tokens;
  right_mem *rm;
  long matches_one_level_up;
  long matches_at_this_level;
  //#define MATCH_COUNT_STRING_BUFFER_SIZE 20
  //char match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE];
  rete_node *parent;

  /* --- find the number of matches for this condition --- */
  tokens = get_all_left_tokens_emerging_from_node (thisAgent, node);
  matches_at_this_level = 0;
  for (t=tokens; t!=NIL; t=t->next_of_node) matches_at_this_level++;
  deallocate_token_list (thisAgent, tokens);

  /* --- if we're at the cutoff node, we're done --- */
  if (node==cutoff) return matches_at_this_level;

  /* --- do stuff higher up --- */
  parent = real_parent_node(node);
  matches_one_level_up = xml_aux (thisAgent, parent, cutoff,
                                   cond->prev, wtt, indent);

  /* --- Form string for current match count:  If an earlier cond had no
     matches, just leave it blank; if this is the first 0, use ">>>>" --- */
  if (! matches_one_level_up) {
    //xmlInt(kMatchCount, 0) ;
    //strncpy (match_count_string, "    ", MATCH_COUNT_STRING_BUFFER_SIZE);
	//match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
  } else if (! matches_at_this_level) {
    //xmlInt(kMatchCount, 0) ;
    //strncpy (match_count_string, ">>>>", MATCH_COUNT_STRING_BUFFER_SIZE);
	//match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
  } else {
    //xmlInt(kMatchCount, matches_at_this_level) ;
    //snprintf (match_count_string, MATCH_COUNT_STRING_BUFFER_SIZE, "%4ld", matches_at_this_level);
	//match_count_string[MATCH_COUNT_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
  }

  /* --- print extra indentation spaces --- */
  //print_spaces (thisAgent, indent);

  if (cond->type==CONJUNCTIVE_NEGATION_CONDITION) {
    /* --- recursively print match counts for the NCC subconditions --- */
    xmlBeginTag(kTagConjunctive_Negation_Condition) ;
    //print (thisAgent, "    -{\n");
    xml_aux (thisAgent, real_parent_node(node->b.cn.partner),
              parent,
              cond->data.ncc.bottom,
              wtt,
              indent+5);
    //print_spaces (thisAgent, indent);
    //print (thisAgent, "%s }\n", match_count_string);
	xmlEndTag(kTagConjunctive_Negation_Condition) ;
  } else {
    //print (thisAgent, "%s", match_count_string);
    xml_condition (thisAgent, cond);

	// DJP: This is a trick to let us insert more attributes into xml_condition().
	xmlMoveCurrentToLastChild() ;
    // DJP: Moved this test from earlier down to here as no longer building match_count_string
    if (!matches_one_level_up)
		xmlInt(kMatchCount, 0) ;
	else
	    xmlInt(kMatchCount, matches_at_this_level) ;
	xmlMoveCurrentToParent() ;

    //print (thisAgent, "\n");
    /* --- if this is the first match-failure (0 matches), print info on
       matches for left and right --- */
    if (matches_one_level_up && (!matches_at_this_level)) {
      if (wtt!=NONE_WME_TRACE) {
        //print_spaces (thisAgent, indent);
		xmlBeginTag(kTagLeftMatches) ;
        //print (thisAgent, "*** Matches For Left ***\n");
        parent_tokens = get_all_left_tokens_emerging_from_node (thisAgent, parent);
        for (t=parent_tokens; t!=NIL; t=t->next_of_node) {
          //print_spaces (thisAgent, indent);
		  xmlBeginTag(kTagToken) ;
          xml_whole_token (thisAgent, t, wtt);
		  xmlEndTag(kTagToken) ;
          //print (thisAgent, "\n");
        }
        deallocate_token_list (thisAgent, parent_tokens);
		xmlEndTag(kTagLeftMatches) ;
        //print_spaces (thisAgent, indent);
        //print (thisAgent, "*** Matches for Right ***\n");
		xmlBeginTag(kTagRightMatches) ;
        //print_spaces (thisAgent, indent);
        for (rm=node->b.posneg.alpha_mem_->right_mems; rm!=NIL;
             rm=rm->next_in_am) {
          //if (wtt==TIMETAG_WME_TRACE) print (thisAgent, "%lu", rm->w->timetag);
          //else if (wtt==FULL_WME_TRACE) print_wme (thisAgent, rm->w);
          //print (thisAgent, " ");
          if (wtt==TIMETAG_WME_TRACE) xmlULong(kWME_TimeTag, rm->w->timetag);
          else if (wtt==FULL_WME_TRACE) xml_wme (thisAgent, rm->w);
        }
		xmlEndTag(kTagRightMatches) ;
        //print (thisAgent, "\n");
      }
    } /* end of if (matches_one_level_up ...) */
  }

  /* --- return result --- */  
  return matches_at_this_level;
}

void xml_partial_match_information (agent* thisAgent, rete_node *p_node, wme_trace_type wtt) {
  condition *top_cond, *bottom_cond;
  long n;
  token *tokens, *t;

  xmlBeginTag(kTagProduction) ;
  p_node_to_conditions_and_nots (thisAgent, p_node, NIL, NIL, &top_cond, &bottom_cond,
                                 NIL, NIL);
  n = xml_aux (thisAgent, p_node->parent, thisAgent->dummy_top_node, bottom_cond,
                wtt, 0);
  xmlInt(kMatches, n) ;
  //print (thisAgent, "\n%d complete matches.\n", n);
  if (n && (wtt!=NONE_WME_TRACE)) {
    print (thisAgent, "*** Complete Matches ***\n");
    tokens = get_all_left_tokens_emerging_from_node (thisAgent, p_node->parent);
    for (t=tokens; t!=NIL; t=t->next_of_node) {
      xml_whole_token (thisAgent, t, wtt);
      //print (thisAgent, "\n");
    }
    deallocate_token_list (thisAgent, tokens);
  }
  deallocate_condition_list (thisAgent, top_cond);
  xmlEndTag(kTagProduction) ;
}


/* **********************************************************************

   SECTION 19:  Rete Initialization

   EXTERNAL INTERFACE:
   Init_rete() initializes everything.
********************************************************************** */
void init_left_and_right_addition_routines()
{
   static bool is_initialized = false;
   if(!is_initialized)
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


void init_rete (agent* thisAgent) {

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

  init_memory_pool (thisAgent, &thisAgent->alpha_mem_pool, sizeof(alpha_mem),
                    "alpha mem");
  init_memory_pool (thisAgent, &thisAgent->rete_test_pool, sizeof(rete_test),
                    "rete test");
  init_memory_pool (thisAgent, &thisAgent->rete_node_pool, sizeof(rete_node),
                    "rete node");
  init_memory_pool (thisAgent, &thisAgent->node_varnames_pool,sizeof(node_varnames),
                    "node varnames");
  init_memory_pool (thisAgent, &thisAgent->token_pool, sizeof(token), "token");
  init_memory_pool (thisAgent, &thisAgent->right_mem_pool, sizeof(right_mem),
                    "right mem");
  init_memory_pool (thisAgent, &thisAgent->ms_change_pool, sizeof(ms_change),
                    "ms change");

  for (i=0; i<16; i++)
    thisAgent->alpha_hash_tables[i] = make_hash_table (thisAgent, 0, hash_alpha_mem);

  thisAgent->left_ht = allocate_memory_and_zerofill
    (thisAgent, sizeof(char *) * LEFT_HT_SIZE, HASH_TABLE_MEM_USAGE);
  thisAgent->right_ht = allocate_memory_and_zerofill
    (thisAgent, sizeof(char *) * RIGHT_HT_SIZE, HASH_TABLE_MEM_USAGE);

  init_dummy_top_node(thisAgent);

  thisAgent->max_rhs_unbound_variables = 1;
  thisAgent->rhs_variable_bindings = (Symbol **)
    allocate_memory_and_zerofill (thisAgent, sizeof(Symbol *), MISCELLANEOUS_MEM_USAGE);
  
  /* This is still not thread-safe. -AJC (8/9/02) */
  static Bool bInit = FALSE;
  if (bInit)
    return;

  bInit = TRUE;

  init_test_type_conversion_tables ();

  init_bnode_type_names(thisAgent);
  
  init_left_and_right_addition_routines();

  //
  // rete_test_routines is now statically initialized.
  //
  //for (i=0; i<256; i++) rete_test_routines[i] = error_rete_test_routine;
  rete_test_routines[DISJUNCTION_RETE_TEST] = disjunction_rete_test_routine;
  rete_test_routines[ID_IS_GOAL_RETE_TEST] = id_is_goal_rete_test_routine;
  rete_test_routines[ID_IS_IMPASSE_RETE_TEST]= id_is_impasse_rete_test_routine;
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
}
