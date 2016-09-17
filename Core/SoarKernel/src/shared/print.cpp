/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  print.cpp
 *
 * =======================================================================
 *  These are the routines that support printing Soar data structures.
 *
 * =======================================================================
 */

#include "print.h"

#include "agent.h"
#include "callback.h"
#include "condition.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "run_soar.h"
#include "symbol.h"
#include "soar_TraceNames.h"
#include "test.h"
#include "working_memory.h"
#include "working_memory_activation.h"
#include "xml.h"

#include <stdlib.h>
#include <stdarg.h>

using namespace soar_TraceNames;

void print_spaces(agent* thisAgent, int n)
{
    char* ch;
    char buf[PRINT_BUFSIZE];

    ch = buf;
    while (n)
    {
        *(ch++) = ' ';
        n--;
    }
    *ch = 0;
    thisAgent->outputManager->printa(thisAgent, buf);
}

/* ------------------------------------------------------------------------
                String to Escaped String Conversion
           {Symbol, Test, RHS Value} to String Conversion

   These routines produce strings.  Each takes an optional parameter "dest"
   which, if non-nil, points to the destination buffer for the result string.
   If dest is nil, these routines use a global buffer, and return a pointer
   to it.  (Otherwise "dest" itself is returned.)  Note that a single global
   buffer is shared by all three routines, so callers should assume the
   buffer will be destroyed by the next call to these routines with dest=NIL.

   String_to_escaped_string() takes a string and a first/last char,
   and produces an "escaped string" representation of the string; i.e.,
   a string that uses '\' escapes to include special characters.
   For example, input 'ab"c' with first/last character '"' yields
   '"ab\"c"'.  This is used for printing quoted strings and for printing
   symbols using |vbar| notation.

   Rhs_value_to_string() takes an rhs_value and produces a string
   representation.  The rhs_value MUST NOT be a reteloc.
----------------------------------------------------------------------- */

const std::string string_to_escaped_string(char* s, char first_and_last_char)
{
    std::string returnStr;
    returnStr.push_back(first_and_last_char);
    while (*s)
    {
        if ((*s == first_and_last_char) || (*s == '\\'))
        {
            returnStr.push_back('\\');
        }
        returnStr.push_back(*s++);
    }
    returnStr.push_back(first_and_last_char);
    return returnStr;
}


char const* symbol_to_typeString(agent* /*thisAgent*/, Symbol* sym)
{
    switch (sym->symbol_type)
    {
        case VARIABLE_SYMBOL_TYPE:
            return kTypeVariable ;
        case IDENTIFIER_SYMBOL_TYPE:
            return kTypeID ;
        case INT_CONSTANT_SYMBOL_TYPE:
            return kTypeInt ;
        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return kTypeDouble ;
        case STR_CONSTANT_SYMBOL_TYPE:
            return kTypeString ;
        default:
            return 0 ;
    }
}

char* rhs_value_to_string(rhs_value rv, char* dest, size_t dest_size)
{
    cons* c;
    list* fl;
    rhs_function* rf;
    char* ch;

    if (rhs_value_is_reteloc(rv))
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "Internal error: rhs_value_to_string called on reteloc.\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error_noagent(msg);
    }

    if (rhs_value_is_symbol(rv))
    {
        return rhs_value_to_symbol(rv)->to_string(true, dest, dest_size);
    }

    fl = rhs_value_to_funcall_list(rv);
    rf = static_cast<rhs_function_struct*>(fl->first);

    if (!dest)
    {
        dest = Output_Manager::Get_OM().get_printed_output_string();
        dest_size = output_string_size; /* from agent.h */
    }
    ch = dest;

    strncpy(ch, "(", dest_size);
    ch[dest_size - 1] = 0;
    while (*ch)
    {
        ch++;
    }

    if (!strcmp(rf->name->sc->name, "+"))
    {
        strncpy(ch, "+", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
    }
    else if (!strcmp(rf->name->sc->name, "-"))
    {
        strncpy(ch, "-", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
    }
    else
    {
        rf->name->to_string(true, ch, dest_size - (ch - dest));
    }

    while (*ch)
    {
        ch++;
    }
    for (c = fl->rest; c != NIL; c = c->rest)
    {
        strncpy(ch, " ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
        {
            ch++;
        }
        rhs_value_to_string(static_cast<char*>(c->first), ch, dest_size - (ch - dest));
        while (*ch)
        {
            ch++;
        }
    }
    strncpy(ch, ")", dest_size - (ch - dest));
    ch[dest_size - (ch - dest) - 1] = 0;
    return dest;
}

/* UITODO| Make this preference type to string.  Maybe move to preference struct? */
char preference_to_char(byte type)
{
    switch (type)
    {
        case ACCEPTABLE_PREFERENCE_TYPE:
            return '+';
        case REQUIRE_PREFERENCE_TYPE:
            return '!';
        case REJECT_PREFERENCE_TYPE:
            return '-';
        case PROHIBIT_PREFERENCE_TYPE:
            return '~';
        case NUMERIC_INDIFFERENT_PREFERENCE_TYPE:
            return '=';
        case UNARY_INDIFFERENT_PREFERENCE_TYPE:
            return '=';
        case BINARY_INDIFFERENT_PREFERENCE_TYPE:
            return '=';
        case BEST_PREFERENCE_TYPE:
            return '>';
        case BETTER_PREFERENCE_TYPE:
            return '>';
        case WORST_PREFERENCE_TYPE:
            return '<';
        case WORSE_PREFERENCE_TYPE:
            return '<';
        default:
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg,
                    "print.c: Error: bad type passed to preference_type_indicator\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error_noagent(msg);
        }
    }
    return 0; /* unreachable, but without it, gcc -Wall warns here */
}

/* ------------------------------------------------------------------
                        Print Condition List

   This prints a list of conditions.  The "indent" parameter tells
   how many spaces to indent each line other than the first--the first
   line is not indented (the caller must handle this).  The last line
   is printed without a trailing linefeed.  The "internal" parameter,
   if true, indicates that the condition list should be printed in
   internal format--one condition per line, without grouping all the
   conditions for the same id into one line.
------------------------------------------------------------------ */

bool pick_conds_with_matching_id_test(dl_cons* dc, agent* thisAgent)
{
    condition* cond;
    cond = static_cast<condition_struct*>(dc->item);
    if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
    {
        return false;
    }
    return tests_are_equal(thisAgent->id_test_to_match, cond->data.tests.id_test, false);
}

/*
==============================

==============================
*/
#define PRINT_CONDITION_LIST_TEMP_SIZE 10000
void print_condition_list(agent* thisAgent, condition* conds,
                          int indent, bool internal)
{
    dl_list* conds_not_yet_printed, *tail_of_conds_not_yet_printed;
    dl_list* conds_for_this_id;
    dl_cons* dc;
    condition* c;
    bool removed_goal_test, removed_impasse_test;
    test id_test;
    std::string c_id_test;

    if (!conds)
    {
        return;
    }

    /* --- build dl_list of all the actions --- */
    conds_not_yet_printed = NIL;
    tail_of_conds_not_yet_printed = NIL;

    for (c = conds; c != NIL; c = c->next)
    {
        thisAgent->memoryManager->allocate_with_pool(MP_dl_cons, &dc);
        dc->item = c;
        if (conds_not_yet_printed)
        {
            tail_of_conds_not_yet_printed->next = dc;
        }
        else
        {
            conds_not_yet_printed = dc;
        }
        dc->prev = tail_of_conds_not_yet_printed;
        tail_of_conds_not_yet_printed = dc;
    }
    tail_of_conds_not_yet_printed->next = NIL;

    /* --- main loop: find all conds for first id, print them together --- */
    bool did_one_line_already = false;
    while (conds_not_yet_printed)
    {
        if (did_one_line_already)
        {
            thisAgent->outputManager->printa(thisAgent, "\n");
            print_spaces(thisAgent, indent);
        }
        else
        {
            did_one_line_already = true;
        }

        dc = conds_not_yet_printed;
        remove_from_dll(conds_not_yet_printed, dc, next, prev);
        c = static_cast<condition_struct*>(dc->item);
        if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);
            thisAgent->outputManager->printa(thisAgent, "-{");
            xml_begin_tag(thisAgent, kTagConjunctive_Negation_Condition);
            print_condition_list(thisAgent, c->data.ncc.top, indent + 2, internal);
            xml_end_tag(thisAgent, kTagConjunctive_Negation_Condition);
            thisAgent->outputManager->printa(thisAgent, "}");
            continue;
        }

        /* --- normal pos/neg conditions --- */
        removed_goal_test = removed_impasse_test = false;
        id_test = copy_test_removing_goal_impasse_tests(thisAgent, c->data.tests.id_test,
                  &removed_goal_test,
                  &removed_impasse_test);
        thisAgent->id_test_to_match = copy_of_equality_test_found_in_test(thisAgent, id_test);

        /* --- collect all cond's whose id test matches this one --- */
        conds_for_this_id = dc;
        dc->prev = NIL;
        if (internal)
        {
            dc->next = NIL;
        }
        else
        {
            dc->next = extract_dl_list_elements(thisAgent, &conds_not_yet_printed,
                                                pick_conds_with_matching_id_test);
        }

        /* --- print the collected cond's all together --- */
        thisAgent->outputManager->printa(thisAgent, " (");
        xml_begin_tag(thisAgent, kTagCondition);

        if (removed_goal_test)
        {
            thisAgent->outputManager->printa(thisAgent, "state ");
            xml_att_val(thisAgent, kConditionTest, kConditionTestState);

        }

        if (removed_impasse_test)
        {
            thisAgent->outputManager->printa(thisAgent, "impasse ");
            xml_att_val(thisAgent, kConditionTest, kConditionTestImpasse);
        }
        c_id_test.clear();
        Output_Manager::Get_OM().sprinta_sf(thisAgent, c_id_test, "%t", id_test);
        thisAgent->outputManager->printa(thisAgent, c_id_test.c_str());
        xml_att_val(thisAgent, kConditionId, c_id_test.c_str());
        deallocate_test(thisAgent, thisAgent->id_test_to_match);
        deallocate_test(thisAgent, id_test);

        growable_string gs = make_blank_growable_string(thisAgent);
        while (conds_for_this_id)
        {
            dc = conds_for_this_id;
            conds_for_this_id = conds_for_this_id->next;
            c = static_cast<condition_struct*>(dc->item);
            thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);

            {
                /* --- build and print attr/value test for condition c --- */
                char temp[PRINT_CONDITION_LIST_TEMP_SIZE], *ch;

                memset(temp, 0, PRINT_CONDITION_LIST_TEMP_SIZE);
                ch = temp;
                strncpy(ch, " ", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                if (c->type == NEGATIVE_CONDITION)
                {
                    strncat(ch, "-", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                }
                while (*ch)
                {
                    ch++;
                }

                strncpy(ch, "^", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                while (*ch)
                {
                    ch++;
                }
                Output_Manager::Get_OM().sprinta_sf_cstr(thisAgent, ch, PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp), "%t", c->data.tests.attr_test);
                while (*ch)
                {
                    ch++;
                }
                if (c->data.tests.value_test)
                {
                    *(ch++) = ' ';
                    Output_Manager::Get_OM().sprinta_sf_cstr(thisAgent, ch, PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp), "%t", c->data.tests.value_test);
                    while (*ch)
                    {
                        ch++;
                    }
                    if (c->test_for_acceptable_preference)
                    {
                        strncpy(ch, " +", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                        while (*ch)
                        {
                            ch++;
                        }
                    }
                }
                *ch = 0;
                if (thisAgent->outputManager->get_printer_output_column(thisAgent) + (ch - temp) >= COLUMNS_PER_LINE)
                {
                    thisAgent->outputManager->printa(thisAgent, "\n");
                    print_spaces(thisAgent, indent + 6);
                }
                thisAgent->outputManager->printa(thisAgent, temp);
                add_to_growable_string(thisAgent, &gs, temp);
            }
        }
        xml_att_val(thisAgent, kCondition, text_of_growable_string(gs));
        free_growable_string(thisAgent, gs);
        thisAgent->outputManager->printa(thisAgent, ")");
        xml_end_tag(thisAgent, kTagCondition);
    } /* end of while (conds_not_yet_printed) */
}

/* ------------------------------------------------------------------
                        Print Action List

   This prints a list of actions.  The "indent" parameter tells how
   many spaces to indent each line other than the first--the first
   line is not indented (the caller must handle this).  The last line
   is printed without a trailing linefeed.  The "internal" parameter,
   if true, indicates that the action list should be printed in
   internal format--one action per line, without grouping all the
   actions for the same id into one line.
   Note:  the actions MUST NOT contain any reteloc's.
------------------------------------------------------------------ */

bool pick_actions_with_matching_id(dl_cons* dc, agent* thisAgent)
{
    action* a;
    a = static_cast<action_struct*>(dc->item);
    if (a->type != MAKE_ACTION)
    {
        return false;
    }
    return (rhs_value_to_symbol(a->id) == thisAgent->action_id_to_match);
}

#define PRINT_ACTION_LIST_TEMP_SIZE 10000
void print_action_list(agent* thisAgent, action* actions,
                       int indent, bool internal)
{
    bool did_one_line_already;
    dl_list* actions_not_yet_printed, *tail_of_actions_not_yet_printed;
    dl_list* actions_for_this_id;
    dl_cons* dc;
    action* a;

    if (!actions)
    {
        return;
    }

    did_one_line_already = false;

    /* --- build dl_list of all the actions --- */
    actions_not_yet_printed = NIL;
    tail_of_actions_not_yet_printed = NIL;
    for (a = actions; a != NIL; a = a->next)
    {
        thisAgent->memoryManager->allocate_with_pool(MP_dl_cons, &dc);
        dc->item = a;
        if (actions_not_yet_printed)
        {
            tail_of_actions_not_yet_printed->next = dc;
        }
        else
        {
            actions_not_yet_printed = dc;
        }
        dc->prev = tail_of_actions_not_yet_printed;
        tail_of_actions_not_yet_printed = dc;
    }
    tail_of_actions_not_yet_printed->next = NIL;

    /* --- main loop: find all actions for first id, print them together --- */
    while (actions_not_yet_printed)
    {
        if (did_one_line_already)
        {
            thisAgent->outputManager->printa(thisAgent, "\n");
            print_spaces(thisAgent, indent);
        }
        else
        {
            did_one_line_already = true;
        }
        dc = actions_not_yet_printed;
        remove_from_dll(actions_not_yet_printed, dc, next, prev);
        a = static_cast<action_struct*>(dc->item);
        if (a->type == FUNCALL_ACTION)
        {
            thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);
            xml_begin_tag(thisAgent, kTagAction);
            thisAgent->outputManager->printa(thisAgent, rhs_value_to_string(a->value, NULL, 0));
            xml_att_val(thisAgent, kAction, rhs_value_to_string(a->value, NULL, 0));
            xml_end_tag(thisAgent, kTagAction);
            continue;
        }

        /* --- normal make actions --- */
        /* --- collect all actions whose id matches the first action's id --- */
        actions_for_this_id = dc;
        thisAgent->action_id_to_match = rhs_value_to_symbol(a->id);
        dc->prev = NIL;
        if (internal)
        {
            dc->next = NIL;
        }
        else
        {
            dc->next = extract_dl_list_elements(thisAgent, &actions_not_yet_printed,
                                                pick_actions_with_matching_id);
        }

        /* --- print the collected actions all together --- */
        thisAgent->outputManager->printa_sf(thisAgent, "(%y", thisAgent->action_id_to_match);
        xml_begin_tag(thisAgent, kTagAction);
        xml_att_val(thisAgent, kActionId, thisAgent->action_id_to_match);
        growable_string gs = make_blank_growable_string(thisAgent);
        while (actions_for_this_id)
        {
            dc = actions_for_this_id;
            actions_for_this_id = actions_for_this_id->next;
            a = static_cast<action_struct*>(dc->item);
            thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);

            {
                /* --- build and print attr/value test for action a --- */
                char temp[PRINT_ACTION_LIST_TEMP_SIZE], *ch;

                ch = temp;
                strncpy(ch, " ^", PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp));
                while (*ch)
                {
                    ch++;
                }
                rhs_value_to_string(a->attr, ch, PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp));
                while (*ch)
                {
                    ch++;
                }
                *(ch++) = ' ';
                rhs_value_to_string(a->value, ch, PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp));
                while (*ch)
                {
                    ch++;
                }
                *(ch++) = ' ';
                *(ch++) = preference_to_char(a->preference_type);
                if (preference_is_binary(a->preference_type))
                {
                    *(ch++) = ' ';
                    rhs_value_to_string(a->referent, ch, PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp));
                    while (*ch)
                    {
                        ch++;
                    }
                }
                *ch = 0;
                if (thisAgent->outputManager->get_printer_output_column(thisAgent) + (ch - temp) >=
                        COLUMNS_PER_LINE)
                {
                    thisAgent->outputManager->printa(thisAgent, "\n");
                    print_spaces(thisAgent, indent + 6);
                }
                thisAgent->outputManager->printa(thisAgent, temp);
                add_to_growable_string(thisAgent, &gs, temp);
            }
        }
        xml_att_val(thisAgent, kAction, text_of_growable_string(gs));
        free_growable_string(thisAgent, gs);
        thisAgent->outputManager->printa(thisAgent, ")");
        xml_end_tag(thisAgent, kTagAction);
    } /* end of while (actions_not_yet_printed) */
}

/* ------------------------------------------------------------------
                         Print Production

   This prints a production.  The "internal" parameter, if true,
   indicates that the LHS and RHS should be printed in internal format.
------------------------------------------------------------------ */

void print_production(agent* thisAgent, production* p, bool internal)
{
    condition* top, *bottom;
    action* rhs;

    /*
    --- print "sp" and production name ---
    */
    thisAgent->outputManager->printa_sf(thisAgent, "sp {%y\n", p->name);

    xml_begin_tag(thisAgent, kTagProduction);
    xml_att_val(thisAgent, kProduction_Name, p->name);

    /*
    --- print optional documention string ---
    */
    if (p->documentation)
    {
        std::string temp;
        temp = string_to_escaped_string(p->documentation, '"');
        thisAgent->outputManager->printa_sf(thisAgent, "    %s\n", temp.c_str());
        xml_att_val(thisAgent, kProductionDocumentation, temp.c_str());
    }

    /*
    --- print any flags ---
    */
    switch (p->type)
    {
        case DEFAULT_PRODUCTION_TYPE:
            thisAgent->outputManager->printa(thisAgent, "    :default\n");
            xml_att_val(thisAgent, kProductionType, kProductionTypeDefault);
            break;
        case USER_PRODUCTION_TYPE:
            break;
        case CHUNK_PRODUCTION_TYPE:
            thisAgent->outputManager->printa(thisAgent, "    :chunk\n");
            xml_att_val(thisAgent, kProductionType, kProductionTypeChunk);
            break;
        case JUSTIFICATION_PRODUCTION_TYPE:
            thisAgent->outputManager->printa(thisAgent, "    :justification ;# not reloadable\n");
            xml_att_val(thisAgent, kProductionType, kProductionTypeJustification);
            break;
        case TEMPLATE_PRODUCTION_TYPE:
            thisAgent->outputManager->printa(thisAgent, "   :template\n");
            xml_att_val(thisAgent, kProductionType, kProductionTypeTemplate);
            break;
        case NUM_PRODUCTION_TYPES:
            assert(false);
            break;
    }

    if (p->declared_support == DECLARED_O_SUPPORT)
    {
        thisAgent->outputManager->printa(thisAgent, "    :o-support\n");
        xml_att_val(thisAgent, kProductionDeclaredSupport, kProductionDeclaredOSupport);
    }

    else if (p->declared_support == DECLARED_I_SUPPORT)
    {
        thisAgent->outputManager->printa(thisAgent, "    :i-support\n");
        xml_att_val(thisAgent, kProductionDeclaredSupport, kProductionDeclaredISupport);
    }

    if (p->interrupt && !p->interrupt_break)
    {
        thisAgent->outputManager->printa(thisAgent, "    :interrupt\n");
    }

    /*
    --- print the LHS and RHS ---
    */
    p_node_to_conditions_and_rhs(thisAgent, p->p_node, NIL, NIL,
                                 &top, &bottom, &rhs);
    thisAgent->outputManager->printa(thisAgent, "   ");

    xml_begin_tag(thisAgent, kTagConditions);
    print_condition_list(thisAgent, top, 3, internal);
    xml_end_tag(thisAgent, kTagConditions);
    deallocate_condition_list(thisAgent, top);

    thisAgent->outputManager->printa(thisAgent, "\n    -->\n  ");
    thisAgent->outputManager->printa(thisAgent, "  ");
    xml_begin_tag(thisAgent, kTagActions);
    print_action_list(thisAgent, rhs, 4, internal);
    xml_end_tag(thisAgent, kTagActions);
    thisAgent->outputManager->printa(thisAgent, "\n}\n");
    xml_end_tag(thisAgent, kTagProduction);

    deallocate_action_list(thisAgent, rhs);
}

/* ------------------------------------------------------------------
                       Other Printing Utilities

   Print_condition() prints a single condition.  Print_action() prints
   a single action (which MUST NOT contain any reteloc's).
   Note that these routines work by calling print_condition_list() and
   print_action_list(), respectively, so they print a linefeed if the
   output would go past COLUMNS_PER_LINE.

   Preference_type_indicator() returns a character corresponding to
   a given preference type (byte)--for example, given BEST_PREFERENCE_TYPE,
   it returns '>'.

   Print_preference() prints a given preference.  Print_wme() prints a
   wme (including the timetag).  Print_instantiation_with_wmes() prints
   an instantiation's production name and the wmes it matched, using a
   given wme_trace_type (e.g., TIMETAG_WME_TRACE).
------------------------------------------------------------------ */

void print_condition(agent* thisAgent, condition* cond)
{
    condition* old_next, *old_prev;

    old_next = cond->next;
    old_prev = cond->prev;
    cond->next = NIL;
    cond->prev = NIL;
    print_condition_list(thisAgent, cond, 0, true);
    cond->next = old_next;
    cond->prev = old_prev;
}

void print_action(agent* thisAgent, action* a)
{
    action* old_next;

    old_next = a->next;
    a->next = NIL;
    print_action_list(thisAgent, a, 0, true);
    a->next = old_next;
}

void print_preference(agent* thisAgent, preference* pref)
{
    char pref_type = preference_to_char(pref->type);

    thisAgent->outputManager->printa_sf(thisAgent, "(%y ^%y %y ", pref->id, pref->attr, pref->value);
    thisAgent->outputManager->printa_sf(thisAgent, "%c", pref_type);
    if (preference_is_binary(pref->type))
    {
        thisAgent->outputManager->printa_sf(thisAgent, " %y", pref->referent);
    }
    if (pref->o_supported)
    {
        thisAgent->outputManager->printa(thisAgent, "  :O ");
    }
    thisAgent->outputManager->printa(thisAgent, ")\n");

    // <preference id="s1" attr="foo" value="123" pref_type=">"></preference>
    xml_begin_tag(thisAgent, kTagPreference);
    xml_att_val(thisAgent, kWME_Id, pref->id);
    xml_att_val(thisAgent, kWME_Attribute, pref->attr);
    xml_att_val(thisAgent, kWME_Value, pref->value);

    char buf[2];
    buf[0] = pref_type;
    buf[1] = 0;
    xml_att_val(thisAgent, kPreference_Type, buf);

    if (preference_is_binary(pref->type))
    {
        xml_att_val(thisAgent, kReferent, pref->referent);
    }
    if (pref->o_supported)
    {
        xml_att_val(thisAgent, kOSupported, ":O");
    }
    xml_end_tag(thisAgent, kTagPreference);

}

extern "C" bool passes_wme_filtering(agent* thisAgent, wme* w, bool isAdd);
void
filtered_print_wme_add(agent* thisAgent, wme* w)
{
    if (passes_wme_filtering(thisAgent, w, true))
    {
        thisAgent->outputManager->printa(thisAgent, "=>WM: ");
        xml_begin_tag(thisAgent, kTagWMEAdd);
        print_wme(thisAgent, w);
        xml_end_tag(thisAgent, kTagWMEAdd);
    }
}

void filtered_print_wme_remove(agent* thisAgent, wme* w)
{
    if (passes_wme_filtering(thisAgent, w, false))
    {
        thisAgent->outputManager->printa(thisAgent, "<=WM: ");
        xml_begin_tag(thisAgent, kTagWMERemove);
        print_wme(thisAgent, w);  /*  print_wme takes care of tagged output itself */
        xml_end_tag(thisAgent, kTagWMERemove);
    }
}

void print_wme(agent* thisAgent, wme* w)
{
    thisAgent->outputManager->printa_sf(thisAgent, "(%u: %y ^%y %y", w->timetag, w->id, w->attr, w->value);

    if (wma_enabled(thisAgent))
    {
        thisAgent->outputManager->printa_sf(thisAgent, " [%f]", wma_get_wme_activation(thisAgent, w, true));
    }

    if (w->acceptable)
    {
        thisAgent->outputManager->printa(thisAgent, " +");
    }
    thisAgent->outputManager->printa(thisAgent, ")\n");

    // <wme tag="123" id="s1" attr="foo" attrtype="string" val="123" valtype="string"></wme>
    xml_object(thisAgent, w);
}

void print_wme_without_timetag(agent* thisAgent, wme* w)
{
    thisAgent->outputManager->printa_sf(thisAgent, "(%y ^%y %y", w->id, w->attr, w->value);
    if (w->acceptable)
    {
        thisAgent->outputManager->printa(thisAgent, " +");
    }
    thisAgent->outputManager->printa(thisAgent, ")\n");

    // <wme id="s1" attr="foo" attrtype="string" val="123" valtype="string"></wme>
    xml_object(thisAgent, w, XML_WME_NO_TIMETAG);
}

void print_instantiation_with_wmes(agent* thisAgent, instantiation* inst,
                                   wme_trace_type wtt, int action)
{
    int PRINTING = -1;
    int FIRING = 0;
    int RETRACTING = 1;
    condition* cond;


    if (action == PRINTING)
    {
        xml_begin_tag(thisAgent, kTagProduction);
    }
    else if (action == FIRING)
    {
        xml_begin_tag(thisAgent, kTagProduction_Firing);
        xml_begin_tag(thisAgent, kTagProduction);
    }
    else if (action == RETRACTING)
    {
        xml_begin_tag(thisAgent, kTagProduction_Retracting);
        xml_begin_tag(thisAgent, kTagProduction);
    }

    thisAgent->outputManager->printa_sf(thisAgent, "%y", inst->prod_name);
    xml_att_val(thisAgent, kProduction_Name, inst->prod_name->to_string(true));

    thisAgent->outputManager->printa(thisAgent, "\n");

    if (wtt == NONE_WME_TRACE)
    {
        if (action == PRINTING)
        {
            xml_end_tag(thisAgent, kTagProduction);
        }
        else if (action == FIRING)
        {
            xml_end_tag(thisAgent, kTagProduction);
            xml_end_tag(thisAgent, kTagProduction_Firing);
        }
        else if (action == RETRACTING)
        {
            xml_end_tag(thisAgent, kTagProduction);
            xml_end_tag(thisAgent, kTagProduction_Retracting);
        }
        return;
    }

    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION)
        {
            switch (wtt)
            {
                case TIMETAG_WME_TRACE:
                    thisAgent->outputManager->printa_sf(thisAgent, " %u", cond->bt.wme_->timetag);

                    xml_begin_tag(thisAgent, kTagWME);
                    xml_att_val(thisAgent, kWME_TimeTag, cond->bt.wme_->timetag);
                    xml_end_tag(thisAgent, kTagWME);

                    break;
                case FULL_WME_TRACE:
                    // Not all conds and wme_'s available when retracting, depending on DO_TOP_LEVEL_REF_CTS
#ifdef DO_TOP_LEVEL_REF_CTS
                    thisAgent->outputManager->printa_sf(thisAgent,  " ");
                    print_wme(thisAgent, cond->bt.wme_);
#else
                    if (action != RETRACTING && cond->bt.level > TOP_GOAL_LEVEL)
                    {
                        thisAgent->outputManager->printa_sf(thisAgent, " ");
                        print_wme(thisAgent, cond->bt.wme_);
                    }
                    else
                    {
                        // Wmes that matched the LHS of a retraction may already be free'd; just print tt.
                        thisAgent->outputManager->printa_sf(thisAgent, " %u", cond->bt.wme_->timetag);

                        xml_begin_tag(thisAgent, kTagWME);
                        xml_att_val(thisAgent, kWME_TimeTag, cond->bt.wme_->timetag);
                        xml_end_tag(thisAgent, kTagWME);
                    }
#endif
                    break;
            }
        }

    if (action == PRINTING)
    {
        xml_end_tag(thisAgent, kTagProduction);
    }
    else if (action == FIRING)
    {
        xml_end_tag(thisAgent, kTagProduction);
        xml_end_tag(thisAgent, kTagProduction_Firing);
    }
    else if (action == RETRACTING)
    {
        xml_end_tag(thisAgent, kTagProduction);
        xml_end_tag(thisAgent, kTagProduction_Retracting);
    }
}

/***************************************************************************
* Function     : print_list_of_conditions
**************************************************************************/

void print_list_of_conditions(agent* thisAgent, condition* cond)
{

    while (cond != NULL)
    {
        if (thisAgent->outputManager->get_printer_output_column(thisAgent) >= COLUMNS_PER_LINE - 20)
        {
            thisAgent->outputManager->printa(thisAgent, "\n      ");
        }
        print_condition(thisAgent, cond);
        thisAgent->outputManager->printa(thisAgent, "\n");

        cond = cond->next;
    }
}

void print_phase(agent* thisAgent, const char* s, bool end_of_phase)
{
    // should be more consistent with creating string, but for now, for
    // consistency with previous versions, we'll let calling code set string.
    thisAgent->outputManager->printa(thisAgent, s);

    // the rest is all for tagged output events

    xml_begin_tag(thisAgent, kTagPhase);

    if (end_of_phase)
    {
        xml_att_val(thisAgent, kPhase_Status, kPhaseStatus_End);
    }

    switch (thisAgent->current_phase)
    {
        case INPUT_PHASE:
            xml_att_val(thisAgent, kPhase_Name, kPhaseName_Input);
            break;
        case PREFERENCE_PHASE:
            xml_att_val(thisAgent, kPhase_Name, kPhaseName_Pref);
            break;
        case WM_PHASE:
            xml_att_val(thisAgent, kPhase_Name, kPhaseName_WM);
            switch (thisAgent->FIRING_TYPE)
            {
                case PE_PRODS:  /* no longer needed;  Soar8 has PROPOSE/APPLY */
                    xml_att_val(thisAgent, kPhase_FiringType, kPhaseFiringType_PE);
                    break;
                case IE_PRODS:
                    xml_att_val(thisAgent, kPhase_FiringType, kPhaseFiringType_IE);
                    break;
            }
            break;
        case DECISION_PHASE:
            xml_att_val(thisAgent, kPhase_Name, kPhaseName_Decision);
            break;
        case OUTPUT_PHASE:
            xml_att_val(thisAgent, kPhase_Name, kPhaseName_Output);
            break;
        case PROPOSE_PHASE:
            xml_att_val(thisAgent, kPhase_Name, kPhaseName_Propose);
            break;
        case APPLY_PHASE:
            xml_att_val(thisAgent, kPhase_Name, kPhaseName_Apply);
            break;
        default:
            xml_att_val(thisAgent, kPhase_Name, kPhaseName_Unknown);
            break;
    } // end switch

    xml_end_tag(thisAgent, kTagPhase);
    return;
}

/*
===========================

===========================
*/
bool wme_filter_component_match(Symbol* filterComponent, Symbol* wmeComponent)
{
    if ((filterComponent->symbol_type == STR_CONSTANT_SYMBOL_TYPE) &&
            (!strcmp(filterComponent->sc->name, "*")))
    {
        return true;
    }

    return (filterComponent == wmeComponent);

}

/*
===========================

===========================
*/
extern "C" bool passes_wme_filtering(agent* thisAgent, wme* w, bool isAdd)
{
    cons* c;
    wme_filter* wf;

    /*  print ("testing wme for filtering: ");  print_wme(w); */

    for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest)
    {
        wf = (wme_filter*) c->first;
        /*     thisAgent->outputManager->printa_sf(thisAgent, "  trying filter: %y ^%y %y\n",wf->id,wf->attr,wf->value); */
        if (((isAdd && wf->adds) || ((!isAdd) && wf->removes)) &&
                (!wme_filter_component_match(wf->id, w->id)
                 || !wme_filter_component_match(wf->attr, w->attr)
                 || !wme_filter_component_match(wf->value, w->value)))
        {
            return false;
        }
    }
    return true; /* no defined filters match -> w passes */
}

/*
=================================================================================

                                print_trace

  Prints a message, but only if either the index passed into function
  points to an enabled sysparam.  Accepts format strings. Use INVALID_SYSPARAM
  (or 0) as the index to force print.

=================================================================================
*/
extern void print_sysparam_trace(agent* thisAgent, int64_t sysParamIndex, const char* format, ...)
{
    va_list args;
    std::string buf;

    if ((sysParamIndex == INVALID_SYSPARAM) || thisAgent->sysparams[ sysParamIndex ])
    {
        va_start(args, format);
        thisAgent->outputManager->sprinta_sf(thisAgent, buf, format, args);
        va_end(args);
        xml_generate_warning(thisAgent, buf.c_str());
    }
}
