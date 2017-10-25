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
        uint64_t            duplicates;
        bool                tested_local_negation;
        bool                tested_quiescence;
        bool                tested_ltm_recall;
        bool                repaired;

        uint64_t            merged_conditions;
        uint64_t            merged_disjunctions;
        uint64_t            operational_constraints;
        uint64_t            constraints_attached;
        uint64_t            constraints_collected;

        uint64_t            identities_created;
        uint64_t            identities_participated;
        uint64_t            identities_joined;
        uint64_t            identities_literalized;

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

        void                    record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_join_map* pIdentitySetMappings, instantiation* pBaseInstantiation, tc_number pBacktraceNumber, instantiation* pChunkInstantiation, ProductionType prodType);
        void                    generate_dependency_paths();
        void                    end_chunk_record();
        void                    excise_chunk_record();

        void                    visualize();

    private:
        agent*                  thisAgent;

        Symbol*                 name;
        ebc_rule_type           type;
        uint64_t                chunkID;
        uint64_t                chunkInstantiationID;
        uint64_t                original_productionID;
        production_record*      excised_production;
        uint64_t                time_formed;
        goal_stack_level        match_level;

        instantiation_record*   chunkInstantiation;
        instantiation_record*   baseInstantiation;
        inst_set*               result_instantiations;
        inst_record_set*        result_inst_records;

        inst_set*               backtraced_instantiations;
        inst_record_list*       backtraced_inst_records;

        identity_record         identity_analysis;
        chunk_stats             stats;
};



#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_CHUNK_RECORD_H_ */
