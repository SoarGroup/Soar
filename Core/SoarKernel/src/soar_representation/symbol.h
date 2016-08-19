/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* =======================================================================
                             symtab.h

   Soar uses five kinds of symbols:  symbolic constants, integer
   constants, floating-point constants, identifiers, and variables.
   We use five resizable hash tables, one for each kind of symbol.

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

#include "kernel.h"

#include "Export.h"
#include "symbol_factory.h"
#include "soar_TraceNames.h"

#include <sstream>

//#define DEBUG_TRACE_REFCOUNT_FOR "O3"

/* -- Forward declarations needed for symbol base struct -- */
struct floatSymbol;
struct idSymbol;
struct varSymbol;
struct intSymbol;
struct strSymbol;

/* -------------------------
 * Symbol base struct
 * -------------------------
 *
 * This contains the common data found in all five symbol types.
 *
 * fc, ic, sc, id, var:  These are cached pointers for each of the 5 type-specific
 * symbol structs.  It made refactoring the code much easier and makes the code
 * much cleaner than using helper functions to typecast all over the place.

   Symbol_to_string() converts a symbol to a string.  The "rereadable"
   parameter indicates whether a rereadable representation is desired.
   Normally symbols are printed rereadably, but for (write) and Text I/O,
   we don't want this.

 *
 * Note:  I've disabled the union when in debugging mode.  It makes it much easier
 * to see the type-specific variables in a debugger.  It can also help find some
 * bugs where some part of the kernel may be treating a symbol as the wrong type.
 *
 * WARNING: next_in_hash_table MUST be the first field.  This field is used
 *       by the resizable hash table routines.
 *
 * Explanations of all the fields are at the end of the file.
 *
 * -- */

typedef struct EXPORT symbol_struct
{
    struct symbol_struct* next_in_hash_table;
    uint64_t reference_count;
    byte symbol_type;
    byte decider_flag;
    struct wme_struct* decider_wme;
    uint64_t retesave_symindex;
    uint32_t hash_id;
    tc_number tc_num;

    epmem_hash_id epmem_hash;
    uint64_t epmem_valid;

    smem_hash_id smem_hash;
    uint64_t smem_valid;

#ifdef SOAR_RELEASE_VERSION
    union
    {
#endif

        floatSymbol* fc;
        idSymbol*    id;
        varSymbol*   var;
        intSymbol*   ic;
        strSymbol*   sc;

#ifdef SOAR_RELEASE_VERSION
    };
#endif

    bool        is_identifier();
    bool        is_variable();
    bool        is_constant();
    bool        is_lti();
    bool        is_sti();
    bool        is_variablizable();
    bool        is_constant_or_marked_variable(tc_number tc);
    bool        is_in_tc(tc_number tc);
    bool        is_string();
    bool        is_int();
    bool        is_float();
    bool        is_state();
    bool        is_top_state();
    tc_number   get_tc_num();
    void        set_tc_num(tc_number n);
    bool        get_id_name(std::string& n);
    void        mark_if_unmarked(agent* thisAgent, tc_number tc, cons** sym_list);
    const char* type_string();
    char*       to_string(bool rereadable = false, char* dest = NIL, size_t dest_size = 0);

    struct symbol_struct*   get_parent_state();
} Symbol;

struct floatSymbol : public Symbol
{
    double value;
};
struct intSymbol   : public Symbol
{
    int64_t value;
};
struct strSymbol   : public Symbol
{
    char* name;
    struct production_struct* production;
};
struct varSymbol   : public Symbol
{
    char* name;
    Symbol* current_binding_value;
    uint64_t gensym_number;
    ::cons* rete_binding_locations;
};

struct idSymbol    : public Symbol
{
    uint64_t name_number;
    char name_letter;

    bool isa_goal;
    bool isa_impasse;

    bool did_PE;

    unsigned short isa_operator;

    bool allow_bottom_up_chunks;

    /* --- ownership, promotion, demotion, & garbage collection stuff --- */
    bool could_be_a_link_from_below;
    goal_stack_level level;
    goal_stack_level promotion_level;
    uint64_t link_count;
    dl_cons* unknown_level;

    struct slot_struct* slots;  /* dll of slots for this identifier */

    /* --- fields used only on goals and impasse identifiers --- */
    struct wme_struct* impasse_wmes;

    /* --- fields used only on goals --- */
    Symbol* higher_goal, *lower_goal;
    struct slot_struct* operator_slot;
    struct preference_struct* preferences_from_goal;

    Symbol* reward_header;
    struct rl_data_struct* rl_info;

    Symbol* epmem_header;
    Symbol* epmem_cmd_header;
    Symbol* epmem_result_header;
    struct wme_struct* epmem_time_wme;
    struct epmem_data_struct* epmem_info;


    Symbol* smem_header;
    Symbol* smem_cmd_header;
    Symbol* smem_result_header;
    struct smem_data_struct* smem_info;


    struct gds_struct* gds;

    int saved_firing_type;
    struct ms_change_struct* ms_o_assertions;
    struct ms_change_struct* ms_i_assertions;
    struct ms_change_struct* ms_retractions;


    /* --- fields used for Soar I/O stuff --- */
    ::cons* associated_output_links;
    struct wme_struct* input_wmes;

    int depth;

    epmem_node_id epmem_id;
    uint64_t epmem_valid;

    smem_lti_id smem_lti;
    epmem_time_id smem_time_id;
    uint64_t smem_valid;

    /*Agent::RL_Trace*/ void* rl_trace;
};

inline bool Symbol::is_identifier()
{
    return (symbol_type == IDENTIFIER_SYMBOL_TYPE);
};
inline bool Symbol::is_variable()
{
    return (symbol_type == VARIABLE_SYMBOL_TYPE);
};
inline bool Symbol::is_constant()
{
    return ((symbol_type == STR_CONSTANT_SYMBOL_TYPE) ||
            (symbol_type == INT_CONSTANT_SYMBOL_TYPE) ||
            (symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE));
};
inline bool Symbol::is_sti()
{
    return ((symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
            (id->smem_lti == NIL));
};
inline bool Symbol::is_lti()
{
    return ((symbol_type == IDENTIFIER_SYMBOL_TYPE) &&
            (id->smem_lti != NIL));
};

inline bool Symbol::is_variablizable()
{

    return (!is_variable());

};

inline bool Symbol::is_constant_or_marked_variable(tc_number tc)
{
    return ((symbol_type != VARIABLE_SYMBOL_TYPE) || (tc_num == tc));
};

inline bool Symbol::is_in_tc(tc_number tc)
{
    if ((symbol_type == VARIABLE_SYMBOL_TYPE) || (symbol_type == IDENTIFIER_SYMBOL_TYPE))
    {
        return (tc_num == tc);
    }
    else
    {
        return false;
    }
};

inline bool Symbol::is_string()
{
    return (symbol_type == STR_CONSTANT_SYMBOL_TYPE);
}

inline bool Symbol::is_int()
{
    return (symbol_type == INT_CONSTANT_SYMBOL_TYPE);
}

inline bool Symbol::is_float()
{
    return (symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE);
}

inline bool Symbol::is_state()
{
    return (is_identifier() && id->isa_goal);
}

inline bool Symbol::is_top_state()
{
    return (is_state() && (id->higher_goal == NULL));
}
inline tc_number Symbol::get_tc_num()
{
    return tc_num;
}

inline void Symbol::set_tc_num(tc_number n)
{
    tc_num = n;
}
inline Symbol* Symbol::get_parent_state()
{
    return id->higher_goal;
}
inline bool Symbol::get_id_name(std::string& n)
{
    std::stringstream ss;
    if (!is_identifier())
    {
        return false;
    }
    ss << id->name_letter << id->name_number;
    n = ss.str();
    return true;
}

inline bool get_symbol_value(Symbol* sym, std::string& v)
{
    if (sym->is_string())
    {
        v = sym->to_string();
        return true;
    }
    return false;
}

inline bool get_symbol_value(Symbol* sym, long& v)
{
    if (sym->is_int())
    {
        v = static_cast<long>(sym->ic->value);
        return true;
    }
    return false;
}

inline bool get_symbol_value(Symbol* sym, double& v)
{
    if (sym->is_float())
    {
        v = sym->fc->value;
        return true;
    }
    if (sym->is_int())
    {
        v = static_cast<double>(sym->ic->value);
        return true;
    }
    return false;
}

/* -- mark_if_unmarked set the tc number to the new tc and add the symbol to the list passed in -- */

template <typename P, typename T> void push(agent* thisAgent, P item, T*& list_header);
inline void Symbol::mark_if_unmarked(agent* thisAgent, tc_number tc, cons** sym_list)
{
    if (tc_num != tc)
    {
        tc_num = tc;
        if (sym_list)
        {
            push(thisAgent, this, (*(sym_list)));
        }
    }
}

inline char const* symbol_type_to_string(byte pType)
{
    switch (pType)
    {
        case VARIABLE_SYMBOL_TYPE:
            return soar_TraceNames::kTypeVariable ;
        case IDENTIFIER_SYMBOL_TYPE:
            return soar_TraceNames::kTypeID ;
        case INT_CONSTANT_SYMBOL_TYPE:
            return soar_TraceNames::kTypeInt ;
        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return soar_TraceNames::kTypeDouble ;
        case STR_CONSTANT_SYMBOL_TYPE:
            return soar_TraceNames::kTypeString ;
        default:
            return "UNDEFINED!";
    }
}

inline char const* Symbol::type_string()
{
    return symbol_type_to_string(symbol_type);
}
/* -- Inline functions to make casting to specific symbol types more readable. Also
 *    useful when sym might be null, in which case you couldn't use cached pointers to cast. -- */

inline idSymbol*     idSym(Symbol* sym)
{
    return static_cast<idSymbol*>(sym);
};
inline varSymbol*    varSym(Symbol* sym)
{
    return static_cast<varSymbol*>(sym);
};
inline floatSymbol*  floatSym(Symbol* sym)
{
    return static_cast<floatSymbol*>(sym);
};
inline intSymbol*    intSym(Symbol* sym)
{
    return static_cast<intSymbol*>(sym);
};
inline strSymbol*    strSym(Symbol* sym)
{
    return static_cast<strSymbol*>(sym);
};

/* -- Functions related to symbols.  Descriptions in symtab.cpp -- */

#ifndef SOAR_RELEASE_VERSION
    void debug_store_refcount(Symbol* sym, bool isAdd);
#endif

char first_letter_from_symbol(Symbol* sym);

/* -- This function returns a numeric value from a symbol -- */
double get_number_from_symbol(Symbol* sym);

/* -- Reference count functions for symbols
 *      All symbol creation/copying use these now, so we can avoid accidental leaks more easily.
 *      If DEBUG_TRACE_REFCOUNT_INVENTORY is defined, an alternate version of the function is used
 *      that sends a bunch of trace information to the debug database for deeper analysis of possible
 *      bugs. -- */

#ifdef DEBUG_TRACE_REFCOUNT_FOR
#include <string>
#include <iostream>
#include "enums.h"
std::string get_stacktrace(const char* prefix);
extern bool is_DT_mode_enabled(TraceMode mode);
#endif

#ifdef DEBUG_TRACE_REFCOUNT_INVENTORY
#define symbol_add_ref(thisAgent, sym) \
    ({debug_store_refcount(sym, true); \
        symbol_add_ref_func(thisAgent, sym); })

inline void symbol_add_ref_func(agent* thisAgent, Symbol* x)
#else
inline void symbol_add_ref(agent* thisAgent, Symbol* x)
#endif
{
#ifdef DEBUG_TRACE_REFCOUNT_INVENTORY
    //dprint(DT_REFCOUNT_ADDS, "ADD-REF %t -> %u\n", x, (x)->reference_count + 1);
#else
    //dprint(DT_REFCOUNT_ADDS, "ADD-REF %t -> %u\n", x, (x)->reference_count + 1);
#endif

#ifdef DEBUG_TRACE_REFCOUNT_FOR
    std::string strName(x->to_string());
    if (strName == DEBUG_TRACE_REFCOUNT_FOR)
    {
        std::string caller_string = get_stacktrace("add_ref");
//        dprint(DT_ID_LEAKING, "-- | %s(%u) | %s++\n", strName.c_str(), x->reference_count, caller_string.c_str());
        if (is_DT_mode_enabled(DT_ID_LEAKING))
        {
            std::cout << "++ | " << strName.c_str() << " |" << x->reference_count << " | " << caller_string.c_str() << "\n";
        }
    }
#endif

    (x)->reference_count++;
}
#ifdef DEBUG_TRACE_REFCOUNT_INVENTORY
#define symbol_remove_ref(thisAgent, &sym) \
    ({debug_store_refcount(sym, false); \
        symbol_remove_ref_func(thisAgent, sym);})

inline void symbol_remove_ref_func(agent* thisAgent, Symbol*& x)
#else
inline void symbol_remove_ref(agent* thisAgent, Symbol** x)
#endif
{
#ifdef DEBUG_TRACE_REFCOUNT_INVENTORY
    //dprint(DT_REFCOUNT_REMS, "REMOVE-REF %y -> %u\n", x, (x)->reference_count - 1);
#else
//    dprint(DT_REFCOUNT_REMS, "REMOVE-REF %y -> %u\n", x, (x)->reference_count - 1);
//    std::cout << "REMOVE-REF " << x->to_string() << "->" <<  ((x)->reference_count - 1) << "\n";
#endif
//    assert((x)->reference_count > 0);
    (*x)->reference_count--;

#ifdef DEBUG_TRACE_REFCOUNT_FOR
    std::string strName((*x)->to_string());
    if (strName == DEBUG_TRACE_REFCOUNT_FOR)
    {
        std::string caller_string = get_stacktrace("remove_ref");
//        dprint(DT_ID_LEAKING, "-- | %s(%u) | %s--\n", strName.c_str(), (*x)->reference_count, caller_string.c_str());
        if (is_DT_mode_enabled(DT_ID_LEAKING))
        {
            std::cout << "-- | " << strName.c_str() << " | " << (*x)->reference_count << " | " << caller_string.c_str() << "\n";
        }
    }
#endif

    if ((*x)->reference_count == 0)
    {
        deallocate_symbol(thisAgent, (*x));
        (*x) = NULL;
    }
}

/* -----------------------------------------------------------------
 *
 *                  Symbol Struct Variables
 *
 * =====================
 * Common to all 5 types
 * =====================
 * symbol_type                 Indicates which of the five kinds of symbols
 * reference_count             Current reference count for this symbol
 * next_in_hash_table          Next item in hash bucket
 * hash_id                     Used for hashing in the rete (and elsewhere)
 * retesave_symindex           Used for rete fastsave/fastload
 * tc_num                      Used for transitive closure/marking
 * =====================
 * Floating-point constants
 * =====================
 * value      The floating point value of the symbol.
 * =====================
 * Integer constants
 * =====================
 * value      The integer value value of the symbol. (int64_t).
 * =====================
 * String constants
 * =====================
 * name                        A null-terminated string giving its name
 * production                  When string is used for a production name, this
 *                             points to the production structure.  NIL otherwise
 * =====================
 * Variables
 * =====================
 * name                        Points to null-terminated string giving its name
 * tc_num                      Used for transitive closure computations
 * current_binding_value       When productions are fired, this indicates
 *                             the variable's binding
 * gensym_number               Used by the variable generator to prevent certain
 *                             already-in-use variables from being generated
 * rete_binding_locations      Used temporarily by the Rete, while adding
 *                             productions, to store a list of places where this
 *                             variable is bound and/or tested
 * =====================
 * Identifiers
 * =====================
 * isa_goal, isa_impasse       Whether this is the identifier of a goal or attribute
 *                             impasse
 * isa_operator                How many (normal or acceptable preference) wmes contain
 *                             (^operator <this-id>). The tracing code uses this to
 *                             figure out whether a given object is an operator.
 * name_number, name_letter    Name and letter of the identifier
 * slots                       DLL of all slots this symbol is used in
 * tc_num                      Unique numbers put in here to mark ID for things like
 *                             transitive closures
 * variablization              When variablizing chunks, this points to the variable to
 *                             which this bound id gets replaced with
 * ---------------
 * Link/Level Info
 * ---------------
 * could_be_a_link_from_below  true if there might be a link to this id from some other
 *                             id lower in the goal stack.
 * level                       Current goal_stack_level of this id
 * link_count                  How many links there are to this id.
 * promotion_level             Level to which this id is going to be promoted as soon as
 *                             ownership info is updated.
 * unknown_level               If the goal_stack_level of this id is known, this is NIL.
 *                             If the level isn't known, it points to a dl_cons in a
 *                             dl_list used by the demotion routines.
 * --------------
 * For goals only
 * --------------
 * allow_bottom_up_chunks      For goals, true iff no chunk has yet been built for a
 *                             subgoal of this goal
 * gds                         For goals, its goal's dependency set
 * higher_goal                 For goals, superstate of this goal in the context stack
 * impasse_wmes                For goals and impasses, DLL of architecturally created
 *                             wmes (e.g., (G37 ^object G36))
 * lower_goal                  For goals, substate of this goal in the context stack
 * operator_slot               For goals, the operator slot of this goal
 * preferences_from_goal       For goals, DLL of all preferences supported by this goal.
 *                             This is needed so we can remove o-supported preferences
 *                             when the goal goes away.
 * --------------------------
 * Find out if only for goals
 * --------------------------
 * did_PE
 * saved_firing_type           The firing type that must be restored if Waterfall
 *                             processing returns to this level. See consistency.cpp.
 * ms_o_assertions             DLL of o-assertions at this level
 * ms_i_assertions             DLL of i-assertions at this level
 * ms_retractions              DLL of all retractions at this level
 * associated_output_links     Used by the output module
 * input_wmes                  DLL of wmes added by input functions --*/



#endif
