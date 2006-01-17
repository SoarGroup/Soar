/*
 * =======================================================================
 *  File:  soarCommandUtils.c
 *
 * This file includes the utility routines for supporting the
 * Soar Command set, which is found in soarCommands.c.  
 *
 * =======================================================================
 *
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
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "soarapi.h"
#include "soarapiUtils.h"

#define MAXPATHLEN 1024

/* DJP 4/3/96 */
#define dealloc_and_return(x,y) { deallocate_test(x) ; return (y) ; }

char *preference_name[] = {
    "acceptable",
    "require",
    "reject",
    "prohibit",
    "reconsider",
    "unary indifferent",
    "unary parallel",
    "best",
    "worst",
    "binary indifferent",
    "binary parallel",
    "better",
    "worse"
};

int getInt(const char *string, int *i)
{

    long l;
    char *c;

    if (*string == '\0')
        return SOAR_ERROR;

    l = strtol(string, &c, 10);

    if (*c == '\0') {
        *i = (int) l;
        return SOAR_OK;
    }

    return SOAR_ERROR;
}

Symbol *space_to_remove_from_cfps;

bool cfps_removal_test_function(cons * c)
{
    return (bool) (c->first == space_to_remove_from_cfps);
}

void do_print_for_production_name(const char *prod_name, bool internal, bool print_filename, bool full_prod)
{
    Symbol *sym;

    sym = find_sym_constant(current_agent(lexeme).string);
    if (sym && sym->sc.production) {
        /* kjh CUSP(B11) begin *//* also Soar-Bugs #161 */
        if (print_filename) {
            if (full_prod)
                print_string("# sourcefile : ");
            print("%s\n", sym->sc.production->filename);
        }
        /* KJC added so get at least some output for any named productions */
        if ((full_prod) || (!print_filename)) {
            print_production(sym->sc.production, internal);
            print("\n");
        }                       /* end CUSP B11 kjh */
    } else {
        print("No production named %s\n", prod_name);
        print_location_of_most_recent_lexeme();
    }
}

void do_print_for_wme(wme * w, int depth, bool internal)
{
    tc_number tc;

    if (internal && (depth == 0)) {
        print_wme(w);
        /* print ("\n"); */
    } else {
        tc = get_new_tc_number();
        print_augs_of_id(w->id, depth, internal, 0, tc);
    }
}

void execute_go_selection(agent * the_agent,
                          long go_number,
                          enum go_type_enum go_type, Symbol * go_slot_attr, goal_stack_level go_slot_level)
{
    switch (go_type) {
    case GO_PHASE:
        run_for_n_phases(go_number);
        break;
    case GO_ELABORATION:
        run_for_n_elaboration_cycles(go_number);
        break;
    case GO_DECISION:
        run_for_n_decision_cycles(go_number);
        break;
    case GO_STATE:
        run_for_n_selections_of_slot(go_number, the_agent->state_symbol);
        break;
    case GO_OPERATOR:
        run_for_n_selections_of_slot(go_number, the_agent->operator_symbol);
        break;
    case GO_SLOT:
        run_for_n_selections_of_slot_at_level(go_number, go_slot_attr, go_slot_level);
        break;
    case GO_OUTPUT:
        run_for_n_modifications_of_output(go_number);
        break;
    }
}

/* ===================================================================
                       Get Context Var Info

   This utility routine is used by interface routines that take context
   variable arguments (e.g., <s> for the current state).  It looks at
   the current lexeme (which must be of type VARIABLE_LEXEME), and
   checks to see if it's a context variable.  Returns:

    if lexeme is not a context variable, dest_attr_of_slot=NIL; else
      dest_attr_of_slot = {goal_symbol, problem_space_symbol, etc.}.
      dest_goal = goal identifier for the given slot (NIL if no such goal)
      dest_current_value = currently installed value (goal id itself for goals,
                           NIL if no installed value)
=================================================================== */

/* This routine was moved to command_utils.c for Tcl.  However, it cannot
 * reside outside the Soar kernel since it references lexeme.  This should
 * be rewritten to take a string argument and added to the Soar kernel
 * interface file (whatever that will be).
 */

/*
 *----------------------------------------------------------------------
 *
 * print_current_learn_settings --
 *
 *	This procedure prints the current learn settings.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Prints the current settings.
 *
 *----------------------------------------------------------------------
 */

void print_current_learn_settings(void)
{
    print("Current learn settings:\n");
/* AGR MVL1 begin */
    if ((!current_agent(sysparams)[LEARNING_ONLY_SYSPARAM]) && (!current_agent(sysparams)[LEARNING_EXCEPT_SYSPARAM]))
        print("   %s\n", current_agent(sysparams)[LEARNING_ON_SYSPARAM] ? "-on" : "-off");
    else
        print("   %s\n", current_agent(sysparams)[LEARNING_ONLY_SYSPARAM] ? "-only" : "-except");

/* AGR MVL1 end */
    print("   %s\n", current_agent(sysparams)[LEARNING_ALL_GOALS_SYSPARAM] ? "-all-levels" : "-bottom-up");

}

/*
 *----------------------------------------------------------------------
 *
 * print_multi_attribute_symbols --
 *
 *	This procedure prints a list of the symbols that are
 *      declared to have multi_attributes.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Prints the list of symbols and the value of multi_attribute.
 *
 *----------------------------------------------------------------------
 */
void print_multi_attribute_symbols(void)
{
    multi_attribute *m = current_agent(multi_attributes);

    print("\n");
    if (!m) {
        print("No multi-attributes declared for this agent.\n");
    } else {
        print("Value\tSymbol\n");
        while (m) {
            print("%ld\t%s\n", m->value, symbol_to_string(m->symbol, TRUE, NIL, 0));
            m = m->next;
        }
    }
    print("\n");
}

/* kjh (CUSP-B7):  Add to soarCommandUtils.c after procedure
   "read_id_or_context_var_from_string" */

/*
 *----------------------------------------------------------------------
 *
 * read_pref_detail_from_string --
 *
 *	This procedure parses a string to determine if it is a
 *      lexeme for the detail level indicator for the 'preferences'
 *      command.  If so, it sets the_lexeme and wme_trace_type accordingly
 *      and returns SOAR_OK; otherwise, it leaves those parameters untouched
 *      and returns SOAR_ERROR.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int read_pref_detail_from_string(const char *the_lexeme, bool * print_productions, wme_trace_type * wtt)
{
    if (!strncmp(the_lexeme, "-none", 3)
        || !strcmp(the_lexeme, "0")) {
        *print_productions = FALSE;
        *wtt = NONE_WME_TRACE;
    } else if (!strncmp(the_lexeme, "-names", 3)
               || !strcmp(the_lexeme, "1")) {
        *print_productions = TRUE;
        *wtt = NONE_WME_TRACE;
    } else if (!strncmp(the_lexeme, "-timetags", 2)
               || !strcmp(the_lexeme, "2")) {
        *print_productions = TRUE;
        *wtt = TIMETAG_WME_TRACE;
    } else if (!strncmp(the_lexeme, "-wmes", 2)
               || !strcmp(the_lexeme, "3")) {
        *print_productions = TRUE;
        *wtt = FULL_WME_TRACE;
    } else {
        return SOAR_ERROR;
    }
    return SOAR_OK;
}

/* ===================================================================
                Read Pattern And Get Matching Wmes

   This routine reads a pattern and returns a list of all wmes that
   match it.  At entry, the current lexeme should be the "("; at exit,
   the current lexeme will be the ")".  If any error occurs or if no
   wmes match the pattern, the function returns NIL.

   pattern ::= ( {identifier | '*'} ^ { attribute | '*'} { value | '*' } [+])

=================================================================== */

/* This should be added to the Soar kernel.  It must be re-written to
 * take a string to parse.
 */

int read_pattern_component(Symbol ** dest_sym)
{
    /* --- Read and consume one pattern element.  Return 0 if error, 1 if "*",
       otherwise return 2 and set dest_sym to find_symbol() result. --- */
    if (strcmp(current_agent(lexeme).string, "*") == 0)
        return 1;
    switch (current_agent(lexeme).type) {
    case SYM_CONSTANT_LEXEME:
        *dest_sym = find_sym_constant(current_agent(lexeme).string);
        return 2;
    case INT_CONSTANT_LEXEME:
        *dest_sym = find_int_constant(current_agent(lexeme).int_val);
        return 2;
    case FLOAT_CONSTANT_LEXEME:
        *dest_sym = find_float_constant(current_agent(lexeme).float_val);
        return 2;
    case IDENTIFIER_LEXEME:
        *dest_sym = find_identifier(current_agent(lexeme).id_letter, current_agent(lexeme).id_number);
        return 2;
    case VARIABLE_LEXEME:
        *dest_sym = read_identifier_or_context_variable();
        if (*dest_sym)
            return 2;
        return 0;
    default:
        print("Expected identifier or constant in wme pattern\n");
        print_location_of_most_recent_lexeme();
        return 0;
    }
}

list *read_pattern_and_get_matching_wmes(void)
{
    int parentheses_level;
    list *wmes;
    wme *w;
    Symbol *id, *attr, *value;
    int id_result, attr_result, value_result;
    bool acceptable;

    if (current_agent(lexeme).type != L_PAREN_LEXEME) {
        print("Expected '(' to begin wme pattern not string '%s' or char '%c'\n",
              current_agent(lexeme).string, current_agent(current_char));
        print_location_of_most_recent_lexeme();
        return NIL;
    }
    parentheses_level = current_lexer_parentheses_level();

    get_lexeme();
    id_result = read_pattern_component(&id);
    if (!id_result) {
        skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }
    get_lexeme();
    if (current_agent(lexeme).type != UP_ARROW_LEXEME) {
        print("Expected ^ in wme pattern\n");
        print_location_of_most_recent_lexeme();
        skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }
    get_lexeme();
    attr_result = read_pattern_component(&attr);
    if (!attr_result) {
        skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }
    get_lexeme();
    value_result = read_pattern_component(&value);
    if (!value_result) {
        skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }
    get_lexeme();
    if (current_agent(lexeme).type == PLUS_LEXEME) {
        acceptable = TRUE;
        get_lexeme();
    } else {
        acceptable = FALSE;
    }
    if (current_agent(lexeme).type != R_PAREN_LEXEME) {
        print("Expected ')' to end wme pattern\n");
        print_location_of_most_recent_lexeme();
        skip_ahead_to_balanced_parentheses(parentheses_level - 1);
        return NIL;
    }
    {
        int i = 0;
        wmes = NIL;
        for (w = current_agent(all_wmes_in_rete); w != NIL; w = w->rete_next) {
            if ((id_result == 1) || (id == w->id))
                if ((attr_result == 1) || (attr == w->attr))
                    if ((value_result == 1) || (value == w->value))
                        if (acceptable == w->acceptable) {
                            push(w, wmes);
                            i++;
                        }

        }
        /* printf( "--- FOUND %d matching wmes\n", i ); */
    }
    return wmes;
}

void cb_appendToSoarResultResult(agent * the_agent, soar_callback_data data, soar_call_data call_data)
{

    the_agent = the_agent;

    appendSoarResultResult(((soarResult *) data), (char *) call_data);

}

/*
 *----------------------------------------------------------------------
 *
 * soar_agent_already_defined --
 *
 *	This procedure determines if a Soar agent name is
 *      already in use.
 *
 * Results:
 *	Returns a boolean indication of whether the agent
 *      name is already defined or not.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

bool soar_agent_already_defined(char *name)
{
    cons *c;                    /* Cons cell index            */
    agent *the_agent;           /* Agent index                */

    for (c = all_soar_agents; c != NIL; c = c->rest) {
        the_agent = (agent *) c->first;
        if (!strcmp(the_agent->name, name)) {
            return TRUE;
        }
    }

    return FALSE;
}

typedef struct binding_structure {
    Symbol *from, *to;
} Binding;

Symbol *get_binding(Symbol * f, list * bindings)
{
    cons *c;

    for (c = bindings; c != NIL; c = c->rest) {
        if (((Binding *) c->first)->from == f)
            return ((Binding *) c->first)->to;
    }
    return NIL;
}

void reset_old_binding_point(list ** bindings, list ** current_binding_point)
{
    cons *c, *c_next;

    c = *bindings;
    while (c != *current_binding_point) {
        c_next = c->rest;
        free_memory(c->first, MISCELLANEOUS_MEM_USAGE);
        free_cons(c);
        c = c_next;
    }

    bindings = current_binding_point;
}

void free_binding_list(list * bindings)
{
    cons *c;

    for (c = bindings; c != NIL; c = c->rest)
        free_memory(c->first, MISCELLANEOUS_MEM_USAGE);
    free_list(bindings);
}

void print_binding_list(list * bindings)
{
    cons *c;

    for (c = bindings; c != NIL; c = c->rest)
        print_with_symbols("   (%y -> %y)\n", ((Binding *) c->first)->from, ((Binding *) c->first)->to);
}

bool symbols_are_equal_with_bindings(Symbol * s1, Symbol * s2, list ** bindings)
{
    Binding *b;
    Symbol *bvar;

    /* SBH/MVP 7-5-94 */
    if ((s1 == s2) && (s1->common.symbol_type != VARIABLE_SYMBOL_TYPE))
        return TRUE;

    /* "*" matches everything. */
    if ((s1->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) && (!strcmp(s1->sc.name, "*")))
        return TRUE;
    if ((s2->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) && (!strcmp(s2->sc.name, "*")))
        return TRUE;

    if ((s1->common.symbol_type != VARIABLE_SYMBOL_TYPE) || (s2->common.symbol_type != VARIABLE_SYMBOL_TYPE))
        return FALSE;
    /* Both are variables */
    bvar = get_binding(s1, *bindings);
    if (bvar == NIL) {
        b = (Binding *) allocate_memory(sizeof(Binding), MISCELLANEOUS_MEM_USAGE);
        b->from = s1;
        b->to = s2;
        push(b, *bindings);
        return TRUE;
    } else if (bvar == s2) {
        return TRUE;
    } else
        return FALSE;
}

/* ----------------------------------------------------------------
   Returns TRUE iff the two tests are identical given a list of bindings.
   Augments the bindings list if necessary.
---------------------------------------------------------------- */

/* DJP 4/3/96 -- changed t2 to test2 in declaration */
bool tests_are_equal_with_bindings(test t1, test test2, list ** bindings)
{
    cons *c1, *c2;
    complex_test *ct1, *ct2;
    bool goal_test, impasse_test;

    /* DJP 4/3/96 -- The problem here is that sometimes test2 was being copied      */
    /*               and sometimes it wasn't.  If it was copied, the copy was never */
    /*               deallocated.  There's a few choices about how to fix this.  I  */
    /*               decided to just create a copy always and then always           */
    /*               deallocate it before returning.  Added a macro to do that.     */

    test t2;

    /* t1 is from the pattern given to "pf"; t2 is from a production's condition list. */
    if (test_is_blank_test(t1)) {
        return (bool) (test_is_blank_test(test2));
    }

    /* If the pattern doesn't include "(state", but the test from the
       production does, strip it out of the production's. */
    if ((!test_includes_goal_or_impasse_id_test(t1, TRUE, FALSE)) &&
        test_includes_goal_or_impasse_id_test(test2, TRUE, FALSE)) {
        goal_test = FALSE;
        impasse_test = FALSE;
        t2 = copy_test_removing_goal_impasse_tests(test2, &goal_test, &impasse_test);
    } else {
        t2 = copy_test(test2);  /* DJP 4/3/96 -- Always make t2 into a copy */
    }

    if (test_is_blank_or_equality_test(t1)) {
        if (!(test_is_blank_or_equality_test(t2) && !(test_is_blank_test(t2)))) {
            dealloc_and_return(t2, FALSE);
        } else {
            if (symbols_are_equal_with_bindings(referent_of_equality_test(t1), referent_of_equality_test(t2), bindings)) {
                dealloc_and_return(t2, TRUE);
            } else {
                dealloc_and_return(t2, FALSE);
            }
        }
    }

    ct1 = complex_test_from_test(t1);
    ct2 = complex_test_from_test(t2);

    if (ct1->type != ct2->type) {
        dealloc_and_return(t2, FALSE);
    }

    switch (ct1->type) {
    case GOAL_ID_TEST:
        dealloc_and_return(t2, TRUE);
    case IMPASSE_ID_TEST:
        dealloc_and_return(t2, TRUE);

    case DISJUNCTION_TEST:
        for (c1 = ct1->data.disjunction_list, c2 = ct2->data.disjunction_list; ((c1 != NIL) && (c2 != NIL));
             c1 = c1->rest, c2 = c2->rest) {
            if (c1->first != c2->first) {
                dealloc_and_return(t2, FALSE);
            }
        }
        if (c1 == c2) {
            dealloc_and_return(t2, TRUE);       /* make sure they both hit end-of-list */
        }
        dealloc_and_return(t2, FALSE);

    case CONJUNCTIVE_TEST:
        for (c1 = ct1->data.conjunct_list, c2 = ct2->data.conjunct_list; ((c1 != NIL) && (c2 != NIL));
             c1 = c1->rest, c2 = c2->rest) {
            if (!tests_are_equal_with_bindings(c1->first, c2->first, bindings)) {
                dealloc_and_return(t2, FALSE);
            }
        }
        if (c1 == c2) {
            dealloc_and_return(t2, TRUE);       /* make sure they both hit end-of-list */
        }
        dealloc_and_return(t2, FALSE);

    default:                   /* relational tests other than equality */
        if (symbols_are_equal_with_bindings(ct1->data.referent, ct2->data.referent, bindings)) {
            dealloc_and_return(t2, TRUE);
        }
        dealloc_and_return(t2, FALSE);
    }
}

bool conditions_are_equal_with_bindings(condition * c1, condition * c2, list ** bindings)
{
    if (c1->type != c2->type)
        return FALSE;
    switch (c1->type) {
    case POSITIVE_CONDITION:
    case NEGATIVE_CONDITION:
        if (!tests_are_equal_with_bindings(c1->data.tests.id_test, c2->data.tests.id_test, bindings))
            return FALSE;
        if (!tests_are_equal_with_bindings(c1->data.tests.attr_test, c2->data.tests.attr_test, bindings))

            return FALSE;
        if (!tests_are_equal_with_bindings(c1->data.tests.value_test, c2->data.tests.value_test, bindings))
            return FALSE;
        if (c1->test_for_acceptable_preference != c2->test_for_acceptable_preference)
            return FALSE;
        return TRUE;

    case CONJUNCTIVE_NEGATION_CONDITION:
        for (c1 = c1->data.ncc.top, c2 = c2->data.ncc.top; ((c1 != NIL) && (c2 != NIL)); c1 = c1->next, c2 = c2->next)
            if (!conditions_are_equal_with_bindings(c1, c2, bindings))
                return FALSE;
        if (c1 == c2)
            return TRUE;        /* make sure they both hit end-of-list */
        return FALSE;
    }
    return FALSE;               /* unreachable, but without it, gcc -Wall warns here */
}

/* Routine for LHS. */
void read_pattern_and_get_matching_productions(list ** current_pf_list, bool show_bindings,
                                               bool just_chunks, bool no_chunks)
{
    condition *c, *clist, *top, *bottom, *pc;
    int i;
    production *prod;
    list *bindings, *current_binding_point;
    bool match, match_this_c;

    bindings = NIL;
    current_binding_point = NIL;

/*  print("Parsing as a lhs...\n"); */
    clist = (condition *) parse_lhs();
    if (!clist) {
        print("Error: not a valid condition list.\n");
        current_pf_list = NIL;
        return;
    }
/*
  print("Valid condition list:\n");
  print_condition_list(clist,0,FALSE);
  print("\nMatches:\n");
*/

    /* For the moment match against productions of all types (user,chunk,default, justification).     Later on the type should be a parameter.
     */

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
        if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) || (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
            for (prod = current_agent(all_productions_of_type)[i]; prod != NIL; prod = prod->next) {

                /* Now the complicated part. */
                /* This is basically a full graph-match.  Like the rete.  Yikes! */
                /* Actually it's worse, because there are so many different KINDS of
                   conditions (negated, >/<, acc prefs, ...) */
                /* Is there some way I could *USE* the rete for this?  -- for lhs
                   positive conditions, possibly.  Put some fake stuff into WM
                   (i.e. with make-wme's), see what matches all of them, and then
                   yank out the fake stuff.  But that won't work for RHS or for
                   negateds.       */

                /* Also note that we need bindings for every production.  Very expensive
                   (but don't necc. need to save them -- maybe can just print them as we go). */

                match = TRUE;
                p_node_to_conditions_and_nots(prod->p_node, NIL, NIL, &top, &bottom, NIL, NIL);

                free_binding_list(bindings);
                bindings = NIL;

                for (c = clist; c != NIL; c = c->next) {
                    match_this_c = FALSE;
                    current_binding_point = bindings;

                    for (pc = top; pc != NIL; pc = pc->next) {
                        if (conditions_are_equal_with_bindings(c, pc, &bindings)) {
                            match_this_c = TRUE;
                            break;
                        } else {
                            /* Remove new, incorrect bindings. */
                            reset_old_binding_point(&bindings, &current_binding_point);
                            bindings = current_binding_point;
                        }
                    }
                    if (!match_this_c) {
                        match = FALSE;
                        break;
                    }
                }
                deallocate_condition_list(top); /* DJP 4/3/96 -- Never dealloced */
                if (match) {
                    push(prod, (*current_pf_list));
                    if (show_bindings) {
                        print_with_symbols("%y, with bindings:\n", prod->name);
                        print_binding_list(bindings);
                    } else
                        print_with_symbols("%y\n", prod->name);
                }
            }
    if (bindings)
        free_binding_list(bindings);    /* DJP 4/3/96 -- To catch the last production */
}

bool funcalls_match(list * fc1, list * fc2)
{
    /* I have no idea how to do this. */

    /* voigtjr: neither do I, so I'm just going to shut up the compiler 
       warnings with these next two lines */
    fc1 = fc1;
    fc2 = fc2;

    return FALSE;
}

bool actions_are_equal_with_bindings(action * a1, action * a2, list ** bindings)
{
    if (a1->type == FUNCALL_ACTION) {
        if ((a2->type == FUNCALL_ACTION)) {
            if (funcalls_match(rhs_value_to_funcall_list(a1->value), rhs_value_to_funcall_list(a2->value))) {
                return TRUE;
            } else
                return FALSE;
        } else
            return FALSE;
    }
    if (a2->type == FUNCALL_ACTION)
        return FALSE;

    /* Both are make_actions. */

    if (a1->preference_type != a2->preference_type)
        return FALSE;

    if (!symbols_are_equal_with_bindings(rhs_value_to_symbol(a1->id), rhs_value_to_symbol(a2->id), bindings))
        return FALSE;

    if ((rhs_value_is_symbol(a1->attr)) && (rhs_value_is_symbol(a2->attr))) {
        if (!symbols_are_equal_with_bindings(rhs_value_to_symbol(a1->attr), rhs_value_to_symbol(a2->attr), bindings))
            return FALSE;
    } else {
        if ((rhs_value_is_funcall(a1->attr)) && (rhs_value_is_funcall(a2->attr))) {
            if (!funcalls_match(rhs_value_to_funcall_list(a1->attr), rhs_value_to_funcall_list(a2->attr)))
                return FALSE;
        }
    }

    /* Values are different. They are rhs_value's. */

    if ((rhs_value_is_symbol(a1->value)) && (rhs_value_is_symbol(a2->value))) {
        if (symbols_are_equal_with_bindings(rhs_value_to_symbol(a1->value), rhs_value_to_symbol(a2->value), bindings))
            return TRUE;
        else
            return FALSE;
    }
    if ((rhs_value_is_funcall(a1->value)) && (rhs_value_is_funcall(a2->value))) {
        if (funcalls_match(rhs_value_to_funcall_list(a1->value), rhs_value_to_funcall_list(a2->value)))
            return TRUE;
        else
            return FALSE;
    }
    return FALSE;
}

/* Routine for RHS. */
void read_rhs_pattern_and_get_matching_productions(list ** current_pf_list, bool show_bindings,
                                                   bool just_chunks, bool no_chunks)
{
    action *a, *alist, *pa;
    int i;
    production *prod;
    list *bindings, *current_binding_point;
    bool match, match_this_a, parsed_ok;
    action *rhs;
    condition *top_cond, *bottom_cond;

    bindings = NIL;
    current_binding_point = NIL;

/*  print("Parsing as a rhs...\n"); */
    parsed_ok = parse_rhs(&alist);
    if (!parsed_ok) {
        print("Error: not a valid rhs.\n");
        current_pf_list = NIL;
        return;
    }

/*
  print("Valid RHS:\n");
  print_action_list(alist,0,FALSE);
  print("\nMatches:\n");
*/

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
        if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) || (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
            for (prod = current_agent(all_productions_of_type)[i]; prod != NIL; prod = prod->next) {
                match = TRUE;

                free_binding_list(bindings);
                bindings = NIL;

                p_node_to_conditions_and_nots(prod->p_node, NIL, NIL, &top_cond, &bottom_cond, NIL, &rhs);
                deallocate_condition_list(top_cond);
                for (a = alist; a != NIL; a = a->next) {
                    match_this_a = FALSE;
                    current_binding_point = bindings;

                    for (pa = rhs; pa != NIL; pa = pa->next) {
                        if (actions_are_equal_with_bindings(a, pa, &bindings)) {
                            match_this_a = TRUE;
                            break;
                        } else {
                            /* Remove new, incorrect bindings. */
                            reset_old_binding_point(&bindings, &current_binding_point);
                            bindings = current_binding_point;
                        }
                    }
                    if (!match_this_a) {
                        match = FALSE;
                        break;
                    }
                }
                deallocate_action_list(rhs);
                if (match) {
                    push(prod, (*current_pf_list));
                    if (show_bindings) {
                        print_with_symbols("%y, with bindings:\n", prod->name);
                        print_binding_list(bindings);
                    } else
                        print_with_symbols("%y\n", prod->name);
                }
            }
    if (bindings)
        free_binding_list(bindings);    /* DJP 4/3/96 -- To catch the last production */
}

/*
Soar_TextWidgetPrintData *
Soar_MakeTextWidgetPrintData (Tcl_Interp * interp, char * widget_name)
{
  Soar_TextWidgetPrintData * data;

  data = (Soar_TextWidgetPrintData *) 
         malloc (sizeof (Soar_TextWidgetPrintData));
  data->interp = interp;
  data->text_widget = savestring(widget_name);

  return data;
}

*/

/*
extern void Soar_FreeTextWidgetPrintData (Soar_TextWidgetPrintData * data)
{
  free((void *) data->text_widget);
  free((void *) data);
}

*/
/* kjh(CUSP-B2) begin */
extern Symbol *make_symbol_for_current_lexeme(void);

/*
 *----------------------------------------------------------------------
 *
 * print_current_watch_settings --
 *
 *	This procedure prints the current watch settings.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Prints the current watch settings.
 *
 *----------------------------------------------------------------------
 */

void print_current_watch_settings(void)
{
/* Added this to avoid segfault on Solaris when attempt to print NULL */
    /*char *a = NULL; */

    print("Current watch settings:\n");
    print("  Decisions:  %s\n", current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off");
    print("  Phases:  %s\n", current_agent(sysparams)[TRACE_PHASES_SYSPARAM] ? "on" : "off");
    print("  Production firings/retractions\n");
    print("    default productions:  %s\n",
          current_agent(sysparams)[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off");
    print("    user productions:  %s\n", current_agent(sysparams)[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off");
    print("    chunks:  %s\n", current_agent(sysparams)[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off");
    print("    justifications:  %s\n",
          current_agent(sysparams)[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
    print("    WME detail level:  %d\n", current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]);
    print("  Working memory changes:  %s\n", current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off");
    print("  Preferences generated by firings/retractions:  %s\n",
          current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off");
    /*  don't print these individually...see chunk-creation
     *  print ("  Chunk names:  %s\n",
     *      current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] ? "on" : "off");
     *  print ("  Justification names:  %s\n",
     *      current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM] ? "on" : "off");
     *  print ("  Chunks:  %s\n",
     *      current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM] ? "on" : "off");
     *  print ("  Justifications:  %s\n",
     *      current_agent(sysparams)[TRACE_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
     */
    print("\n");
    if (current_agent(sysparams)[TRACE_CHUNKS_SYSPARAM]) {
        print("  Learning:  -fullprint  (watch creation of chunks/just.)\n");
    } else {
        print("  Learning:  %s  (watch creation of chunks/just.)\n",
              current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] ? "-print" : "-noprint");
    }
    print("  Backtracing:  %s\n", current_agent(sysparams)[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off");

    print("  Loading:  %s\n", current_agent(sysparams)[TRACE_LOADING_SYSPARAM] ? "on" : "off");

	print("  Indifferent-selection:  %s\n", current_agent(sysparams)[TRACE_INDIFFERENT_SYSPARAM] ? "on" : "off");

    print("\n");
}

/*
 *----------------------------------------------------------------------
 *
 * set_watch_setting --
 *
 *	This procedure parses an argument to a watch variable to
 *      determine if "on" or "off" has been selected.
 *
 * Results:
 *	Tcl status code.
 *
 * Side effects:
 *	Sets the desired paramater to the indicated setting.
 *
 *----------------------------------------------------------------------
 */

int set_watch_setting(int dest_sysparam_number, const char *param, const char *arg, soarResult * res)
{
    if (arg == NULL) {
        setSoarResultResult(res, "Missing setting for watch parameter %s", param);
        return SOAR_ERROR;
    }

    if (!strcmp("-on", arg)) {
        set_sysparam(dest_sysparam_number, TRUE);
    } else if (!strcmp("-off", arg)) {
        set_sysparam(dest_sysparam_number, FALSE);
    } else if (!strncmp("-inclusive", arg, 3)) {
        soar_ecWatchLevel(dest_sysparam_number);
    } else {
        setSoarResultResult(res, "Unrecognized setting for watch parameter %s: %s", param, arg);
        return SOAR_ERROR;
    }
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * set_watch_prod_group_setting --
 *
 *	This procedure parses an argument to a watch production-type to
 *      determine if "print" or "noprint" has been selected.
 *
 * Results:
 *	Tcl status code.
 *
 * Side effects:
 *	Sets the desired prod-group to the indicated setting.
 *
 *----------------------------------------------------------------------
 */

int set_watch_prod_group_setting(int prodgroup, const char *prodtype, const char *arg, soarResult * res)
{
    if (arg == NULL) {
        setSoarResultResult(res, "Missing setting for watch %s", prodtype);
        return SOAR_ERROR;
    }

    if (!strcmp("-print", arg)) {
        switch (prodgroup) {
        case 0:
            set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
            set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
            set_sysparam(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
            set_sysparam(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, TRUE);
            break;
        case 1:
            set_sysparam(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, TRUE);
            break;
        case 2:
            set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, TRUE);
            break;
        case 3:
            set_sysparam(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, TRUE);
            break;
        case 4:
            set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, TRUE);
            break;
        }
    } else if (!strcmp("-noprint", arg)) {
        switch (prodgroup) {
        case 0:
            set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
            set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
            set_sysparam(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, FALSE);
            set_sysparam(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, FALSE);
            break;
        case 1:
            set_sysparam(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, FALSE);
            break;
        case 2:
            set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
            break;
        case 3:
            set_sysparam(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, FALSE);
            break;
        case 4:
            set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, FALSE);
            break;
        }
    } else if (!strcmp("-fullprint", arg)) {
        setSoarResultResult(res, "Sorry, -fullprint not yet implemented for watch productions");
        return SOAR_ERROR;
    } else {
        setSoarResultResult(res,
                            "Unrecognized setting for watch %s: %s.  Use -print|-noprint|-fullprint", prodtype, arg);
        return SOAR_ERROR;
    }
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_run_command--
 *
 *	This procedure parses an argv array and sets the agent parameters.
 *
 *      The syntax being parsed is:
 *          run [n] [units] [-self]
 *
 *              [n] is an integer, which defaults to 1 if a unit is 
 *                  specified; if no units, [n] defaults to forever.
 *              [units]:
 *                 d - decision cycles
 *                 e - elaboration cycles
 *                 p - phases
 *                 s - state
 *                 o - operator selection
 *               out - run by decision cycles until output generated
 *                <s>- current level of subgoaling
 *               <ss>- superstate's level of subgoaling
 *              <sss>- supersuperstate's level of subgoaling
 *                <o>- operator selection AT THIS LEVEL of subgoaling
 *                        or current level of subgoaling terminated
 *               <so>- superoperator selection (or THAT subgoaling terminated)
 *              <sso>- supersuperoperator selection (or subgoaling terminated)
 * 
 *              [-self] to run only the current agent interp.
 *                      (flag only returned, RunCmd processes it)
 *
 * Results:
 *	Returns the agent parameters and the Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
int parse_run_command(int argc, const char *argv[],
                      long *go_number,
                      enum go_type_enum *go_type,
                      Symbol * *go_slot_attr, goal_stack_level * go_slot_level, bool * self_only, soarResult * res)
{
    Symbol *g, *attr, *value;
    int i;
    bool no_number = TRUE;

    for (i = 1; i < argc; i++) {
        get_lexeme_from_string(argv[i]);

        if (current_agent(lexeme).type == INT_CONSTANT_LEXEME) {
            *go_number = current_agent(lexeme).int_val;
            no_number = FALSE;
        } else if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME) {
            if (!strcmp("forever", argv[i])) {
                *go_number = -1;
            } else if (!strcmp("p", argv[i])) {
                *go_type = GO_PHASE;
                if (no_number) {
                    *go_number = 1;
                    no_number = FALSE;
                }
            } else if (!strcmp("e", argv[i])) {
                *go_type = GO_ELABORATION;
                if (no_number) {
                    *go_number = 1;
                    no_number = FALSE;
                }
            } else if (!strcmp("d", argv[i])) {
                *go_type = GO_DECISION;
                if (no_number) {
                    *go_number = 1;
                    no_number = FALSE;
                }
            } else if (!strcmp("s", argv[i])) {
                *go_type = GO_STATE;
                if (no_number) {
                    *go_number = 1;
                    no_number = FALSE;
                }
            } else if (!strcmp("o", argv[i])) {
                *go_type = GO_OPERATOR;
                if (no_number) {
                    *go_number = 1;
                    no_number = FALSE;
                }
            } else if (!strcmp("out", argv[i])) {
                *go_type = GO_OUTPUT;
                if (no_number) {
                    *go_number = 1;
                    no_number = FALSE;
                }
            } else if (!strcmp("-self", argv[i])) {
                *self_only = TRUE;
            } else {
                setSoarResultResult(res, "Unrecognized argument to run command: %s", argv[i]);
                return SOAR_ERROR;
            }
        } else if (current_agent(lexeme).type == VARIABLE_LEXEME) {
            get_context_var_info(&g, &attr, &value);
            if (!attr) {
                setSoarResultResult(res,
                                    "Expected <s>, <o>, <ss>, <so>, <sss>, or <sso> as variable in run command, not %s",
                                    argv[i]);
                return SOAR_ERROR;
            }

            if (!g) {
                setSoarResultResult(res,
                                    "There is either no superstate, or no supersuperstate (whichever you specified, %s) of the current level",
                                    argv[i]);
                return SOAR_ERROR;
            }
            *go_type = GO_SLOT;
            *go_slot_level = g->id.level;
            *go_slot_attr = attr;
            if (no_number) {
                *go_number = 1;
                no_number = FALSE;
            }
        } else {
            setSoarResultResult(res, "Unrecognized argument to run command: %s", argv[i]);
            return SOAR_ERROR;
        }
    }

    /* if there were no units and no number given as args to run, 
     * then run forever.
     */
    if (no_number) {
        *go_number = -1;
    }

    return SOAR_OK;

}

int parse_go_command(int argc, char *argv[],
                     long *go_number,
                     enum go_type_enum *go_type,
                     Symbol * *go_slot_attr, goal_stack_level * go_slot_level, soarResult * res)
{
    Symbol *g, *attr, *value;
    int i;

    for (i = 1; i < argc; i++) {
        get_lexeme_from_string(argv[i]);

        if (current_agent(lexeme).type == INT_CONSTANT_LEXEME) {
            *go_number = current_agent(lexeme).int_val;
        } else if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME) {
            if (!strcmp("forever", argv[i])) {
                *go_number = -1;
            } else if (!strcmp("p", argv[i])) {
                *go_type = GO_PHASE;
            } else if (!strcmp("e", argv[i])) {
                *go_type = GO_ELABORATION;
            } else if (!strcmp("d", argv[i])) {
                *go_type = GO_DECISION;
            } else if (!strcmp("s", argv[i])) {
                *go_type = GO_STATE;
            } else if (!strcmp("o", argv[i])) {
                *go_type = GO_OPERATOR;
            } else {
                setSoarResultResult(res, "Unrecognized argument to go command: %s", argv[i]);
                return SOAR_ERROR;
            }
        } else if (current_agent(lexeme).type == VARIABLE_LEXEME) {
            get_context_var_info(&g, &attr, &value);
            if (!attr) {
                setSoarResultResult(res, "Expected a context variable in go command, not %s", argv[i]);
                return SOAR_ERROR;
            }

            if (!g) {
                setSoarResultResult(res, "Goal stack level %s does not exist", argv[i]);
                return SOAR_ERROR;
            }

            *go_type = GO_SLOT;
            *go_slot_level = g->id.level;
            *go_slot_attr = attr;
        } else {
            setSoarResultResult(res, "Unrecognized argument to go command: %s", argv[i]);
            return SOAR_ERROR;
        }
    }

    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_memory_stats --
 *
 *	This procedure parses an argv array and prints the selected
 *      statistics.
 *
 *      The syntax being parsed is:
 *         stats -rete <mtype>
 *         <mtype> ::= -total
 *                     -overhead
 *                     -strings
 *                     -hash-table
 *                     -pool [-total | pool-name [<aspect>]]
 *                     -misc
 *        <aspect> ::= -used                   |M
 *                     -free                   |M
 *                     -item-size
 *                     -total-bytes
 *
 *          The items marked |M are available only when Soar has
 *          been compiled with the MEMORY_POOL_STATS option.
 *
 * Results:
 *	Returns the statistic and Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int parse_memory_stats(int argc, const char *argv[], soarResult * res)
{
    if (argc == 2) {
        soar_ecPrintMemoryStatistics();
        soar_ecPrintMemoryPoolStatistics();

        return SOAR_OK;
    }

    if (!strcmp("-total", argv[2])) {
        unsigned long total;
        int i;

        total = 0;
        for (i = 0; i < NUM_MEM_USAGE_CODES; i++)
            total += current_agent(memory_for_usage)[i];

		print("%lu", total);
        /*setSoarResultResult(res, "%lu", total);*/
    } else if (!strcmp("-overhead", argv[2])) {
		print("%lu", current_agent(memory_for_usage)[STATS_OVERHEAD_MEM_USAGE]);
        /*setSoarResultResult(res, "%lu", current_agent(memory_for_usage)[STATS_OVERHEAD_MEM_USAGE]);*/
    } else if (!strcmp("-strings", argv[2])) {
		print("%lu", current_agent(memory_for_usage)[STRING_MEM_USAGE]);
        /*setSoarResultResult(res, "%lu", current_agent(memory_for_usage)[STRING_MEM_USAGE]);*/
    } else if (!strcmp("-hash-table", argv[2])) {
		print("%lu", current_agent(memory_for_usage)[HASH_TABLE_MEM_USAGE]);
        /*setSoarResultResult(res, "%lu", current_agent(memory_for_usage)[HASH_TABLE_MEM_USAGE]);*/
    } else if (!strcmp("-pool", argv[2])) {     /* Handle pool stats */
        if (argc == 3) {
            soar_ecPrintMemoryPoolStatistics();
        } else if (!strcmp("-total", argv[3])) {
			print("%lu", current_agent(memory_for_usage)[POOL_MEM_USAGE]);
            /*setSoarResultResult(res, "%lu", current_agent(memory_for_usage)[POOL_MEM_USAGE]);*/
        } else {                /* Match pool name or invalid item */
            memory_pool *p;
            memory_pool *pool_found = NIL;

            for (p = current_agent(memory_pools_in_use); p != NIL; p = p->next) {
                if (!strcmp(argv[3], p->name)) {
                    pool_found = p;
                    break;
                }
            }

            if (!pool_found) {
                setSoarResultResult(res, "Unrecognized pool name: stats -memory -pool %s", argv[4]);
                return SOAR_ERROR;
            }

            if (argc == 4) {
#ifdef MEMORY_POOL_STATS
                long total_items;
#endif
                print("Memory pool statistics:\n\n");
#ifdef MEMORY_POOL_STATS
                print("Pool Name        Used Items  Free Items  Item Size  Total Bytes\n");
                print("---------------  ----------  ----------  ---------  -----------\n");
#else
                print("Pool Name        Item Size  Total Bytes\n");
                print("---------------  ---------  -----------\n");
#endif

                print_string(pool_found->name);
                print_spaces(MAX_POOL_NAME_LENGTH - strlen(pool_found->name));
#ifdef MEMORY_POOL_STATS
                print("  %10lu", pool_found->used_count);
                total_items = pool_found->num_blocks * pool_found->items_per_block;
                print("  %10lu", total_items - pool_found->used_count);
#endif
                print("  %9lu", pool_found->item_size);
                print("  %11lu\n", pool_found->num_blocks * pool_found->items_per_block * pool_found->item_size);
            } else if (argc == 5) {     /* get pool attribute */
                long total_items;

                total_items = pool_found->num_blocks * pool_found->items_per_block;

                if (!strcmp("-item-size", argv[4])) {
					print("%lu", pool_found->item_size);
                    /*setSoarResultResult(res, "%lu", pool_found->item_size);*/
                }
#ifdef MEMORY_POOL_STATS
                else if (!strcmp("-used", argv[4])) {
					print("%lu", pool_found->used_count);
                    /*setSoarResultResult(res, "%lu", pool_found->used_count);*/
                } else if (!strcmp("-free", argv[4])) {
					print("%lu", total_items - pool_found->used_count);
                    /*setSoarResultResult(res, "%lu", total_items - pool_found->used_count);*/
                }
#endif
                else if (!strcmp("-total-bytes", argv[4])) {
					print("%lu",
                          pool_found->num_blocks * pool_found->items_per_block * pool_found->item_size);
                    /*setSoarResultResult(res, "%lu",
                                        pool_found->num_blocks * pool_found->items_per_block * pool_found->item_size);*/
                } else {
                    setSoarResultResult(res, "Unrecognized argument to stats: -memory -pool %s %s", argv[3], argv[4]);
                    return SOAR_ERROR;
                }
            } else {
                setSoarResultResult(res,
                                    "Too many arguments, should be: stats -memory -pool [-total | pool-name [<aspect>]]");
                return SOAR_ERROR;
            }
        }
    } else if (!strcmp("-misc", argv[2])) {
		print("%lu", current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);
        /*setSoarResultResult(res, "%lu", current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);*/
    } else {
        setSoarResultResult(res, "Unrecognized argument to stats: -memory %s", argv[2]);
        return SOAR_ERROR;
    }

    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_rete_stats --
 *
 *	This procedure parses an argv array and prints the selected
 *      statistics.
 *
 *      The syntax being parsed is:
 *          stats -rete <rtype> <qualifier>
 *          <rtype> ::= unhashed memory
 *                      memory
 *                      unhashed mem-pos
 *                      mem-pos
 *                      unhashed negative
 *                      negative
 *                      unhashed positive
 *                      positive
 *                      dummy top
 *                      dummy matches
 *                      nconj. neg.
 *                      conj. neg. partner
 *                      production
 *                      total
 *          <qualifier> ::= -actual | -if-no-merging | -if-no-sharing
 *
 * Results:
 *	Returns the statistic and Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int parse_rete_stats(int argc, const char *argv[], soarResult * res)
{
    unsigned long data;

    if (argc == 2) {
        soar_ecPrintReteStatistics();
        return SOAR_OK;
    }

    if (argc == 3) {
        setSoarResultResult(res, "Too few arguments, should be: stats -rete [type qualifier]");
        return SOAR_ERROR;
    }

    if (argc > 4) {
        setSoarResultResult(res, "Too many arguments, should be: stats -rete [type qualifier]");
        return SOAR_ERROR;
    }

    if (get_node_count_statistic(argv[2], (char *) argv[3] + 1, &data)) {
		print("%lu", data);
        /*setSoarResultResult(res, "%lu", data);*/
    } else {
        setSoarResultResult(res, "Unrecognized argument to stats: -rete %s %s", argv[2], argv[3]);
        return SOAR_ERROR;
    }
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_system_stats --
 *
 *	This procedure parses an argv array and prints the selected
 *      statistics.
 *
 *      The syntax being parsed is:
 *         stats -system <stype>
 *         <stype> ::= -default-production-count
 *                     -user-production-count
 *                     -chunk-count
 *                     -justification-count
 *                     -all-productions-count
 *                     -dc-count
 *                     -ec-count
 *                     -ecs/dc
 *                     -firings-count
 *                     -firings/ec
 *                     -wme-change-count
 *                     -wme-addition-count
 *                     -wme-removal-count
 *                     -wme-count
 *                     -wme-avg-count
 *                     -wme-max-count
 *                     -total-time             |T
 *                     -ms/dc                  |T
 *                     -ms/ec                  |T
 *                     -ms/firing              |T
 *                     -ms/wme-change          |T
 *                     -match-time             |D
 *                     -ownership-time         |D
 *                     -chunking-time          |D
 *
 *         The items marked |T are available when Soar has been
 *         compiled with the NO_TIMING_STUFF flag NOT SET and 
 *         the items marked |D are available when Soar has been
 *         compiled with the DETAILED_TIMING_STATS flag SET.
 *
 * Results:
 *	Returns the statistic and Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int parse_system_stats(int argc, const char *argv[], soarResult * res)
{
#ifndef NO_TIMING_STUFF
    double total_kernel_time, total_kernel_msec;

#ifdef DETAILED_TIMING_STATS
    double time;
#endif
#endif

    if (argc > 3) {
        setSoarResultResult(res, "Too many arguments, should be: stats -system [<type>]");
        return SOAR_ERROR;
    }

    total_kernel_time = timer_value(&current_agent(total_kernel_time));
    total_kernel_msec = total_kernel_time * 1000.0;

    if (argc <= 2) {            /* Invoked as stats or stats -system */
        soar_ecPrintSystemStatistics();
    } else {
        if (!strcmp("-default-production-count", argv[2])) {
			print("%lu", current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE]);
            /*setSoarResultResult(res, "%lu", current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE]);*/
        } else if (!strcmp("-user-production-count", argv[2])) {
            print("%lu", current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE]);
			/*setSoarResultResult(res, "%lu", current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE]);*/

        } else if (!strcmp("-chunk-count", argv[2])) {
			print("%lu", current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);
            /*setSoarResultResult(res, "%lu", current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);*/
        } else if (!strcmp("-justification-count", argv[2])) {
			print("%lu", current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);
            /*setSoarResultResult(res, "%lu", current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);*/
        } else if (!strcmp("-all-productions-count", argv[2])) {
            print("%lu", current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE]
                                + current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE]
                                + current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);

			/*setSoarResultResult(res, "%lu", current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE]
                                + current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE]
                                + current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);*/

        } else if (!strcmp("-dc-count", argv[2])) {
			print("%lu", current_agent(d_cycle_count));
            /*setSoarResultResult(res, "%lu", current_agent(d_cycle_count));*/
        } else if (!strcmp("-ec-count", argv[2])) {
			print("%lu", current_agent(e_cycle_count));
            /*setSoarResultResult(res, "%lu", current_agent(e_cycle_count));*/
        } else if (!strcmp("-ecs/dc", argv[2])) {
            print("%.3f", (current_agent(d_cycle_count)
                                              ? ((double) current_agent(e_cycle_count)
                                                 / current_agent(d_cycle_count))
                                              : 0.0));
			/*setSoarResultResult(res, "%.3f", (current_agent(d_cycle_count)
                                              ? ((double) current_agent(e_cycle_count)
                                                 / current_agent(d_cycle_count))
                                              : 0.0));*/
        } else if (!strcmp("-firings-count", argv[2])) {
			print("%lu", current_agent(production_firing_count));
            /*setSoarResultResult(res, "%lu", current_agent(production_firing_count));*/
        } else if (!strcmp("-firings/ec", argv[2])) {
			print("%.3f", (current_agent(e_cycle_count)
                                              ? ((double) current_agent(production_firing_count)
                                                 / current_agent(e_cycle_count))
                                              : 0.0));
            /*setSoarResultResult(res, "%.3f", (current_agent(e_cycle_count)
                                              ? ((double) current_agent(production_firing_count)
                                                 / current_agent(e_cycle_count))
                                              : 0.0));*/
        } else if (!strcmp("-wme-change-count", argv[2])) {
            print("%lu", current_agent(wme_addition_count)
                                + current_agent(wme_removal_count));
			/*setSoarResultResult(res, "%lu", current_agent(wme_addition_count)
                                + current_agent(wme_removal_count));*/
        } else if (!strcmp("-wme-addition-count", argv[2])) {
			print("%lu", current_agent(wme_addition_count));
            /*setSoarResultResult(res, "%lu", current_agent(wme_addition_count));*/
        } else if (!strcmp("-wme-removal-count", argv[2])) {
			print("%lu", current_agent(wme_removal_count));
            /*setSoarResultResult(res, "%lu", current_agent(wme_removal_count));*/
        } else if (!strcmp("-wme-count", argv[2])) {
            setSoarResultResult(res, "%lu", current_agent(num_wmes_in_rete));
        } else if (!strcmp("-wme-avg-count", argv[2])) {
			print("%.3f", (current_agent(num_wm_sizes_accumulated)
                                              ? (current_agent(cumulative_wm_size)
                                                 / current_agent(num_wm_sizes_accumulated))
                                              : 0.0));
            /*setSoarResultResult(res, "%.3f", (current_agent(num_wm_sizes_accumulated)
                                              ? (current_agent(cumulative_wm_size)
                                                 / current_agent(num_wm_sizes_accumulated))
                                              : 0.0));*/
        } else if (!strcmp("-wme-max-count", argv[2])) {
			print("%lu", current_agent(max_wm_size));
            /*setSoarResultResult(res, "%lu", current_agent(max_wm_size));*/
        }
#ifndef NO_TIMING_STUFF
        else if (!strcmp("-total-time", argv[2])) {
			print("%.3f", total_kernel_time);
            /*setSoarResultResult(res, "%.3f", total_kernel_time);*/
        } else if (!strcmp("-ms/dc", argv[2])) {
			print("%.3f", (current_agent(d_cycle_count)
                                              ? total_kernel_msec / current_agent(d_cycle_count)
                                              : 0.0));
            /*setSoarResultResult(res, "%.3f", (current_agent(d_cycle_count)
                                              ? total_kernel_msec / current_agent(d_cycle_count)
                                              : 0.0));*/
        } else if (!strcmp("-ms/ec", argv[2])) {
			print("%.3f", (current_agent(e_cycle_count)
                                              ? total_kernel_msec / current_agent(e_cycle_count)
                                              : 0.0));
            /*setSoarResultResult(res, "%.3f", (current_agent(e_cycle_count)
                                              ? total_kernel_msec / current_agent(e_cycle_count)
                                              : 0.0));*/
        } else if (!strcmp("-ms/firing", argv[2])) {
			print("%.3f", (current_agent(production_firing_count)
                                              ? total_kernel_msec / current_agent(production_firing_count)
                                              : 0.0));
            /*setSoarResultResult(res, "%.3f", (current_agent(production_firing_count)
                                              ? total_kernel_msec / current_agent(production_firing_count)
                                              : 0.0));*/
        }
#endif                          /* NO_TIMING_STUFF */
#ifdef DETAILED_TIMING_STATS
        else if (!strcmp("-ms/wme-change", argv[2])) {
            long wme_changes;
            time = timer_value(&current_agent(match_cpu_time[INPUT_PHASE]))
                + timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE]))
                + timer_value(&current_agent(match_cpu_time[PREFERENCE_PHASE]))
                + timer_value(&current_agent(match_cpu_time[WM_PHASE]))
                + timer_value(&current_agent(match_cpu_time[OUTPUT_PHASE]))
                + timer_value(&current_agent(match_cpu_time[DECISION_PHASE]));

            time *= 1000;       /* convert to msec */

            wme_changes = current_agent(wme_addition_count)
                + current_agent(wme_removal_count);

			print("%.3f", (wme_changes ? time / wme_changes : 0.0));
            /*setSoarResultResult(res, "%.3f", (wme_changes ? time / wme_changes : 0.0));*/
        } else if (!strcmp("-match-time", argv[2])) {

            time = timer_value(&current_agent(match_cpu_time[INPUT_PHASE]))
                + timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE]))
                + timer_value(&current_agent(match_cpu_time[PREFERENCE_PHASE]))
                + timer_value(&current_agent(match_cpu_time[WM_PHASE]))
                + timer_value(&current_agent(match_cpu_time[OUTPUT_PHASE]))
                + timer_value(&current_agent(match_cpu_time[DECISION_PHASE]));

			print("%.3f", time);
            /*setSoarResultResult(res, "%.3f", time);*/

        } else if (!strcmp("-ownership-time", argv[2])) {
            time = timer_value(&current_agent(ownership_cpu_time[INPUT_PHASE]))
                + timer_value(&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE]))
                + timer_value(&current_agent(ownership_cpu_time[PREFERENCE_PHASE]))
                + timer_value(&current_agent(ownership_cpu_time[WM_PHASE]))
                + timer_value(&current_agent(ownership_cpu_time[OUTPUT_PHASE]))
                + timer_value(&current_agent(ownership_cpu_time[DECISION_PHASE]));

            print("%.3f", time);
			/*setSoarResultResult(res, "%.3f", time);*/

        } else if (!strcmp("-chunking-time", argv[2])) {
            time = timer_value(&current_agent(chunking_cpu_time[INPUT_PHASE]))
                + timer_value(&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE]))
                + timer_value(&current_agent(chunking_cpu_time[PREFERENCE_PHASE]))
                + timer_value(&current_agent(chunking_cpu_time[WM_PHASE]))
                + timer_value(&current_agent(chunking_cpu_time[OUTPUT_PHASE]))
                + timer_value(&current_agent(chunking_cpu_time[DECISION_PHASE]));

			print("%.3f", time);
            /*setSoarResultResult(res, "%.3f", time);*/

        }
#endif                          /* DETAILED_TIMING_STATS */
        else {
            setSoarResultResult(res, "Unrecognized argument to stats: -system %s", argv[2]);
        }
    }

    return SOAR_OK;
}

int printTimingInfo()
{

#ifdef PII_TIMERS
    unsigned long long int start, stop;
#else
    struct timeval start, stop;
#endif

    double min, max, min_nz;

    min_nz = soar_cDetermineTimerResolution(&min, &max);

#ifdef PII_TIMERS
    print("Using Pentium II Time Stamp -- Assuming %d MHZ Processor...\n", MHZ);
#else
    print("Using system timers...\n");
#endif

    print("---------------------------------------------------------------\n");
    print("A timing loop is used to obtain the following values.\n");
    print("At least one additional instruction takes place between\n");
    print("starting and stopping the timers, thus a perfectly accurate\n");
    print("timer which costs nothing to invoke would still accumulate\n");
    print("non-zero value. The loop runs for a total of ~2 seconds as \n");
    print("a result, the Maximum timer value is likely to be relatively \n");
    print("large, but should be < 2 seconds\n");
    print("** NOTE: If the code was optimized, the timing loop may yield\n");
    print("         unanticipated results.  If both minimum values are\n");
    print("         zero, it is unclear what the timer resolution is...\n");
    print("---------------------------------------------------------------\n");
    print("Minimum (Non-Zero) Timer Value: %11.5e sec\n", min_nz);
    print("Minimum Timer Value           : %11.5e sec\n", min);
    print("Maximum Timer Value           : %11.5e sec\n\n", max);

    print("---------------------------------------------------------------\n");
    print("A short delay will be issued using the sleep() command, and \n");
    print("timed using Soar's timers....\n");
    reset_timer(&stop);
    start_timer(&start);
    sys_sleep(3);
    stop_timer(&start, &stop);
    print("Sleep interval  -->   3 seconds\n");
    print("Timers report   -->  %8.5f seconds\n", timer_value(&stop));
    print("---------------------------------------------------------------\n");

    return 1;

}
