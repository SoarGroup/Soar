/////////////////////////////////////////////////////////////////
// production-find command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

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

void free_binding_list (agent* agnt, list *bindings)
{
    cons *c;

    for (c=bindings;c!=NIL;c=c->rest)
        free_memory(agnt, c->first,MISCELLANEOUS_MEM_USAGE);
    free_list(agnt, bindings);
}

void print_binding_list (agent* agnt, list *bindings)
{
    cons *c;

    for (c = bindings ; c != NIL ; c = c->rest)
    {
        print_with_symbols (agnt, "   (%y -> %y)\n", static_cast<Binding *>(c->first)->from, static_cast<Binding *>(c->first)->to);
    }
}

void reset_old_binding_point(agent* agnt, list **bindings, list **current_binding_point)
{
    cons *c,*c_next;

    c = *bindings;
    while (c != *current_binding_point) {
        c_next = c->rest;
        free_memory(agnt, c->first,MISCELLANEOUS_MEM_USAGE);
        free_cons (agnt, c);
        c = c_next;
    }

    bindings = current_binding_point;
}

void read_pattern_and_get_matching_productions (agent* agnt,
    list **current_pf_list,
    bool show_bindings,
    bool just_chunks,
    bool no_chunks)
{
    condition *c, *clist, *top, *bottom, *pc;
    int i;
    production *prod;
    list *bindings, *current_binding_point;
    bool match, match_this_c;


    bindings = NIL;
    current_binding_point = NIL;

    /*  print("Parsing as a lhs...\n"); */
    clist = parse_lhs(agnt);
    if (!clist) {
        print(agnt, "Error: not a valid condition list.\n");
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

    for (i=0; i<NUM_PRODUCTION_TYPES; i++)
        if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) ||
            (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
            for (prod=agnt->all_productions_of_type[i]; prod!=NIL; prod=prod->next) {

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
                /* Debug | See if this works with last param true (add complex conditions) */
                p_node_to_conditions(agnt, prod->p_node, NIL, NIL, &top, &bottom,
                    NIL, false);

                free_binding_list(agnt, bindings);
                bindings = NIL;

                for (c=clist;c!=NIL;c=c->next) {
                    match_this_c= FALSE;
                    current_binding_point = bindings;

                    for (pc = top; pc != NIL; pc=pc->next) {
                        if (conditions_are_equal_with_bindings(agnt, c,pc,&bindings)) {
                            match_this_c = TRUE;
                            break;}
                        else {
                            /* Remove new, incorrect bindings. */
                            reset_old_binding_point(agnt, &bindings,&current_binding_point);
                            bindings= current_binding_point;
                        }
                    }
                    if (!match_this_c) {match = FALSE; break;}
                }
                deallocate_condition_list (agnt, top); /* DJP 4/3/96 -- Never dealloced */
                if (match) {
                    push(agnt, prod,(*current_pf_list));
                    if (show_bindings) {
                        print_with_symbols(agnt, "%y, with bindings:\n",prod->name);
                        print_binding_list(agnt, bindings);}
                    else
                        print_with_symbols(agnt, "%y\n",prod->name);
                }
            }
            if (bindings) free_binding_list(agnt, bindings); /* DJP 4/3/96 -- To catch the last production */
}

void read_rhs_pattern_and_get_matching_productions (agent* agnt,
    list **current_pf_list,
    bool show_bindings,
    bool just_chunks,
    bool no_chunks)
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
    parsed_ok = (parse_rhs(agnt, &alist) == TRUE);
    if (!parsed_ok) {
        print(agnt, "Error: not a valid rhs.\n");
        current_pf_list = NIL;
        return;
    }

    /*
    print("Valid RHS:\n");
    print_action_list(alist,0,FALSE);
    print("\nMatches:\n");
    */

    for (i=0; i<NUM_PRODUCTION_TYPES; i++)
    {
        if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) || (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
        {
            for (prod=agnt->all_productions_of_type[i]; prod!=NIL; prod=prod->next)
            {
                match = TRUE;

                free_binding_list(agnt, bindings);
                bindings = NIL;

                /* Debug | See if this works with last param true (add complex conditions) */
                p_node_to_conditions(agnt, prod->p_node, NIL, NIL, &top_cond,
                    &bottom_cond, &rhs, false);
                deallocate_condition_list (agnt, top_cond);
                for (a=alist;a!=NIL;a=a->next)
                {
                    match_this_a= FALSE;
                    current_binding_point = bindings;

                    for (pa = rhs; pa != NIL; pa=pa->next)
                    {
                        if (actions_are_equal_with_bindings(agnt, a,pa,&bindings))
                        {
                            match_this_a = TRUE;
                            break;
                        }
                        else
                        {
                            /* Remove new, incorrect bindings. */
                            reset_old_binding_point(agnt, &bindings,&current_binding_point);
                            bindings= current_binding_point;
                        }
                    }
                    if (!match_this_a)
                    {
                        match = FALSE;
                        break;
                    }
                }

                deallocate_action_list (agnt, rhs);
                if (match)
                {
                    push(agnt,prod,(*current_pf_list));
                    if (show_bindings)
                    {
                        print_with_symbols(agnt, "%y, with bindings:\n",prod->name);
                        print_binding_list(agnt,bindings);
                    }
                    else
                    {
                        print_with_symbols(agnt,"%y\n",prod->name);
                    }
                }
            }
        }
    }
    if (bindings)
    {
        free_binding_list(agnt, bindings); /* DJP 4/3/96 -- To catch the last production */
    }
}

bool CommandLineInterface::DoProductionFind(const ProductionFindBitset& options, const std::string& pattern) {
    list* current_pf_list = 0;
    agent* agnt = m_pAgentSML->GetSoarAgent();

    if (options.test(PRODUCTION_FIND_INCLUDE_LHS))
    {
        /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
        /* Soar-Bugs #54 TMH */
        agnt->alternate_input_string = pattern.c_str();
        agnt->alternate_input_suffix = ") ";

        get_lexeme(agnt);
        read_pattern_and_get_matching_productions (agnt,
            &current_pf_list,
            options.test(PRODUCTION_FIND_SHOWBINDINGS),
            options.test(PRODUCTION_FIND_ONLY_CHUNKS),
            options.test(PRODUCTION_FIND_NO_CHUNKS));
        agnt->current_char = ' ';
    }
    if (options.test(PRODUCTION_FIND_INCLUDE_RHS))
    {
        /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
        /* Soar-Bugs #54 TMH */
        agnt->alternate_input_string = pattern.c_str();
        agnt->alternate_input_suffix = ") ";

        get_lexeme(agnt);
        read_rhs_pattern_and_get_matching_productions (agnt, &current_pf_list,
            options.test(PRODUCTION_FIND_SHOWBINDINGS),
            options.test(PRODUCTION_FIND_ONLY_CHUNKS),
            options.test(PRODUCTION_FIND_NO_CHUNKS));
        agnt->current_char = ' ';
    }
    if (current_pf_list == NIL)
    {
        print(agnt, "No matches.\n");
    }

    free_list(agnt, current_pf_list);
    return true;
}

