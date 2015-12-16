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

Explanation_Based_Chunker::Explanation_Based_Chunker(agent* myAgent)
{
    /* Cache agent and Output Manager pointer */
    thisAgent = myAgent;
    outputManager = &Output_Manager::Get_OM();

    /* Initialize instantiation and identity ID counters */
    inst_id_counter = 0;
    ovar_id_counter = 0;

    /* Create data structures used for EBC */
    sym_to_var_map = new std::unordered_map< Symbol*, Symbol* >();
    o_id_to_var_map = new std::unordered_map< uint64_t, Symbol* >();
    rulesym_to_identity_map = new std::unordered_map< uint64_t, std::unordered_map< Symbol*, uint64_t > >();
    o_id_to_ovar_debug_map = new std::unordered_map< uint64_t, Symbol* >();
    constraints = new std::list< constraint* >;
    attachment_points = new std::unordered_map< uint64_t, attachment_point* >();
    unification_map = new std::unordered_map< uint64_t, uint64_t >();
    cond_merge_map = new std::unordered_map< Symbol*, std::unordered_map< Symbol*, std::unordered_map< Symbol*, condition*> > >();
    init_chunk_cond_set(&negated_set);

    /* Initialize learning setting */
    m_learning_on = thisAgent->sysparams[LEARNING_ON_SYSPARAM];
    m_learning_on_for_instantiation = m_learning_on;

    backtrace_number                   = 0;
    chunk_count                        = 1;
    justification_count                = 1;
    strcpy(chunk_name_prefix, "chunk"); /* ajc (5/14/02) */
    grounds_tc                         = 0;
    locals_tc                          = 0;
    potentials_tc                      = 0;

    max_chunks_reached = false;
    chunk_free_problem_spaces          = NIL;
    chunky_problem_spaces              = NIL;

}

Explanation_Based_Chunker::~Explanation_Based_Chunker()
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

void Explanation_Based_Chunker::reinit()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing...\n");
    clear_data();
    inst_id_counter = 0;
    ovar_id_counter = 0;
    chunk_free_problem_spaces          = NIL;
    chunky_problem_spaces              = NIL;
}

bool Explanation_Based_Chunker::set_learning_for_instantiation(instantiation* inst)
{
    m_learning_on = thisAgent->sysparams[LEARNING_ON_SYSPARAM];

    if (thisAgent->sysparams[LEARNING_ON_SYSPARAM] == 0)
    {
        m_learning_on_for_instantiation = false;
        return false;
    }

    if (thisAgent->sysparams[LEARNING_EXCEPT_SYSPARAM] &&
            member_of_list(inst->match_goal, chunk_free_problem_spaces))
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
            !member_of_list(inst->match_goal, chunky_problem_spaces))
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
