/*
 * symbols_predefined.h
 *
 *  Created on: Sep 27, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SOAR_REPRESENTATION_SYMBOLS_PREDEFINED_H_
#define CORE_SOARKERNEL_SRC_SOAR_REPRESENTATION_SYMBOLS_PREDEFINED_H_

typedef struct predefined_sym_struct {
        /* ---------------- Predefined Symbols -------------------------
           Certain symbols are used so frequently that we create them at
           system startup time and never deallocate them.
           ------------------------------------------------------------- */

        Symbol*             attribute_symbol;
        Symbol*             choices_symbol;
        Symbol*             conflict_symbol;
        Symbol*             constraint_failure_symbol;
        Symbol*             goal_symbol;
        Symbol*             impasse_symbol;
        Symbol*             io_symbol;
        Symbol*             item_symbol;
        Symbol*             non_numeric_symbol;
        Symbol*             multiple_symbol;
        Symbol*             name_symbol;
        Symbol*             nil_symbol;
        Symbol*             no_change_symbol;
        Symbol*             none_symbol;
        Symbol*             o_context_variable;
        Symbol*             object_symbol;
        Symbol*             operator_symbol;
        Symbol*             problem_space_symbol;
        Symbol*             quiescence_symbol;
        Symbol*             s_context_variable;
        Symbol*             so_context_variable;
        Symbol*             ss_context_variable;
        Symbol*             sso_context_variable;
        Symbol*             sss_context_variable;
        Symbol*             state_symbol;
        Symbol*             superstate_symbol;
        Symbol*             t_symbol;
        Symbol*             tie_symbol;
        Symbol*             to_context_variable;
        Symbol*             ts_context_variable;
        Symbol*             type_symbol;

        Symbol*             item_count_symbol; // SBW 5/07
        Symbol*             non_numeric_count_symbol; // NLD 11/11

        Symbol*             fake_instantiation_symbol;
        Symbol*             architecture_inst_symbol;
        Symbol*             sti_symbol;

        /* RPM 9/06 begin */
        Symbol*             input_link_symbol;
        Symbol*             output_link_symbol;
        /* RPM 9/06 end */

        Symbol*             rl_sym_reward_link;
        Symbol*             rl_sym_reward;
        Symbol*             rl_sym_value;

        Symbol*             epmem_sym;
        Symbol*             epmem_sym_cmd;
        Symbol*             epmem_sym_result;

        Symbol*             epmem_sym_retrieved;
        Symbol*             epmem_sym_status;
        Symbol*             epmem_sym_match_score;
        Symbol*             epmem_sym_cue_size;
        Symbol*             epmem_sym_normalized_match_score;
        Symbol*             epmem_sym_match_cardinality;
        Symbol*             epmem_sym_memory_id;
        Symbol*             epmem_sym_present_id;
        Symbol*             epmem_sym_no_memory;
        Symbol*             epmem_sym_graph_match;
        Symbol*             epmem_sym_graph_match_mapping;
        Symbol*             epmem_sym_graph_match_mapping_node;
        Symbol*             epmem_sym_graph_match_mapping_cue;
        Symbol*             epmem_sym_success;
        Symbol*             epmem_sym_failure;
        Symbol*             epmem_sym_bad_cmd;

        Symbol*             epmem_sym_retrieve;
        Symbol*             epmem_sym_next;
        Symbol*             epmem_sym_prev;
        Symbol*             epmem_sym_query;
        Symbol*             epmem_sym_negquery;
        Symbol*             epmem_sym_before;
        Symbol*             epmem_sym_after;
        Symbol*             epmem_sym_prohibit;
        Symbol*             yes;
        Symbol*             no;

        Symbol*             smem_sym;
        Symbol*             smem_sym_cmd;
        Symbol*             smem_sym_result;

        Symbol*             smem_sym_retrieved;
        Symbol*             smem_sym_depth_retrieved;
        Symbol*             smem_sym_status;
        Symbol*             smem_sym_success;
        Symbol*             smem_sym_failure;
        Symbol*             smem_sym_bad_cmd;

        Symbol*             smem_sym_retrieve;
        Symbol*             smem_sym_query;
        Symbol*             smem_sym_negquery;
        Symbol*             smem_sym_prohibit;
        Symbol*             smem_sym_store;
        Symbol*             smem_sym_math_query;
        Symbol*             smem_sym_depth;
        Symbol*             smem_sym_store_new;
        Symbol*             smem_sym_overwrite;;

        Symbol*             smem_sym_math_query_less;
        Symbol*             smem_sym_math_query_greater;
        Symbol*             smem_sym_math_query_less_or_equal;
        Symbol*             smem_sym_math_query_greater_or_equal;
        Symbol*             smem_sym_math_query_max;
        Symbol*             smem_sym_math_query_min;

} predefined_symbols;

#endif /* CORE_SOARKERNEL_SRC_SOAR_REPRESENTATION_SYMBOLS_PREDEFINED_H_ */
