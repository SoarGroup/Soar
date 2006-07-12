/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* gdatastructs.h */

/* ========================================================================= */
/*                                                                           */
/*                         Global Data Structures                            */
/*                                                                           */
/* ========================================================================= */

#ifndef GDATASTRUCTS_H
#define GDATASTRUCTS_H

#include "symtab.h" /* needed for definition of symbol_union */
#include "kernel.h" /* needed for definition of NIL */

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef unsigned char byte;
typedef unsigned long tc_number;
typedef signed short goal_stack_level;
typedef struct complex_test_struct complex_test;
typedef struct cons_struct cons;
typedef struct dl_cons_struct dl_cons;
typedef struct wme_struct wme;
typedef union symbol_union Symbol;
typedef cons list;

/* REW: begin 09.15.96 */

/* ------------------------------------------------------------------------
			     Goal Dependency Set

   The Goal Dependency Set is a data strcuture used in Operand2 to maintain
   the integrity of a subgoal with respect to changes in supergoal WMEs.
   Whenever a WME in the goal's dependency set changes, the goal is immediately
   removed.  The routines for maintaining the GDS and determining if a goal
   should be retracted are in decide.c

   Fields in a goal dependency set:

      goal:  points to the goal for which this dependency set was created.
             The goal also has a pointer back to the GDS.

      wmes_in_gds:  A DLL of WMEs in the goal dependency set

   The GDS is created only when necessary; that is, when an o-suppported WME
   is created in some subgoal and that subgoal has no GDS already.  The
   instantiations that led to the creation of the o-supported WME are 
   examined; any supergoal WMEs in these instantiations are added to the 
   wmes_in_gds DLL.  The GDS for each goal is examined for every WM change;
   if a WME changes that is on a GDS, the goal that the GDS points to is
   immediately removed.  

   When a goal is removed, the GDS is not immediately removed.  Instead,
   whenever a WME is removed (or when it is added to another GDS), we check
   to also make certain that its GDS has other WMEs on the wmes_in_gds DLL.
   If not, then we remove the GDS then.  This delay avoids having to scan
   over all the WMEs in the GDS in addition to removing the goal (i.e., the
   maintenance cost is amortized over a number of WM phases).

   */

typedef struct gds_struct {
  Symbol *goal;                /* pointer to the goal for the dependency set */
  wme *wmes_in_gds;            /* pointer to the dll of WMEs in GDS of goal */
} goal_dependency_set;
/* REW: end   09.15.96 */

/* ------------------------------------------------------------------------
                               Preferences

   Fields in a preference:

      type:  indicates the type of the preference.  This is one of the
             types defined below:  ACCEPTABLE_PREFERENCE_TYPE, etc.

      o_supported:  TRUE iff the preference has o-support

      in_tm:  TRUE iff the preference is currently in temporary memory

      on_goal_list:  TRUE iff the preference is on the list of preferences
                     supported by its match goal (see all_of_goal_next below)

      reference_count:  (see below)

      id, attr, value, referent:  points to the symbols.  Referent is only
                                  used for binary preferences.

      slot:  points to the slot this preference is for.  (NIL if the
        preference is not in TM.)

      next, prev:  used for a doubly-linked list of preferences of the
                   same type in that particular slot

      all_of_slot_next, all_of_slot_prev:  used for a doubly-linked list
          of all preferences (of any type) in that particular slot

      all_of_goal_next, all_of_goal_prev:  used for a doubly-linked list
          of all preferences supported by this particular match goal.
          This is needed in order to remove all o-support from a particular
          goal when that goal is removed from the context stack.

      next_clone, prev_clone:  used for a doubly-linked list of all "clones"
        of this preference.  When a result is returned from a subgoal and a
        chunk is built, we get two copies of the "same" preference, one from
        the subgoal's production firing, and one from the chunk instantiation.
        If results are returned more than one level, or the same result is
        returned simultaneously by multiple production firings, we can get
        lots of copies of the "same" preference.  These clone preferences
        are kept on a list so that we can find the right one to backtrace
        through, given a wme supported by "all the clones."

      inst:  points to the instantiation that generated this preference

      inst_next, inst_prev:  used for a doubly-linked list of all
        existing preferences that were generated by that instantiation

      next_candidate:  used by the decider for lists of candidate values
        for a certain slot

      next_result:  used by the chunker for a list of result preferences

   Reference counts on preferences:
      +1 if the preference is currently in TM
      +1 for each instantiation condition that points to it (bt.trace)
      +1 if it supports an installed context WME

   We deallocate a preference if:
      (1) reference_count==0 and all its clones have reference_count==0
          (hence it couldn't possibly be needed anymore)
   or (2) its match goal is removed from the context stack
          (hence there's no way we'll ever want to BT through it)
------------------------------------------------------------------------ */

/* WARNING: preference types must be numbered 0..(NUM_PREFERENCE_TYPES-1),
   because the slot structure contains an array using these indices. */
#define NUM_PREFERENCE_TYPES 15  /* number of different preference types */

#define ACCEPTABLE_PREFERENCE_TYPE 0
#define REQUIRE_PREFERENCE_TYPE 1
#define REJECT_PREFERENCE_TYPE 2
#define PROHIBIT_PREFERENCE_TYPE 3
#define RECONSIDER_PREFERENCE_TYPE 4
#define UNARY_INDIFFERENT_PREFERENCE_TYPE 5
#define UNARY_PARALLEL_PREFERENCE_TYPE 6
#define BEST_PREFERENCE_TYPE 7
#define WORST_PREFERENCE_TYPE 8
#define BINARY_INDIFFERENT_PREFERENCE_TYPE 9
#define BINARY_PARALLEL_PREFERENCE_TYPE 10
#define BETTER_PREFERENCE_TYPE 11
#define WORSE_PREFERENCE_TYPE 12
#define NUMERIC_INDIFFERENT_PREFERENCE_TYPE 13 
 // #define TEMPLATE_PREFERENCE_TYPE 14

#ifdef USE_MACROS

#define preference_is_unary(p) ((p)<9)
#define preference_is_binary(p) ((p)>8)

#else

inline Bool preference_is_unary(byte p)
{
  return (p < 9);
}

inline Bool preference_is_binary(byte p)
{
  return (p > 8);
}

#endif /* USE_MACROS */

#ifdef _cplusplus
extern "C" {
#endif

   extern char * preference_name[NUM_PREFERENCE_TYPES];

#ifdef _cplusplus
}
#endif

typedef struct preference_struct {
  byte type;         /* acceptable, better, etc. */
  Bool o_supported;  /* is the preference o-supported? */
  Bool in_tm;        /* is this currently in TM? */
  Bool on_goal_list; /* is this pref on the list for its match goal */
  unsigned long reference_count;
  Symbol *id;
  Symbol *attr;
  Symbol *value;
  Symbol *referent;
  struct slot_struct *slot;

  /* dll of pref's of same type in same slot */
  struct preference_struct *next, *prev;

  /* dll of all pref's in same slot */
  struct preference_struct *all_of_slot_next, *all_of_slot_prev;

  /* dll of all pref's from the same match goal */
  struct preference_struct *all_of_goal_next, *all_of_goal_prev;
  
  /* dll (without header) of cloned preferences (created when chunking) */
  struct preference_struct *next_clone, *prev_clone;
    
  struct instantiation_struct *inst;
  struct preference_struct *inst_next, *inst_prev;
  struct preference_struct *next_candidate;
  struct preference_struct *next_result;

  /////#ifdef NUMERIC_INDIFFERENCE
  //   These members used only when #def'd NUMERIC_INDIFFERENCE
  //   in kernel.h but always compiled for clarity.
    /* REW: 2003-01-08 Behavior Variability Kernel Experiements
       See decide.c for more information
       This is just a hack until we determine
       what we really want from these changes.
     */
  int total_preferences_for_candidate;
  float numeric_value;  /* RL.  snason */
    /* END: REW: 2003-01-08 */
  /////#endif

} preference;

/* Decl'd in prefmem.cpp and needed in decide.cpp */
extern Bool remove_preference_from_clones (agent* thisAgent, preference *pref);

/* ------------------------------------------------------------------------

                             Impasse Types

------------------------------------------------------------------------ */

#define NONE_IMPASSE_TYPE 0                   /* no impasse */
#define CONSTRAINT_FAILURE_IMPASSE_TYPE 1
#define CONFLICT_IMPASSE_TYPE 2
#define TIE_IMPASSE_TYPE 3
#define NO_CHANGE_IMPASSE_TYPE 4
#define STATE_NO_CHANGE_IMPASSE_TYPE 5		/* for RL */
#define OP_NO_CHANGE_IMPASSE_TYPE 6		/* for RL */
/* ------------------------------------------------------------------------
                                Slots

   Fields in a slot:

      next, prev:  used for a doubly-linked list of all slots for a certain
        identifier.

      id, attr:   identifier and attribute of the slot

      wmes:  header of a doubly-linked list of all wmes in the slot

      acceptable_preference_wmes:  header of doubly-linked list of all
        acceptable preference wmes in the slot.  (This is only used for
        context slots.)

      all_preferences:  header of a doubly-linked list of all preferences
        currently in the slot

      preferences[NUM_PREFERENCE_TYPES]: array of headers of doubly-linked
        lists, one for each possible type of preference.  These store
        all the preferences, sorted into lists according to their types.
        Within each list, the preferences are sorted according to their
        match goal, with the pref. supported by the highest goal at the
        head of the list.

      impasse_id:  points to the identifier of the attribute impasse object
        for this slot.  (NIL if the slot isn't impassed.)

      isa_context_slot:  TRUE iff this is a context slot

      impasse_type:  indicates the type of the impasse for this slot.  This
        is one of NONE_IMPASSE_TYPE, CONSTRAINT_FAILURE_IMPASSE_TYPE, etc.
  
      marked_for_possible_removal:  TRUE iff this slot is on the list of
        slots that might be deallocated at the end of the current top-level
        phase.

      changed:  indicates whether the preferences for this slot have changed.
        For non-context slots, this is either NIL or a pointer to the
        corresponding dl_cons in changed_slots (see decide.c); for context
        slots, it's just a zero/nonzero flag.

      acceptable_preference_changed:  for context slots only; this is zero
        if no acceptable or require preference in this slot has changed;
        if one has changed, it points to a dl_cons.
------------------------------------------------------------------------ */

typedef struct slot_struct {
  struct slot_struct *next, *prev;  /* dll of slots for this id */
  Symbol *id;                       /* id, attr of the slot */
  Symbol *attr;
  wme *wmes;                        /* dll of wmes in the slot */
  wme *acceptable_preference_wmes;  /* dll of acceptable pref. wmes */
  preference *all_preferences;      /* dll of all pref's in the slot */
  preference *preferences[NUM_PREFERENCE_TYPES]; /* dlls for each type */
  Symbol *impasse_id;               /* NIL if slot is not impassed */
  Bool isa_context_slot;            
  byte impasse_type;
  Bool marked_for_possible_removal;
  dl_cons *changed;   /* for non-context slots: points to the corresponding
                         dl_cons in changed_slots;  for context slots: just
                         zero/nonzero flag indicating slot changed */
  dl_cons *acceptable_preference_changed; /* for context slots: either zero,
                                             or points to dl_cons if the slot
                                             has changed + or ! pref's */
} slot;

/* -------------------------------------------------------------------
                              Tests
   
   Tests in conditions can be blank (null) tests, tests for equality
   with a variable or constant, or more complicated tests (such as
   not-equal tests, conjunctive tests, etc.).  We use some bit twiddling
   here to minimize space.  We use just a pointer to represent any kind
   of test.  For blank tests, this is the NIL pointer.  For equality tests,
   it points to the symbol referent of the test.  For other kinds of tests,
   bit 0 of the pointer is set to 1, and the pointer (minus 1) points to
   a complex_test structure.  (A field in the complex_test structure 
   further indicates the type of the test.)
------------------------------------------------------------------- */

typedef char * test;

#ifdef USE_MACROS

#define test_is_blank_test(t) ((t)==NIL)
#define test_is_complex_test(t) (((unsigned long)(t)) & 1)
#define test_is_blank_or_equality_test(t) (! test_is_complex_test(t))

#define make_blank_test() ((test)NIL)
#define make_equality_test(sym) ((sym)->common.reference_count++, (test)(sym)) // what's this???
#define make_equality_test_without_adding_reference(sym) ((test)(sym))
#define make_blank_or_equality_test(sym_or_nil) \
  ((sym_or_nil) ? make_equality_test(sym_or_nil) : make_blank_test() )
#define make_test_from_complex_test(ct) ((test) (((char *)(ct))+1))

#define referent_of_equality_test(t) ((Symbol *) (t))
#define complex_test_from_test(t) ((complex_test *) (((char *)(t))-1))
 
#else

inline Bool test_is_blank_test(test t) 
{ 
  return (t == NIL); 
}

#ifdef _MSC_VER
#pragma warning (disable : 4311)
#endif

inline Bool test_is_complex_test(test t) 
{ 
  return (char)(reinterpret_cast<unsigned long>(t) & 1);
}

#ifdef _MSC_VER
#pragma warning (default : 4311)
#endif

inline Bool test_is_blank_or_equality_test(test t)
{
  return (!test_is_complex_test(t));
}

inline test make_blank_test()
{
  return static_cast<test>(NIL);
}

inline test make_equality_test(Symbol * sym) // is this equivalent to the macro above??
{
  (sym)->common.reference_count++;
  return reinterpret_cast<test>(sym);
}

inline test make_equality_test_without_adding_reference(Symbol * sym)
{
  return reinterpret_cast<test>(sym);
}

inline test make_blank_or_equality_test(Symbol * sym_or_nil)
{
  return ((sym_or_nil) ? (make_equality_test(sym_or_nil)) : make_blank_test());
}

inline char * make_test_from_complex_test(complex_test * ct)
{
  return static_cast<test>(reinterpret_cast<char *>(ct) + 1);
}

inline Symbol * referent_of_equality_test(test t)
{
  return reinterpret_cast<Symbol *>(t);
}

inline complex_test * complex_test_from_test(test t)
{
  return reinterpret_cast<complex_test *>(static_cast<char *>(t) - 1);
}

#endif /* USE_MACROS */

typedef struct complex_test_struct {
  byte type;                  /* see definitions below */
  union test_info_union {
    Symbol *referent;         /* for relational tests */
    list *disjunction_list;   /* for disjunction tests */
    list *conjunct_list;      /* for conjunctive tests */
  } data;
} complex_test;

/* types of the complex_test's */
/* WARNING -- none of these can be 254 or 255 -- see rete.cpp */
//#define NOT_EQUAL_TEST 1         /* various relational tests */
//#define LESS_TEST 2
//#define GREATER_TEST 3
//#define LESS_OR_EQUAL_TEST 4
//#define GREATER_OR_EQUAL_TEST 5
//#define SAME_TYPE_TEST 6
//#define DISJUNCTION_TEST 7       /* item must be one of a list of constants */
//#define CONJUNCTIVE_TEST 8       /* item must pass each of a list of tests */
//#define GOAL_ID_TEST 9           /* item must be a goal identifier */
//#define IMPASSE_ID_TEST 10       /* item must be an impasse identifier */
enum ComplexTextTypes {
         NOT_EQUAL_TEST = 1,         /* various relational tests */
         LESS_TEST = 2,
         GREATER_TEST = 3,
         LESS_OR_EQUAL_TEST = 4,
         GREATER_OR_EQUAL_TEST = 5,
         SAME_TYPE_TEST = 6,
         DISJUNCTION_TEST = 7,       /* item must be one of a list of constants */
         CONJUNCTIVE_TEST = 8,       /* item must pass each of a list of tests */
         GOAL_ID_TEST = 9,           /* item must be a goal identifier */
         IMPASSE_ID_TEST = 10       /* item must be an impasse identifier */
};

#define NUM_TEST_TYPES 10

//
// Symbol types.
//
#define VARIABLE_SYMBOL_TYPE 0
#define IDENTIFIER_SYMBOL_TYPE 1
#define SYM_CONSTANT_SYMBOL_TYPE 2
#define INT_CONSTANT_SYMBOL_TYPE 3
#define FLOAT_CONSTANT_SYMBOL_TYPE 4
#define NUM_SYMBOL_TYPES 5
#define NUM_PRODUCTION_TYPES 5


/* -------------------------------------------------------------------
                             Conditions

   Conditions are used for two things:  (1) to represent the LHS of newly
   entered productions (new SP's or chunks); and (2) to represent the 
   instantiated LHS in production instantiations.
   
   Fields in a condition:

      type:  indicates the type of condition:  either POSITIVE_CONDITION,
        NEGATIVE_CONDITION, or CONJUNCTIVE_NEGATION_CONDITION.

      already_in_tc:  (reserved for use by the cond_is_in_tc() stuff in
        production.c)

      next, prev:  used for a doubly-linked list of all conditions on the
        LHS, or all subconditions of an NCC.

      data.tests.id_test, data.tests.attr_test, data.tests.value_test:
        for positive and negative conditions, these are the three wme
        field tests for the condition.

      test_for_acceptable_preference:  for positive and negative conditions,
        this is TRUE iff the condition tests for acceptable preference wmes.

      data.ncc.top, data.ncc.bottom:  for NCC's, these point to the top and
        bottom of the subconditions likned list.

      bt:  for top-level positive conditions in production instantiations,
        this structure gives information for that will be used in backtracing.

      reorder:  (reserved for use by the reorderer)
------------------------------------------------------------------- */

/* --- types of conditions --- */
#define POSITIVE_CONDITION 0
#define NEGATIVE_CONDITION 1
#define CONJUNCTIVE_NEGATION_CONDITION 2

/* --- info on conditions used for backtracing (and by the rete) --- */
typedef struct bt_info_struct {
  wme * wme_;               /* the actual wme that was matched */
  goal_stack_level level;   /* level (at firing time) of the id of the wme */
  preference *trace;        /* preference for BT, or NIL */

  /* mvp 5-17-94 */
  list *prohibits;          /* list of prohibit prefs to backtrace through */
} bt_info;

/* --- info on conditions used only by the reorderer --- */
typedef struct reorder_info_struct {
  list *vars_requiring_bindings;           /* used only during reordering */
  struct condition_struct *next_min_cost;  /* used only during reordering */
} reorder_info;

/* --- info on positive and negative conditions only --- */
typedef struct three_field_tests_struct {
  test id_test;
  test attr_test;
  test value_test;
} three_field_tests;

/* --- info on negated conjunctive conditions only --- */
typedef struct ncc_info_struct {
  struct condition_struct *top;
  struct condition_struct *bottom;
} ncc_info;

/* --- finally, the structure of a condition --- */
typedef struct condition_struct {
  byte type;
  Bool already_in_tc;                 /* used only by cond_is_in_tc stuff */
  Bool test_for_acceptable_preference;   /* for pos, neg cond's only */
  struct condition_struct *next, *prev;
  union condition_main_data_union {
    three_field_tests tests;             /* for pos, neg cond's only */
    ncc_info ncc;                        /* for ncc's only */
  } data;
  bt_info bt;  /* for top-level positive cond's: used for BT and by the rete */
  reorder_info reorder;  /* used only during reordering */
} condition;

#ifdef __cplusplus
}
#endif

#endif
