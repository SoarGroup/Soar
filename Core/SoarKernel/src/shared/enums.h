/*
 * enums.h
 *
 *  Created on: Jul 17, 2013
 *      Author: mazzin
 */

#ifndef ENUMS_H_
#define ENUMS_H_

typedef unsigned char byte;

/* ------------------------- debug trace channels -----------------------------
 *
 * NOTE: IF YOU ADD A NEW TRACE OR DEBUG MODE, MAKE SURE TO INITIALIZE PREFIX
 *       INFO AND INITIAL VALUE IN Output_Manager::initialize_debug_trace
 *
 * ------------------------------------------------------------------------- */
enum TraceMode
{
    No_Mode,
    DT_DEBUG,
    DT_ID_LEAKING,
    DT_LHS_VARIABLIZATION,
    DT_ADD_ADDITIONALS,
    DT_RHS_VARIABLIZATION,
    DT_VARIABLIZATION_MANAGER,
    DT_PRINT_INSTANTIATIONS,
    DT_DEALLOCATES,
    DT_DEALLOCATE_SYMBOLS,
    DT_REFCOUNT_ADDS,
    DT_REFCOUNT_REMS,
    DT_EPMEM_CMD,
    DT_PARSER,
    DT_MILESTONES,
    DT_REORDERER,
    DT_BACKTRACE,
    DT_GDS,
    DT_RL_VARIABLIZATION,
    DT_NCC_VARIABLIZATION,
    DT_IDENTITY_PROP,
    DT_SOAR_INSTANCE,
    DT_CLI_LIBRARIES,
    DT_CONSTRAINTS,
    DT_MERGE,
    DT_UNGROUNDED_STI,
    DT_UNIFICATION,
    DT_VM_MAPS,
    DT_BUILD_CHUNK_CONDS,
    DT_RHS_VALUE,
    DT_WME_CHANGES,
    DT_DEALLOCATES_TESTS,
    DT_LINKS,
    DT_EBC_CLEANUP,
    DT_UNKNOWN_LEVEL,
    DT_EXPLAIN,
    num_trace_modes
};

enum EBCTraceType {
    ebc_chunk,
    ebc_match,
    ebc_explanation,
    ebc_original
};

enum EBCExplainStatus {
    explain_unrecorded,
    explain_recording,
    explain_recorded,
    explain_connected
};
enum MemoryPoolType
{
MP_float_constant,
MP_identifier,
MP_int_constant,
MP_str_constant,
MP_variable,
MP_instantiation,
MP_chunk_cond,
MP_preference,
MP_wme,
MP_output_link,
MP_io_wme,
MP_slot,
MP_gds,
MP_action,
MP_test,
MP_condition,
MP_not,
MP_production,
MP_rhs_symbol,
MP_saved_test,
MP_cons_cell,
MP_dl_cons,
MP_rete_node,
MP_rete_test,
MP_right_mem,
MP_token,
MP_alpha_mem,
MP_ms_change,
MP_node_varnames,
MP_rl_info,
MP_rl_et,
MP_rl_rule,
MP_wma_decay_element,
MP_wma_decay_set,
MP_wma_wme_oset,
MP_wma_slot_refs,
MP_epmem_wmes,
MP_epmem_info,
MP_smem_wmes,
MP_smem_info,
MP_epmem_literal,
MP_epmem_pedge,
MP_epmem_uedge,
MP_epmem_interval,
MP_constraints,
MP_attachments,
num_memory_pools
};

enum chunkNameFormats
{
    numberedFormat,
    longFormat,
    ruleFormat
};

enum MessageType
{
    debug_msg,
    trace_msg,
    refcnt_msg
};

enum SymbolTypes
{
    VARIABLE_SYMBOL_TYPE = 0,
    IDENTIFIER_SYMBOL_TYPE = 1,
    STR_CONSTANT_SYMBOL_TYPE = 2,
    INT_CONSTANT_SYMBOL_TYPE = 3,
    FLOAT_CONSTANT_SYMBOL_TYPE = 4,
    UNDEFINED_SYMBOL_TYPE = 5
};

enum AddAdditionalTestsMode
{
    DONT_EXPLAIN,
    ALL_ORIGINALS,
    JUST_INEQUALITIES
};

enum WME_Field
{
    ID_ELEMENT = 0,
    ATTR_ELEMENT = 1,
    VALUE_ELEMENT = 2,
    NO_ELEMENT = 3
};

enum Print_Header_Type
{
    PrintBoth = 0,
    PrintAfter = 1,
    PrintBefore = 2
};

/* -- An implementation of an on/off boolean parameter --*/

enum boolean { off, on };

/* -- Possible modes for numeric indifference -- */

enum ni_mode
{
    NUMERIC_INDIFFERENT_MODE_AVG,
    NUMERIC_INDIFFERENT_MODE_SUM,
};

/* --- Types of tests (can't be 255 -- see rete.cpp) --- */

enum TestType
{
    UNINITIALIZED_TEST = 0,
    NOT_EQUAL_TEST = 1,          /* various relational tests */
    LESS_TEST = 2,
    GREATER_TEST = 3,
    LESS_OR_EQUAL_TEST = 4,
    GREATER_OR_EQUAL_TEST = 5,
    SAME_TYPE_TEST = 6,
    DISJUNCTION_TEST = 7,        /* item must be one of a list of constants */
    CONJUNCTIVE_TEST = 8,        /* item must pass each of a list of non-conjunctive tests */
    GOAL_ID_TEST = 9,            /* item must be a goal identifier */
    IMPASSE_ID_TEST = 10,        /* item must be an impasse identifier */
    EQUALITY_TEST = 11,
    NUM_TEST_TYPES
};

/* Null variablization identity set (used by EBC) */
#define NULL_IDENTITY_SET 0

/* -------------------------------
      Types of Productions
------------------------------- */

enum ProductionType
{
    USER_PRODUCTION_TYPE,
    DEFAULT_PRODUCTION_TYPE,
    CHUNK_PRODUCTION_TYPE,
    JUSTIFICATION_PRODUCTION_TYPE,
    TEMPLATE_PRODUCTION_TYPE,
    NUM_PRODUCTION_TYPES
};

// Soar-RL assumes that the production types start at 0 and go to (NUM_PRODUCTION_TYPES-1) sequentially

/* WARNING: preference types must be numbered 0..(NUM_PREFERENCE_TYPES-1),
   because the slot structure contains an array using these indices. Also
   make sure to update the strings in prefmem.h.  Finally, make sure the
   helper function defined below (for e.g. preference_is_unary) use the
   correct indices.

   NOTE: Reconsider, binary and unary parallel preferences are all
   deprecated.  Their types are not removed here because it would break
   backward compatibility of rete fast loading/saving.  It's possible that
   can be fixed in rete.cpp, but for now, we're just keeping the preference
   types.  There is no code that actually uses them any more, though.*/

enum PreferenceType
{
    ACCEPTABLE_PREFERENCE_TYPE = 0,
    REQUIRE_PREFERENCE_TYPE = 1,
    REJECT_PREFERENCE_TYPE = 2,
    PROHIBIT_PREFERENCE_TYPE = 3,
    RECONSIDER_PREFERENCE_TYPE = 4,
    UNARY_INDIFFERENT_PREFERENCE_TYPE = 5,
    UNARY_PARALLEL_PREFERENCE_TYPE = 6,
    BEST_PREFERENCE_TYPE = 7,
    WORST_PREFERENCE_TYPE = 8,
    BINARY_INDIFFERENT_PREFERENCE_TYPE = 9,
    BINARY_PARALLEL_PREFERENCE_TYPE = 10,
    BETTER_PREFERENCE_TYPE = 11,
    WORSE_PREFERENCE_TYPE = 12,
    NUMERIC_INDIFFERENT_PREFERENCE_TYPE = 13,
    NUM_PREFERENCE_TYPES = 14,
};

inline bool preference_is_unary(byte p)
{
    return (p < 9);
}
inline bool preference_is_binary(byte p)
{
    return (p > 8);
}

extern const char* preference_name[NUM_PREFERENCE_TYPES];


/* --- types of conditions --- */
enum ConditionType {
    POSITIVE_CONDITION,
    NEGATIVE_CONDITION,
    CONJUNCTIVE_NEGATION_CONDITION
};

enum ActionType {
    MAKE_ACTION = 0,
    FUNCALL_ACTION = 1,
};

enum SupportType {
    UNKNOWN_SUPPORT = 0,
    O_SUPPORT = 1,
    I_SUPPORT = 2,
    UNDECLARED_SUPPORT = 0,
    DECLARED_O_SUPPORT = 1,
    DECLARED_I_SUPPORT = 2,
};

#define PE_PRODS 0
#define IE_PRODS 1
#define NO_SAVED_PRODS -1

/* ------------------------------------------------------------------------

                             Impasse Types

------------------------------------------------------------------------ */

#define NONE_IMPASSE_TYPE 0                   /* no impasse */
#define CONSTRAINT_FAILURE_IMPASSE_TYPE 1
#define CONFLICT_IMPASSE_TYPE 2
#define TIE_IMPASSE_TYPE 3
#define NO_CHANGE_IMPASSE_TYPE 4

/* ---------------------------------------
    Match Set print parameters
--------------------------------------- */

#define MS_ASSERT_RETRACT 0      /* print both retractions and assertions */
#define MS_ASSERT         1      /* print just assertions */
#define MS_RETRACT        2      /* print just retractions */

typedef byte ms_trace_type;   /* must be one of the above constants */

/* ---------------------------------------
    How much information to print about
    the wmes matching an instantiation
--------------------------------------- */

#define NONE_WME_TRACE    1      /* don't print anything */
#define TIMETAG_WME_TRACE 2      /* print just timetag */
#define FULL_WME_TRACE    3      /* print whole wme */
#define NO_WME_TRACE_SET  4

typedef byte wme_trace_type;   /* must be one of the above constants */

/* -------------------------------
      Ways to Do User-Select
------------------------------- */

#define USER_SELECT_BOLTZMANN 1   /* boltzmann algorithm, with respect to temperature */
#define USER_SELECT_E_GREEDY  2   /* with probability epsilon choose random, otherwise greedy */
#define USER_SELECT_FIRST     3   /* just choose the first candidate item */
#define USER_SELECT_LAST      4   /* choose the last item   AGR 615 */
#define USER_SELECT_RANDOM    5   /* pick one at random */
#define USER_SELECT_SOFTMAX   6   /* pick one at random, probabalistically biased by numeric preferences */
#define USER_SELECT_INVALID   7   /* should be 1+ last item, used for validity checking */

#endif /* ENUMS_H_ */
