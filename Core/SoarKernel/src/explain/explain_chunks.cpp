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
    dependency_paths    = NULL;
//    dependency_paths = new condition_to_ipath_map();

    result_instantiations = new inst_set;
    result_inst_records = new inst_record_set;

    stats.constraints_attached = 0;
    stats.constraints_collected = 0;
    stats.duplicates = 0;
    stats.instantations_backtraced = 0;
    stats.merged_conditions = 0;
    stats.seen_instantations_backtraced = 0;
    stats.tested_local_negation = false;

    dprint(DT_EXPLAIN, "Created new empty chunk record c%u\n", chunkID);
}

chunk_record::~chunk_record()
{
    dprint(DT_EXPLAIN, "Deleting chunk record c%u\n", chunkID);
    symbol_remove_ref(thisAgent, name);
//    production_remove_ref(thisAgent, original_production);
    delete conditions;
    delete actions;
    delete result_instantiations;
    delete result_inst_records;
}

int condition_count(condition* pCond)
{
    int cnt = 0;
    while (pCond != NULL)
    {
        ++cnt;
        pCond = pCond->next;
    }
    return cnt;
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
    conditions         = new condition_record_list;
    actions            = new action_record_list;

    dprint(DT_EXPLAIN, "(1) Recording base instantiation i%u of chunk (%d conditions)...\n", pBaseInstantiation->i_id, condition_count(pBaseInstantiation->top_of_instantiated_conditions));
    baseInstantiation = thisAgent->explanationLogger->add_instantiation(pBaseInstantiation, 1);

    dprint(DT_EXPLAIN, "(2) Recording other result instantiation of chunk...\n", pBaseInstantiation->i_id);
    instantiation_record* lResultInstRecord;
    for (auto it = result_instantiations->begin(); it != result_instantiations->end(); ++it)
    {
//        lResultInstRecord = thisAgent->explanationLogger->get_instantiation((*it));
//        if (!lResultInstRecord)
//        {
//            dprint(DT_EXPLAIN, "...FOUND EXTRA RESULT INSTANTIATION: %u (%y)...\n", (*it)->i_id, ((*it)->prod ? (*it)->prod->name : thisAgent->fake_instantiation_symbol));
            lResultInstRecord = thisAgent->explanationLogger->add_instantiation((*it), 1);
            assert(lResultInstRecord);
            result_inst_records->insert(lResultInstRecord);
//        } else {
//            dprint(DT_EXPLAIN, "...result instantiation already exists: %u (%y)...\n", (*it)->i_id, ((*it)->prod ? (*it)->prod->name : thisAgent->fake_instantiation_symbol));
//        }
    }

    /* Comment not true if we keep here:  We link up all of the dependencies here.  Since the linking may be expensive and
     * we may be watching all chunk formations, we wait until someone attempts to look
     * at the dependency analysis before we perform the linking. */
    dprint(DT_EXPLAIN, "(3) Connecting conditions in trace and generating baths to base instantiations...\n");
    record_dependencies();

    dprint(DT_EXPLAIN, "(4) Recording conditions of chunk...\n");
    thisAgent->explanationLogger->print_involved_instantiations();
    /* Create condition and action records */
    bool has_backtrace_num = false;
    instantiation_record* lchunkInstantiation;
    condition_record* lcondRecord;

    for (condition* cond = lhs; cond != NIL; cond = cond->next)
    {
        dprint(DT_EXPLAIN, "Matching chunk condition %l from instantiation i%u (%y)", cond, cond->bt.inst->i_id,
            (cond->bt.inst->prod ? cond->bt.inst->prod->name : thisAgent->fake_instantiation_symbol));
        has_backtrace_num = (cond->bt.trace && cond->bt.trace->inst && (cond->bt.trace->inst->backtrace_number == pBacktraceNumber));
        lcondRecord = thisAgent->explanationLogger->add_condition(conditions, cond, NULL, has_backtrace_num, 0);
        lchunkInstantiation = thisAgent->explanationLogger->get_instantiation(cond->bt.inst);
//        lchunkInstantiation = thisAgent->explanationLogger->add_instantiation(cond->bt.inst, 1);
        /* The backtrace should have already added all instantiations that contained
         * grounds, so we can just look up the instantiation for each condition */
        assert(lchunkInstantiation);
        lcondRecord->set_instantiation(lchunkInstantiation);
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


void chunk_record::record_dependencies()
{

    dprint(DT_EXPLAIN_CONNECT, "Connecting conditions to specific RHS actions...\n");
    /* Now that instantiations in backtrace are guaranteed to be recorded, connect
     * each condition to the appropriate parent instantiation action record */
    /* MToDo | We might only need to connect base conditions and CDPS */
    baseInstantiation->connect_conds_to_actions();

    dprint(DT_EXPLAIN_CONNECT, "Done connecting conditions to specific RHS actions...\n");

    dprint(DT_EXPLAIN_PATHS, "Creating paths based on identities affected %u...\n", chunkID);

    inst_record_list* lInstPath = new inst_record_list();

    baseInstantiation->create_identity_paths(lInstPath);

    if (lInstPath)
    {
        delete lInstPath;
    }

    dprint(DT_EXPLAIN_PATHS, "Searching for paths for conditions in chunk %u...\n", chunkID);
    condition_record* linked_cond, *l_cond;
    preference* l_cachedPref;
    wme* l_cachedWME;
    instantiation_record* l_inst;
    inst_record_list* l_path;

    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        l_cond = (*it);
        l_cachedPref = l_cond->get_cached_pref();
        l_cachedWME = l_cond->get_cached_wme();
        l_inst = l_cond->get_instantiation();
        linked_cond = l_inst->find_condition_for_chunk(l_cachedPref, l_cachedWME);
        assert(linked_cond);
        l_path = linked_cond->get_path_to_base();
        dprint(DT_EXPLAIN_PATHS, "Condition %u found with path: ", linked_cond->get_conditionID());
        l_cond->set_path_to_base(l_path);
        l_cond->print_path_to_base();
        dprint(DT_EXPLAIN_PATHS, "\n");
    }
}




