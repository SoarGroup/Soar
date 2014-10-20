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

   Any_assertions_or_retractions_ready() returns true iff there are any
   pending changes to the match set.  This is used to test for quiescence.
   Get_next_assertion() retrieves a pending assertion (returning true) or
   returns false is no more are available.  Get_next_retraction() is
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
   files.  They return true if successful, false if any error occurred.
======================================================================= */

#ifndef RETE_H
#define RETE_H

#include <stdio.h>  // Needed for FILE token below
#include "kernel.h"

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
typedef struct cons_struct cons;
typedef char varnames;
typedef cons list;
typedef struct test_struct test_info;
typedef test_info* test;
extern void abort_with_fatal_error_noagent(const char* msg);

inline varnames* one_var_to_varnames(Symbol* x)
{
    return reinterpret_cast<varnames*>(x);
}
inline varnames* var_list_to_varnames(cons* x)
{
    return reinterpret_cast<varnames*>(reinterpret_cast<char*>(x) + 1);
}
inline uint64_t varnames_is_var_list(varnames* x)
{
    return reinterpret_cast<uint64_t>(x) & 1;
}
inline bool varnames_is_one_var(varnames* x)
{
    return ! varnames_is_var_list(x);
}
inline Symbol* varnames_to_one_var(varnames* x)
{
    return reinterpret_cast<Symbol*>(x);
}
inline list* varnames_to_var_list(varnames* x)
{
    return reinterpret_cast<list*>(static_cast<char*>(x) - 1);
}

/* --- tells where to find a variable --- */
typedef unsigned short rete_node_level;
Symbol* var_bound_in_reconstructed_conds(
    agent* thisAgent,
    condition* cond,
    byte where_field_num,
    rete_node_level where_levels_up);
test var_test_bound_in_reconstructed_conds(
    agent* thisAgent,
    condition* cond,
    byte where_field_num,
    rete_node_level where_levels_up);
Symbol* var_bound_in_reconstructed_original_conds(
    agent* thisAgent,
    condition* cond,
    byte where_field_num,
    rete_node_level where_levels_up);

/* ----------------------------------------------------------------------

       Structures and Declarations:  Alpha Portion of the Rete Net

---------------------------------------------------------------------- */

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


/* --- structure of each alpha memory --- */
typedef struct alpha_mem_struct
{
    struct alpha_mem_struct* next_in_hash_table;  /* next mem in hash bucket */
    struct right_mem_struct* right_mems;  /* dll of right_mem structures */
    struct rete_node_struct* beta_nodes;  /* list of attached beta nodes */
    struct rete_node_struct* last_beta_node; /* tail of above dll */
    Symbol* id;                  /* constants tested by this alpha mem */
    Symbol* attr;                /* (NIL if this alpha mem ignores that field) */
    Symbol* value;
    bool acceptable;             /* does it test for acceptable pref? */
    uint32_t am_id;            /* id for hashing */
    uint64_t reference_count;  /* number of beta nodes using this mem */
    uint64_t retesave_amindex;
} alpha_mem;

/* --- the entry for one WME in one alpha memory --- */
typedef struct right_mem_struct
{
    wme* w;                      /* the wme */
    alpha_mem* am;               /* the alpha memory */
    struct right_mem_struct* next_in_bucket, *prev_in_bucket; /*hash bucket dll*/
    struct right_mem_struct* next_in_am, *prev_in_am;       /*rm's in this amem*/
    struct right_mem_struct* next_from_wme, *prev_from_wme; /*tree-based remove*/
} right_mem;

/* Note: right_mem's are stored in hash table thisAgent->right_ht */

typedef struct var_location_struct
{
    rete_node_level levels_up; /* 0=current node's alphamem, 1=parent's, etc. */
    byte field_num;            /* 0=id, 1=attr, 2=value */
} var_location;


/* --- gives data for a test that must be applied at a node --- */
typedef struct rete_test_struct
{
    byte right_field_num;          /* field (0, 1, or 2) from wme */
    byte type;                     /* test type (ID_IS_GOAL_RETE_TEST, etc.) */
    union rete_test_data_union
    {
        var_location variable_referent;   /* for relational tests to a variable */
        Symbol* constant_referent;        /* for relational tests to a constant */
        list* disjunction_list;           /* list of symbols in disjunction test */
    } data;
    struct rete_test_struct* next; /* next in list of tests at the node */
} rete_test;

/* --- data for positive nodes only --- */
typedef struct pos_node_data_struct
{
    /* --- dll of left-linked pos nodes from the parent beta memory --- */
    struct rete_node_struct* next_from_beta_mem, *prev_from_beta_mem;
} pos_node_data;

/* --- data for both positive and negative nodes --- */
typedef struct posneg_node_data_struct
{
    rete_test* other_tests; /* tests other than the hashed test */
    alpha_mem* alpha_mem_;  /* the alpha memory this node uses */
    struct rete_node_struct* next_from_alpha_mem; /* dll of nodes using that */
    struct rete_node_struct* prev_from_alpha_mem; /*   ... alpha memory */
    struct rete_node_struct* nearest_ancestor_with_same_am;
} posneg_node_data;

/* --- data for beta memory nodes only --- */
typedef struct beta_memory_node_data_struct
{
    /* --- first pos node child that is left-linked --- */
    struct rete_node_struct* first_linked_child;
} beta_memory_node_data;

/* --- data for cn and cn_partner nodes only --- */
typedef struct cn_node_data_struct
{
    struct rete_node_struct* partner;    /* cn, cn_partner point to each other */
} cn_node_data;

/* --- data for production nodes only --- */
typedef struct p_node_data_struct
{
    struct production_struct* prod;                  /* the production */
    struct node_varnames_struct* parents_nvn;        /* records variable names */
    struct ms_change_struct* tentative_assertions;   /* pending MS changes */
    struct ms_change_struct* tentative_retractions;
} p_node_data;

#define O_LIST 0
#define I_LIST 1     /*   values for prod->OPERAND_which_assert_list */

/* --- data for all except positive nodes --- */
typedef struct non_pos_node_data_struct
{
    struct token_struct* tokens;           /* dll of tokens at this node */
    unsigned is_left_unlinked: 1;          /* used on mp nodes only */
} non_pos_node_data;

/* --- structure of a rete beta node --- */
typedef struct rete_node_struct
{
    byte node_type;                  /* tells what kind of node this is */

    /* -- used only on hashed nodes -- */
    /* field_num: 0=id, 1=attr, 2=value */
    byte left_hash_loc_field_num;
    /* left_hash_loc_levels_up: 0=current node's alphamem, 1=parent's, etc. */
    rete_node_level left_hash_loc_levels_up;
    /* node_id: used for hash function */
    uint32_t node_id;

#ifdef SHARING_FACTORS
    uint64_t sharing_factor;
#endif

    struct rete_node_struct* parent;       /* points to parent node */
    struct rete_node_struct* first_child;  /* used for dll of all children, */
    struct rete_node_struct* next_sibling; /*   regardless of unlinking status */
    union rete_node_a_union
    {
        pos_node_data pos;                   /* for pos. nodes */
        non_pos_node_data np;                /* for all other nodes */
    } a;
    union rete_node_b_union
    {
        posneg_node_data posneg;            /* for pos, neg, mp nodes */
        beta_memory_node_data mem;          /* for beta memory nodes */
        cn_node_data cn;                    /* for cn, cn_partner nodes */
        p_node_data p;                      /* for p nodes */
    } b;
} rete_node;

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

/* --- types of tests found at beta nodes --- */
#define CONSTANT_RELATIONAL_RETE_TEST 0x00
#define VARIABLE_RELATIONAL_RETE_TEST 0x10
#define DISJUNCTION_RETE_TEST         0x20
#define ID_IS_GOAL_RETE_TEST          0x30
#define ID_IS_IMPASSE_RETE_TEST       0x31
//#define test_is_constant_relational_test(x) (((x) & 0xF0)==0x00)
//#define test_is_variable_relational_test(x) (((x) & 0xF0)==0x10)

inline bool test_is_constant_relational_test(byte x)
{
    return (((x) & 0xF0) == CONSTANT_RELATIONAL_RETE_TEST);
}

inline bool test_is_variable_relational_test(byte x)
{
    return (((x) & 0xF0) == VARIABLE_RELATIONAL_RETE_TEST);
}


//#define kind_of_relational_test(x) ((x) & 0x0F)
//#define test_is_not_equal_test(x) (((x)==0x01) || ((x)==0x11))

inline byte kind_of_relational_test(byte x)
{
    return ((x) & 0x0F);
}

inline bool test_is_not_equal_test(byte x)
{
    return (((x) == (CONSTANT_RELATIONAL_RETE_TEST + RELATIONAL_NOT_EQUAL_RETE_TEST))
            || ((x) == (VARIABLE_RELATIONAL_RETE_TEST + RELATIONAL_NOT_EQUAL_RETE_TEST)));
}

typedef struct three_field_varnames_struct
{
    varnames* id_varnames;
    varnames* attr_varnames;
    varnames* value_varnames;
} three_field_varnames;

typedef struct node_varnames_struct
{
    struct node_varnames_struct* parent;
    union varname_data_union
    {
        three_field_varnames fields;
        struct node_varnames_struct* bottom_of_subconditions;
    } data;
} node_varnames;

typedef struct token_struct
{
    /* --- Note: "parent" is NIL on negative node negrm (local join result)
       tokens, non-NIL on all other tokens including CN and CN_P stuff.
       I put "parent" at offset 0 in the structure, so that upward scans
       are fast (saves doing an extra integer addition in the inner loop) --- */
    struct token_struct* parent;
    union token_a_union
    {
        struct token_in_hash_table_data_struct
        {
            struct token_struct* next_in_bucket, *prev_in_bucket; /*hash bucket dll*/
            Symbol* referent; /* referent of the hash test (thing we hashed on) */
        } ht;
        struct token_from_right_memory_of_negative_or_cn_node_struct
        {
            struct token_struct* next_negrm, *prev_negrm;/*other local join results*/
            struct token_struct* left_token; /* token this is local join result for*/
        } neg;
    } a;
    rete_node* node;
    wme* w;
    struct token_struct* first_child;  /* first of dll of children */
    struct token_struct* next_sibling, *prev_sibling; /* for dll of children */
    struct token_struct* next_of_node, *prev_of_node; /* dll of tokens at node */
    struct token_struct* next_from_wme, *prev_from_wme; /* tree-based remove */
    struct token_struct* negrm_tokens; /* join results: for Neg, CN nodes only */
} token;

extern void init_rete(agent* thisAgent);

extern bool any_assertions_or_retractions_ready(agent* thisAgent);
extern bool postpone_assertion(agent* thisAgent, production** prod, struct token_struct** tok, wme** w);
extern void consume_last_postponed_assertion(agent* thisAgent);
extern void restore_postponed_assertions(agent* thisAgent);
extern bool get_next_retraction(agent* thisAgent, struct instantiation_struct** inst);
/* REW: begin 08.20.97 */
/* Special routine for retractions in removed goals.  See note in rete.cpp */
extern bool get_next_nil_goal_retraction(agent* thisAgent, struct instantiation_struct** inst);
/* REW: end   08.20.97 */

#define NO_REFRACTED_INST 0              /* no refracted inst. was given */
#define REFRACTED_INST_MATCHED 1         /* there was a match for the inst. */
#define REFRACTED_INST_DID_NOT_MATCH 2   /* there was no match for it */
#define DUPLICATE_PRODUCTION 3           /* the prod. was a duplicate */
extern byte add_production_to_rete(agent* thisAgent, production* p, condition* lhs_top,
                                   instantiation* refracted_inst,
                                   bool warn_on_duplicates, bool ignore_rhs = false);
extern void excise_production_from_rete(agent* thisAgent, production* p);

extern void add_wme_to_rete(agent* thisAgent, wme* w);
extern void remove_wme_from_rete(agent* thisAgent, wme* w);

extern void p_node_to_conditions_and_rhs(agent* thisAgent,
        struct rete_node_struct* p_node,
        struct token_struct* tok,
        wme* w,
        condition** dest_top_cond,
        condition** dest_bottom_cond,
        action** dest_rhs,
        AddAdditionalTestsMode additional_tests = DONT_ADD_TESTS);
extern Symbol* get_symbol_from_rete_loc(unsigned short levels_up,
                                        byte field_num,
                                        struct token_struct* tok, wme* w);

extern uint64_t count_rete_tokens_for_production(agent* thisAgent, production* prod);
extern void print_partial_match_information(agent* thisAgent, struct rete_node_struct* p_node,
        wme_trace_type wtt);
extern void xml_partial_match_information(agent* thisAgent, rete_node* p_node, wme_trace_type wtt) ;

extern void print_match_set(agent* thisAgent, wme_trace_type wtt, ms_trace_type  mst);
extern void xml_match_set(agent* thisAgent, wme_trace_type wtt, ms_trace_type  mst);
extern void get_all_node_count_stats(agent* thisAgent);
extern int get_node_count_statistic(agent* thisAgent, char* node_type_name,
                                    char* column_name,
                                    uint64_t* result);

extern bool save_rete_net(agent* thisAgent, FILE* dest_file, bool use_rete_net_64);
extern bool load_rete_net(agent* thisAgent, FILE* source_file);

extern void add_varnames_to_test(agent* thisAgent, varnames* vn, test* t);

/* ---------------------------------------------------------------------

       Test Type <---> Relational (Rete) Test Type Conversion

   These functions convert from xxx_TEST's (defined in test.h for various
   kinds of tests) to xxx_RETE_TEST's (defined in rete.cpp for
   the different kinds of Rete tests), and vice-versa.  We might just
   use the same set of constants for both purposes, but we want to be
   able to do bit-twiddling on the RETE_TEST types.

--------------------------------------------------------------------- */

inline TestType relational_test_type_to_test_type(byte test_type)
{
    /* we don't need ...[equal test] */
    switch (test_type)
    {
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
    abort_with_fatal_error_noagent("Bad test_type in add_rete_test_to_test!!!\n");
    return EQUALITY_TEST;
}
inline byte test_type_to_relational_test_type(byte test_type)
{
    /* we don't need ...[equal test] */
    switch (test_type)
    {
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
