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

void Variablization_Manager::clear_o_id_to_ovar_debug_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    o_id_to_ovar_debug_map->clear();
}

void Variablization_Manager::clear_o_id_substitution_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    o_id_substitution_map->clear();
}

void Variablization_Manager::clear_ovar_to_o_id_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    /* -- Clear original variable map -- */
    for (std::map< Symbol*, std::map< uint64_t, uint64_t > >::iterator it = (*ovar_to_o_id_map).begin(); it != (*ovar_to_o_id_map).end(); ++it)
    {
        dprint(DT_VARIABLIZATION_MANAGER, "Clearing %y\n", it->first);
        symbol_remove_ref(thisAgent, it->first);
    }
    ovar_to_o_id_map->clear();
}

uint64_t Variablization_Manager::get_existing_o_id(Symbol* orig_var, uint64_t inst_id)
{
    std::map< Symbol*, std::map< uint64_t, uint64_t > >::iterator iter_sym;
    std::map< uint64_t, uint64_t >::iterator iter_inst;

//    dprint(DT_OVAR_PROP, "...looking for symbol %y\n", orig_var);
    iter_sym = ovar_to_o_id_map->find(orig_var);
    if (iter_sym != ovar_to_o_id_map->end())
    {
//        dprint(DT_OVAR_PROP, "...Found.  Looking  for instantiation id %u\n", inst_id);
        iter_inst = iter_sym->second.find(inst_id);
        if (iter_inst != iter_sym->second.end())
        {
            dprint(DT_OVAR_PROP, "...get_existing_o_id found mapping for %y in instantiation %u.  Returning existing o_id %u\n", orig_var, inst_id, iter_inst->second);
            return iter_inst->second;
        }
    }

    dprint(DT_OVAR_PROP, "...get_existing_o_id did not find mapping for %y in instantiation %u.\n", orig_var, inst_id);
    return 0;

}

uint64_t Variablization_Manager::get_or_create_o_id(Symbol* orig_var, uint64_t inst_id)
{
    int64_t existing_o_id = 0;

    existing_o_id = get_existing_o_id(orig_var, inst_id);
    if (!existing_o_id)
    {
        ++ovar_id_counter;
        (*ovar_to_o_id_map)[orig_var][inst_id] = ovar_id_counter;
        symbol_add_ref(thisAgent, orig_var);
        (*o_id_to_ovar_debug_map)[ovar_id_counter] = orig_var;
        dprint(DT_OVAR_PROP, "...Created and returning new o_id %u for orig var %y in instantiation %u.\n", ovar_id_counter, orig_var, inst_id);
        return ovar_id_counter;
    } else {
        return existing_o_id;
    }
}

Symbol * Variablization_Manager::get_ovar_for_o_id(uint64_t o_id)
{
    if (o_id == 0) return NULL;

//    dprint(DT_OVAR_PROP, "...looking for ovar for o_id %u...", o_id);
    std::map< uint64_t, Symbol* >::iterator iter = o_id_to_ovar_debug_map->find(o_id);
    if (iter != o_id_to_ovar_debug_map->end())
    {
//        dprint_noprefix(DT_OVAR_PROP, "found.  Returning %y\n", iter->second);
        return iter->second;
    }
//    dprint_noprefix(DT_OVAR_PROP, "not found.  Returning NULL.\n");
    return NULL;

}

void Variablization_Manager::add_unification_constraint(test* t, test t_add, uint64_t gid)
{
    test new_test = copy_test(thisAgent, t_add);
    new_test->identity->grounding_id = gid;
    add_test(thisAgent, t, new_test);
    dprint(DT_UNIFICATION, "Added unifying equality test between two symbols.  Test is now: %t\n", (*t));
}

void Variablization_Manager::add_unifications_to_test(test* t, WME_Field default_f, goal_stack_level level, uint64_t pI_id)
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
                add_unifications_to_test(&ct, default_f, level, pI_id);
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
                    assert((*t)->identity->grounding_id);
                    /* -- Check if we ned to add a unifying constraint, b/c this original variable
                     *    already has a different g_id matched to it -- */
                    if (((*t)->identity->grounding_id != NON_GENERALIZABLE) && (*t)->identity->original_var)
                    {
                        dprint(DT_UNIFICATION, "Checking original variable mappings entry for %y to %u.\n", (*t)->identity->original_var, (*t)->identity->grounding_id);
                        /* MToDo | Consolidate these two calls when we get rid of original vars */
                        uint64_t existing_gid = thisAgent->variablizationManager->add_orig_var_to_gid_mapping((*t)->identity->original_var, (*t)->identity->grounding_id, pI_id);
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
                            dprint(DT_UNIFICATION, "- %y(%i) had no ovar to g_id mapping, so new one created.  No unification test needed.\n", sym, (*t)->identity->grounding_id, existing_gid);
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

void Variablization_Manager::add_unifications(condition* cond, goal_stack_level level, uint64_t pI_id)
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
            add_unifications_to_test(&(c->data.tests.id_test), ID_ELEMENT, level, pI_id);
            add_unifications_to_test(&(c->data.tests.attr_test), ATTR_ELEMENT, level, pI_id);
            add_unifications_to_test(&(c->data.tests.value_test), VALUE_ELEMENT, level, pI_id);
            dprint_set_indents(DT_UNIFICATION, "          ");
            dprint(DT_UNIFICATION, "Condition is now: %l\n", c);
            dprint_clear_indents(DT_UNIFICATION);
        }
    }
}
