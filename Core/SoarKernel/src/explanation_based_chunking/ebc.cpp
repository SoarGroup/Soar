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
#include "production.h"
#include "print.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "soar_instance.h"
#include "soar_TraceNames.h"
#include "test.h"
#include "xml.h"

#include <assert.h>

using namespace soar_TraceNames;

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
    identity_to_var_map = new id_to_sym_id_map();
    instantiation_identities = new sym_to_id_map();
    id_to_rule_sym_debug_map = new id_to_sym_map();
    identities_to_clean_up = new id_set();
    constraints = new constraint_list();
    attachment_points = new attachment_points_map();
    unification_map = new id_to_id_map();
    cond_merge_map = new triple_merge_map();
    local_linked_STIs = new rhs_value_list();

    init_chunk_cond_set(&negated_set);

    /* Initialize learning setting */
    chunk_name_prefix = make_memory_block_for_string(thisAgent, "chunk");
    justification_name_prefix = make_memory_block_for_string(thisAgent, "justify");

    local_singleton_superstate_identity = NULL;
    chunk_history = new std::string();
    lti_link_function = NULL;
    reinit();
}

Explanation_Based_Chunker::~Explanation_Based_Chunker()
{
    clear_data();
    delete identity_to_var_map;
    delete constraints;
    delete attachment_points;
    delete cond_merge_map;
    delete instantiation_identities;
    delete unification_map;
    delete id_to_rule_sym_debug_map;
    delete chunk_history;
    delete local_linked_STIs;

    free_memory_block_for_string(thisAgent, chunk_name_prefix);
    free_memory_block_for_string(thisAgent, justification_name_prefix);
}

void Explanation_Based_Chunker::reinit()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing...\n");
    clear_data();

    inst_id_counter                     = 0;
    prod_id_counter                     = 0;
    m_chunk_new_i_id                    = 0;
    ovar_id_counter                     = 0;
    backtrace_number                    = 0;
    chunk_naming_counter                = 0;
    justification_naming_counter        = 0;
    grounds_tc                          = 0;
    m_results_match_goal_level          = 0;
    m_results_tc                        = 0;
    m_reliable                          = false;
    m_inst                              = NULL;
    m_results                           = NULL;
    m_extra_results                     = NULL;
    m_lhs                         = NULL;
    m_rhs                               = NULL;
    m_prod                              = NULL;
    m_chunk_inst                        = NULL;
    m_prod_name                         = NULL;
    chunk_free_problem_spaces           = NIL;
    chunky_problem_spaces               = NIL;
    local_singleton_superstate_identity = NULL_IDENTITY_SET;
    m_failure_type                      = ebc_success;
    m_rule_type                         = ebc_no_rule;
    m_learning_on_for_instantiation     = ebc_settings[SETTING_EBC_LEARNING_ON];

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
        if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\nWill not attempt to learn a chunk for match of " << inst->prod_name->to_string() << " because state " << inst->match_goal->to_string() << " was flagged to prevent learning";
            thisAgent->outputManager->printa_sf(thisAgent,  message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());

        }
        //            chunk_history += "Did not attempt to learn a chunk for match of ";
        //            chunk_history += inst->prod_name->to_string();
        //            chunk_history += " because state ";
        //            chunk_history += inst->match_goal->to_string();
        //            chunk_history += " was flagged to prevent learning (chunk all-except)\n";        m_learning_on_for_instantiation = false;
        m_learning_on_for_instantiation = false;
        return false;
    }

    if (ebc_settings[SETTING_EBC_ONLY]  &&
            !member_of_list(inst->match_goal, chunky_problem_spaces))
    {
        if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\nWill not attempt to learn a chunk for match of " << inst->prod_name->to_string() << " because state " << inst->match_goal->to_string() << " was not flagged for learning";
            thisAgent->outputManager->printa_sf(thisAgent,  message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
        }
        //            chunk_history += "Did not attempt to learn a chunk for match of ";
        //            chunk_history += inst->prod_name->to_string();
        //            chunk_history += " because state ";
        //            chunk_history += inst->match_goal->to_string();
        //            chunk_history += " was not flagged for learning.  (chunk only)\n";
        m_learning_on_for_instantiation = false;
        return false;
    }

    /* allow_bottom_up_chunks will be false if a chunk was already
       learned in a lower goal
     */
    if (ebc_settings[SETTING_EBC_BOTTOM_ONLY]  &&
            !inst->match_goal->id->allow_bottom_up_chunks)
    {
        if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\nWill not attempt to learn a chunk for match of " << inst->prod_name->to_string() << " because state " << inst->match_goal->to_string() << " is not the bottom state";
            thisAgent->outputManager->printa_sf(thisAgent,  message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
        }
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

Symbol* Explanation_Based_Chunker::generate_name_for_new_rule()
{
    Symbol* generated_name;
    Symbol* goal;
    byte impasse_type;
    preference* p;
    goal_stack_level lowest_result_level;
    std::string lImpasseName;
    std::stringstream lName;
    std::string rule_prefix;
    uint64_t rule_number, rule_naming_counter;
    chunkNameFormats chunkNameFormat = ebc_params->naming_style->get_value();

    if (m_rule_type == ebc_chunk)
    {
        rule_prefix = chunk_name_prefix;
        rule_number = chunks_this_d_cycle;
        if (chunkNameFormat == numberedFormat)
        {
            increment_counter(chunk_naming_counter);
            rule_naming_counter = chunk_naming_counter;
        }
    } else {
        rule_prefix = justification_name_prefix;
        rule_number = justifications_this_d_cycle;
        if (chunkNameFormat == numberedFormat)
        {
            increment_counter(justification_naming_counter);
            rule_naming_counter = justification_naming_counter;
        }
    }

    lowest_result_level = thisAgent->top_goal->id->level;
    for (p = m_inst->preferences_generated; p != NIL; p = p->inst_next)
        if (p->id->id->level > lowest_result_level)
        {
            lowest_result_level = p->id->id->level;
        }

    goal = find_goal_at_goal_stack_level(thisAgent, lowest_result_level);

    switch (chunkNameFormat)
    {
        case numberedFormat:
        {
            return (thisAgent->symbolManager->generate_new_str_constant(rule_prefix.c_str(), &(rule_naming_counter)));
        }
        case ruleFormat:
        {
            std::string real_prod_name;

            lImpasseName.erase();
            lName << rule_prefix;

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
                        sym = find_impasse_wme_value(goal->id->lower_goal, thisAgent->symbolManager->soarSymbols.attribute_symbol);
                        if (sym)
                        {
                            if (sym == thisAgent->symbolManager->soarSymbols.operator_symbol)
                            {
                                lImpasseName = "OpNoChange";
                            }
                            else if (sym == thisAgent->symbolManager->soarSymbols.state_symbol)
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

            if (m_inst->prod)
            {
                std::string:: size_type pos1, pos2, pos3;
                std::string numberString;
                std::string sourceString = m_inst->prod_name->sc->name;
                std::string targetString = m_inst->prod->original_rule_name;
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
                            std::cout << "Exception caught!" << std::endl;
                        }
                        lName << 'x' << (pNum + 1);
                    } else {
                        lName << "x2";
                    }
                }
                lName << "*" << m_inst->prod->original_rule_name;
            }
            if (!lImpasseName.empty())
            {
                lName << "*" << lImpasseName;
            }

            if (thisAgent->init_count)
            {
                lName << "*t" << (thisAgent->init_count + 1) << "-" << thisAgent->d_cycle_count << "-" << rule_number;
            } else {
                lName << "*t" << thisAgent->d_cycle_count << "-" << rule_number;
            }
        }
    }
    lImpasseName.erase();
    if (lName.str().empty()) { return NULL; }

    if (thisAgent->symbolManager->find_str_constant(lName.str().c_str()))
    {
        uint64_t collision_count = 1;
        std::stringstream newLName;
        std::string lFeedback;

        do
        {
            newLName.str("");
            newLName << lName.str() << "-" << collision_count++;
        }
        while (thisAgent->symbolManager->find_str_constant(newLName.str().c_str()));

        thisAgent->outputManager->sprinta_sf(thisAgent, lFeedback, "Warning: generated %s name %s already exists.  Used %s instead.\n", rule_prefix.c_str(), lName.str().c_str(), newLName.str().c_str());
        thisAgent->outputManager->printa(thisAgent, lFeedback.c_str());
        xml_generate_warning(thisAgent, lFeedback.c_str());

        lName.str(newLName.str());
        newLName.str("");
    }

    generated_name = thisAgent->symbolManager->make_str_constant(lName.str().c_str());
//    dprint(DT_DEBUG, "Generated name %s.\n", lName.str().c_str());
    return generated_name;
}
void Explanation_Based_Chunker::set_up_rule_name()
{
    /* Generate a new symbol for the name of the new chunk or justification */
    if (m_rule_type == ebc_chunk)
    {
        chunks_this_d_cycle++;
        m_prod_name = generate_name_for_new_rule();

        m_prod_type = CHUNK_PRODUCTION_TYPE;
        m_should_print_name = (thisAgent->trace_settings[TRACE_CHUNK_NAMES_SYSPARAM] != 0);
        m_should_print_prod = (thisAgent->trace_settings[TRACE_CHUNKS_SYSPARAM] != 0);
    }
    else
    {
        justifications_this_d_cycle++;
        m_prod_name = generate_name_for_new_rule();
        m_prod_type = JUSTIFICATION_PRODUCTION_TYPE;
        m_should_print_name = (thisAgent->trace_settings[TRACE_JUSTIFICATION_NAMES_SYSPARAM] != 0);
        m_should_print_prod = (thisAgent->trace_settings[TRACE_JUSTIFICATIONS_SYSPARAM] != 0);
        thisAgent->explanationMemory->increment_stat_justifications_attempted();
    }

    if (m_should_print_name)
    {
        thisAgent->outputManager->start_fresh_line(thisAgent);
        thisAgent->outputManager->printa_sf(thisAgent, "\nLearning new rule %y\n", m_prod_name);
        xml_begin_tag(thisAgent, kTagLearning);
        xml_begin_tag(thisAgent, kTagProduction);
        xml_att_val(thisAgent, kProduction_Name, m_prod_name);
        xml_end_tag(thisAgent, kTagProduction);
        xml_end_tag(thisAgent, kTagLearning);
    }
}
