/*
 * ebc.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Mazin Assanie
 */

#ifndef EBC_EXPLAIN_H_
#define EBC_EXPLAIN_H_

#include "kernel.h"
#include <list>
#include <set>
#include <unordered_map>
#include <list>
#include <cstdlib>
#include <string>

typedef char* rhs_value;
typedef signed short goal_stack_level;
typedef struct action_struct action;
typedef struct agent_struct agent;
typedef struct chunk_cond_struct chunk_cond;
typedef struct condition_struct condition;
typedef struct instantiation_struct instantiation;
typedef struct preference_struct preference;
typedef struct symbol_struct Symbol;
typedef struct test_struct test_info;
typedef test_info* test;
typedef struct wme_struct wme;
typedef struct rete_node_struct rete_node;
typedef struct node_varnames_struct node_varnames;
typedef unsigned short rete_node_level;
typedef uint64_t tc_number;
typedef struct cons_struct cons;
typedef cons list;
namespace soar_module
{
    typedef struct symbol_triple_struct symbol_triple;
    typedef struct identity_triple_struct identity_triple;
    typedef struct rhs_triple_struct rhs_triple;
}

class Output_Manager;

class action_record {

        friend class Explanation_Logger;

    public:
        action_record(agent* myAgent, preference* pPref, uint64_t pActionID);
        ~action_record();
        preference*     original_pref;          // Only used when building explain records
        uint64_t get_actionID() { return actionID; };

    private:
        agent* thisAgent;
        preference*     instantiated_pref;
        action*         variablized_action;
        uint64_t       actionID;
} ;

class condition_record{

        friend class Explanation_Logger;

    public:
        condition_record(agent* myAgent, condition* pCond, uint64_t pCondID);
        ~condition_record();
        uint64_t get_conditionID() { return conditionID; };

    private:
        agent* thisAgent;
        condition*                  variablized_cond;
        condition*                  instantiated_cond;
        soar_module::symbol_triple* matched_wme;
        action_record*              parent_action;
        uint64_t                   conditionID;
} ;

//#ifdef USE_MEM_POOL_ALLOCATORS
//#include "soar_module.h"
//typedef std::list< condition_record*, soar_module::soar_memory_pool_allocator< condition_record* > > condition_record_list;
//typedef std::list< action_record*, soar_module::soar_memory_pool_allocator< action_record* > > action_record_list;
//typedef std::list< uint64_t, soar_module::soar_memory_pool_allocator< uint64_t > > id_list;
//#else
//typedef std::list< condition_record* > condition_record_list;
//typedef std::list< action_record* > action_record_list;
//typedef std::list< uint64_t > id_list;
//#endif
typedef std::list< condition_record* > condition_record_list;
typedef std::list< action_record* > action_record_list;
typedef std::list< uint64_t > id_list;

class instantiation_record {

        friend class Explanation_Logger;

    public:
        instantiation_record(agent* myAgent, instantiation* pInst);
        ~instantiation_record();
        uint64_t get_instantiationID() { return instantiationID; };

        action_record* find_rhs_action(preference* pPref);

    private:
        agent* thisAgent;
        Symbol*                 production_name;
        condition_record_list*  conditions;
        action_record_list*     actions;
        uint64_t                instantiationID;
} ;

class chunk_record {

        friend class Explanation_Logger;

    public:
        chunk_record(agent* myAgent, instantiation* pBaseInstantiation, uint64_t pChunkID);
        ~chunk_record();

    private:
        agent* thisAgent;
        std::string             name;
        condition_record_list*  conditions;
        action_record_list*     actions;
        std::string             identity_set_mapping_explanation;
        instantiation_record*   baseInstantiation;
        uint64_t               chunkID;
} ;

class Explanation_Logger
{
        friend class instantiation_record;

    public:

        void                    re_init();
        void                    clear_explanations();

        void                    set_chunk_name(uint64_t pC_ID, Symbol* pNameSym);
        void                    set_backtrace_number(uint64_t pBT_num) { backtrace_number = pBT_num; };

        chunk_record*           add_chunk_record(instantiation* pbaseInstantiation);
        instantiation_record*   add_instantiation(instantiation* pInst);
        action_record*          get_action_for_instantiation(preference* pPref, instantiation* pInst);

        Explanation_Logger(agent* myAgent);
        ~Explanation_Logger();

    private:

        agent* thisAgent;
        Output_Manager* outputManager;

        tc_number               backtrace_number;

        void                    initialize_counters();
        instantiation_record*   get_instantiation(instantiation* pInst);
        condition_record*       add_condition(condition* pCond);
        action_record*          add_result(preference* pPref);
        uint64_t                add_condition_to_chunk_record(condition* pCond, chunk_record* pChunkRecord);
        uint64_t                add_result_to_chunk_record(action* pAction, preference* pPref, chunk_record* pChunkRecord);

        /* Statistics on learning performed so far */
        uint64_t            chunks_attempted_count;
        uint64_t            duplicate_chunks_count;
        uint64_t            merge_count;

        uint64_t            chunk_id_count;
        uint64_t            condition_id_count;
        uint64_t            action_id_count;

        /* These maps store all of the records the logger keeps */
        std::unordered_map< uint64_t, chunk_record* >*          chunks;
        std::unordered_map< uint64_t, instantiation_record* >*  instantiations;
        std::unordered_map< uint64_t, condition_record* >*      conditions;
        std::unordered_map< uint64_t, action_record* >*         actions;
};

#endif /* EBC_EXPLAIN_H_ */
