/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "agent.h"
#include "instantiations.h"
#include "prefmem.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "debug.h"
#include "rhs.h"
#include "xml.h"

EBC_Manager::EBC_Manager(agent* myAgent)
{
    thisAgent = myAgent;
    sym_to_var_map = new std::unordered_map< Symbol*, Symbol* >();
    o_id_to_var_map = new std::unordered_map< uint64_t, Symbol* >();

    rulesym_to_identity_map = new std::unordered_map< uint64_t, std::unordered_map< Symbol*, uint64_t > >();
    o_id_to_ovar_debug_map = new std::unordered_map< uint64_t, Symbol* >();

    constraints = new std::list< constraint* >;
    attachment_points = new std::unordered_map< uint64_t, attachment_point* >();

    unification_map = new std::unordered_map< uint64_t, uint64_t >();

    cond_merge_map = new std::unordered_map< Symbol*, std::unordered_map< Symbol*, std::unordered_map< Symbol*, condition*> > >();

    inst_id_counter = 0;
    ovar_id_counter = 0;

    m_learning_on = thisAgent->sysparams[LEARNING_ON_SYSPARAM];
    m_learning_on_for_instantiation = m_learning_on;

    outputManager = &Output_Manager::Get_OM();
}

EBC_Manager::~EBC_Manager()
{
    clear_data();
    delete sym_to_var_map;
    delete o_id_to_var_map;
    delete constraints;
    delete attachment_points;
    delete cond_merge_map;
    delete rulesym_to_identity_map;
    delete unification_map;
    delete o_id_to_ovar_debug_map;
}

void EBC_Manager::reinit()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing...\n");
    clear_data();
    inst_id_counter = 0;
    ovar_id_counter = 0;
}

bool EBC_Manager::set_learning_for_instantiation(instantiation* inst)
{
    m_learning_on = thisAgent->sysparams[LEARNING_ON_SYSPARAM];

    if (thisAgent->sysparams[LEARNING_ON_SYSPARAM] == 0)
    {
        m_learning_on_for_instantiation = false;
        return false;
    }

    if (thisAgent->sysparams[LEARNING_EXCEPT_SYSPARAM] &&
            member_of_list(inst->match_goal, thisAgent->chunk_free_problem_spaces))
    {
        if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_CHUNKS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\nnot chunking due to chunk-free state " << inst->match_goal->to_string();
            print(thisAgent,  message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
        }
        m_learning_on_for_instantiation = false;
        return false;
    }

    if (thisAgent->sysparams[LEARNING_ONLY_SYSPARAM] &&
            !member_of_list(inst->match_goal, thisAgent->chunky_problem_spaces))
    {
        if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_CHUNKS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\nnot chunking due to non-chunky state " << inst->match_goal->to_string();
            print(thisAgent,  message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
        }
        m_learning_on_for_instantiation = false;
        return false;
    }

    /* allow_bottom_up_chunks will be false if a chunk was already
       learned in a lower goal
     */
    if (!thisAgent->sysparams[LEARNING_ALL_GOALS_SYSPARAM] &&
            !inst->match_goal->id->allow_bottom_up_chunks)
    {
        m_learning_on_for_instantiation = false;
        return false;
    }

    m_learning_on_for_instantiation = true;
    return true;
}
