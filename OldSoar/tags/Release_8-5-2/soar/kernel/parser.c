/*************************************************************************
 *
 *  file:  parser.c
 *
 * =======================================================================
 *                  Production (SP) Parser for Soar 6
 *
 * There are two top-level routines here:  init_parser(), which
 * should be called at startup time, and parse_production(), which
 * reads an SP (starting from the production name), builds a production,
 * adds it to the rete, and returns a pointer to the new production
 * (or NIL if any error occurred).
 * =======================================================================
 *
 * Copyright 1995-2004 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 */

#include "soarkernel.h"
#include <ctype.h>

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

unsigned long placeholder_counter[26];

void reset_placeholder_variable_generator(void)
{
    int i;
    for (i = 0; i < 26; i++)
        placeholder_counter[i] = 1;
}

/* -----------------------------------------------------------------
               Make Placeholder (Dummy) Equality Test
   
   Creates and returns a test for equality with a newly generated
   placeholder variable.
----------------------------------------------------------------- */

#define NAMEBUF_SIZE 30
test make_placeholder_test(char first_letter)
{
    char namebuf[NAMEBUF_SIZE];
    Symbol *new_var;

    if (isalpha(first_letter)) {
        if (isupper(first_letter))
            first_letter = (char) tolower(first_letter);
    } else {
        first_letter = 'v';
    }
    /* --- create variable with "#" in its name:  this couldn't possibly be a
       variable in the user's code, since the lexer doesn't handle "#" --- */
    snprintf(namebuf, NAMEBUF_SIZE, "<#%c*%lu>", first_letter, placeholder_counter[first_letter - 'a']++);
    namebuf[NAMEBUF_SIZE - 1] = 0;      /* snprintf doesn't set last char to null if output is truncated */
    new_var = make_variable(namebuf);
    /* --- indicate that there is no corresponding "real" variable yet --- */
    new_var->var.current_binding_value = NIL;
    /* --- return an equality test for that variable --- */
    return make_equality_test_without_adding_reference(new_var);
}

/* -----------------------------------------------------------------
            Substituting Real Variables for Placeholders
   
   When done parsing the production, we go back and substitute "real"
   variables for all the placeholders.  This is done by walking all the
   LHS conditions and destructively modifying any tests involving
   placeholders.  The placeholder-->real mapping is maintained on each
   placeholder symbol: placeholder->var.current_binding_value is the
   corresponding "real" variable, or NIL if no such "real" variable has
   been created yet.

   To use this, first call reset_variable_generator (lhs, rhs) with the
   lhs and rhs of the production just parsed; then call
   substitute_for_placeholders_in_condition_list (lhs).
----------------------------------------------------------------- */

void substitute_for_placeholders_in_symbol(Symbol ** sym)
{
    char prefix[3];
    Symbol *var;
    bool just_created;

    /* --- if not a variable, do nothing --- */
    if ((*sym)->common.symbol_type != VARIABLE_SYMBOL_TYPE)
        return;
    /* --- if not a placeholder variable, do nothing --- */
    if (*((*sym)->var.name + 1) != '#')
        return;

    just_created = FALSE;

    if (!(*sym)->var.current_binding_value) {
        prefix[0] = *((*sym)->var.name + 2);
        prefix[1] = '*';
        prefix[2] = 0;
        (*sym)->var.current_binding_value = generate_new_variable(prefix);
        just_created = TRUE;
    }

    var = (*sym)->var.current_binding_value;
    symbol_remove_ref(*sym);
    *sym = var;
    if (!just_created)
        symbol_add_ref(var);
}

void substitute_for_placeholders_in_test(test * t)
{
    cons *c;
    complex_test *ct;

    if (test_is_blank_test(*t))
        return;
    if (test_is_blank_or_equality_test(*t)) {
        substitute_for_placeholders_in_symbol((Symbol **) t);
        /* Warning: this relies on the representation of tests */
        return;
    }

    ct = complex_test_from_test(*t);

    switch (ct->type) {
    case GOAL_ID_TEST:
    case IMPASSE_ID_TEST:
    case DISJUNCTION_TEST:
        return;
    case CONJUNCTIVE_TEST:
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            substitute_for_placeholders_in_test((test *) (&(c->first)));
        return;
    default:                   /* relational tests other than equality */
        substitute_for_placeholders_in_symbol(&(ct->data.referent));
        return;
    }
}

void substitute_for_placeholders_in_condition_list(condition * cond)
{
    for (; cond != NIL; cond = cond->next) {
        switch (cond->type) {
        case POSITIVE_CONDITION:
        case NEGATIVE_CONDITION:
            substitute_for_placeholders_in_test(&(cond->data.tests.id_test));
            substitute_for_placeholders_in_test(&(cond->data.tests.attr_test));
            substitute_for_placeholders_in_test(&(cond->data.tests.value_test));
            break;
        case CONJUNCTIVE_NEGATION_CONDITION:
            substitute_for_placeholders_in_condition_list(cond->data.ncc.top);
            break;
        }
    }
}

/* begin KJC 10/19/98 */
void substitute_for_placeholders_in_action_list(action * a)
{
    for (; a != NIL; a = a->next) {
        if (a->type == MAKE_ACTION) {
            if (rhs_value_is_symbol(a->id)) {
                substitute_for_placeholders_in_symbol((Symbol **) & (a->id));
            }
            if (rhs_value_is_symbol(a->attr))
                substitute_for_placeholders_in_symbol((Symbol **) & (a->attr));
            if (rhs_value_is_symbol(a->value))
                substitute_for_placeholders_in_symbol((Symbol **) & (a->value));
        }
    }
}

/* end KJC 10/19/98 */

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
   <single_test> ::= variable | <constant>
   <constant> ::= sym_constant | int_constant | float_constant

================================================================= */

char *help_on_lhs_grammar[] = {
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
    "   <single_test> ::= variable | <constant>",
    "   <constant> ::= sym_constant | int_constant | float_constant",
    "",
    "See also:  rhs-grammar, sp",
    0
};

/* =================================================================

                  Make symbol for current lexeme

================================================================= */

Symbol *make_symbol_for_current_lexeme(void)
{
    switch (current_agent(lexeme).type) {
    case SYM_CONSTANT_LEXEME:
        return make_sym_constant(current_agent(lexeme).string);
    case VARIABLE_LEXEME:
        return make_variable(current_agent(lexeme).string);
    case INT_CONSTANT_LEXEME:
        return make_int_constant(current_agent(lexeme).int_val);
    case FLOAT_CONSTANT_LEXEME:
        return make_float_constant(current_agent(lexeme).float_val);

    case IDENTIFIER_LEXEME:
        {
            char msg[MESSAGE_SIZE];
            strncpy(msg, "parser.c: Internal error:  ID found in make_symbol_for_current_lexeme\n", MESSAGE_SIZE);
            msg[MESSAGE_SIZE - 1] = 0;  /* strncpy doesn't set last char to null if output is truncated */
            abort_with_fatal_error(msg);
        }
    default:
        {
            char msg[MESSAGE_SIZE];
            snprintf(msg, MESSAGE_SIZE,
                     "parser.c: Internal error:  bad lexeme type in make_symbol_for_current_lexeme\n, current_agent(lexeme).string=%s\n",
                     current_agent(lexeme).string);
            msg[MESSAGE_SIZE - 1] = 0;  /* snprintf doesn't set last char to null if output is truncated */
            abort_with_fatal_error(msg);
        }
    }
    return NIL;                 /* unreachable, but without it, gcc -Wall warns here */
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
   <single_test> ::= variable | <constant>
   <constant> ::= sym_constant | int_constant | float_constant
----------------------------------------------------------------- */

test parse_relational_test(void)
{
    byte test_type;
    bool use_equality_test;
    test t;
    Symbol *referent;
    complex_test *ct;

    use_equality_test = FALSE;
    test_type = NOT_EQUAL_TEST; /* unnecessary, but gcc -Wall warns without it */

    /* --- read optional relation symbol --- */
    switch (current_agent(lexeme).type) {
    case EQUAL_LEXEME:
        use_equality_test = TRUE;
        get_lexeme();
        break;

    case NOT_EQUAL_LEXEME:
        test_type = NOT_EQUAL_TEST;
        get_lexeme();
        break;

    case LESS_LEXEME:
        test_type = LESS_TEST;
        get_lexeme();
        break;

    case GREATER_LEXEME:
        test_type = GREATER_TEST;
        get_lexeme();
        break;

    case LESS_EQUAL_LEXEME:
        test_type = LESS_OR_EQUAL_TEST;
        get_lexeme();
        break;

    case GREATER_EQUAL_LEXEME:
        test_type = GREATER_OR_EQUAL_TEST;
        get_lexeme();
        break;

    case LESS_EQUAL_GREATER_LEXEME:
        test_type = SAME_TYPE_TEST;
        get_lexeme();
        break;

    default:
        use_equality_test = TRUE;
        break;
    }

    /* --- read variable or constant --- */
    switch (current_agent(lexeme).type) {
    case SYM_CONSTANT_LEXEME:
    case INT_CONSTANT_LEXEME:
    case FLOAT_CONSTANT_LEXEME:
    case VARIABLE_LEXEME:
        referent = make_symbol_for_current_lexeme();
        get_lexeme();
        if (use_equality_test) {
            t = make_equality_test_without_adding_reference(referent);
        } else {
            allocate_with_pool(&current_agent(complex_test_pool), &ct);
            ct->type = test_type;
            ct->data.referent = referent;
            t = make_test_from_complex_test(ct);
        }
        return t;

    default:
        print("Expected variable or constant for test\n");
        print_location_of_most_recent_lexeme();
        return NIL;
    }
}

/* -----------------------------------------------------------------
                      Parse Disjunction Test
                      
   <disjunction_test> ::= << <constant>* >>
   <constant> ::= sym_constant | int_constant | float_constant
----------------------------------------------------------------- */

test parse_disjunction_test(void)
{
    complex_test *ct;
    test t;

    if (current_agent(lexeme).type != LESS_LESS_LEXEME) {
        print("Expected << to begin disjunction test\n");
        print_location_of_most_recent_lexeme();
        return NIL;
    }
    get_lexeme();

    allocate_with_pool(&current_agent(complex_test_pool), &ct);
    ct->type = DISJUNCTION_TEST;
    ct->data.disjunction_list = NIL;
    t = make_test_from_complex_test(ct);

    while (current_agent(lexeme).type != GREATER_GREATER_LEXEME) {
        switch (current_agent(lexeme).type) {
        case SYM_CONSTANT_LEXEME:
        case INT_CONSTANT_LEXEME:
        case FLOAT_CONSTANT_LEXEME:
            push(make_symbol_for_current_lexeme(), ct->data.disjunction_list);
            get_lexeme();
            break;
        default:
            print("Expected constant or >> while reading disjunction test\n");
            print_location_of_most_recent_lexeme();
            deallocate_test(t);
            return NIL;
        }
    }
    get_lexeme();               /* consume the >> */
    ct->data.disjunction_list = destructively_reverse_list(ct->data.disjunction_list);
    return t;
}

/* -----------------------------------------------------------------
                        Parse Simple Test
                      
   <simple_test> ::= <disjunction_test> | <relational_test>
----------------------------------------------------------------- */

test parse_simple_test(void)
{
    if (current_agent(lexeme).type == LESS_LESS_LEXEME)
        return parse_disjunction_test();
    return parse_relational_test();
}

/* -----------------------------------------------------------------
                            Parse Test
                      
    <test> ::= <conjunctive_test> | <simple_test>
    <conjunctive_test> ::= { <simple_test>+ }
----------------------------------------------------------------- */

test parse_test(void)
{
    complex_test *ct;
    test t, temp;

    if (current_agent(lexeme).type != L_BRACE_LEXEME)
        return parse_simple_test();
    /* --- parse and return conjunctive test --- */
    get_lexeme();
    t = make_blank_test();
    do {
        temp = parse_simple_test();
        if (!temp) {
            deallocate_test(t);
            return NIL;
        }
        add_new_test_to_test(&t, temp);
    } while (current_agent(lexeme).type != R_BRACE_LEXEME);
    get_lexeme();               /* consume the "}" */

    if (test_is_complex_test(t)) {
        ct = complex_test_from_test(t);
        if (ct->type == CONJUNCTIVE_TEST)
            ct->data.conjunct_list = destructively_reverse_list(ct->data.conjunct_list);
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

void fill_in_id_tests(condition * conds, test t)
{
    condition *positive_c, *c;
    test equality_test_from_t;

    /* --- see if there's at least one positive condition --- */
    for (positive_c = conds; positive_c != NIL; positive_c = positive_c->next)
        if ((positive_c->type == POSITIVE_CONDITION) && (positive_c->data.tests.id_test == NIL))
            break;

    if (positive_c) {           /* --- there is at least one positive condition --- */
        /* --- add just the equality test to most of the conditions --- */
        equality_test_from_t = copy_of_equality_test_found_in_test(t);
        for (c = conds; c != NIL; c = c->next) {
            if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
                fill_in_id_tests(c->data.ncc.top, equality_test_from_t);
            else if (c->data.tests.id_test == NIL)
                c->data.tests.id_test = copy_test(equality_test_from_t);
        }
        deallocate_test(equality_test_from_t);
        /* --- add the whole test to one positive condition --- */
        deallocate_test(positive_c->data.tests.id_test);
        positive_c->data.tests.id_test = copy_test(t);
        return;
    }

    /* --- all conditions are negative --- */
    for (c = conds; c != NIL; c = c->next) {
        if (c->type == CONJUNCTIVE_NEGATION_CONDITION) {
            fill_in_id_tests(c->data.ncc.top, t);
        } else {
            if (c->data.tests.id_test == NIL)
                c->data.tests.id_test = copy_test(t);
        }
    }
}

void fill_in_attr_tests(condition * conds, test t)
{
    condition *positive_c, *c;
    test equality_test_from_t;

    /* --- see if there's at least one positive condition --- */
    for (positive_c = conds; positive_c != NIL; positive_c = positive_c->next)
        if ((positive_c->type == POSITIVE_CONDITION) && (positive_c->data.tests.attr_test == NIL))
            break;

    if (positive_c) {           /* --- there is at least one positive condition --- */
        /* --- add just the equality test to most of the conditions --- */
        equality_test_from_t = copy_of_equality_test_found_in_test(t);
        for (c = conds; c != NIL; c = c->next) {
            if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
                fill_in_attr_tests(c->data.ncc.top, equality_test_from_t);
            else if (c->data.tests.attr_test == NIL)
                c->data.tests.attr_test = copy_test(equality_test_from_t);
        }
        deallocate_test(equality_test_from_t);
        /* --- add the whole test to one positive condition --- */
        deallocate_test(positive_c->data.tests.attr_test);
        positive_c->data.tests.attr_test = copy_test(t);
        return;
    }

    /* --- all conditions are negative --- */
    for (c = conds; c != NIL; c = c->next) {
        if (c->type == CONJUNCTIVE_NEGATION_CONDITION) {
            fill_in_attr_tests(c->data.ncc.top, t);
        } else {
            if (c->data.tests.attr_test == NIL)
                c->data.tests.attr_test = copy_test(t);
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

condition *negate_condition_list(condition * conds)
{
    condition *temp, *last;

    if (conds->next == NIL) {
        /* --- only one condition to negate, so toggle the type --- */
        switch (conds->type) {
        case POSITIVE_CONDITION:
            conds->type = NEGATIVE_CONDITION;
            return conds;
        case NEGATIVE_CONDITION:
            conds->type = POSITIVE_CONDITION;
            return conds;
        case CONJUNCTIVE_NEGATION_CONDITION:
            temp = conds->data.ncc.top;
            free_with_pool(&current_agent(condition_pool), conds);
            return temp;
        }
    }
    /* --- more than one condition; so build a conjunctive negation --- */
    allocate_with_pool(&current_agent(condition_pool), &temp);
    temp->type = CONJUNCTIVE_NEGATION_CONDITION;
    temp->next = NIL;
    temp->prev = NIL;
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

condition *parse_conds_for_one_id(char first_letter_if_no_id_given, test * dest_id_test);

condition *parse_value_test_star(char first_letter)
{
    condition *c, *last_c, *first_c, *new_conds;
    test value_test;
    bool acceptable;

    if ((current_agent(lexeme).type == MINUS_LEXEME) ||
        (current_agent(lexeme).type == UP_ARROW_LEXEME) || (current_agent(lexeme).type == R_PAREN_LEXEME)) {
        /* --- value omitted, so create dummy value test --- */
        allocate_with_pool(&current_agent(condition_pool), &c);
        c->type = POSITIVE_CONDITION;
        c->next = c->prev = NIL;
        c->data.tests.id_test = NIL;
        c->data.tests.attr_test = NIL;
        c->data.tests.value_test = make_placeholder_test(first_letter);
        c->test_for_acceptable_preference = FALSE;
        return c;
    }

    first_c = NIL;
    last_c = NIL;
    do {
        if (current_agent(lexeme).type == L_PAREN_LEXEME) {
            /* --- read <conds_for_one_id>, take the id_test from it --- */
            new_conds = parse_conds_for_one_id(first_letter, &value_test);
            if (!new_conds) {
                deallocate_condition_list(first_c);
                return NIL;
            }
        } else {
            /* --- read <value_test> --- */
            new_conds = NIL;
            value_test = parse_test();
            if (!value_test) {
                deallocate_condition_list(first_c);
                return NIL;
            }
            if (!test_includes_equality_test_for_symbol(value_test, NIL)) {
                add_new_test_to_test(&value_test, make_placeholder_test(first_letter));
            }
        }
        /* --- check for acceptable preference indicator --- */
        acceptable = FALSE;
        if (current_agent(lexeme).type == PLUS_LEXEME) {
            acceptable = TRUE;
            get_lexeme();
        }
        /* --- build condition using the new value test --- */
        allocate_with_pool(&current_agent(condition_pool), &c);
        insert_at_head_of_dll(new_conds, c, next, prev);
        c->type = POSITIVE_CONDITION;
        c->data.tests.id_test = NIL;
        c->data.tests.attr_test = NIL;
        c->data.tests.value_test = value_test;
        c->test_for_acceptable_preference = acceptable;
        /* --- add new conditions to the end of the list --- */
        if (last_c)
            last_c->next = new_conds;
        else
            first_c = new_conds;
        new_conds->prev = last_c;
        for (last_c = new_conds; last_c->next != NIL; last_c = last_c->next);
    } while ((current_agent(lexeme).type != MINUS_LEXEME) &&
             (current_agent(lexeme).type != UP_ARROW_LEXEME) && (current_agent(lexeme).type != R_PAREN_LEXEME));
    return first_c;
}

/* -----------------------------------------------------------------
                      Parse Attr Value Tests
                      
   <attr_value_tests> ::= [-] ^ <attr_test> [.<attr_test>]* <value_test>*
   <attr_test> ::= <test>

   (This routine parses <attr_value_tests>, given as input the id_test 
   already read.)
----------------------------------------------------------------- */

condition *parse_attr_value_tests(void)
{
    test id_test_to_use, attr_test;
    bool negate_it;
    condition *first_c, *last_c, *c, *new_conds;

    /* --- read optional minus sign --- */
    negate_it = FALSE;
    if (current_agent(lexeme).type == MINUS_LEXEME) {
        negate_it = TRUE;
        get_lexeme();
    }

    /* --- read up arrow --- */
    if (current_agent(lexeme).type != UP_ARROW_LEXEME) {
        print("Expected ^ followed by attribute\n");
        print_location_of_most_recent_lexeme();
        return NIL;
    }
    get_lexeme();

    first_c = NIL;
    last_c = NIL;

    /* --- read first <attr_test> --- */
    attr_test = parse_test();
    if (!attr_test)
        return NIL;
    if (!test_includes_equality_test_for_symbol(attr_test, NIL)) {
        add_new_test_to_test(&attr_test, make_placeholder_test('a'));
    }

    /* --- read optional attribute path --- */
    id_test_to_use = NIL;
    while (current_agent(lexeme).type == PERIOD_LEXEME) {
        get_lexeme();           /* consume the "." */
        /* --- setup for next attribute in path:  make a dummy variable,
           create a new condition in the path --- */
        allocate_with_pool(&current_agent(condition_pool), &c);
        c->type = POSITIVE_CONDITION;
        if (last_c)
            last_c->next = c;
        else
            first_c = c;
        c->next = NIL;
        c->prev = last_c;
        last_c = c;
        if (id_test_to_use)
            c->data.tests.id_test = copy_test(id_test_to_use);
        else
            c->data.tests.id_test = NIL;
        c->data.tests.attr_test = attr_test;
        id_test_to_use = make_placeholder_test(first_letter_from_test(attr_test));
        c->data.tests.value_test = id_test_to_use;
        c->test_for_acceptable_preference = FALSE;
        /* --- update id and attr tests for the next path element --- */
        attr_test = parse_test();
        if (!attr_test) {
            deallocate_condition_list(first_c);
            return NIL;
        }
/* AGR 544 begin */
        if (!test_includes_equality_test_for_symbol(attr_test, NIL)) {
            add_new_test_to_test(&attr_test, make_placeholder_test('a'));
        }
/* AGR 544 end */
    }                           /* end of while (current_agent(lexeme).type==PERIOD_LEXEME) */

    /* --- finally, do the <value_test>* part --- */
    new_conds = parse_value_test_star(first_letter_from_test(attr_test));
    if (!new_conds) {
        deallocate_condition_list(first_c);
        deallocate_test(attr_test);
        return NIL;
    }
    fill_in_attr_tests(new_conds, attr_test);
    if (id_test_to_use)
        fill_in_id_tests(new_conds, id_test_to_use);
    deallocate_test(attr_test);
    if (last_c)
        last_c->next = new_conds;
    else
        first_c = new_conds;
    new_conds->prev = last_c;
    /* should update last_c here, but it's not needed anymore */

    /* --- negate everything if necessary --- */
    if (negate_it)
        first_c = negate_condition_list(first_c);

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

test parse_head_of_conds_for_one_id(char first_letter_if_no_id_given)
{
    test id_test, id_goal_impasse_test, check_for_symconstant;
    complex_test *ct;
    Symbol *sym;

    if (current_agent(lexeme).type != L_PAREN_LEXEME) {
        print("Expected ( to begin condition element\n");
        print_location_of_most_recent_lexeme();
        return NIL;
    }
    get_lexeme();

    /* --- look for goal/impasse indicator --- */
    if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME) {
        if (!strcmp(current_agent(lexeme).string, "state")) {
            allocate_with_pool(&current_agent(complex_test_pool), &ct);
            ct->type = GOAL_ID_TEST;
            id_goal_impasse_test = make_test_from_complex_test(ct);
            get_lexeme();
            first_letter_if_no_id_given = 's';
        } else if (!strcmp(current_agent(lexeme).string, "impasse")) {
            allocate_with_pool(&current_agent(complex_test_pool), &ct);
            ct->type = IMPASSE_ID_TEST;
            id_goal_impasse_test = make_test_from_complex_test(ct);
            get_lexeme();
            first_letter_if_no_id_given = 'i';
        } else {
            id_goal_impasse_test = make_blank_test();
        }
    } else {
        id_goal_impasse_test = make_blank_test();
    }

    /* --- read optional id test; create dummy one if none given --- */
    if ((current_agent(lexeme).type != MINUS_LEXEME) &&
        (current_agent(lexeme).type != UP_ARROW_LEXEME) && (current_agent(lexeme).type != R_PAREN_LEXEME)) {
        id_test = parse_test();
        if (!id_test) {
            deallocate_test(id_goal_impasse_test);
            return NIL;
        }
        if (!test_includes_equality_test_for_symbol(id_test, NIL)) {
            add_new_test_to_test(&id_test, make_placeholder_test(first_letter_if_no_id_given));
        } else {
            check_for_symconstant = copy_of_equality_test_found_in_test(id_test);
            sym = referent_of_equality_test(check_for_symconstant);
            deallocate_test(check_for_symconstant);     /* RBD added 3/28/95 */
            if (sym->common.symbol_type != VARIABLE_SYMBOL_TYPE) {
                print_with_symbols("Warning: Constant %y in id field test.\n", sym);
                print("         This will never match.\n");
                print_location_of_most_recent_lexeme();
                deallocate_test(id_test);       /* AGR 527c */
                return NIL;     /* AGR 527c */
            }
        }
    } else {
        id_test = make_placeholder_test(first_letter_if_no_id_given);
    }

    /* --- add the goal/impasse test to the id test --- */
    add_new_test_to_test(&id_test, id_goal_impasse_test);

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

condition *parse_tail_of_conds_for_one_id(void)
{
    condition *first_c, *last_c, *new_conds;
    condition *c;

    /* --- if no <attr_value_tests> are given, create a dummy one --- */
    if (current_agent(lexeme).type == R_PAREN_LEXEME) {
        get_lexeme();           /* consume the right parenthesis */
        allocate_with_pool(&current_agent(condition_pool), &c);
        c->type = POSITIVE_CONDITION;
        c->next = NIL;
        c->prev = NIL;
        c->data.tests.id_test = NIL;
        c->data.tests.attr_test = make_placeholder_test('a');
        c->data.tests.value_test = make_placeholder_test('v');
        c->test_for_acceptable_preference = FALSE;
        return c;
    }

    /* --- read <attr_value_tests>* --- */
    first_c = NIL;
    last_c = NIL;
    while (current_agent(lexeme).type != R_PAREN_LEXEME) {
        new_conds = parse_attr_value_tests();
        if (!new_conds) {
            deallocate_condition_list(first_c);
            return NIL;
        }
        if (last_c)
            last_c->next = new_conds;
        else
            first_c = new_conds;
        new_conds->prev = last_c;
        for (last_c = new_conds; last_c->next != NIL; last_c = last_c->next);
    }

    /* --- reached the end of the condition --- */
    get_lexeme();               /* consume the right parenthesis */

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

condition *parse_conds_for_one_id(char first_letter_if_no_id_given, test * dest_id_test)
{
    condition *conds;
    test id_test, equality_test_from_id_test;

    /* --- parse the head --- */
    id_test = parse_head_of_conds_for_one_id(first_letter_if_no_id_given);
    if (!id_test)
        return NIL;

    /* --- parse the tail --- */
    conds = parse_tail_of_conds_for_one_id();
    if (!conds) {
        deallocate_test(id_test);
        return NIL;
    }

    /* --- fill in the id test in all the conditions just read --- */
    if (dest_id_test) {
        *dest_id_test = id_test;
        equality_test_from_id_test = copy_of_equality_test_found_in_test(id_test);
        fill_in_id_tests(conds, equality_test_from_id_test);
        deallocate_test(equality_test_from_id_test);
    } else {
        fill_in_id_tests(conds, id_test);
        deallocate_test(id_test);
    }

    return conds;
}

/* -----------------------------------------------------------------
                            Parse Cond
                      
   <cond> ::= <positive_cond> | - <positive_cond>
   <positive_cond> ::= <conds_for_one_id> | { <cond>+ }
----------------------------------------------------------------- */

condition *parse_cond_plus(void);

condition *parse_cond(void)
{
    condition *c;
    bool negate_it;

    /* --- look for leading "-" sign --- */
    negate_it = FALSE;
    if (current_agent(lexeme).type == MINUS_LEXEME) {
        negate_it = TRUE;
        get_lexeme();
    }

    /* --- parse <positive_cond> --- */
    if (current_agent(lexeme).type == L_BRACE_LEXEME) {
        /* --- read conjunctive condition --- */
        get_lexeme();
        c = parse_cond_plus();
        if (!c)
            return NIL;
        if (current_agent(lexeme).type != R_BRACE_LEXEME) {
            print("Expected } to end conjunctive condition\n");
            print_location_of_most_recent_lexeme();
            deallocate_condition_list(c);
            return NIL;
        }
        get_lexeme();           /* consume the R_BRACE */
    } else {
        /* --- read conds for one id --- */
        c = parse_conds_for_one_id('s', NULL);
        if (!c)
            return NIL;
    }

    /* --- if necessary, handle the negation --- */
    if (negate_it)
        c = negate_condition_list(c);

    return c;
}

/* -----------------------------------------------------------------
                            Parse Cond Plus
                      
   (Parses <cond>+ and builds a condition list.)
----------------------------------------------------------------- */

condition *parse_cond_plus(void)
{
    condition *first_c, *last_c, *new_conds;

    first_c = NIL;
    last_c = NIL;
    do {
        /* --- get individual <cond> --- */
        new_conds = parse_cond();
        if (!new_conds) {
            deallocate_condition_list(first_c);
            return NIL;
        }
        if (last_c)
            last_c->next = new_conds;
        else
            first_c = new_conds;
        new_conds->prev = last_c;
        for (last_c = new_conds; last_c->next != NIL; last_c = last_c->next);
    } while ((current_agent(lexeme).type == MINUS_LEXEME) ||
             (current_agent(lexeme).type == L_PAREN_LEXEME) || (current_agent(lexeme).type == L_BRACE_LEXEME));
    return first_c;
}

/* -----------------------------------------------------------------
                            Parse LHS
                      
   (Parses <lhs> and builds a condition list.)

   <lhs> ::= <cond>+
----------------------------------------------------------------- */

condition *parse_lhs(void)
{
    condition *c;

    c = parse_cond_plus();
    if (!c)
        return NIL;
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
   <rhs_action> ::= ( variable <attr_value_make>+ ) | <function_call>
   <function_call> ::= ( <function_name> <rhs_value>* )
   <function_name> ::= sym_constant | + | -
     (WARNING: might need others besides +, - here if the lexer changes)
   <rhs_value> ::= <constant> | <function_call> | variable
   <constant> ::= sym_constant | int_constant | float_constant
   <attr_value_make> ::= ^ <rhs_value> <value_make>+
   <value_make> ::= <rhs_value> <preferences>

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

char *help_on_rhs_grammar[] = {
    "Grammar for right hand sides of productions:",
    "",
    "   <rhs> ::= <rhs_action>*",
    "   <rhs_action> ::= ( variable <attr_value_make>+ ) | <function_call>",
    "   <function_call> ::= ( <function_name> <rhs_value>* )",
    "   <function_name> ::= sym_constant | + | -",
    "   <rhs_value> ::= <constant> | <function_call> | variable",
    "   <constant> ::= sym_constant | int_constant | float_constant",
    "   <attr_value_make> ::= ^ <rhs_value> <value_make>+",
    "   <value_make> ::= <rhs_value> <preferences>",
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
   <function_name> ::= sym_constant | + | -
     (Warning: might need others besides +, - here if the lexer changes)
----------------------------------------------------------------- */

rhs_value parse_rhs_value(void);

rhs_value parse_function_call_after_lparen(bool is_stand_alone_action)
{
    rhs_function *rf;
    Symbol *fun_name;
    list *fl;
    cons *c, *prev_c;
    rhs_value arg_rv;
    int num_args;

    /* --- read function name, find the rhs_function structure --- */
    if (current_agent(lexeme).type == PLUS_LEXEME)
        fun_name = find_sym_constant("+");
    else if (current_agent(lexeme).type == MINUS_LEXEME)
        fun_name = find_sym_constant("-");
    else
        fun_name = find_sym_constant(current_agent(lexeme).string);
    if (!fun_name) {
        print("No RHS function named %s\n", current_agent(lexeme).string);
        print_location_of_most_recent_lexeme();
        return NIL;
    }
    rf = lookup_rhs_function(fun_name);
    if (!rf) {
        print("No RHS function named %s\n", current_agent(lexeme).string);
        print_location_of_most_recent_lexeme();
        return NIL;
    }

    /* --- make sure stand-alone/rhs_value is appropriate --- */
    if (is_stand_alone_action && (!rf->can_be_stand_alone_action)) {
        print("Function %s cannot be used as a stand-alone action\n", current_agent(lexeme).string);
        print_location_of_most_recent_lexeme();
        return NIL;
    }
    if ((!is_stand_alone_action) && (!rf->can_be_rhs_value)) {
        print("Function %s can only be used as a stand-alone action\n", current_agent(lexeme).string);
        print_location_of_most_recent_lexeme();
        return NIL;
    }

    /* --- build list of rhs_function and arguments --- */
    allocate_cons(&fl);
    fl->first = rf;
    prev_c = fl;
    get_lexeme();               /* consume function name, advance to argument list */
    num_args = 0;
    while (current_agent(lexeme).type != R_PAREN_LEXEME) {
        arg_rv = parse_rhs_value();
        if (!arg_rv) {
            prev_c->rest = NIL;
            deallocate_rhs_value(funcall_list_to_rhs_value(fl));
            return NIL;
        }
        num_args++;
        allocate_cons(&c);
        c->first = arg_rv;
        prev_c->rest = c;
        prev_c = c;
    }
    prev_c->rest = NIL;

    /* --- check number of arguments --- */
    if ((rf->num_args_expected != -1) && (rf->num_args_expected != num_args)) {
        print("Wrong number of arguments to function %s (expected %d)\n", rf->name->sc.name, rf->num_args_expected);
        print_location_of_most_recent_lexeme();
        deallocate_rhs_value(funcall_list_to_rhs_value(fl));
        return NIL;
    }

    get_lexeme();               /* consume the right parenthesis */
    return funcall_list_to_rhs_value(fl);
}

/* -----------------------------------------------------------------
                          Parse RHS Value

   Parses an <rhs_value>.  Returns an rhs_value, or NIL if any error
   occurred.

   <rhs_value> ::= <constant> | <function_call> | variable
   <constant> ::= sym_constant | int_constant | float_constant
----------------------------------------------------------------- */

rhs_value parse_rhs_value(void)
{
    rhs_value rv;

    if (current_agent(lexeme).type == L_PAREN_LEXEME) {
        get_lexeme();
        return parse_function_call_after_lparen(FALSE);
    }
    if ((current_agent(lexeme).type == SYM_CONSTANT_LEXEME) ||
        (current_agent(lexeme).type == INT_CONSTANT_LEXEME) ||
        (current_agent(lexeme).type == FLOAT_CONSTANT_LEXEME) || (current_agent(lexeme).type == VARIABLE_LEXEME)) {
        rv = symbol_to_rhs_value(make_symbol_for_current_lexeme());
        get_lexeme();
        return rv;
    }
    print("Illegal value for RHS value\n");
    print_location_of_most_recent_lexeme();
    return FALSE;
}

/* -----------------------------------------------------------------
                         Is Preference Lexeme

   Given a token type, returns TRUE if the token is a preference
   lexeme.

----------------------------------------------------------------- */

bool is_preference_lexeme(enum lexer_token_type test_lexeme)
{
    switch (test_lexeme) {

    case PLUS_LEXEME:
        return TRUE;
    case MINUS_LEXEME:
        return TRUE;
    case EXCLAMATION_POINT_LEXEME:
        return TRUE;
    case TILDE_LEXEME:
        return TRUE;
    case AT_LEXEME:
        return TRUE;
    case GREATER_LEXEME:
        return TRUE;
    case EQUAL_LEXEME:
        return TRUE;
    case LESS_LEXEME:
        return TRUE;
    case AMPERSAND_LEXEME:
        return TRUE;
    default:
        return FALSE;
    }
}

/* -----------------------------------------------------------------
               Parse Preference Specifier Without Referent

   Parses a <preference-specifier>.  Returns the appropriate 
   xxx_PREFERENCE_TYPE (see soarkernel.h).

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

byte parse_preference_specifier_without_referent(void)
{
    switch (current_agent(lexeme).type) {

    case PLUS_LEXEME:
        get_lexeme();
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
        return ACCEPTABLE_PREFERENCE_TYPE;

    case MINUS_LEXEME:
        get_lexeme();
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
        return REJECT_PREFERENCE_TYPE;

    case EXCLAMATION_POINT_LEXEME:
        get_lexeme();
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
        return REQUIRE_PREFERENCE_TYPE;

    case TILDE_LEXEME:
        get_lexeme();
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
        return PROHIBIT_PREFERENCE_TYPE;

    case AT_LEXEME:
        get_lexeme();
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
        return RECONSIDER_PREFERENCE_TYPE;

/****************************************************************************
 * [Soar-Bugs #55] <forced-unary-preference> ::= <binary-preference> 
 *                                             {<any-preference> | , | ) | ^} 
 *
 *   Forced unary preferences can now occur when a binary preference is
 *   followed by a ",", ")", "^" or any preference specifier
 ****************************************************************************/

    case GREATER_LEXEME:
        get_lexeme();
        if ((current_agent(lexeme).type != COMMA_LEXEME) &&
            (current_agent(lexeme).type != R_PAREN_LEXEME) &&
            (current_agent(lexeme).type != UP_ARROW_LEXEME) && (!is_preference_lexeme(current_agent(lexeme).type)))
            return BETTER_PREFERENCE_TYPE;
        /* --- forced unary preference --- */
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
        return BEST_PREFERENCE_TYPE;

    case EQUAL_LEXEME:
        get_lexeme();
        if ((current_agent(lexeme).type != COMMA_LEXEME) &&
            (current_agent(lexeme).type != R_PAREN_LEXEME) &&
            (current_agent(lexeme).type != UP_ARROW_LEXEME) && (!is_preference_lexeme(current_agent(lexeme).type)))
            return BINARY_INDIFFERENT_PREFERENCE_TYPE;
        /* --- forced unary preference --- */
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
        return UNARY_INDIFFERENT_PREFERENCE_TYPE;

    case LESS_LEXEME:
        get_lexeme();
        if ((current_agent(lexeme).type != COMMA_LEXEME) &&
            (current_agent(lexeme).type != R_PAREN_LEXEME) &&
            (current_agent(lexeme).type != UP_ARROW_LEXEME) && (!is_preference_lexeme(current_agent(lexeme).type)))
            return WORSE_PREFERENCE_TYPE;
        /* --- forced unary preference --- */
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
        return WORST_PREFERENCE_TYPE;

    case AMPERSAND_LEXEME:
        get_lexeme();
        if ((current_agent(lexeme).type != COMMA_LEXEME) &&
            (current_agent(lexeme).type != R_PAREN_LEXEME) &&
            (current_agent(lexeme).type != UP_ARROW_LEXEME) && (!is_preference_lexeme(current_agent(lexeme).type)))
            return BINARY_PARALLEL_PREFERENCE_TYPE;
        /* --- forced unary preference --- */
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
        return UNARY_PARALLEL_PREFERENCE_TYPE;

    default:
        /* --- if no preference given, make it an acceptable preference --- */
        return ACCEPTABLE_PREFERENCE_TYPE;
    }                           /* end of switch statement */
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

action *parse_preferences(Symbol * id, rhs_value attr, rhs_value value)
{
    action *a;
    action *prev_a;
    rhs_value referent;
    byte preference_type;
    bool saw_plus_sign;

    /* --- Note: this routine is set up so if there's not preference type
       indicator at all, we return a single acceptable preference make --- */

    prev_a = NIL;

    saw_plus_sign = (bool) (current_agent(lexeme).type == PLUS_LEXEME);
    preference_type = parse_preference_specifier_without_referent();
    if ((preference_type == ACCEPTABLE_PREFERENCE_TYPE) && (!saw_plus_sign)) {
        /* If the routine gave us a + pref without seeing a + sign, then it's
           just giving us the default acceptable preference.  Look for optional
           comma. */
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
    }

    for (;;) {
        /* --- read referent --- */
        if (preference_is_binary(preference_type)) {
            referent = parse_rhs_value();
            if (!referent) {
                deallocate_action_list(prev_a);
                return NIL;
            }
            if (current_agent(lexeme).type == COMMA_LEXEME)
                get_lexeme();
        } else {
            referent = NIL;     /* unnecessary, but gcc -Wall warns without it */
        }

        /* --- create the appropriate action --- */
        allocate_with_pool(&current_agent(action_pool), &a);
        a->next = prev_a;
        prev_a = a;
        a->type = MAKE_ACTION;
        a->preference_type = preference_type;
        a->id = symbol_to_rhs_value(id);
        symbol_add_ref(id);
        a->attr = copy_rhs_value(attr);
        a->value = copy_rhs_value(value);
        if (preference_is_binary(preference_type))
            a->referent = referent;

        /* --- look for another preference type specifier --- */
        saw_plus_sign = (bool) (current_agent(lexeme).type == PLUS_LEXEME);
        preference_type = parse_preference_specifier_without_referent();

        /* --- exit loop when done reading preferences --- */
        if ((preference_type == ACCEPTABLE_PREFERENCE_TYPE) && (!saw_plus_sign))
            /* If the routine gave us a + pref without seeing a + sign, then it's
               just giving us the default acceptable preference, it didn't see any
               more preferences specified. */
            return prev_a;
    }
}

/* KJC begin:  10.09.98 */
/* modified 3.99 to take out parallels and only create acceptables */
/* -----------------------------------------------------------------
              Parse Preferences for Soar8 Non-Operators

   Given the id, attribute, and value already read, this routine
   parses zero or more <preference-specifier>'s.  If preferences
   other than reject and acceptable are specified, it prints
   a warning message that they are being ignored.  It builds an
   action list for creating an ACCEPTABLE preference.  If binary 
   preferences are encountered, a warning message is printed and 
   the production is ignored (returns NIL).  It returns NIL if any 
   other error occurred.  This works in conjunction with the code
   that supports attribute_preferences_mode == 2.  Anywhere that
   attribute_preferences_mode == 2 is tested, the code now tests
   for operand2_mode == TRUE.

   <value_make> ::= <rhs_value> <preferences>
   <preferences> ::= [,] | <preference_specifier>+   
   <preference-specifier> ::= <naturally-unary-preference> [,]
                            | <forced-unary-preference>
                            | <binary-preference> <rhs_value> [,]
----------------------------------------------------------------- */

action *parse_preferences_soar8_non_operator(Symbol * id, rhs_value attr, rhs_value value)
{
    action *a;
    action *prev_a;
    rhs_value referent;
    byte preference_type;
    bool saw_plus_sign;

    /* --- Note: this routine is set up so if there's not preference type
       indicator at all, we return an acceptable preference make
       and a parallel preference make.  For non-operators, allow
       only REJECT_PREFERENCE_TYPE, (and UNARY_PARALLEL and ACCEPTABLE).
       If any other preference type indicator is found, a warning or
       error msg (error only on binary prefs) is printed. --- */

    prev_a = NIL;

    saw_plus_sign = (bool) (current_agent(lexeme).type == PLUS_LEXEME);
    preference_type = parse_preference_specifier_without_referent();
    if ((preference_type == ACCEPTABLE_PREFERENCE_TYPE) && (!saw_plus_sign)) {
        /* If the routine gave us a + pref without seeing a + sign, then it's
           just giving us the default acceptable preference.  Look for optional
           comma. */
        if (current_agent(lexeme).type == COMMA_LEXEME)
            get_lexeme();
    }

    for (;;) {
        /* step through the pref list, print warning messages when necessary. */

        /* --- read referent --- */
        if (preference_is_binary(preference_type)) {
            print("\nERROR: in Soar8, binary preference illegal for non-operator.");
            print_with_symbols("\n id = %y\t attr = %y\t value = %y\n",
                               id, rhs_value_to_symbol(attr), rhs_value_to_symbol(value));
            deallocate_action_list(prev_a);
            return NIL;
        } else {
            referent = NIL;     /* unnecessary, but gcc -Wall warns without it */
        }

        if ((preference_type != ACCEPTABLE_PREFERENCE_TYPE) && (preference_type != REJECT_PREFERENCE_TYPE)) {
            print
                ("\nWARNING: in Soar8, the only allowable non-operator preference \nis REJECT - .\nIgnoring specified preferences.\n");
            print_with_symbols("id = %y\t attr = %y\t value = %y\n", id, rhs_value_to_symbol(attr),
                               rhs_value_to_symbol(value));
            print_location_of_most_recent_lexeme();
        }

        if (preference_type == REJECT_PREFERENCE_TYPE) {
            /* --- create the appropriate action --- */
            allocate_with_pool(&current_agent(action_pool), &a);
            a->next = prev_a;
            prev_a = a;
            a->type = MAKE_ACTION;
            a->preference_type = preference_type;
            a->id = symbol_to_rhs_value(id);
            symbol_add_ref(id);
            a->attr = copy_rhs_value(attr);
            a->value = copy_rhs_value(value);
        }

        /* --- look for another preference type specifier --- */
        saw_plus_sign = (bool) (current_agent(lexeme).type == PLUS_LEXEME);
        preference_type = parse_preference_specifier_without_referent();

        /* --- exit loop when done reading preferences --- */
        if ((preference_type == ACCEPTABLE_PREFERENCE_TYPE) && (!saw_plus_sign)) {
            /* If the routine gave us a + pref without seeing a + sign, then it's
               just giving us the default acceptable preference, it didn't see any
               more preferences specified. */

            /* for soar8, if this wasn't a REJECT preference, then
               create acceptable preference makes.  */
            if (prev_a == NIL) {

                allocate_with_pool(&current_agent(action_pool), &a);
                a->next = prev_a;
                prev_a = a;
                a->type = MAKE_ACTION;
                a->preference_type = ACCEPTABLE_PREFERENCE_TYPE;
                a->id = symbol_to_rhs_value(id);
                symbol_add_ref(id);
                a->attr = copy_rhs_value(attr);
                a->value = copy_rhs_value(value);
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

action *parse_attr_value_make(Symbol * id)
{
    rhs_value attr, value;
    action *all_actions, *new_actions, *last;
    Symbol *old_id, *new_var;
    char namebuf[NAMEBUF_SIZE], first_letter;

    if (current_agent(lexeme).type != UP_ARROW_LEXEME) {
        print("Expected ^ in RHS make action\n");
        print_location_of_most_recent_lexeme();
        return NIL;
    }
    old_id = id;

    get_lexeme();               /* consume up-arrow, advance to attribute */
    attr = parse_rhs_value();
    if (!attr)
        return NIL;
    all_actions = NIL;

    /*  allow dot notation "." in RHS attribute path  10/15/98 KJC */
    while (current_agent(lexeme).type == PERIOD_LEXEME) {
        get_lexeme();           /* consume the "."  */
        /* set up for next attribute in path: make dummy variable,
           and create new action in the path */

        /* --- create variable with "#" in its name:  this couldn't possibly be a
           variable in the user's code, since the lexer doesn't handle "#" --- */
        /* KJC used same format so could steal code... */
        first_letter = first_letter_from_rhs_value(attr);
        snprintf(namebuf, NAMEBUF_SIZE, "<#%c*%lu>", first_letter, placeholder_counter[first_letter - 'a']++);
        namebuf[NAMEBUF_SIZE - 1] = 0;  /* snprintf doesn't set last char to null if output is truncated */
        new_var = make_variable(namebuf);
        /* --- indicate that there is no corresponding "real" variable yet --- */
        new_var->var.current_binding_value = NIL;

        /* parse_preferences actually creates the action.  eventhough
           there aren't really any preferences to read, we need the default
           acceptable and parallel prefs created for all attributes in path */
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) &&
#else
        if (
#endif
               (strcmp(rhs_value_to_string(attr, NULL, 0), "operator") != 0)) {
            new_actions = parse_preferences_soar8_non_operator(id, attr, symbol_to_rhs_value(new_var));
        } else {
            new_actions = parse_preferences(id, attr, symbol_to_rhs_value(new_var));
        }
        for (last = new_actions; last->next != NIL; last = last->next);
        last->next = all_actions;
        all_actions = new_actions;

        /* if there was a "." then there must be another attribute
           set id for next action and get the next attribute */
        id = new_var;
        attr = parse_rhs_value();
        if (!attr)
            return NIL;

    }                           /* end of while (current_agent(lexeme).type == PERIOD_LEXEME */
    /* end KJC 10/15/98 */

    do {
        value = parse_rhs_value();
        if (!value) {
            deallocate_rhs_value(attr);
            deallocate_action_list(all_actions);
            return NIL;
        }
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) &&
#else
        if (
#endif
               (strcmp(rhs_value_to_string(attr, NULL, 0), "operator") != 0)) {
            new_actions = parse_preferences_soar8_non_operator(id, attr, value);
        } else {
            new_actions = parse_preferences(id, attr, value);
        }
        deallocate_rhs_value(value);
        if (!new_actions) {
            deallocate_rhs_value(attr);
            return NIL;
        }
        for (last = new_actions; last->next != NIL; last = last->next);
        last->next = all_actions;
        all_actions = new_actions;
    } while ((current_agent(lexeme).type != R_PAREN_LEXEME) && (current_agent(lexeme).type != UP_ARROW_LEXEME));

    deallocate_rhs_value(attr);
    return all_actions;
}

/* -----------------------------------------------------------------
                        Parse RHS Action

   Parses an <rhs_action> and returns an action list.  If any error
   occurrs, NIL is returned.

   <rhs_action> ::= ( variable <attr_value_make>+ ) | <function_call>
----------------------------------------------------------------- */

action *parse_rhs_action(void)
{
    action *all_actions, *new_actions, *last;
    Symbol *var;
    rhs_value funcall_value;

    if (current_agent(lexeme).type != L_PAREN_LEXEME) {
        print("Expected ( to begin RHS action\n");
        print_location_of_most_recent_lexeme();
        return NIL;
    }
    get_lexeme();
    if (current_agent(lexeme).type != VARIABLE_LEXEME) {
        /* --- the action is a function call --- */
        funcall_value = parse_function_call_after_lparen(TRUE);
        if (!funcall_value)
            return NIL;
        allocate_with_pool(&current_agent(action_pool), &all_actions);
        all_actions->type = FUNCALL_ACTION;
        all_actions->next = NIL;
        all_actions->value = funcall_value;
        return all_actions;
    }
    /* --- the action is a regular make action --- */
    var = make_variable(current_agent(lexeme).string);
    get_lexeme();
    all_actions = NIL;
    while (current_agent(lexeme).type != R_PAREN_LEXEME) {
        new_actions = parse_attr_value_make(var);
        if (new_actions) {
            for (last = new_actions; last->next != NIL; last = last->next);
            last->next = all_actions;
            all_actions = new_actions;
        } else {
            symbol_remove_ref(var);
            deallocate_action_list(all_actions);
            return NIL;
        }
    }
    get_lexeme();               /* consume the right parenthesis */
    symbol_remove_ref(var);
    return all_actions;
}

/* -----------------------------------------------------------------
                            Parse RHS

   Parses the <rhs> and sets *dest_rhs to the resulting action list.
   Returns TRUE if successful, FALSE if any error occurred.

   <rhs> ::= <rhs_action>*
----------------------------------------------------------------- */

bool parse_rhs(action ** dest_rhs)
{
    action *all_actions, *new_actions, *last;

    all_actions = NIL;
    while (current_agent(lexeme).type != R_PAREN_LEXEME) {
        new_actions = parse_rhs_action();
        if (new_actions) {
            for (last = new_actions; last->next != NIL; last = last->next);
            last->next = all_actions;
            all_actions = new_actions;
        } else {
            deallocate_action_list(all_actions);
            return FALSE;
        }
    }
    *dest_rhs = all_actions;
    return TRUE;
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

action *destructively_reverse_action_list(action * a)
{
    action *prev, *current, *next;

    prev = NIL;
    current = a;
    while (current) {
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

production *parse_production(void)
{
    Symbol *name;
    char *documentation;
    condition *lhs, *lhs_top, *lhs_bottom;
    action *rhs;
    production *p;
    byte declared_support;
    byte prod_type;
    byte rete_addition_result;
    bool rhs_okay;
    bool interrupt_on_match;

    reset_placeholder_variable_generator();

    /* --- read production name --- */
    if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME) {
        print("Expected symbol for production name\n");
        print_location_of_most_recent_lexeme();
        return NIL;
    }
    name = make_sym_constant(current_agent(lexeme).string);
    get_lexeme();

    /* --- if there's already a prod with this name, excise it --- */
    if (name->sc.production) {
        excise_production(name->sc.production, (bool) (TRUE && current_agent(sysparams)[TRACE_LOADING_SYSPARAM]));
    }

    /* --- read optional documentation string --- */
    if (current_agent(lexeme).type == QUOTED_STRING_LEXEME) {
        documentation = make_memory_block_for_string(current_agent(lexeme).string);
        get_lexeme();
    } else {
        documentation = NIL;
    }

    /* --- read optional flags --- */
    declared_support = UNDECLARED_SUPPORT;
    prod_type = USER_PRODUCTION_TYPE;
    interrupt_on_match = FALSE;
    for (;;) {
        if (current_agent(lexeme).type != SYM_CONSTANT_LEXEME)
            break;
        if (!strcmp(current_agent(lexeme).string, ":o-support")) {
            declared_support = DECLARED_O_SUPPORT;
            get_lexeme();
            continue;
        }
        if (!strcmp(current_agent(lexeme).string, ":i-support")) {
            declared_support = DECLARED_I_SUPPORT;
            get_lexeme();
            continue;
        }
        if (!strcmp(current_agent(lexeme).string, ":chunk")) {
            prod_type = CHUNK_PRODUCTION_TYPE;
            get_lexeme();
            continue;
        }
        if (!strcmp(current_agent(lexeme).string, ":default")) {
            prod_type = DEFAULT_PRODUCTION_TYPE;
            get_lexeme();
            continue;
        }
        if (!strcmp(current_agent(lexeme).string, ":interrupt")) {
#ifdef MATCHTIME_INTERRUPT
            interrupt_on_match = TRUE;
#else
            print("WARNING :interrupt is not supported with the current build options...");
#endif
            get_lexeme();
            continue;
        }
        break;
    }                           /* end of while (TRUE) */

    /* --- read the LHS --- */
    lhs = parse_lhs();
    if (!lhs) {
        print_with_symbols("(Ignoring production %y)\n\n", name);
        if (documentation)
            free_memory_block_for_string(documentation);
        symbol_remove_ref(name);
/*    if (! reading_from_top_level()) respond_to_load_errors ();  AGR 527c */
        return NIL;
    }

    /* --- read the "-->" --- */
    if (current_agent(lexeme).type != RIGHT_ARROW_LEXEME) {
        print("Expected --> in production\n");
        print_location_of_most_recent_lexeme();
        print_with_symbols("(Ignoring production %y)\n\n", name);
        if (documentation)
            free_memory_block_for_string(documentation);
        symbol_remove_ref(name);
        deallocate_condition_list(lhs);
/*    if (! reading_from_top_level()) respond_to_load_errors ();  AGR 527c */
        return NIL;
    }
    get_lexeme();

    /* --- read the RHS --- */
    rhs_okay = parse_rhs(&rhs);
    if (!rhs_okay) {
        print_with_symbols("(Ignoring production %y)\n\n", name);
        if (documentation)
            free_memory_block_for_string(documentation);
        symbol_remove_ref(name);
        deallocate_condition_list(lhs);
/*    if (! reading_from_top_level()) respond_to_load_errors ();  AGR 527c */
        return NIL;
    }
    rhs = destructively_reverse_action_list(rhs);

    /* --- finally, make sure there's a closing right parenthesis (but
       don't consume it) --- */
    if (current_agent(lexeme).type != R_PAREN_LEXEME) {
        print("Expected ) to end production\n");
        print_location_of_most_recent_lexeme();
        if (documentation)
            free_memory_block_for_string(documentation);
        print_with_symbols("(Ignoring production %y)\n\n", name);
        symbol_remove_ref(name);
        deallocate_condition_list(lhs);
        deallocate_action_list(rhs);
/*    if (! reading_from_top_level()) respond_to_load_errors ();  AGR 527c */
        return NIL;
    }

    /* --- replace placeholder variables with real variables --- */
    reset_variable_generator(lhs, rhs);
    substitute_for_placeholders_in_condition_list(lhs);
    substitute_for_placeholders_in_action_list(rhs);

    /* --- everything parsed okay, so make the production structure --- */
    lhs_top = lhs;
    for (lhs_bottom = lhs; lhs_bottom->next != NIL; lhs_bottom = lhs_bottom->next);
    p = make_production(prod_type, name, &lhs_top, &lhs_bottom, &rhs, TRUE);

    if (!p) {
        if (documentation)
            free_memory_block_for_string(documentation);
        print_with_symbols("(Ignoring production %y)\n\n", name);
        symbol_remove_ref(name);
        deallocate_condition_list(lhs_top);
        deallocate_action_list(rhs);
/*    if (! reading_from_top_level()) respond_to_load_errors ();  AGR 527c */
        return NIL;
    }

    p->documentation = documentation;
    p->declared_support = declared_support;
    p->interrupt = interrupt_on_match;
    rete_addition_result = add_production_to_rete(p, lhs_top, NIL, TRUE);
    deallocate_condition_list(lhs_top);

    if (rete_addition_result == DUPLICATE_PRODUCTION) {
        excise_production(p, FALSE);
        p = NIL;
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
  add_help ("lhs-grammar", help_on_lhs_grammar);
  add_help ("rhs-grammar", help_on_rhs_grammar);
}
*/
