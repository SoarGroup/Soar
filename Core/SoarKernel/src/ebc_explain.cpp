#include "ebc_explain.h"

#include "condition.h"
#include "debug.h"
#include "instantiations.h"
#include "prefmem.h"
#include "production.h"
#include "output_manager.h"

Explanation_Logger::Explanation_Logger(agent* myAgent)
{
    /* Cache agent and Output Manager pointer */
    thisAgent = myAgent;
    outputManager = &Output_Manager::Get_OM();

    /* Initialize instantiation and identity ID counters */
    chunks_attempted_count = 0;
    duplicate_chunks_count = 0;
    merge_count = 0;
    chunk_id_count = 1;
    instantiation_id_count = 1;
    condition_id_count = 1;

    /* Create data structures used for EBC */
    chunks = new std::unordered_map< uint64_t, chunk_record* >();
}

uint64_t Explanation_Logger::add_chunk_record(instantiation* pBaseInstantiation)
{
    dprint(DT_EXPLAIN, "Adding chunk record for %y (ri%u)\n", pBaseInstantiation->prod->name, pBaseInstantiation->i_id);
    chunk_record* lChunkRecord = new chunk_record;
    lChunkRecord->baseInstantiationID = add_instantiation_record(pBaseInstantiation);
    lChunkRecord->conditions = NULL;
    lChunkRecord->actions = NULL;
    lChunkRecord->chunkID = chunk_id_count++;
    dprint(DT_EXPLAIN, "Returning chunk record id %u\n", lChunkRecord->chunkID);
    (*chunks)[lChunkRecord->chunkID] = lChunkRecord;

    return lChunkRecord->chunkID;
}

uint64_t Explanation_Logger::add_instantiation_record(instantiation* pInst)
{
    dprint(DT_EXPLAIN, "Adding instantiation record for %y (ri%u)\n", pInst->prod->name, pInst->i_id);
    if (pInst->explain_id)
    {
        dprint(DT_EXPLAIN, "Found existing explanation instantiation record i%u\n", pInst->explain_id);
        return pInst->explain_id;
    }
    instantiation_record* lInstRecord = new instantiation_record;
    lInstRecord->production_name = pInst->prod->name;
    lInstRecord->instantiationID = instantiation_id_count++;
    pInst->explain_id = lInstRecord->instantiationID;

    /* Create condition and action records */
    for (condition* cond = pInst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        add_condition_to_instantiation_record(cond, lInstRecord->instantiationID);
    }
    dprint(DT_EXPLAIN, "   -->\n");
    for (preference* pref = pInst->preferences_generated; pref != NIL; pref = pref->next_result)
    {
        add_result_to_instantiation_record(pref, lInstRecord->instantiationID);
    }
    lInstRecord->actions = NULL;

    dprint(DT_EXPLAIN, "Returning new explanation instantiation record i%u\n", pInst->explain_id);
    return lInstRecord->instantiationID;
}
uint64_t Explanation_Logger::add_condition_to_instantiation_record(condition* pCond, uint64_t pI_ID)
{
    dprint(DT_EXPLAIN, "   Adding to i%u: %l", pI_ID, pCond);
    return 0;
}
uint64_t Explanation_Logger::add_condition_to_chunk_record(condition* pCond, uint64_t pC_ID)
{
    dprint(DT_EXPLAIN, "   Adding to i%u: %l", pC_ID, pCond);
    return 0;
}
uint64_t Explanation_Logger::add_result_to_instantiation_record(preference* pPref, uint64_t pI_ID)
{
    dprint(DT_EXPLAIN, "   Adding to i%u: %p", pI_ID, pPref);
    return 0;
}

uint64_t Explanation_Logger::add_result_to_chunk_record(action* pAction, preference* pPref, uint64_t pC_ID)
{
    dprint(DT_EXPLAIN, "   Adding to i%u: %a %p", pC_ID, pAction, pPref);
    return 0;
}
