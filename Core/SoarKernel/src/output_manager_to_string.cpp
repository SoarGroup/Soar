/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
             output_manager_print.cpp

   @brief output_manager_print.cpp provides many functions to
   print Soar data structures.  Many were originally written
   for debugging purposes and are only fount in print commands.

------------------------------------------------------------------ */

#include "rhs.h"
#include "print.h"
#include "agent.h"
#include "instantiations.h"
#include "rete.h"
#include "reorder.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "output_manager.h"
#include "output_manager_db.h"
#include "prefmem.h"
#include "wmem.h"
#include "soar_instance.h"
#include "test.h"

char* Output_Manager::wme_to_string(agent* thisAgent, wme* w, char* dest, size_t dest_size, bool pOnlyWithIdentity)
{
    assert(thisAgent && dest && w);
    char* ch = dest;

    bool lFoundIdentity;
    if (pOnlyWithIdentity)
    {
        grounding_info* g = w->ground_id_list;
        lFoundIdentity = false;
        for (; g && !lFoundIdentity; g = g->next)
        {
            if ((g->grounding_id[0] > 0) || (g->grounding_id[1] > 0) || (g->grounding_id[2] > 0))
            {
                lFoundIdentity = true;
            }
        }
    }
    if (!pOnlyWithIdentity || (pOnlyWithIdentity && lFoundIdentity))
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "(t%u: %y ^%y %y%s",
            w->timetag, w->id, w->attr, w->value,
            (w->acceptable ? " +) [" : ") ["));
        while (*ch) ch++;

        grounding_info* g = w->ground_id_list;
        for (; g; g = g->next)
        {
            sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "l%d: g%u g%u g%u", g->level, g->grounding_id[0], g->grounding_id[1], g->grounding_id[2]);
            while (*ch) ch++;
            if (g->next)
            {
                strcpy(ch, ", ");
                ch += 2;
            }
        }
        *(ch++) = ']';
        *ch = 0;
    }
    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::WM_to_string(agent* thisAgent, char* dest, size_t dest_size, bool pOnlyWithIdentity)
{
    assert(thisAgent && dest);
    char* ch = dest;

    sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "--------------------------- WMEs --------------------------\n");
    while (*ch) ch++;
    for (wme* w = m_defaultAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "          %w\n", w);
        while (*ch) ch++;
    }
    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

/* UITODO| Make this method of Test */
const char* Output_Manager::test_type_to_string_brief(byte test_type, const char* equality_str)
{
    switch (test_type)
    {
        case NOT_EQUAL_TEST:
            return "!= ";
            break;
        case LESS_TEST:
            return "< ";
            break;
        case GREATER_TEST:
            return "> ";
            break;
        case LESS_OR_EQUAL_TEST:
            return "<= ";
            break;
        case GREATER_OR_EQUAL_TEST:
            return ">= ";
            break;
        case SAME_TYPE_TEST:
            return "<=> ";
            break;
        case CONJUNCTIVE_TEST:
            return "{ }";
            break;
        case GOAL_ID_TEST:
            return "IS_G_ID ";
            break;
        case IMPASSE_ID_TEST:
            return "IS_IMPASSE ";
            break;
        case EQUALITY_TEST:
            return equality_str;
            break;
    }
    return "UNDEFINED TEST TYPE";
}

/* UITODO| Make this method of Test */
char* Output_Manager::test_to_string(test t, char* dest, size_t dest_size, bool show_equality)
{
    cons* c;
    char* ch;

    if (!dest)
    {
        dest = get_printed_output_string();
        dest_size = output_string_size; /* from agent.h */
    }
    ch = dest;

    if (test_is_blank(t))
    {
        strcpy(dest, "[BLANK TEST]");   /* this should never get executed */
        dest[dest_size - 1] = 0; /* ensure null termination */
        return dest;
    }

    switch (t->type)
    {
        case EQUALITY_TEST:
            if (show_equality)
            {
                strcpy(ch, "= ");
                ch += 2;
                t->data.referent->to_string(true, ch, dest_size - (ch - dest));
            }
            else
            {
                t->data.referent->to_string(true, dest, dest_size);
            }
            break;
        case NOT_EQUAL_TEST:
            strcpy(ch, "<> ");
            ch += 3;
            t->data.referent->to_string(true, ch, dest_size - (ch - dest));
            break;
        case LESS_TEST:
            strcpy(ch, "< ");
            ch += 2;
            t->data.referent->to_string(true, ch, dest_size - (ch - dest));
            break;
        case GREATER_TEST:
            strcpy(ch, "> ");
            ch += 2;
            t->data.referent->to_string(true, ch, dest_size - (ch - dest));
            break;
        case LESS_OR_EQUAL_TEST:
            strcpy(ch, "<= ");
            ch += 3;
            t->data.referent->to_string(true, ch, dest_size - (ch - dest));
            break;
        case GREATER_OR_EQUAL_TEST:
            strcpy(ch, ">= ");
            ch += 3;
            t->data.referent->to_string(true, ch, dest_size - (ch - dest));
            break;
        case SAME_TYPE_TEST:
            strcpy(ch, "<=> ");
            ch += 4;
            t->data.referent->to_string(true, ch, dest_size - (ch - dest));
            break;
        case DISJUNCTION_TEST:
            strcpy(ch, "<< ");
            ch += 3;
            for (c = t->data.disjunction_list; c != NIL; c = c->rest)
            {
                static_cast<symbol_struct*>(c->first)->to_string(true, ch, dest_size - (ch - dest));
                while (*ch)
                {
                    ch++;
                }
                *(ch++) = ' ';
            }
            strcpy(ch, ">>");
            ch += 2;
            *ch = 0;
            break;
        case CONJUNCTIVE_TEST:
            strcpy(ch, "{ ");
            ch += 2;
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                test_to_string(static_cast<test>(c->first), ch, dest_size - (ch - dest));
                while (*ch)
                {
                    ch++;
                }
                *(ch++) = ' ';
            }
            strcpy(ch, "}");
            ch++;
            *ch = 0;
            break;
        case GOAL_ID_TEST:
            strcpy(dest, "[GOAL ID TEST]");  /* this should never get executed */
            break;
        case IMPASSE_ID_TEST:
            strcpy(dest, "[IMPASSE ID TEST]");  /* this should never get executed */
            break;
        default:
            strcpy(ch, "INVALID TEST!");   /* this should never get executed */
            break;
    }
    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}


char* Output_Manager::condition_cons_to_string(agent* thisAgent, cons* c, char* dest, size_t dest_size)
{
    char* ch=dest;

    while (c)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s: %l\n", m_pre_string, static_cast<condition_struct*>(c->first));
        while (*ch) ch++;
        c = c->rest;
    }
    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::condition_to_string(agent* thisAgent, condition* cond, char* dest, size_t dest_size)
{
    char* ch=dest;

    if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        if (m_print_actual)
        {
            sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "(%t %s^%t %t)",
            cond->data.tests.id_test,
                (cond->type == NEGATIVE_CONDITION) ? "- ": NULL,
            cond->data.tests.attr_test, cond->data.tests.value_test);
            while (*ch) ch++;
        }
        if (m_print_original) {
            sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s(%y %s^%y %y)",
                (m_print_actual) ? ", " : NULL,
                cond->data.tests.id_test->identity->original_var,
                (cond->type == NEGATIVE_CONDITION) ? "- ": NULL,
                cond->data.tests.attr_test->identity->original_var, cond->data.tests.value_test->identity->original_var);
            while (*ch) ch++;
        }
        if (m_print_identity) {
            sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s(g%u %s^g%u g%u)",
                (m_print_actual || m_print_original) ? ", " : NULL,
                cond->data.tests.id_test->identity->grounding_id,
                (cond->type == NEGATIVE_CONDITION) ? "- ": NULL,
                cond->data.tests.attr_test->identity->grounding_id, cond->data.tests.value_test->identity->grounding_id);
            while (*ch) ch++;
        }
        *(ch++) = 0;
    }
    else
    {
        sprinta_sf(thisAgent, dest, dest_size, "-{\n%c2}", cond->data.ncc.top);
    }
    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::condition_list_to_string(agent* thisAgent, condition* top_cond, char* dest, size_t dest_size)
{
    condition* cond;
    char* ch=dest;
    int64_t count = 0;

    for (cond = top_cond; cond != NIL; cond = cond->next)
    {
        assert(cond != cond->next);
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s%i: %l\n", m_pre_string, ++count, cond);
        while (*ch) ch++;
    }

    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::rhs_value_to_string(agent* thisAgent, rhs_value rv, char* dest, size_t dest_size, struct token_struct* tok, wme* w)
{
    assert(thisAgent && dest);
    char* ch = dest;

    rhs_symbol rsym = NIL;
    Symbol* sym = NIL;
    cons* c;
    list* fl;
    rhs_function* rf;
    int i;
    if (!rv)
    {
        *(ch++) = '#';
        *ch = 0;
    }
    else if (rhs_value_is_unboundvar(rv))
    {
        /* -- unbound variable -- */
        strcpy(dest, "<unbound-var>");
        dest[dest_size - 1] = 0; /* ensure null termination */
    }
    else if (rhs_value_is_symbol(rv))
    {

        /* -- rhs symbol -- */
        rsym = rhs_value_to_rhs_symbol(rv);
        if (this->m_print_actual)
        {
            sprinta_sf(thisAgent, dest, dest_size, "%y", rsym->referent);
        } else if (m_print_original) {
            sprinta_sf(thisAgent, dest, dest_size, "%y", rsym->original_rhs_variable);
        } else if (m_print_identity) {
            sprinta_sf(thisAgent, dest, dest_size, "%u", rsym->g_id);
        }
    }
    else if (rhs_value_is_reteloc(rv))
    {
        /* -- rete location (cannot get symbol without token information) -- */
        if (tok && w)
        {
            sym = get_symbol_from_rete_loc(
                      rhs_value_to_reteloc_levels_up(rv),
                      rhs_value_to_reteloc_field_num(rv), tok, w);
            sprinta_sf(thisAgent, dest, dest_size, "%y (reteloc)", sym);
        }
        else
        {
            strcpy(dest, "(rete-loc no tok/w)");
            dest[dest_size - 1] = 0; /* ensure null termination */
        }
    }
    else
    {
        /* -- function call -- */
        fl = rhs_value_to_funcall_list(rv);
        rf = static_cast<rhs_function_struct*>(fl->first);

//        print_sf("(");
//        if (!strcmp(rf->name->sc->name, "+"))
//        {
//            print_sf("+");
//        }
//        else if (!strcmp(rf->name->sc->name, "-"))
//        {
//            print_sf("-");
//        }
//        else
//        {
//            print_sf("(%y", rf->name);
//        }
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "(%y", rf->name);
        while (*ch) ch++;

        for (c = fl->rest; c != NIL; c = c->rest)
        {
            *(ch++) = ' ';
            rhs_value_to_string(thisAgent, static_cast<rhs_value>(c->first), ch, dest_size - (ch - dest), tok, w);
            while (*ch) ch++;
        }
        *(ch++) = ')';
        *(ch++) = 0;
    }
    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::action_to_string(agent* thisAgent, action* a, char* dest, size_t dest_size)
{
    assert(thisAgent && dest && a);
    char* ch = dest;

    if (a->type == FUNCALL_ACTION)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s(funcall ", m_pre_string);
        while (*ch) ch++;
        rhs_value_to_string(thisAgent, a->value, ch, dest_size - (ch - dest), NULL, NULL);
        while (*ch) ch++;
        strcpy(ch, ")");
    }
    else
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s(", m_pre_string);
        rhs_value_to_string(thisAgent, a->id, ch, dest_size - (ch - dest), NULL, NULL);
        while (*ch) ch++;
        strcpy(ch, " ^");
        ch += 2;
        rhs_value_to_string(thisAgent, a->attr, ch, dest_size - (ch - dest), NULL, NULL);
        while (*ch) ch++;
        strcpy(ch++, " ");
        rhs_value_to_string(thisAgent, a->value, ch, dest_size - (ch - dest), NULL, NULL);
        while (*ch) ch++;
        strcpy(ch, " ref: ");
        ch += 6;
        rhs_value_to_string(thisAgent, a->referent, ch, dest_size - (ch - dest), NULL, NULL);
        while (*ch) ch++;
        strcpy(ch, ")");
    }
    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::action_list_to_string(agent* thisAgent, action* action_list, char* dest, size_t dest_size)
{
    assert(thisAgent && dest && action_list);
    char* ch = dest;

    action* a = NIL;

    for (a = action_list; a != NIL; a = a->next)
    {
        action_to_string(thisAgent, a, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        *(ch++) = '\n';
        *ch = 0;

    }

    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::pref_to_string(agent* thisAgent, preference* pref, char* dest, size_t dest_size)
{
    assert(thisAgent && dest && pref);

    if (m_print_actual)
    {
        sprinta_sf(thisAgent, dest, dest_size, "%s(%y ^%y %y) %c %y%s", m_pre_string, pref->id, pref->attr, pref->value,
            preference_to_char(pref->type),
            (m_print_actual && preference_is_binary(pref->type)) ? pref->referent : NULL,
            (pref->o_supported) ? " :O " : NULL);
        return dest;
    }
    else if (m_print_original)
    {
        sprinta_sf(thisAgent, dest, dest_size, "%s(%y ^%y %y) %c %y%s", m_pre_string,
            pref->original_symbols.id, pref->original_symbols.attr, pref->original_symbols.value,
            preference_to_char(pref->type),
            (m_print_actual && preference_is_binary(pref->type)) ? pref->referent : NULL,
            (pref->o_supported) ? " :O " : NULL);
        return dest;
    }
    else if (m_print_identity)
    {
        sprinta_sf(thisAgent, dest, dest_size, "%s(g%u ^g%u g%u) %c %y%s", m_pre_string,
            pref->g_ids.id, pref->g_ids.attr, pref->g_ids.value,
            preference_to_char(pref->type),
            (m_print_actual && preference_is_binary(pref->type)) ? pref->referent : NULL,
            (pref->o_supported) ? " :O " : NULL);
        return dest;
    }
    return NULL;
}

char* Output_Manager::preflist_inst_to_string(agent* thisAgent, preference* top_pref, char* dest, size_t dest_size)
{
    char* ch = dest;

    for (preference* pref = top_pref; pref != NIL;)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%p\n", pref);
        while (*ch) ch++;
        pref = pref->inst_next;
    }
    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;

}

char* Output_Manager::preflist_result_to_string(agent* thisAgent, preference* top_pref, char* dest, size_t dest_size)
{
    char* ch = dest;

    for (preference* pref = top_pref; pref != NIL;)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%p\n", pref);
        while (*ch) ch++;
        pref = pref->next_result;
    }
    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

void Output_Manager::debug_print_production(TraceMode mode, production* prod)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    if (prod)
    {
        print_production(m_defaultAgent, prod, true);
    }
}

char* Output_Manager::cond_prefs_to_string(agent* thisAgent, condition* top_cond, preference* top_pref, char* dest, size_t dest_size)
{
    char* ch=dest;

    /* MToDo | Only print headers and latter two if that setting is on */
    if (m_print_actual)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "--------------------------- Match --------------------------\n");
        while (*ch) ch++;
        set_print_test_format(true, false, false);
        condition_list_to_string(thisAgent, top_cond, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s-->\n", m_pre_string);
        while (*ch) ch++;
        preflist_inst_to_string(thisAgent, top_pref, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        clear_print_test_format();
    }
    if (m_print_original)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "-------------------------- Original ------------------------\n");
        while (*ch) ch++;
        set_print_test_format(false, true, false);
        condition_list_to_string(thisAgent, top_cond, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s-->\n", m_pre_string);
        while (*ch) ch++;
        preflist_inst_to_string(thisAgent, top_pref, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        clear_print_test_format();
    }
    if (m_print_identity)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "------------------------- Identity -------------------------\n");
        while (*ch) ch++;
        set_print_test_format(false, false, true);
        condition_list_to_string(thisAgent, top_cond, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s-->\n", m_pre_string);
        while (*ch) ch++;
        preflist_inst_to_string(thisAgent, top_pref, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        clear_print_test_format();
    }

    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::cond_results_to_string(agent* thisAgent, condition* top_cond, preference* top_pref, char* dest, size_t dest_size)
{
    char* ch=dest;

    /* MToDo | Only print headers and latter two if that setting is on */
    if (m_print_actual)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "--------------------------- Match --------------------------\n");
        while (*ch) ch++;
        set_print_test_format(true, false, false);
        condition_list_to_string(thisAgent, top_cond, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s-->\n", m_pre_string);
        while (*ch) ch++;
        preflist_result_to_string(thisAgent, top_pref, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        clear_print_test_format();
    }
    if (m_print_original)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "-------------------------- Original ------------------------\n");
        while (*ch) ch++;
        set_print_test_format(false, true, false);
        condition_list_to_string(thisAgent, top_cond, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s-->\n", m_pre_string);
        while (*ch) ch++;
        preflist_result_to_string(thisAgent, top_pref, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        clear_print_test_format();
    }
    if (m_print_identity)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "------------------------- Identity -------------------------\n");
        while (*ch) ch++;
        set_print_test_format(false, false, true);
        condition_list_to_string(thisAgent, top_cond, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s-->\n", m_pre_string);
        while (*ch) ch++;
        preflist_result_to_string(thisAgent, top_pref, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        clear_print_test_format();
    }

    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::cond_actions_to_string(agent* thisAgent, condition* top_cond, action* top_action, char* dest, size_t dest_size)
{
    char* ch=dest;

    /* MToDo | Only print headers and latter two if that setting is on */
    if (m_print_actual)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "--------------------------- Match --------------------------\n");
        while (*ch) ch++;
        set_print_test_format(true, false, false);
        condition_list_to_string(thisAgent, top_cond, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s-->\n", m_pre_string);
        while (*ch) ch++;
        action_list_to_string(thisAgent, top_action, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        clear_print_test_format();
    }
    if (m_print_original)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "-------------------------- Original ------------------------\n");
        while (*ch) ch++;
        set_print_test_format(false, true, false);
        condition_list_to_string(thisAgent, top_cond, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s-->\n", m_pre_string);
        while (*ch) ch++;
        action_list_to_string(thisAgent, top_action, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        clear_print_test_format();
    }
    if (m_print_identity)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "------------------------- Identity -------------------------\n");
        set_print_test_format(false, false, true);
        while (*ch) ch++;
        condition_list_to_string(thisAgent, top_cond, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%s-->\n", m_pre_string);
        while (*ch) ch++;
        action_list_to_string(thisAgent, top_action, ch, dest_size - (ch - dest));
        while (*ch) ch++;
        clear_print_test_format();
    }

    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

char* Output_Manager::instantiation_to_string(agent* thisAgent, instantiation* inst, char* dest, size_t dest_size)
{
    char* ch=dest;

    if (inst->prod)
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%sMatched %y ", m_pre_string, inst->prod->name);
        while (*ch) ch++;
    }
    else
    {
        sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "%sMatched nothing (dummy production?) \n", m_pre_string);
        while (*ch) ch++;
    }
    sprinta_sf(thisAgent, ch, dest_size - (ch - dest), "in state %y (level %i)\n", inst->match_goal, inst->match_goal_level);
    while (*ch) ch++;
    cond_prefs_to_string(thisAgent, inst->top_of_instantiated_conditions, inst->preferences_generated, ch, dest_size - (ch - dest));

    dest[dest_size - 1] = 0; /* ensure null termination */
    return dest;
}

void add_inst_of_type(agent* thisAgent, unsigned int productionType, std::vector<instantiation*>& instantiation_list)
{
    for (production* prod = thisAgent->all_productions_of_type[productionType]; prod != NIL; prod = prod->next)
    {
        for (instantiation* inst = prod->instantiations; inst != NIL; inst = inst->next)
        {
            instantiation_list.push_back(inst);
        }
    }
}

void Output_Manager::print_all_inst(TraceMode mode)
{
    if (!debug_mode_enabled(mode)) return;
    if (!m_defaultAgent) return;

    print( "--- Instantiations: ---\n");

    std::vector<instantiation*> instantiation_list;
    add_inst_of_type(m_defaultAgent, CHUNK_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(m_defaultAgent, DEFAULT_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(m_defaultAgent, JUSTIFICATION_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(m_defaultAgent, USER_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(m_defaultAgent, TEMPLATE_PRODUCTION_TYPE, instantiation_list);

    for (int y = 0; y < instantiation_list.size(); y++)
    {
        print_sf("========================================= Instantiation %d\n", y);
//        instantiation_to_string(mode, instantiation_list[y]);
    }
}

void Output_Manager::print_saved_test(TraceMode mode, saved_test* st)
{
    if (!debug_mode_enabled(mode)) return;

    print_sf("  Index: %y  Test: %t\n", st->var, st->the_test);
}

void Output_Manager::print_saved_test_list(TraceMode mode, saved_test* st)
{
    if (!debug_mode_enabled(mode)) return;

    while (st)
    {
        print_saved_test(mode, st);
        st = st->next;
    }
}

void Output_Manager::print_varnames(TraceMode mode, varnames* var_names)
{
    cons* c;;

    if (!debug_mode_enabled(mode)) return;

    if (!var_names)
    {
        print("None.");;
    }
    else if (varnames_is_one_var(var_names))
    {
        print_sf("%y ", varnames_to_one_var(var_names));;
    }
    else
    {
        c = varnames_to_var_list(var_names);
        while (c)
        {
            print_sf("%y ", static_cast<Symbol*>(c->first));;
            c = c->rest;
        }
    }
}
void Output_Manager::print_varnames_node(TraceMode mode, node_varnames* var_names_node)
{

    if (!debug_mode_enabled(mode)) return;

    if (!var_names_node)
    {
        print("varnames node empty.\n");
    }
    else
    {
        print("varnames for node = ID: ");

        print_varnames(mode, var_names_node->data.fields.id_varnames);
        print_sf(" | Attr: ");
        print_varnames(mode, var_names_node->data.fields.attr_varnames);
        print_sf(" | Value: ");
        print_varnames(mode, var_names_node->data.fields.value_varnames);
        print_sf("\n");
    }
}

void Output_Manager::debug_find_and_print_sym(char* find_string)
{
    Symbol* newSym = NULL;
    if (find_string)
    {
        bool found = false;
        bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
        bool rereadable;
        std::string convertStr(find_string);
        std::stringstream convert(convertStr);
        int newInt;
        double newFloat;


        if (!m_defaultAgent)
        {
            return;
        }

        soar::Lexer::determine_possible_symbol_types_for_string(find_string,
                static_cast<size_t>(strlen(find_string)),
                &possible_id,
                &possible_var,
                &possible_sc,
                &possible_ic,
                &possible_fc,
                &rereadable);

        if (possible_id)
        {
            newSym = find_identifier(m_defaultAgent, toupper(find_string[0]), strtol(&find_string[1], NULL, 10));
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_var)
        {
            newSym = find_variable(m_defaultAgent, find_string);
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_sc)
        {
            newSym = find_str_constant(m_defaultAgent, find_string);
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_ic)
        {
            if (convert >> newInt)
            {
                newSym = find_int_constant(m_defaultAgent, newInt);
            }
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_fc)
        {
            if (convert >> newFloat)
            {
                newSym = find_float_constant(m_defaultAgent, newFloat);
            }
            if (newSym)
            {
                found = true;
            }
        }
    }
    if (newSym)
    {
        debug_print_sf(DT_DEBUG,
               "%y:\n"
               "  type     = %s\n"
               "  refcount = %d\n"
               "  tc_num   = %d\n",
               newSym,
               newSym->type_string(),
               newSym->reference_count,
               newSym->tc_num);
    }
    else
    {
        debug_print_sf(DT_DEBUG, "No symbol %s found.\n", find_string);
    }
}

bool om_print_sym(agent* thisAgent, void* item, void* vMode)
{
    TraceMode mode = * static_cast < TraceMode* >(vMode);

    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return false;

    Output_Manager::Get_OM().printa_sf(thisAgent, "%y (%i)\n", static_cast<symbol_struct*>(item), static_cast<symbol_struct*>(item)->reference_count);
    return false;
}

void Output_Manager::print_identifiers(TraceMode mode)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    print("--- Identifiers: ---\n");
    do_for_all_items_in_hash_table(m_defaultAgent, m_defaultAgent->identifier_hash_table, om_print_sym, &mode);
}

void Output_Manager::print_variables(TraceMode mode)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    print("--- Variables: ---\n");
    do_for_all_items_in_hash_table(m_defaultAgent, m_defaultAgent->variable_hash_table, om_print_sym, &mode);
}

void debug_print_db_err(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
    if (!thisAgent) return;

    print_sysparam_trace(thisAgent, 0, "Debug| Printing database status/errors...\n");
//  if (thisAgent->debug_params->epmem_commands->get_value() == on)
//  {
//    if (!db_err_epmem_db)
//    {
//      print_trace (thisAgent,0, "Debug| Cannot access epmem database because wmg not yet initialized.\n");
//    }
//    else
//    {
//      print_trace (thisAgent,0, "Debug| EpMem DB: %d - %s\n", sqlite3_errcode( db_err_epmem_db->get_db() ),
//          sqlite3_errmsg( db_err_epmem_db->get_db() ));
//    }
//  }
//  if (thisAgent->debug_params->smem_commands->get_value() == on)
//  {
//    if (!db_err_smem_db)
//    {
//      print_trace (thisAgent,0, "Debug| Cannot access smem database because wmg not yet initialized.\n");
//    }
//    else
//    {
//      print_trace (thisAgent,0, "Debug| SMem DB: %d - %s\n", sqlite3_errcode( db_err_smem_db->get_db() ),
//          sqlite3_errmsg( db_err_smem_db->get_db() ));
//    }
//  }
}

void debug_print_epmem_table(const char* table_name, TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    //agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
//    if (!thisAgent) return;

//  if (!db_err_epmem_db)
//  {
//    if ((thisAgent->epmem_db) && ( thisAgent->epmem_db->get_status() == soar_module::connected ))
//    {
//      db_err_epmem_db = m_defaultAgent->epmem_db;
//      thisAgent->debug_params->epmem_commands->set_value(on);
//    }
//    else
//    {
//      print_trace (thisAgent,0, "Debug| Cannot access epmem database because database not yet initialized.\n");
//      return;
//    }
//  }
//
//  db_err_epmem_db->print_table(table_name);
}

void debug_print_smem_table(const char* table_name, TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    //agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
//    if (!thisAgent) return;

//  if (!db_err_smem_db)
//  {
//    if (thisAgent->smem_db && ( thisAgent->smem_db->get_status() == soar_module::connected ))
//    {
//      db_err_smem_db = m_defaultAgent->smem_db;
//      thisAgent->debug_params->smem_commands->set_value(on);
//    }
//    else
//    {
//      print_trace (thisAgent,0, "Debug| Cannot access smem database because database not yet initialized.\n");
//      return;
//    }
//  }
//  db_err_smem_db->print_table(table_name);
}

void Output_Manager::print_current_lexeme(TraceMode mode, soar::Lexer* lexer)
{
    std::string lex_type_string;

    if (!debug_mode_enabled(mode)) return;

    switch (lexer->current_lexeme.type)
    {
        case EOF_LEXEME:
            lex_type_string = "EOF_LEXEME";
            break;
        case IDENTIFIER_LEXEME:
            lex_type_string = "IDENTIFIER_LEXEME";
            break;
        case VARIABLE_LEXEME:
            lex_type_string = "VARIABLE_LEXEME";
            break;
        case STR_CONSTANT_LEXEME:
            lex_type_string = "STR_CONSTANT_LEXEME";
            break;
        case INT_CONSTANT_LEXEME:
            lex_type_string = "INT_CONSTANT_LEXEME";
            break;
        case FLOAT_CONSTANT_LEXEME:
            lex_type_string = "FLOAT_CONSTANT_LEXEME";
            break;
        case L_PAREN_LEXEME:
            lex_type_string = "L_PAREN_LEXEME";
            break;
        case R_PAREN_LEXEME:
            lex_type_string = "R_PAREN_LEXEME";
            break;
        case L_BRACE_LEXEME:
            lex_type_string = "L_BRACE_LEXEME";
            break;
        case R_BRACE_LEXEME:
            lex_type_string = "R_BRACE_LEXEME";
            break;
        case PLUS_LEXEME:
            lex_type_string = "PLUS_LEXEME";
            break;
        case MINUS_LEXEME:
            lex_type_string = "MINUS_LEXEME";
            break;
        case RIGHT_ARROW_LEXEME:
            lex_type_string = "RIGHT_ARROW_LEXEME";
            break;
        case GREATER_LEXEME:
            lex_type_string = "GREATER_LEXEME";
            break;
        case LESS_LEXEME:
            lex_type_string = "LESS_LEXEME";
            break;
        case EQUAL_LEXEME:
            lex_type_string = "EQUAL_LEXEME";
            break;
        case LESS_EQUAL_LEXEME:
            lex_type_string = "LESS_EQUAL_LEXEME";
            break;
        case GREATER_EQUAL_LEXEME:
            lex_type_string = "GREATER_EQUAL_LEXEME";
            break;
        case NOT_EQUAL_LEXEME:
            lex_type_string = "NOT_EQUAL_LEXEME";
            break;
        case LESS_EQUAL_GREATER_LEXEME:
            lex_type_string = "LESS_EQUAL_GREATER_LEXEME";
            break;
        case LESS_LESS_LEXEME:
            lex_type_string = "LESS_LESS_LEXEME";
            break;
        case GREATER_GREATER_LEXEME:
            lex_type_string = "GREATER_GREATER_LEXEME";
            break;
        case AMPERSAND_LEXEME:
            lex_type_string = "AMPERSAND_LEXEME";
            break;
        case AT_LEXEME:
            lex_type_string = "AT_LEXEME";
            break;
        case TILDE_LEXEME:
            lex_type_string = "TILDE_LEXEME";
            break;
        case UP_ARROW_LEXEME:
            lex_type_string = "UP_ARROW_LEXEME";
            break;
        case EXCLAMATION_POINT_LEXEME:
            lex_type_string = "EXCLAMATION_POINT_LEXEME";
            break;
        case COMMA_LEXEME:
            lex_type_string = "COMMA_LEXEME";
            break;
        case PERIOD_LEXEME:
            lex_type_string = "PERIOD_LEXEME";
            break;
        case QUOTED_STRING_LEXEME:
            lex_type_string = "QUOTED_STRING_LEXEME";
            break;
        case DOLLAR_STRING_LEXEME:
            lex_type_string = "DOLLAR_STRING_LEXEME";
            break;
        case NULL_LEXEME:
            lex_type_string = "NULL_LEXEME";
            break;
        default:
            break;
    }
    print_sf( "%s: \"%s\"\n", lex_type_string.c_str(), lexer->current_lexeme.string());

}
