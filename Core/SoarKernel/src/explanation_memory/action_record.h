/*
 * action_record.h
 *
 *  Created on: Apr 22, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_ACTION_RECORD_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_ACTION_RECORD_H_

#include "kernel.h"
#include "stl_typedefs.h"

class production_record;

class action_record
{
        friend class Explanation_Memory;

    public:
        action_record() {};
        ~action_record() {};

        void init(agent* myAgent, preference* pPref, action* pAction, uint64_t pActionID, bool isChunkInstantiation);
        void clean_up();

        uint64_t                get_actionID()   { return actionID; };

        void                    print_rhs_chunk_value(const rhs_value pRHS_value, const rhs_value pRHS_variablized_value, bool printActual);
        void                    print_rhs_instantiation_value(const rhs_value pRHS_value, const rhs_value pPref_func, uint64_t pID, uint64_t pIDClone, bool printActual);
        void                    print_chunk_action(action* pAction, int lActionCount);
        void                    print_instantiation_action(action* pAction, int lActionCount);
        void                    viz_rhs_value(const rhs_value pRHS_value, const rhs_value pRHS_variablized_value, const rhs_value pRHS_func = NULL, uint64_t pID = 0, uint64_t pIDClone = 0, uint64_t pNodeID = 0, char pTypeChar = ' ', WME_Field pField = NO_ELEMENT);
        void                    viz_action(action* pAction);
        void                    viz_preference();
        /* Action lists are common to chunk records and instantiation records, but don't have a class to themselves */
        static void             viz_action_list(agent* thisAgent, action_record_list* pActionRecords, production* pOriginalRule, action* pRhs = NULL, production_record* pExcisedRule = NULL);

        preference*             original_pref;          // Only used when building explain records

    private:
        agent*                  thisAgent;
        preference*             instantiated_pref;
        action*                 variablized_action;
        id_set*                 identities_used;
        uint64_t                actionID;
};

#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_ACTION_RECORD_H_ */
