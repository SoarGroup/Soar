/*
 * chunk_record.h
 *
 *  Created on: Apr 22, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_CHUNK_RECORD_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_CHUNK_RECORD_H_

#include "kernel.h"
#include "identity_record.h"
#include "stl_typedefs.h"

class instantiation_record;
class production_record;

typedef struct chunk_stats_struct {
        uint64_t            instantations_backtraced;
        uint64_t            seen_instantations_backtraced;
        uint64_t            duplicates;
        uint64_t            max_dupes;
        bool                tested_local_negation;
        bool                tested_quiescence;
        bool                tested_ltm_recall;

        uint64_t            num_grounding_conditions_added;
        uint64_t            merged_conditions;
        uint64_t            merged_disjunctions;
        uint64_t            merged_disjunction_values;
        uint64_t            eliminated_disjunction_values;
        uint64_t            constraints_attached;
        uint64_t            constraints_collected;
        uint64_t            rhs_arguments_literalized;
        bool                tested_deep_copy;

        uint64_t            identities_created;
        uint64_t            identities_joined;
        uint64_t            identities_joined_variable;
        uint64_t            identities_joined_local_singleton;
        uint64_t            identities_joined_singleton;
        uint64_t            identities_joined_user_singleton;
        uint64_t            identities_joined_child_results;
        uint64_t            identities_literalized_literal;
        uint64_t            identities_literalized_rhs_func_arg;
        uint64_t            identities_literalized_rhs_func_compare;

        uint64_t            identities_participated;
        uint64_t            identity_propagations;
        uint64_t            identity_propagations_blocked;
        uint64_t            operational_constraints;
        uint64_t            OSK_instantiations;
        uint64_t            child_result_instantiations;

        bool                reverted;
        bool                lhs_unconnected;
        bool                rhs_unconnected;
        bool                repair_failed;
        bool                did_not_match_wm;
} chunk_stats;

class chunk_record
{
        friend class Explanation_Memory;
        friend class instantiation_record;

    public:
        chunk_record() {};
        ~chunk_record() {};

        void init(agent* myAgent, uint64_t pChunkID);
        void clean_up();

        void                    record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_join_map* pIdentitySetMappings, instantiation* pBaseInstantiation, tc_number pBacktraceNumber, instantiation* pChunkInstantiation);
        void                    generate_dependency_paths();
        void                    end_chunk_record();

        void                    excise_chunk_record();

        void                    print_for_explanation_trace();
        void                    print_for_wme_trace();
        void                    visualize();

    private:
        agent*                  thisAgent;

        Symbol*                 name;
        ebc_rule_type           type;
        uint64_t                chunkID;
        instantiation*          chunkInstantiation;
        uint64_t                chunkInstantiationID;
        uint64_t                original_productionID;
        production_record*      excised_production;
        uint64_t                time_formed;
        goal_stack_level        match_level;


        condition_record_list*  conditions;
        action_record_list*     actions;

        instantiation_record*   baseInstantiation;
        inst_set*               result_instantiations;
        inst_record_set*        result_inst_records;

        inst_set*               backtraced_instantiations;
        inst_record_list*       backtraced_inst_records;

        identity_record         identity_analysis;
        chunk_stats             stats;
};



#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_CHUNK_RECORD_H_ */
