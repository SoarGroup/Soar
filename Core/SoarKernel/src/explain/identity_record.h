/*
 * identity_record.h
 *
 *  Created on: Apr 22, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLAIN_IDENTITY_RECORD_H_
#define CORE_SOARKERNEL_SRC_EXPLAIN_IDENTITY_RECORD_H_

#include "kernel.h"
#include "stl_typedefs.h"

class chunk_record;
class instantiation_record;

typedef struct identity_set_struct {
        uint64_t    identity_set_ID;
        Symbol*     rule_variable;
} identity_set_info;

class identity_record
{
        friend class Explanation_Memory;

    public:
        identity_record(agent* myAgent, chunk_record* pChunkRecord, id_to_id_map_type* pIdentitySetMappings);
        ~identity_record();

        void    generate_identity_sets(condition* lhs);
        void    print_identity_mappings_for_instantiation(instantiation_record* pInstRecord);
        void    print_identity_explanation(chunk_record* pChunkRecord);
    private:

        agent*                  thisAgent;
        id_set*                 identities_in_chunk;
        id_to_id_map_type*      original_ebc_mappings;
        id_to_idset_map_type*   id_to_id_set_mappings;

        void    print_identities_in_chunk();
        void    print_identity_mappings();
        void    print_original_ebc_mappings();
};

#endif /* CORE_SOARKERNEL_SRC_EXPLAIN_IDENTITY_RECORD_H_ */
