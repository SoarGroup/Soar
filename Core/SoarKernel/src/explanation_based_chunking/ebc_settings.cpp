#include "ebc_settings.h"

#include "agent.h"
#include "ebc.h"
#include "explanation_memory.h"
#include "output_manager.h"

#define setting_on(s) pEBC_settings[s] ? on : off

ebc_param_container::ebc_param_container(agent* new_agent, bool pEBC_settings[], uint64_t& pMaxChunks, uint64_t& pMaxDupes, uint64_t& pConfidenceThreshold): soar_module::param_container(new_agent) // <- CBC edit
{

    /* Set up the settings array that is used for quick access */
    pEBC_settings[SETTING_EBC_LEARNING_ON] = false;
    pEBC_settings[SETTING_EBC_ALWAYS] = false;
    pEBC_settings[SETTING_EBC_NEVER] = true;
    pEBC_settings[SETTING_EBC_ONLY] = false;
    pEBC_settings[SETTING_EBC_EXCEPT] = false;
    pEBC_settings[SETTING_EBC_BOTTOM_ONLY] = false;
    pEBC_settings[SETTING_EBC_INTERRUPT] = false;
    pEBC_settings[SETTING_EBC_INTERRUPT_WARNING] = false;
    pEBC_settings[SETTING_EBC_INTERRUPT_WATCHED] = false;
    pEBC_settings[SETTING_EBC_ADD_OSK] = true;
    pEBC_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS] = true;
    pEBC_settings[SETTING_EBC_ALLOW_OPAQUE] = true;
    pEBC_settings[SETTING_EBC_ADD_LTM_LINKS] = false;

    pMaxChunks = 50;
    pMaxDupes = 3;
    pConfidenceThreshold = 3;	// CBC

    chunk_in_states = new soar_module::constant_param<EBCLearnChoices>("learn", ebc_never, new soar_module::f_predicate<EBCLearnChoices>());
    chunk_in_states->add_mapping(ebc_always, "enabled");
    chunk_in_states->add_mapping(ebc_always, "on");
    chunk_in_states->add_mapping(ebc_always, "all");
    chunk_in_states->add_mapping(ebc_always, "always");
    chunk_in_states->add_mapping(ebc_never, "disabled");
    chunk_in_states->add_mapping(ebc_never, "off");
    chunk_in_states->add_mapping(ebc_never, "none");
    chunk_in_states->add_mapping(ebc_never, "never");
    chunk_in_states->add_mapping(ebc_only, "flagged");
    chunk_in_states->add_mapping(ebc_only, "only");
    chunk_in_states->add_mapping(ebc_except, "unflagged");
    chunk_in_states->add_mapping(ebc_except, "except");
    add(chunk_in_states);

    naming_style = new soar_module::constant_param<chunkNameFormats>("naming-style", ruleFormat, new soar_module::f_predicate<chunkNameFormats>());
    naming_style->add_mapping(ruleFormat, "rule");
    naming_style->add_mapping(numberedFormat, "numbered");
    add(naming_style);

    always_cmd = new soar_module::boolean_param("always", on, new soar_module::f_predicate<boolean>());
    add(always_cmd);
    never_cmd = new soar_module::boolean_param("never", on, new soar_module::f_predicate<boolean>());
    add(never_cmd);
    flagged_cmd = new soar_module::boolean_param("only", on, new soar_module::f_predicate<boolean>());
    add(flagged_cmd);
    unflagged_cmd = new soar_module::boolean_param("except", on, new soar_module::f_predicate<boolean>());
    add(unflagged_cmd);

    stats_cmd = new soar_module::boolean_param("stats", on, new soar_module::f_predicate<boolean>());
    add(stats_cmd);
    help_cmd = new soar_module::boolean_param("help", on, new soar_module::f_predicate<boolean>());
    add(help_cmd);
    qhelp_cmd = new soar_module::boolean_param("?", on, new soar_module::f_predicate<boolean>());
    add(qhelp_cmd);

    singleton = new soar_module::boolean_param("singleton", on, new soar_module::f_predicate<boolean>());
    add(singleton);

    element_type = new soar_module::constant_param<singleton_element_type>("zxElementType", ebc_any, new soar_module::f_predicate<singleton_element_type>());
    element_type->add_mapping(ebc_identifier, "identifier");
    element_type->add_mapping(ebc_state, "state");
    element_type->add_mapping(ebc_operator, "operator");
    element_type->add_mapping(ebc_constant, "constant");
    element_type->add_mapping(ebc_any, "any");
    add(element_type);

    max_chunks = new soar_module::integer_param("max-chunks", pMaxChunks, new soar_module::gt_predicate<int64_t>(1, true), new soar_module::f_predicate<int64_t>());
    add(max_chunks);
    max_dupes = new soar_module::integer_param("max-dupes", pMaxDupes, new soar_module::gt_predicate<int64_t>(1, true), new soar_module::f_predicate<int64_t>());
    add(max_dupes);
    confidence_threshold = new soar_module::integer_param("confidence-threshold", pConfidenceThreshold, new soar_module::gt_predicate<int64_t>(1, true), new soar_module::f_predicate<int64_t>()); // CBC
    add(confidence_threshold);

    confidence_function = new soar_module::constant_param<chunkConfidenceFunctions>("confidence-function", cbc_constant, new soar_module::f_predicate<chunkConfidenceFunctions>());
    confidence_function->add_mapping(cbc_constant, "constant");
    confidence_function->add_mapping(cbc_lin_incr, "lin-increase");
    confidence_function->add_mapping(cbc_lin_decr, "lin-decrease");
    confidence_function->add_mapping(cbc_exp_incr, "exp-increase");
    confidence_function->add_mapping(cbc_exp_decr, "exp-decrease");
    add(confidence_function);

    bottom_level_only = new soar_module::boolean_param("bottom-only", setting_on(SETTING_EBC_BOTTOM_ONLY), new soar_module::f_predicate<boolean>());
    add(bottom_level_only);
    interrupt_on_chunk = new soar_module::boolean_param("interrupt", setting_on(SETTING_EBC_INTERRUPT), new soar_module::f_predicate<boolean>());
    add(interrupt_on_chunk);
    interrupt_on_warning = new soar_module::boolean_param("warning-interrupt", setting_on(SETTING_EBC_INTERRUPT_WARNING), new soar_module::f_predicate<boolean>());
    add(interrupt_on_warning);
    interrupt_on_watched = new soar_module::boolean_param("explain-interrupt", setting_on(SETTING_EBC_INTERRUPT_WATCHED), new soar_module::f_predicate<boolean>());
    add(interrupt_on_watched);

    // mechanisms
    mechanism_add_OSK = new soar_module::boolean_param("add-osk", setting_on(SETTING_EBC_ADD_OSK), new soar_module::f_predicate<boolean>());
    add(mechanism_add_OSK);

    mechanism_add_ltm_links = new soar_module::boolean_param("add-ltm-links", setting_on(SETTING_EBC_ADD_LTM_LINKS), new soar_module::f_predicate<boolean>());
    add(mechanism_add_ltm_links);

    allow_missing_negative_reasoning = new soar_module::boolean_param("allow-local-negations", setting_on(SETTING_EBC_ALLOW_LOCAL_NEGATIONS), new soar_module::f_predicate<boolean>());
    add(allow_missing_negative_reasoning);
    allow_opaque_knowledge = new soar_module::boolean_param("allow-opaque", setting_on(SETTING_EBC_ALLOW_OPAQUE), new soar_module::f_predicate<boolean>());
    add(allow_opaque_knowledge);
}

void ebc_param_container::update_ebc_settings(agent* thisAgent, soar_module::boolean_param* pChangedParam, soar_module::integer_param* pChangedIntParam)
{
    if (!pChangedParam)
    {
        if (!pChangedIntParam)
        {
            if (chunk_in_states->get_value() == ebc_always)
            {
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ALWAYS] = true;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_NEVER] = false;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ONLY] = false;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
            }
            else if (chunk_in_states->get_value() == ebc_never)
            {
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ALWAYS] = false;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_NEVER] = true;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ONLY] = false;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
            }
            else if (chunk_in_states->get_value() == ebc_only)
            {
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ALWAYS] = false;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_NEVER] = false;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ONLY] = true;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
            }
            else if (chunk_in_states->get_value() == ebc_except)
            {
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ALWAYS] = false;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_NEVER] = false;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ONLY] = false;
                thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_EXCEPT] = true;
            }
            thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = (thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_NEVER] == false);
        } else {

            if (pChangedIntParam == max_chunks)
            {
                thisAgent->explanationBasedChunker->max_chunks = pChangedIntParam->get_value();
            }
            else if (pChangedIntParam == max_dupes)
            {
                thisAgent->explanationBasedChunker->max_dupes = pChangedIntParam->get_value();
            }
            else if (pChangedIntParam == confidence_threshold)
            {
                thisAgent->explanationBasedChunker->confidence_threshold = pChangedIntParam->get_value();		// CBC
            }
        }
    }
    else if (pChangedParam == bottom_level_only)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_BOTTOM_ONLY] = pChangedParam->get_value();
    }
    else if (pChangedParam == interrupt_on_chunk)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_INTERRUPT] = pChangedParam->get_value();
    }
    else if (pChangedParam == interrupt_on_warning)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_INTERRUPT_WARNING] = pChangedParam->get_value();
    }
    else if (pChangedParam == interrupt_on_watched)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_INTERRUPT_WATCHED] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_add_OSK)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ADD_OSK] = pChangedParam->get_value();
    }
    else if (pChangedParam == mechanism_add_ltm_links)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ADD_LTM_LINKS] = pChangedParam->get_value();
    }
    else if (pChangedParam == allow_missing_negative_reasoning)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS] = pChangedParam->get_value();
    }
    else if (pChangedParam == always_cmd)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ALWAYS] = true;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_NEVER] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ONLY] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = true;
        chunk_in_states->set_value(ebc_always);
        thisAgent->outputManager->printa_sf(thisAgent, "Learns rules in states: %s\n", chunk_in_states->get_string().c_str());
    }
    else if (pChangedParam == never_cmd)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ALWAYS] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_NEVER] = true;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ONLY] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = false;
        chunk_in_states->set_value(ebc_never);
        thisAgent->outputManager->printa_sf(thisAgent, "Learns rules in states: %s\n", chunk_in_states->get_string().c_str());
    }
    else if (pChangedParam == flagged_cmd)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ALWAYS] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_NEVER] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ONLY] = true;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = true;
        chunk_in_states->set_value(ebc_only);
        thisAgent->outputManager->printa_sf(thisAgent, "Learns rules in states: %s\n", chunk_in_states->get_string().c_str());
    }
    else if (pChangedParam == unflagged_cmd)
    {
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ALWAYS] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_NEVER] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ONLY] = false;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_EXCEPT] = true;
        thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = true;
        chunk_in_states->set_value(ebc_except);
        thisAgent->outputManager->printa_sf(thisAgent, "Learns rules in states: %s\n", chunk_in_states->get_string().c_str());
    }
}

void ebc_param_container::update_params(bool pEBC_settings[])
{
    if (pEBC_settings[SETTING_EBC_ALWAYS])
    {
        chunk_in_states->set_value(ebc_always);
    }
    else if (pEBC_settings[SETTING_EBC_NEVER])
    {
        chunk_in_states->set_value(ebc_never);
    }
    else if (pEBC_settings[SETTING_EBC_ONLY])
    {
        chunk_in_states->set_value(ebc_only);
    }
    else if (pEBC_settings[SETTING_EBC_EXCEPT])
    {
        chunk_in_states->set_value(ebc_except);
    }
    bottom_level_only->set_value(pEBC_settings[SETTING_EBC_BOTTOM_ONLY] ? on : off);
    interrupt_on_chunk->set_value(pEBC_settings[SETTING_EBC_INTERRUPT] ? on : off);
    interrupt_on_warning->set_value(pEBC_settings[SETTING_EBC_INTERRUPT_WARNING] ? on : off);

    mechanism_add_OSK->set_value(pEBC_settings[SETTING_EBC_ADD_OSK] ? on : off);
    mechanism_add_ltm_links->set_value(pEBC_settings[SETTING_EBC_ADD_LTM_LINKS] ? on : off);
    allow_missing_negative_reasoning->set_value(pEBC_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS] ? on : off);
}
void Explanation_Based_Chunker::print_chunking_summary()
{
    std::string tempString;

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 55);

    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa(thisAgent,    "           Explanation-Based Chunking Summary\n");
    outputManager->printa(thisAgent,    "=======================================================\n");
//    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Number of chunks:", std::to_string(thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE]).c_str(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("When Soar will learn rules", ebc_params->chunk_in_states->get_string().c_str(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Incorporate operator selection knowledge", std::string(ebc_params->mechanism_add_OSK->get_value() ? "Yes" : "No"), 55).c_str());
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
    outputManager->printa(thisAgent,    "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "\nTry 'chunk ?' to learn more about chunking's sub-commands and settings.\n"
                    "For a detailed article about the chunk command, use 'help chunk'.\n");
}


void Explanation_Based_Chunker::print_chunking_settings()
{
    std::string tempString;
    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 40);
    outputManager->set_column_indent(1, 55);
    outputManager->printa(thisAgent,    "===================================================\n");
    outputManager->printa(thisAgent,    "           Chunk Commands and Settings\n");
    outputManager->printa(thisAgent,    "===================================================\n");
    outputManager->printa_sf(thisAgent, "chunk ? | help %-%-%s\n", "Print all EBC settings");
    outputManager->printa_sf(thisAgent, "chunk stats %-%-%s\n", "Print statistics on learning that has occurred");
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
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-chunks", ebc_params->max_chunks->get_string().c_str(), 45).c_str(), "Maximum chunks that can be learned (per phase)");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("max-dupes", ebc_params->max_dupes->get_string().c_str(), 45).c_str(), "Maximum duplicate chunks (per rule, per phase)");
    // CBC:
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("confidence-threshold", ebc_params->confidence_threshold->get_string().c_str(), 45).c_str(), "Number of times to generate chunk before it is added");
    tempString = "[ ";
        tempString += ebc_params->confidence_function->get_value() == cbc_constant ?  "CONSTANT": "constant";
        tempString += " | ";
        tempString += ebc_params->confidence_function->get_value() == cbc_lin_incr ?  "LIN-INCREASE" : "lin-increase";
        tempString += " | \n    ";
        tempString += ebc_params->confidence_function->get_value() == cbc_lin_decr ?  "LIN-DECREASE" : "lin-decrease";
        tempString += " | ";
        tempString += ebc_params->confidence_function->get_value() == cbc_exp_incr ?  "EXP-INCREASE" : "exp-increase";
        tempString += " | ";
        tempString += ebc_params->confidence_function->get_value() == cbc_exp_decr ?  "EXP-DECREASE" : "exp-decrease";
        tempString += "]";
    outputManager->printa_sf(thisAgent, "%s %-%s\n",
        concatJustified("confidence-function", tempString, 51).c_str(),"Function for determining confidence threshold for chunk.");
    outputManager->printa_sf(thisAgent, "------------------- Debugging ---------------------\n");
    outputManager->printa_sf(thisAgent, "interrupt                  %-%s%-%s\n", capitalizeOnOff(ebc_params->interrupt_on_chunk->get_value()), "Stop Soar after learning from any rule");
    outputManager->printa_sf(thisAgent, "explain-interrupt          %-%s%-%s\n", capitalizeOnOff(ebc_params->interrupt_on_watched->get_value()), "Stop Soar after learning rule watched by explainer");
    outputManager->printa_sf(thisAgent, "warning-interrupt          %-%s%-%s\n", capitalizeOnOff(ebc_params->interrupt_on_warning->get_value()), "Stop Soar after detecting learning issue");
    outputManager->printa_sf(thisAgent, "------------------- Fine Tune ---------------------\n");
    outputManager->printa_sf(thisAgent, "singleton %-%-%s\n", "Print all WME singletons");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("singleton", "<type> <attribute> <type>", 50).c_str(), "Add a WME singleton pattern");
    outputManager->printa_sf(thisAgent, "%s   %-%s\n", concatJustified("singleton -r", "<type> <attribute> <type>", 50).c_str(), "Remove a WME singleton pattern");
    outputManager->printa_sf(thisAgent, "----------------- EBC Mechanisms ------------------\n");
    outputManager->printa_sf(thisAgent, "add-ltm-links              %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_add_ltm_links->get_value()), "Recreate LTM links in original results");
    outputManager->printa_sf(thisAgent, "add-osk                    %-%s%-%s\n", capitalizeOnOff(ebc_params->mechanism_add_OSK->get_value()), "Incorporate operator selection knowledge");
    outputManager->printa_sf(thisAgent, "---------- Correctness Guarantee Filters ----------%-%s\n", "Allow rules to form that...");
    outputManager->printa_sf(thisAgent, "allow-local-negations          %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_missing_negative_reasoning->get_value()), "...used local negative reasoning");
    outputManager->printa_sf(thisAgent, "allow-opaque                   %-%s%-%s\n", capitalizeOnOff(ebc_params->allow_opaque_knowledge->get_value()), "...used knowledge from a LTM recall");
    outputManager->printa_sf(thisAgent, "---------------------------------------------------\n");

    outputManager->printa_sf(thisAgent, "\nTo change a setting: %-%- chunk <setting> [<value>]\n");
    outputManager->printa_sf(thisAgent, "For a detailed explanation of these settings:  %-%-help chunk\n");

}


