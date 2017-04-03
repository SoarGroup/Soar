#include "identity_record.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "ebc.h"
#include "ebc_identity.h"
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
#include "visualize.h"
#include "working_memory.h"

void identity_record::init(agent* myAgent)
{
    thisAgent = myAgent;
    idset_to_var_map = new id_to_sym_map();
    identities_in_chunk = new id_set();
    instantiation_mappings = new inst_identities_map();
    identity_joins = new id_to_id_map();
}

void identity_record::add_identity_mapping(uint64_t pI_ID, IDSet_Mapping_Type pType, Identity* pFromID,  Identity* pToID)
{
    identity_mapping_list* lInstMappingList;

    dprint(DT_EXPLAIN_IDENTITIES, "Adding identity mappings type %d: %u -> %u\n", static_cast<int>(pType), pFromID->get_identity(), pToID ? pToID->get_identity() : 0);
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
    lMapping->from_identity = pFromID->get_identity();
    lMapping->to_identity = pToID ? pToID->get_identity() : 0;
    lMapping->mappingType = pType;
    lInstMappingList->push_back(lMapping);

    #ifdef EBC_DETAILED_STATISTICS
    switch (lMapping->mappingType)
    {
        case IDS_join:
            thisAgent->explanationMemory->increment_stat_identities_joined_variable();
            break;
        case IDS_unified_with_singleton:
            thisAgent->explanationMemory->increment_stat_identities_joined_singleton();
            break;
        case IDS_unified_child_result:
            thisAgent->explanationMemory->increment_stat_identities_joined_child_results();
            break;
        case IDS_literalized_LHS_literal:
            thisAgent->explanationMemory->increment_stat_identities_literalized_lhs_literal();
            break;
        case IDS_literalized_RHS_literal:
            thisAgent->explanationMemory->increment_stat_identities_literalized_rhs_literal();
            break;
        case IDS_literalized_RHS_function_arg:
            thisAgent->explanationMemory->increment_stat_identities_literalized_rhs_func_arg();
            break;
        case IDS_literalized_RHS_function_compare:
            thisAgent->explanationMemory->increment_stat_identities_literalized_rhs_func_compare();
            break;
        default:
            assert(false);
            break;
    }
    #endif

}

void identity_record::clean_up()
{
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
    delete identity_joins;

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

void generate_identity_sets_from_test(agent* thisAgent, test pTest, uint64_t pInstID, id_set* pIdentity, id_to_sym_map* pID_Set_Map)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
        pTest = pTest->eq_test;
    }
    if (pTest->inst_identity)
    {
        if (pIdentity->find(pTest->inst_identity) == pIdentity->end())
        {
            pIdentity->insert(pTest->inst_identity);
            pID_Set_Map->insert({pTest->inst_identity, pTest->data.referent});
            thisAgent->symbolManager->symbol_add_ref(pTest->data.referent);
        }
    }
}

void generate_identity_sets_from_conditions(agent* thisAgent, condition* lhs, uint64_t pInstID, id_set* pIdentity, id_to_sym_map* pID_Set_Map)
{
    for (condition* lCond = lhs; lCond != NULL; lCond = lCond->next)
    {
        if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            generate_identity_sets_from_conditions(thisAgent, lCond->data.ncc.top, pInstID, pIdentity, pID_Set_Map);
        } else {
            generate_identity_sets_from_test(thisAgent, lCond->data.tests.id_test, pInstID, pIdentity, pID_Set_Map);
            generate_identity_sets_from_test(thisAgent, lCond->data.tests.attr_test, pInstID, pIdentity, pID_Set_Map);
            generate_identity_sets_from_test(thisAgent, lCond->data.tests.value_test, pInstID, pIdentity, pID_Set_Map);
        }
    }
}

void identity_record::analyze_chunk_identities(uint64_t pInstID, condition* lhs)
{
    /* Generate identity sets and add mappings for all conditions in chunk */

    dprint(DT_EXPLAIN_IDENTITIES, "Building identity mappings based on conditions of chunk...\n");
    generate_identity_sets_from_conditions(thisAgent, lhs, pInstID, identities_in_chunk, idset_to_var_map);

    /* MToDo | Might need to generate identity sets for RHS as well for unbound vars */

    #ifdef EBC_DETAILED_STATISTICS
    for (auto it = identities_in_chunk->begin(); it != identities_in_chunk->end(); ++it)
    {
        thisAgent->explanationMemory->increment_stat_identities_participated();
    }
    #endif

}

void identity_record::print_identity_mappings_for_instantiation(instantiation_record* pInstRecord)
{
    id_set* identities_in_inst = pInstRecord->get_lhs_identities();

}

void identity_record::print_identities_in_chunk()
{
    thisAgent->outputManager->printa_sf(thisAgent, "\nLearned rules contained %u identities: ", identities_in_chunk->size());
    for (auto it = identities_in_chunk->begin(); it != identities_in_chunk->end(); ++it)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%u ", (*it));
    }
    thisAgent->outputManager->printa(thisAgent, "\n");
}

void identity_record::print_instantiation_mappings(uint64_t pI_ID)
{
    auto lIterInst = instantiation_mappings->find(pI_ID);
    if (lIterInst == instantiation_mappings->end())
    {
        thisAgent->outputManager->printa_sf(thisAgent, "No identity set unifications for instantiation %u.\n", pI_ID);
    } else {
        thisAgent->outputManager->printa_sf(thisAgent, "Identity set unifications:\n\n", pI_ID);
        print_mapping_list(lIterInst->second, false);
        print_mapping_list(lIterInst->second, true);
    }
}

void identity_record::print_mappings()
{
    Output_Manager* outputManager = thisAgent->outputManager;
    outputManager->set_column_indent(0, 4);
    outputManager->set_column_indent(1, 14);
    outputManager->set_column_indent(2, 20);
    outputManager->set_column_indent(3, 37);

    bool unification_found = false;
    std::string lInstString;

    for (auto it = instantiation_mappings->begin(); it != instantiation_mappings->end(); ++it)
    {
        if (it->second->size() != 0)
        {
            if (!unification_found)
            {
                thisAgent->outputManager->printa_sf(thisAgent, "Identity set unifications:\n\n");
                unification_found = true;
            }
            lInstString.clear();
            outputManager->sprinta_sf(thisAgent, lInstString, " in instantiation %u", it->first);
            print_mapping_list(it->second, false, &lInstString);
        }
    }
    if (!unification_found)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "No identity unifications occurred during backtracing.\n");
    } else {
        for (auto it = instantiation_mappings->begin(); it != instantiation_mappings->end(); ++it)
        {
            if (it->second->size() != 0)
            {
                lInstString.clear();
                outputManager->sprinta_sf(thisAgent, lInstString, " in instantiation %u", it->first);
                print_mapping_list(it->second, true, &lInstString);
            }
        }
    }
}

void identity_record::print_mapping_list(identity_mapping_list* pMapList, bool pLiteralizationMode, std::string* pInstString)
{
    Output_Manager* outputManager = thisAgent->outputManager;
    identity_mapping* lMapping;
    bool printOnlyChunkIdentities = thisAgent->explanationMemory->settings->only_print_chunk_identities->get_value();
    Symbol* lSym;

    outputManager->reset_column_indents();
    outputManager->set_column_indent(1, 3);
    outputManager->set_column_indent(2, 33);

    for (auto it = pMapList->begin(); it != pMapList->end(); ++it)
    {
        lMapping = *it;
        if (pLiteralizationMode)
        {
            if (lMapping->to_identity) continue;
        } else {
            if (!lMapping->to_identity) continue;
        }
        auto lFindIter = idset_to_var_map->find(lMapping->to_identity);

        if (printOnlyChunkIdentities &&
            (lFindIter != idset_to_var_map->end()) &&
            !lFindIter->second)
            continue;

        if (lMapping->to_identity)
            outputManager->printa_sf(thisAgent, "%-%u merged with %u", lMapping->from_identity, lMapping->to_identity);
        else
            outputManager->printa_sf(thisAgent, "%-%u literalized", lMapping->from_identity);

        if (lFindIter != idset_to_var_map->end())
        {
            outputManager->printa_sf(thisAgent, " %y", lFindIter->second);
        }
        switch (lMapping->mappingType)
        {
            case IDS_join:
                outputManager->printa_sf(thisAgent, "%-| Two identities propagated into the same variable");
                break;
            case IDS_unified_with_singleton:
                outputManager->printa_sf(thisAgent, "%-| Tested a super-state singleton WME previously tested by another rule");
                break;
            case IDS_unified_child_result:
                outputManager->printa_sf(thisAgent, "%-| Identities joined to connected child result to parent result");
                break;
            case IDS_literalized_LHS_literal:
                outputManager->printa_sf(thisAgent, "%-| Literal value in another rule compared against RHS variable");
                break;
            case IDS_literalized_RHS_literal:
                outputManager->printa_sf(thisAgent, "%-| Variable in another rule compared against literal RHS value");
                break;
            case IDS_literalized_RHS_function_arg:
                outputManager->printa_sf(thisAgent, "%-| Variable was used as argument in a RHS function");
                break;
            case IDS_literalized_RHS_function_compare:
                outputManager->printa_sf(thisAgent, "%-| Variable in another rule tested result of RHS function");
                break;
            default:
                assert(false);
                outputManager->printa_sf(thisAgent, "%-| Bad identity mapping type");
                break;
        }
        if (pInstString) outputManager->printa_sf(thisAgent, "%s\n", pInstString->c_str());
        else outputManager->printa_sf(thisAgent, "\n");
    }
}
void identity_record::record_identity_sets(identity_set* identity_sets)
{
    Identity* l_inst_identity;

    for (auto it = identity_sets->begin(); it != identity_sets->end(); it++)
    {
        l_inst_identity = (*it);
        (*identity_joins)[l_inst_identity->idset_id] = l_inst_identity->joined_identity->idset_id;
    }
}

void identity_record::visualize()
{
    Symbol*             lSym;
    identity_mapping*   lMapping;
    Identity*        l_inst_identity;

//    for (auto it = identity_joins->begin(); it != identity_joins->end(); ++it)
//    {
//        if (it->first != it->second)
//            thisAgent->visualizationManager->viz_connect_identities(it->first, it->second);
//    }

    for (auto it = instantiation_mappings->begin(); it != instantiation_mappings->end(); ++it)
    {
        if (it->second->size() != 0)
        {
            for (auto it2 = it->second->begin(); it2 != it->second->end(); ++it2)
             {
                lMapping = *it2;
                if (lMapping->to_identity)
                    thisAgent->visualizationManager->viz_connect_identities(lMapping->from_identity, lMapping->to_identity);
             }
        }
    }
}
