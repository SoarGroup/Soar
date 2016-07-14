/*
 * instantiation_record.h
 *
 *  Created on: Apr 22, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLAIN_INSTANTIATION_RECORD_H_
#define CORE_SOARKERNEL_SRC_EXPLAIN_INSTANTIATION_RECORD_H_

#include "kernel.h"
#include "stl_typedefs.h"

class condition_record;
class production_record;
class action_record;

class instantiation_record
{
        friend class Explanation_Memory;
        friend class chunk_record;

    public:

        instantiation_record(agent* myAgent, instantiation* pInst);
        ~instantiation_record();

        uint64_t                get_instantiationID() { return instantiationID; };
        goal_stack_level        get_match_level() { return match_level; };
        inst_record_list*       get_path_to_base() { return path_to_base; };
        uint64_t                get_chunk_creator() { return creating_chunk; }
        id_set*                 get_lhs_identities();
        void                    record_instantiation_contents();
        void                    update_instantiation_contents();
        void                    create_identity_paths(const inst_record_list* pInstPath);
        condition_record*       find_condition_for_chunk(preference* pPref, wme* pWME);
        action_record*          find_rhs_action(preference* pPref);

        void                    print_for_explanation_trace(bool printFooter);
        void                    print_for_wme_trace(bool printFooter);
        void                    visualize();

        void                    delete_instantiation();

        instantiation*          cached_inst;

    private:

        agent* thisAgent;
        uint64_t                instantiationID;
        Symbol*                 production_name;
        production*             original_production;
        production_record*      excised_production;
        uint64_t                creating_chunk;

        goal_stack_level        match_level;
        bool                    terminal;
        inst_record_list*       path_to_base;
        id_set*                 lhs_identities;

        condition_record_list*  conditions;
        action_record_list*     actions;

        void                    viz_et_instantiation();
        void                    viz_wm_instantiation();
        void                    viz_simple_instantiation();
        void                    viz_connect_conditions();

};

#endif /* CORE_SOARKERNEL_SRC_EXPLAIN_INSTANTIATION_RECORD_H_ */


