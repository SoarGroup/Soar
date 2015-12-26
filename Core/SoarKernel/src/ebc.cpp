/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "agent.h"
#include "assert.h"
#include "debug.h"
#include "decide.h"
#include "ebc_explain.h"
#include "instantiations.h"
#include "prefmem.h"
#include "print.h"
#include "rhs.h"
#include "soar_instance.h"
#include "test.h"
#include "xml.h"

extern Symbol* find_goal_at_goal_stack_level(agent* thisAgent, goal_stack_level level);
extern Symbol* find_impasse_wme_value(Symbol* id, Symbol* attr);
byte type_of_existing_impasse(agent* thisAgent, Symbol* goal);

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

Symbol* Explanation_Based_Chunker::generate_chunk_name(instantiation* inst)
{
    Symbol* generated_name;
    Symbol* goal;
    byte impasse_type;
    preference* p;
    goal_stack_level lowest_result_level;
    std::string lImpasseName;
    std::stringstream lName;

    chunkNameFormats chunkNameFormat = Soar_Instance::Get_Soar_Instance().Get_Chunk_Name_Format();

    lowest_result_level = thisAgent->top_goal->id->level;
    for (p = inst->preferences_generated; p != NIL; p = p->inst_next)
        if (p->id->id->level > lowest_result_level)
        {
            lowest_result_level = p->id->id->level;
        }

    goal = find_goal_at_goal_stack_level(thisAgent, lowest_result_level);

    switch (chunkNameFormat)
    {
        case numberedFormat:
        {
            return (generate_new_str_constant(
                        thisAgent,
                        chunk_name_prefix,
                        &chunk_count));
        }
        case longFormat:
        {
            if (goal)
            {
                impasse_type = type_of_existing_impasse(thisAgent, goal);
                switch (impasse_type)
                {
                    case NONE_IMPASSE_TYPE:
                        lImpasseName = "unknownimpasse";
                        break;
                    case CONSTRAINT_FAILURE_IMPASSE_TYPE:
                        lImpasseName = "cfailure";
                        break;
                    case CONFLICT_IMPASSE_TYPE:
                        lImpasseName = "conflict";
                        break;
                    case TIE_IMPASSE_TYPE:
                        lImpasseName = "tie";
                        break;
                    case NO_CHANGE_IMPASSE_TYPE:
                    {
                        Symbol* sym;

                        if ((sym = find_impasse_wme_value(goal->id->lower_goal, thisAgent->attribute_symbol)) == NIL)
                        {
                            lImpasseName = "unknownimpasse";
                        }
                        else if (sym == thisAgent->operator_symbol)
                        {
                            lImpasseName = "opnochange";
                        }
                        else if (sym == thisAgent->state_symbol)
                        {
                            lImpasseName = "snochange";
                        }
                        else
                        {
                            lImpasseName = "unknownimpasse";
                        }
                    }
                    break;
                    default:
                        lImpasseName = "unknownimpasse";
                        break;
                }
            }
            else
            {
                lImpasseName = "unknownimpasse";
            }
            lName << chunk_name_prefix << "-" << chunk_count++ << "*" <<
                  thisAgent->d_cycle_count << "*" << lImpasseName << "*" << chunks_this_d_cycle;

            break;
        }
        case ruleFormat:
        {
            std::string real_prod_name;

            lImpasseName.erase();
            lName << chunk_name_prefix;

            chunk_count++;
            if (goal)
            {
                impasse_type = type_of_existing_impasse(thisAgent, goal);
                switch (impasse_type)
                {
                    case CONSTRAINT_FAILURE_IMPASSE_TYPE:
                        lImpasseName = "cfo";
                        break;
                    case CONFLICT_IMPASSE_TYPE:
                        lImpasseName = "con";
                        break;
                    case TIE_IMPASSE_TYPE:
                        lImpasseName = "tie";
                        break;
                    case NO_CHANGE_IMPASSE_TYPE:
                    {
                        Symbol* sym;
                        sym = find_impasse_wme_value(goal->id->lower_goal, thisAgent->attribute_symbol);
                        if (sym)
                        {
                            if (sym == thisAgent->operator_symbol)
                            {
                                lImpasseName = "onc";
                            }
                            else if (sym == thisAgent->state_symbol)
                            {
                                lImpasseName = "snc";
                            }
                        }
                    }
                    break;
                    default:
                        break;
                }
            }

            if (inst->prod)
            {
                if (strstr(inst->prod->name->sc->name, chunk_name_prefix) == inst->prod->name->sc->name)
                {
                    /*-- This is a chunk based on a chunk, so annotate name to indicate --*/
                    lName << "-multi";
                }
                lName << "*" << inst->prod->original_rule_name;
            }
            if (!lImpasseName.empty())
            {
                lName << "*" << lImpasseName;
            }
            lName << "*t" << thisAgent->d_cycle_count << "-" << chunks_this_d_cycle;
        }
    }
    lImpasseName.erase();
    if (lName.str().empty()) { return NULL; }

    /* Any user who named a production like this deserves to be burned, but we'll have mercy: */
    if (find_str_constant(thisAgent, lName.str().c_str()))
    {
        uint64_t collision_count = 1;
        std::stringstream newLName;

        print(thisAgent, "Warning: generated chunk name already exists.  Will find unique name.\n");
        xml_generate_warning(thisAgent, "Warning: generated chunk name already exists.  Will find unique name.");
//        dprint(DT_DEBUG, "Chunk name %s already exists...trying...", lName.str().c_str());
        do
        {
            newLName.str("");
            newLName << lName.str() << "-" << collision_count++;
//            dprint_noprefix(DT_DEBUG, "%s\n", newLName.str().c_str());
        }
        while (find_str_constant(thisAgent, newLName.str().c_str()));
        lName.str(newLName.str());
        newLName.str("");
    }

    generated_name = make_str_constant(thisAgent, lName.str().c_str());
    return generated_name;
}
