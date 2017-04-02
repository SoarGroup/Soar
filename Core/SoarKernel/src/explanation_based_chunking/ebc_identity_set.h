/*
 * ebc_identity_set.h
 *
 *  Created on: Mar 4, 2017
 *      Author: mazzin
 */
#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SET_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SET_H_

#include "kernel.h"

#include "mempool_allocator.h"

#include <list>

//#define DEBUG_TRACE_IDSET_REFCOUNTS

#ifndef SOAR_RELEASE_VERSION
#ifdef DEBUG_TRACE_IDSET_REFCOUNTS
#define DEBUG_MAC_STACKTRACE
    void get_stacktrace(std::string& return_string);
#endif
#endif

#ifdef USE_MEM_POOL_ALLOCATORS
    typedef std::list< IdentitySet*, soar_module::soar_memory_pool_allocator< IdentitySet* > >                      identity_set_list;
#else
    typedef std::list< IdentitySet* >                           identity_set_list;
#endif

class IdentitySet
{
    public:

        IdentitySet() {};
        ~IdentitySet() {};

        void init(agent* my_agent);
        void clean_up();

#ifndef DEBUG_TRACE_IDSET_REFCOUNTS
        void        add_ref()               { ++refcount; }
        bool        remove_ref()            { --refcount; return (refcount == 0);}
#else
        void        add_ref()
        {
            ++refcount;
            std::string caller_string;
            get_stacktrace(caller_string);
            dprint_noprefix(DT_IDSET_REFCOUNTS, "++ %u --> %u: %s\n", idset_id, refcount, caller_string.c_str());
        }
        bool        remove_ref()
        {
            --refcount;
            std::string caller_string;
            get_stacktrace(caller_string);
            dprint_noprefix(DT_IDSET_REFCOUNTS, "-- %u --> %u: %s\n", idset_id, refcount, caller_string.c_str());
            return (refcount == 0);
        }
#endif

        bool        literalized()           { return super_join->m_literalized; }
        bool        joined()                { return (super_join != this); }
        uint64_t    get_identity()          { return super_join->idset_id; }
        uint64_t    get_sub_identity()      { return idset_id; }
        uint64_t    get_clone_identity()    { return super_join->clone_identity; }
        Symbol*     get_var()               { return super_join->new_var; }
        condition*  get_operational_cond()  { return super_join->operational_cond; }
        WME_Field   get_operational_field() { return super_join->operational_field; }
        uint64_t    get_refcount()          { return refcount; }

        void literalize()                               { super_join->m_literalized = true; super_join->touch(); }
        void set_clone_identity(uint64_t pID)           { super_join->clone_identity = pID; super_join->touch(); }
        void set_var(Symbol* pVar)                      { super_join->new_var = pVar; super_join->touch(); }

        void        set_operational_cond(condition* pCond, WME_Field pField);
        void        store_variablization(Symbol* variable, Symbol* pMatched_sym);
        uint64_t    update_clone_id();
        void        touch();
        void        clean_up_transient();

        /* An ID used for printing and to make debugging easier. Not used by identity analysis logic. */
        uint64_t                    idset_id;
        uint64_t                    clone_identity;

        /* A pointer to the join super identity.  Used to point back to itself but that's bad with shared pointers */
        IdentitySet*                super_join;

        /* Pointers to other identity_set nodes that map to this one */
        identity_set_list*          identity_sets;

        /* Flag indicating whether any transient data has been changed */
        bool                        dirty;

    private:

        agent*                      thisAgent;

        /* Fields for variablization and chunk instantiation identity creation */
        Symbol*                     new_var;
        bool                        m_literalized;

        /* Fields for transitive constraint attachment  */
        condition*                  operational_cond;
        WME_Field                   operational_field;

        uint64_t                    refcount;

};

struct IDSetLessThan : public std::binary_function<IdentitySet*, IdentitySet*, bool> {
        bool operator()(const IdentitySet* lhs, const IdentitySet* rhs) const
        {
            return (lhs->idset_id < rhs->idset_id);
        }
};

#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_BASED_CHUNKING_EBC_IDENTITY_SET_H_ */
