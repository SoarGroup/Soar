/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"

#include "agent.h"
#include "decide.h"
#include "dprint.h"
#include "ebc_settings.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "rhs.h"
#include "soar_instance.h"
#include "test.h"
#include "xml.h"

#include <assert.h>

extern Symbol* find_goal_at_goal_stack_level(agent* thisAgent, goal_stack_level level);
extern Symbol* find_impasse_wme_value(Symbol* id, Symbol* attr);
byte type_of_existing_impasse(agent* thisAgent, Symbol* goal);

Explanation_Based_Chunker::Explanation_Based_Chunker(agent* myAgent)
{
    /* Cache agent and Output Manager pointer */
    thisAgent = myAgent;
    outputManager = &Output_Manager::Get_OM();

    /* Create the parameter object where the cli settings are stored.
     * This also initializes the ebc_settings array */
    ebc_params = new ebc_param_container(thisAgent, ebc_settings, max_chunks, max_dupes);

    /* Create data structures used for EBC */
    o_id_to_var_map = new id_to_sym_map_type();
    instantiation_identities = new inst_to_id_map_type();
    id_to_rule_sym_debug_map = new id_to_sym_map_type();
    constraints = new constraint_list();
    attachment_points = new attachment_points_map_type();
    unification_map = new id_to_id_map_type();
    cond_merge_map = new triple_merge_map();
    rhs_var_to_match_map = new sym_to_sym_map_type();
    init_chunk_cond_set(&negated_set);

    /* Initialize learning setting */
    m_learning_on_for_instantiation = ebc_settings[SETTING_EBC_LEARNING_ON];

    max_chunks_reached = false;
    m_failure_type = ebc_success;

    chunk_name_prefix = make_memory_block_for_string(thisAgent, "chunk");
    justification_name_prefix = make_memory_block_for_string(thisAgent, "justify");

    local_singleton_superstate_identity = NULL;
    chunk_history = new std::string();

    reinit();
}

Explanation_Based_Chunker::~Explanation_Based_Chunker()
{
    clear_data();
    delete o_id_to_var_map;
    delete constraints;
    delete attachment_points;
    delete cond_merge_map;
    delete instantiation_identities;
    delete unification_map;
    delete id_to_rule_sym_debug_map;
    delete rhs_var_to_match_map;
    delete chunk_history;

    free_memory_block_for_string(thisAgent, chunk_name_prefix);
    free_memory_block_for_string(thisAgent, justification_name_prefix);
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
    m_failure_type                      = ebc_success;

//    chunk_history += "Soar re-initialization performed.\n";
}

bool Explanation_Based_Chunker::set_learning_for_instantiation(instantiation* inst)
{
    if (!ebc_settings[SETTING_EBC_LEARNING_ON] || (inst->match_goal_level == TOP_GOAL_LEVEL))
    {
        m_learning_on_for_instantiation = false;
        return false;
    }

    if (ebc_settings[SETTING_EBC_EXCEPT] &&
            member_of_list(inst->match_goal, chunk_free_problem_spaces))
    {
        if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_CHUNKS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\nWill not attempt to learn a chunk for match of " << inst->prod_name->to_string() << " because state " << inst->match_goal->to_string() << " was flagged to prevent learning";
            print(thisAgent,  message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
//            chunk_history += "Did not attempt to learn a chunk for match of ";
//            chunk_history += inst->prod_name->to_string();
//            chunk_history += " because state ";
//            chunk_history += inst->match_goal->to_string();
//            chunk_history += " was flagged to prevent learning (chunk all-except)\n";
        }
        m_learning_on_for_instantiation = false;
        return false;
    }

    if (ebc_settings[SETTING_EBC_ONLY]  &&
            !member_of_list(inst->match_goal, chunky_problem_spaces))
    {
        if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_CHUNKS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\nWill not attempt to learn a chunk for match of " << inst->prod_name->to_string() << " because state " << inst->match_goal->to_string() << " was not flagged for learning";
            print(thisAgent,  message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
//            chunk_history += "Did not attempt to learn a chunk for match of ";
//            chunk_history += inst->prod_name->to_string();
//            chunk_history += " because state ";
//            chunk_history += inst->match_goal->to_string();
//            chunk_history += " was not flagged for learning.  (chunk only)\n";
        }
        m_learning_on_for_instantiation = false;
        return false;
    }

    /* allow_bottom_up_chunks will be false if a chunk was already
       learned in a lower goal
     */
    if (ebc_settings[SETTING_EBC_BOTTOM_ONLY]  &&
            !inst->match_goal->id->allow_bottom_up_chunks)
    {
        std::ostringstream message;
        message << "\nWill not attempt to learn a chunk for match of " << inst->prod_name->to_string() << " because state " << inst->match_goal->to_string() << " is not the bottom state";
        print(thisAgent,  message.str().c_str());
        xml_generate_verbose(thisAgent, message.str().c_str());
//        chunk_history += "Did not attempt to learn a chunk for match of ";
//        chunk_history += inst->prod_name->to_string();
//        chunk_history += " because state ";
//        chunk_history += inst->match_goal->to_string();
//        chunk_history += " not the bottom state.  (chunk bottom-only)\n";
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
    std::string rule_prefix;
    uint64_t rule_number;
    chunkNameFormats chunkNameFormat = ebc_params->naming_style->get_value();

    if (pIsChunk)
    {
        rule_prefix = chunk_name_prefix;
        rule_number = chunks_this_d_cycle;
    } else {
        rule_prefix = justification_name_prefix;
        rule_number = justifications_this_d_cycle;
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
            return (generate_new_str_constant(thisAgent, rule_prefix.c_str(), &(chunk_count)));
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
                        lImpasseName = "Failure";
                        break;
                    case CONFLICT_IMPASSE_TYPE:
                        lImpasseName = "Conflict";
                        break;
                    case TIE_IMPASSE_TYPE:
                        lImpasseName = "Tie";
                        break;
                    case NO_CHANGE_IMPASSE_TYPE:
                    {
                        Symbol* sym;
                        sym = find_impasse_wme_value(goal->id->lower_goal, thisAgent->attribute_symbol);
                        if (sym)
                        {
                            if (sym == thisAgent->operator_symbol)
                            {
                                lImpasseName = "OpNoChange";
                            }
                            else if (sym == thisAgent->state_symbol)
                            {
                                lImpasseName = "StateNoChange";
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
                std::string:: size_type pos1, pos2, pos3;
                std::string numberString;
                std::string sourceString = inst->prod_name->sc->name;
                std::string targetString = inst->prod->original_rule_name;
                int pNum(0);

                pos1 = sourceString.find(rule_prefix);
                pos2 = sourceString.find(targetString);
                if ((pos1 ==  0)  && (pos2 !=  std::string::npos) && (pos2 > pos1))
                {
                    if ((pos2 - rule_prefix.length()) >= 3)
                    {
                        numberString = sourceString.substr((rule_prefix.length() + 1), (pos2 - rule_prefix.length()-2));
                        try
                        {
                            pNum = std::stoi(numberString);
                        }
                        catch(std::invalid_argument&) //or catch(...) to catch all exceptions
                        {
                            std::cout << "Exception caugth!" << std::endl;
                        }
                        lName << 'x' << (pNum + 1);
                    } else {
                        lName << "x2";
                    }
                }
                lName << "*" << inst->prod->original_rule_name;
            }
            if (!lImpasseName.empty())
            {
                lName << "*" << lImpasseName;
            }

            lName << "*t" << thisAgent->d_cycle_count << "-" << rule_number;
        }
    }
    lImpasseName.erase();
    if (lName.str().empty()) { return NULL; }

    if (find_str_constant(thisAgent, lName.str().c_str()))
    {
        uint64_t collision_count = 1;
        std::stringstream newLName;

        print(thisAgent, "Warning: generated chunk name already exists.  Will find unique name.\n");
        xml_generate_warning(thisAgent, "Warning: generated chunk name already exists.  Will find unique name.");
        do
        {
            newLName.str("");
            newLName << lName.str() << "-" << collision_count++;
        }
        while (find_str_constant(thisAgent, newLName.str().c_str()));
        lName.str(newLName.str());
        newLName.str("");
    }

    generated_name = make_str_constant(thisAgent, lName.str().c_str());
    return generated_name;
}
