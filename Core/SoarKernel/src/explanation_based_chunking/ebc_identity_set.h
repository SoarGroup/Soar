/*
 * ebc_identity_set.h
 *
 *  Created on: Mar 4, 2017
 *      Author: mazzin
 */
#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SET_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SET_H_

#include "kernel.h"
#include "stl_typedefs.h"

class IdentitySet
{
    public:

        IdentitySet(agent* my_agent);
        ~IdentitySet();

        /* An ID used for printing and to make debugging easier. Not used by identity analysis logic. */
        uint64_t                    identity;

        /* Flag indicating whether any transient data has been changed */
        bool                        dirty;

        /* Fields that link this identity_set node to others in the identity graph */
        IdentitySetSharedPtr        super_join;
        identity_set_list*          identity_sets;

        /* Fields for variablization and chunk instantiation identity creation */
        Symbol*                     new_var;
        uint64_t                    clone_identity;
        bool                        literalized;

        /* Fields for transitive constraint attachment  */
        condition*                  operational_cond;
        WME_Field                   operational_field;

        /* This is an ID used by DEBUG_IDSET_INVENTORY.  Not used otherwise. */
        uint64_t                    is_id;

    private:

        agent*                      thisAgent;

};

#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SET_H_ */
