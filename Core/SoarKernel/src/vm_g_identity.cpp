#include "variablization_manager.h"
#include "kernel.h"
#include "debug.h"
#include "soar_module.h"
#include "wmem.h"


/* --------------------------------------------------------------------------
                 Get grounding IDs for a WME
 --------------------------------------------------------------------------*/
inline uint64_t get_gid_from_field(const soar_module::identity_triple gt, WME_Field f)
{
    if (f == VALUE_ELEMENT)
        return gt.value;
    else if (f == ATTR_ELEMENT)
        return gt.attr;
    else if (f == ID_ELEMENT)
        return gt.id;

    assert(false);
    return 0;
}

inline uint64_t get_ground_id(agent* thisAgent, wme* w, WME_Field f)
{
    if (!w)
    {
        assert(false);
        return NON_GENERALIZABLE;
    }

    dprint(DT_IDENTITY_PROP, "- g_id requested for %s of %w...\n", field_to_string(f), w);

    uint64_t found_g_id = get_gid_from_field(w->g_ids, f);

    if (found_g_id == 0)
    {
        found_g_id = thisAgent->variablizationManager->get_new_ground_id();
        dprint_noprefix(DT_IDENTITY_PROP, "generating new g_id ");
        if (f == VALUE_ELEMENT)
            w->g_ids.value = found_g_id;
        else if (f == ATTR_ELEMENT)
            w->g_ids.attr = found_g_id;
        else if (f == ID_ELEMENT)
            w->g_ids.id = found_g_id;
    }
    else
    {
        dprint_noprefix(DT_IDENTITY_PROP, "returning existing g_id ");
    }
    dprint_noprefix(DT_IDENTITY_PROP, "%u\n", found_g_id);
    return found_g_id;
}

void Variablization_Manager::add_identity_to_test(test* t, WME_Field default_f)
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
                add_identity_to_test(&ct, default_f);
            }
            break;

        default:
            if ((*t)->identity->grounding_field == NO_ELEMENT)
            {
                (*t)->identity->grounding_field = default_f;
            }

            /* -- Set the grounding id for all variablizable constants, i.e. non short-term identifiers -- */
            Symbol* sym = get_wme_element((*t)->identity->grounding_wme, (*t)->identity->grounding_field);

            /* -- Do not generate identity for identifier symbols.  This is important in other parts of the
             *    chunking code, since it is used to determine whether a constant or identifier was variablized -- */
            if (sym)
            {
                if (!sym->is_sti())
                {
                    (*t)->identity->grounding_id = get_ground_id(thisAgent, (*t)->identity->grounding_wme, (*t)->identity->grounding_field);
                    dprint(DT_IDENTITY_PROP, "- Setting g_id for %y to %i.\n", sym, (*t)->identity->grounding_id);
                    if ((*t)->identity->original_var_id)
                    {
                        if ((*t)->identity->grounding_id != NON_GENERALIZABLE)
                        {
                            dprint(DT_OVAR_MAPPINGS, "Adding original variable mappings entry: o%u(%y) to g%u.\n", (*t)->identity->original_var_id, (*t)->identity->original_var, (*t)->identity->grounding_id);
                            thisAgent->variablizationManager->add_o_id_to_gid_mapping((*t)->identity->original_var_id, (*t)->identity->grounding_id);
                        }
                        else
                        {
//                            dprint(DT_IDENTITY_PROP, "- Not adding ovar to g_id mapping for %y. Marked ungeneralizable (g0).\n", sym);
                        }
                    }
                    else
                    {
//                        dprint(DT_IDENTITY_PROP, "- Not adding ovar to g_id mapping for literal %t. No original variable.\n", (*t));
                    }
                }
                else
                {
//                    dprint(DT_IDENTITY_PROP, "- Skipping %y.  No g_id necessary for STI.\n", sym);
                }
            }
            else
            {
                dprint(DT_IDENTITY_PROP, "- Skipping.  No %s sym retrieved from wme in add_identity_to_test!\n", field_to_string((*t)->identity->grounding_field));
            }
            break;
    }
}

void Variablization_Manager::add_identity_to_negative_test(test t, WME_Field default_f)
{
    assert(t);
    cons* c;


    switch (t->type)
    {
        case DISJUNCTION_TEST:
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            dprint(DT_IDENTITY_PROP, "Will not propagate g_id for NC b/c test type does not take a referent.\n");
            break;

        case CONJUNCTIVE_TEST:
            dprint(DT_IDENTITY_PROP, "Propagating g_ids to NCC...\n");
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                add_identity_to_negative_test(static_cast<test>(c->first), default_f);
            }
            break;

        default:
            if (t->identity->grounding_field == NO_ELEMENT)
            {
                t->identity->grounding_field = default_f;
            }

            /* -- Set the grounding id for all variablizable constants, i.e. non short-term identifiers -- */
            Symbol* sym = t->data.referent;
            Symbol* orig_sym = t->identity->original_var;

            /* -- Do not generate identity for identifier symbols.  This is important in other parts of the
             *    chunking code, since it is used to determine whether a constant or identifier was variablized -- */
            if (sym && orig_sym)
            {
                if (!sym->is_sti() && !sym->is_variable())
                {
                    t->identity->grounding_id = thisAgent->variablizationManager->get_gid_for_o_id(t->identity->original_var_id);
                    dprint(DT_IDENTITY_PROP, "Setting g_id for %y to %i.\n", sym, t->identity->grounding_id);
                }
                else
                {
                    dprint(DT_IDENTITY_PROP, "Could not propagate g_id for NC b/c symbol %y is STI or variable.\n", sym);
                }
            }
            else
            {
                dprint(DT_IDENTITY_PROP, "Will not propagate g_id for NC b/c no referent in add_identity_to_negative_test (or one with no original variable)!\n");
            }
            break;
    }
}

void Variablization_Manager::propagate_identity(condition* cond, bool use_negation_lookup)
{
    condition* c;
    bool has_negative_conds = false;

    dprint_set_indents(DT_IDENTITY_PROP, "          ");
    dprint(DT_IDENTITY_PROP, "Pre-propagation conditions: \n%1", cond);
    dprint_clear_indents(DT_IDENTITY_PROP);
    for (c = cond; c; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            dprint(DT_IDENTITY_PROP, "Propagating identity for positive condition: %l\n", c);

            if (use_negation_lookup)
            {
                /* -- Positive conditions within an NCC.  This was recursive call. -- */
                add_identity_to_negative_test(c->data.tests.id_test, ID_ELEMENT);
                add_identity_to_negative_test(c->data.tests.attr_test, ATTR_ELEMENT);
                add_identity_to_negative_test(c->data.tests.value_test, VALUE_ELEMENT);
            }
            else
            {
                /* -- Either a top-level positive condition or a -- */
                /* -- The last parameter determines whether to cache g_ids for NCCs.  We
                 *    only need to do this when negative conditions exist (has_negative_conds == true)
                 *    and this isn't a recursive call on an NCC list (use_negation_lookup = true) -- */
                add_identity_to_test(&(c->data.tests.id_test), ID_ELEMENT);
                add_identity_to_test(&(c->data.tests.attr_test), ATTR_ELEMENT);
                add_identity_to_test(&(c->data.tests.value_test), VALUE_ELEMENT);
            }
            dprint_set_indents(DT_IDENTITY_PROP, "          ");
            dprint(DT_IDENTITY_PROP, "Condition is now: %l\n", c);
            dprint_clear_indents(DT_IDENTITY_PROP);
        }
        else
        {
            has_negative_conds = true;
        }
    }

    if (has_negative_conds)
    {
        for (c = cond; c; c = c->next)
        {

            if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
            {
                dprint(DT_IDENTITY_PROP, "Propagating identity for NCC.  Calling propagate_identity recursively.\n%c\n", c);

                propagate_identity(c->data.ncc.top, true);
            }
            else if (c->type == NEGATIVE_CONDITION)
            {
                dprint(DT_IDENTITY_PROP, "Propagating identity for negative condition: %l", c);
                add_identity_to_negative_test(c->data.tests.id_test, ID_ELEMENT);
                add_identity_to_negative_test(c->data.tests.attr_test, ATTR_ELEMENT);
                add_identity_to_negative_test(c->data.tests.value_test, VALUE_ELEMENT);
            } else {
                continue;
            }

            dprint_set_indents(DT_IDENTITY_PROP, "          ");
            dprint(DT_IDENTITY_PROP, "Condition is now: %l\n", c);
            dprint_clear_indents(DT_IDENTITY_PROP);

        }
    }
    dprint_set_indents(DT_IDENTITY_PROP, "          ");
    dprint(DT_IDENTITY_PROP, "Post-propagation conditions: \n%1", cond);
    dprint_clear_indents(DT_IDENTITY_PROP);
    thisAgent->variablizationManager->print_tables(DT_IDENTITY_PROP);
}
