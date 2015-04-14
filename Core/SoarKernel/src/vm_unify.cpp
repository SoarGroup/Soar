/*
 * variablization_manager_map.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "wmem.h"
#include "debug.h"

void Variablization_Manager::add_o_id_unification(uint64_t pOld_o_id, uint64_t pNew_o_id)
{
    std::map< uint64_t, uint64_t >::iterator iter = (*o_id_substitution_map).find(pNew_o_id);
    // Do we need to check of old_id is there too?

    if (iter == (*o_id_substitution_map).end())
    {
        dprint(DT_OVAR_PROP, "Did not find o_id to o_id_substitution_map entry for o%u.  Adding %y[o%u] -> %y[o%u].\n", pOld_o_id, get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
        (*o_id_substitution_map)[pOld_o_id] = pNew_o_id;
        print_o_id_substitution_map(DT_OVAR_PROP);
    }
    else
    {
        dprint(DT_OVAR_PROP, "o_id unification (%y[o%u] -> %y[o%u]) already exists.  Adding transitive mapping %y[o%u] -> %y[o%u].\n",
            get_ovar_for_o_id(pNew_o_id), pNew_o_id, get_ovar_for_o_id(iter->second), iter->second,
            get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(iter->second), iter->second);
        (*o_id_substitution_map)[pOld_o_id] = iter->second;
    }
}

uint64_t Variablization_Manager::get_o_id_substitution(uint64_t pO_id)
{
    std::map< uint64_t, uint64_t >::iterator iter = (*o_id_substitution_map).find(pO_id);
    if (iter != (*o_id_substitution_map).end())
    {
        dprint(DT_OVAR_PROP, "...found o%u in o_id_substitution_map table for o%u\n",
            iter->second, pO_id);

        return iter->second;
    } else {
//        dprint(DT_OVAR_PROP, "...did not find o%u in o_id_substitution_map.\n", pO_id);
//        print_o_id_substitution_map(DT_OVAR_PROP);
    }
    return 0;
}

void Variablization_Manager::add_unification_constraint(test* t, test t_add, uint64_t gid)
{
    test new_test = copy_test(thisAgent, t_add);
    new_test->identity->grounding_id = gid;
    add_test(thisAgent, t, new_test);
    dprint(DT_UNIFICATION, "Added unifying equality test between two symbols.  Test is now: %t\n", (*t));
}

void Variablization_Manager::add_unifications_to_test(test* t, WME_Field default_f, goal_stack_level level)
{
    cons* c;

    assert(t);
    assert((*t));

    switch ((*t)->type)
    {
        case DISJUNCTION_TEST:
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            break;

        case CONJUNCTIVE_TEST:
            for (c = (*t)->data.conjunct_list; c != NIL; c = c->rest)
            {
                test ct = static_cast<test>(c->first);
                add_unifications_to_test(&ct, default_f, level);
            }
            break;

        default:

            assert((*t)->identity->grounding_field != NO_ELEMENT);
            /* -- Set the grounding id for all variablizable constants, i.e. non short-term identifiers -- */
            Symbol* sym = get_wme_element((*t)->identity->grounding_wme, (*t)->identity->grounding_field);

            /* -- Do not generate identity for identifier symbols.  This is important in other parts of the
             *    chunking code, since it is used to determine whether a constant or identifier was variablized -- */
            if (sym)
            {
                if (!sym->is_sti())
                {
                    /* -- Check if we ned to add a unifying constraint, b/c this original variable
                     *    already has a different g_id matched to it -- */
                    if (((*t)->identity->grounding_id != NON_GENERALIZABLE) && (*t)->identity->original_var_id)
                    {
                        dprint(DT_UNIFICATION, "Checking if unification needed for o%u(%y) and g%u.\n", (*t)->identity->original_var_id, (*t)->identity->original_var, (*t)->identity->grounding_id);
                        /* MToDo | Consolidate these two calls when we get rid of original vars */
                        uint64_t existing_gid = thisAgent->variablizationManager->get_gid_for_o_id((*t)->identity->original_var_id);
                        if (existing_gid)
                        {
                            if (existing_gid != (*t)->identity->grounding_id)
                            {
                                dprint(DT_UNIFICATION, "- %y(%i) already has g_id %i.  Unification test needed.  Adding it.\n", sym, (*t)->identity->grounding_id, existing_gid);
                                add_unification_constraint(t, *t, existing_gid);
                            } else {
//                                dprint(DT_UNIFICATION, "- %y(%i) already has g_id %i.  No unification test needed.\n", sym, (*t)->identity->grounding_id, existing_gid);
                            }
                        } else {
//                            thisAgent->variablizationManager->print_ovar_gid_propogation_table(DT_UNIFICATION);
//                            thisAgent->variablizationManager->print_o_id_tables(DT_UNIFICATION);
                            dprint(DT_UNIFICATION, "- %y o%u(%y, g%u)had no o_id to g_id mapping, so must be ungrounded.  No unification test needed.\n", sym, (*t)->identity->original_var_id, (*t)->identity->original_var, (*t)->identity->grounding_id);
                        }
                    } else {
//                        dprint(DT_UNIFICATION, "- Not adding ovar to g_id mapping for %y. %s.\n", sym,
//                            ((*t)->identity->grounding_id == NON_GENERALIZABLE) ? "Marked ungeneralizable" : "No original var");
                    }
                } else {
//                    dprint(DT_UNIFICATION, "- Skipping %y.  No g_id necessary for STI.\n", sym);
                }
            } else {
                dprint(DT_UNIFICATION, "- Skipping.  No %s sym retrieved from wme in add_identity_and_unifications_to_test!\n", field_to_string((*t)->identity->grounding_field));
            }
            break;
    }
    /* -- We no longer need the wme and didn't increase refcount, so discard reference -- */
//    (*t)->identity->grounding_wme = NULL;
}

void Variablization_Manager::add_unifications(condition* cond, goal_stack_level level)
{
    condition* c;

    dprint(DT_UNIFICATION, "Looking for unifications...\n");
    thisAgent->variablizationManager->print_o_id_tables(DT_UNIFICATION);
    thisAgent->variablizationManager->print_variablization_tables(DT_UNIFICATION);
    for (c = cond; c; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            dprint(DT_UNIFICATION, "Looking for unifications in condition: %l\n", c);
            add_unifications_to_test(&(c->data.tests.id_test), ID_ELEMENT, level);
            add_unifications_to_test(&(c->data.tests.attr_test), ATTR_ELEMENT, level);
            add_unifications_to_test(&(c->data.tests.value_test), VALUE_ELEMENT, level);
            dprint_set_indents(DT_UNIFICATION, "          ");
            dprint(DT_UNIFICATION, "After unifications, condition is now: %l\n", c);
            dprint_clear_indents(DT_UNIFICATION);
        }
    }
}
