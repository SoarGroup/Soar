/*
 * variablization_manager_map.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "agent.h"
#include "instantiation.h"
#include "condition.h"
#include "preference.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "working_memory.h"
#include "rhs.h"
#include "debug.h"

bool Explanation_Based_Chunker::in_null_identity_set(test t)
{
    std::unordered_map< uint64_t, uint64_t >::iterator iter = (*unification_map).find(t->identity);
    if (iter != (*unification_map).end())
    {
        dprint(DT_UNIFICATION, "...found variablization unification o%u -> o%u\n",
            t->identity, iter->second);

        return (iter->second == NULL_IDENTITY_SET);
    }
    return (t->identity == NULL_IDENTITY_SET);
}

void Explanation_Based_Chunker::unify_identity(test t)
{
    if (!m_learning_on) return;
    std::unordered_map< uint64_t, uint64_t >::iterator iter = (*unification_map).find(t->identity);
    if (iter != (*unification_map).end())
    {
        dprint(DT_UNIFICATION, "...found variablization unification o%u -> o%u\n",
            t->identity, iter->second);

        t->identity = iter->second;
    }
}

void Explanation_Based_Chunker::update_unification_table(uint64_t pOld_o_id, uint64_t pNew_o_id, uint64_t pOld_o_id_2)
{
    std::unordered_map< uint64_t, uint64_t >::iterator iter;

    for (iter = unification_map->begin(); iter != unification_map->end(); ++iter)
    {

        if ((iter->second == pOld_o_id) || (pOld_o_id_2 && (iter->second == pOld_o_id_2)))
        {
            dprint(DT_UNIFICATION, "...found secondary o_id unification mapping that needs updated: o%u = o%u -> o%u = o%u.\n", iter->first, iter->second, iter->first, pNew_o_id );
            (*unification_map)[iter->first] = pNew_o_id;
        }
    }
}

void Explanation_Based_Chunker::add_identity_unification(uint64_t pOld_o_id, uint64_t pNew_o_id)
{
    assert(pOld_o_id);
    if (pOld_o_id == pNew_o_id)
    {
        dprint(DT_UNIFICATION, "Attempting to unify identical conditions %y[o%u].  Skipping.\n", pNew_o_id);
        return;
    }

    std::unordered_map< uint64_t, uint64_t >::iterator iter;
    uint64_t newID;

    if (pNew_o_id == 0)
    {
        /* Do not check if a unification already exists if we're propagating back a literalization */
        dprint(DT_UNIFICATION, "Adding literalization substitution in o_id to o_id_substitution_map for o%u.  Adding %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(pOld_o_id), pOld_o_id);
        newID = 0;
    } else {
        /* See if a unification already exists for the new identity propagating back*/
        iter = (*unification_map).find(pNew_o_id);

        if (iter == (*unification_map).end())
        {
            /* Map all cases of this identity with its parent identity */
            dprint(DT_UNIFICATION, "Did not find o_id to o_id_substitution_map entry for o%u.  Adding %y[o%u] -> %y[o%u].\n", pNew_o_id, get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
            newID = pNew_o_id;
            dprint(DT_UNIFICATION, "Old identity propagation map:\n");
            dprint_o_id_substitution_map(DT_UNIFICATION);
        }
        else
        {
            /* Map all cases of what this identity is already remapped to with its parent identity */
            dprint(DT_UNIFICATION, "o_id unification (%y[o%u] -> %y[o%u]) already exists.  Adding transitive mapping %y[o%u] -> %y[o%u].\n",
                get_ovar_for_o_id(pNew_o_id), pNew_o_id, get_ovar_for_o_id(iter->second), iter->second,
                get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(iter->second), iter->second);
            newID = iter->second;
            iter = (*unification_map).find(pOld_o_id);
            if ((iter != (*unification_map).end()) && (iter->second == newID))
            {
                dprint(DT_UNIFICATION, "The unification %y[o%u] -> %y[o%u] already exists.  Skipping.\n", get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(newID), newID);
                return;
            }
        }
    }

    /* See if a unification already exists for the identity being replaced in this instantiation*/
    iter = (*unification_map).find(pOld_o_id);
    uint64_t existing_mapping;
    if (iter != (*unification_map).end())
    {
        existing_mapping = iter->second;
        if (existing_mapping == 0)
        {
            if (newID != 0)
            {
                /* The existing identity we're unifying with is already literalized from a different trace.  So,
                 * literalize any tests with identity of parent in this trace */
                dprint(DT_UNIFICATION, "Literalization exists for o%u.  Propagating literalization substitution with %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
                (*unification_map)[newID] = 0;
                update_unification_table(newID, 0);
            } else {
                dprint(DT_UNIFICATION, "Literalizing something already literalized o%u.  Skipping %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
            }
        } else {
            if (newID == 0)
            {
                /* The existing identity we're literalizing is already unified with another identity from
                 * a different trace.  So, literalize the identity, that it is already remapped to.*/
                dprint(DT_UNIFICATION, "Unification with another identity exists for o%u.  Propagating literalization substitution with %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(existing_mapping), existing_mapping);
                (*unification_map)[existing_mapping] = 0;
                update_unification_table(existing_mapping, 0, pOld_o_id);
            } else {
                /* The existing identity we're unifying with is already unified with another identity from
                 * a different trace.  So, unify the identity that it is already remapped to with identity
                 * of the parent in this trace */
                dprint(DT_UNIFICATION, "Unification with another identity exists for o%u.  Adding %y[o%u] -> %y[o%u].\n", pOld_o_id, get_ovar_for_o_id(existing_mapping), existing_mapping, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
                (*unification_map)[pNew_o_id] = existing_mapping;
                (*unification_map)[newID] = existing_mapping;
                update_unification_table(newID, existing_mapping);
                update_unification_table(pNew_o_id, existing_mapping);
            }
        }
    } else {
        (*unification_map)[pOld_o_id] = newID;
        update_unification_table(pOld_o_id, newID);
    }

    /* Unify identity in this instantiation with final identity */
    dprint(DT_UNIFICATION, "New identity propagation map:\n");
    dprint_o_id_substitution_map(DT_UNIFICATION);
}

bool Explanation_Based_Chunker::unify_backtraced_dupe_conditions(condition* ground_cond, condition* new_cond)
{
    if (!m_learning_on) return true;

    dprint(DT_IDENTITY_PROP, "Adding identity mappings for dupe match condition: %l from %l\n", new_cond, ground_cond);
    test new_cond_id, new_cond_attr, new_cond_value, ground_cond_id, ground_cond_attr, ground_cond_value;

    new_cond_id = new_cond_attr = new_cond_value = NULL;
    if (new_cond->data.tests.id_test->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_IDENTITY_PROP, "Condition has additional constraints.  Not unifying.\n");
//        return false;
        new_cond_id = new_cond->data.tests.id_test->eq_test;
    } else {
        new_cond_id = new_cond->data.tests.id_test;
        assert(new_cond_id->type == EQUALITY_TEST);
    }
    if (new_cond->data.tests.attr_test->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_IDENTITY_PROP, "Condition has additional constraints.  Not unifying.\n");
//        return false;
        new_cond_attr = new_cond->data.tests.attr_test->eq_test;
    } else {
        new_cond_attr = new_cond->data.tests.attr_test;
        assert(new_cond_attr->type == EQUALITY_TEST);
    }
    if (new_cond->data.tests.value_test->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_IDENTITY_PROP, "Condition has additional constraints.  Not unifying.\n");
//        return false;
        new_cond_value = new_cond->data.tests.value_test->eq_test;
    } else {
        new_cond_value = new_cond->data.tests.value_test;
        assert(new_cond_value->type == EQUALITY_TEST);
    }

    if (!new_cond_id && !new_cond_attr && !new_cond_value)
    {
        dprint(DT_IDENTITY_PROP, "Condition has additional constraints.  Not unifying.\n");
        return false;
    }

    bool mismatched_literal = false;
    ground_cond_id = ground_cond->data.tests.id_test->eq_test;
    ground_cond_attr = ground_cond->data.tests.attr_test->eq_test;
    ground_cond_value = ground_cond->data.tests.value_test->eq_test;

    if (!new_cond_id->identity || !ground_cond_id->identity)
    {
        if (new_cond_id->identity != ground_cond_id->identity)
        {
            mismatched_literal = true;
        }
    }
    if (!mismatched_literal && (!new_cond_attr->identity || !ground_cond_attr->identity))
    {
        if (new_cond_attr->identity != ground_cond_attr->identity)
        {
            mismatched_literal = true;
        }
    }
    if (!mismatched_literal && (!new_cond_value->identity || !ground_cond_value->identity))
    {
        if (new_cond_value->identity != ground_cond_value->identity)
        {
            mismatched_literal = true;
        }
    }


    if (mismatched_literal)
    {
        dprint(DT_IDENTITY_PROP, "One of the conditions has a literal not in the other condition.  Not unifying.\n");
        return false;
    }
    /* We now know either both conds are literal or both have identities */
    if (new_cond_id->identity)
    {
        add_identity_unification(new_cond_id->identity, ground_cond_id->identity);
    }
    if (new_cond_attr->identity)
    {
        add_identity_unification(new_cond_attr->identity, ground_cond_attr->identity);
    }
    if (new_cond_value->identity)
    {
        add_identity_unification(new_cond_value->identity, ground_cond_value->identity);
    }
    dprint_o_id_substitution_map(DT_IDENTITY_PROP);
    return true;
}

void Explanation_Based_Chunker::literalize_RHS_function_args(const rhs_value rv)
{
    /* Assign identities of all arguments in rhs fun call to null identity set*/
    list* fl = rhs_value_to_funcall_list(rv);
    cons* c;

    for (c = fl->rest; c != NIL; c = c->rest)
    {
        dprint(DT_RHS_VARIABLIZATION, "Literalizing RHS function argument %r\n", static_cast<char*>(c->first));
        if (rhs_value_is_funcall(static_cast<char*>(c->first)))
        {
            literalize_RHS_function_args(static_cast<char*>(c->first));
        } else {
            assert(rhs_value_is_symbol(static_cast<char*>(c->first)));
            rhs_symbol rs = rhs_value_to_rhs_symbol(static_cast<char*>(c->first));
            if (rs->o_id && !rs->referent->is_sti())
            {
                add_identity_unification(rs->o_id, 0);
            }
        }
    }
}

void Explanation_Based_Chunker::unify_backtraced_conditions(condition* parent_cond,
                                                         const identity_triple o_ids_to_replace,
                                                         const rhs_triple rhs_funcs)
{
    if (!m_learning_on) return;
    test lId = 0, lAttr = 0, lValue = 0;
    lId = parent_cond->data.tests.id_test->eq_test;
    lAttr = parent_cond->data.tests.attr_test->eq_test;
    lValue = parent_cond->data.tests.value_test->eq_test;

    if (!lId->data.referent->is_sti() && o_ids_to_replace.id)
    {
        if (lId->identity)
        {
            dprint(DT_IDENTITY_PROP, "Found an o_id to replace for identifier element: %y [o%u] -> %y [o%u]\n", get_ovar_for_o_id(o_ids_to_replace.id), o_ids_to_replace.id,
                get_ovar_for_o_id(lId->identity), lId->identity);
        } else {
            dprint(DT_IDENTITY_PROP, "Found an o_id to literalize for identifier element: %y [o%u] -> %t\n", get_ovar_for_o_id(o_ids_to_replace.id), o_ids_to_replace.id, lId);
        }
        add_identity_unification(o_ids_to_replace.id, lId->identity);
        dprint_o_id_substitution_map(DT_IDENTITY_PROP);
    }
    else if (rhs_value_is_funcall(rhs_funcs.id))
    {
        literalize_RHS_function_args(rhs_funcs.id);
    }
    else
    {
        dprint(DT_IDENTITY_PROP, "Did not unify because %s%s\n", lId->data.referent->is_sti() ? "is STI " : "", !o_ids_to_replace.id ? "RHS pref is literal " : "");
    }
    if (!lAttr->data.referent->is_sti() && o_ids_to_replace.attr)
    {
        if (lAttr->identity)
        {
            dprint(DT_IDENTITY_PROP, "Found an o_id to replace for attribute element: %y [o%u] -> %y [o%u]\n", get_ovar_for_o_id(o_ids_to_replace.attr), o_ids_to_replace.attr,
                get_ovar_for_o_id(lAttr->identity), lAttr->identity);
        } else {
            dprint(DT_IDENTITY_PROP, "Found an o_id to literalize for attribute element: %y [o%u] -> %t\n", get_ovar_for_o_id(o_ids_to_replace.attr), o_ids_to_replace.attr, lAttr);
        }
        add_identity_unification(o_ids_to_replace.attr, lAttr->identity);
        dprint_o_id_substitution_map(DT_IDENTITY_PROP);
    }
    else if (rhs_value_is_funcall(rhs_funcs.attr))
    {
        literalize_RHS_function_args(rhs_funcs.attr);
    }
    else
    {
        dprint(DT_IDENTITY_PROP, "Did not unify because %s%s\n", lAttr->data.referent->is_sti() ? "is STI " : "", !o_ids_to_replace.attr ? "RHS pref is literal " : "");
    }
    if (!lValue->data.referent->is_sti() && o_ids_to_replace.value)
    {
        if (lValue->identity)
        {
            dprint(DT_IDENTITY_PROP, "Found an o_id to replace for value element: %y [o%u] -> %y [o%u]\n", get_ovar_for_o_id(o_ids_to_replace.value), o_ids_to_replace.value,
                get_ovar_for_o_id(lValue->identity), lValue->identity);
        } else {
            dprint(DT_IDENTITY_PROP, "Found an o_id to literalize for value element: %y [o%u] -> %t\n", get_ovar_for_o_id(o_ids_to_replace.value), o_ids_to_replace.value, lValue);
        }
        add_identity_unification(o_ids_to_replace.value, lValue->identity);
        dprint_o_id_substitution_map(DT_IDENTITY_PROP);
    }
    else if (rhs_value_is_funcall(rhs_funcs.value))
    {
        literalize_RHS_function_args(rhs_funcs.value);
    }
    else
    {
        dprint(DT_IDENTITY_PROP, "Did not unify because %s%s\n", lValue->data.referent->is_sti() ? "is STI " : "", !o_ids_to_replace.value ? "RHS pref is literal " : "");
    }
}
