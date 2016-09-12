/*
 * variablization_manager_merge.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"

#include "agent.h"
#include "dprint.h"
#include "ebc_settings.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "print.h"
#include "test.h"
#include "working_memory.h"

#include <assert.h>

void Explanation_Based_Chunker::print_current_built_rule(const char* pHeader)
{
    if (pHeader)
    {
        outputManager->printa_sf(thisAgent, "\n%s\n   ", pHeader);
    }
    if (m_prod_name)
    {
        outputManager->printa_sf(thisAgent, "\nsp {%y\n   ", m_prod_name);
    }
    if (m_vrblz_top)
    {
        print_condition_list(thisAgent, m_vrblz_top, 2, false);
    }
    if (m_rhs)
    {
        outputManager->printa(thisAgent, "\n  -->\n   ");
        print_action_list(thisAgent, m_rhs, 3, false);
        outputManager->printa_sf(thisAgent, "}\n\n");
    }
}

void Explanation_Based_Chunker::print_o_id_tables(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    print_ovar_to_o_id_map(mode);
    print_o_id_substitution_map(mode);
    print_o_id_to_ovar_debug_map(mode);

}

void Explanation_Based_Chunker::print_merge_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "            Merge Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (cond_merge_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    triple_merge_map::iterator          iter_id;
    sym_to_sym_to_cond_map::iterator    iter_attr;
    sym_to_cond_map::iterator           iter_value;

    for (iter_id = cond_merge_map->begin(); iter_id != cond_merge_map->end(); ++iter_id)
    {
        outputManager->printa_sf(thisAgent, "%y conditions: \n", iter_id->first);
        for (iter_attr = iter_id->second.begin(); iter_attr != iter_id->second.end(); ++iter_attr)
        {
            for (iter_value = iter_attr->second.begin(); iter_value != iter_attr->second.end(); ++iter_value)
            {
                outputManager->printa_sf(thisAgent, "   %l\n", iter_value->second);
            }
        }
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_ovar_to_o_id_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "     Instantiation Identity Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (instantiation_identities->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    inst_to_id_map::iterator iter_inst;
    sym_to_id_map::iterator iter_sym;

    for (iter_inst = instantiation_identities->begin(); iter_inst != instantiation_identities->end(); ++iter_inst)
    {
        outputManager->printa_sf(thisAgent, "Identities for i%u: \n", iter_inst->first);
        for (iter_sym = iter_inst->second.begin(); iter_sym != iter_inst->second.end(); ++iter_sym)
        {
            outputManager->printa_sf(thisAgent, "   %y = o%u\n", iter_sym->first, iter_sym->second);
        }
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}


void Explanation_Based_Chunker::print_o_id_substitution_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "     Identity to Identity Set Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (unification_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    id_to_id_map::iterator iter;

    for (iter = unification_map->begin(); iter != unification_map->end(); ++iter)
    {
        outputManager->printa_sf(thisAgent, "   o%u(%y) = o%u(%y)\n",
            iter->first, get_ovar_for_o_id(iter->first),
            iter->second, get_ovar_for_o_id(iter->second));
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_o_id_to_ovar_debug_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, " Identity to Original Sym Debug Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (id_to_rule_sym_debug_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    id_to_sym_map::iterator iter;

    for (iter = id_to_rule_sym_debug_map->begin(); iter != id_to_rule_sym_debug_map->end(); ++iter)
    {
        outputManager->printa_sf(thisAgent, "   o%u = %y\n",  iter->first, iter->second);
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}
void Explanation_Based_Chunker::print_attachment_points(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "   Attachment Points in Conditions\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (attachment_points->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    for (std::unordered_map< uint64_t, attachment_point* >::iterator it = (*attachment_points).begin(); it != (*attachment_points).end(); ++it)
    {
        outputManager->printa_sf(thisAgent, "%y(o%u) -> %s of %l\n", get_ovar_for_o_id(it->first), it->first, field_to_string(it->second->field), it->second->cond);
    }

}
void Explanation_Based_Chunker::print_constraints(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "    Relational Constraints List\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    if (constraints->empty())
    {
        outputManager->printa_sf(thisAgent, "NO CONSTRAINTS RECORDED\n");
    }
    for (std::list< constraint* >::iterator it = constraints->begin(); it != constraints->end(); ++it)
    {
        outputManager->printa_sf(thisAgent, "%t[%g]:   %t[%g]\n", (*it)->eq_test, (*it)->eq_test, (*it)->constraint_test, (*it)->constraint_test);
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}
/* -- A utility function to print all data stored in the variablization manager.  Used only for debugging -- */

void Explanation_Based_Chunker::print_variablization_table(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "== Identity Set -> Variablization ==\n");
    if (o_id_to_var_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }
    for (id_to_sym_map::iterator it = (*o_id_to_var_map).begin(); it != (*o_id_to_var_map).end(); ++it)
    {
        outputManager->printa_sf(thisAgent, "o%u -> %y\n", it->first, it->second);
    }
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_tables(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    print_variablization_table(mode);
    print_o_id_tables(mode);
}

void Explanation_Based_Chunker::print_chunking_summary()
{
    std::string tempString;

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 55);

    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa(thisAgent,    "                     Chunking Summary\n");
    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("When Soar will learn rules", ebc_params->chunk_in_states->get_string(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Incorporate operator selection knowledge", std::string(ebc_params->mechanism_OSK->get_value() ? "Yes" : "No"), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Interrupt after learning from watched rule", std::string(ebc_params->interrupt_on_chunk->get_value() ? "Yes" : "No"), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n\n", concatJustified("Interrupt after learning failure", std::string(ebc_params->interrupt_on_failure->get_value() ? "Yes" : "No"), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Chunks learned", std::to_string(thisAgent->explanationMemory->get_stat_succeeded()), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Chunks attempted", std::to_string(thisAgent->explanationMemory->get_stat_chunks_attempted()), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Justifications learned", std::to_string(thisAgent->explanationMemory->get_stat_justifications()), 55).c_str());

    if (ebc_settings[SETTING_EBC_ONLY] )
    {
        outputManager->printa_sf(thisAgent, "Only Learning In States\n");
        if (!chunky_problem_spaces)
        {
            outputManager->printa_sf(thisAgent, "No current learning states.\n");
        } else
        {
            for (cons* c = chunky_problem_spaces; c != NIL; c = c->rest)
            {
                thisAgent->outputManager->sprinta_sf(thisAgent, tempString, "%y\n", static_cast<Symbol*>(c->first));
                outputManager->printa_sf(thisAgent, tempString.c_str());
                tempString.clear();
            }
        }
    } else if (ebc_settings[SETTING_EBC_EXCEPT])
    {
        outputManager->printa_sf(thisAgent, "Learning in All States Except\n");
        if (!chunky_problem_spaces)
        {
            outputManager->printa_sf(thisAgent, "Currently learning in all states.\n");
        } else
        {
            for (cons* c = chunk_free_problem_spaces; c != NIL; c = c->rest)
            {
                thisAgent->outputManager->sprinta_sf(thisAgent, tempString, "%y\n", static_cast<Symbol*>(c->first));
                outputManager->printa_sf(thisAgent, tempString.c_str());
                tempString.clear();
            }
        }
    }
    outputManager->printa_sf(thisAgent, "\nFor a full list of EBC's sub-commands and settings:  chunk ?");
}


void Explanation_Based_Chunker::print_chunking_settings()
{
    std::string tempString;
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 55);
    outputManager->printa_sf(thisAgent, "========== Chunk Commands and Settings ============\n");
    outputManager->printa_sf(thisAgent, "? | help %-%-%s\n", "Print this help listing");
    outputManager->printa_sf(thisAgent, "history %-%-%s\n", "Print a bullet-point list of all chunking events");
    outputManager->printa_sf(thisAgent, "stats %-%-%s\n", "Print statistics on learning that has occurred");
    outputManager->printa_sf(thisAgent, "------------------- Settings ----------------------\n");
    outputManager->printa_sf(thisAgent, "%s | %s | %s | %s       %-%s\n",
        ebc_params->chunk_in_states->get_value() == ebc_always  ? "ALWAYS" : "always",
            ebc_params->chunk_in_states->get_value() == ebc_never ? "NEVER" : "never",
                ebc_params->chunk_in_states->get_value() == ebc_only ? "FLAGGED" : "flagged",
                    ebc_params->chunk_in_states->get_value() == ebc_except ? "ALL-EXCEPT" : "all-except",
        "When Soar will learn new rules");
    outputManager->printa_sf(thisAgent, "bottom-only                %-%s%-%s\n", capitalizeOnOff(ebc_params->bottom_level_only->get_value()), "Learn only from bottom sub-state");
    tempString = "[ ";
    tempString += ebc_params->naming_style->get_value() == ruleFormat ?  "numbered" : "NUMBERED";
    tempString += " | ";
    tempString += ebc_params->naming_style->get_value() == ruleFormat ?  "RULE" : "rule";
    tempString += "]";
    outputManager->printa_sf(thisAgent, "%s %-%s\n",
        concatJustified("naming-style", tempString, 51).c_str(),"Simple numeric chunk names or informational rule-based name");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-chunks", ebc_params->max_chunks->get_string(), 45).c_str(), "Maximum chunks that can be learned (per phase)");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-dupes", ebc_params->max_dupes->get_string(), 45).c_str(), "Maximum duplicate chunks (per rule, per phase)");
    outputManager->printa_sf(thisAgent, "------------------- Debugging ---------------------\n");
    outputManager->printa_sf(thisAgent, "interrupt                   %-%s%-%s\n", capitalizeOnOff(ebc_params->interrupt_on_chunk->get_value()), "Stop Soar after learning from a watched rule");
    outputManager->printa_sf(thisAgent, "interrupt-on-failure        %-%s%-%s\n", capitalizeOnOff(ebc_params->interrupt_on_failure->get_value()), "Stop Soar after learning failure");
    outputManager->printa_sf(thisAgent, "record-utility              %-%s%-%s\n", capitalizeOnOff(ebc_params->utility_mode->get_value()), "Record utility instead of firing");
    outputManager->printa_sf(thisAgent, "----------------- EBC Mechanisms ------------------\n");
    outputManager->printa_sf(thisAgent, "add-osk                     %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_OSK->get_value()), "Learn from operator selection knowledge");
    outputManager->printa_sf(thisAgent, "variablize-identity         %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_identity_analysis->get_value()), "Variablize symbols based on identity analysis");
    outputManager->printa_sf(thisAgent, "variablize-rhs-funcs        %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_variablize_rhs_funcs->get_value()), "Variablize and compose RHS functions");
    outputManager->printa_sf(thisAgent, "enforce-constraints         %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_constraints->get_value()), "Track and enforce transitive constraints");
    outputManager->printa_sf(thisAgent, "repair-rhs                  %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_repair_rhs->get_value()), "Repair rules with unconnected RHS actions");
    outputManager->printa_sf(thisAgent, "repair-lhs                  %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_repair_lhs->get_value()), "Repair rules with unconnected LHS conditions");
    outputManager->printa_sf(thisAgent, "repair-rhs-promotion        %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_promotion_tracking->get_value()), "Repair rules that augment previous results");
    outputManager->printa_sf(thisAgent, "merge                       %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_merge->get_value()), "Merge redundant conditions");
    outputManager->printa_sf(thisAgent, "user-singletons             %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_user_singletons->get_value()), "Unify identities using domain-specific singletons");
    outputManager->printa_sf(thisAgent, "--------- Correctness Guarantee Filters ------------\n");
    outputManager->printa_sf(thisAgent, "allow-local-negations       %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_missing_negative_reasoning->get_value()), "Used local negative reasoning");
    outputManager->printa_sf(thisAgent, "allow-missing-osk           %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_missing_OSK->get_value()), "Used operator selection rules to choose operator");
    outputManager->printa_sf(thisAgent, "allow-opaque                %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_opaque_knowledge->get_value()), "Used knowledge from opaque knowledge retrieval");
    outputManager->printa_sf(thisAgent, "allow-uncertain-operators   %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_probabilistic_operators->get_value()), "Used operators selected probabilistically");
    outputManager->printa_sf(thisAgent, "allow-multiple-prefs        %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_multiple_prefs->get_value()), "Tests a WME that has multiple reasons it exists");

}
