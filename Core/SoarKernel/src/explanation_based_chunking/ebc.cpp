/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "ebc_settings.h"
#include "ebc_timers.h"

#include "agent.h"
#include "condition.h"
#include "decide.h"
#include "dprint.h"
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
#include "working_memory.h"
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

    /* Create the timers */
    ebc_timers = new ebc_timer_container(thisAgent);

    /* Create data structures used for EBC */
    identity_to_var_map = new id_to_sym_id_map();
    instantiation_identities = new sym_to_id_map();
    constraints = new constraint_list();
    attachment_points = new attachment_points_map();
    identities_to_id_sets = new id_to_id_map();
    identity_set_join_map = new id_to_join_map();
    literalized_identity_sets = new id_set();
    cond_merge_map = new triple_merge_map();
    local_linked_STIs = new rhs_value_list();

    init_chunk_cond_set(&negated_set);

    /* Initialize learning setting */
    chunk_name_prefix = make_memory_block_for_string(thisAgent, "chunk");
    justification_name_prefix = make_memory_block_for_string(thisAgent, "justify");

    singletons = new symbol_set();

    chunk_history = new std::string();
    lti_link_function = NULL;
    reinit();
}

Explanation_Based_Chunker::~Explanation_Based_Chunker()
{
    clear_data();

    delete ebc_params;
    delete ebc_timers;

    delete identity_to_var_map;
    delete instantiation_identities;
    delete constraints;
    delete attachment_points;
    delete identities_to_id_sets;
    delete identity_set_join_map;
    delete literalized_identity_sets;
    delete cond_merge_map;
    delete local_linked_STIs;
    free_memory_block_for_string(thisAgent, chunk_name_prefix);
    free_memory_block_for_string(thisAgent, justification_name_prefix);
    clear_singletons();
    delete singletons;
    delete chunk_history;
}

void Explanation_Based_Chunker::reinit()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing...\n");
    clear_data();
    ebc_timers->reset();
    instantiation_being_built           = NULL;
    inst_id_counter                     = 0;
    prod_id_counter                     = 0;
    ovar_id_counter                     = 0;
    backtrace_number                    = 0;
    chunk_naming_counter                = 0;
    justification_naming_counter        = 0;
    grounds_tc                          = 0;
    m_results_match_goal_level          = 0;
    m_goal_level                        = 0;
    m_results_tc                        = 0;
    m_correctness_issue_possible        = true;
    m_inst                              = NULL;
    m_results                           = NULL;
    m_extra_results                     = NULL;
    m_lhs                               = NULL;
    m_rhs                               = NULL;
    m_prod                              = NULL;
    m_chunk_inst                        = NULL;
    m_prod_name                         = NULL;
    chunk_free_problem_spaces           = NIL;
    chunky_problem_spaces               = NIL;
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

    if (ebc_settings[SETTING_EBC_EXCEPT] && member_of_list(inst->match_goal, chunk_free_problem_spaces))
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

    if (ebc_settings[SETTING_EBC_ONLY]  && !member_of_list(inst->match_goal, chunky_problem_spaces))
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
void Explanation_Based_Chunker::clear_data()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing all EBC maps.\n");
    clear_cached_constraints();
    clear_variablization_maps();
    clear_merge_map();
    clear_symbol_identity_map();
    clear_identity_to_id_set_map();
    clear_attachment_map();
}

void Explanation_Based_Chunker::clear_attachment_map()
{
    for (attachment_points_map::iterator it = (*attachment_points).begin(); it != (*attachment_points).end(); ++it)
    {
        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
    }
    attachment_points->clear();
}

void Explanation_Based_Chunker::clear_variablization_maps()
{
    dprint(DT_EBC_CLEANUP, "Original_Variable_Manager clearing variablization map...\n");
    for (auto it = (*identity_to_var_map).begin(); it != (*identity_to_var_map).end(); ++it)
    {
        thisAgent->symbolManager->symbol_remove_ref(&it->second->variable_sym);
        thisAgent->memoryManager->free_with_pool(MP_sym_identity, it->second);
    }
    identity_to_var_map->clear();
    literalized_identity_sets->clear();
    identity_set_join_map->clear();
}

void Explanation_Based_Chunker::sanity_chunk_test (test pTest)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
        for (cons* c = pTest->data.conjunct_list; c != NIL; c = c->rest)
        {
            sanity_chunk_test(static_cast<test>(c->first));
        }
    } else {
        assert(!test_has_referent(pTest) || !pTest->data.referent->is_sti());
    }
}

void Explanation_Based_Chunker::sanity_chunk_conditions(condition* top_cond)
{
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
            sanity_chunk_test(cond->data.tests.id_test);
            sanity_chunk_test(cond->data.tests.attr_test);
            sanity_chunk_test(cond->data.tests.value_test);
        }
        else
        {
            sanity_chunk_conditions(cond->data.ncc.top);
        }
    }
}

void Explanation_Based_Chunker::sanity_justification_test (test pTest, bool pIsNCC)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
        for (cons* c = pTest->data.conjunct_list; c != NIL; c = c->rest)
        {
            sanity_justification_test(static_cast<test>(c->first), pIsNCC);
        }
    } else {
        if (pIsNCC)
        {
            assert(!test_has_referent(pTest) || (!pTest->data.referent->is_variable() || !pTest->identity_set));

        } else {
            assert(!test_has_referent(pTest) || !pTest->data.referent->is_variable() || !pTest->identity_set);
        }
    }
}
ebc_timer_container::ebc_timer_container(agent* new_agent): soar_module::timer_container(new_agent)
{
    instantiation_creation = new ebc_timer("1.00 Instantiation creation", thisAgent, soar_module::timer::one);
    chunk_instantiation_creation = new ebc_timer("2.01 Chunk instantiation creation", thisAgent, soar_module::timer::one);
    dependency_analysis = new ebc_timer("2.02 Dependency analysis", thisAgent, soar_module::timer::one);
    identity_unification = new ebc_timer("2.03 Identity unification", thisAgent, soar_module::timer::one);
    identity_update = new ebc_timer("2.04 Identity transitive updates", thisAgent, soar_module::timer::one);
    variablization_lhs = new ebc_timer("2.05 Variablizing LHS", thisAgent, soar_module::timer::one);
    variablization_rhs = new ebc_timer("2.06 Variablizing RHS", thisAgent, soar_module::timer::one);
    merging = new ebc_timer("2.07 Merging Conditions", thisAgent, soar_module::timer::one);
    reorder = new ebc_timer("2.08 Validation and reordering", thisAgent, soar_module::timer::one);
    repair = new ebc_timer("2.09 Rule repair", thisAgent, soar_module::timer::one);
    reinstantiate = new ebc_timer("2.10 Reinstantiation", thisAgent, soar_module::timer::one);
    add_to_rete = new ebc_timer("2.11 Adding rule to RETE", thisAgent, soar_module::timer::one);
    clean_up = new ebc_timer("2.12 EBC Clean-Up", thisAgent, soar_module::timer::one);
    ebc_total = new ebc_timer("2.13 EBC Total", thisAgent, soar_module::timer::one);

    add(instantiation_creation);
    add(ebc_total);
    add(dependency_analysis);
    add(chunk_instantiation_creation);
    add(variablization_lhs);
    add(variablization_rhs);
    add(merging);
    add(repair);
    add(reorder);
    add(reinstantiate);
    add(add_to_rete);
    add(clean_up);
    add(identity_unification);
    add(identity_update);
}

ebc_timer_level_predicate::ebc_timer_level_predicate(agent* new_agent): soar_module::agent_predicate<soar_module::timer::timer_level>(new_agent) {}

bool ebc_timer_level_predicate::operator()(soar_module::timer::timer_level val)
{
    return (thisAgent->explanationBasedChunker->ebc_params->timers_cmd->get_value() == on);
}

//

ebc_timer::ebc_timer(const char* new_name, agent* new_agent, soar_module::timer::timer_level new_level): soar_module::timer(new_name, new_agent, new_level, new ebc_timer_level_predicate(new_agent)) {}

