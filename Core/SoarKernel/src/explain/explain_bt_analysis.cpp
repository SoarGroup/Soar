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

condition_record* instantiation_record::find_condition_for_chunk(preference* pPref, wme* pWME)
{
    dprint(DT_EXPLAIN_PATHS, "Looking for condition in i%u: ", instantiationID);
    if (pPref)
    {
        dprint_noprefix(DT_EXPLAIN_PATHS, " pref %p", pPref);
    }
    if (pWME)
    {
        dprint_noprefix(DT_EXPLAIN_PATHS, " wme %w", pWME);
    }
    dprint_noprefix(DT_EXPLAIN_PATHS, "\n");
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        dprint(DT_EXPLAIN_PATHS, "Comparing against condition %u", (*it)->get_conditionID());
        condition_record* lit = (*it);
        preference* lp = lit->get_cached_pref();
        wme* lw = lit->get_cached_wme();
        uint64_t li = (*it)->get_conditionID();
        if ((*it)->get_conditionID() == 24) {
            dprint(DT_EXPLAIN_PATHS, "Comparing against condition %u", (*it)->get_conditionID());
        }
        if (lp)
        {
            dprint_noprefix(DT_EXPLAIN_PATHS, " %p", lp);
        }
        if (lw)
        {
            dprint_noprefix(DT_EXPLAIN_PATHS, " %w", lw);
        }
        dprint_noprefix(DT_EXPLAIN_PATHS, "\n");

        if (pPref && ((*it)->get_cached_pref() == pPref))
        {
            dprint(DT_EXPLAIN_PATHS, "Found condition %u %p for target preference %p\n", (*it)->get_conditionID(), (*it)->get_cached_pref(), pPref);
            return (*it);
        } else if (pWME && ((*it)->get_cached_wme() == pWME))
        {
            dprint(DT_EXPLAIN_PATHS, "Found condition %u %w for target wme %w\n", (*it)->get_conditionID(), (*it)->get_cached_wme(), pWME);
            return (*it);
        }
    }
//    assert(false);
    return NULL;
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
            dprint(DT_EXPLAIN_PATHS, "Adding identity %u from rhs value to id set...\n", rhs_value_to_o_id(rv));
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
            dprint(DT_EXPLAIN_PATHS, "Adding identity %u from rhs action id to id set...\n", rhs_value_to_o_id(a->id));
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

    dprint(DT_EXPLAIN_PATHS, "condition_record::contains_identity_from_set returning %s.\n", returnVal ? "TRUE" : "FALSE");

    return returnVal;
}

void instantiation_record::create_identity_paths(const inst_record_list* pInstPath, action_record* pAction, bool pUseId, bool pUseAttr, bool pUseValue)
{
    inst_record_list* lInstPath = new inst_record_list();
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

    dprint(DT_EXPLAIN_PATHS, "Traversing conditions of i%u (%y) looking for %d IDs...\n", instantiationID, production_name, lNumIdsFounds);
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

            if (lAffectedCondition || ((match_level > 0) && ((*it)->get_level() < match_level)))
            {
                dprint(DT_EXPLAIN_PATHS, "   Found affected condition %u in i%u.  Recording its dependencies...%s\n", (*it)->get_conditionID(), instantiationID,
                    ((match_level > 0) && ((*it)->get_level() < match_level)) ? "Super" : "Local");
                (*it)->create_identity_paths(lInstPath);
            }
            dprint(DT_EXPLAIN_PATHS, "   path_to_base for condition %u in i%u is now: ", (*it)->get_conditionID(), instantiationID);
//            (*it)->print_path_to_base();
            dprint(DT_EXPLAIN_PATHS, "\n");

        }
    }
    if (lInstPath)
    {
        dprint(DT_EXPLAIN_PATHS, "Deleting lInstPath of size %d for instantiation record %u.\n", lInstPath->size(), instantiationID);
        delete lInstPath;
    }
    /* Note:  lIdsFound will be deleted by the action record.  That way they can be
     *        re-used, since they won't change between different chunks explained */
}

void condition_record::create_identity_paths(const inst_record_list* pInstPath)
{
    if (path_to_base)
    {
        /* MToDo | Should allow multiple paths to base. */
        dprint(DT_EXPLAIN_PATHS, "      Condition already has a path to base.  Skipping (%t ^%t %t).\n", condition_tests.id, condition_tests.attr, condition_tests.value);
        return;
    } else
    {
        assert(!path_to_base);
        path_to_base = new inst_record_list();
        (*path_to_base) = (*pInstPath);
        dprint(DT_EXPLAIN_PATHS, "      Condition record copied path_to_base %d = %d.\n", path_to_base->size(), pInstPath->size());
    }
    if (parent_instantiation)
    {
        assert(parent_action);
        dprint(DT_EXPLAIN_PATHS, "      Traversing parent instantiation i%u for condition (%t ^%t %t).\n", parent_instantiation->get_instantiationID(), condition_tests.id, condition_tests.attr, condition_tests.value);
        parent_instantiation->create_identity_paths(path_to_base);
        /* List has current instantiation, which we needed for parent calls but don't need for this condition, so we pop it off */
//        dprint(DT_EXPLAIN_DEP, "   Popping last item from path_to_base for condition %u (%t ^%t %t): ", conditionID, condition_tests.id, condition_tests.attr, condition_tests.value);
//        print_path_to_base();
//        dprint(DT_EXPLAIN_DEP, "\n");
//        path_to_base->pop_back();
    } else {
        dprint(DT_EXPLAIN_PATHS, "      No parent instantiation to traverse for condition (%t ^%t %t).\n", condition_tests.id, condition_tests.attr, condition_tests.value);
    }
}
