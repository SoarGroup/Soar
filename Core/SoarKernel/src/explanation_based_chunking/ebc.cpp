/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "assert.h"
#include "debug.h"
#include "decide.h"
#include "explain.h"
#include "ebc.h"
#include "print.h"
#include "soar_instance.h"
#include "xml.h"
#include "agent.h"
#include "instantiation.h"
#include "preference.h"
#include "rhs.h"
#include "test.h"

extern Symbol* find_goal_at_goal_stack_level(agent* thisAgent, goal_stack_level level);
extern Symbol* find_impasse_wme_value(Symbol* id, Symbol* attr);
byte type_of_existing_impasse(agent* thisAgent, Symbol* goal);

Explanation_Based_Chunker::Explanation_Based_Chunker(agent* myAgent)
{
    /* Cache agent and Output Manager pointer */
    thisAgent = myAgent;
    outputManager = &Output_Manager::Get_OM();

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

    chunkNameFormat = ruleFormat;
    max_chunks_reached = false;
    strcpy(chunk_name_prefix, "chunk");
    strcpy(justification_name_prefix, "justify");

    reinit();
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
    free(chunk_name_prefix);
    free(justification_name_prefix);
}

void Explanation_Based_Chunker::reinit()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing...\n");
    clear_data();

    inst_id_counter                     = 0;
    m_chunk_new_i_id                    = 0;
    ovar_id_counter                     = 0;
    backtrace_number                    = 0;
    chunk_count                         = 0;
    justification_count                 = 0;
    grounds_tc                          = 0;
    locals_tc                           = 0;
    potentials_tc                       = 0;
    m_results_match_goal_level          = 0;
    m_results_tc                        = 0;
    m_reliable                          = false;
    m_inst                              = NULL;
    m_results                           = NULL;
    m_extra_results                     = NULL;
    m_inst_top                          = NULL;
    m_inst_bottom                       = NULL;
    m_vrblz_top                         = NULL;
    m_rhs                               = NULL;
    m_prod                              = NULL;
    m_chunk_inst                        = NULL;
    m_prod_name                         = NULL;
    m_saved_justification_top           = NULL;
    m_saved_justification_bottom        = NULL;
    chunk_free_problem_spaces           = NIL;
    chunky_problem_spaces               = NIL;
}

bool Explanation_Based_Chunker::set_learning_for_instantiation(instantiation* inst)
{
    m_learning_on = thisAgent->sysparams[LEARNING_ON_SYSPARAM];

    if ((thisAgent->sysparams[LEARNING_ON_SYSPARAM] == 0) || (inst->match_goal_level == TOP_GOAL_LEVEL))
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
            message << "\nWill not attempt to learn a new rule because state " << inst->match_goal->to_string() << " was flagged to prevent learning";
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
            message << "\nWill not attempt to learn a new rule because state " << inst->match_goal->to_string() << " was not flagged for learning";
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

Symbol* Explanation_Based_Chunker::generate_chunk_name(instantiation* inst, bool pIsChunk)
{
    Symbol* generated_name;
    Symbol* goal;
    byte impasse_type;
    preference* p;
    goal_stack_level lowest_result_level;
    std::string lImpasseName;
    std::stringstream lName;
    char* rule_prefix;
    chunkNameFormats chunkNameFormat = Get_Chunk_Name_Format();

    if (pIsChunk)
    {
        rule_prefix = chunk_name_prefix;
    } else {
        rule_prefix = justification_name_prefix;
    }

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
            increment_counter(chunk_count);
            return (generate_new_str_constant(
                        thisAgent,
                        rule_prefix,
                        &(chunk_count)));
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
            increment_counter(chunk_count);
            lName << rule_prefix << "-" << chunk_count << "*" <<
                  thisAgent->d_cycle_count << "*" << lImpasseName << "*" << chunks_this_d_cycle;

            break;
        }
        case ruleFormat:
        {
            std::string real_prod_name;

            lImpasseName.erase();
            lName << rule_prefix;

            increment_counter(chunk_count);
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
                if (strstr(inst->prod_name->sc->name, rule_prefix) == inst->prod_name->sc->name)
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
