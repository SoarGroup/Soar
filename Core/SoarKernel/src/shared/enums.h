/*
 * enums.h
 *
 *  Created on: Jul 17, 2013
 *      Author: mazzin
 */

#ifndef ENUMS_H_
#define ENUMS_H_

#include "constants.h"

/* ------------------------- debug trace channels -----------------------------
 *
 * NOTE: IF YOU ADD A NEW TRACE OR DEBUG MODE, MAKE SURE TO INITIALIZE PREFIX
 *       INFO AND INITIAL VALUE IN initialize_debug_trace() in debug.cpp
 *
 * ------------------------------------------------------------------------- */
enum TraceMode
{
    // Generic
    No_Mode                     = 0,
    DT_DEBUG                    = 1,

    // General
    DT_MILESTONES               = 2,
    DT_PRINT_INSTANTIATIONS     = 3,

    // Explanation trace and identity creation
    DT_ADD_EXPLANATION_TRACE    = 4,
    DT_IDENTITY_GENERATION      = 5,

    // EBC
    DT_VARIABLIZATION_MANAGER   = 6,
    DT_EXTRA_RESULTS            = 7,
    DT_BACKTRACE                = 8,
    DT_UNIFY_IDENTITY_SETS      = 9,
    DT_UNIFY_SINGLETONS         = 10,
    DT_BUILD_CHUNK_CONDS        = 11,
    DT_LHS_VARIABLIZATION       = 12,
    DT_RHS_VARIABLIZATION       = 13,
    DT_NCC_VARIABLIZATION       = 14,
    DT_RL_VARIABLIZATION        = 15,
    DT_CONSTRAINTS              = 16,
    DT_MERGE                    = 17,
    DT_REORDERER                = 18,
    DT_REPAIR                   = 19,
    DT_REINSTANTIATE            = 20,
    DT_CLONES                   = 21,
    DT_EBC_CLEANUP              = 22,

    // Explainer
    DT_EXPLAIN                  = 23,
    DT_EXPLAIN_PATHS            = 24,
    DT_EXPLAIN_ADD_INST         = 25,
    DT_EXPLAIN_CONNECT          = 26,
    DT_EXPLAIN_UPDATE           = 27,
    DT_EXPLAIN_CONDS            = 28,
    DT_EXPLAIN_IDENTITIES       = 29,
    DT_EXPLAIN_CACHE            = 30,

    // Other Soar modules
    DT_EPMEM_CMD                = 31,
    DT_GDS                      = 32,
    DT_SMEM_INSTANCE            = 33,
    DT_PARSER                   = 34,
    DT_SOAR_INSTANCE            = 35,
    DT_WME_CHANGES              = 36,

    // Memory and refcounts
    DT_ALLOCATE_RHS_VALUE       = 37,
    DT_ID_LEAKING               = 38,
    DT_DEALLOCATE_INST          = 39,
    DT_DEALLOCATE_PREF          = 40,
    DT_DEALLOCATE_PROD          = 41,
    DT_DEALLOCATE_RHS_VALUE     = 42,
    DT_DEALLOCATE_SLOT          = 43,
    DT_DEALLOCATE_SYMBOL        = 44,
    DT_DEALLOCATE_TEST          = 45,
    DT_REFCOUNT_ADDS            = 46,
    DT_REFCOUNT_REMS            = 47,

    // Other low-level debugging
    DT_LINKS                    = 48,
    DT_UNKNOWN_LEVEL            = 49,
    DT_PREFS                    = 50,
    DT_RETE_PNODE_ADD           = 51,
    DT_WATERFALL                = 52,
    DT_GDS_HIGH                 = 53,
    DT_RHS_FUN_VARIABLIZATION   = 54,
    DT_DEEP_COPY                = 55,
    DT_RHS_LTI_LINKING          = 56,
    DT_VALIDATE                 = 57,
    DT_OSK                      = 58,
    DT_IDSET_REFCOUNTS          = 59,
    DT_PROPAGATE_ID_SETS        = 60,
    DT_DEALLOCATE_ID_SETS       = 61,
    num_trace_modes
};

enum ebc_rule_type {
    ebc_no_rule,
    ebc_chunk,
    ebc_justification,
    ebc_template
};

enum singleton_element_type {
    ebc_identifier,
    ebc_state,
    ebc_operator,
    ebc_constant,
    ebc_any,
    ebc_num_element_types
};

enum Decider_settings {
    DECIDER_KEEP_TOP_OPREFS,
    DECIDER_MAX_GP,
    DECIDER_MAX_DC_TIME,
    DECIDER_MAX_ELABORATIONS,
    DECIDER_MAX_GOAL_DEPTH,
    DECIDER_MAX_MEMORY_USAGE,
    DECIDER_MAX_NIL_OUTPUT_CYCLES,
    DECIDER_STOP_PHASE,
    DECIDER_WAIT_SNC,
    DECIDER_EXPLORATION_POLICY,
    DECIDER_AUTO_REDUCE,
    num_decider_settings
};

enum Output_sysparams {
    OM_ECHO_COMMANDS,
    OM_AGENT_WRITES,
    OM_WARNINGS,
    OM_PRINT_DEPTH,
    num_output_sysparams
};
enum EBCLearnChoices { ebc_always, ebc_never, ebc_only, ebc_except };

enum ChunkingSettings {
    SETTING_EBC_LEARNING_ON,
    SETTING_EBC_ALWAYS,
    SETTING_EBC_NEVER,
    SETTING_EBC_ONLY,
    SETTING_EBC_EXCEPT,
    SETTING_EBC_BOTTOM_ONLY,
    SETTING_EBC_INTERRUPT,
    SETTING_EBC_INTERRUPT_WARNING,
    SETTING_EBC_INTERRUPT_WATCHED,
    SETTING_EBC_UTILITY_MODE,
    SETTING_EBC_IDENTITY_VRBLZ,
    SETTING_EBC_CONSTRAINTS,
    SETTING_EBC_RHS_VRBLZ,
    SETTING_EBC_ADD_OSK,
    SETTING_EBC_REPAIR_LHS,
    SETTING_EBC_REPAIR_RHS,
    SETTING_EBC_MERGE,
    SETTING_EBC_USER_SINGLETONS,
    SETTING_EBC_UNIFY_ALL,
    SETTING_EBC_ALLOW_LOCAL_NEGATIONS,
    SETTING_EBC_ALLOW_OSK,
    SETTING_EBC_ALLOW_OPAQUE,
    SETTING_EBC_ALLOW_PROB,
    SETTING_EBC_ALLOW_CONFLATED,
    SETTING_EBC_ALLOW_LOCAL_PROMOTION,
    SETTING_EBC_REORDER_JUSTIFICATIONS,
    SETTING_EBC_ADD_LTM_LINKS,
    SETTING_EBC_DONT_ADD_INVALID_JUSTIFICATIONS,
    SETTING_EBC_TIMERS,
    num_ebc_settings
 };

enum IDSet_Mapping_Type {
    IDS_join,
    IDS_unified_with_local_singleton,
    IDS_unified_with_singleton,
    IDS_unified_child_result,
    IDS_literalized,
    IDS_literalized_RHS_function_arg,
};

enum IDSet_Deallocation_Type {
    IDS_pref_dealloc,
    IDS_test_dealloc,
    IDS_wme_dealloc,
    IDS_update_test,
    IDS_update_pref,
};

enum BTSourceType {
    BT_BaseInstantiation,
    BT_OSK,
    BT_ExtraResults,
    BT_Normal
};

enum visMemoryFormat {
    viz_node,
    viz_record
};

enum visRuleFormat {
    viz_name,
    viz_full
};

enum visObjectType {
    viz_inst_record,
    viz_chunk_record,
    viz_simple_inst,
    viz_id_and_augs,
    viz_wme,
    viz_wme_terminal
};

enum go_type_enum { GO_PHASE, GO_ELABORATION, GO_DECISION,
                    GO_STATE, GO_OPERATOR, GO_SLOT, GO_OUTPUT
                  };

enum top_level_phase { INPUT_PHASE = 0,
                       PROPOSE_PHASE,
                       DECISION_PHASE,
                       APPLY_PHASE,
                       OUTPUT_PHASE,
                       PREFERENCE_PHASE,
                       WM_PHASE,
                       NUM_PHASE_TYPES
                     };

enum SoarCannedMessageType {
    ebc_error_max_chunks,
    ebc_error_max_dupes,
    ebc_error_invalid_chunk,
    ebc_error_invalid_justification,
    ebc_error_no_conditions,
    ebc_progress_repairing,
    ebc_progress_repaired,
    ebc_progress_validating
};

enum EBCTraceType {
    ebc_actual_trace,
    ebc_match_trace,
    ebc_explanation_trace
};

enum EBCFailureType {
    ebc_success,
    ebc_failed_no_roots,
    ebc_failed_negative_relational_test_bindings,
    ebc_failed_reordering_rhs,
    ebc_failed_unconnected_conditions
};

enum EBCExplainStatus {
    explain_unrecorded,
    explain_recording,
    explain_recording_update,
    explain_recorded
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
MP_action_record,
MP_chunk_element,
MP_chunk_record,
MP_condition_record,
MP_identity_mapping,
MP_identity_sets,
MP_instantiation_record,
MP_production_record,
MP_repair_path,
MP_sym_triple,

num_memory_pools
};

enum chunkNameFormats
{
    numberedFormat,
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

enum ExplainTraceType
{
    WM_Trace,
    Explanation_Trace,
    WM_Trace_w_Inequalities
};

enum WME_Field
{
    ID_ELEMENT = 0,
    ATTR_ELEMENT = 1,
    VALUE_ELEMENT = 2,
    REFERENT_ELEMENT = 3,
    NO_ELEMENT = 4
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
    SMEM_LINK_TEST = 12,
    SMEM_LINK_NOT_TEST = 13,
    SMEM_LINK_UNARY_TEST = 14,
    SMEM_LINK_UNARY_NOT_TEST = 15,
    NUM_TEST_TYPES
};

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

enum SOAR_CALLBACK_TYPE             // if you change this, update soar_callback_names
{
    NO_CALLBACK,                      /* Used for missing callback */
    AFTER_INIT_AGENT_CALLBACK,
    BEFORE_INIT_SOAR_CALLBACK,
    AFTER_INIT_SOAR_CALLBACK,
    AFTER_HALT_SOAR_CALLBACK,
    BEFORE_ELABORATION_CALLBACK,
    AFTER_ELABORATION_CALLBACK,
    BEFORE_DECISION_CYCLE_CALLBACK,
    AFTER_DECISION_CYCLE_CALLBACK,
    BEFORE_INPUT_PHASE_CALLBACK,
    INPUT_PHASE_CALLBACK,
    AFTER_INPUT_PHASE_CALLBACK,
    BEFORE_PREFERENCE_PHASE_CALLBACK,
    AFTER_PREFERENCE_PHASE_CALLBACK,
    BEFORE_WM_PHASE_CALLBACK,
    AFTER_WM_PHASE_CALLBACK,
    BEFORE_OUTPUT_PHASE_CALLBACK,
    OUTPUT_PHASE_CALLBACK,
    AFTER_OUTPUT_PHASE_CALLBACK,
    BEFORE_DECISION_PHASE_CALLBACK,
    AFTER_DECISION_PHASE_CALLBACK,
    BEFORE_PROPOSE_PHASE_CALLBACK,
    AFTER_PROPOSE_PHASE_CALLBACK,
    BEFORE_APPLY_PHASE_CALLBACK,
    AFTER_APPLY_PHASE_CALLBACK,
    WM_CHANGES_CALLBACK,
    CREATE_NEW_CONTEXT_CALLBACK,
    POP_CONTEXT_STACK_CALLBACK,
    CREATE_NEW_ATTRIBUTE_IMPASSE_CALLBACK,
    REMOVE_ATTRIBUTE_IMPASSE_CALLBACK,
    PRODUCTION_JUST_ADDED_CALLBACK,
    PRODUCTION_JUST_ABOUT_TO_BE_EXCISED_CALLBACK,
    AFTER_INTERRUPT_CALLBACK,
    AFTER_HALTED_CALLBACK,
    BEFORE_RUN_STARTS_CALLBACK,
    AFTER_RUN_ENDS_CALLBACK,
    BEFORE_RUNNING_CALLBACK,
    AFTER_RUNNING_CALLBACK,
    FIRING_CALLBACK,
    RETRACTION_CALLBACK,
    SYSTEM_PARAMETER_CHANGED_CALLBACK,
    MAX_MEMORY_USAGE_CALLBACK,
    XML_GENERATION_CALLBACK,
    PRINT_CALLBACK,
    LOG_CALLBACK,
    INPUT_WME_GARBAGE_COLLECTED_CALLBACK,
    NUMBER_OF_CALLBACKS               /* Not actually a callback   */
    /* type.  Used to indicate   */
    /* list size and MUST ALWAYS */
    /* BE LAST.                  */
} ;

enum epmem_variable_key
{
    var_rit_offset_1, var_rit_leftroot_1, var_rit_rightroot_1, var_rit_minstep_1,
    var_rit_offset_2, var_rit_leftroot_2, var_rit_rightroot_2, var_rit_minstep_2,
    var_next_id
};

enum smem_query_levels { qry_search, qry_full };
enum smem_install_type { wm_install, fake_install };
enum smem_storage_type { store_level, store_recursive }; // ways to store an identifier
enum smem_cue_element_type { attr_t, value_const_t, value_lti_t, smem_cue_element_type_none };
enum smem_variable_key { var_max_cycle, var_num_nodes, var_num_edges, var_act_thresh, var_act_mode };

#endif /* ENUMS_H_ */
