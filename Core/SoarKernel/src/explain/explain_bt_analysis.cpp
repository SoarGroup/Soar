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

void Explanation_Logger::record_dependencies()
{

    assert(current_discussed_chunk);

    current_discussed_chunk->record_dependencies();

}

void chunk_record::record_dependencies()
{
    dprint(DT_EXPLAIN_DEP, "Traversing conditions in chunk...\n");

    instantiation_record_list* lInstPath = new instantiation_record_list();

    baseInstantiation->record_dependencies(lInstPath);

    if (lInstPath)
    {
        dprint(DT_EXPLAIN_DEP, "Deleting lInstPath of size %d for chunk record.\n", lInstPath->size());
        delete lInstPath;
    }

//    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
//    {
//        (*it)->record_dependencies();
//    }
}

void add_all_identities_in_rhs_value(agent* thisAgent, rhs_value rv, id_set* pIDSet)
{
    list* fl;
    cons* c;
    Symbol* sym;

    if (rhs_value_is_symbol(rv))
    {
        /* --- ordinary values (i.e., symbols) --- */
        sym = rhs_value_to_symbol(rv);
        if (sym->is_variable())
        {
            dprint(DT_EXPLAIN_DEP, "Adding identity %u from rhs value to id set...\n", rhs_value_to_o_id(rv));
            pIDSet->insert(rhs_value_to_o_id(rv));
        }
    }
    else
    {
        /* --- function calls --- */
        fl = rhs_value_to_funcall_list(rv);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            add_all_identities_in_rhs_value(thisAgent, static_cast<char*>(c->first), pIDSet);
        }
    }
}

/* The add_LTI parameter is available so that when Soar is marking symbols for
 * action ordering based on whether the levels of the symbols would be known,
 * it also consider whether the LTIs level can be determined by being linked
 * to a LHS element or a RHS action that has already been executed */

void add_all_identities_in_action(agent* thisAgent, action* a, id_set* pIDSet)
{
    Symbol* id;

    if (a->type == MAKE_ACTION)
    {
        /* --- ordinary make actions --- */
        id = rhs_value_to_symbol(a->id);
        if (id->is_variable())
        {
            dprint(DT_EXPLAIN_DEP, "Adding identity %u from rhs action id to id set...\n", rhs_value_to_o_id(a->id));
            pIDSet->insert(rhs_value_to_o_id(a->id));
        }
        add_all_identities_in_rhs_value(thisAgent, a->attr, pIDSet);
        add_all_identities_in_rhs_value(thisAgent, a->value, pIDSet);
        if (preference_is_binary(a->preference_type))
        {
            add_all_identities_in_rhs_value(thisAgent, a->referent, pIDSet);
        }
    }
    else
    {
        /* --- function call actions --- */
        add_all_identities_in_rhs_value(thisAgent, a->value, pIDSet);
    }
}

id_set* action_record::get_identities()
{
    if (!identities_used)
    {
        identities_used = new id_set();
        add_all_identities_in_action(thisAgent, variablized_action, identities_used);
    }

    return identities_used;
}

bool test_contains_identity_in_set(agent* thisAgent, test t, const id_set* pIDSet)
{
    cons* c;

    switch (t->type)
    {
        case EQUALITY_TEST:
            if (t->identity)
            {
                id_set::const_iterator it;
                it = pIDSet->find(t->identity);
                if (it != pIDSet->end())
                {
                    return true;
                }
            }

            return false;
            break;
        case CONJUNCTIVE_TEST:
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                if (test_contains_identity_in_set(thisAgent, static_cast<test>(c->first), pIDSet))
                {
                    return true;
                }
            }

        default:  /* relational tests other than equality */
            return false;
    }
}

bool condition_record::contains_identity_from_set(const id_set* pIDSet)
{
    bool returnVal = (test_contains_identity_in_set(thisAgent, condition_tests.value, pIDSet) ||
        test_contains_identity_in_set(thisAgent, condition_tests.id, pIDSet) ||
        test_contains_identity_in_set(thisAgent, condition_tests.attr, pIDSet));

    dprint(DT_EXPLAIN_DEP, "condition_record::contains_identity_from_set returning %s.\n", returnVal ? "TRUE" : "FALSE");

    return returnVal;
}

void instantiation_record::record_dependencies(const instantiation_record_list* pInstPath, action_record* pAction, bool pUseId, bool pUseAttr, bool pUseValue)
{
    instantiation_record_list* lInstPath = new instantiation_record_list();
    (*lInstPath) = (*pInstPath);
    lInstPath->push_back(this);

    id_set* lIdsFound;
    int lNumIdsFounds = 0;
    if (pAction)
    {
        /* Compile a list of identities in rhs action */
        lIdsFound = pAction->get_identities();
        lNumIdsFounds = lIdsFound->size();
    }

    dprint(DT_EXPLAIN_DEP, "Traversing conditions looking for %d IDs...\n", lNumIdsFounds);
    /* If no pAction is passed in, this must be from the base instantiation or
     * the CDPS, so we process all conditions.
     *
     * If pAction but !lIdsFound, then the action is not connected to any LHS
     * conditions, so we don't process any conditions
     *
     * If pAction but !lIdsFound, then we look for conditions that contain any
     * of the identities in lIdsFound and call function recursively on those */
    if (!pAction || lNumIdsFounds)
    {
        bool lAffectedCondition = !pAction;
        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            if (lNumIdsFounds)
            {
                lAffectedCondition = (*it)->contains_identity_from_set(lIdsFound);
            }
            if (lAffectedCondition)
            {
                dprint(DT_EXPLAIN_DEP, "Found affected condition %u in i%u.  Recording its dependencies...\n", (*it)->get_conditionID(), instantiationID);
                (*it)->record_dependencies(lInstPath);
            }
        }
    }
    if (lInstPath)
    {
        dprint(DT_EXPLAIN_DEP, "Deleting lInstPath of size %d for instantiation record %u.\n", lInstPath->size(), instantiationID);
        delete lInstPath;
    }
    /* Note:  lIdsFound will be deleted by the action record.  That way they can be
     *        re-used, since they won't change between different chunks explained */
}

void condition_record::record_dependencies(const instantiation_record_list* pInstPath)
{
    if (path_to_base)
    {
        /* MToDo | Should allow multiple paths to base. */
        dprint(DT_EXPLAIN_DEP, "Condition already has a path to base.  Skipping (%t ^%t %t).\n", condition_tests.id, condition_tests.attr, condition_tests.value);
        return;
    } else
    {
        path_to_base = new instantiation_record_list();
        (*path_to_base) = (*pInstPath);
        dprint(DT_EXPLAIN_DEP, "Condition record copied path_to_base %d = %d.\n", path_to_base->size(), pInstPath->size());
    }
    if (parent_instantiation)
    {
        assert(parent_action);
        dprint(DT_EXPLAIN_DEP, "Traversing parent instantiation for condition (%t ^%t %t).\n", condition_tests.id, condition_tests.attr, condition_tests.value);
        parent_instantiation->record_dependencies(path_to_base);
        /* List has current instantiation, which we needed for parent calls but don't need for this condition, so we pop it off */
        path_to_base->pop_back();
    } else {
        dprint(DT_EXPLAIN_DEP, "No parent instantiation to traverse for condition (%t ^%t %t).\n", condition_tests.id, condition_tests.attr, condition_tests.value);
    }
}
