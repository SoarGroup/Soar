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

identity_record::identity_record(agent* myAgent, chunk_record* pChunkRecord)
{
    thisAgent = myAgent;
    id_to_id_set_mappings = new id_to_idset_map();
    identities_in_chunk = new id_set();
    instantiation_mappings = new inst_identities_map();
    original_ebc_mappings = NULL;
}

identity_record::~identity_record()
{
    if (original_ebc_mappings) delete original_ebc_mappings;
    if (id_to_id_set_mappings)
    {
        for (auto it = id_to_id_set_mappings->begin(); it != id_to_id_set_mappings->end(); ++it)
        {
            if (it->second->rule_variable) thisAgent->symbolManager->symbol_remove_ref(&it->second->rule_variable);
            delete it->second;
        }
        delete id_to_id_set_mappings;
    }
    if (identities_in_chunk) delete identities_in_chunk;

    clear_mappings();
    delete instantiation_mappings;
}


//#include "output_manager.h"
void identity_record::generate_identity_sets(uint64_t pInstID, condition* lhs)
{
    /* Generate identity sets and add mappings for all conditions in chunk */

    dprint(DT_EXPLAIN_IDENTITIES, "Building identity mappings based on conditions of chunk...\n");
    add_identities_in_condition_list(thisAgent, lhs, pInstID, identities_in_chunk, id_to_id_set_mappings);
}

void identity_record::map_originals_to_sets()
{

    /* Add mappings for other instantiations's identities based on original ebc_mappings */
    id_to_id_map::iterator iter;
    id_to_idset_map::iterator lIter;
    identity_set_info* lNewIDSet;
    uint64_t lMapping, lNewIdSetID;
    for (iter = original_ebc_mappings->begin(); iter != original_ebc_mappings->end(); ++iter)
    {
        lNewIDSet = new identity_set_info();
        if (iter->second != NULL_IDENTITY_SET)
        {
            lMapping = iter->first;
//            lIter = id_to_id_set_mappings->find(iter->first);
//            assert(lIter == id_to_id_set_mappings->end());

            lIter = id_to_id_set_mappings->find(iter->second);
            if (lIter != id_to_id_set_mappings->end())
            {
                /* Identity points to a current identity set */
                lNewIDSet->identity_set_ID = lIter->second->identity_set_ID;
                lNewIDSet->rule_variable = lIter->second->rule_variable;
                if (lNewIDSet->rule_variable) thisAgent->symbolManager->symbol_add_ref(lNewIDSet->rule_variable);
                id_to_id_set_mappings->insert({iter->first, lNewIDSet});
            } else {
                /* Identity points to an identity not in the chunk.  Create a new identity
                 * set and assign both identities to it. */
                lNewIdSetID = thisAgent->explanationMemory->get_identity_set_counter();
                lNewIDSet->identity_set_ID = lNewIdSetID;
                lNewIDSet->rule_variable = NULL;
                id_to_id_set_mappings->insert({iter->first, lNewIDSet});
                lNewIDSet = new identity_set_info();
                lNewIDSet->identity_set_ID = lNewIdSetID;
                lNewIDSet->rule_variable = NULL;
                id_to_id_set_mappings->insert({iter->first, lNewIDSet});
            }
        } else {
            /* Identity maps to NULL identity set */
            lNewIDSet->identity_set_ID = NULL_IDENTITY_SET;
            lNewIDSet->rule_variable = NULL;
            id_to_id_set_mappings->insert({iter->first, lNewIDSet});
        }
    }

//    print_identity_mappings();
}


void identity_record::print_identity_mappings_for_instantiation(instantiation_record* pInstRecord)
{
    id_set* identities_in_inst = pInstRecord->get_lhs_identities();

}

void identity_record::print_identities_in_chunk()
{
    dprint_noprefix(DT_EXPLAIN_IDENTITIES, "\nIdentities in chunk (%u): ", identities_in_chunk->size());
    for (auto it = identities_in_chunk->begin(); it != identities_in_chunk->end(); ++it)
    {
        dprint_noprefix(DT_EXPLAIN_IDENTITIES, "%u ", (*it));
    }
}
void identity_record::print_identity_mappings()
{
    thisAgent->outputManager->set_column_indent(0, 4);
    thisAgent->outputManager->set_column_indent(1, 14);
    thisAgent->outputManager->set_column_indent(2, 20);
    thisAgent->outputManager->set_column_indent(3, 37);

    thisAgent->outputManager->printa_sf(thisAgent, "%fHow the element identities in problem solving map to identity sets:\n\n");
    thisAgent->outputManager->printa_sf(thisAgent, "Identity %- %- Identity Set %- Chunk Variable\n");
    for (auto it = id_to_id_set_mappings->begin(); it != id_to_id_set_mappings->end(); ++it)
    {
        if (it->second->identity_set_ID)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%-%u %--> %-    %u %-%y\n", it->first, it->second->identity_set_ID, it->second->rule_variable);
        }
    }
    thisAgent->outputManager->printa(thisAgent, "\nThe following element identities in problem solving map to the null \n"
        "identity set. Elements in conditions that test those identities will \n"
        "not be generalized:\n\n");
    thisAgent->outputManager->printa_sf(thisAgent, "%- ");
    for (auto it = id_to_id_set_mappings->begin(); it != id_to_id_set_mappings->end(); ++it)
    {
        if (!it->second->identity_set_ID)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "%u ", it->first);
        }
    }
    thisAgent->outputManager->printa(thisAgent, "\n");

    //print_original_ebc_mappings();

}

void identity_record::print_original_ebc_mappings()
{
    thisAgent->outputManager->set_column_indent(0, 6);
    thisAgent->outputManager->set_column_indent(1, 26);
    thisAgent->outputManager->set_column_indent(2, 31);
    thisAgent->outputManager->printa_sf(thisAgent, "ID %-Original %-Set %-Final\n\n");

    dprint_noprefix(DT_EXPLAIN_IDENTITIES, "\nOriginal EBC Mappings (%u):\n", original_ebc_mappings->size());
    std::unordered_map< uint64_t, uint64_t >::iterator iter;
    for (iter = original_ebc_mappings->begin(); iter != original_ebc_mappings->end(); ++iter)
    {
        dprint_noprefix(DT_EXPLAIN_IDENTITIES, "%u%-%y %-%u%-%y\n",
            iter->first, thisAgent->explanationBasedChunker->get_ovar_for_o_id(iter->first), iter->second, thisAgent->explanationBasedChunker->get_ovar_for_o_id(iter->second));
    }
}

void identity_record::print_identity_explanation(chunk_record* pChunkRecord)
{
//    for (condition_record_list::iterator it = pChunkRecord->conditions->begin(); it != pChunkRecord->conditions->end(); it++)
//    {
//        l_cond = (*it);
//        dprint(DT_EXPLAIN, "Adding identities in chunk cond %l: \n", l_cond);
//    }
//
//    baseInstantiation->create_identity_paths(lInstPath);
//    for (auto it = result_inst_records->begin(); it != result_inst_records->end(); it++)
//    {
//        (*it)->create_identity_paths(lInstPath);
//    }
//
//    delete lInstPath;
}

void identity_record::add_identity_mapping(uint64_t pI_ID, IDSet_Mapping_Type pType,
                                           uint64_t pFromID,  uint64_t pToID,
                                           Symbol*  pFromSym, Symbol*  pToSym)
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
    identity_mapping* lMapping = new identity_mapping();
    lMapping->from_identity = pFromID;
    lMapping->from_symbol = pFromSym;
    if (pFromSym)
    {
        thisAgent->symbolManager->symbol_add_ref(pFromSym);
    }
    lMapping->to_identity = pToID;
    lMapping->to_symbol = pToSym;
    if (pToSym)
    {
        thisAgent->symbolManager->symbol_add_ref(pToSym);
    }
    lMapping->mappingType = pType;
    lInstMappingList->push_back(lMapping);
}

void identity_record::print_mappings()
{
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
}

void identity_record::print_mapping_list(identity_mapping_list* pMapList, bool printHeader)
{
    Output_Manager* outputManager = thisAgent->outputManager;
    identity_mapping* lMapping;
    bool printOnlyChunkIdentities = thisAgent->explanationMemory->settings->only_print_chunk_identities->get_value();

    outputManager->reset_column_indents();
    outputManager->set_column_indent(1, 3);  // Variablization identity mapping
    outputManager->set_column_indent(2, 23); // Identity set mapping
    outputManager->set_column_indent(3, 35); // Chunk variable
    outputManager->set_column_indent(4, 45); // Variable mappings
    outputManager->set_column_indent(5, 75); // Mapping type

    if (printHeader)
    {
        #ifdef DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS
                outputManager->printa_sf(thisAgent, "Variablization IDs %-  Identity %-   CVar%-    Original variables %- Mapping Type\n");
        #else
                outputManager->printa_sf(thisAgent, "Variablization IDs %-  Identity %-   CVar%- Mapping Type\n");
        #endif
    }
    for (auto it = pMapList->begin(); it != pMapList->end(); ++it)
    {
        lMapping = *it;
        auto lFindIter = id_to_id_set_mappings->find(lMapping->from_identity);

        if (printOnlyChunkIdentities && !lFindIter->second->rule_variable) continue;

        outputManager->printa_sf(thisAgent, "%-%u -> %u", lMapping->from_identity, lMapping->to_identity);

        if (lFindIter != id_to_id_set_mappings->end())
        {
            if (lFindIter->second->identity_set_ID)
            {
                outputManager->printa_sf(thisAgent, "%-| IdSet %u", lFindIter->second->identity_set_ID);
            } else {
                outputManager->printa_sf(thisAgent, "%-| Null Set");
            }
            if (lFindIter->second->rule_variable)
            {
                outputManager->printa_sf(thisAgent, "%-| %y", lFindIter->second->rule_variable);
            } else {
                outputManager->printa_sf(thisAgent, "%-|");
            }
        }
        #ifdef DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS
            outputManager->printa_sf(thisAgent, "%-| %y -> %y", lMapping->from_symbol, lMapping->to_symbol);
        #endif
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
            if (lMapping->from_symbol)
            {
                thisAgent->symbolManager->symbol_remove_ref(&(lMapping->from_symbol));
            }
            if (lMapping->to_symbol)
            {
                thisAgent->symbolManager->symbol_remove_ref(&(lMapping->to_symbol));
            }
            delete lMapping;
        }
        delete lMapList;
    }
}
