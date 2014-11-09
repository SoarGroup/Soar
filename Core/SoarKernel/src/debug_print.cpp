/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
             debug_print.cpp

   @brief debugPrint.cpp provides some printing functions that are
          not used from the kernel but can be used for debugging,
          for example by being called from gdb.

------------------------------------------------------------------ */

#include "debug.h"

#ifdef SOAR_DEBUG_UTILITIES
#include "rhs.h"
#include "print.h"
#include "agent.h"
#include "instantiations.h"
#include "rete.h"
#include "reorder.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "output_manager.h"
#include "prefmem.h"
#include "wmem.h"
#include "soar_instance.h"
#include "test.h"

inline void dprint_string(TraceMode mode, const char* message, bool noPrefix = false)
{
    Output_Manager::Get_OM().print_debug(message, mode, noPrefix);
}

void dprint(TraceMode mode, const char* format, ...)
{

    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }

    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    dprint_string(mode, buf);

}

void dprint_noprefix(TraceMode mode, const char* format, ...)
{

    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }

    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    dprint_string(mode, buf, true);
}

void dprint_start_fresh_line(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent) return;
    Output_Manager::Get_OM().start_fresh_line(debug_agent);
}

void dprint_identity(TraceMode mode, identity_info* i, const char* pre_string, const char* post_string)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }

    if (i->original_var)
    {
        dprint_noprefix(mode, "%s%s", pre_string, i->original_var->to_string());
    }
    else
    {
        dprint_noprefix(mode, "%s", pre_string);
    }

    if (i->grounding_id != NON_GENERALIZABLE)
    {
        if (i->original_var)
        {
            dprint_noprefix(mode, " g%llu\%s", i->grounding_id, post_string);
        }
        else
        {
            dprint_noprefix(mode, "g%llu\%s", i->grounding_id, post_string);
        }
    }
    else
    {
        dprint_noprefix(mode, "%s", post_string);
    }
}

void dprint_wme(TraceMode mode, wme* w, bool pOnlyWithIdentity)
{
    if (!w) return;
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent) return;


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
        dprint_noprefix(mode, "(%llu: ", w->timetag);
        dprint_noprefix(mode, "%s ^%s %s", w->id->to_string(), w->attr->to_string(), w->value->to_string());
        if (w->acceptable)
        {
            dprint_noprefix(mode, " +");
        }
        dprint_noprefix(mode, "): [");
        grounding_info* g = w->ground_id_list;
        for (; g; g = g->next)
        {
            dprint_noprefix(mode, "%hi: g%llu g%llu g%llu", g->level, g->grounding_id[0], g->grounding_id[1], g->grounding_id[2]);
            if (g->next)
            {
                dprint_noprefix(mode, ", ");
            }
        }
        dprint_noprefix(mode, "]");
    }
}

void dprint_wmes(TraceMode mode, bool pOnlyWithIdentity)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    wme* w;
    dprint_noprefix(mode, "--------------------------- WMEs --------------------------\n");
    bool lFoundIdentity;
    for (w = debug_agent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
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
            dprint_noprefix(mode, "(%llu: ", w->timetag);
            dprint_noprefix(mode, "%s ^%s %s", w->id->to_string(), w->attr->to_string(), w->value->to_string());
            if (w->acceptable)
            {
                dprint_noprefix(mode, " +");
            }
            dprint_noprefix(mode, "): [");
            grounding_info* g = w->ground_id_list;
            for (; g; g = g->next)
            {
                dprint_noprefix(mode, "%hi: g%llu g%llu g%llu", g->level, g->grounding_id[0], g->grounding_id[1], g->grounding_id[2]);
                if (g->next)
                {
                    dprint_noprefix(mode, ", ");
                }
            }
            dprint_noprefix(mode, "]\n");
        }
    }
}

/* UITODO| Make this method of Test */
void dprint_test(TraceMode mode, test t, bool print_actual, bool print_original, bool print_identity, const char* pre_string, const char* post_string)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }

    cons* c;
    const char* no_type_test_fstring, *type_test_fstring;


    if (!t)
    {
        dprint_noprefix(mode, "%sNIL%s", pre_string, post_string);
        return;
    }

    if (t->type == CONJUNCTIVE_TEST)
    {
        dprint_noprefix(mode, "%s{ ", pre_string);
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {
            dprint_test(mode, static_cast<test>(c->first), print_actual, print_original, print_identity, "", (c->rest != NULL ? ", " : ""));
        }
        dprint_noprefix(mode, " }%s", post_string);
        return;
    }

    if (print_actual)
    {
        no_type_test_fstring = "%s%s";
        type_test_fstring = "%s%s%s";
        switch (t->type)
        {
            case GOAL_ID_TEST:
            case IMPASSE_ID_TEST:
                dprint_noprefix(mode, no_type_test_fstring, pre_string, test_type_to_string_brief(t->type));
                break;
            case DISJUNCTION_TEST:
                dprint_noprefix(mode, "%s<< ", pre_string);
                for (c = t->data.disjunction_list; c != NIL; c = c->rest)
                {
                    dprint_noprefix(mode, no_type_test_fstring, static_cast<symbol_struct*>(c->first)->to_string(), " ");
                }
                dprint_noprefix(mode, ">>");
                break;
            default:
                dprint_noprefix(mode, type_test_fstring, pre_string,
                                test_type_to_string_brief(t->type),
                                t->data.referent->to_string());
                break;
        }
        if (!print_original && !print_identity)
        {
            dprint_noprefix(mode, "%s", post_string);
            return;
        }
    }

    if (print_original)
    {
        if (t->original_test)
        {
            if (print_actual)
            {
                dprint_test(mode, t->original_test, true, false, false, " (", ")");
            }
            else
            {
                dprint_test(mode, t->original_test, true, false, false, pre_string, "");
            }
        }
        else
        {
            if (print_actual)
            {
                dprint_test(mode, t, true, false, false, " (", "*)");
            }
            else
            {
                dprint_test(mode, t, true, false, false, pre_string, "*");
//                dprint_noprefix(mode, " (0)");
//            }
//            else
//            {
//                dprint_noprefix(mode, "%sNULL", pre_string);
            }
        }
    }

    if (print_identity)
    {
        if (print_actual)
        {
            dprint_identity(mode, t->identity, "[", "]");
        }
        else
        {
            dprint_noprefix(mode, "%s", pre_string);
            dprint_identity(mode, t->identity, "[", "]");
        }
    }

    dprint_noprefix(mode, "%s", post_string);
}


bool dprint_sym(agent* thisAgent, void* item, void* vMode)
{
    TraceMode mode = * static_cast < TraceMode* >(vMode);

    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return false;
    }

    dprint(mode,  "%s (%lld)\n", static_cast<symbol_struct*>(item)->to_string(true), static_cast<symbol_struct*>(item)->reference_count);
    return false;
}

void dprint_identifiers(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    dprint(mode,  "--- Identifiers: ---\n");
    do_for_all_items_in_hash_table(debug_agent, debug_agent->identifier_hash_table, dprint_sym, &mode);
}

void dprint_variables(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    dprint(mode,  "--- Variables: ---\n");
    do_for_all_items_in_hash_table(debug_agent, debug_agent->variable_hash_table, dprint_sym, &mode);
}

void debug_print_db_err(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }


    print_sysparam_trace(debug_agent, 0, "Debug| Printing database status/errors...\n");
//  if (debug_agent->debug_params->epmem_commands->get_value() == on)
//  {
//    if (!db_err_epmem_db)
//    {
//      print_trace (debug_agent,0, "Debug| Cannot access epmem database because wmg not yet initialized.\n");
//    }
//    else
//    {
//      print_trace (debug_agent,0, "Debug| EpMem DB: %d - %s\n", sqlite3_errcode( db_err_epmem_db->get_db() ),
//          sqlite3_errmsg( db_err_epmem_db->get_db() ));
//    }
//  }
//  if (debug_agent->debug_params->smem_commands->get_value() == on)
//  {
//    if (!db_err_smem_db)
//    {
//      print_trace (debug_agent,0, "Debug| Cannot access smem database because wmg not yet initialized.\n");
//    }
//    else
//    {
//      print_trace (debug_agent,0, "Debug| SMem DB: %d - %s\n", sqlite3_errcode( db_err_smem_db->get_db() ),
//          sqlite3_errmsg( db_err_smem_db->get_db() ));
//    }
//  }
}

void debug_print_epmem_table(const char* table_name, TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

//  if (!db_err_epmem_db)
//  {
//    if ((debug_agent->epmem_db) && ( debug_agent->epmem_db->get_status() == soar_module::connected ))
//    {
//      db_err_epmem_db = debug_agent->epmem_db;
//      debug_agent->debug_params->epmem_commands->set_value(on);
//    }
//    else
//    {
//      print_trace (debug_agent,0, "Debug| Cannot access epmem database because database not yet initialized.\n");
//      return;
//    }
//  }
//
//  db_err_epmem_db->print_table(table_name);
}

void debug_print_smem_table(const char* table_name, TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

//  if (!db_err_smem_db)
//  {
//    if (debug_agent->smem_db && ( debug_agent->smem_db->get_status() == soar_module::connected ))
//    {
//      db_err_smem_db = debug_agent->smem_db;
//      debug_agent->debug_params->smem_commands->set_value(on);
//    }
//    else
//    {
//      print_trace (debug_agent,0, "Debug| Cannot access smem database because database not yet initialized.\n");
//      return;
//    }
//  }
//  db_err_smem_db->print_table(table_name);
}

void dprint_current_lexeme(TraceMode mode)
{
    std::string lex_type_string;

    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    switch (debug_agent->lexeme.type)
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
    dprint(mode,  "%s: \"%s\"\n", lex_type_string.c_str(), debug_agent->lexeme.string);

}

void dprint_condition_cons(TraceMode mode, cons* c, bool print_actual, bool print_original, bool print_identity, const char* pre_string)
{
    while (c)
    {
        dprint_condition(mode, static_cast<condition_struct*>(c->first), pre_string, print_actual, print_original, print_identity);
        c = c->rest;
    }
}

void dprint_condition(TraceMode mode, condition* cond, const char* indent_string, bool print_actual, bool print_original, bool print_identity)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }


    if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        dprint_noprefix(mode, "%s(", indent_string);
        dprint_test(mode, cond->data.tests.id_test, print_actual, print_original, print_identity, "", " ");
        if (cond->type == NEGATIVE_CONDITION)
        {
            dprint_noprefix(mode, "-");
        }
        dprint_test(mode, cond->data.tests.attr_test, print_actual, print_original, print_identity, "^", " ");
        dprint_test(mode, cond->data.tests.value_test, print_actual, print_original, print_identity, "", ")\n");
    }
    else
    {
        dprint_noprefix(mode, "%s-{\n", indent_string);
        dprint_condition_list(mode, cond->data.ncc.top, indent_string, print_actual, print_original, print_identity);
        dprint_noprefix(mode, "%s }\n", indent_string);
    }
}

void dprint_condition_list(TraceMode mode, condition* top_cond, const char* indent_string, bool print_actual, bool print_original, bool print_identity)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }

    condition* cond;
    int64_t count = 0;
    for (cond = top_cond; cond != NIL; cond = cond->next)
    {
        assert(cond != cond->next);
        dprint_noprefix(mode, "%s%lld: ", indent_string, ++count);
        dprint_condition(mode, cond, "", print_actual, print_original, print_identity);
    }
    return;
}

void dprint_rhs_value(TraceMode mode, rhs_value rv, struct token_struct* tok = NIL, wme* w = NIL)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }


    rhs_symbol rsym = NIL;
    Symbol* sym = NIL;
    cons* c;
    list* fl;
    rhs_function* rf;
    int i;
    if (!rv)
    {
        dprint_noprefix(mode, "#");
    }
    else if (rhs_value_is_unboundvar(rv))
    {
        /* -- unbound variable -- */
        dprint_noprefix(mode, "<unbound-var>");
    }
    else if (rhs_value_is_symbol(rv))
    {

        /* -- rhs symbol -- */
        rsym = rhs_value_to_rhs_symbol(rv);
        dprint_noprefix(mode, "%s [%s %llu]", rsym->referent->to_string(), rsym->original_rhs_variable->to_string(), rsym->g_id);
    }
    else if (rhs_value_is_reteloc(rv))
    {
        /* -- rete location (cannot get symbol without token information) -- */
        if (tok && w)
        {
            sym = get_symbol_from_rete_loc(
                      rhs_value_to_reteloc_levels_up(rv),
                      rhs_value_to_reteloc_field_num(rv), tok, w);
            dprint_noprefix(mode, "%s(reteloc)", sym->to_string());
        }
        else
        {
            dprint_noprefix(mode, "(rete-loc no tok/w)");
        }
    }
    else
    {
        /* -- function call -- */
        fl = rhs_value_to_funcall_list(rv);
        rf = static_cast<rhs_function_struct*>(fl->first);

        dprint_noprefix(mode, "(");
        if (!strcmp(rf->name->sc->name, "+"))
        {
            dprint_noprefix(mode, "+");
        }
        else if (!strcmp(rf->name->sc->name, "-"))
        {
            dprint_noprefix(mode, "-");
        }
        else
        {
            dprint_noprefix(mode, "(", rf->name->to_string());
        }

        for (c = fl->rest; c != NIL; c = c->rest)
        {
            dprint_noprefix(mode, " ");
            dprint_rhs_value(mode, static_cast<char*>(c->first));
        }
        dprint_noprefix(mode, ")");
    }
}

void dprint_action(TraceMode mode, action* a, const char* indent_string)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }

    if (a->type == FUNCALL_ACTION)
    {
        dprint_noprefix(mode, "%s(funcall ", indent_string);
        dprint_rhs_value(mode, a->value);
        dprint_noprefix(mode, ")\n");
    }
    else
    {
        dprint_noprefix(mode, "%s(", indent_string);
        dprint_rhs_value(mode, a->id);
        dprint_noprefix(mode, " ^");
        dprint_rhs_value(mode, a->attr);
        dprint_noprefix(mode, " ");
        dprint_rhs_value(mode, a->value);
        dprint_noprefix(mode, " ref: ");
        dprint_rhs_value(mode, a->referent);
        dprint_noprefix(mode, ")\n");
    }
}

void dprint_action_list(TraceMode mode, action* action_list, const char* indent_string)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }

    action* a = NIL;

    for (a = action_list; a != NIL; a = a->next)
    {
        dprint_action(mode, a);
    }
}

void dprint_action_list_old(TraceMode mode, action* top_action, const char* indent_string)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

//  dprint_noprefix(mode, "           ");
    print_action_list(debug_agent, top_action, strlen(indent_string), false);
}

// Use pref_list_type = 0 to print a single pref
void dprint_preferences(TraceMode mode, preference* top_pref, const char* indent_string, bool print_actual, bool print_original, bool print_identity, int pref_list_type)
{
    preference* pref;
    char pref_type;

    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }

    for (pref = top_pref; pref != NIL;)
    {
        pref_type = preference_to_char(pref->type);
        if (print_actual)
        {
            dprint_noprefix(mode, "%s(%s ^%s %s)", indent_string,
                            (pref->id ? pref->id->to_string() : ""),
                            (pref->attr ? pref->attr->to_string() : ""),
                            (pref->value ? pref->value->to_string() : "")
                           );
        }
        else if (print_original)
        {
            dprint_noprefix(mode, "%s(%s ^%s %s)", indent_string,
                            (pref->original_symbols.id ? pref->original_symbols.id->to_string() : "#"),
                            (pref->original_symbols.attr ? pref->original_symbols.attr->to_string() : "#"),
                            (pref->original_symbols.value ? pref->original_symbols.value->to_string() : "#")
                           );
        }
        else if (print_identity)
        {
            dprint_noprefix(mode, "%s(%s(g%llu) ^%s[g%llu] %s[g%llu])", indent_string,
                            (pref->id ? pref->id->to_string() : ""), pref->g_ids.id,
                            (pref->attr ? pref->attr->to_string() : ""), pref->g_ids.attr,
                            (pref->value ? pref->value->to_string() : ""), pref->g_ids.value
                           );
        }
        dprint_noprefix(mode, " %c", pref_type);
        if (preference_is_binary(pref->type))
        {
            dprint_noprefix(mode, " %s", pref->referent->to_string());
        }
        if (pref->o_supported)
        {
            dprint_noprefix(mode, " :O ");
        }
        dprint_noprefix(mode, ")\n");
        if (pref_list_type == 1)
        {
            pref = pref->inst_next;
        }
        else if (pref_list_type == 2)
        {
            pref = pref->next_result;
        }
        else
        {
            pref = NIL;
        }
    }
}

void dprint_production(TraceMode mode, production* prod)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    if (prod)
    {
        print_production(debug_agent, prod, false);
    }
}

void dprint_cond_prefs(TraceMode mode, condition* top_cond, preference* top_pref,  const char* indent_string, int pref_list_type)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }


    dprint_noprefix(mode, "%s--------------------------- Match --------------------------\n", indent_string);
    dprint_condition_list(mode, top_cond, indent_string, true, false, false);
    dprint_noprefix(mode, "%s-->\n", indent_string);
    dprint_preferences(mode, top_pref, indent_string, true, false, false, pref_list_type);

    dprint_noprefix(mode, "%s-------------------------- Original ------------------------\n", indent_string);
    dprint_condition_list(mode, top_cond, indent_string, false, true, false);
    dprint_noprefix(mode, "%s-->\n", indent_string);
    dprint_preferences(mode, top_pref, indent_string, false, true, false, pref_list_type);

    dprint_noprefix(mode, "%s------------------------- Identity -------------------------\n", indent_string);
    dprint_condition_list(mode, top_cond, indent_string, true, false, true);
    dprint_noprefix(mode, "%s-->\n", indent_string);
    dprint_preferences(mode, top_pref, indent_string, false, false, true, pref_list_type);

    dprint_noprefix(mode, "%s\n", indent_string);

}

void dprint_cond_results(TraceMode mode, condition* top_cond, preference* top_pref,  const char* indent_string)
{
    dprint_cond_prefs(mode, top_cond, top_pref, indent_string, 2);

}

void dprint_cond_actions(TraceMode mode, condition* top_cond, action* top_action,  const char* indent_string)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }


    dprint_noprefix(mode, "%s--------------------------- Match --------------------------\n", indent_string);
    dprint_condition_list(mode, top_cond, indent_string, true, false, false);
    dprint_noprefix(mode, "%s-->\n", indent_string);
    dprint_action_list(mode, top_action, indent_string);

    dprint_noprefix(mode, "%s-------------------------- Original ------------------------\n", indent_string);
    dprint_condition_list(mode, top_cond, indent_string, false, true, false);
    dprint_noprefix(mode, "%s-->\n", indent_string);
    dprint_action_list(mode, top_action, indent_string);

    dprint_noprefix(mode, "%s------------------------- Identity -------------------------\n", indent_string);
    dprint_condition_list(mode, top_cond, indent_string, true, false, true);
    dprint_noprefix(mode, "%s-->\n", indent_string);
    dprint_action_list(mode, top_action, indent_string);

    dprint_noprefix(mode, "%s\n", indent_string);

}

void dprint_instantiation(TraceMode mode, instantiation* inst, const char* indent_string)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }


    if (inst->prod)
    {
        dprint_noprefix(mode, "%sMatched %s ", indent_string, inst->prod->name->to_string());
    }
    else
    {
        dprint_noprefix(mode, "%sMatched nothing (dummy production?) \n", indent_string);
    }
    dprint_noprefix(mode, "in state %s (level %d)\n", inst->match_goal->to_string(), inst->match_goal_level);
    dprint_cond_prefs(mode, inst->top_of_instantiated_conditions, inst->preferences_generated, indent_string);

}

void add_inst_of_type(agent* thisAgent, unsigned int productionType, std::vector<instantiation*>& instantiation_list)
{
    for (production* prod = thisAgent->all_productions_of_type[productionType]; prod != NIL; prod = prod->next)
        for (instantiation* inst = prod->instantiations; inst != NIL; inst = inst->next)
        {
            instantiation_list.push_back(inst);
        }
}

void dprint_all_inst(TraceMode mode)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    dprint_noprefix(mode,  "--- Instantiations: ---\n");

    std::vector<instantiation*> instantiation_list;
    add_inst_of_type(debug_agent, CHUNK_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(debug_agent, DEFAULT_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(debug_agent, JUSTIFICATION_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(debug_agent, USER_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(debug_agent, TEMPLATE_PRODUCTION_TYPE, instantiation_list);

    for (int y = 0; y < instantiation_list.size(); y++)
    {
        dprint_noprefix(mode, "========================================= Instantiation %d\n", y);
        dprint_instantiation(mode, instantiation_list[y], "");
    }
}

void dprint_saved_test(TraceMode mode, saved_test* st)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }


    dprint(mode, "  Index: %y  Test: ", st->var->to_string());
    dprint_test(mode, st->the_test);
    dprint(mode,  "\n");
}

void dprint_saved_test_list(TraceMode mode, saved_test* st)
{
    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }


    while (st)
    {
        dprint_saved_test(mode, st);
        st = st->next;
    }
}

void dprint_varnames(TraceMode mode, varnames* var_names)
{
    cons* c;;

    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }


    if (!var_names)
    {
        dprint_noprefix(mode, "None.");;
    }
    else if (varnames_is_one_var(var_names))
    {
        dprint_noprefix(mode, "%s ", varnames_to_one_var(var_names)->to_string());;
    }
    else
    {
        c = varnames_to_var_list(var_names);
        while (c)
        {
            dprint_noprefix(mode, "%s ", static_cast<Symbol*>(c->first)->to_string());;
            c = c->rest;
        }
    }
}
void dprint_varnames_node(TraceMode mode, node_varnames* var_names_node)
{

    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return;
    }


    if (!var_names_node)
    {
        dprint_noprefix(mode, "varnames node empty.\n");
    }
    else
    {
        dprint(mode, "varnames for node = ID: ");

        dprint_varnames(mode, var_names_node->data.fields.id_varnames);
        dprint_noprefix(mode, " | Attr: ");
        dprint_varnames(mode, var_names_node->data.fields.attr_varnames);
        dprint_noprefix(mode, " | Value: ");
        dprint_varnames(mode, var_names_node->data.fields.value_varnames);
        dprint_noprefix(mode, "\n");
    }
}

#endif
