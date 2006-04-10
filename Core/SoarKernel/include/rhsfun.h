/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* ====================================================================
                             rhsfun.h

   The system maintains a list of available RHS functions.  Functions
   can appear on the RHS of productions either as values (in make actions
   or as arguments to other function calls) or as stand-alone actions
   (e.g., "write" and "halt").  When a function is executed, its C code
   is called with one parameter--a (consed) list of the arguments (symbols).
   The C function should return either a symbol (if all goes well) or NIL
   (if an error occurred, or if the function is a stand-alone action).

   All available RHS functions should be setup at system startup time via
   calls to add_rhs_function().  It takes as arguments the name of the
   function (a symbol), a pointer to the corresponding C function, the
   number of arguments the function expects (-1 if the function can take
   any number of arguments), and flags indicating whether the function can
   be a RHS value or a stand-alone action.

   Lookup_rhs_function() takes a symbol and returns the corresponding
   rhs_function structure (or NIL if there is no such function).

   Init_built_in_rhs_functions() should be called at system startup time
   to setup all the built-in functions.
==================================================================== */


/* -------------------------------------------------------------------
                      Right-Hand-Side Values
   
   Values on the RHS of productions can be given by symbols
   (constants or variables), by Rete locations, by indices of variables
   not bound on the LHS, or by function calls.  We use the low-order two
   bits of a pointer to differentiate between these types of values.

   If the low-order bits are:   the rhs_value is:
                       00       a pointer to a symbol
                       01       a pointer to a list (for a function call)
                       10       a Rete location
                       11       the index of an RHS unbound variable

   For function calls, the list is a consed list whose first element is
   the rhs_function structure, and whose remaining elements are the
   arguments of the function call.  (Each argument is an rhs_value.)

   WARNING: part of rete.cpp relies on the the fact that two rhs_values
   representing the same symbol, reteloc, or unboundvar will be equal (==),
   while two representing the same funcall will not be equal (==).
------------------------------------------------------------------- */

#ifndef RHSFUN_H
#define RHSFUN_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char * rhs_value;
typedef unsigned char byte;
typedef unsigned short rete_node_level;
typedef struct cons_struct cons;
typedef cons list;
typedef union symbol_union Symbol;


#ifdef USE_MACROS

#define rhs_value_is_symbol(rv) ((((unsigned long)(rv)) & 3)==0)
#define rhs_value_is_funcall(rv) ((((unsigned long)(rv)) & 3)==1)
#define rhs_value_is_reteloc(rv) ((((unsigned long)(rv)) & 3)==2)
#define rhs_value_is_unboundvar(rv) ((((unsigned long)(rv)) & 3)==3)

/* Warning: symbol_to_rhs_value() doesn't symbol_add_ref.  The caller must
   do the reference count update */
#define symbol_to_rhs_value(sym) ((rhs_value) (sym))
#define funcall_list_to_rhs_value(fl) ((rhs_value) (((char *)(fl))+1))
#define reteloc_to_rhs_value(field_num,levels_up) \
  ((rhs_value) ( (levels_up)<<4) + ((field_num)<<2) + 2 )
#define unboundvar_to_rhs_value(n) ((rhs_value) (((n)<<2) + 3))

#define rhs_value_to_symbol(rv) ((Symbol *)(rv))
#define rhs_value_to_funcall_list(rv) ((list *) (((char *)(rv))-1))
#define rhs_value_to_reteloc_field_num(rv) ((((unsigned long)(rv))>>2) & 3)
#define rhs_value_to_reteloc_levels_up(rv) ((((unsigned long)(rv))>>4)& 0xFFFF)
#define rhs_value_to_unboundvar(rv) (((unsigned long)(rv))>>2)

#else

#ifdef _MSC_VER
#pragma warning (disable : 4311 4312)
#endif

inline unsigned long rhs_value_is_symbol(rhs_value rv) 
{ 
  return ((((unsigned long)(rv)) & 3)==0); 
}

inline unsigned long rhs_value_is_funcall(rhs_value rv) 
{
  return ((((unsigned long)(rv)) & 3)==1);
}

inline unsigned long rhs_value_is_reteloc(rhs_value rv) 
{ 
  return ((((unsigned long)(rv)) & 3)==2); 
}

inline unsigned long rhs_value_is_unboundvar(rhs_value rv) 
{ 
  return ((((unsigned long)(rv)) & 3)==3);
}

inline rhs_value symbol_to_rhs_value(Symbol * sym) 
{ 
  return ((rhs_value) (sym)); 
}

inline rhs_value funcall_list_to_rhs_value(list * fl) 
{
  return ((rhs_value) (((char *)(fl))+1));
}

inline rhs_value reteloc_to_rhs_value(byte field_num, rete_node_level levels_up) 
{
  return ((rhs_value) ( (levels_up)<<4) + ((field_num)<<2) + 2 );
}

inline rhs_value unboundvar_to_rhs_value(unsigned long n) 
{ 
  return ((rhs_value) (((n)<<2) + 3));
}

inline Symbol * rhs_value_to_symbol(rhs_value rv)
{
  return ((Symbol *)(rv));
}

inline list * rhs_value_to_funcall_list(rhs_value rv)
{
  return ((list *) (((char *)(rv))-1));
}

inline unsigned long rhs_value_to_reteloc_field_num(rhs_value rv)
{
  return ((((unsigned long)(rv))>>2) & 3);
}

inline unsigned long rhs_value_to_reteloc_levels_up(rhs_value rv)
{
  return ((((unsigned long)(rv))>>4)& 0xFFFF);
}

inline unsigned long rhs_value_to_unboundvar(rhs_value rv)
{
  return (((unsigned long)(rv))>>2);
}

#ifdef _MSC_VER
#pragma warning (default : 4311 4312)
#endif

#endif /* USE_MACROS */

/* -------------------------------------------------------------------
                             RHS Actions

   Fields in an action:
 
      next:  points to the next action in a singly-linked list of all
        actions in the RHS.

      type:  indicates the type of action:  usually this is MAKE_ACTION,
        but for function calls it is FUNCALL_ACTION.

      preference_type:  for make actions, this indicates the type of the
        preference being created:  ACCEPTABLE_PREFERENCE_TYPE, etc.

      support:  indicates the compile-time calculated o-support of the action.
        This is either UNKNOWN_SUPPORT, O_SUPPORT, or I_SUPPORT.
  
      already_in_tc:  (reserved for use by compile-time o-support calcs)

      id, attr:  for make actions, these give the symbols (or Rete locations)
        for the id and attribute fields of the preference.

      value:  for MAKE_ACTION's, this gives the value field of the preference
        (a symbol or function call).  For FUNCALL_ACTION's, this holds the
        function call itself.
  
      referent:  for MAKE_ACTION's of binary preferences, this gives the
        referent field of the preference.
------------------------------------------------------------------- */

#define MAKE_ACTION 0
#define FUNCALL_ACTION 1

#define UNKNOWN_SUPPORT 0
#define O_SUPPORT 1
#define I_SUPPORT 2

typedef char Bool;
typedef unsigned char byte;

typedef struct agent_struct agent;
typedef struct kernel_struct Kernel;

typedef struct action_struct {
  struct action_struct *next;
  byte type;
  byte preference_type;
  byte support;
  Bool already_in_tc;  /* used only by compile-time o-support calcs */
  rhs_value id;
  rhs_value attr;
  rhs_value value;   /* for FUNCALL_ACTION's, this holds the funcall */
  rhs_value referent;
} action;

typedef Symbol * ((*rhs_function_routine)(agent* thisAgent, list *args, void* user_data));

typedef struct rhs_function_struct {
  struct rhs_function_struct *next;
  Symbol *name;
  rhs_function_routine f;
  int num_args_expected;     /* -1 means it can take any number of args */
  Bool can_be_rhs_value;
  Bool can_be_stand_alone_action;
  void* user_data;           /* Pointer to anything the user may want to pass into the function */
} rhs_function;

extern void add_rhs_function (agent* thisAgent, 
                              Symbol *name,
                              rhs_function_routine f,
                              int num_args_expected,
                              Bool can_be_rhs_value,
                              Bool can_be_stand_alone_action,
                              void* user_data);
extern void remove_rhs_function (agent* thisAgent, Symbol *name);
extern rhs_function *lookup_rhs_function(agent* thisAgent, Symbol *name);
extern void init_built_in_rhs_functions(agent* thisAgent);
extern void remove_built_in_rhs_functions(agent* thisAgent);

#ifdef __cplusplus
}
#endif

#endif
