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
    if (m_lhs)
    {
        print_condition_list(thisAgent, m_lhs, 2, false);
    }
    if (m_rhs)
    {
        outputManager->printa(thisAgent, "\n  -->\n   ");
        print_action_list(thisAgent, m_rhs, 3, false);
        outputManager->printa_sf(thisAgent, "}\n\n");
    }
}

void Explanation_Based_Chunker::print_identity_tables(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_trace_enabled(mode)) return;
    print_instantiation_identities_map(mode);
    print_unification_map(mode);
}

void Explanation_Based_Chunker::print_merge_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_trace_enabled(mode)) return;
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

void Explanation_Based_Chunker::print_instantiation_identities_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_trace_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "     Instantiation Identity Map\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (instantiation_identities->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    sym_to_id_map::iterator iter_sym;

    for (auto iter_sym = instantiation_identities->begin(); iter_sym != instantiation_identities->end(); ++iter_sym)
    {
        outputManager->printa_sf(thisAgent, "   %y = o%u\n", iter_sym->first, iter_sym->second);
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}


void Explanation_Based_Chunker::print_unification_map(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_trace_enabled(mode)) return;
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
        outputManager->printa_sf(thisAgent, "   %u = %u\n", iter->first, iter->second);
    }

    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_attachment_points(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_trace_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "   Attachment Points in Conditions\n");
    outputManager->printa_sf(thisAgent, "------------------------------------\n");

    if (attachment_points->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }

    for (std::unordered_map< uint64_t, attachment_point* >::iterator it = (*attachment_points).begin(); it != (*attachment_points).end(); ++it)
    {
        outputManager->printa_sf(thisAgent, "%u -> %s of %l\n", it->first, field_to_string(it->second->field), it->second->cond);
    }
}

void Explanation_Based_Chunker::print_constraints(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_trace_enabled(mode)) return;
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
    if (!Output_Manager::Get_OM().is_trace_enabled(mode)) return;
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
    outputManager->printa_sf(thisAgent, "== Identity Set -> Variablization ==\n");
    if (identity_to_var_map->size() == 0)
    {
        outputManager->printa_sf(thisAgent, "EMPTY MAP\n");
    }
    for (auto it = (*identity_to_var_map).begin(); it != (*identity_to_var_map).end(); ++it)
    {
        outputManager->printa_sf(thisAgent, "%u -> %y (%u)\n", it->first, it->second->variable_sym, it->second->identity);
    }
    outputManager->printa_sf(thisAgent, "------------------------------------\n");
}

void Explanation_Based_Chunker::print_tables(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_trace_enabled(mode)) return;
    print_variablization_table(mode);
    print_identity_tables(mode);
}

void Explanation_Based_Chunker::print_chunking_summary()
{
    std::string tempString;

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 55);

    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa(thisAgent,    "           Explanation-Based Chunking Summary\n");
    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("When Soar will learn rules", ebc_params->chunk_in_states->get_string(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Incorporate operator selection knowledge", std::string(ebc_params->mechanism_OSK->get_value() ? "Yes" : "No"), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Interrupt after learning any rule", std::string(ebc_params->interrupt_on_chunk->get_value() ? "Yes" : "No"), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Interrupt after learning from watched rule", std::string(ebc_params->interrupt_on_watched->get_value() ? "Yes" : "No"), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n\n", concatJustified("Interrupt after learning failure", std::string(ebc_params->interrupt_on_warning->get_value() ? "Yes" : "No"), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Chunks learned", std::to_string(thisAgent->explanationMemory->get_stat_succeeded()), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Justifications learned", std::to_string(thisAgent->explanationMemory->get_stat_justifications()), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Substates analyzed", std::to_string(thisAgent->explanationMemory->get_stat_chunks_attempted()), 55).c_str());

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
    outputManager->printa_sf(thisAgent, "\nTry 'chunk ?' to learn more about chunking's sub-commands and settings.\n"
                    "For a detailed article about the chunk command, use 'help chunk'.\n");
}


void Explanation_Based_Chunker::print_chunking_settings()
{
    std::string tempString;
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 55);
    outputManager->printa_sf(thisAgent, "========== Chunk Commands and Settings ============\n");
    outputManager->printa_sf(thisAgent, "? | help %-%-%s\n", "Print this help listing");
//    outputManager->printa_sf(thisAgent, "history %-%-%s\n", "Print a bullet-point list of all chunking events");
    outputManager->printa_sf(thisAgent, "stats %-%-%s\n", "Print statistics on learning that has occurred");
    outputManager->printa_sf(thisAgent, "------------------- Settings ----------------------\n");
    outputManager->printa_sf(thisAgent, "%s | %s | %s | %s                   %-%s\n",
        ebc_params->chunk_in_states->get_value() == ebc_always  ? "ALWAYS" : "always",
            ebc_params->chunk_in_states->get_value() == ebc_never ? "NEVER" : "never",
                ebc_params->chunk_in_states->get_value() == ebc_only ? "ONLY" : "only",
                    ebc_params->chunk_in_states->get_value() == ebc_except ? "EXCEPT" : "except",
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
    outputManager->printa_sf(thisAgent, "interrupt                   %-%s%-%s\n", capitalizeOnOff(ebc_params->interrupt_on_chunk->get_value()), "Stop Soar after learning from any rule");
    outputManager->printa_sf(thisAgent, "explain-interrupt        %-%s%-%s\n", capitalizeOnOff(ebc_params->interrupt_on_watched->get_value()), "Stop Soar after learning rule watched by explainer");
    outputManager->printa_sf(thisAgent, "warning-interrupt        %-%s%-%s\n", capitalizeOnOff(ebc_params->interrupt_on_warning->get_value()), "Stop Soar after detecting learning issue");
//    outputManager->printa_sf(thisAgent, "\n*record-utility (disabled)   %-%s%-%s\n", capitalizeOnOff(ebc_params->utility_mode->get_value()), "Record utility instead of firing");
    outputManager->printa_sf(thisAgent, "------------------- Fine Tune ---------------------\n");
    outputManager->printa_sf(thisAgent, "singleton %-%-%s\n", "Print all WME singletons");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("singleton", "<type> <attribute> <type>", 50).c_str(), "Add a WME singleton pattern");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("singleton -r", "<type> <attribute> <type>", 50).c_str(), "Remove a WME singleton pattern");
    outputManager->printa_sf(thisAgent, "----------------- EBC Mechanisms ------------------\n");
    outputManager->printa_sf(thisAgent, "add-osk                     %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_OSK->get_value()), "Learn from operator selection knowledge");
    outputManager->printa_sf(thisAgent, "dont-add-ltm-links          %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_no_ltm_links->get_value()), "Don't create LTM links in chunks");
    outputManager->printa_sf(thisAgent, "merge                       %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_merge->get_value()), "Merge redundant conditions");
    outputManager->printa_sf(thisAgent, "lhs-repair                  %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_repair_lhs->get_value()), "Add grounding conditions for unconnected LHS identifiers");
    outputManager->printa_sf(thisAgent, "rhs-repair                  %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_repair_rhs->get_value()), "Add grounding conditions for unconnected RHS identifiers");
    outputManager->printa_sf(thisAgent, "*user-singletons            %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_user_singletons->get_value()), "Unify identities using domain-specific singletons");
//    outputManager->printa_sf(thisAgent, "*variablize-identity (disabled) %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_identity_analysis->get_value()), "Variablize symbols based on identity analysis");
//    outputManager->printa_sf(thisAgent, "*variablize-rhs-funcs (disabled) %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_variablize_rhs_funcs->get_value()), "Variablize and compose RHS functions");
//    outputManager->printa_sf(thisAgent, "*enforce-constraints (disabled) %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_constraints->get_value()), "Track and enforce transitive constraints");
//    outputManager->printa_sf(thisAgent, "*merge (disabled) %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_merge->get_value()), "Merge redundant conditions");
    outputManager->printa_sf(thisAgent, "---------- Correctness Guarantee Filters ----------\n");
    outputManager->printa_sf(thisAgent, "allow-local-negations       %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_missing_negative_reasoning->get_value()), "Allow rules to form that used local negative reasoning");
    outputManager->printa_sf(thisAgent, "*allow-opaque               %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_opaque_knowledge->get_value()), "Used knowledge from LTM recall");
//    outputManager->printa_sf(thisAgent, "*allow-missing-osk (disabled) %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_missing_OSK->get_value()), "Used operator selection rules to choose operator");
//    outputManager->printa_sf(thisAgent, "*allow-uncertain-operators (disabled) %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_probabilistic_operators->get_value()), "Used operators selected probabilistically");
//    outputManager->printa_sf(thisAgent, "*allow-conflated-reasoning (disabled) %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_conflated_reasoning->get_value()), "Tests a WME that has multiple reasons it exists");
    outputManager->printa_sf(thisAgent, "------------- Experimental Settings ---------------\n");
    outputManager->printa_sf(thisAgent, "unify-all             %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_unify_all->get_value()), "Unify all conditions that match same superstate WMEs");
    outputManager->printa_sf(thisAgent, "repair-justifications %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_reorder_justifications->get_value()), "Re-order justifications");

    outputManager->printa_sf(thisAgent, "\nTo change a setting: %-%- chunk <setting> [<value>]\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of these settings:  %-%-help chunk\n");

}

const char* Explanation_Based_Chunker::singletonTypeToString(singleton_element_type pType)
{
    switch (pType) {
        case ebc_any:
            return "<any>";
        case ebc_state:
            return "<state>";
        case ebc_operator:
            return "<operator>";
        case ebc_identifier:
            return "<identifier>";
        case ebc_constant:
            return "<constant>";
        default:
            return "INVALID";
    }
}

void Explanation_Based_Chunker::print_singleton_summary()
{
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 55);
    outputManager->printa(thisAgent,    "==== Singleton WME Unification Patterns ====\n");
    outputManager->printa(thisAgent,    "----------------- Local --------------------\n");
    outputManager->printa(thisAgent,    "   (<state> ^superstate <state>)\n");
    outputManager->printa(thisAgent,    "\n-------------- Super-state -----------------\n");
    outputManager->printa(thisAgent,    "   (<state> ^superstate <any>)\n");
    outputManager->printa_sf(thisAgent, "   (<state> ^operator   <operator>)             %-(unless condition only tests operator proposal)\n");
    outputManager->printa(thisAgent,    "   (<state> ^type       <constant>)\n");
    outputManager->printa(thisAgent,    "   (<state> ^impasse    <constant>)\n");
    outputManager->printa(thisAgent,    "   (<state> ^smem       <identifier>)\n");
    outputManager->printa(thisAgent,    "   (<state> ^epmem      <identifier>)\n");
    outputManager->printa(thisAgent,    "   ---------- user-defined ----------\n");
    if (singletons->empty())
    {
        thisAgent->outputManager->printa(thisAgent, "   None.\n");
    }
    else
    {
        Symbol* lSym;
        for (auto it = singletons->begin(); it != singletons->end(); it++)
        {
            lSym = static_cast<Symbol*>(*it);
            thisAgent->outputManager->printa_sf(thisAgent, "   (%s ^%y %s)\n", singletonTypeToString(lSym->sc->singleton.id_type), lSym, singletonTypeToString(lSym->sc->singleton.value_type));
        }
    }
    outputManager->printa(thisAgent, "\n\n"
        "To add a new pattern:    chunk singleton    <type> attribute <type>\n"
        "To remove a pattern:     chunk singleton -r <type> attribute <type>\n\n"
        "   Valid types:          [ any | constant | identifier | operator | state ]  \n");
}
