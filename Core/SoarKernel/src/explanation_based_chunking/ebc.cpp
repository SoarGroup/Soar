/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "assert.h"
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
#include "settings.h"
#include "test.h"
#include "dprint.h"

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
    ebc_params = new ebc_param_container(thisAgent, ebc_settings);

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
    ebc_settings[SETTING_EBC_LEARNING_ON] = ebc_settings[SETTING_EBC_LEARNING_ON];
    m_learning_on_for_instantiation = ebc_settings[SETTING_EBC_LEARNING_ON];

    chunkNameFormat = ruleFormat;
    max_chunks_reached = false;
    m_failure_type = ebc_success;

    chunk_name_prefix = make_memory_block_for_string(thisAgent, "chunk");
    justification_name_prefix = make_memory_block_for_string(thisAgent, "justify");

    local_singleton_superstate_identity = NULL;


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

    free_memory_block_for_string(thisAgent, chunk_name_prefix);
    free_memory_block_for_string(thisAgent, justification_name_prefix);

}
void Explanation_Based_Chunker::update_learning_on()
{
    ebc_settings[SETTING_EBC_LEARNING_ON] = ebc_settings[SETTING_EBC_LEARNING_ON];
}

void Explanation_Based_Chunker::set_chunk_name_prefix(const char* pChunk_name_prefix)
{
    free_memory_block_for_string(thisAgent, chunk_name_prefix);
    chunk_name_prefix = make_memory_block_for_string(thisAgent, pChunk_name_prefix);
};

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
    uint64_t rule_number;
    chunkNameFormats chunkNameFormat = Get_Chunk_Name_Format();

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
                  thisAgent->d_cycle_count << "*" << lImpasseName << "*" << rule_number;

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
                if ((strstr(inst->prod_name->sc->name, rule_prefix) == inst->prod_name->sc->name) &&
                    (strstr(inst->prod_name->sc->name, "-multi") == inst->prod_name->sc->name))
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
            lName << "*t" << thisAgent->d_cycle_count << "-" << rule_number;
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
