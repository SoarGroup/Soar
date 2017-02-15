/*
 * ebc_identity_sets.h
 *
 *  Created on: Feb 15, 2017
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SETS_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SETS_H_

#include "kernel.h"
#include "stl_typedefs.h"
#include "test.h"

class Identity_Sets
{
    public:

        Identity_Sets(agent* myAgent);
        ~Identity_Sets() {}

        void        reset();

        uint64_t    get_base_id_set(uint64_t base_identity);
        uint64_t    get_intermediate_id_set(uint64_t base_identity);
        bool        in_null_identity_set(test t)                        { return (literalized_id_sets.find(t->identity) != literalized_id_sets.end()); }

        void        add_operational_instantiation(instantiation* pInst);
        void        add_identity_set_unification(uint64_t identity_set_ID1, uint64_t identity_set_ID2);

    private:

        agent*                  thisAgent;

        uint64_t                id_set_counter;

        id_to_id_map            base_id_to_id_set_map;
        id_to_id_map            interm_id_to_id_set_map;
        id_to_id_map            id_set_unifications;
        id_set                  literalized_id_sets;

        inst_set                operational_instantiations;

        void    move_intermediate_id_set_to_base(uint64_t pIDSetID);

};



#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SETS_H_ */
