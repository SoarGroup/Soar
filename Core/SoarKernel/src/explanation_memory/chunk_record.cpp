#include "chunk_record.h"

#include "action_record.h"
#include "condition_record.h"
#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "identity_record.h"
#include "instantiation.h"
#include "instantiation_record.h"
#include "output_manager.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "visualize.h"
#include "working_memory.h"

chunk_record::chunk_record(agent* myAgent, uint64_t pChunkID)
{
    thisAgent                   = myAgent;
    name                        = NULL;
    chunkID                     = pChunkID;
    chunkInstantiation          = NULL;
    original_production         = NULL;
    excised_production          = NULL;
    time_formed                 = 0;
    match_level                 = 0;

    conditions                  = new condition_record_list;
    actions                     = new action_record_list;

    baseInstantiation           = NULL;
    result_instantiations       = new inst_set();
    result_inst_records         = new inst_record_set();

    backtraced_instantiations   = new inst_set();
    backtraced_inst_records     = new inst_record_list();

    identity_analysis           = NULL;

    stats.duplicates                        = 0;
    stats.tested_local_negation             = false;
    stats.reverted                          = false;
    stats.num_grounding_conditions_added    = 0;
    stats.merged_conditions                 = 0;
    stats.instantations_backtraced          = 0;
    stats.seen_instantations_backtraced     = 0;
    stats.constraints_attached              = 0;
    stats.constraints_collected             = 0;

    dprint(DT_EXPLAIN, "Created new empty chunk record c%u\n", chunkID);
}

/* This function is not currently used, but I'm leaving in case we ever add something
 * that needs to excise explanations, for example a command or something to limit
 * memory use.  This function has not been tested. */
void chunk_record::excise_chunk_record()
{
    for (auto it = conditions->begin(); it != conditions->end(); it++)
    {
        thisAgent->explanationMemory->delete_condition((*it)->get_conditionID());
    }
    for (auto it = actions->begin(); it != actions->end(); it++)
    {
        thisAgent->explanationMemory->delete_action((*it)->get_actionID());
    }
    /* For this to work, store id of last chunk that created instantiation record.  If it's the
     * same as this chunk being excised, no other chunk uses it.  This assumes that this is only
     * called when a chunk fails.  If we need to excise records in general, we'll need refcounts */
    for (auto it = backtraced_inst_records->begin(); it != backtraced_inst_records->end(); it++)
    {
        if ((*it)->get_chunk_creator() == chunkID)
        {
            (*it)->delete_instantiation();
            thisAgent->explanationMemory->delete_instantiation((*it)->get_instantiationID());
        }
    }
}

chunk_record::~chunk_record()
{
    dprint(DT_EXPLAIN, "Deleting chunk record c%u\n", chunkID);
//    production_remove_ref(thisAgent, original_production);
    if (original_production)
    {
        original_production->save_for_justification_explanation = false;
    }
    if (name) thisAgent->symbolManager->symbol_remove_ref(&name);
    if (conditions) delete conditions;
    if (actions) delete actions;
    if (result_instantiations) delete result_instantiations;
    if (result_inst_records) delete result_inst_records;
    if (backtraced_inst_records) delete backtraced_inst_records;
    if (backtraced_instantiations) delete backtraced_instantiations;
    if (identity_analysis) delete identity_analysis;
    dprint(DT_EXPLAIN, "Done deleting chunk record c%u\n", chunkID);
}

void chunk_record::record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_id_map* pIdentitySetMappings, instantiation* pBaseInstantiation, tc_number pBacktraceNumber, instantiation* pChunkInstantiation)
{
    name = pProduction->name;
    thisAgent->symbolManager->symbol_add_ref(name);
    original_production = pProduction;
    original_production->save_for_justification_explanation = true;

    time_formed = thisAgent->d_cycle_count;
    match_level = pChunkInstantiation->match_goal_level;

    conditions         = new condition_record_list;
    actions            = new action_record_list;

    instantiation_record* lResultInstRecord, *lNewInstRecord;
    instantiation* lNewInst;

    dprint(DT_EXPLAIN, "(1) Recording all bt instantiations (%d instantiations)...\n", backtraced_instantiations->size());
    for (auto it = backtraced_instantiations->begin(); it != backtraced_instantiations->end(); it++)
    {
        lNewInst = (*it);
        lNewInstRecord = thisAgent->explanationMemory->add_instantiation((*it), chunkID);
        assert(lNewInstRecord);
        backtraced_inst_records->push_back(lNewInstRecord);
        dprint(DT_EXPLAIN, "%u (%y)\n", (*it)->i_id, (*it)->prod_name);
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
        }
        lNewInstRecord->cached_inst->explain_status = explain_recorded;
    }

    baseInstantiation = thisAgent->explanationMemory->get_instantiation(pBaseInstantiation);

    dprint(DT_EXPLAIN, "(2) Recording other result instantiation of chunk...\n", pBaseInstantiation->i_id);
    for (auto it = result_instantiations->begin(); it != result_instantiations->end(); ++it)
    {
        lResultInstRecord = thisAgent->explanationMemory->get_instantiation((*it));
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
        if (lChunkCondInst)
        {
            dprint(DT_EXPLAIN, "Matching chunk condition %l from instantiation i%u (%y)", cond, lChunkCondInst->i_id, lChunkCondInst->prod_name);
            /* The backtrace should have already added all instantiations that contained
             * grounds, so we can just look up the instantiation for each condition */
            lchunkInstRecord = thisAgent->explanationMemory->get_instantiation(lChunkCondInst);
        } else {
            lchunkInstRecord = NULL;
        }
        lcondRecord = thisAgent->explanationMemory->add_condition(conditions, cond, lchunkInstRecord);
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
        new_action_record = thisAgent->explanationMemory->add_result(pref, lAction);
        actions->push_back(new_action_record);
    }

    dprint(DT_EXPLAIN, "(5) Recording identity mappings...\n");

    identity_analysis = new identity_record(thisAgent, this, pIdentitySetMappings);
    identity_analysis->generate_identity_sets(lhs);

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
        if (l_inst)
        {
            l_path = l_inst->get_path_to_base();
            dprint(DT_EXPLAIN_PATHS, "Path to base of length %d for chunk cond found from instantiation i%u: \n", l_path->size(), l_inst->get_instantiationID());
            l_cond->set_path_to_base(l_path);
        }
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

void chunk_record::print_for_explanation_trace()
{
    Output_Manager* outputManager = thisAgent->outputManager;

    outputManager->set_column_indent(0, 7);
    outputManager->set_column_indent(1, 60);
    outputManager->set_column_indent(2, 110);

    outputManager->printa_sf(thisAgent, "Explanation Trace %-Using variable identity IDs   %-Shortest Path to Result Instantiation\n\n");
    outputManager->printa_sf(thisAgent, "sp {%y\n", name);

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;
        test id_test_without_goal_test = NULL;
        bool removed_goal_test, removed_impasse_test;

        outputManager->set_print_test_format(true, false);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     }\n");
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     -{\n");
                    lInNegativeConditions = true;
                }
            }
            outputManager->printa_sf(thisAgent, "%d:%-", lConditionCount);

            id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, lCond->condition_tests.id, &removed_goal_test, &removed_impasse_test);
            outputManager->printa_sf(thisAgent, "(%t%s^%t %t)%s%-",
                id_test_without_goal_test, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                lCond->condition_tests.attr, lCond->condition_tests.value, thisAgent->explanationMemory->is_condition_related(lCond) ? "*" : "");
            outputManager->printa_sf(thisAgent, "(%g%s^%g %g)%-",
                id_test_without_goal_test, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                lCond->condition_tests.attr, lCond->condition_tests.value);
            deallocate_test(thisAgent, id_test_without_goal_test);

            thisAgent->explanationMemory->print_path_to_base(lCond->get_path_to_base(), true);
        }
        if (lInNegativeConditions)
        {
            outputManager->printa(thisAgent, "     }\n");
        }
    }
    outputManager->printa(thisAgent, "      -->\n");

    /* For chunks, actual rhs is same as explanation trace without identity information on the rhs*/
    thisAgent->explanationMemory->print_chunk_actions(actions, original_production, excised_production);
    outputManager->printa(thisAgent, "}\n");
    thisAgent->explanationMemory->print_footer(true);
}

void chunk_record::print_for_wme_trace()
{
    Output_Manager* outputManager = thisAgent->outputManager;

    outputManager->set_column_indent(0, 7);
    outputManager->set_column_indent(1, 60);
    outputManager->set_column_indent(2, 110);
    outputManager->printa_sf(thisAgent, "Working Memory Trace %-Instantiation that created matched WME\n\n");
    outputManager->printa_sf(thisAgent, "sp {%y\n", name);

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;
        test id_test_without_goal_test = NULL, id_test_without_goal_test2 = NULL;
        bool removed_goal_test, removed_impasse_test;

        outputManager->set_print_test_format(true, false);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     }\n");
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     -{\n");
                    lInNegativeConditions = true;
                }
            }
            outputManager->printa_sf(thisAgent, "%d:%-", lConditionCount);

            if (lCond->matched_wme)
            {
                outputManager->printa_sf(thisAgent, "(%y ^%y %y)%s",
                    lCond->matched_wme->id, lCond->matched_wme->attr, lCond->matched_wme->value, thisAgent->explanationMemory->is_condition_related(lCond) ? "*" : "");
            } else {
                outputManager->printa_sf(thisAgent, "(N/A)%s", thisAgent->explanationMemory->is_condition_related(lCond) ? "*" : "");
            }
            if (lCond->matched_wme != NULL)
            {
                instantiation_record* lInstRecord = lCond->get_instantiation();
                bool isSuper = (match_level > 0) && (lCond->wme_level_at_firing < match_level);
                if (lInstRecord)
                {
                    outputManager->printa_sf(thisAgent, "%-i %u (%y)\n",
                        lInstRecord->instantiationID,
                        lInstRecord->production_name);
                } else if (lCond->type == POSITIVE_CONDITION)
                {
                    outputManager->printa_sf(thisAgent, isSuper ? "%-Higher-level Problem Space%-" : "%-Soar Architecture%-");
                } else {
                    outputManager->printa_sf(thisAgent, "%-N/A%-");
                }
            } else {
                outputManager->printa(thisAgent, "\n");
            }
        }
        if (lInNegativeConditions)
        {
            outputManager->printa(thisAgent, "     }\n");
        }
    }
    outputManager->printa(thisAgent, "      -->\n");

    /* For chunks, actual rhs is same as explanation trace without identity information on the rhs*/
    thisAgent->explanationMemory->print_chunk_actions(actions, original_production, excised_production);
    outputManager->printa(thisAgent, "}\n");
    thisAgent->explanationMemory->print_footer(true);
}

void chunk_record::visualize()
{
    Output_Manager* outputManager = thisAgent->outputManager;
    GraphViz_Visualizer* visualizer = thisAgent->visualizationManager;
    condition_record* lCond;

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
        assert(false);
    }
    else
    {
        bool lInNegativeConditions = false;
        int lConditionCount = 0;
        test id_test_without_goal_test = NULL, id_test_without_goal_test2 = NULL;
        bool removed_goal_test, removed_impasse_test;

        thisAgent->outputManager->set_print_test_format(false, true);
        visualizer->viz_object_start(name, chunkID, viz_chunk_record);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);

            ++lConditionCount;
            if (lConditionCount > 1)
                visualizer->viz_endl();

            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    visualizer->viz_NCC_end();
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    visualizer->viz_NCC_start();
                    lInNegativeConditions = true;
                }
            }
            lCond->visualize_for_chunk();
        }
        if (lInNegativeConditions)
        {
            visualizer->viz_NCC_end();
        } else {
            visualizer->viz_endl();
        }
        visualizer->viz_seperator();
        action_record::viz_action_list(thisAgent, actions, original_production);
        visualizer->viz_object_end(viz_chunk_record);
    }

    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        lCond = (*it);
        visualizer->viz_connect_inst_to_chunk(lCond->get_instantiation()->get_instantiationID(), this->chunkID, lCond->get_conditionID());
    }

}
