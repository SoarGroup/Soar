#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  parser.cpp
 *
 * =======================================================================
 *                  Production (SP) Parser
 *
 * There are two top-level routines here:  init_parser(), which
 * should be called at startup time, and parse_production(), which
 * reads an SP (starting from the production name), builds a production,
 * adds it to the rete, and returns a pointer to the new production
 * (or NIL if any error occurred).
 * =======================================================================
 */

#include <stdlib.h>

#include "parser.h"
#include "symbol.h"
#include "kernel.h"
#include "condition.h"
#include "production.h"
#include "mem.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "agent.h"
#include "explain.h"
#include "init_soar.h"
#include "print.h"
#include "rete.h"
#include "xml.h"
#include "reinforcement_learning.h"
#include "semantic_memory.h"
#include "test.h"
#include "debug.h"

#include <ctype.h>

using soar::Lexer;
using soar::Lexeme;

/* =================================================================
                   Placeholder (Dummy) Variables

   In attribute paths (and some other places) we need to create dummy
   variables.  But we need to make sure these dummy variables don't
   accidently have the same names as variables that occur later in
   the user's production.  So, we create "placeholder" variables, whose
   names have funky characters in them so they couldn't possibly occur
   in user-written code.  When we're all done parsing the production, we
   go back and replace the placeholder variables with "real" variables
   (names without funky characters), making sure the real variables
   don't occur anywhere else in the production.
================================================================= */


void reset_placeholder_variable_generator(agent* thisAgent)
{
    int i;
    for (i = 0; i < 26; i++)
    {
        thisAgent->placeholder_counter[i] = 1;
    }
}

Symbol* make_placeholder_var(agent* thisAgent, char first_letter)
{
    Symbol* v;
    char buf[30];
    int i;

    if (!isalpha(first_letter))
    {
        first_letter = 'v';
    }
    i = tolower(first_letter) - static_cast<int>('a');
    assert(i >= 0 && i < 26);

    /* --- create variable with "#" in its name:  this couldn't possibly be a
       variable in the user's code, since the lexer doesn't handle "#" --- */
    SNPRINTF(buf, sizeof(buf) - 1, "<#%c*%lu>", first_letter, static_cast<long unsigned int>(thisAgent->placeholder_counter[i]++));
    buf[sizeof(buf) - 1] = '\0';

    v = make_variable(thisAgent, buf);
    dprint(DT_PARSER, "Adding variable lexeme to parser strings %y\n", v);
    push(thisAgent, (v), thisAgent->parser_syms);
    /* --- indicate that there is no corresponding "real" variable yet --- */
    v->var->current_binding_value = NIL;

    return v;
}

/* -----------------------------------------------------------------
               Make Placeholder (Dummy) Equality Test

   Creates and returns a test for equality with a newly generated
   placeholder variable.
----------------------------------------------------------------- */

test make_placeholder_test(agent* thisAgent, char first_letter)
{
    Symbol* new_var = make_placeholder_var(thisAgent, first_letter);
    test new_test = make_test(thisAgent, new_var, EQUALITY_TEST);
    symbol_remove_ref(thisAgent, new_var);
    return new_test;
}

/* -----------------------------------------------------------------
            Substituting Real Variables for Placeholders

   When done parsing the production, we go back and substitute "real"
   variables for all the placeholders.  This is done by walking all the
   LHS conditions and destructively modifying any tests involving
   placeholders.  The placeholder-->real mapping is maintained on each
   placeholder symbol: placeholder->var->current_binding_value is the
   corresponding "real" variable, or NIL if no such "real" variable has
   been created yet.

   To use this, first call reset_variable_generator (lhs, rhs) with the
   lhs and rhs of the production just parsed; then call
   substitute_for_placeholders_in_condition_list (lhs).
----------------------------------------------------------------- */

void substitute_for_placeholders_in_symbol(agent* thisAgent, Symbol** sym)
{
    char prefix[3];
    Symbol* var;
    bool just_created;

    /* --- if not a variable, do nothing --- */
    if ((*sym)->symbol_type != VARIABLE_SYMBOL_TYPE)
    {
        return;
    }
    /* --- if not a placeholder variable, do nothing --- */
    if (*((*sym)->var->name + 1) != '#')
    {
        return;
    }

    just_created = false;

    if (!(*sym)->var->current_binding_value)
    {
        prefix[0] = *((*sym)->var->name + 2);
        prefix[1] = '*';
        prefix[2] = 0;
        (*sym)->var->current_binding_value = generate_new_variable(thisAgent, prefix);
        dprint(DT_PARSER, "Substituting for placeholder %y with newly generated %y.\n", (*sym), (*sym)->var->current_binding_value);
        just_created = true;
    }
    else
    {
        dprint(DT_PARSER, "Substituting for placeholder %y with existing %y.\n", (*sym), (*sym)->var->current_binding_value);
    }
    var = (*sym)->var->current_binding_value;
    symbol_remove_ref(thisAgent, (*sym));
    *sym = var;
    if (!just_created)
    {
        symbol_add_ref(thisAgent, var);
    }
}

void substitute_for_placeholders_in_test(agent* thisAgent, test* t)
{
    cons* c;
    test ct;

    if (!(*t))
    {
        return;
    }

    ct = *t;

    switch (ct->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case DISJUNCTION_TEST:
            return;
        case CONJUNCTIVE_TEST:
            for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            {
                substitute_for_placeholders_in_test(thisAgent, reinterpret_cast<test*>(&(c->first)));
            }
            return;
        default:  /* relational tests other than equality */
            substitute_for_placeholders_in_symbol(thisAgent, &(ct->data.referent));
            return;
    }
}

void substitute_for_placeholders_in_condition_list(agent* thisAgent,
        condition* cond)
{
    for (; cond != NIL; cond = cond->next)
    {
        switch (cond->type)
        {
            case POSITIVE_CONDITION:
            case NEGATIVE_CONDITION:
                substitute_for_placeholders_in_test(thisAgent, &(cond->data.tests.id_test));
                substitute_for_placeholders_in_test(thisAgent, &(cond->data.tests.attr_test));
                substitute_for_placeholders_in_test(thisAgent, &(cond->data.tests.value_test));
                break;
            case CONJUNCTIVE_NEGATION_CONDITION:
                substitute_for_placeholders_in_condition_list(thisAgent, cond->data.ncc.top);
                break;
        }
    }
}

void substitute_for_placeholders_in_action_list(agent* thisAgent, action* a)
{
    for (; a != NIL; a = a->next)
    {
        if (a->type == MAKE_ACTION)
        {
            rhs_symbol r;
            if (rhs_value_is_symbol(a->id))
            {
                r = rhs_value_to_rhs_symbol(a->id);
                substitute_for_placeholders_in_symbol(thisAgent, &(r->referent));
            }
            if (rhs_value_is_symbol(a->attr))
            {
                r = rhs_value_to_rhs_symbol(a->attr);
                substitute_for_placeholders_in_symbol(thisAgent, &(r->referent));
            }
            if (rhs_value_is_symbol(a->value))
            {
                r = rhs_value_to_rhs_symbol(a->value);
                substitute_for_placeholders_in_symbol(thisAgent, &(r->referent));
            }
        }
    }
}

/* =================================================================

   Grammar for left hand sides of productions

   <lhs> ::= <cond>+
   <cond> ::= <positive_cond> | - <positive_cond>
   <positive_cond> ::= <conds_for_one_id> | { <cond>+ }
   <conds_for_one_id> ::= ( [state|impasse] [<id_test>] <attr_value_tests>* )
   <id_test> ::= <test>
   <attr_value_tests> ::= [-] ^ <attr_test> [.<attr_test>]* <value_test>*
   <attr_test> ::= <test>
   <value_test> ::= <test> [+] | <conds_for_one_id> [+]

   <test> ::= <conjunctive_test> | <simple_test>
   <conjunctive_test> ::= { <simple_test>+ }
   <simple_test> ::= <disjunction_test> | <relational_test>
   <disjunction_test> ::= << <constant>* >>
   <relational_test> ::= [<relation>] <single_test>
   <relation> ::= <> | < | > | <= | >= | = | <=>
   <single_test> ::= <variable> | <constant>
   <constant> ::= str_constant | int_constant | float_constant
   <variable> ::= variable | lti

================================================================= */

const char* help_on_lhs_grammar[] =
{
    "Grammar for left hand sides of productions:",
    "",
    "   <lhs> ::= <cond>+",
    "   <cond> ::= <positive_cond> | - <positive_cond>",
    "   <positive_cond> ::= <conds_for_one_id> | { <cond>+ }",
    "   <conds_for_one_id> ::= ( [state|impasse] [<id_test>] <attr_value_tests>* )",
    "   <id_test> ::= <test>",
    "   <attr_value_tests> ::= [-] ^ <attr_test> [.<attr_test>]* <value_test>*",
    "   <attr_test> ::= <test>",
    "   <value_test> ::= <test> [+] | <conds_for_one_id> [+]",
    "",
    "   <test> ::= <conjunctive_test> | <simple_test>",
    "   <conjunctive_test> ::= { <simple_test>+ }",
    "   <simple_test> ::= <disjunction_test> | <relational_test>",
    "   <disjunction_test> ::= << <constant>* >>",
    "   <relational_test> ::= [<relation>] <single_test>",
    "   <relation> ::= <> | < | > | <= | >= | = | <=>",
    "   <single_test> ::= <variable> | <constant>",
    "   <constant> ::= str_constant | int_constant | float_constant",
    "   <variable> ::= variable | lti",
    "",
    "See also:  rhs-grammar, sp",
    0
};



/* =================================================================

                  Make symbol for current lexeme

================================================================= */

Symbol* make_symbol_for_lexeme(agent* thisAgent, Lexeme* lexeme, bool allow_lti)
{
    Symbol* newSymbol;

    switch (lexeme->type)
    {
        case STR_CONSTANT_LEXEME:
        {
            newSymbol = make_str_constant(thisAgent, lexeme->string());
            push(thisAgent, (newSymbol), thisAgent->parser_syms);
            dprint(DT_PARSER, "Adding lexeme to parser strings %y\n", newSymbol);
            return newSymbol;
        }
        case VARIABLE_LEXEME:
        {
            newSymbol = make_variable(thisAgent, lexeme->string());
            push(thisAgent, (newSymbol), thisAgent->parser_syms);
            dprint(DT_PARSER, "Adding lexeme to parser strings %y\n", newSymbol);
            return newSymbol;
        }
        case INT_CONSTANT_LEXEME:
        {
            newSymbol = make_int_constant(thisAgent, lexeme->int_val);
            push(thisAgent, (newSymbol), thisAgent->parser_syms);
            dprint(DT_PARSER, "Adding lexeme to parser strings %y\n", newSymbol);
            return newSymbol;
        }
        case FLOAT_CONSTANT_LEXEME:
        {
            newSymbol =  make_float_constant(thisAgent, lexeme->float_val);
            push(thisAgent, (newSymbol), thisAgent->parser_syms);
            dprint(DT_PARSER, "Adding lexeme to parser strings %y\n", newSymbol);
            return newSymbol;
        }
        case IDENTIFIER_LEXEME:
            if (!allow_lti)
            {
                char msg[BUFFER_MSG_SIZE];
                strncpy(msg, "parser.c: Internal error:  ID found in make_symbol_for_lexeme\n", BUFFER_MSG_SIZE);
                msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
                abort_with_fatal_error(thisAgent, msg);
            }
            else
            {
                smem_lti_id lti_id = smem_lti_get_id(thisAgent, lexeme->id_letter, lexeme->id_number);

                if (lti_id == NIL)
                {
                    dprint(DT_PARSER, "Could not find LTI @%s%i.\n", lexeme->id_letter, lexeme->id_number);
                    char msg[BUFFER_MSG_SIZE];
                    strncpy(msg, "parser.c: Internal error:  invalid long-term identifier found in make_symbol_for_lexeme\n", BUFFER_MSG_SIZE);
                    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
                    abort_with_fatal_error(thisAgent, msg);
                }
                else
                {
                    newSymbol =  smem_lti_soar_make(thisAgent, lti_id, lexeme->id_letter, lexeme->id_number, SMEM_LTI_UNKNOWN_LEVEL);
                    push(thisAgent, (newSymbol), thisAgent->parser_syms);
                    dprint(DT_PARSER, "Adding lexeme to parser strings %y\n", newSymbol);
                    return newSymbol;
                }
            }
            break;
        default:
        {
            char msg[BUFFER_MSG_SIZE];
            SNPRINTF(msg, BUFFER_MSG_SIZE, "parser.c: Internal error:  bad lexeme type in make_symbol_for_lexeme\n, lexeme->string()=%s\n", lexeme->string());
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
            break;
        }
    }
    return NIL; /* unreachable, but without it, gcc -Wall warns here */
}

/* =================================================================
                          Routines for Tests

   The following routines are used to parse and build <test>'s.
   At entry, they expect the current lexeme to be the start of a
   test.  At exit, they leave the current lexeme at the first lexeme
   following the test.  They return the test read, or NIL if any
   error occurred.  (They never return a blank_test.)
================================================================= */

/* -----------------------------------------------------------------
                      Parse Relational Test

   <relational_test> ::= [<relation>] <single_test>
   <relation> ::= <> | < | > | <= | >= | = | <=>
   <single_test> ::= <variable> | <constant>
   <constant> ::= str_constant | int_constant | float_constant
   <variable> ::= variable | lti
----------------------------------------------------------------- */

test parse_relational_test(agent* thisAgent, Lexer* lexer)
{
    TestType test_type;
    test t;
    Symbol* referent;

    test_type = NOT_EQUAL_TEST; /* unnecessary, but gcc -Wall warns without it */

    /* --- read optional relation symbol --- */
    switch (lexer->current_lexeme.type)
    {
        case EQUAL_LEXEME:
            test_type = EQUALITY_TEST;
            lexer->get_lexeme();
            break;

        case NOT_EQUAL_LEXEME:
            test_type = NOT_EQUAL_TEST;
            lexer->get_lexeme();
            break;

        case LESS_LEXEME:
            test_type = LESS_TEST;
            lexer->get_lexeme();
            break;

        case GREATER_LEXEME:
            test_type = GREATER_TEST;
            lexer->get_lexeme();
            break;

        case LESS_EQUAL_LEXEME:
            test_type = LESS_OR_EQUAL_TEST;
            lexer->get_lexeme();
            break;

        case GREATER_EQUAL_LEXEME:
            test_type = GREATER_OR_EQUAL_TEST;
            lexer->get_lexeme();
            break;

        case LESS_EQUAL_GREATER_LEXEME:
            test_type = SAME_TYPE_TEST;
            lexer->get_lexeme();
            break;
        case STR_CONSTANT_LEXEME:
        case INT_CONSTANT_LEXEME:
        case FLOAT_CONSTANT_LEXEME:
        case VARIABLE_LEXEME:
            test_type = EQUALITY_TEST;
            break;
        default:
            test_type = EQUALITY_TEST;
            break;
    }

    // Check for long term identifier notation
    bool id_lti = parse_lti(thisAgent, lexer);

    /* --- read variable or constant --- */
    switch (lexer->current_lexeme.type)
    {
        case STR_CONSTANT_LEXEME:
        case INT_CONSTANT_LEXEME:
        case FLOAT_CONSTANT_LEXEME:
        case VARIABLE_LEXEME:
        case IDENTIFIER_LEXEME: // IDENTIFIER_LEXEME only possible if id_lti true due to set_lexer_allow_ids above
            referent = make_symbol_for_lexeme(thisAgent, &(lexer->current_lexeme), id_lti);
            lexer->get_lexeme();
            t = make_test(thisAgent, referent, test_type);
            symbol_remove_ref(thisAgent, referent);
            return t;

        default:
            print(thisAgent,  "Expected variable or constant for test\n");
            lexer->print_location_of_most_recent_lexeme();
            return NIL;
    }
}

/* -----------------------------------------------------------------
                      Parse Disjunction Test

   <disjunction_test> ::= << <constant>* >>
   <constant> ::= str_constant | int_constant | float_constant
----------------------------------------------------------------- */

test parse_disjunction_test(agent* thisAgent, Lexer* lexer)
{
    test t;

    if (lexer->current_lexeme.type != LESS_LESS_LEXEME)
    {
        print(thisAgent,  "Expected << to begin disjunction test\n");
        lexer->print_location_of_most_recent_lexeme();
        return NIL;
    }
    lexer->get_lexeme();

    t = make_test(thisAgent, NIL, DISJUNCTION_TEST);

    while (lexer->current_lexeme.type != GREATER_GREATER_LEXEME)
    {
        switch (lexer->current_lexeme.type)
        {
            case STR_CONSTANT_LEXEME:
            case INT_CONSTANT_LEXEME:
            case FLOAT_CONSTANT_LEXEME:
                push(thisAgent, make_symbol_for_lexeme(thisAgent, &(lexer->current_lexeme), false), t->data.disjunction_list);
                lexer->get_lexeme();
                break;
            default:
                print(thisAgent,  "Expected constant or >> while reading disjunction test\n");
                lexer->print_location_of_most_recent_lexeme();
                deallocate_test(thisAgent, t);
                return NIL;
        }
    }
    lexer->get_lexeme();  /* consume the >> */
    t->data.disjunction_list =
        destructively_reverse_list(t->data.disjunction_list);
    return t;
}

/* -----------------------------------------------------------------
                        Parse Simple Test

   <simple_test> ::= <disjunction_test> | <relational_test>
----------------------------------------------------------------- */

test parse_simple_test(agent* thisAgent, Lexer* lexer)
{
    if (lexer->current_lexeme.type == LESS_LESS_LEXEME)
    {
        return parse_disjunction_test(thisAgent, lexer);
    }
    return parse_relational_test(thisAgent, lexer);
}

/* -----------------------------------------------------------------
                            Parse Test

    <test> ::= <conjunctive_test> | <simple_test>
    <conjunctive_test> ::= { <simple_test>+ }
----------------------------------------------------------------- */

test parse_test(agent* thisAgent, Lexer* lexer)
{
    test t, temp;

    if (lexer->current_lexeme.type != L_BRACE_LEXEME)
    {
        return parse_simple_test(thisAgent, lexer);
    }
    /* --- parse and return conjunctive test --- */
    lexer->get_lexeme();
    t = NULL;
    do
    {
        temp = parse_simple_test(thisAgent, lexer);
        if (!temp)
        {
            deallocate_test(thisAgent, t);
            return NIL;
        }
        add_test(thisAgent, &t, temp);
    }
    while (lexer->current_lexeme.type != R_BRACE_LEXEME);
    lexer->get_lexeme(); /* consume the "}" */

    if (t->type == CONJUNCTIVE_TEST)
    {
        t->data.conjunct_list =
            destructively_reverse_list(t->data.conjunct_list);
    }

    return t;
}

/* =================================================================
                        Routines for Conditions

   Various routines here are used to parse and build conditions, etc.
   At entry, each expects the current lexeme to be at the start of whatever
   thing they're supposed to parse.  At exit, each leaves the current
   lexeme at the first lexeme following the parsed object.  Each returns
   a list of the conditions they parsed, or NIL if any error occurred.
================================================================= */

/* -----------------------------------------------------------------
                          Fill In Id Tests
                         Fill In Attr Tests

   As low-level routines (e.g., parse_value_test_star) parse, they
   leave the id (and sometimes attribute) test fields blank (NIL) in the
   condition structures they return.  The calling routine must fill in
   the id tests and/or attribute tests.  These routines fill in any
   still-blank {id, attr} tests with copies of a given test.  They try
   to add non-equality portions of the test only once, if possible.
----------------------------------------------------------------- */

void fill_in_id_tests(agent* thisAgent, condition* conds, test t)
{
    condition* positive_c, *c;
    test equality_test_from_t;

    /* --- see if there's at least one positive condition --- */
    for (positive_c = conds; positive_c != NIL; positive_c = positive_c->next)
        if ((positive_c->type == POSITIVE_CONDITION) &&
                (positive_c->data.tests.id_test == NIL))
        {
            break;
        }

    if (positive_c)    /* --- there is at least one positive condition --- */
    {
        /* --- add just the equality test to most of the conditions --- */
        equality_test_from_t = copy_of_equality_test_found_in_test(thisAgent, t);
        for (c = conds; c != NIL; c = c->next)
        {
            if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
            {
                fill_in_id_tests(thisAgent, c->data.ncc.top, equality_test_from_t);
            }
            else if (c->data.tests.id_test == NIL)
            {
                c->data.tests.id_test = copy_test(thisAgent, equality_test_from_t);
            }
        }
        deallocate_test(thisAgent, equality_test_from_t);
        /* --- add the whole test to one positive condition --- */
        deallocate_test(thisAgent, positive_c->data.tests.id_test);
        positive_c->data.tests.id_test = copy_test(thisAgent, t);
        return;
    }

    /* --- all conditions are negative --- */
    for (c = conds; c != NIL; c = c->next)
    {
        if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            fill_in_id_tests(thisAgent, c->data.ncc.top, t);
        }
        else
        {
            if (c->data.tests.id_test == NIL)
            {
                c->data.tests.id_test = copy_test(thisAgent, t);
            }
        }
    }
}

void fill_in_attr_tests(agent* thisAgent, condition* conds, test t)
{
    condition* positive_c, *c;
    test equality_test_from_t;

    /* --- see if there's at least one positive condition --- */
    for (positive_c = conds; positive_c != NIL; positive_c = positive_c->next)
        if ((positive_c->type == POSITIVE_CONDITION) &&
                (positive_c->data.tests.attr_test == NIL))
        {
            break;
        }

    if (positive_c)    /* --- there is at least one positive condition --- */
    {
        /* --- add just the equality test to most of the conditions --- */
        equality_test_from_t = copy_of_equality_test_found_in_test(thisAgent, t);
        for (c = conds; c != NIL; c = c->next)
        {
            if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
            {
                fill_in_attr_tests(thisAgent, c->data.ncc.top, equality_test_from_t);
            }
            else if (c->data.tests.attr_test == NIL)
            {
                c->data.tests.attr_test = copy_test(thisAgent, equality_test_from_t);
            }
        }
        deallocate_test(thisAgent, equality_test_from_t);
        /* --- add the whole test to one positive condition --- */
        deallocate_test(thisAgent, positive_c->data.tests.attr_test);
        positive_c->data.tests.attr_test = copy_test(thisAgent, t);
        return;
    }

    /* --- all conditions are negative --- */
    for (c = conds; c != NIL; c = c->next)
    {
        if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            fill_in_attr_tests(thisAgent, c->data.ncc.top, t);
        }
        else
        {
            if (c->data.tests.attr_test == NIL)
            {
                c->data.tests.attr_test = copy_test(thisAgent, t);
            }
        }
    }
}

/* -----------------------------------------------------------------
                     Negate Condition List

   Returns the negation of the given condition list.  If the given
   list is a single positive or negative condition, it just toggles
   the type.  If the given list is a single ncc, it strips off the ncc
   part and returns the subconditions.  Otherwise it makes a new ncc
   using the given conditions.
----------------------------------------------------------------- */

condition* negate_condition_list(agent* thisAgent, condition* conds)
{
    condition* temp, *last;

    if (conds->next == NIL)
    {
        /* --- only one condition to negate, so toggle the type --- */
        switch (conds->type)
        {
            case POSITIVE_CONDITION:
                conds->type = NEGATIVE_CONDITION;
                return conds;
            case NEGATIVE_CONDITION:
                conds->type = POSITIVE_CONDITION;
                return conds;
            case CONJUNCTIVE_NEGATION_CONDITION:
                temp = conds->data.ncc.top;
                thisAgent->memoryManager->free_with_pool(MP_condition, conds);
                return temp;
        }
    }
    /* --- more than one condition; so build a conjunctive negation --- */
    thisAgent->memoryManager->allocate_with_pool(MP_condition,  &temp);
    init_condition(temp);
    temp->type = CONJUNCTIVE_NEGATION_CONDITION;
    temp->data.ncc.top = conds;
    for (last = conds; last->next != NIL; last = last->next);
    temp->data.ncc.bottom = last;
    return temp;
}

/* -----------------------------------------------------------------
                        Parse Value Test Star

   <value_test> ::= <test> [+] | <conds_for_one_id> [+]

   (This routine parses <value_test>*, given as input the id_test and
   attr_test already read.)
----------------------------------------------------------------- */

condition* parse_conds_for_one_id(agent* thisAgent, Lexer* lexer,
                                  char first_letter_if_no_id_given,
                                  test* dest_id_test);

condition* parse_value_test_star(agent* thisAgent, Lexer* lexer, char first_letter)
{
    condition* c, *last_c, *first_c, *new_conds;
    test value_test;
    bool acceptable;

    if ((lexer->current_lexeme.type == MINUS_LEXEME) ||
            (lexer->current_lexeme.type == UP_ARROW_LEXEME) ||
            (lexer->current_lexeme.type == R_PAREN_LEXEME))
    {
        /* --- value omitted, so create dummy value test --- */
        thisAgent->memoryManager->allocate_with_pool(MP_condition,  &c);
        init_condition(c);
        c->type = POSITIVE_CONDITION;
        c->data.tests.value_test = make_placeholder_test(thisAgent, first_letter);
        return c;
    }

    first_c = NIL;
    last_c = NIL;
    do
    {
        if (lexer->current_lexeme.type == L_PAREN_LEXEME)
        {
            /* --- read <conds_for_one_id>, take the id_test from it --- */
            new_conds = parse_conds_for_one_id(thisAgent, lexer, first_letter, &value_test);
            if (!new_conds)
            {
                deallocate_condition_list(thisAgent, first_c);
                return NIL;
            }
        }
        else
        {
            /* --- read <value_test> --- */
            new_conds = NIL;
            value_test = parse_test(thisAgent, lexer);
            if (!value_test)
            {
                deallocate_condition_list(thisAgent, first_c);
                return NIL;
            }
            if (!value_test->eq_test)
            {
                add_test(thisAgent, &value_test, make_placeholder_test(thisAgent, first_letter));
            }
        }
        /* --- check for acceptable preference indicator --- */
        acceptable = false;
        if (lexer->current_lexeme.type == PLUS_LEXEME)
        {
            acceptable = true;
            lexer->get_lexeme();
        }
        /* --- build condition using the new value test --- */
        thisAgent->memoryManager->allocate_with_pool(MP_condition,  &c);
        init_condition(c);
        c->type = POSITIVE_CONDITION;
        c->data.tests.value_test = value_test;
        c->test_for_acceptable_preference = acceptable;
        insert_at_head_of_dll(new_conds, c, next, prev);
        /* --- add new conditions to the end of the list --- */
        if (last_c)
        {
            last_c->next = new_conds;
        }
        else
        {
            first_c = new_conds;
        }
        new_conds->prev = last_c;
        for (last_c = new_conds; last_c->next != NIL; last_c = last_c->next);
    }
    while ((lexer->current_lexeme.type != MINUS_LEXEME) &&
            (lexer->current_lexeme.type != UP_ARROW_LEXEME) &&
            (lexer->current_lexeme.type != R_PAREN_LEXEME));
    return first_c;
}

/* -----------------------------------------------------------------
                      Parse Attr Value Tests

   <attr_value_tests> ::= [-] ^ <attr_test> [.<attr_test>]* <value_test>*
   <attr_test> ::= <test>

   (This routine parses <attr_value_tests>, given as input the id_test
   already read.)
----------------------------------------------------------------- */

condition* parse_attr_value_tests(agent* thisAgent, Lexer* lexer)
{
    test id_test_to_use, attr_test;
    bool negate_it;
    condition* first_c, *last_c, *c, *new_conds;

    /* --- read optional minus sign --- */
    negate_it = false;
    if (lexer->current_lexeme.type == MINUS_LEXEME)
    {
        negate_it = true;
        lexer->get_lexeme();
    }

    /* --- read up arrow --- */
    if (lexer->current_lexeme.type != UP_ARROW_LEXEME)
    {
        print(thisAgent,  "Expected ^ followed by attribute\n");
        lexer->print_location_of_most_recent_lexeme();
        return NIL;
    }
    lexer->get_lexeme();

    first_c = NIL;
    last_c = NIL;

    /* --- read first <attr_test> --- */
    attr_test = parse_test(thisAgent, lexer);
    if (!attr_test)
    {
        return NIL;
    }
    if (!attr_test->eq_test)
    {
        add_test(thisAgent, &attr_test, make_placeholder_test(thisAgent, 'a'));
    }

    /* --- read optional attribute path --- */
    id_test_to_use = NIL;
    while (lexer->current_lexeme.type == PERIOD_LEXEME)
    {
        lexer->get_lexeme();  /* consume the "." */
        /* --- setup for next attribute in path:  make a dummy variable,
           create a new condition in the path --- */
        thisAgent->memoryManager->allocate_with_pool(MP_condition,  &c);
        init_condition(c);
        c->type = POSITIVE_CONDITION;
        if (last_c)
        {
            last_c->next = c;
        }
        else
        {
            first_c = c;
        }
        c->next = NIL;
        c->prev = last_c;
        last_c = c;
        if (id_test_to_use)
        {
            c->data.tests.id_test = copy_test(thisAgent, id_test_to_use);
        }
        else
        {
            c->data.tests.id_test = NIL;
        }
        c->data.tests.attr_test = attr_test;
        id_test_to_use = make_placeholder_test(thisAgent, first_letter_from_test(attr_test));
        c->data.tests.value_test = id_test_to_use;
        /* --- update id and attr tests for the next path element --- */
        attr_test = parse_test(thisAgent, lexer);
        if (!attr_test)
        {
            deallocate_condition_list(thisAgent, first_c);
            return NIL;
        }
        /* AGR 544 begin */
        if (!attr_test->eq_test)
        {
            add_test(thisAgent, &attr_test, make_placeholder_test(thisAgent, 'a'));
        }
        /* AGR 544 end */
    } /* end of while (lexer->current_lexeme.type==PERIOD_LEXEME) */

    /* --- finally, do the <value_test>* part --- */
    new_conds = parse_value_test_star(thisAgent, lexer, first_letter_from_test(attr_test));
    if (!new_conds)
    {
        deallocate_condition_list(thisAgent, first_c);
        deallocate_test(thisAgent, attr_test);
        return NIL;
    }
    fill_in_attr_tests(thisAgent, new_conds, attr_test);
    if (id_test_to_use)
    {
        fill_in_id_tests(thisAgent, new_conds, id_test_to_use);
    }
    deallocate_test(thisAgent, attr_test);
    if (last_c)
    {
        last_c->next = new_conds;
    }
    else
    {
        first_c = new_conds;
    }
    new_conds->prev = last_c;
    /* should update last_c here, but it's not needed anymore */

    /* --- negate everything if necessary --- */
    if (negate_it)
    {
        first_c = negate_condition_list(thisAgent, first_c);
    }

    return first_c;
}

/* -----------------------------------------------------------------
                    Parse Head Of Conds For One Id

   <conds_for_one_id> ::= ( [state|impasse] [<id_test>] <attr_value_tests>* )
   <id_test> ::= <test>

   This routine parses the "( [state|impasse] [<id_test>]" part of
   <conds_for_one_id> and returns the resulting id_test (or NIL if
   any error occurs).
----------------------------------------------------------------- */

test parse_head_of_conds_for_one_id(agent* thisAgent, Lexer* lexer, char first_letter_if_no_id_given)
{
    test id_test, id_goal_impasse_test, check_for_symconstant;
    Symbol* sym;

    if (lexer->current_lexeme.type != L_PAREN_LEXEME)
    {
        print(thisAgent,  "Expected ( to begin condition element\n");
        lexer->print_location_of_most_recent_lexeme();
        return NIL;
    }
    lexer->get_lexeme();

    /* --- look for goal/impasse indicator --- */
    if (lexer->current_lexeme.type == STR_CONSTANT_LEXEME)
    {
        if (!strcmp(lexer->current_lexeme.string(), "state"))
        {
            id_goal_impasse_test = make_test(thisAgent, NIL, GOAL_ID_TEST);
            lexer->get_lexeme();
            first_letter_if_no_id_given = 's';
        }
        else if (!strcmp(lexer->current_lexeme.string(), "impasse"))
        {
            id_goal_impasse_test = make_test(thisAgent, NIL, IMPASSE_ID_TEST);
            lexer->get_lexeme();
            first_letter_if_no_id_given = 'i';
        }
        else
        {
            id_goal_impasse_test = NULL;
        }
    }
    else
    {
        id_goal_impasse_test = NULL;
    }

    /* --- read optional id test; create dummy one if none given --- */
    if ((lexer->current_lexeme.type != MINUS_LEXEME) &&
            (lexer->current_lexeme.type != UP_ARROW_LEXEME) &&
            (lexer->current_lexeme.type != R_PAREN_LEXEME))
    {
        id_test = parse_test(thisAgent, lexer);
        if (!id_test)
        {
            deallocate_test(thisAgent, id_goal_impasse_test);
            return NIL;
        }
        if (!id_test->eq_test)
        {
            add_test
            (thisAgent, &id_test, make_placeholder_test(thisAgent, first_letter_if_no_id_given));
        }
        else
        {
            check_for_symconstant = copy_of_equality_test_found_in_test(thisAgent, id_test);
            sym = check_for_symconstant->data.referent;
            deallocate_test(thisAgent, check_for_symconstant);  /* RBD added 3/28/95 */

            // Symbol type can only be IDENTIFIER_SYMBOL_TYPE if it is a long term identifier (lti),
            // Otherwise, it isn't possible to have an IDENTIFIER_SYMBOL_TYPE here.
            if ((sym->symbol_type != VARIABLE_SYMBOL_TYPE) && (sym->symbol_type != IDENTIFIER_SYMBOL_TYPE))
            {
                print_with_symbols(thisAgent, "Warning: Constant %y in id field test.\n", sym);
                print(thisAgent,  "         This will never match.\n");

                growable_string gs = make_blank_growable_string(thisAgent);
                add_to_growable_string(thisAgent, &gs, "Warning: Constant ");
                add_to_growable_string(thisAgent, &gs, sym->to_string(true));
                add_to_growable_string(thisAgent, &gs, " in id field test.\n         This will never match.");
                xml_generate_warning(thisAgent, text_of_growable_string(gs));
                free_growable_string(thisAgent, gs);
                //TODO: should we append this to the previous XML message or create a new message for it?
                lexer->print_location_of_most_recent_lexeme();
                deallocate_test(thisAgent, id_test);    /* AGR 527c */
                return NIL;                  /* AGR 527c */
            }
        }
    }
    else
    {
        id_test = make_placeholder_test(thisAgent, first_letter_if_no_id_given);
    }

    /* --- add the goal/impasse test to the id test --- */
    add_test(thisAgent, &id_test, id_goal_impasse_test);

    /* --- return the resulting id test --- */
    return id_test;
}

/* -----------------------------------------------------------------
                    Parse Tail Of Conds For One Id

   <conds_for_one_id> ::= ( [state|impasse] [<id_test>] <attr_value_tests>* )
   <id_test> ::= <test>

   This routine parses the "<attr_value_tests>* )" part of <conds_for_one_id>
   and returns the resulting conditions (or NIL if any error occurs).
   It does not fill in the id tests of the conditions.
----------------------------------------------------------------- */

condition* parse_tail_of_conds_for_one_id(agent* thisAgent, Lexer* lexer)
{
    condition* first_c, *last_c, *new_conds;
    condition* c;

    /* --- if no <attr_value_tests> are given, create a dummy one --- */
    if (lexer->current_lexeme.type == R_PAREN_LEXEME)
    {
        lexer->get_lexeme();       /* consume the right parenthesis */
        thisAgent->memoryManager->allocate_with_pool(MP_condition, &c);
        init_condition(c);
        c->type = POSITIVE_CONDITION;
        c->data.tests.attr_test = make_placeholder_test(thisAgent, 'a');
        c->data.tests.value_test = make_placeholder_test(thisAgent, 'v');
        return c;
    }

    /* --- read <attr_value_tests>* --- */
    first_c = NIL;
    last_c = NIL;
    while (lexer->current_lexeme.type != R_PAREN_LEXEME)
    {
        new_conds = parse_attr_value_tests(thisAgent, lexer);
        if (!new_conds)
        {
            deallocate_condition_list(thisAgent, first_c);
            return NIL;
        }
        if (last_c)
        {
            last_c->next = new_conds;
        }
        else
        {
            first_c = new_conds;
        }
        new_conds->prev = last_c;
        for (last_c = new_conds; last_c->next != NIL; last_c = last_c->next);
    }

    /* --- reached the end of the condition --- */
    lexer->get_lexeme();       /* consume the right parenthesis */

    return first_c;
}

/* -----------------------------------------------------------------
                      Parse Conds For One Id

   <conds_for_one_id> ::= ( [state|impasse] [<id_test>] <attr_value_tests>* )
   <id_test> ::= <test>

   This routine parses <conds_for_one_id> and returns the conditions (of
   NIL if any error occurs).

   If the argument dest_id_test is non-NULL, then *dest_id_test is set
   to the resulting complete id test (which includes any goal/impasse test),
   and in all the conditions the id field is filled in with just the
   equality portion of id_test.

   If the argument dest_id_test is NULL, then the complete id_test is
   included in the conditions.
----------------------------------------------------------------- */

condition* parse_conds_for_one_id(agent* thisAgent, Lexer* lexer, char first_letter_if_no_id_given,
                                  test* dest_id_test)
{
    condition* conds;
    test id_test, equality_test_from_id_test;

    /* --- parse the head --- */
    id_test = parse_head_of_conds_for_one_id(thisAgent, lexer, first_letter_if_no_id_given);
    if (! id_test)
    {
        return NIL;
    }

    /* --- parse the tail --- */
    conds = parse_tail_of_conds_for_one_id(thisAgent, lexer);
    if (! conds)
    {
        deallocate_test(thisAgent, id_test);
        return NIL;
    }

    /* --- fill in the id test in all the conditions just read --- */
    if (dest_id_test)
    {
        *dest_id_test = id_test;
        equality_test_from_id_test = copy_of_equality_test_found_in_test(thisAgent, id_test);
        fill_in_id_tests(thisAgent, conds, equality_test_from_id_test);
        deallocate_test(thisAgent, equality_test_from_id_test);
    }
    else
    {
        fill_in_id_tests(thisAgent, conds, id_test);
        deallocate_test(thisAgent, id_test);
    }

    return conds;
}

/* -----------------------------------------------------------------
                            Parse Cond

   <cond> ::= <positive_cond> | - <positive_cond>
   <positive_cond> ::= <conds_for_one_id> | { <cond>+ }
----------------------------------------------------------------- */

condition* parse_cond_plus(agent* thisAgent, Lexer* lexer);

condition* parse_cond(agent* thisAgent, Lexer* lexer)
{
    condition* c;
    bool negate_it;

    /* --- look for leading "-" sign --- */
    negate_it = false;
    if (lexer->current_lexeme.type == MINUS_LEXEME)
    {
        negate_it = true;
        lexer->get_lexeme();
    }

    /* --- parse <positive_cond> --- */
    if (lexer->current_lexeme.type == L_BRACE_LEXEME)
    {
        /* --- read conjunctive condition --- */
        lexer->get_lexeme();
        c = parse_cond_plus(thisAgent, lexer);
        if (!c)
        {
            return NIL;
        }
        if (lexer->current_lexeme.type != R_BRACE_LEXEME)
        {
            print(thisAgent,  "Expected } to end conjunctive condition\n");
            lexer->print_location_of_most_recent_lexeme();
            deallocate_condition_list(thisAgent, c);
            return NIL;
        }
        lexer->get_lexeme();  /* consume the R_BRACE */
    }
    else
    {
        /* --- read conds for one id --- */
        c = parse_conds_for_one_id(thisAgent, lexer, 's', NULL);
        if (!c)
        {
            return NIL;
        }
    }

    /* --- if necessary, handle the negation --- */
    if (negate_it)
    {
        c = negate_condition_list(thisAgent, c);
    }

    return c;
}

/* -----------------------------------------------------------------
                            Parse Cond Plus

   (Parses <cond>+ and builds a condition list.)
----------------------------------------------------------------- */

condition* parse_cond_plus(agent* thisAgent, Lexer* lexer)
{
    condition* first_c, *last_c, *new_conds;

    first_c = NIL;
    last_c = NIL;
    do
    {
        /* --- get individual <cond> --- */
        new_conds = parse_cond(thisAgent, lexer);
        if (!new_conds)
        {
            deallocate_condition_list(thisAgent, first_c);
            return NIL;
        }
        if (last_c)
        {
            last_c->next = new_conds;
        }
        else
        {
            first_c = new_conds;
        }
        new_conds->prev = last_c;
        for (last_c = new_conds; last_c->next != NIL; last_c = last_c->next);
    }
    while ((lexer->current_lexeme.type == MINUS_LEXEME) ||
            (lexer->current_lexeme.type == L_PAREN_LEXEME) ||
            (lexer->current_lexeme.type == L_BRACE_LEXEME));
    return first_c;
}

/* -----------------------------------------------------------------
                            Parse LHS

   (Parses <lhs> and builds a condition list.)

   <lhs> ::= <cond>+
----------------------------------------------------------------- */

condition* parse_lhs(agent* thisAgent, Lexer* lexer)
{
    condition* c;

    c = parse_cond_plus(thisAgent, lexer);
    if (!c)
    {
        return NIL;
    }
    return c;
}



/* =================================================================

                        Routines for Actions

   The following routines are used to parse and build actions, etc.
   Except as otherwise noted, at entry each routine expects the
   current lexeme to be at the start of whatever thing it's supposed
   to parse.  At exit, each leaves the current lexeme at the first
   lexeme following the parsed object.
================================================================= */

/* =====================================================================

   Grammar for right hand sides of productions

   <rhs> ::= <rhs_action>*
   <rhs_action> ::= ( <variable> <attr_value_make>+ ) | <function_call>
   <function_call> ::= ( <function_name> <rhs_value>* )
   <function_name> ::= str_constant | + | -
     (WARNING: might need others besides +, - here if the lexer changes)
   <rhs_value> ::= <constant> | <function_call> | <variable>
   <constant> ::= str_constant | int_constant | float_constant
   <attr_value_make> ::= ^ <rhs_value> <value_make>+
   <value_make> ::= <rhs_value> <preferences>
   <variable> ::= variable | lti

   <preferences> ::= [,] | <preference_specifier>+
   <preference-specifier> ::= <naturally-unary-preference> [,]
                            | <forced-unary-preference>
                            | <binary-preference> <rhs_value> [,]
   <naturally-unary-preference> ::= + | - | ! | ~ | @
   <binary-preference> ::= > | = | < | &
   <any-preference> ::= <naturally-unary-preference> | <binary-preference>
   <forced-unary-preference> ::= <binary-preference>
                                 {<any-preference> | , | ) | ^}
     ;but the parser shouldn't consume the <any-preference>, ")" or "^"
      lexeme here
===================================================================== */

const char* help_on_rhs_grammar[] =
{
    "Grammar for right hand sides of productions:",
    "",
    "   <rhs> ::= <rhs_action>*",
    "   <rhs_action> ::= ( <variable> <attr_value_make>+ ) | <function_call>",
    "   <function_call> ::= ( <function_name> <rhs_value>* )",
    "   <function_name> ::= str_constant | + | -",
    "   <rhs_value> ::= <constant> | <function_call> | <variable>",
    "   <constant> ::= str_constant | int_constant | float_constant",
    "   <attr_value_make> ::= ^ <rhs_value> <value_make>+",
    "   <value_make> ::= <rhs_value> <preferences>",
    "   <variable> ::= variable | lti",
    "",
    "   <preferences> ::= [,] | <preference_specifier>+",
    "   <preference-specifier> ::= <naturally-unary-preference> [,]",
    "                            | <forced-unary-preference>",
    "                            | <binary-preference> <rhs_value> [,]",
    "   <naturally-unary-preference> ::= + | - | ! | ~ | @",
    "   <binary-preference> ::= > | = | < | &",
    "   <any-preference> ::= <naturally-unary-preference> | <binary-preference>",
    "   <forced-unary-preference> ::= <binary-preference> ",
    "                                 {<any-preference> | , | ) | ^}",
    "     ;but the parser shouldn't consume the <any-preference>, \")\" or \"^\"",
    "      lexeme here",
    "",
    "See also:  lhs-grammar, sp",
    0
};

/* -----------------------------------------------------------------
                 Parse Function Call After Lparen

   Parses a <function_call> after the "(" has already been consumed.
   At entry, the current lexeme should be the function name.  Returns
   an rhs_value, or NIL if any error occurred.

   <function_call> ::= ( <function_name> <rhs_value>* )
   <function_name> ::= str_constant | + | -
     (Warning: might need others besides +, - here if the lexer changes)
----------------------------------------------------------------- */

rhs_value parse_rhs_value(agent* thisAgent, Lexer* lexer);

rhs_value parse_function_call_after_lparen(agent* thisAgent,
        Lexer* lexer, bool is_stand_alone_action)
{
    rhs_function* rf;
    Symbol* fun_name;
    list* fl;
    cons* c, *prev_c;
    rhs_value arg_rv;
    int num_args;

    /* --- read function name, find the rhs_function structure --- */
    if (lexer->current_lexeme.type == PLUS_LEXEME)
    {
        fun_name = find_str_constant(thisAgent, "+");
    }
    else if (lexer->current_lexeme.type == MINUS_LEXEME)
    {
        fun_name = find_str_constant(thisAgent, "-");
    }
    else
    {
        fun_name = find_str_constant(thisAgent, lexer->current_lexeme.string());
    }

	if (!fun_name && (std::string(lexer->current_lexeme.string()) == "succeeded" || std::string(lexer->current_lexeme.string()) == "failed"))
	{
		print(thisAgent, "WARNING: Replacing function named %s with halt since this is a unit test but running in a non-unit testing environment.\n", lexer->current_lexeme.string());
		fun_name = find_str_constant(thisAgent, "halt");
	}

    if (!fun_name)
    {
        print(thisAgent,  "No RHS function named %s\n", lexer->current_lexeme.string());
        lexer->print_location_of_most_recent_lexeme();
        return NIL;
    }
    rf = lookup_rhs_function(thisAgent, fun_name);

	if (!rf && (std::string(lexer->current_lexeme.string()) == "succeeded" || std::string(lexer->current_lexeme.string()) == "failed"))
	{
		print(thisAgent, "WARNING: Replacing function named %s with halt since this is a unit test but running in a non-unit testing environment.\n", lexer->current_lexeme.string());
		rf = lookup_rhs_function(thisAgent, find_str_constant(thisAgent, "halt"));
	}

    if (!rf)
    {
        print(thisAgent,  "No RHS function named %s\n", lexer->current_lexeme.string());
        lexer->print_location_of_most_recent_lexeme();
        return NIL;
    }

    /* --- make sure stand-alone/rhs_value is appropriate --- */
    if (is_stand_alone_action && (! rf->can_be_stand_alone_action))
    {
        print(thisAgent,  "Function %s cannot be used as a stand-alone action\n",
              lexer->current_lexeme.string());
        lexer->print_location_of_most_recent_lexeme();
        return NIL;
    }
    if ((! is_stand_alone_action) && (! rf->can_be_rhs_value))
    {
        print(thisAgent,  "Function %s can only be used as a stand-alone action\n",
              lexer->current_lexeme.string());
        lexer->print_location_of_most_recent_lexeme();
        return NIL;
    }

    /* --- build list of rhs_function and arguments --- */
    allocate_cons(thisAgent, &fl);
    fl->first = rf;
    prev_c = fl;
    lexer->get_lexeme(); /* consume function name, advance to argument list */
    num_args = 0;
    while (lexer->current_lexeme.type != R_PAREN_LEXEME)
    {
        arg_rv = parse_rhs_value(thisAgent, lexer);
        if (!arg_rv)
        {
            prev_c->rest = NIL;
            deallocate_rhs_value(thisAgent, funcall_list_to_rhs_value(fl));
            return NIL;
        }
        num_args++;
        allocate_cons(thisAgent, &c);
        c->first = arg_rv;
        prev_c->rest = c;
        prev_c = c;
    }
    prev_c->rest = NIL;

    /* --- check number of arguments --- */
    if ((rf->num_args_expected != -1) && (rf->num_args_expected != num_args))
    {
        print(thisAgent,  "Wrong number of arguments to function %s (expected %d)\n",
              rf->name->sc->name, rf->num_args_expected);
        lexer->print_location_of_most_recent_lexeme();
        deallocate_rhs_value(thisAgent, funcall_list_to_rhs_value(fl));
        return NIL;
    }

    lexer->get_lexeme();  /* consume the right parenthesis */
    return funcall_list_to_rhs_value(fl);
}

/* -----------------------------------------------------------------
                          Parse RHS Value

   Parses an <rhs_value>.  Returns an rhs_value, or NIL if any error
   occurred.

   <rhs_value> ::= <constant> | <function_call> | <variable>
   <constant> ::= str_constant | int_constant | float_constant
   <variable> ::= variable | lti
----------------------------------------------------------------- */

rhs_value parse_rhs_value(agent* thisAgent, Lexer* lexer)
{
    rhs_value rv;

    if (lexer->current_lexeme.type == L_PAREN_LEXEME)
    {
        lexer->get_lexeme();
        return parse_function_call_after_lparen(thisAgent, lexer, false);
    }

    // Check for long term identifier notation
    bool id_lti = parse_lti(thisAgent, lexer);

    if ((lexer->current_lexeme.type == STR_CONSTANT_LEXEME) ||
            (lexer->current_lexeme.type == INT_CONSTANT_LEXEME) ||
            (lexer->current_lexeme.type == FLOAT_CONSTANT_LEXEME) ||
            (lexer->current_lexeme.type == VARIABLE_LEXEME) ||
            (lexer->current_lexeme.type == IDENTIFIER_LEXEME))
    {
        // IDENTIFIER_LEXEME only possible if id_lti true due to set_lexer_allow_ids above
        Symbol* new_sym = make_symbol_for_lexeme(thisAgent, &(lexer->current_lexeme), id_lti);
        rv = allocate_rhs_value_for_symbol_no_refcount(thisAgent, new_sym, 0);
        lexer->get_lexeme();
        return rv;
    }
    print(thisAgent,  "Illegal value for RHS value\n");
    lexer->print_location_of_most_recent_lexeme();
    return NULL;
}


/* -----------------------------------------------------------------
                         Is Preference Lexeme

   Given a token type, returns true if the token is a preference
   lexeme.

----------------------------------------------------------------- */

bool is_preference_lexeme(enum lexer_token_type test_lexeme)
{
    switch (test_lexeme)
    {

        case PLUS_LEXEME:
            return true;
        case MINUS_LEXEME:
            return true;
        case EXCLAMATION_POINT_LEXEME:
            return true;
        case TILDE_LEXEME:
            return true;
        case GREATER_LEXEME:
            return true;
        case EQUAL_LEXEME:
            return true;
        case LESS_LEXEME:
            return true;
        case AMPERSAND_LEXEME:
            return true;
        default:
            return false;
    }
}

/* -----------------------------------------------------------------
               Parse Preference Specifier Without Referent

   Parses a <preference-specifier>.  Returns the appropriate
   xxx_PREFERENCE_TYPE

   Note:  in addition to the grammar below, if there is no preference
   specifier given, then this routine returns ACCEPTABLE_PREFERENCE_TYPE.
   Also, for <binary-preference>'s, this routine *does not* read the
   <rhs_value> referent.  This must be done by the caller routine.

   <preference-specifier> ::= <naturally-unary-preference> [,]
                            | <forced-unary-preference>
                            | <binary-preference> <rhs_value> [,]
   <naturally-unary-preference> ::= + | - | ! | ~ | @
   <binary-preference> ::= > | = | < | &
   <any-preference> ::= <naturally-unary-preference> | <binary-preference>
   <forced-unary-preference> ::= <binary-preference>
                                 {<any-preference> | , | ) | ^}
     ;but the parser shouldn't consume the <any-preference>, ")" or "^"
      lexeme here
----------------------------------------------------------------- */

PreferenceType parse_preference_specifier_without_referent(agent* thisAgent, Lexer* lexer)
{
    switch (lexer->current_lexeme.type)
    {

        case PLUS_LEXEME:
            lexer->get_lexeme();
            if (lexer->current_lexeme.type == COMMA_LEXEME)
            {
                lexer->get_lexeme();
            }
            return ACCEPTABLE_PREFERENCE_TYPE;

        case MINUS_LEXEME:
            lexer->get_lexeme();
            if (lexer->current_lexeme.type == COMMA_LEXEME)
            {
                lexer->get_lexeme();
            }
            return REJECT_PREFERENCE_TYPE;

        case EXCLAMATION_POINT_LEXEME:
            lexer->get_lexeme();
            if (lexer->current_lexeme.type == COMMA_LEXEME)
            {
                lexer->get_lexeme();
            }
            return REQUIRE_PREFERENCE_TYPE;

        case TILDE_LEXEME:
            lexer->get_lexeme();
            if (lexer->current_lexeme.type == COMMA_LEXEME)
            {
                lexer->get_lexeme();
            }
            return PROHIBIT_PREFERENCE_TYPE;

        /****************************************************************************
         * [Soar-Bugs #55] <forced-unary-preference> ::= <binary-preference>
         *                                             {<any-preference> | , | ) | ^}
         *
         *   Forced unary preferences can now occur when a binary preference is
         *   followed by a ",", ")", "^" or any preference specifier
         ****************************************************************************/

        case GREATER_LEXEME:
            lexer->get_lexeme();
            if ((lexer->current_lexeme.type != COMMA_LEXEME) &&
                    (lexer->current_lexeme.type != R_PAREN_LEXEME) &&
                    (lexer->current_lexeme.type != UP_ARROW_LEXEME) &&
                    (!is_preference_lexeme(lexer->current_lexeme.type)))
            {
                return BETTER_PREFERENCE_TYPE;
            }
            /* --- forced unary preference --- */
            if (lexer->current_lexeme.type == COMMA_LEXEME)
            {
                lexer->get_lexeme();
            }
            return BEST_PREFERENCE_TYPE;

        case EQUAL_LEXEME:
            lexer->get_lexeme();
            if ((lexer->current_lexeme.type != COMMA_LEXEME) &&
                    (lexer->current_lexeme.type != R_PAREN_LEXEME) &&
                    (lexer->current_lexeme.type != UP_ARROW_LEXEME) &&
                    (!is_preference_lexeme(lexer->current_lexeme.type)))
            {

                if ((lexer->current_lexeme.type == INT_CONSTANT_LEXEME) ||
                        (lexer->current_lexeme.type == FLOAT_CONSTANT_LEXEME))
                {
                    return NUMERIC_INDIFFERENT_PREFERENCE_TYPE;
                }
                else
                {
                    return BINARY_INDIFFERENT_PREFERENCE_TYPE;
                }
            }

            /* --- forced unary preference --- */
            if (lexer->current_lexeme.type == COMMA_LEXEME)
            {
                lexer->get_lexeme();
            }
            return UNARY_INDIFFERENT_PREFERENCE_TYPE;

        case LESS_LEXEME:
            lexer->get_lexeme();
            if ((lexer->current_lexeme.type != COMMA_LEXEME) &&
                    (lexer->current_lexeme.type != R_PAREN_LEXEME) &&
                    (lexer->current_lexeme.type != UP_ARROW_LEXEME) &&
                    (!is_preference_lexeme(lexer->current_lexeme.type)))
            {
                return WORSE_PREFERENCE_TYPE;
            }
            /* --- forced unary preference --- */
            if (lexer->current_lexeme.type == COMMA_LEXEME)
            {
                lexer->get_lexeme();
            }
            return WORST_PREFERENCE_TYPE;

        default:
            /* --- if no preference given, make it an acceptable preference --- */
            return ACCEPTABLE_PREFERENCE_TYPE;
    } /* end of switch statement */
}

/* -----------------------------------------------------------------
                         Parse Preferences

   Given the id, attribute, and value already read, this routine
   parses zero or more <preference-specifier>'s.  It builds and
   returns an action list for these RHS make's.  It returns NIL if
   any error occurred.

   <value_make> ::= <rhs_value> <preferences>
   <preferences> ::= [,] | <preference_specifier>+
   <preference-specifier> ::= <naturally-unary-preference> [,]
                            | <forced-unary-preference>
                            | <binary-preference> <rhs_value> [,]
----------------------------------------------------------------- */

action* parse_preferences(agent* thisAgent, Lexer* lexer, Symbol* id,
                          rhs_value attr, rhs_value value)
{
    action* a;
    action* prev_a;
    rhs_value referent;
    PreferenceType preference_type;
    bool saw_plus_sign;

    /* --- Note: this routine is set up so if there's not preference type
       indicator at all, we return a single acceptable preference make --- */

    prev_a = NIL;

    saw_plus_sign = (lexer->current_lexeme.type == PLUS_LEXEME);
    preference_type = parse_preference_specifier_without_referent(thisAgent, lexer);
    if ((preference_type == ACCEPTABLE_PREFERENCE_TYPE) && (! saw_plus_sign))
    {
        /* If the routine gave us a + pref without seeing a + sign, then it's
           just giving us the default acceptable preference.  Look for optional
           comma. */
        if (lexer->current_lexeme.type == COMMA_LEXEME)
        {
            lexer->get_lexeme();
        }
    }

    while (true)
    {
        /* --- read referent --- */
        if (preference_is_binary(preference_type))
        {
            referent = parse_rhs_value(thisAgent, lexer);
            if (! referent)
            {
                deallocate_action_list(thisAgent, prev_a);
                return NIL;
            }
            if (lexer->current_lexeme.type == COMMA_LEXEME)
            {
                lexer->get_lexeme();
            }
        }
        else
        {
            referent = NIL; /* unnecessary, but gcc -Wall warns without it */
        }

        /* --- create the appropriate action --- */
        a = make_action(thisAgent);
        a->next = prev_a;
        prev_a = a;
        a->type = MAKE_ACTION;
        a->preference_type = preference_type;
        a->id = allocate_rhs_value_for_symbol(thisAgent, id, 0);
        a->attr = copy_rhs_value(thisAgent, attr);
        a->value = copy_rhs_value(thisAgent, value);
        if (preference_is_binary(preference_type))
        {
            a->referent = referent;
        }

        /* --- look for another preference type specifier --- */
        saw_plus_sign = (lexer->current_lexeme.type == PLUS_LEXEME);
        preference_type = parse_preference_specifier_without_referent(thisAgent, lexer);

        /* --- exit loop when done reading preferences --- */

        /* If the routine gave us a + pref without seeing a + sign, then it's
           just giving us the default acceptable preference, it didn't see any
           more preferences specified. */
        if ((preference_type == ACCEPTABLE_PREFERENCE_TYPE) && (! saw_plus_sign))
        {
            return prev_a;
        }
    }
    return NIL; // Unreachable.  Eliminates compiler warning.
}

/* -----------------------------------------------------------------
              Parse Preferences for Soar8 Non-Operators

   Given the id, attribute, and value already read, this routine
   parses zero or more <preference-specifier>'s.  If preferences
   other than reject and acceptable are specified, it prints
   a warning message that they are being ignored.  It builds an
   action list for creating an ACCEPTABLE preference.  If binary
   preferences are encountered, a warning message is printed and
   the production is ignored (returns NIL).  It returns NIL if any
   other error occurred.

   <value_make> ::= <rhs_value> <preferences>
   <preferences> ::= [,] | <preference_specifier>+
   <preference-specifier> ::= <naturally-unary-preference> [,]
                            | <forced-unary-preference>
                            | <binary-preference> <rhs_value> [,]
----------------------------------------------------------------- */

action* parse_preferences_soar8_non_operator(agent* thisAgent, Lexer* lexer, Symbol* id,
        rhs_value attr, rhs_value value)
{
    action* a;
    action* prev_a;
    rhs_value referent;
    PreferenceType preference_type;
    bool saw_plus_sign;

    /* JC ADDED: for printint */
    char szPrintAttr[256];
    char szPrintValue[256];
    char szPrintId[256];

    /* --- Note: this routine is set up so if there's not preference type
       indicator at all, we return an acceptable preference make.  For
       non-operators, allow only REJECT_PREFERENCE_TYPE, (and ACCEPTABLE).
       If any other preference type indicator is found, a warning or
       error msg (error only on binary prefs) is printed. --- */

    prev_a = NIL;

    saw_plus_sign = (lexer->current_lexeme.type == PLUS_LEXEME);
    preference_type = parse_preference_specifier_without_referent(thisAgent, lexer);
    if ((preference_type == ACCEPTABLE_PREFERENCE_TYPE) && (! saw_plus_sign))
    {
        /* If the routine gave us a + pref without seeing a + sign, then it's
           just giving us the default acceptable preference.  Look for optional
           comma. */
        if (lexer->current_lexeme.type == COMMA_LEXEME)
        {
            lexer->get_lexeme();
        }
    }

    while (true)
    {
        /* step through the pref list, print warning messages when necessary. */

        /* --- read referent --- */
        if (preference_is_binary(preference_type))
        {
            print(thisAgent,  "\nERROR: Binary preference illegal for non-operator.");

            /* JC BUG FIX: Have to check to make sure that the rhs_values are converted to strings
                     correctly before we print */
            rhs_value_to_string(attr, szPrintAttr, 256);
            rhs_value_to_string(value, szPrintValue, 256);
            id->to_string(true, szPrintId, 256);
            print(thisAgent,  "id = %s\t attr = %s\t value = %s\n", szPrintId, szPrintAttr, szPrintValue);

            deallocate_action_list(thisAgent, prev_a);
            return NIL;

        }
        else
        {
            referent = NIL; /* unnecessary, but gcc -Wall warns without it */
        }

        if ((preference_type != ACCEPTABLE_PREFERENCE_TYPE) &&
                (preference_type != REJECT_PREFERENCE_TYPE))
        {
            print(thisAgent,  "\nWARNING: The only allowable non-operator preference \nis REJECT - .\nIgnoring specified preferences.\n");
            xml_generate_warning(thisAgent, "WARNING: The only allowable non-operator preference \nis REJECT - .\nIgnoring specified preferences.");

            /* JC BUG FIX: Have to check to make sure that the rhs_values are converted to strings
                     correctly before we print */
            rhs_value_to_string(attr, szPrintAttr, 256);
            rhs_value_to_string(value, szPrintValue, 256);
            id->to_string(true, szPrintId, 256);
            print(thisAgent,  "id = %s\t attr = %s\t value = %s\n", szPrintId, szPrintAttr, szPrintValue);

            lexer->print_location_of_most_recent_lexeme();
        }

        if (preference_type == REJECT_PREFERENCE_TYPE)
        {
            /* --- create the appropriate action --- */
            a = make_action(thisAgent);
            a->next = prev_a;
            prev_a = a;
            a->type = MAKE_ACTION;
            a->preference_type = preference_type;
            a->id = allocate_rhs_value_for_symbol(thisAgent, id, 0);
            a->attr = copy_rhs_value(thisAgent, attr);
            a->value = copy_rhs_value(thisAgent, value);
        }

        /* --- look for another preference type specifier --- */
        saw_plus_sign = (lexer->current_lexeme.type == PLUS_LEXEME);
        preference_type = parse_preference_specifier_without_referent(thisAgent, lexer);

        /* --- exit loop when done reading preferences --- */
        if ((preference_type == ACCEPTABLE_PREFERENCE_TYPE) && (! saw_plus_sign))
        {
            /* If the routine gave us a + pref without seeing a + sign, then it's
               just giving us the default acceptable preference, it didn't see any
               more preferences specified. */

            /* for soar8, if this wasn't a REJECT preference, then
              create acceptable preference makes.  */
            if (prev_a == NIL)
            {
                a = make_action(thisAgent);
                a->next = prev_a;
                prev_a = a;
                a->type = MAKE_ACTION;
                a->preference_type = ACCEPTABLE_PREFERENCE_TYPE;
                a->id = allocate_rhs_value_for_symbol(thisAgent, id, 0);
                a->attr = copy_rhs_value(thisAgent, attr);
                a->value = copy_rhs_value(thisAgent, value);
            }
            return prev_a;
        }
    }
}
/* KJC end:  10.09.98 */

/* -----------------------------------------------------------------
                      Parse Attr Value Make

   Given the id already read, this routine parses an <attr_value_make>.
   It builds and returns an action list for these RHS make's.  It
   returns NIL if any error occurred.

   <attr_value_make> ::= ^ <rhs_value> <value_make>+
   <value_make> ::= <rhs_value> <preferences>
----------------------------------------------------------------- */

action* parse_attr_value_make(agent* thisAgent, Lexer* lexer, Symbol* id)
{
    rhs_value attr, value;
    action* all_actions, *new_actions, *last;
    Symbol* old_id, *new_var;

    /* JC Added, need to store the attribute name */
    char    szAttribute[256];

    if (lexer->current_lexeme.type != UP_ARROW_LEXEME)
    {
        print(thisAgent,  "Expected ^ in RHS make action\n");
        lexer->print_location_of_most_recent_lexeme();
        return NIL;
    }
    old_id = id;

    lexer->get_lexeme(); /* consume up-arrow, advance to attribute */
    attr = parse_rhs_value(thisAgent, lexer);
    if (! attr)
    {
        return NIL;
    }

    /* JC Added, we will need the attribute as a string, so we get it here */
    rhs_value_to_string(attr, szAttribute, 256);

    all_actions = NIL;

    /*  allow dot notation "." in RHS attribute path  10/15/98 KJC */
    while (lexer->current_lexeme.type == PERIOD_LEXEME)
    {
        lexer->get_lexeme(); /* consume the "."  */

        /* set up for next attribute in path: make dummy variable,
           and create new action in the path */
        new_var = make_placeholder_var(thisAgent, first_letter_from_rhs_value(attr));

        /* parse_preferences actually creates the action.  Even though
         there aren't really any preferences to read, we need the default
         acceptable prefs created for all attributes in path */

        if (strcmp(szAttribute, "operator") != 0)
        {
            new_actions = parse_preferences_soar8_non_operator(thisAgent, lexer, id, attr,
                          allocate_rhs_value_for_symbol(thisAgent, new_var, 0));
        }
        else
        {
            new_actions = parse_preferences(thisAgent, lexer, id, attr, allocate_rhs_value_for_symbol(thisAgent, new_var, 0));
        }

        for (last = new_actions; last->next != NIL; last = last->next)
            /* continue */;

        last->next = all_actions;
        all_actions = new_actions;

        /* Remove references for dummy var used to represent dot notation links */
        deallocate_rhs_value(thisAgent, attr);
        symbol_remove_ref(thisAgent, new_var);

        /* if there was a "." then there must be another attribute
           set id for next action and get the next attribute */
        id = new_var;
        attr = parse_rhs_value(thisAgent, lexer);
        if (! attr)
        {
            return NIL;
        }

        /* JC Added. We need to get the new attribute's name */
        rhs_value_to_string(attr, szAttribute, 256);
    }
    /* end of while (lexer->current_lexeme.type == PERIOD_LEXEME */
    /* end KJC 10/15/98 */

    do
    {
        value = parse_rhs_value(thisAgent, lexer);
        if (!value)
        {
            deallocate_rhs_value(thisAgent, attr);
            deallocate_action_list(thisAgent, all_actions);
            return NIL;
        }
        if (strcmp(szAttribute, "operator") != 0)
        {
            new_actions = parse_preferences_soar8_non_operator(thisAgent, lexer, id, attr, value);
        }
        else
        {
            new_actions = parse_preferences(thisAgent, lexer, id, attr, value);
        }
        deallocate_rhs_value(thisAgent, value);
        if (!new_actions)
        {
            deallocate_rhs_value(thisAgent, attr);
            return NIL;
        }
        for (last = new_actions; last->next != NIL; last = last->next);
        last->next = all_actions;
        all_actions = new_actions;
    }
    while ((lexer->current_lexeme.type != R_PAREN_LEXEME) &&
            (lexer->current_lexeme.type != UP_ARROW_LEXEME));

    deallocate_rhs_value(thisAgent, attr);
    return all_actions;
}

/* -----------------------------------------------------------------
                        Parse RHS Action

   Parses an <rhs_action> and returns an action list.  If any error
   occurrs, NIL is returned.

   <rhs_action> ::= ( <variable> <attr_value_make>+ ) | <function_call>
----------------------------------------------------------------- */

action* parse_rhs_action(agent* thisAgent, Lexer* lexer)
{
    action* all_actions, *new_actions, *last;
    Symbol* var = NULL;
    rhs_value funcall_value;

    if (lexer->current_lexeme.type != L_PAREN_LEXEME)
    {
        print(thisAgent,  "Expected ( to begin RHS action\n");
        lexer->print_location_of_most_recent_lexeme();
        return NIL;
    }
    lexer->get_lexeme();

    // Check for long term identifier notation
    bool id_lti = parse_lti(thisAgent, lexer);

    if ((lexer->current_lexeme.type != VARIABLE_LEXEME) && (lexer->current_lexeme.type != IDENTIFIER_LEXEME))
    {
        /* --- the action is a function call --- */
        funcall_value = parse_function_call_after_lparen(thisAgent, lexer, true);
        if (!funcall_value)
        {
            return NIL;
        }
        all_actions = make_action(thisAgent);
        all_actions->type = FUNCALL_ACTION;
        all_actions->value = funcall_value;
        return all_actions;
    }
    /* --- the action is a regular make action --- */
    if (id_lti)
    {
        smem_lti_id lti_id = smem_lti_get_id(thisAgent, lexer->current_lexeme.id_letter, lexer->current_lexeme.id_number);

        if (lti_id == NIL)
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg, "parser.c: Internal error:  invalid long-term identifier found in make_symbol_for_lexeme\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }
        else
        {
            var = smem_lti_soar_make(thisAgent, lti_id, lexer->current_lexeme.id_letter, lexer->current_lexeme.id_number, SMEM_LTI_UNKNOWN_LEVEL);
            /* I don't think we need to add these two here to the parser clean up list.  It seems it cleans it up at the end --*/
            // push (thisAgent, (var), thisAgent->parser_syms);
        }
    }
    else
    {
        var = make_variable(thisAgent, lexer->current_lexeme.string());
//    push (thisAgent, (var), thisAgent->parser_syms);
    }

    lexer->get_lexeme();
    all_actions = NIL;
    while (lexer->current_lexeme.type != R_PAREN_LEXEME)
    {
        new_actions = parse_attr_value_make(thisAgent, lexer, var);
        if (new_actions)
        {
            for (last = new_actions; last->next != NIL; last = last->next);
            last->next = all_actions;
            all_actions = new_actions;
        }
        else
        {
            symbol_remove_ref(thisAgent, var);
            deallocate_action_list(thisAgent, all_actions);
            return NIL;
        }
    }
    lexer->get_lexeme();  /* consume the right parenthesis */
    symbol_remove_ref(thisAgent, var);
    return all_actions;
}

bool parse_lti(agent* thisAgent, Lexer* lexer)
{
    switch (lexer->current_lexeme.type)
    {
        case AT_LEXEME:
        {
            bool saved = lexer->get_allow_ids();
            lexer->set_allow_ids(true);
            lexer->get_lexeme();
            lexer->set_allow_ids(saved);
        }
        return true;

        default:
            break;
    }
    return false;
}

/* -----------------------------------------------------------------
                            Parse RHS

   Parses the <rhs> and sets *dest_rhs to the resulting action list.
   Returns true if successful, false if any error occurred.

   <rhs> ::= <rhs_action>*
----------------------------------------------------------------- */

bool parse_rhs(agent* thisAgent, Lexer* lexer, action** dest_rhs)
{
    action* all_actions, *new_actions, *last;

    all_actions = NIL;
    while (lexer->current_lexeme.type != EOF_LEXEME)
    {
        new_actions = parse_rhs_action(thisAgent, lexer);
        if (new_actions)
        {
            for (last = new_actions; last->next != NIL; last = last->next);
            last->next = all_actions;
            all_actions = new_actions;
        }
        else
        {
            deallocate_action_list(thisAgent, all_actions);
            return false;
        }
    }
    *dest_rhs = all_actions;
    return true;
}




/* =================================================================
                 Destructively Reverse Action List

   As the parser builds the action list for the RHS, it adds each new
   action onto the front of the list.  This results in the order of
   the actions getting reversed.  This has certain problems--for example,
   if there are several (write) actions on the RHS, reversing their order
   means the output lines get printed in the wrong order.  To avoid this
   problem, we reverse the list after building it.

   This routine destructively reverses an action list.
================================================================= */

action* destructively_reverse_action_list(action* a)
{
    action* prev, *current, *next;

    prev = NIL;
    current = a;
    while (current)
    {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}


/* =================================================================
                        Parse Production

   This routine reads a production (everything inside the body of the
   "sp" command), builds a production, and adds it to the rete.

   If successful, it returns a pointer to the new production struct.
   If any error occurred, it returns NIL (and may or may not read
   the rest of the body of the sp).
================================================================= */
production* parse_production(agent* thisAgent, const char* prod_string, unsigned char* rete_addition_result)
{
    Symbol*         name;
    char*           documentation;
    condition       *lhs, *lhs_top, *lhs_bottom;
    action*         rhs;
    production*     p;
    SupportType     declared_support;
    ProductionType  prod_type;

    Lexer lexer(thisAgent, prod_string);
    lexer.set_allow_ids(false);
    lexer.get_lexeme();

    bool rhs_okay, interrupt_on_match, explain_chunks;

    reset_placeholder_variable_generator(thisAgent);

    /* --- read production name --- */
    if (lexer.current_lexeme.type != STR_CONSTANT_LEXEME)
    {
        print(thisAgent,  "Expected symbol for production name\n");
        lexer.print_location_of_most_recent_lexeme();
        return NIL;
    }
    name = make_str_constant(thisAgent, lexer.current_lexeme.string());
    lexer.get_lexeme();

    /* --- if there's already a prod with this name, excise it --- */
    if (name->sc->production)
    {
        excise_production(thisAgent, name->sc->production, (true && thisAgent->sysparams[TRACE_LOADING_SYSPARAM]));
    }

    /* --- read optional documentation string --- */
    if (lexer.current_lexeme.type == QUOTED_STRING_LEXEME)
    {
        documentation = make_memory_block_for_string(thisAgent, lexer.current_lexeme.string());
        lexer.get_lexeme();
    }
    else
    {
        documentation = NIL;
    }

    /* --- read optional flags --- */
    declared_support = UNDECLARED_SUPPORT;
    prod_type = USER_PRODUCTION_TYPE;
    interrupt_on_match = false;
    explain_chunks = false;
    while (true)
    {
        if (lexer.current_lexeme.type != STR_CONSTANT_LEXEME)
        {
            break;
        }
        if (!strcmp(lexer.current_lexeme.string(), ":o-support"))
        {
            declared_support = DECLARED_O_SUPPORT;
            lexer.get_lexeme();
            continue;
        }
        if (!strcmp(lexer.current_lexeme.string(), ":i-support"))
        {
            declared_support = DECLARED_I_SUPPORT;
            lexer.get_lexeme();
            continue;
        }
        if (!strcmp(lexer.current_lexeme.string(), ":chunk"))
        {
            prod_type = CHUNK_PRODUCTION_TYPE;
            lexer.get_lexeme();
            continue;
        }
        if (!strcmp(lexer.current_lexeme.string(), ":default"))
        {
            prod_type = DEFAULT_PRODUCTION_TYPE;
            lexer.get_lexeme();
            continue;
        }
        if (!strcmp(lexer.current_lexeme.string(), ":template"))
        {
            prod_type = TEMPLATE_PRODUCTION_TYPE;
            lexer.get_lexeme();
            continue;
        }
        if (!strcmp(lexer.current_lexeme.string(), ":interrupt"))
        {
            interrupt_on_match = true;
            lexer.get_lexeme();
            continue;
        }
        if (!strcmp(lexer.current_lexeme.string(), ":explain-chunks"))
        {
            explain_chunks = true;
            lexer.get_lexeme();
            continue;
        }
        break;
    } /* end of while (true) */

    /* --- read the LHS --- */
    dprint(DT_PARSER, "Parsing LHS\n");
    lhs = parse_lhs(thisAgent, &lexer);
    if (!lhs)
    {
        print_with_symbols(thisAgent, "(Ignoring production %y)\n\n", name);
        if (documentation)
        {
            free_memory_block_for_string(thisAgent, documentation);
        }
        symbol_remove_ref(thisAgent, name);
        thisAgent->current_production_name = NIL;
        return NIL;
    }

    /* --- read the "-->" --- */
    if (lexer.current_lexeme.type != RIGHT_ARROW_LEXEME)
    {
        print(thisAgent,  "Expected --> in production\n");
        lexer.print_location_of_most_recent_lexeme();
        print_with_symbols(thisAgent, "(Ignoring production %y)\n\n", name);
        if (documentation)
        {
            free_memory_block_for_string(thisAgent, documentation);
        }
        symbol_remove_ref(thisAgent, name);
        thisAgent->current_production_name = NIL;
        deallocate_condition_list(thisAgent, lhs);
        return NIL;
    }
    lexer.get_lexeme();

    /* --- read the RHS --- */
    dprint(DT_PARSER, "Parsing RHS\n");
    rhs_okay = parse_rhs(thisAgent, &lexer, &rhs);
    if (!rhs_okay)
    {
        print_with_symbols(thisAgent, "(Ignoring production %y)\n\n", name);
        if (documentation)
        {
            free_memory_block_for_string(thisAgent, documentation);
        }
        symbol_remove_ref(thisAgent, name);
        thisAgent->current_production_name = NIL;
        deallocate_condition_list(thisAgent, lhs);
        return NIL;
    }
    rhs = destructively_reverse_action_list(rhs);

    /* --- replace placeholder variables with real variables --- */
    reset_variable_generator(thisAgent, lhs, rhs);
    substitute_for_placeholders_in_condition_list(thisAgent, lhs);
    substitute_for_placeholders_in_action_list(thisAgent, rhs);

    /* --- everything parsed okay, so make the production structure --- */
    lhs_top = lhs;
    for (lhs_bottom = lhs; lhs_bottom->next != NIL; lhs_bottom = lhs_bottom->next);
    dprint(DT_PARSER, "Parse OK.  Making production.\n");

    p = make_production(thisAgent, prod_type, name, name->sc->name, &lhs_top, &rhs, true);
    if (!p)
    {
        if (documentation)
        {
            free_memory_block_for_string(thisAgent, documentation);
        }
        print_with_symbols(thisAgent, "(Ignoring production %y)\n\n", name);
        symbol_remove_ref(thisAgent, name);
        thisAgent->current_production_name = NIL;
        deallocate_condition_list(thisAgent, lhs_top);
        deallocate_action_list(thisAgent, rhs);
        return NIL;
    }

    if (prod_type == TEMPLATE_PRODUCTION_TYPE)
    {
        if (!rl_valid_template(p))
        {
            print_with_symbols(thisAgent, "Invalid Soar-RL template (%y)\n\n", name);
            excise_production(thisAgent, p, false);
            return NIL;
        }
    }

    p->documentation = documentation;
    p->declared_support = declared_support;
    p->interrupt = interrupt_on_match;
    if (explain_chunks) {
        thisAgent->explanationLogger->toggle_production_watch(p);
    } else {
        p->explain_its_chunks = false;
    }
    production* duplicate_rule = NULL;
    *rete_addition_result = add_production_to_rete(thisAgent, p, lhs_top, NIL, true, duplicate_rule);
    deallocate_condition_list(thisAgent, lhs_top);

    if (*rete_addition_result == DUPLICATE_PRODUCTION)
    {
        excise_production(thisAgent, p, false);
        p = NIL;
    }

    if (p && p->rl_rule && p->documentation)
    {
        rl_rule_meta(thisAgent, p);
    }

    return p;
}

/* =================================================================
                          Init Parser

   This routine initializes the parser.  At present, all it does is
   set up the help screens for the LHS and RHS grammars.
================================================================= */

/*
  This is not longer used.

void init_parser (void) {
  add_help (thisAgent, "lhs-grammar", help_on_lhs_grammar);
  add_help (thisAgent, "rhs-grammar", help_on_rhs_grammar);
}
*/

