/*
 * identity_record.h
 *
 *  Created on: Apr 22, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_IDENTITY_RECORD_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_IDENTITY_RECORD_H_

#include "kernel.h"
#include "stl_typedefs.h"

class chunk_record;
class instantiation_record;

typedef struct identity_set_struct {
        uint64_t    identity_set_ID;
        Symbol*     rule_variable;
} identity_set_info;

typedef struct identity_mapping_struct {
        uint64_t            from_identity;
        uint64_t            to_identity;
        Symbol*             from_symbol;
        Symbol*             to_symbol;
        IDSet_Mapping_Type  mappingType;
} identity_mapping;

class identity_record
{
        friend class Explanation_Memory;

    public:
        identity_record(agent* myAgent, chunk_record* pChunkRecord);
        ~identity_record();

        void    add_identity_mapping(uint64_t pI_ID, IDSet_Mapping_Type pType, uint64_t pFromID, uint64_t pToID, Symbol* pFromSym = NULL, Symbol* pToSym = NULL);
        void    set_original_ebc_mappings(id_to_id_map* pIdentitySetMappings) { original_ebc_mappings = new id_to_id_map(); (*original_ebc_mappings) = (*pIdentitySetMappings); }
        void    generate_identity_sets(uint64_t pInstID, condition* lhs);
        void    map_originals_to_sets();

        void    print_mappings();
        void    print_identity_mappings_for_instantiation(instantiation_record* pInstRecord);
        void    print_instantiation_mappings(uint64_t pI_ID);
        void    print_identities_in_chunk();

    private:

        agent*                  thisAgent;
        id_set*                 identities_in_chunk;
        id_to_id_map*           original_ebc_mappings;
        id_to_idset_map*        id_to_id_set_mappings;
        inst_identities_map*    instantiation_mappings;

        void    clear_mappings();

        void    print_mapping_list(identity_mapping_list* pMapList, bool printHeader = false);
        void    print_original_ebc_mappings();

};

#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_IDENTITY_RECORD_H_ */
