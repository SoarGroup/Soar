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

//class IdentitySet : public std::enable_shared_from_this<IdentitySet>
class IdentitySet
{
    public:

        IdentitySet(agent* my_agent);
        ~IdentitySet();

        bool        literalized()           { if (super_join) return super_join->m_literalized; else return m_literalized; }
        bool        joined()                { if (super_join) return true; else return false; }
        uint64_t    get_identity()          { if (super_join) return super_join->idset_id; else return idset_id; }
        uint64_t    get_sub_identity()      { return idset_id; }
        uint64_t    get_clone_identity()    { if (super_join) return super_join->clone_identity; else return clone_identity; }
        Symbol*     get_var()               { if (super_join) return super_join->new_var; else return new_var; }
        condition*  get_operational_cond()  { if (super_join) return super_join->operational_cond; else return operational_cond; }
        WME_Field   get_operational_field() { if (super_join) return super_join->operational_field; else return operational_field; }

        void literalize()                               { if (super_join) super_join->m_literalized = true; else { m_literalized = true; touch(); }}
        void set_clone_identity(uint64_t pID)           { if (super_join) super_join->clone_identity = pID; else { clone_identity = pID; touch(); }}
        void set_var(Symbol* pVar)                      { if (super_join) super_join->new_var = pVar; else { new_var = pVar; touch(); }}

        void set_operational_cond(condition* pCond, WME_Field pField);
        void store_variablization(Symbol* variable, Symbol* pMatched_sym);
        void update_clone_id();
        void touch();
        void clean_up_transient();

        /* An ID used for printing and to make debugging easier. Not used by identity analysis logic. */
        uint64_t                    idset_id;

        /* A pointer to the join super identity.  Used to point back to itself but that's bad with shared pointers */
        IdentitySetSharedPtr        super_join;

        /* Pointers to other identity_set nodes that map to this one */
        identity_set_list*          identity_sets;

        /* Flag indicating whether any transient data has been changed */
        bool                        dirty;

    private:

        agent*                      thisAgent;


        /* Fields for variablization and chunk instantiation identity creation */
        Symbol*                     new_var;
        uint64_t                    clone_identity;
        bool                        m_literalized;

        /* Fields for transitive constraint attachment  */
        condition*                  operational_cond;
        WME_Field                   operational_field;


};

#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SET_H_ */
