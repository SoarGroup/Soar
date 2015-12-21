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

class Output_Manager;

typedef struct action_record_struct {
    preference*     instantiated_pref;
    action*         variablized_action;
    u_int64_t       actionID;
} action_record;

typedef struct condition_record_struct {
    condition*      variablized_cond;
    condition*      instantiated_cond;
    wme*            matched_wme;
    action_record*  parent_action;
    u_int64_t       conditionID;
} condition_record;

#ifdef USE_MEM_POOL_ALLOCATORS
#include "soar_module.h"
typedef std::list< condition_record*, soar_module::soar_memory_pool_allocator< condition_record* > > condition_record_list;
typedef std::list< action_record*, soar_module::soar_memory_pool_allocator< action_record* > > action_record_list;
#else
typedef std::list< condition_record* > condition_record_list;
typedef std::list< action_record* > action_record_list;
#endif

typedef struct instantiation_record_struct {
    Symbol*                 production_name;
    condition_record_list*  conditions;
    action_record_list*     actions;
    std::string             identity_set_mapping_explanation;
    u_int64_t               instantiationID;
} instantiation_record;

typedef struct chunk_record_struct {
    std::string             name;
    condition_record_list*  conditions;
    action_record_list*     actions;
    std::string             identity_set_mapping_explanation;
    u_int64_t               baseInstantiationID;
    u_int64_t               chunkID;
} chunk_record;


class Explanation_Logger
{
    public:

        uint64_t add_chunk_record(instantiation* pbaseInstantiation);
        void     set_chunk_name(uint64_t pC_ID, Symbol* pNameSym);

        uint64_t add_instantiation_record(instantiation* pInst);
        uint64_t add_condition_to_instantiation_record(condition* pCond, uint64_t pI_ID);
        uint64_t add_result_to_instantiation_record(preference* pPref, uint64_t pI_ID);
        uint64_t add_condition_to_chunk_record(condition* pCond, uint64_t pC_ID);
        uint64_t add_result_to_chunk_record(action* pAction, preference* pPref, uint64_t pC_ID);

        Explanation_Logger(agent* myAgent);
        ~Explanation_Logger();

    private:

        agent* thisAgent;
        Output_Manager* outputManager;

        /* Statistics on learning performed so far */
        uint64_t            chunks_attempted_count;
        uint64_t            duplicate_chunks_count;
        uint64_t            merge_count;

        uint64_t            chunk_id_count;
        uint64_t            instantiation_id_count;
        uint64_t            condition_id_count;
        /* -- The following are tables used by the variablization manager during
         *    instantiation creation, backtracing and chunk formation.  The data
         *    they store is temporary and cleared after use. -- */

        std::unordered_map< uint64_t, chunk_record* >* chunks;
};

#endif /* EBC_EXPLAIN_H_ */
