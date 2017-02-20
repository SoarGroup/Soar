#include "identity_record.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "ebc.h"
#include "explanation_memory.h"
#include "instantiation_record.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"

void identity_record::init(agent* myAgent)
{
    thisAgent = myAgent;
    idset_to_var_map = new id_to_sym_map();
    identities_in_chunk = new id_set();
    instantiation_mappings = new inst_identities_map();
    original_ebc_mappings = NULL;
}

void identity_record::clean_up()
{
    if (original_ebc_mappings) delete original_ebc_mappings;
    if (idset_to_var_map)
    {
        Symbol* lSym;
        for (auto it = idset_to_var_map->begin(); it != idset_to_var_map->end(); ++it)
        {
            lSym = it->second;
            if (lSym) thisAgent->symbolManager->symbol_remove_ref(&lSym);
        }
        delete idset_to_var_map;
    }
    if (identities_in_chunk) delete identities_in_chunk;

    clear_mappings();
    delete instantiation_mappings;
}


void generate_identity_sets_from_test(agent* thisAgent, test pTest, uint64_t pInstID, id_set* pID_Set, id_to_sym_map* pID_Set_Map)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
        pTest = pTest->eq_test;
    }
    if (test_has_referent(pTest)) {
        if (pTest->identity_set)
        {
            if (pID_Set->find(pTest->identity_set) == pID_Set->end())
            {
                pID_Set->insert(pTest->identity_set);
                pID_Set_Map->insert({pTest->identity_set, pTest->data.referent});
                thisAgent->symbolManager->symbol_add_ref(pTest->data.referent);
                //thisAgent->explanationMemory->add_identity_set_mapping(pInstID, IDS_base_instantiation, pTest->identity_set, lNewIDSet->identity);
            }
        }
    }
}

void generate_identity_sets_from_conditions(agent* thisAgent, condition* lhs, uint64_t pInstID, id_set* pID_Set, id_to_sym_map* pID_Set_Map)
{
    for (condition* lCond = lhs; lCond != NULL; lCond = lCond->next)
    {
        if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            generate_identity_sets_from_conditions(thisAgent, lCond->data.ncc.top, pInstID, pID_Set, pID_Set_Map);
        } else {
            generate_identity_sets_from_test(thisAgent, lCond->data.tests.id_test, pInstID, pID_Set, pID_Set_Map);
            generate_identity_sets_from_test(thisAgent, lCond->data.tests.attr_test, pInstID, pID_Set, pID_Set_Map);
            generate_identity_sets_from_test(thisAgent, lCond->data.tests.value_test, pInstID, pID_Set, pID_Set_Map);
        }
    }
}

void identity_record::generate_identity_sets(uint64_t pInstID, condition* lhs)
{
    /* Generate identity sets and add mappings for all conditions in chunk */

    dprint(DT_EXPLAIN_IDENTITIES, "Building identity mappings based on conditions of chunk...\n");
    thisAgent->explanationMemory->reset_identity_set_counter();
    generate_identity_sets_from_conditions(thisAgent, lhs, pInstID, identities_in_chunk, idset_to_var_map);
}

void identity_record::print_identity_mappings_for_instantiation(instantiation_record* pInstRecord)
{
    id_set* identities_in_inst = pInstRecord->get_lhs_identities();

}

void identity_record::print_identities_in_chunk()
{
    thisAgent->outputManager->printa_sf(thisAgent, "\nVariable identities (%u): ", identities_in_chunk->size());
    for (auto it = identities_in_chunk->begin(); it != identities_in_chunk->end(); ++it)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%u ", (*it));
    }
    thisAgent->outputManager->printa(thisAgent, "\n");
}

void identity_record::print_original_ebc_mappings()
{
    thisAgent->outputManager->set_column_indent(0, 6);
    thisAgent->outputManager->set_column_indent(1, 26);
    thisAgent->outputManager->set_column_indent(2, 31);

    thisAgent->outputManager->printa_sf(thisAgent, "\nOriginal EBC Mappings (%u):\n", original_ebc_mappings->size());
    thisAgent->outputManager->printa_sf(thisAgent, "ID %-Original %-Set %-Final\n\n");

    std::unordered_map< uint64_t, uint64_t >::iterator iter;
    for (iter = original_ebc_mappings->begin(); iter != original_ebc_mappings->end(); ++iter)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%u%- %-%u%-\n", iter->first, iter->second);
    }
}

void identity_record::add_identity_mapping(uint64_t pI_ID, IDSet_Mapping_Type pType,
                                           uint64_t pFromID,  uint64_t pToID)
{
    identity_mapping_list* lInstMappingList;

    auto lIterInst = instantiation_mappings->find(pI_ID);
    if (lIterInst == instantiation_mappings->end())
    {
        lInstMappingList = new identity_mapping_list();
        (*instantiation_mappings)[pI_ID] = lInstMappingList;
    } else {
        lInstMappingList = lIterInst->second;
    }
    identity_mapping* lMapping;
    thisAgent->memoryManager->allocate_with_pool(MP_identity_mapping, &lMapping);
    lMapping->from_identity = pFromID;
    lMapping->to_identity = pToID;
    lMapping->mappingType = pType;
    lInstMappingList->push_back(lMapping);
}

void identity_record::print_mappings()
{
    thisAgent->outputManager->set_column_indent(0, 4);
    thisAgent->outputManager->set_column_indent(1, 14);
    thisAgent->outputManager->set_column_indent(2, 20);
    thisAgent->outputManager->set_column_indent(3, 37);

    thisAgent->outputManager->printa(thisAgent,
        "\n-== NULL Identity Set ==-\n\n"
        "The following variable identities map to the null identity set and will not be\ngeneralized:");
    for (auto it = idset_to_var_map->begin(); it != idset_to_var_map->end(); ++it)
    {
        if (!it->second)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%u ", it->first);
        }
    }
    thisAgent->outputManager->printa_sf(thisAgent, "\n\n-== How variable identities map to identity sets ==-\n\n");

    Output_Manager* outputManager = thisAgent->outputManager;
    bool printHeader = true;
    for (auto it = instantiation_mappings->begin(); it != instantiation_mappings->end(); ++it)
    {
        if (it->second->size() == 0)
        {
            outputManager->printa_sf(thisAgent, "No identity set mappings for instantiation %u.\n", it->first);
        } else {
            outputManager->printa_sf(thisAgent, "Instantiation %u:\n", it->first);
            print_mapping_list(it->second, printHeader);
            printHeader = false;
        }
    }

    #ifndef SOAR_RELEASE_VERSION
    thisAgent->outputManager->printa(thisAgent, "\n\n---------------------------\n\n");

    print_original_ebc_mappings();
    #endif
}

void identity_record::print_mapping_list(identity_mapping_list* pMapList, bool printHeader)
{
    Output_Manager* outputManager = thisAgent->outputManager;
    identity_mapping* lMapping;
    bool printOnlyChunkIdentities = thisAgent->explanationMemory->settings->only_print_chunk_identities->get_value();
    Symbol* lSym;

    outputManager->reset_column_indents();
    outputManager->set_column_indent(1, 3);  // Variablization identity mapping
    outputManager->set_column_indent(2, 23); // Chunk variable
    outputManager->set_column_indent(3, 35); // Mapping type

    if (printHeader)
    {
        outputManager->printa_sf(thisAgent, "Variablization IDs %-   Var%- Mapping Type\n");
    }
    for (auto it = pMapList->begin(); it != pMapList->end(); ++it)
    {
        lMapping = *it;
        auto lFindIter = idset_to_var_map->find(lMapping->from_identity);

        if (printOnlyChunkIdentities &&
            (lFindIter != idset_to_var_map->end()) &&
            !lFindIter->second)
            continue;

        outputManager->printa_sf(thisAgent, "%-%u -> %u", lMapping->from_identity, lMapping->to_identity);

        if (lFindIter != idset_to_var_map->end())
        {
            outputManager->printa_sf(thisAgent, "%-| %y", lFindIter->second);
        } else {
            outputManager->printa_sf(thisAgent, "%-|");
        }
        switch (lMapping->mappingType)
        {
            case IDS_no_existing_mapping:
                outputManager->printa_sf(thisAgent, "%-| New identity set\n");
                break;
            case IDS_transitive:
                outputManager->printa_sf(thisAgent, "%-| Identity set merge\n");
                break;
            case IDS_literalize_mappings_exist:
                outputManager->printa_sf(thisAgent, "%-| Literalized an existing identity set\n");
                break;
            case IDS_unified_with_existing_mappings:
                outputManager->printa_sf(thisAgent, "%-| Added to identity set\n");
                break;
            case IDS_unified_with_literalized_identity:
                outputManager->printa_sf(thisAgent, "%-| Added to identity set already literalized\n");
                break;
            case IDS_base_instantiation:
                outputManager->printa_sf(thisAgent, "%-| Chunk or base instantiation\n");
                break;
            case IDS_unified_with_local_singleton:
                outputManager->printa_sf(thisAgent, "%-| Unified with local singleton WME set\n");
                break;
            case IDS_unified_with_singleton:
                outputManager->printa_sf(thisAgent, "%-| Unified with singleton WME set\n");
                break;
            case IDS_unified_child_result:
                outputManager->printa_sf(thisAgent, "%-| Unified child result\n");
                break;
            case IDS_literalized_RHS_function_arg:
                outputManager->printa_sf(thisAgent, "%-| Literalized RHS function argument\n");
                break;
            default:
                outputManager->printa_sf(thisAgent, "%-| Chunk or base instantiation\n");
                break;
        }
    }
}

void identity_record::print_instantiation_mappings(uint64_t pI_ID)
{
    auto lIterInst = instantiation_mappings->find(pI_ID);
    if (lIterInst == instantiation_mappings->end())
    {
        thisAgent->outputManager->printa_sf(thisAgent, "No identity set mappings for instantiation %u.\n", pI_ID);
    } else {
        thisAgent->outputManager->printa_sf(thisAgent, "Identity Set Mappings:\n\n", pI_ID);
        print_mapping_list(lIterInst->second, true);
    }
}

void identity_record::clear_mappings()
{
    Output_Manager* outputManager = thisAgent->outputManager;

    for (auto it = instantiation_mappings->begin(); it != instantiation_mappings->end(); ++it)
    {
        identity_mapping* lMapping;
        identity_mapping_list* lMapList = it->second;
        for (auto it2 = lMapList->begin(); it2 != lMapList->end(); ++it2)
        {
            lMapping = *it2;
            thisAgent->memoryManager->free_with_pool(MP_identity_mapping, lMapping);
        }
        delete lMapList;
    }
}
