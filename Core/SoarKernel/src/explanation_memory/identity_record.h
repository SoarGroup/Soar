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

class identity_record
{
        friend class Explanation_Memory;

    public:
        identity_record() {};
        ~identity_record() {};

        void    init(agent* myAgent);
        void    clean_up();

        void    add_identity_mapping(uint64_t pI_ID, IDSet_Mapping_Type pType, IdentitySet* pFromID, IdentitySet* pToID);
        void    analyze_chunk_identities(uint64_t pInstID, condition* lhs);
        void    record_identity_sets(identity_set_set* identity_sets);

        void    print_mappings();
        void    print_identity_mappings_for_instantiation(instantiation_record* pInstRecord);
        void    print_instantiation_mappings(uint64_t pI_ID);
        void    print_identities_in_chunk();

        void    visualize();

    private:

        agent*                  thisAgent;
        id_set*                 identities_in_chunk;
        id_to_sym_map*          idset_to_var_map;
        inst_identities_map*    instantiation_mappings;
        id_to_id_map*           identity_joins;

        void    clear_mappings();

        void    print_mapping_list(identity_mapping_list* pMapList, bool pLiteralizationMode, std::string* pInstString = NULL);

};

#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_IDENTITY_RECORD_H_ */
