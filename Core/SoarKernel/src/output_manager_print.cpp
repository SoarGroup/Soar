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

void Output_Manager::printa_sf(agent* pSoarAgent, const char* format, ...)
{
    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
    vsnprint_sf(pSoarAgent, buf, PRINT_BUFSIZE, format, args);
    va_end(args);
    printa(pSoarAgent, buf);
}

void Output_Manager::print_sf(const char* format, ...)
{
    if (m_defaultAgent)
    {
        va_list args;
        char buf[PRINT_BUFSIZE];

        va_start(args, format);
        vsnprint_sf(m_defaultAgent, buf, PRINT_BUFSIZE, format, args);
        va_end(args);
        printa(m_defaultAgent, buf);
    }
}

void Output_Manager::printa(agent* pSoarAgent, const char* msg)
{
    if (pSoarAgent)
    {
        if (!pSoarAgent->output_settings->print_enabled) return;
        if (pSoarAgent->output_settings->callback_mode)
        {
            soar_invoke_callbacks(pSoarAgent, PRINT_CALLBACK, static_cast<soar_call_data>(const_cast<char*>(msg)));
        }
        if (pSoarAgent->output_settings->stdout_mode)
        {
            fputs(msg, stdout);
        }

        update_printer_columns(pSoarAgent, msg);

        if (db_mode)
        {
            m_db->print_db(trace_msg, mode_info[No_Mode].prefix, msg);
        }
    }
}

void Output_Manager::printa_database(TraceMode mode, agent* pSoarAgent, MessageType msgType, const char* msg)
{
    soar_module::sqlite_statement*   target_statement = NIL;

    if (((msgType == trace_msg) && mode_info[mode].enabled) ||
            ((msgType == debug_msg) && mode_info[mode].enabled))
    {
        m_db->print_db(msgType, mode_info[mode].prefix, msg);
    }
}

void Output_Manager::debug_print(TraceMode mode, const char* msg)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    char buf[PRINT_BUFSIZE];
    strcpy(buf, mode_info[mode].prefix);
    int s = strlen(buf);
    strcpy((buf + s), msg);
    printa(m_defaultAgent, buf);
}

void Output_Manager::vsnprint_sf(agent* thisAgent, char* dest, size_t dest_size, const char* format, va_list args)
{
    char* ch;
    char c;
    Symbol* sym;

    ch = dest;

    while (true)
    {
        /* --- copy anything up to the first "%" --- */
        while ((*format != '%') && (*format != 0))
        {
            *(ch++) = *(format++);
        }
        if (*format == 0)
        {
            break;
        }
        /* --- handle the %-thingy --- */
        /* the size of the remaining buffer (after ch) is
            the difference between the address of ch and
            the address of the beginning of the buffer
         */
        if (*(format + 1) == 's')
        {
            char *ch2 = va_arg(args, char *);
            if (ch2 && strlen(ch2))
            {
                //SNPRINTF(ch, count - (ch - dest), "%s", va_arg(args, char *));
                strcpy(ch, ch2);
                while (*ch) ch++;
            }
            format += 2;
        } else if (*(format + 1) == 'y')
        {
            sym = va_arg(args, Symbol*);
            if (sym)
            {
                (sym)->to_string(true, ch, dest_size - (ch - dest));
                while (*ch) ch++;
            } else {
                *(ch++) = '#';
            }
            format += 2;
        } else if (*(format + 1) == 'i')
        {
            SNPRINTF(ch, dest_size - (ch - dest), "%lld", va_arg(args, int64_t));
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'u')
        {
            SNPRINTF(ch, dest_size - (ch - dest), "%llu", va_arg(args, uint64_t));
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 't')
        {
            test_to_string(va_arg(args, test_info *), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'p')
        {
            pref_to_string(thisAgent, va_arg(args, preference *), ch, dest_size - (ch - dest) );
            while (*ch) ch++;
            format += 2;
        } else if (*(format + 1) == 'c')
        {
            c = static_cast<char>(va_arg(args, int));
            SNPRINTF(ch, dest_size - (ch - dest), "%c", c);
            while (*ch) ch++;
            format += 2;
        } else
        {
            *(ch++) = *(format++);
        }
    }
    *ch = 0;
}

void Output_Manager::sprint_sf(agent* thisAgent, char* dest, size_t dest_size, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprint_sf(thisAgent, dest, dest_size, format, args);
    va_end(args);
}

void Output_Manager::debug_print_sf(TraceMode mode, const char* format, ...)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    va_list args;
    char buf[PRINT_BUFSIZE];

    strcpy(buf, mode_info[mode].prefix);
    int s = strlen(buf);
    va_start(args, format);
    vsnprint_sf(m_defaultAgent, (buf+s), PRINT_BUFSIZE, format, args);
    va_end(args);
    printa(m_defaultAgent, buf);
}

void Output_Manager::debug_print_sf_noprefix(TraceMode mode, const char* format, ...)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
    vsnprint_sf(m_defaultAgent, buf, PRINT_BUFSIZE, format, args);
    va_end(args);

    printa(m_defaultAgent, buf);

}

void Output_Manager::debug_start_fresh_line(TraceMode mode)
{
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;
    start_fresh_line(m_defaultAgent);
}

void Output_Manager::print_identity(TraceMode mode, identity_info* i, const char* pre_string, const char* post_string)
{
    print_sf("%s%y %u%s", i->original_var, i->grounding_id);
}

void Output_Manager::print_wme(TraceMode mode, wme* w, bool pOnlyWithIdentity)
{
    if (!w) return;
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;


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
        print_sf("(%u: ", w->timetag);
        print_sf("%y ^%y %y", w->id, w->attr, w->value);
        if (w->acceptable)
        {
            print_sf(" +");
        }
        print_sf("): [");
        grounding_info* g = w->ground_id_list;
        for (; g; g = g->next)
        {
            print_sf("%i: g%u g%u g%u", g->level, g->grounding_id[0], g->grounding_id[1], g->grounding_id[2]);
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
    if (!debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    wme* w;
    print_sf("--------------------------- WMEs --------------------------\n");
    bool lFoundIdentity;
    for (w = m_defaultAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
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
            print_sf("(%u: %y ^y %y", w->timetag, w->id, w->attr, w->value);
            if (w->acceptable)
            {
                print(" +");
            }
            print_sf("): [");
            grounding_info* g = w->ground_id_list;
            for (; g; g = g->next)
            {
                print_sf("%i: g%u g%u g%u", g->level, g->grounding_id[0], g->grounding_id[1], g->grounding_id[2]);
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
void Output_Manager::print_test(TraceMode mode, test t, bool pActual, bool pOriginal, bool pIdentity, const char* pPre_string, const char* pPost_string)
{
    cons* c;
    const char* no_type_test_fstring, *type_test_fstring;


    if (!t)
    {
        print_sf("%NULL%s", pPre_string, pPost_string);
        return;
    }

    if (t->type == CONJUNCTIVE_TEST)
    {
        print_sf("%s{ ", pPre_string);
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {
            print_test(mode, static_cast<test>(c->first), "", (c->rest != NULL ? ", " : ""));
        }
        print_sf(" }%s", pPost_string);
        return;
    }

    if (pActual)
    {
        no_type_test_fstring = "%y%s";
        type_test_fstring = "%s%s%y";
        switch (t->type)
        {
            case GOAL_ID_TEST:
            case IMPASSE_ID_TEST:
                print_sf(no_type_test_fstring, pPre_string, test_type_to_string_brief(t->type));
                break;
            case DISJUNCTION_TEST:
                print_sf("%s<< ", pPre_string);
                for (c = t->data.disjunction_list; c != NIL; c = c->rest)
                {
                    print_sf(no_type_test_fstring, static_cast<symbol_struct*>(c->first), " ");
                }
                print_sf(">>");
                break;
            default:
                print_sf(type_test_fstring, pPre_string,
                                test_type_to_string_brief(t->type),
                                t->data.referent);
                break;
        }
        if (!pOriginal && !pIdentity)
        {
            print_sf("%s", pPost_string);
            return;
        }
    }

    if (pOriginal)
    {
        if (t->original_test)
        {
            if (!pActual)
            {
                print_sf("%s%t", pPre_string, t->original_test);
            } else {
                print_sf(" (%t)", t->original_test);
            }
        }
        else
        {
            if (!pActual)
            {
                print_sf("%s%t*", pPre_string, t->original_test);
            } else {
                print_sf(" (%t*)", t->original_test);
            }
            if (pActual)
            {
                print_test(mode, t, true, false, false, " (", "*)");
            }
            else
            {
                print_test(mode, t, true, false, false, pPre_string, "*");
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
        if (!pActual)
        {
            print(pPre_string);
        }
        print_sf("[%y g%u]", t->identity->original_var, t->identity->grounding_id);
    }

    print(pPost_string);
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
    agent* thisAgent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
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
    //agent* thisAgent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
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
    //agent* thisAgent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
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

void Output_Manager::print_condition_cons(TraceMode mode, cons* c)
{
    if (!debug_mode_enabled(mode)) return;

    while (c)
    {
        print_condition(mode, static_cast<condition_struct*>(c->first));
        c = c->rest;
    }
}

void Output_Manager::print_condition(TraceMode mode, condition* cond)
{
    if (!debug_mode_enabled(mode)) return;

    if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        print_sf("(%t %s ^%t %t)\n",
            cond->data.tests.id_test,
            (cond->type == NEGATIVE_CONDITION) ? "-": NULL,
            cond->data.tests.attr_test, cond->data.tests.value_test);
    }
    else
    {
        print("%s-{\n");
        print_condition_list(mode, cond->data.ncc.top);
        print("}\n");
    }
}

void Output_Manager::print_condition_list(TraceMode mode, condition* top_cond)
{
    if (!debug_mode_enabled(mode)) return;

    condition* cond;
    int64_t count = 0;
    for (cond = top_cond; cond != NIL; cond = cond->next)
    {
        assert(cond != cond->next);
        print_sf("%s%i: ", m_pre_string, ++count);
        print_condition(mode, cond);
    }
    return;
}

void Output_Manager::print_rhs_value(TraceMode mode, rhs_value rv, struct token_struct* tok, wme* w)
{
    if (!debug_mode_enabled(mode)) return;

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
        if (this->m_print_actual)
        {
            print_sf("%y", rsym->referent);
        } else if (m_print_original) {
            print_sf("%y", rsym->original_rhs_variable);
        } else if (m_print_identity) {
            print_sf("%u", rsym->g_id);
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
            print_sf("(%y", rf->name);
//        }
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            print_sf(" ");
            this->print_rhs_value(mode, static_cast<rhs_value>(c->first));
        }
        print_sf(")");
    }
}

void Output_Manager::print_action(TraceMode mode, action* a)
{
    if (!debug_mode_enabled(mode)) return;

    if (a->type == FUNCALL_ACTION)
    {
        print_sf("%s(funcall ", m_pre_string);
        print_rhs_value(mode, a->value);
        print_sf(")\n");
    }
    else
    {
        print_sf("%s(", m_pre_string);
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

void Output_Manager::print_action_list(TraceMode mode, action* action_list)
{
    if (!debug_mode_enabled(mode)) return;

    action* a = NIL;

    for (a = action_list; a != NIL; a = a->next)
    {
        print_action(mode, a);
    }
}

char* Output_Manager::pref_to_string(agent* thisAgent, preference* pref, char* dest, size_t dest_size)
{
    char pref_type;

    if (!dest)
    {
        dest_size = output_string_size; /* from agent.h */;
        dest = get_printed_output_string();
    }

    pref_type = preference_to_char(pref->type);
    if (m_print_actual)
    {
        sprint_sf(thisAgent, dest, dest_size, "%s(%y ^%y %y) %c %y%s", m_pre_string, pref->id, pref->attr, pref->value,
            preference_to_char(pref->type),
            (m_print_actual && preference_is_binary(pref->type)) ? pref->referent : NULL,
            (pref->o_supported) ? " :O " : NULL);
        return dest;
    }
    else if (m_print_original)
    {
        sprint_sf(thisAgent, dest, dest_size, "%s(%y ^%y %y) %c %y%s", m_pre_string,
            pref->original_symbols.id, pref->original_symbols.attr, pref->original_symbols.value,
            preference_to_char(pref->type),
            (m_print_actual && preference_is_binary(pref->type)) ? pref->referent : NULL,
            (pref->o_supported) ? " :O " : NULL);
        return dest;
    }
    else if (m_print_identity)
    {
        sprint_sf(thisAgent, dest, dest_size, "%s(g%u ^g%u g%u) %c %y%s", m_pre_string,
            pref->g_ids.id, pref->g_ids.attr, pref->g_ids.value,
            preference_to_char(pref->type),
            (m_print_actual && preference_is_binary(pref->type)) ? pref->referent : NULL,
            (pref->o_supported) ? " :O " : NULL);
        return dest;
    }
    return NULL;
}

void Output_Manager::debug_print_preflist_inst(TraceMode mode, preference* top_pref)
{
    preference* pref;
    char pref_type;

    if (!debug_mode_enabled(mode)) return;

    for (pref = top_pref; pref != NIL;)
    {
        print_sf("%p\n", pref);
        pref = pref->inst_next;
    }
}

void Output_Manager::debug_print_preflist_result(TraceMode mode, preference* top_pref)
{
    preference* pref;
    char pref_type;

    if (!debug_mode_enabled(mode)) return;

    for (pref = top_pref; pref != NIL;)
    {
        print_sf("%p\n", pref);
        pref = pref->next_result;
    }
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

void Output_Manager::print_cond_prefs(TraceMode mode, condition* top_cond, preference* top_pref)
{
    if (!debug_mode_enabled(mode)) return;

    /* MToDo | Only print headers and latter two if that setting is on */
    if (m_print_actual)
    {
        print_sf("%s--------------------------- Match --------------------------\n", m_pre_string);
        set_dprint_params(mode, m_pre_string, m_post_string, true, false, false);
        print_condition_list(mode, top_cond);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_inst(mode, top_pref);
    }
    if (m_print_original)
    {
        print_sf("%s-------------------------- Original ------------------------\n", m_pre_string);
        set_dprint_params(mode, m_pre_string, m_post_string, false, true, false);
        print_condition_list(mode, top_cond);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_inst(mode, top_pref);
    }
    if (m_print_identity)
    {
        print_sf("%s------------------------- Identity -------------------------\n", m_pre_string);
        set_dprint_params(mode, m_pre_string, m_post_string, false, false, true);
        print_condition_list(mode, top_cond);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_inst(mode, top_pref);
    }
    print_sf("%s\n", m_pre_string);
    clear_dprint_params(mode);
}

void Output_Manager::print_cond_results(TraceMode mode, condition* top_cond, preference* top_pref)
{
    if (!debug_mode_enabled(mode)) return;

    /* MToDo | Only print headers and latter two if that setting is on */
    if (m_print_actual)
    {
        print_sf("%s--------------------------- Match --------------------------\n", m_pre_string);
        set_dprint_params(mode, m_pre_string, m_post_string, true, false, false);
        print_condition_list(mode, top_cond);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_result(mode, top_pref);
    }
    if (m_print_original)
    {
        print_sf("%s-------------------------- Original ------------------------\n", m_pre_string);
        set_dprint_params(mode, m_pre_string, m_post_string, false, true, false);
        print_condition_list(mode, top_cond);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_result(mode, top_pref);
    }
    if (m_print_identity)
    {
        print_sf("%s------------------------- Identity -------------------------\n", m_pre_string);
        set_dprint_params(mode, m_pre_string, m_post_string, false, false, true);
        print_condition_list(mode, top_cond);
        print_sf("%s-->\n", m_pre_string);
        debug_print_preflist_result(mode, top_pref);
    }
    print_sf("%s\n", m_pre_string);
    clear_dprint_params(mode);
}

void Output_Manager::print_cond_actions(TraceMode mode, condition* top_cond, action* top_action)
{
    if (!debug_mode_enabled(mode)) return;

    if (m_print_actual)
    {
        print_sf("%s--------------------------- Match --------------------------\n", m_pre_string);
        set_dprint_params(mode, m_pre_string, m_post_string, true, false, false);
        print_condition_list(mode, top_cond);
        print_sf("%s-->\n", m_pre_string);
        print_action_list(mode, top_action);
    }
    if (m_print_original)
    {
        set_dprint_params(mode, m_pre_string, m_post_string, false, true, false);
        print_sf("%s-------------------------- Original ------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond);
        print_sf("%s-->\n", m_pre_string);
        print_action_list(mode, top_action);
    }
    if (m_print_identity)
    {
        set_dprint_params(mode, m_pre_string, m_post_string, false, false, true);
        print_sf("%s------------------------- Identity -------------------------\n", m_pre_string);
        print_condition_list(mode, top_cond);
        print_sf("%s-->\n", m_pre_string);
        print_action_list(mode, top_action);
    }
    print_sf("%s\n", m_pre_string);
    clear_dprint_params(mode);
}

void Output_Manager::print_instantiation(TraceMode mode, instantiation* inst)
{
    if (!debug_mode_enabled(mode)) return;

    if (inst->prod)
    {
        print_sf("%sMatched %y ", m_pre_string, inst->prod->name);
    }
    else
    {
        print_sf("%sMatched nothing (dummy production?) \n", m_pre_string);
    }
    print_sf("in state %y (level %i)\n", inst->match_goal, inst->match_goal_level);
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
        print_instantiation(mode, instantiation_list[y]);
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
