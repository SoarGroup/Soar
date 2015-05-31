/////////////////////////////////////////////////////////////////
// production-find command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "agent.h"
#include "lexer.h"
#include "print.h"
#include "mem.h"
#include "parser.h"
#include "rete.h"
#include "rhs.h"
#include "test.h"

using namespace cli;
using namespace sml;

void free_binding_list(agent* thisAgent, list* bindings)
{
    cons* c;

    for (c = bindings; c != NIL; c = c->rest)
    {
        thisAgent->memPoolManager->free_memory(c->first, MISCELLANEOUS_MEM_USAGE);
    }
    free_list(thisAgent, bindings);
}

void print_binding_list(agent* thisAgent, list* bindings)
{
    cons* c;

    for (c = bindings ; c != NIL ; c = c->rest)
    {
        print_with_symbols(thisAgent, "   (%y -> %y)\n", static_cast<Binding*>(c->first)->from, static_cast<Binding*>(c->first)->to);
    }
}

void reset_old_binding_point(agent* thisAgent, list** bindings, list** current_binding_point)
{
    cons* c, *c_next;

    c = *bindings;
    while (c != *current_binding_point)
    {
        c_next = c->rest;
        thisAgent->memPoolManager->free_memory(c->first, MISCELLANEOUS_MEM_USAGE);
        free_cons(thisAgent, c);
        c = c_next;
    }

    bindings = current_binding_point;
}

Symbol* get_binding(Symbol* f, list* bindings)
{
    cons* c;

    for (c = bindings; c != NIL; c = c->rest)
    {
        if (static_cast<Binding*>(c->first)->from == f)
        {
            return static_cast<Binding*>(c->first)->to;
        }
    }
    return NIL;
}

bool symbols_are_equal_with_bindings(agent* thisAgent, Symbol* s1, Symbol* s2, list** bindings)
{
    Binding* b;
    Symbol* bvar;

    if ((s1 == s2) && (s1->symbol_type != VARIABLE_SYMBOL_TYPE))
    {
        return true;
    }

    /* "*" matches everything. */
    if ((s1->symbol_type == STR_CONSTANT_SYMBOL_TYPE) &&
            (!strcmp(s1->sc->name, "*")))
    {
        return true;
    }
    if ((s2->symbol_type == STR_CONSTANT_SYMBOL_TYPE) &&
            (!strcmp(s2->sc->name, "*")))
    {
        return true;
    }


    if ((s1->symbol_type != VARIABLE_SYMBOL_TYPE) ||
            (s2->symbol_type != VARIABLE_SYMBOL_TYPE))
    {
        return false;
    }
    /* Both are variables */
    bvar = get_binding(s1, *bindings);
    if (bvar == NIL)
    {
        b = static_cast<Binding*>(thisAgent->memPoolManager->allocate_memory(sizeof(Binding), MISCELLANEOUS_MEM_USAGE));
        b->from = s1;
        b->to = s2;
        push(thisAgent, b, *bindings);
        return true;
    }
    else if (bvar == s2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool actions_are_equal_with_bindings(agent* thisAgent, action* a1, action* a2, list** bindings)
{
    //         if (a1->type == FUNCALL_ACTION)
    //         {
    //            if ((a2->type == FUNCALL_ACTION))
    //            {
    //               if (funcalls_match(rhs_value_to_funcall_list(a1->value),
    //                  rhs_value_to_funcall_list(a2->value)))
    //               {
    //                     return true;
    //               }
    //               else return false;
    //            }
    //            else return false;
    //         }
    if (a2->type == FUNCALL_ACTION)
    {
        return false;
    }

    /* Both are make_actions. */

    if (a1->preference_type != a2->preference_type)
    {
        return false;
    }

    if (!symbols_are_equal_with_bindings(thisAgent, rhs_value_to_symbol(a1->id),
                                         rhs_value_to_symbol(a2->id),
                                         bindings))
    {
        return false;
    }

    if ((rhs_value_is_symbol(a1->attr)) && (rhs_value_is_symbol(a2->attr)))
    {
        if (!symbols_are_equal_with_bindings(thisAgent, rhs_value_to_symbol(a1->attr),
                                             rhs_value_to_symbol(a2->attr), bindings))
        {
            return false;
        }
    }
    else
    {
        //            if ((rhs_value_is_funcall(a1->attr)) && (rhs_value_is_funcall(a2->attr)))
        //            {
        //               if (!funcalls_match(rhs_value_to_funcall_list(a1->attr),
        //                  rhs_value_to_funcall_list(a2->attr)))
        //               {
        //                  return false;
        //               }
        //            }
    }

    /* Values are different. They are rhs_value's. */

    if ((rhs_value_is_symbol(a1->value)) && (rhs_value_is_symbol(a2->value)))
    {
        if (symbols_are_equal_with_bindings(thisAgent, rhs_value_to_symbol(a1->value),
                                            rhs_value_to_symbol(a2->value), bindings))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    if ((rhs_value_is_funcall(a1->value)) && (rhs_value_is_funcall(a2->value)))
    {
        //            if (funcalls_match(rhs_value_to_funcall_list(a1->value),
        //               rhs_value_to_funcall_list(a2->value)))
        //            {
        //               return true;
        //            }
        //            else
        {
            return false;
        }
    }
    return false;
}


#define dealloc_and_return(thisAgent,x,y) { deallocate_test(thisAgent, x) ; return (y) ; }

bool tests_are_equal_with_bindings(agent* thisAgent, test t1, test test2, list** bindings)
{
    cons* c1, *c2;
    bool goal_test, impasse_test;

    /* DJP 4/3/96 -- The problem here is that sometimes test2 was being copied      */
    /*               and sometimes it wasn't.  If it was copied, the copy was never */
    /*               deallocated.  There's a few choices about how to fix this.  I  */
    /*               decided to just create a copy always and then always           */
    /*               deallocate it before returning.  Added a macro to do that.     */

    test t2;

    /* t1 is from the pattern given to "pf"; t2 is from a production's condition list. */
    if (!t1)
    {
        return (test2 != 0);
    }

    /* If the pattern doesn't include "(state", but the test from the
    * production does, strip it out of the production's.
    */
    if ((!test_includes_goal_or_impasse_id_test(t1, true, false)) &&
            test_includes_goal_or_impasse_id_test(test2, true, false))
    {
        goal_test = false;
        impasse_test = false;
        t2 = copy_test_removing_goal_impasse_tests(thisAgent, test2, &goal_test, &impasse_test);
    }
    else
    {
        t2 = copy_test(thisAgent, test2) ; /* DJP 4/3/96 -- Always make t2 into a copy */
    }
    if (t1->type == EQUALITY_TEST)
    {
        if (!(t2 && (t2->type == EQUALITY_TEST)))
        {
            dealloc_and_return(thisAgent, t2, false);
        }
        else
        {
            if (symbols_are_equal_with_bindings(thisAgent, t1->data.referent, t2->data.referent, bindings))
            {
                dealloc_and_return(thisAgent, t2, true);
            }
            else
            {
                dealloc_and_return(thisAgent, t2, false);
            }
        }
    }

    if (t1->type != t2->type)
    {
        dealloc_and_return(thisAgent, t2, false);
    }

    switch (t1->type)
    {
        case GOAL_ID_TEST:
            dealloc_and_return(thisAgent, t2, true);
            break;
        case IMPASSE_ID_TEST:
            dealloc_and_return(thisAgent, t2, true);
            break;
        case DISJUNCTION_TEST:
            for (c1 = t1->data.disjunction_list, c2 = t2->data.disjunction_list;
                    ((c1 != NIL) && (c2 != NIL));
                    c1 = c1->rest, c2 = c2->rest)
            {
                if (c1->first != c2->first)
                {
                    dealloc_and_return(thisAgent, t2, false)
                }
            }
            if (c1 == c2)
            {
                dealloc_and_return(thisAgent, t2, true); /* make sure they both hit end-of-list */
            }
            else
            {
                dealloc_and_return(thisAgent, t2, false);
            }
            break;
        case CONJUNCTIVE_TEST:
            for (c1 = t1->data.conjunct_list, c2 = t2->data.conjunct_list;
                    ((c1 != NIL) && (c2 != NIL)); c1 = c1->rest, c2 = c2->rest)
            {
                if (!tests_are_equal_with_bindings(thisAgent, static_cast<test>(c1->first), static_cast<test>(c2->first), bindings))
                    dealloc_and_return(thisAgent, t2, false)
                }
            if (c1 == c2)
            {
                dealloc_and_return(thisAgent, t2, true); /* make sure they both hit end-of-list */
            }
            else
            {
                dealloc_and_return(thisAgent, t2, false);
            }
            break;
        default:  /* relational tests other than equality */
            if (symbols_are_equal_with_bindings(thisAgent, t1->data.referent, t2->data.referent, bindings))
            {
                dealloc_and_return(thisAgent, t2, true);
            }
            else
            {
                dealloc_and_return(thisAgent, t2, false);
            }
            break;
    }
    return false;
}

bool conditions_are_equal_with_bindings(agent* thisAgent, condition* c1, condition* c2, list** bindings)
{
    if (c1->type != c2->type)
    {
        return false;
    }
    switch (c1->type)
    {
        case POSITIVE_CONDITION:
        case NEGATIVE_CONDITION:
            if (! tests_are_equal_with_bindings(thisAgent, c1->data.tests.id_test,
                                                c2->data.tests.id_test, bindings))
            {
                return false;
            }
            if (! tests_are_equal_with_bindings(thisAgent, c1->data.tests.attr_test,
                                                c2->data.tests.attr_test, bindings))

            {
                return false;
            }
            if (! tests_are_equal_with_bindings(thisAgent, c1->data.tests.value_test,
                                                c2->data.tests.value_test, bindings))
            {
                return false;
            }
            if (c1->test_for_acceptable_preference != c2->test_for_acceptable_preference)
            {
                return false;
            }
            return true;

        case CONJUNCTIVE_NEGATION_CONDITION:
            for (c1 = c1->data.ncc.top, c2 = c2->data.ncc.top;
                    ((c1 != NIL) && (c2 != NIL));
                    c1 = c1->next, c2 = c2->next)
                if (! conditions_are_equal_with_bindings(thisAgent, c1, c2, bindings))
                {
                    return false;
                }
            if (c1 == c2)
            {
                return true;    /* make sure they both hit end-of-list */
            }
            return false;
    }
    return false; /* unreachable, but without it, gcc -Wall warns here */
}

void read_pattern_and_get_matching_productions(agent* thisAgent,
        const char* lhs_str,
        list** current_pf_list,
        bool show_bindings,
        bool just_chunks,
        bool no_chunks)
{
    condition* c, *clist, *top, *bottom, *pc;
    int i;
    production* prod;
    list* bindings, *current_binding_point;
    bool match, match_this_c;


    bindings = NIL;
    current_binding_point = NIL;

    /*  print("Parsing as a lhs...\n"); */
    soar::Lexer lexer(thisAgent, lhs_str);
    lexer.get_lexeme();
    clist = parse_lhs(thisAgent, &lexer);
    if (!clist)
    {
        print(thisAgent,  "Error: not a valid condition list.\n");
        current_pf_list = NIL;
        return;
    }
    /*
    print("Valid condition list:\n");
    print_condition_list(clist,0,false);
    print("\nMatches:\n");
    */

    /* For the moment match against productions of all types (user,chunk,default, justification).     Later on the type should be a parameter.
    */

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
        if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) ||
                (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
            for (prod = thisAgent->all_productions_of_type[i]; prod != NIL; prod = prod->next)
            {

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

                match = true;
                p_node_to_conditions_and_rhs(thisAgent, prod->p_node, NIL, NIL, &top, &bottom, NIL);

                free_binding_list(thisAgent, bindings);
                bindings = NIL;

                for (c = clist; c != NIL; c = c->next)
                {
                    match_this_c = false;
                    current_binding_point = bindings;

                    for (pc = top; pc != NIL; pc = pc->next)
                    {
                        if (conditions_are_equal_with_bindings(thisAgent, c, pc, &bindings))
                        {
                            match_this_c = true;
                            break;
                        }
                        else
                        {
                            /* Remove new, incorrect bindings. */
                            reset_old_binding_point(thisAgent, &bindings, &current_binding_point);
                            bindings = current_binding_point;
                        }
                    }
                    if (!match_this_c)
                    {
                        match = false;
                        break;
                    }
                }
                deallocate_condition_list(thisAgent, top);  /* DJP 4/3/96 -- Never dealloced */
                if (match)
                {
                    push(thisAgent, prod, (*current_pf_list));
                    if (show_bindings)
                    {
                        print_with_symbols(thisAgent, "%y, with bindings:\n", prod->name);
                        print_binding_list(thisAgent, bindings);
                    }
                    else
                    {
                        print_with_symbols(thisAgent, "%y\n", prod->name);
                    }
                }
            }
    if (bindings)
    {
        free_binding_list(thisAgent, bindings);    /* DJP 4/3/96 -- To catch the last production */
    }
}

void read_rhs_pattern_and_get_matching_productions(agent* thisAgent,
        const char* rhs_string,
        list** current_pf_list,
        bool show_bindings,
        bool just_chunks,
        bool no_chunks)
{

    action* a, *alist, *pa;
    int i;
    production* prod;
    list* bindings, *current_binding_point;
    bool match, match_this_a, parsed_ok;
    action* rhs;
    condition* top_cond, *bottom_cond;

    bindings = NIL;
    current_binding_point = NIL;

    /*  print("Parsing as a rhs...\n"); */
    soar::Lexer lexer(thisAgent, rhs_string);
    lexer.get_lexeme();
    parsed_ok = (parse_rhs(thisAgent, &lexer, &alist) == true);
    if (!parsed_ok)
    {
        print(thisAgent,  "Error: not a valid rhs.\n");
        current_pf_list = NIL;
        return;
    }

    /*
    print("Valid RHS:\n");
    print_action_list(alist,0,false);
    print("\nMatches:\n");
    */

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
    {
        if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) || (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
        {
            for (prod = thisAgent->all_productions_of_type[i]; prod != NIL; prod = prod->next)
            {
                match = true;

                free_binding_list(thisAgent, bindings);
                bindings = NIL;

                p_node_to_conditions_and_rhs(thisAgent, prod->p_node, NIL, NIL, &top_cond, &bottom_cond, &rhs);
                deallocate_condition_list(thisAgent, top_cond);
                for (a = alist; a != NIL; a = a->next)
                {
                    match_this_a = false;
                    current_binding_point = bindings;

                    for (pa = rhs; pa != NIL; pa = pa->next)
                    {
                        if (actions_are_equal_with_bindings(thisAgent, a, pa, &bindings))
                        {
                            match_this_a = true;
                            break;
                        }
                        else
                        {
                            /* Remove new, incorrect bindings. */
                            reset_old_binding_point(thisAgent, &bindings, &current_binding_point);
                            bindings = current_binding_point;
                        }
                    }
                    if (!match_this_a)
                    {
                        match = false;
                        break;
                    }
                }

                deallocate_action_list(thisAgent, rhs);
                if (match)
                {
                    push(thisAgent, prod, (*current_pf_list));
                    if (show_bindings)
                    {
                        print_with_symbols(thisAgent, "%y, with bindings:\n", prod->name);
                        print_binding_list(thisAgent, bindings);
                    }
                    else
                    {
                        print_with_symbols(thisAgent, "%y\n", prod->name);
                    }
                }
            }
        }
    }
    if (bindings)
    {
        free_binding_list(thisAgent, bindings); /* DJP 4/3/96 -- To catch the last production */
    }
}

bool CommandLineInterface::DoProductionFind(const ProductionFindBitset& options, const std::string& pattern)
{
    list* current_pf_list = 0;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (options.test(PRODUCTION_FIND_INCLUDE_LHS))
    {
        /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
        read_pattern_and_get_matching_productions (thisAgent,
                pattern.c_str(),
                &current_pf_list,
                options.test(PRODUCTION_FIND_SHOWBINDINGS),
                options.test(PRODUCTION_FIND_ONLY_CHUNKS),
                options.test(PRODUCTION_FIND_NO_CHUNKS));
    }
    if (options.test(PRODUCTION_FIND_INCLUDE_RHS))
    {
        /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
        /* Soar-Bugs #54 TMH */
        read_pattern_and_get_matching_productions (thisAgent,
                pattern.c_str(),
                &current_pf_list,
                options.test(PRODUCTION_FIND_SHOWBINDINGS),
                options.test(PRODUCTION_FIND_ONLY_CHUNKS),
                options.test(PRODUCTION_FIND_NO_CHUNKS));
    }
    if (current_pf_list == NIL)
    {
        print(thisAgent,  "No matches.\n");
    }

    free_list(thisAgent, current_pf_list);
    return true;
}

