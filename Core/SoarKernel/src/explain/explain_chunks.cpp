#include "explain.h"
#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "symbol.h"
#include "test.h"
#include "output_manager.h"
#include "working_memory.h"

//    auto it = current_discussed_chunk->dependency_paths->find(pCondRecord);
//    if (it != current_discussed_chunk->dependency_paths->end())
//    {
//
//    }
//for (auto it = pUnconnected_LTIs->begin(); it != pUnconnected_LTIs->end(); it++)

chunk_record::chunk_record(agent* myAgent, uint64_t pChunkID)
{
    thisAgent           = myAgent;
    name                = NULL;
    conditions          = new condition_record_list;
    actions             = new action_record_list;
    chunkID             = pChunkID;
    baseInstantiation   = NULL;
    chunkInstantiation  = NULL;
    original_production = NULL;

    backtraced_inst_records = new inst_record_list();
    backtraced_instantiations = new inst_set();
    result_instantiations = new inst_set;
    result_inst_records = new inst_record_set;

    stats.duplicates = 0;
    stats.tested_local_negation = false;
    stats.reverted = false;
    stats.num_grounding_conditions_added = 0;
    stats.merged_conditions = 0;
    stats.instantations_backtraced = 0;
    stats.seen_instantations_backtraced = 0;
    stats.constraints_attached = 0;
    stats.constraints_collected = 0;

    dprint(DT_EXPLAIN, "Created new empty chunk record c%u\n", chunkID);
}

/* This function is not currently used, but I'm leaving in case we ever add something
 * that needs to excise explanations, for example a command or something to limit
 * memory use.  This function has not been tested. */
void chunk_record::excise_chunk_record()
{
    for (auto it = conditions->begin(); it != conditions->end(); it++)
    {
        thisAgent->explanationLogger->delete_condition((*it)->get_conditionID());
    }
    for (auto it = actions->begin(); it != actions->end(); it++)
    {
        thisAgent->explanationLogger->delete_action((*it)->get_actionID());
    }
    /* For this to work, store id of last chunk that created instantiation record.  If it's the
     * same as this chunk being excised, no other chunk uses it.  This assumes that this is only
     * called when a chunk fails.  If we need to excise records in general, we'll need refcounts */
    for (auto it = backtraced_inst_records->begin(); it != backtraced_inst_records->end(); it++)
    {
        if ((*it)->get_chunk_creator() == chunkID)
        {
            (*it)->delete_instantiation();
            thisAgent->explanationLogger->delete_instantiation((*it)->get_instantiationID());
        }
    }
}

chunk_record::~chunk_record()
{
    dprint(DT_EXPLAIN, "Deleting chunk record c%u\n", chunkID);
//    production_remove_ref(thisAgent, original_production);
    if (name) symbol_remove_ref(thisAgent, name);
    if (conditions) delete conditions;
    if (actions) delete actions;
    if (result_instantiations) delete result_instantiations;
    if (result_inst_records) delete result_inst_records;
    if (backtraced_inst_records) delete backtraced_inst_records;
    if (backtraced_instantiations) delete backtraced_instantiations;
}

void chunk_record::record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_id_map_type* pIdentitySetMappings, instantiation* pBaseInstantiation, tc_number pBacktraceNumber, instantiation* pChunkInstantiation)
{
    if (pProduction)
    {
        name = pProduction->name;
        symbol_add_ref(thisAgent, name);
        original_production = pProduction;
        original_production->save_for_justification_explanation = true;

    } else {
        name = NULL;
        original_production = NULL;
        assert(false);
    }
    time_formed = thisAgent->d_cycle_count;

    conditions         = new condition_record_list;
    actions            = new action_record_list;

    instantiation_record* lResultInstRecord, *lNewInstRecord;
    instantiation* lNewInst;

    /* Check if max number of instantiations in list.  If so, take most recent i_ids */
    dprint(DT_EXPLAIN, "(1) Recording all bt instantiations (%d instantiations)...\n", backtraced_instantiations->size());
    for (auto it = backtraced_instantiations->begin(); it != backtraced_instantiations->end(); it++)
    {
        lNewInst = (*it);
        lNewInstRecord = thisAgent->explanationLogger->add_instantiation((*it), chunkID);
        assert(lNewInstRecord);
        backtraced_inst_records->push_back(lNewInstRecord);
        dprint(DT_EXPLAIN, "%u (%y)\n", (*it)->i_id, (*it)->prod ? (*it)->prod->name : thisAgent->fake_instantiation_symbol);
    }
    dprint(DT_EXPLAIN, "(1) There are now %d instantiations records...\n", backtraced_inst_records->size());

    for (auto it = backtraced_inst_records->begin(); it != backtraced_inst_records->end(); it++)
    {
        lNewInstRecord = (*it);
        dprint(DT_EXPLAIN, "Recording instantiation i%u.\n", lNewInstRecord->get_instantiationID());
        if (lNewInstRecord->cached_inst->explain_status == explain_recording)
        {
            lNewInstRecord->record_instantiation_contents();

        } else if (lNewInstRecord->cached_inst->explain_status == explain_recording_update)
        {
            lNewInstRecord->update_instantiation_contents();
        } else {
            assert(false);
        }
        lNewInstRecord->cached_inst->explain_status = explain_recorded;
    }

    baseInstantiation = thisAgent->explanationLogger->get_instantiation(pBaseInstantiation);

    dprint(DT_EXPLAIN, "(2) Recording other result instantiation of chunk...\n", pBaseInstantiation->i_id);
    for (auto it = result_instantiations->begin(); it != result_instantiations->end(); ++it)
    {
        lResultInstRecord = thisAgent->explanationLogger->get_instantiation((*it));
        assert(lResultInstRecord);
        result_inst_records->insert(lResultInstRecord);
    }

    dprint(DT_EXPLAIN, "(4) Recording conditions of chunk...\n");
    /* Create condition and action records */
    instantiation_record* lchunkInstRecord;
    instantiation* lChunkCondInst = NULL;
    condition_record* lcondRecord;

    for (condition* cond = lhs; cond != NIL; cond = cond->next)
    {
        lChunkCondInst = cond->inst;
        dprint(DT_EXPLAIN, "Matching chunk condition %l from instantiation i%u (%y)", cond, lChunkCondInst->i_id,
            (lChunkCondInst->prod ? lChunkCondInst->prod->name : thisAgent->fake_instantiation_symbol));
        assert(lChunkCondInst->backtrace_number == pBacktraceNumber);
        /* The backtrace should have already added all instantiations that contained
         * grounds, so we can just look up the instantiation for each condition */
        lchunkInstRecord = thisAgent->explanationLogger->get_instantiation(lChunkCondInst);
        assert(lchunkInstRecord);
        lcondRecord = thisAgent->explanationLogger->add_condition(conditions, cond, lchunkInstRecord);
        lcondRecord->set_instantiation(lchunkInstRecord);
        cond->inst = pChunkInstantiation;
        cond->counterpart->inst = pChunkInstantiation;
    }
    dprint(DT_EXPLAIN, "...done with (4) adding chunk instantiation conditions!\n");

    dprint(DT_EXPLAIN, "(5) Recording actions...\n");

    action_record* new_action_record;
    preference* pref;
    action* lAction;
    for (pref = results, lAction= rhs; (pref != NIL) && (lAction != NIL); pref = pref->next_result, lAction = lAction->next)
    {
        new_action_record = thisAgent->explanationLogger->add_result(pref, lAction);
        actions->push_back(new_action_record);
    }
    identity_set_mappings = new id_to_id_map_type();
    (*identity_set_mappings) = (*pIdentitySetMappings);


    dprint(DT_EXPLAIN, "DONE recording chunk contents...\n");
}


void chunk_record::generate_dependency_paths()
{
    dprint(DT_EXPLAIN_PATHS, "Creating paths for chunk %u...\n", chunkID);

    inst_record_list* lInstPath = new inst_record_list();

    baseInstantiation->create_identity_paths(lInstPath);
    for (auto it = result_inst_records->begin(); it != result_inst_records->end(); it++)
    {
        (*it)->create_identity_paths(lInstPath);
    }

    delete lInstPath;

    dprint(DT_EXPLAIN_PATHS, "Getting paths from instantiations linked to conditions in chunk %u...\n", chunkID);
    condition_record* l_cond;
    instantiation_record* l_inst;
    inst_record_list* l_path;

    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        l_cond = (*it);
        l_inst = l_cond->get_instantiation();
        l_path = l_inst->get_path_to_base();
        dprint(DT_EXPLAIN_PATHS, "Path to base of length %d for chunk cond found from instantiation i%u: \n", l_path->size(), l_inst->get_instantiationID());
        l_cond->set_path_to_base(l_path);
    }
}

void chunk_record::end_chunk_record()
{
    if (backtraced_instantiations)
    {
        backtraced_instantiations->clear();
        result_instantiations->clear();
    }
}
