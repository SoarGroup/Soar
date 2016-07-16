/*
 * chunk_record.h
 *
 *  Created on: Apr 22, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_CHUNK_RECORD_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_CHUNK_RECORD_H_

#include "kernel.h"
#include "stl_typedefs.h"

class instantiation_record;
class production_record;
class identity_record;

typedef struct chunk_stats_struct {
        uint64_t            duplicates;
        uint64_t            max_dupes;
        bool                tested_local_negation;
        bool                reverted;
        uint64_t            num_grounding_conditions_added;
        uint64_t            merged_conditions;
        uint64_t            instantations_backtraced;
        uint64_t            seen_instantations_backtraced;
        uint64_t            constraints_attached;
        uint64_t            constraints_collected;
} chunk_stats;

class chunk_record
{
        friend class Explanation_Memory;

    public:
        chunk_record(agent* myAgent, uint64_t pChunkID);
        ~chunk_record();

        void                    record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_id_map_type* pIdentitySetMappings, instantiation* pBaseInstantiation, tc_number pBacktraceNumber, instantiation* pChunkInstantiation);
        void                    generate_dependency_paths();
        void                    end_chunk_record();

        void                    excise_chunk_record();

        void					print_for_explanation_trace();
        void					print_for_wme_trace();
        void					visualize();

    private:
        agent*                  thisAgent;

        Symbol*                 name;
        uint64_t                chunkID;
        instantiation_record*   chunkInstantiation;
        production*             original_production;
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

        identity_record*        identity_analysis;
        chunk_stats             stats;
};



#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_CHUNK_RECORD_H_ */
