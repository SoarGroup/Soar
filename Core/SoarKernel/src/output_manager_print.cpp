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

#include "debug.h"
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

void Output_Manager::debug_print(TraceMode mode, const char* msg)
{
    if (!debug_mode_enabled(mode) || !dprint_enabled) return;
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent) return;
    printa_prefix(mode, debug_agent, msg);
}

void Output_Manager::debug_print_f(TraceMode mode, const char* format, ...)
{
    if (!debug_mode_enabled(mode) || !dprint_enabled) return;
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent) return;

    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    printa_prefix(mode, debug_agent, buf);

}

void Output_Manager::debug_print_sf(TraceMode mode, const char* format, ...)
{
    if (!debug_mode_enabled(mode) || !dprint_enabled) return;
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent) return;

    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
    vsnprintf_with_symbols(debug_agent, buf, PRINT_BUFSIZE, format, args);
    va_end(args);

    printa_prefix(mode, debug_agent, buf);

}

void Output_Manager::debug_print_sf_noprefix(TraceMode mode, const char* format, ...)
{
    if (!debug_mode_enabled(mode) || !dprint_enabled) return;
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent) return;

    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
    vsnprintf_with_symbols(debug_agent, buf, PRINT_BUFSIZE, format, args);
    va_end(args);

    printa(debug_agent, buf);

}

void Output_Manager::debug_start_fresh_line(TraceMode mode)
{
    if (!debug_mode_enabled(mode) || !dprint_enabled) return;
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent) return;
    start_fresh_line(debug_agent);
}

void Output_Manager::print_identity(TraceMode mode, identity_info* i, const char* pre_string, const char* post_string)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }

    if (i->original_var)
    {
        print_sf("%s%s", pre_string, i->original_var->to_string());
    }
    else
    {
        print_sf("%s", pre_string);
    }

    if (i->grounding_id != NON_GENERALIZABLE)
    {
        if (i->original_var)
        {
            print_sf(" g%llu\%s", i->grounding_id, post_string);
        }
        else
        {
            print_sf("g%llu\%s", i->grounding_id, post_string);
        }
    }
    else
    {
        print_sf("%s", post_string);
    }
}

void Output_Manager::print_wme(TraceMode mode, wme* w, bool pOnlyWithIdentity)
{
    if (!w) return;
    if (!debug_mode_enabled(mode)) return;
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
        print_sf("(%llu: ", w->timetag);
        print_sf("%y ^%y %y", w->id, w->attr, w->value);
        if (w->acceptable)
        {
            print_sf(" +");
        }
        print_sf("): [");
        grounding_info* g = w->ground_id_list;
        for (; g; g = g->next)
        {
            print_sf("%hi: g%llu g%llu g%llu", g->level, g->grounding_id[0], g->grounding_id[1], g->grounding_id[2]);
            if (g->next)
            {
                print_sf(", ");
            }
        }
        print_sf("]");
    }
}

void Output_Manager::print_wmes(TraceMode mode, bool pOnlyWithIdentity)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    wme* w;
    print_sf("--------------------------- WMEs --------------------------\n");
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
            print_sf("(%llu: ", w->timetag);
            print_sf("%s ^%s %s", w->id->to_string(), w->attr->to_string(), w->value->to_string());
            if (w->acceptable)
            {
                print_sf(" +");
            }
            print_sf("): [");
            grounding_info* g = w->ground_id_list;
            for (; g; g = g->next)
            {
                print_sf("%hi: g%llu g%llu g%llu", g->level, g->grounding_id[0], g->grounding_id[1], g->grounding_id[2]);
                if (g->next)
                {
                    print_sf(", ");
                }
            }
            print_sf("]\n");
        }
    }
}

/* UITODO| Make this method of Test */
void Output_Manager::print_test(TraceMode mode, test t, bool print_actual, bool print_original, bool pIdentity, const char* pre_string, const char* post_string)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }

    cons* c;
    const char* no_type_test_fstring, *type_test_fstring;


    if (!t)
    {
        print_sf("%sNIL%s", pre_string, post_string);
        return;
    }

    if (t->type == CONJUNCTIVE_TEST)
    {
        print_sf("%s{ ", pre_string);
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {
            print_test(mode, static_cast<test>(c->first), print_actual, print_original, pIdentity, "", (c->rest != NULL ? ", " : ""));
        }
        print_sf(" }%s", post_string);
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
                print_sf(no_type_test_fstring, pre_string, test_type_to_string_brief(t->type));
                break;
            case DISJUNCTION_TEST:
                print_sf("%s<< ", pre_string);
                for (c = t->data.disjunction_list; c != NIL; c = c->rest)
                {
                    print_sf(no_type_test_fstring, static_cast<symbol_struct*>(c->first)->to_string(), " ");
                }
                print_sf(">>");
                break;
            default:
                print_sf(type_test_fstring, pre_string,
                                test_type_to_string_brief(t->type),
                                t->data.referent->to_string());
                break;
        }
        if (!print_original && !pIdentity)
        {
            print_sf("%s", post_string);
            return;
        }
    }

    if (print_original)
    {
        if (t->original_test)
        {
            if (print_actual)
            {
                print_test(mode, t->original_test, true, false, false, " (", ")");
            }
            else
            {
                print_test(mode, t->original_test, true, false, false, pre_string, "");
            }
        }
        else
        {
            if (print_actual)
            {
                print_test(mode, t, true, false, false, " (", "*)");
            }
            else
            {
                print_test(mode, t, true, false, false, pre_string, "*");
//                printv_y(" (0)");
//            }
//            else
//            {
//                printv_y("%sNULL", pre_string);
            }
        }
    }

    if (pIdentity)
    {
        if (print_actual)
        {
            print_identity(mode, t->identity, "[", "]");
        }
        else
        {
            print_sf("%s", pre_string);
            print_identity(mode, t->identity, "[", "]");
        }
    }

    print_sf("%s", post_string);
}


bool om_print_sym(agent* thisAgent, void* item, void* vMode)
{
    TraceMode mode = * static_cast < TraceMode* >(vMode);

    if (!Output_Manager::Get_OM().debug_mode_enabled(mode))
    {
        return false;
    }

    Output_Manager::Get_OM().print_sf("%s (%lld)\n", static_cast<symbol_struct*>(item)->to_string(true), static_cast<symbol_struct*>(item)->reference_count);
    return false;
}

void Output_Manager::print_identifiers(TraceMode mode)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    print("--- Identifiers: ---\n");
    do_for_all_items_in_hash_table(debug_agent, debug_agent->identifier_hash_table, om_print_sym, &mode);
}

void Output_Manager::print_variables(TraceMode mode)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    print("--- Variables: ---\n");
    do_for_all_items_in_hash_table(debug_agent, debug_agent->variable_hash_table, om_print_sym, &mode);
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

void Output_Manager::print_current_lexeme(TraceMode mode, soar::Lexer* lexer)
{
    std::string lex_type_string;

    if (!debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

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

void Output_Manager::print_condition_cons(TraceMode mode, cons* c, bool print_actual, bool print_original, bool print_identity, const char* pre_string)
{
    while (c)
    {
        print_condition(mode, static_cast<condition_struct*>(c->first), pre_string, print_actual, print_original, print_identity);
        c = c->rest;
    }
}

void Output_Manager::print_condition(TraceMode mode, condition* cond, const char* indent_string, bool print_actual, bool print_original, bool print_identity)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }


    if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        print_sf("%s(", indent_string);
        print_test(mode, cond->data.tests.id_test, print_actual, print_original, print_identity, "", " ");
        if (cond->type == NEGATIVE_CONDITION)
        {
            print_sf("-");
        }
        print_test(mode, cond->data.tests.attr_test, print_actual, print_original, print_identity, "^", " ");
        print_test(mode, cond->data.tests.value_test, print_actual, print_original, print_identity, "", ")\n");
    }
    else
    {
        print_sf("%s-{\n", indent_string);
        print_condition_list(mode, cond->data.ncc.top, indent_string, print_actual, print_original, print_identity);
        print_sf("%s }\n", indent_string);
    }
}

void Output_Manager::print_condition_list(TraceMode mode, condition* top_cond, const char* indent_string, bool print_actual, bool print_original, bool print_identity)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }

    condition* cond;
    int64_t count = 0;
    for (cond = top_cond; cond != NIL; cond = cond->next)
    {
        assert(cond != cond->next);
        print_sf("%s%lld: ", indent_string, ++count);
        print_condition(mode, cond, "", print_actual, print_original, print_identity);
    }
    return;
}

void Output_Manager::print_rhs_value(TraceMode mode, rhs_value rv, struct token_struct* tok, wme* w)
{
    if (!debug_mode_enabled(mode))
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
        print_sf("#");
    }
    else if (rhs_value_is_unboundvar(rv))
    {
        /* -- unbound variable -- */
        print_sf("<unbound-var>");
    }
    else if (rhs_value_is_symbol(rv))
    {

        /* -- rhs symbol -- */
        rsym = rhs_value_to_rhs_symbol(rv);
        print_sf("%y [%y %llu]", rsym->referent, rsym->original_rhs_variable, rsym->g_id);
    }
    else if (rhs_value_is_reteloc(rv))
    {
        /* -- rete location (cannot get symbol without token information) -- */
        if (tok && w)
        {
            sym = get_symbol_from_rete_loc(
                      rhs_value_to_reteloc_levels_up(rv),
                      rhs_value_to_reteloc_field_num(rv), tok, w);
            print_sf("%y(reteloc)", sym);
        }
        else
        {
            print_sf("(rete-loc no tok/w)");
        }
    }
    else
    {
        /* -- function call -- */
        fl = rhs_value_to_funcall_list(rv);
        rf = static_cast<rhs_function_struct*>(fl->first);

        print_sf("(");
        if (!strcmp(rf->name->sc->name, "+"))
        {
            print_sf("+");
        }
        else if (!strcmp(rf->name->sc->name, "-"))
        {
            print_sf("-");
        }
        else
        {
            print_sf("(%y", rf->name);
        }

        for (c = fl->rest; c != NIL; c = c->rest)
        {
            print_sf(" ");
            print_rhs_value(mode, static_cast<char*>(c->first));
        }
        print_sf(")");
    }
}

void Output_Manager::print_action(TraceMode mode, action* a, const char* indent_string)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }

    if (a->type == FUNCALL_ACTION)
    {
        print_sf("%s(funcall ", indent_string);
        print_rhs_value(mode, a->value);
        print_sf(")\n");
    }
    else
    {
        print_sf("%s(", indent_string);
        print_rhs_value(mode, a->id);
        print_sf(" ^");
        print_rhs_value(mode, a->attr);
        print_sf(" ");
        print_rhs_value(mode, a->value);
        print_sf(" ref: ");
        print_rhs_value(mode, a->referent);
        print_sf(")\n");
    }
}

void Output_Manager::print_action_list(TraceMode mode, action* action_list, const char* indent_string)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }

    action* a = NIL;

    for (a = action_list; a != NIL; a = a->next)
    {
        print_action(mode, a);
    }
}

void Output_Manager::debug_print_preference(TraceMode mode, preference* pref, const char* indent_string, bool print_actual, bool print_original, bool print_identity)
{
    char pref_type;

    if (!debug_mode_enabled(mode))
    {
        return;
    }
    pref_type = preference_to_char(pref->type);
    if (print_actual)
    {
        print_sf("%s(%s ^%s %s)", indent_string,
            (pref->id ? pref->id->to_string() : ""),
            (pref->attr ? pref->attr->to_string() : ""),
            (pref->value ? pref->value->to_string() : "")
        );
    }
    else if (print_original)
    {
        print_sf("%s(%s ^%s %s)", indent_string,
            (pref->original_symbols.id ? pref->original_symbols.id->to_string() : "#"),
            (pref->original_symbols.attr ? pref->original_symbols.attr->to_string() : "#"),
            (pref->original_symbols.value ? pref->original_symbols.value->to_string() : "#")
        );
    }
    else if (print_identity)
    {
        print_sf("%s(%s(g%llu) ^%s[g%llu] %s[g%llu])", indent_string,
            (pref->id ? pref->id->to_string() : ""), pref->g_ids.id,
            (pref->attr ? pref->attr->to_string() : ""), pref->g_ids.attr,
            (pref->value ? pref->value->to_string() : ""), pref->g_ids.value
        );
    }
    print_sf(" %c", pref_type);
    if (preference_is_binary(pref->type))
    {
        print_sf(" %y", pref->referent);
    }
    if (pref->o_supported)
    {
        print_sf(" :O ");
    }
}

void Output_Manager::debug_print_preflist_inst(TraceMode mode, preference* top_pref, const char* indent_string, bool print_actual, bool print_original, bool print_identity)
{
    preference* pref;
    char pref_type;

    if (!debug_mode_enabled(mode))
    {
        return;
    }

    for (pref = top_pref; pref != NIL;)
    {
        debug_print_preference(mode, pref, indent_string, print_actual, print_original, print_identity);
        print_sf(")\n");
        pref = pref->inst_next;
    }
}

void Output_Manager::debug_print_preflist_result(TraceMode mode, preference* top_pref, const char* indent_string, bool print_actual, bool print_original, bool print_identity)
{
    preference* pref;
    char pref_type;

    if (!debug_mode_enabled(mode))
    {
        return;
    }

    for (pref = top_pref; pref != NIL;)
    {
        debug_print_preference(mode, pref, indent_string, print_actual, print_original, print_identity);
        print_sf(")\n");
        pref = pref->next_result;
    }
}

void Output_Manager::debug_print_production(TraceMode mode, production* prod)
{
    if (!debug_mode_enabled(mode))
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
        print_production(debug_agent, prod, true);
    }
}

void Output_Manager::print_cond_prefs(TraceMode mode, condition* top_cond, preference* top_pref)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }

    /* MToDo | Only print headers and latter two if that setting is on */
    if (m_print_actual)
    {
        print_sf("%s--------------------------- Match --------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond, m_pre_string);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_inst(mode, top_pref, m_pre_string);
    }
    if (m_print_original)
    {
        print_sf("%s-------------------------- Original ------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond, m_pre_string, false, true, false);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_inst(mode, top_pref, m_pre_string, false, true, false);
    }
    if (m_print_identity)
    {
        print_sf("%s------------------------- Identity -------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond, m_pre_string, true, false, true);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_inst(mode, top_pref, m_pre_string, false, false, true);
    }
    print_sf("%s\n", m_pre_string);

}

void Output_Manager::print_cond_results(TraceMode mode, condition* top_cond, preference* top_pref)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }

    /* MToDo | Only print headers and latter two if that setting is on */
    if (m_print_actual)
    {
        print_sf("%s--------------------------- Match --------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond, m_pre_string);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_result(mode, top_pref, m_pre_string, true, false, false);
    }
    if (m_print_original)
    {
        print_sf("%s-------------------------- Original ------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond, m_pre_string, false, true, false);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_result(mode, top_pref, m_pre_string, false, true, false);
    }
    if (m_print_identity)
    {
        print_sf("%s------------------------- Identity -------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond, m_pre_string, true, false, true);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_result(mode, top_pref, m_pre_string, false, false, true);
    }
    print_sf("%s\n", m_pre_string);
}

void Output_Manager::print_cond_actions(TraceMode mode, condition* top_cond, action* top_action)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }


    if (m_print_actual)
    {
        print_sf("%s--------------------------- Match --------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond, m_pre_string, true, false, false);
        print_sf("%s-->\n", m_pre_string);
        print_action_list(mode, top_action, m_pre_string);
    }
    if (m_print_original)
    {
        print_sf("%s-------------------------- Original ------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond, m_pre_string, false, true, false);
        print_sf("%s-->\n", m_pre_string);
        print_action_list(mode, top_action, m_pre_string);
    }
    if (m_print_identity)
    {
        print_sf("%s------------------------- Identity -------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond, m_pre_string, true, false, true);
        print_sf("%s-->\n", m_pre_string);
        print_action_list(mode, top_action, m_pre_string);
    }
    print_sf("%s\n", m_pre_string);

}

void Output_Manager::print_instantiation(TraceMode mode, instantiation* inst, const char* indent_string)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }


    if (inst->prod)
    {
        print_sf("%sMatched %s ", m_pre_string, inst->prod->name->to_string());
    }
    else
    {
        print_sf("%sMatched nothing (dummy production?) \n", m_pre_string);
    }
    print_sf("in state %y (level %d)\n", inst->match_goal, inst->match_goal_level);
    print_cond_prefs(mode, inst->top_of_instantiated_conditions, inst->preferences_generated);

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
    if (!debug_mode_enabled(mode))
    {
        return;
    }
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    print_sf( "--- Instantiations: ---\n");

    std::vector<instantiation*> instantiation_list;
    add_inst_of_type(debug_agent, CHUNK_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(debug_agent, DEFAULT_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(debug_agent, JUSTIFICATION_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(debug_agent, USER_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(debug_agent, TEMPLATE_PRODUCTION_TYPE, instantiation_list);

    for (int y = 0; y < instantiation_list.size(); y++)
    {
        print_sf("========================================= Instantiation %d\n", y);
        print_instantiation(mode, instantiation_list[y], "");
    }
}

void Output_Manager::print_saved_test(TraceMode mode, saved_test* st)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }


    print_sf("  Index: %y  Test: %t\n", st->var, st->the_test);
}

void Output_Manager::print_saved_test_list(TraceMode mode, saved_test* st)
{
    if (!debug_mode_enabled(mode))
    {
        return;
    }


    while (st)
    {
        print_saved_test(mode, st);
        st = st->next;
    }
}

void Output_Manager::print_varnames(TraceMode mode, varnames* var_names)
{
    cons* c;;

    if (!debug_mode_enabled(mode))
    {
        return;
    }


    if (!var_names)
    {
        print_sf("None.");;
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

    if (!debug_mode_enabled(mode))
    {
        return;
    }


    if (!var_names_node)
    {
        print_sf("varnames node empty.\n");
    }
    else
    {
        print_f("varnames for node = ID: ");

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

        agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
        if (!debug_agent)
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
            newSym = find_identifier(debug_agent, toupper(find_string[0]), strtol(&find_string[1], NULL, 10));
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_var)
        {
            newSym = find_variable(debug_agent, find_string);
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_sc)
        {
            newSym = find_str_constant(debug_agent, find_string);
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_ic)
        {
            if (convert >> newInt)
            {
                newSym = find_int_constant(debug_agent, newInt);
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
                newSym = find_float_constant(debug_agent, newFloat);
            }
            if (newSym)
            {
                found = true;
            }
        }
    }
    if (newSym)
    {
        debug_print_f(DT_DEBUG,
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
        debug_print_f(DT_DEBUG, "No symbol %s found.\n", find_string);
    }
}
