/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                             symtab.h

   Soar 6 uses five kinds of symbols:  symbolic constants, integer
   constants, floating-point constants, identifiers, and variables.
   We use five resizable hash tables, one for each kind of symbol.

   "symbol" is typedef-ed as a union of the five kinds of symbol
   structures.  Some fields common to all symbols are accessed via
   sym->common.field_name; fields particular to a certain kind of
   symbol are accessed via sym->var.field_name_on_variables, etc.
   (See the definitions below.)  Note that "common" is #defined below.

   Some (but not all) of the fields common to all symbols are:
      symbol_type:  indicates which of the five kinds of symbols this is
      reference_count:  current reference count for this symbol
      hash_id:  used for hash functions in the rete (and elsewhere)
      
   Fields on symbolic constants:
      name:  points to null-terminated string giving its name
      production:  points to a production structure, or NIL if there is
                   no production with that name

   Fields on integer constants:
      value:  gives the value of the symbol.  This is of type (long).

   Fields on floating-point constants:
      value:  gives the value of the symbol.  This is of type (float).

   Fields on variables:
      name:  points to null-terminated string giving its name
      tc_num:  used for transitive closure computations
      current_binding_value:  when productions are fired, this indicates
                              the variable's binding
      gensym_number:  used by the variable generator to prevent certain
                      already-in-use variables from being generated
      rete_binding_locations:  used temporarily by the Rete, while adding
                      productions, to store a list of places where this
                      variable is bound and/or tested

   Fields on identifiers:

       name_number, name_letter:  indicate the name of the identifier

       isa_goal, isa_impasse:  indicate whether this is the identifier of a
                               goal or attribute impasse

       isa_operator:  keeps a count of how many (normal or acceptable
                      preference) wmes contain (^operator <this-id>).
                      The tracing code uses this to figure out whether
                      a given object is an operator.

       allow_bottom_up_chunks:  Used for bottom-up chunking, and only on goal
         identifiers.  This is TRUE iff no chunk has yet been built for a
         subgoal of this goal.
        
       could_be_a_link_from_below:  TRUE if there might be a link to this id
         from some other id lower in the goal stack.

       did_PE: 

       level:  current goal_stack_level of this id

       promotion_level:  level to which this id is going to be promoted as
         soon as ownership info is updated.

       link_count:  count of how many links there are to this id.

       unknown_level:  if the goal_stack_level of this id is known, this is
         NIL.  If the level isn't known, it points to a dl_cons in a dl_list
         used by the demotion routines.

       slots:  this is the header for a dll of the slots for this id.

       tc_num:  used for transitive closures, marking id's, etc.
 
       variablization:  used by the chunker when variablizing chunks--this
         points to the variable to which this id gets changed

       impasse_wmes:  for goal and impasse ids only:  this is the header
         of the dll of architecture-created wmes (e.g., (G37 ^object G36))

       higher_goal, lower_goal:  for goals, these point to adjacent goals
         in the context stack
       problem_space_slot, state_slot, operator_slot:  for goals, these
         point to the corresponding context slots
       preferences_from_goal:  for goals, this is the header of the dll
         of all preferences supported by this goal.  This is needed so
         we can remove o-supported preferences when the goal goes away.

       gds: pointer to a goal's dependency set
       saved_firing_type: the firing type that must be restored if
          Waterfall processing returns to this level. see consistency.c
       ms_o_assertions:  dll of o-assertions at this level
       ms_i_assertions:  dll of i-assertions at this level
       ms_retractions:   dll of all retractions at this level

       associated_output_links:  used by the output module

       input_wmes:  dll of wmes added by input functions

   Reference counting for symbols:  I can't remember all the places I add
     reference counts to symbols.  Here's a bunch I can remember though.
     If you're not sure whether to add/remove a reference for something,
     it's better to play it safe and do the add/remove.

     +1 for each occurrence in a rete test or alpha mem constant test
     +1 for each occurrence in a condition test anywhere
     +1 for each occurrence in a Not
     +1 for each occurrence in a saved_test
     +1 for each occurrence in a WME
     +1 for each occurrence in a preference
     +1 for each occurrence as {id or attr} of a slot
     +1 for goal/impasse identifiers
     +1 if it's the name of a production
     +1 if it's a predefined symbol (e.g., "goal" or "operator")
     +1 for each enqueued add-link or remove-link to/from it
     +1 for each occurrence in a global var. (e.g., chunk-free-problem-spaces)

  We deallocate a symbol when its reference count goes to 0.
======================================================================= */

#ifndef SYMTAB_H
#define SYMTAB_H

#ifdef __cplusplus
extern "C"
{
#endif

#define VARIABLE_SYMBOL_TYPE 0
#define IDENTIFIER_SYMBOL_TYPE 1
#define SYM_CONSTANT_SYMBOL_TYPE 2
#define INT_CONSTANT_SYMBOL_TYPE 3
#define FLOAT_CONSTANT_SYMBOL_TYPE 4

typedef char Bool;
typedef unsigned char byte;
typedef unsigned long tc_number;
typedef signed short goal_stack_level;
typedef struct cons_struct cons;
typedef struct agent_struct agent;
typedef struct dl_cons_struct dl_cons;
typedef cons list;

/* WARNING:  In the following structure, next_in_hash_table MUST be the
   first field.  This field is used by the resizable hash table routines. */

typedef struct symbol_common_data_struct {
  union symbol_union *next_in_hash_table;  /* next item in hash bucket */
  unsigned long reference_count;
  byte symbol_type;                /* one of the above xxx_SYMBOL_TYPE's */
  byte decider_flag;               /* used only by the decider */
  union a_union {
    struct wme_struct *decider_wme;  /* used only by the decider */
    unsigned long retesave_symindex; /* used for rete fastsave/fastload */
  } a;
  unsigned long hash_id;           /* used for hashing in the rete */
} symbol_common_data;

/* WARNING:  In the following structures (the five kinds of symbols),
   common_symbol_info MUST be the first field. */

typedef struct sym_constant_struct {
  symbol_common_data common_symbol_info;
  char *name;
  struct production_struct *production;  /* NIL if no prod. has this name */
} sym_constant;

typedef struct int_constant_struct {
  symbol_common_data common_symbol_info;
  long value;
} int_constant;

typedef struct float_constant_struct {
  symbol_common_data common_symbol_info;
  float value;
} float_constant;

typedef struct variable_struct {
  symbol_common_data common_symbol_info;
  char *name;
  tc_number tc_num;
  union symbol_union *current_binding_value;
  unsigned long gensym_number;
  list *rete_binding_locations;
} variable;

/* Note: I arranged the fields below to try to minimize space */
typedef struct identifier_struct {
  symbol_common_data common_symbol_info;
  unsigned long name_number;
  char name_letter;

  Bool isa_goal;        /* TRUE iff this is a goal identifier */
  Bool isa_impasse;     /* TRUE iff this is an attr. impasse identifier */

  Bool did_PE;   /* RCHONG: 10.11 */

  unsigned short isa_operator;

  Bool allow_bottom_up_chunks;

  /* --- ownership, promotion, demotion, & garbage collection stuff --- */
  Bool could_be_a_link_from_below;
  goal_stack_level level;
  goal_stack_level promotion_level;
  unsigned long link_count;
  dl_cons *unknown_level;

  struct slot_struct *slots;  /* dll of slots for this identifier */
  tc_number tc_num;           /* used for transitive closures, marking, etc. */
  union symbol_union *variablization;  /* used by the chunker */

  /* --- fields used only on goals and impasse identifiers --- */
  struct wme_struct *impasse_wmes;

  /* --- fields used only on goals --- */
  union symbol_union *higher_goal, *lower_goal;
  struct slot_struct *operator_slot;
  struct preference_struct *preferences_from_goal;

  /* REW: begin 09.15.96 */
  struct gds_struct *gds;    /* Pointer to a goal's dependency set */
  /* REW: begin 09.15.96 */

  /* REW: begin 08.20.97 */
  int saved_firing_type;     /* FIRING_TYPE that must be restored if Waterfall
				processing returns to this level.
				See consistency.cpp */
  struct ms_change_struct *ms_o_assertions; /* dll of o assertions at this level */
  struct ms_change_struct *ms_i_assertions; /* dll of i assertions at this level */
  struct ms_change_struct *ms_retractions;  /* dll of retractions at this level */
  /* REW: end   08.2097 */

  /* --- fields used for Soar I/O stuff --- */
  list *associated_output_links;
  struct wme_struct *input_wmes;

  int depth; /* used to track depth of print (bug 988) RPM 4/07 */
} identifier;

typedef union symbol_union {
  variable var;
  identifier id;
  sym_constant sc;
  int_constant ic;
  float_constant fc;
} Symbol;

/* WARNING: this #define's "common".  Don't use "common" anywhere in the
   code unless you intend this meaning of it.  This is so we can
   conveniently access fields used in all kinds of symbols, like this:
   "sym.common.reference_count" rather than "sym.var.common.reference_count"
   or "sym.id.common.reference_count", etc. */

#define common var.common_symbol_info

/* -----------------------------------------------------------------
                       Symbol Table Routines

   Initialization:

     Init_symbol_tables() should be called first, to initialize the
     module.

   Lookup and Creation:

     The find_xxx() routines look for an existing symbol and return it
     if found; if no such symbol exists, they return NIL.

     The make_xxx() routines look for an existing symbol; if the find one,
     they increment the reference count and return it.  If no such symbol
     exists, they create a new one, set the reference count to 1, and
     return it.

     Note that rather than a make_identifier() routine, we have a
     make_new_identifier() routine, which takes two arguments: the first
     letter for the new identifier, and its initial goal_stack_level.
     There is no way to force creation of an identifier with a particular
     name letter/number combination like J37.

   Reference Counting:

     Symbol_add_ref() and symbol_remove_ref() are macros for incrementing
     and decrementing the reference count on a symbol.  When the count
     goes to zero, symbol_remove_ref() calls deallocate_symbol().

   Other Utilities:

     Reset_id_counters() is called during an init-soar to reset the id
     gensym numbers to 1.  It first makes sure there are no existing
     identifiers in the system--otherwise we might generate a second
     identifier with the same name later on.

     Reset_id_and_variable_tc_numbers() resets the tc_num field of every
     existing id and variable to 0.
     
     Reset_variable_gensym_numbers() resets the gensym_number field of
     every existing variable to 0.
     
     Print_internal_symbols() just prints a list of all existing symbols.
     (This is useful for debugging memory leaks.)
     
     Generate_new_sym_constant() is used to gensym new symbols that are
     guaranteed to not already exist.  It takes two arguments: "prefix"
     (the desired prefix of the new symbol's name), and "counter" (a
     pointer to a counter (unsigned long) that is incremented to produce
     new gensym names).
----------------------------------------------------------------- */

extern void init_symbol_tables (agent* thisAgent);

extern Symbol *find_variable (agent* thisAgent, char *name);
extern Symbol *find_identifier (agent* thisAgent, char name_letter, unsigned long name_number);
extern Symbol *find_sym_constant (agent* thisAgent, const char *name);  /* AGR 600 */
extern Symbol *find_int_constant (agent* thisAgent, long value);
extern Symbol *find_float_constant (agent* thisAgent, float value);

extern Symbol *make_variable (agent* thisAgent, char *name);
extern Symbol *make_sym_constant (agent* thisAgent, char const *name);
extern Symbol *make_int_constant (agent* thisAgent, long value);
extern Symbol *make_float_constant (agent* thisAgent, float value);
extern Symbol *make_new_identifier (agent* thisAgent, char name_letter, goal_stack_level level);

extern void deallocate_symbol (agent* thisAgent, Symbol *sym);

extern bool reset_id_counters (agent* thisAgent);
extern void reset_id_and_variable_tc_numbers (agent* thisAgent);
extern void reset_variable_gensym_numbers (agent* thisAgent);
extern void print_internal_symbols (agent* thisAgent);
extern Symbol *generate_new_sym_constant (agent* thisAgent, char *prefix,unsigned long *counter);

#ifdef USE_MACROS

/* --- macros used for changing the reference count --- */
#define symbol_add_ref(x) {(x)->common.reference_count++;}
#define symbol_remove_ref(thisAgent, x) { \
  (x)->common.reference_count--; \
  if ((x)->common.reference_count == 0) \
  deallocate_symbol(thisAgent, x); \
  }
 
#else

inline unsigned long symbol_add_ref(Symbol * x) 
{
  (x)->common.reference_count++;
  unsigned long refCount = (x)->common.reference_count ;
  return refCount ;
}

inline unsigned long symbol_remove_ref(agent* thisAgent, Symbol * x)
{
  (x)->common.reference_count--;
  unsigned long refCount = (x)->common.reference_count ;
  if ((x)->common.reference_count == 0)
    deallocate_symbol(thisAgent, x);

  return refCount ;
}

#endif /* USE_MACROS */

/* -----------------------------------------------------------------
                       Predefined Symbols

   Certain symbols are used so frequently that we create them at
   system startup time and never deallocate them.  These symbols are
   global variables (per-agent) and are named xxx_symbol (see glob_vars.h).
   
   Create_predefined_symbols() should be called to do the creation.
   After that, the global variables can be accessed freely.  Note that
   the reference counts on these symbols should still be updated--
   symbol_add_ref() should be called, etc.--it's just that when the
   symbol isn't really being used, it stays around because the count
   is still 1.
----------------------------------------------------------------- */

extern void create_predefined_symbols (agent* thisAgent);
extern void release_predefined_symbols (agent* thisAgent);

#ifdef __cplusplus
}
#endif

#endif
