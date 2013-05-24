#include <portability.h>
/* -------------------------------------------------------------------
                              rhs.h
                      Right-Hand-Side Values

   Values on the RHS of productions can be given by symbols
   (constants or variables), by Rete locations, by indices of variables
   not bound on the LHS, or by function calls.  We use the low-order two
   bits of a pointer to differentiate between these types of values.

   If the low-order bits are:   the rhs_value is:
                       00       a pointer to a rhs symbol (struct with
                                symbol and original variable symbol)
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
#ifndef RHS_H_
#define RHS_H_

typedef struct rhs_struct {
  Symbol *referent;
  Symbol *original_variable;
} rhs_info;

typedef rhs_info * rhs_symbol;
typedef char * rhs_value;
typedef unsigned short rete_node_level;

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

/* -- Used by cli_productionfind.cpp and related functions -- */
typedef struct binding_structure {
    Symbol *from, *to;
} Binding;

inline rhs_value make_rhs_value_symbol_no_refcount(agent* thisAgent, Symbol * sym, Symbol * original_sym=NULL);
inline rhs_value make_rhs_value_symbol(agent* thisAgent, Symbol * sym, Symbol * original_sym=NULL);
rhs_value copy_rhs_value (agent* thisAgent, rhs_value rv);
void deallocate_rhs_value (agent* thisAgent, rhs_value rv);
void deallocate_action_list (agent* thisAgent, action *actions);

void add_all_variables_in_action (agent* thisAgent, action *a, tc_number tc, ::list **var_list);
void add_all_variables_in_action_list (agent* thisAgent, action *actions, tc_number tc, list **var_list);

char first_letter_from_rhs_value (rhs_value rv);

bool actions_are_equal_with_bindings (agent* agnt, action *a1, action *a2, list **bindings);

inline bool rhs_value_is_symbol(rhs_value rv) { return (reinterpret_cast<uintptr_t>(rv) & 3) == 0;}
inline bool rhs_value_is_funcall(rhs_value rv) { return (reinterpret_cast<uintptr_t>(rv) & 3) == 1;}
inline bool rhs_value_is_reteloc(rhs_value rv){ return (reinterpret_cast<uintptr_t>(rv) & 3) == 2;}
inline bool rhs_value_is_unboundvar(rhs_value rv){ return (reinterpret_cast<uintptr_t>(rv) & 3) == 3;}

inline rhs_symbol rhs_value_to_rhs_symbol(rhs_value rv){ return reinterpret_cast<rhs_symbol>(rv);}
inline Symbol * rhs_value_to_symbol(rhs_value rv){ return reinterpret_cast<rhs_symbol>(rv)->referent;}
inline Symbol * rhs_value_to_original_symbol(rhs_value rv){ return reinterpret_cast<rhs_symbol>(rv)->original_variable;}
inline ::list * rhs_value_to_funcall_list(rhs_value rv){ return reinterpret_cast< ::list * >(reinterpret_cast<char *>(rv) - 1);}
inline uint8_t rhs_value_to_reteloc_field_num(rhs_value rv){ return static_cast<uint8_t>((reinterpret_cast<uintptr_t>(rv) >> 2) & 3);}
inline uint16_t rhs_value_to_reteloc_levels_up(rhs_value rv){ return static_cast<uint16_t>((reinterpret_cast<uintptr_t>(rv) >> 4) & 0xFFFF);}
inline uint64_t rhs_value_to_unboundvar(rhs_value rv){ return static_cast<uint64_t>((reinterpret_cast<uintptr_t>(rv) >> 2));}

inline rhs_value rhs_symbol_to_rhs_value(rhs_symbol rs){ return reinterpret_cast<rhs_value>(rs);}
inline rhs_value unboundvar_to_rhs_value(uint64_t n){ return reinterpret_cast<rhs_value>((n << 2) + 3);}
inline rhs_value funcall_list_to_rhs_value(::list * fl){ return reinterpret_cast<rhs_value>(reinterpret_cast<char *>(fl) + 1);}
inline rhs_value reteloc_to_rhs_value(byte field_num, rete_node_level levels_up){ return reinterpret_cast<rhs_value>(levels_up << 4) + (field_num << 2) + 2;}

#endif /* RHS_H_ */
