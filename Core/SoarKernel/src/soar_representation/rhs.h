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

#ifndef RHS_H
#define RHS_H

#include "kernel.h"

#include "mem.h"

typedef char* rhs_value;
typedef struct rhs_struct
{
    Symbol*             referent;
    uint64_t            inst_identity;          /* instantiation identity ID */
    uint64_t            cv_id;
    Identity*           identity;
    uint64_t            identity_id_unjoined;   /* cached value only used for the explainer */
    bool                was_unbound_var;        /* used by re-orderer */
} rhs_info;
typedef rhs_info* rhs_symbol;

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

typedef struct action_struct
{
    ActionType              type;
    PreferenceType          preference_type;
    rhs_value               id;
    rhs_value               attr;
    rhs_value               value;   /* for FUNCALL_ACTION's, this holds the funcall */
    rhs_value               referent;
    SupportType             support;
    struct action_struct*   next;
} action;

/* -- Used by cli_productionfind.cpp and related functions -- */
typedef struct binding_structure
{
    Symbol* from;
    Symbol* to;
} Binding;

/* -- RHS Methods-- */
action*     make_action(agent* thisAgent);
action*     copy_action(agent* thisAgent, action* pAction);
rhs_value   allocate_rhs_value_for_symbol_no_refcount(agent* thisAgent, Symbol* sym, uint64_t pInstIdentity, uint64_t pChunkIdentity, Identity* pIdentity = NULL_IDENTITY_SET, bool pWasUnbound = false);
rhs_value   allocate_rhs_value_for_symbol(agent* thisAgent, Symbol* sym, uint64_t pInstIdentity, uint64_t pChunkIdentity, Identity* pIdentity = NULL_IDENTITY_SET, bool pWasUnbound = false);
rhs_value   create_RHS_value(agent* thisAgent, rhs_value rv, condition* cond, char first_letter, ExplainTraceType ebcTraceType = WM_Trace);
action*     create_RHS_action_list(agent* thisAgent, action* actions, condition* cond, ExplainTraceType ebcTraceType = WM_Trace);
void        deallocate_rhs_value(agent* thisAgent, rhs_value rv);
void        deallocate_action_list(agent* thisAgent, action* actions);
rhs_value   copy_rhs_value(agent* thisAgent, rhs_value rv, bool get_identity_set = false, bool get_cloned_identity = false);
void        add_all_variables_in_action(agent* thisAgent, action* a, tc_number tc, cons** var_list);
void        add_all_variables_in_action_list(agent* thisAgent, action* actions, tc_number tc, cons** var_list);
char        first_letter_from_rhs_value(rhs_value rv);
bool        rhs_value_is_literalizing_function(rhs_value rv);

/* -- Functions to check the 4 types that a rhs value can take -- */
inline bool rhs_value_is_symbol(rhs_value rv) { return (reinterpret_cast<uintptr_t>(rv) & 3) == 0;}
inline bool rhs_value_is_funcall(rhs_value rv) { return (reinterpret_cast<uintptr_t>(rv) & 3) == 1; }
inline bool rhs_value_is_reteloc(rhs_value rv) { return (reinterpret_cast<uintptr_t>(rv) & 3) == 2; }
inline bool rhs_value_is_unboundvar(rhs_value rv) { return (reinterpret_cast<uintptr_t>(rv) & 3) == 3; }

/* -- Conversion functions -- */
inline rhs_symbol rhs_value_to_rhs_symbol(rhs_value rv) { return reinterpret_cast<rhs_symbol>(rv); }
inline uint64_t   rhs_value_to_unboundvar(rhs_value rv) { return static_cast<uint64_t>((reinterpret_cast<uintptr_t>(rv) >> 2)); }
inline cons*      rhs_value_to_funcall_list(rhs_value rv) { return reinterpret_cast< cons* >(reinterpret_cast<char*>(rv) - 1); }
inline uint8_t    rhs_value_to_reteloc_field_num(rhs_value rv) { return static_cast<uint8_t>((reinterpret_cast<uintptr_t>(rv) >> 2) & 3); }
inline uint16_t   rhs_value_to_reteloc_levels_up(rhs_value rv) { return static_cast<uint16_t>((reinterpret_cast<uintptr_t>(rv) >> 4) & 0xFFFF); }
inline Symbol*    rhs_value_to_symbol(rhs_value rv) { return reinterpret_cast<rhs_symbol>(rv)->referent; }
inline uint64_t   rhs_value_to_inst_identity(rhs_value rv) { return reinterpret_cast<rhs_symbol>(rv)->inst_identity; }
inline bool       rhs_value_to_was_unbound_var(rhs_value rv) { return reinterpret_cast<rhs_symbol>(rv)->was_unbound_var; }
inline rhs_value  rhs_symbol_to_rhs_value(rhs_symbol rs) { return reinterpret_cast<rhs_value>(rs); }
inline rhs_value  unboundvar_to_rhs_value(uint64_t n) { return reinterpret_cast<rhs_value>((n << 2) + 3); }
inline rhs_value  funcall_list_to_rhs_value(cons* fl) { return reinterpret_cast<rhs_value>(reinterpret_cast<char*>(fl) + 1); }
inline rhs_value  reteloc_to_rhs_value(byte field_num, rete_node_level levels_up) { return reinterpret_cast<rhs_value>(levels_up << 4) + (field_num << 2) + 2; }

/* -- Comparison functions -- */
inline bool rhs_values_symbols_equal(rhs_value rv1, rhs_value rv2) { return (reinterpret_cast<rhs_symbol>(rv1)->referent == reinterpret_cast<rhs_symbol>(rv2)->referent); }

inline bool rhs_values_equal(rhs_value rv1, rhs_value rv2)
{
    if (rhs_value_is_symbol(rv1) && rhs_value_is_symbol(rv2))
        return (reinterpret_cast<rhs_symbol>(rv1)->referent == reinterpret_cast<rhs_symbol>(rv2)->referent);
    else if (rhs_value_is_funcall(rv1) && rhs_value_is_funcall(rv2))
    {
        cons* fl1 = rhs_value_to_funcall_list(rv1);
        cons* fl2 = rhs_value_to_funcall_list(rv2);
        if (fl1->first != fl2->first) return false;
        cons* c, *c2;

        for (c = fl1->rest, c2 = fl2->rest; c != NIL && c2 != NIL; c = c->rest, c2 = c2->rest)
            if (!rhs_values_equal(static_cast<char*>(c->first), static_cast<char*>(c2->first)))
                return false;
        return true;
    }
    else
        return (rv1 == rv2);
}

inline bool rhs_value_is_null(rhs_value rv)
{
    if (rv == NULL) return true;
    if (rhs_value_is_reteloc(rv)) return false;
    if (rhs_value_is_unboundvar(rv)) return false;
    if (rhs_value_is_funcall(rv)) { return (rhs_value_to_funcall_list(rv) == 0); };
    if (rhs_value_to_rhs_symbol(rv) == 0) return true;
    return false;
}

#endif /* RHS_H_ */
