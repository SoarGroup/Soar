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
#include "rhsfun.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseProductionFind(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'c', "chunks",			OPTARG_NONE},
		{'l', "lhs",				OPTARG_NONE},
		{'n', "nochunks",		OPTARG_NONE},
		{'r', "rhs",				OPTARG_NONE},
		{'s', "show-bindings",	OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	ProductionFindBitset options(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'c':
				options.set(PRODUCTION_FIND_ONLY_CHUNKS);
				options.reset(PRODUCTION_FIND_NO_CHUNKS);
				break;
			case 'l':
				options.set(PRODUCTION_FIND_INCLUDE_LHS);
				break;
			case 'n':
				options.set(PRODUCTION_FIND_NO_CHUNKS);
				options.reset(PRODUCTION_FIND_ONLY_CHUNKS);
				break;
			case 'r':
				options.set(PRODUCTION_FIND_INCLUDE_RHS);
				break;
			case 's':
				options.set(PRODUCTION_FIND_SHOWBINDINGS);
				break;
			default:
				return SetError(kGetOptError);
		}
	}

	if (!m_NonOptionArguments) {
		SetErrorDetail("Pattern required.");
		return SetError(kTooFewArgs);
	}

	if (options.none()) options.set(PRODUCTION_FIND_INCLUDE_LHS);

	std::string pattern;
	for (unsigned i = m_Argument - m_NonOptionArguments; i < argv.size(); ++i) {
		pattern += argv[i];
		pattern += ' ';
	}
	pattern = pattern.substr(0, pattern.length() - 1);

	return DoProductionFind(options, pattern);
}

void free_binding_list (agent* agnt, list *bindings) 
{
	cons *c;

	for (c=bindings;c!=NIL;c=c->rest)
		free_memory(agnt, c->first,MISCELLANEOUS_MEM_USAGE);
	free_list(agnt, bindings);
}

typedef struct binding_structure {
	Symbol *from, *to;
} Binding;


Symbol *get_binding (Symbol *f, list *bindings) 
{
	cons *c;

	for (c=bindings;c!=NIL;c=c->rest) 
	{
		if (static_cast<Binding *>(c->first)->from == f)
			return static_cast<Binding *>(c->first)->to;
	}
	return NIL;
}

bool symbols_are_equal_with_bindings (agent* agnt, Symbol *s1, Symbol *s2, list **bindings) 
{
	Binding *b;
	Symbol *bvar;

	/* SBH/MVP 7-5-94 */
	if ((s1 == s2) && (s1->common.symbol_type != VARIABLE_SYMBOL_TYPE))
		return TRUE;

	/* "*" matches everything. */
	if ((s1->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
		(!strcmp(s1->sc.name,"*"))) return TRUE;
	if ((s2->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
		(!strcmp(s2->sc.name,"*"))) return TRUE;


	if ((s1->common.symbol_type != VARIABLE_SYMBOL_TYPE) ||
		(s2->common.symbol_type != VARIABLE_SYMBOL_TYPE))
		return FALSE;
	/* Both are variables */
	bvar = get_binding(s1,*bindings);
	if (bvar == NIL) {
		b = static_cast<Binding *>(allocate_memory(agnt, sizeof(Binding),MISCELLANEOUS_MEM_USAGE));
		b->from = s1;
		b->to = s2;
		push(agnt, b,*bindings);
		return TRUE;
	}
	else if (bvar == s2) {
		return TRUE;
	}
	else return FALSE;
}

bool actions_are_equal_with_bindings (agent* agnt, action *a1, action *a2, list **bindings) 
{
	//         if (a1->type == FUNCALL_ACTION) 
	//         {
	//            if ((a2->type == FUNCALL_ACTION)) 
	//            {
	//               if (funcalls_match(rhs_value_to_funcall_list(a1->value),
	//                  rhs_value_to_funcall_list(a2->value))) 
	//               {
	//                     return TRUE;
	//               }
	//               else return FALSE;
	//            }
	//            else return FALSE;
	//         }
	if (a2->type == FUNCALL_ACTION) return FALSE;

	/* Both are make_actions. */

	if (a1->preference_type != a2->preference_type) return FALSE;

	if (!symbols_are_equal_with_bindings(agnt, rhs_value_to_symbol(a1->id),
		rhs_value_to_symbol(a2->id),
		bindings)) return FALSE;

	if ((rhs_value_is_symbol(a1->attr)) && (rhs_value_is_symbol(a2->attr))) 
	{
		if (!symbols_are_equal_with_bindings(agnt, rhs_value_to_symbol(a1->attr),
			rhs_value_to_symbol(a2->attr), bindings)) 
		{
			return FALSE;
		}
	} else {
		//            if ((rhs_value_is_funcall(a1->attr)) && (rhs_value_is_funcall(a2->attr))) 
		//            {
		//               if (!funcalls_match(rhs_value_to_funcall_list(a1->attr),
		//                  rhs_value_to_funcall_list(a2->attr)))
		//               {
		//                  return FALSE;
		//               }
		//            }
	}

	/* Values are different. They are rhs_value's. */

	if ((rhs_value_is_symbol(a1->value)) && (rhs_value_is_symbol(a2->value))) 
	{
		if (symbols_are_equal_with_bindings(agnt, rhs_value_to_symbol(a1->value),
			rhs_value_to_symbol(a2->value), bindings)) 
		{
			return TRUE;
		}
		else 
		{
			return FALSE;
		}
	}
	if ((rhs_value_is_funcall(a1->value)) && (rhs_value_is_funcall(a2->value))) 
	{
		//            if (funcalls_match(rhs_value_to_funcall_list(a1->value),
		//               rhs_value_to_funcall_list(a2->value)))
		//            {
		//               return TRUE;
		//            }
		//            else 
		{
			return FALSE;
		}
	}
	return FALSE;
}


#define dealloc_and_return(agnt,x,y) { deallocate_test(agnt, x) ; return (y) ; }

/* DJP 4/3/96 -- changed t2 to test2 in declaration */
bool tests_are_equal_with_bindings (agent* agnt, test t1, test test2, list **bindings) {
	cons *c1, *c2;
	complex_test *ct1, *ct2;
	Bool goal_test,impasse_test;

	/* DJP 4/3/96 -- The problem here is that sometimes test2 was being copied      */
	/*               and sometimes it wasn't.  If it was copied, the copy was never */
	/*               deallocated.  There's a few choices about how to fix this.  I  */
	/*               decided to just create a copy always and then always           */
	/*               deallocate it before returning.  Added a macro to do that.     */

	test t2;

	/* t1 is from the pattern given to "pf"; t2 is from a production's condition list. */
	if (test_is_blank_test(t1)) 
		return(test_is_blank_test(test2) == 0);

	/* If the pattern doesn't include "(state", but the test from the
	* production does, strip it out of the production's. 
	*/
	if ((!test_includes_goal_or_impasse_id_test(t1,TRUE,FALSE)) &&
		test_includes_goal_or_impasse_id_test(test2,TRUE,FALSE)) 
	{
		goal_test = FALSE;
		impasse_test = FALSE;
		t2 = copy_test_removing_goal_impasse_tests(agnt, test2, &goal_test, &impasse_test);
	}
	else
	{
		t2 = copy_test(agnt,test2) ; /* DJP 4/3/96 -- Always make t2 into a copy */
	}

	if (test_is_blank_or_equality_test(t1)) 
	{
		if (!(test_is_blank_or_equality_test(t2) && !(test_is_blank_test(t2))))
		{
			dealloc_and_return(agnt, t2,FALSE);
		}
		else 
		{
			if (symbols_are_equal_with_bindings(agnt, referent_of_equality_test(t1),
				referent_of_equality_test(t2),
				bindings))
			{
				dealloc_and_return(agnt, t2,TRUE);
			}
			else
			{
				dealloc_and_return(agnt, t2,FALSE);
			}
		}
	}

	ct1 = complex_test_from_test(t1);
	ct2 = complex_test_from_test(t2);

	if (ct1->type != ct2->type) 
	{
		dealloc_and_return(agnt, t2,FALSE);
	}

	switch(ct1->type) 
	{
	case GOAL_ID_TEST: 
		dealloc_and_return(agnt, t2,TRUE);
		break;
	case IMPASSE_ID_TEST: 
		dealloc_and_return(agnt, t2,TRUE);
		break;

	case DISJUNCTION_TEST:
		for (c1 = ct1->data.disjunction_list, c2=ct2->data.disjunction_list;
			((c1!=NIL)&&(c2!=NIL)); c1=c1->rest, c2=c2->rest)
		{
			if (c1->first != c2->first) 
			{
				dealloc_and_return(agnt, t2,FALSE)
			}
		}
		if (c1==c2) 
		{
			dealloc_and_return(agnt, t2,TRUE);  /* make sure they both hit end-of-list */
		}
		else
		{
			dealloc_and_return(agnt, t2,FALSE);
		}

	case CONJUNCTIVE_TEST:
		for (c1=ct1->data.conjunct_list, c2=ct2->data.conjunct_list;
			((c1!=NIL)&&(c2!=NIL)); c1=c1->rest, c2=c2->rest)
		{
			if (! tests_are_equal_with_bindings(agnt, static_cast<test>(c1->first), static_cast<test>(c2->first), bindings)) 
				dealloc_and_return(agnt, t2,FALSE)
		}
		if (c1==c2) 
		{
			dealloc_and_return(agnt, t2,TRUE);  /* make sure they both hit end-of-list */
		}
		else 
		{
			dealloc_and_return(agnt, t2,FALSE);
		}

	default:  /* relational tests other than equality */
		if (symbols_are_equal_with_bindings(agnt, ct1->data.referent,ct2->data.referent,bindings))
		{
			dealloc_and_return(agnt, t2,TRUE);
		}
		else
		{
			dealloc_and_return(agnt, t2,FALSE);
		}
	}
}

void print_binding_list (agent* agnt, list *bindings) 
{
	cons *c;

	for (c = bindings ; c != NIL ; c = c->rest)
	{
		print_with_symbols (agnt, "   (%y -> %y)\n", static_cast<Binding *>(c->first)->from, static_cast<Binding *>(c->first)->to);
	}
}

bool conditions_are_equal_with_bindings (agent* agnt, condition *c1, condition *c2, list **bindings) {
	if (c1->type != c2->type) return FALSE;
	switch (c1->type) 
	{
	case POSITIVE_CONDITION:
	case NEGATIVE_CONDITION:
		if (! tests_are_equal_with_bindings (agnt, c1->data.tests.id_test,
			c2->data.tests.id_test,bindings))
			return FALSE;
		if (! tests_are_equal_with_bindings (agnt, c1->data.tests.attr_test,
			c2->data.tests.attr_test,bindings))

			return FALSE;
		if (! tests_are_equal_with_bindings (agnt, c1->data.tests.value_test,
			c2->data.tests.value_test,bindings))
			return FALSE;
		if (c1->test_for_acceptable_preference != c2->test_for_acceptable_preference)
			return FALSE;
		return TRUE;

	case CONJUNCTIVE_NEGATION_CONDITION:
		for (c1=c1->data.ncc.top, c2=c2->data.ncc.top;
			((c1!=NIL)&&(c2!=NIL));
			c1=c1->next, c2=c2->next)
			if (! conditions_are_equal_with_bindings (agnt, c1,c2,bindings)) return FALSE;
		if (c1==c2) return TRUE;  /* make sure they both hit end-of-list */
		return FALSE;
	}
	return FALSE; /* unreachable, but without it, gcc -Wall warns here */
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
				p_node_to_conditions_and_nots (agnt, prod->p_node, NIL, NIL, &top, &bottom,
					NIL, NIL);

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

				p_node_to_conditions_and_nots (agnt, prod->p_node, NIL, NIL, &top_cond,
					&bottom_cond, NIL, &rhs);
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

	if (options.test(PRODUCTION_FIND_INCLUDE_LHS)) 
	{
		/* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
		/* Soar-Bugs #54 TMH */
		m_pAgentSoar->alternate_input_string = pattern.c_str();
		m_pAgentSoar->alternate_input_suffix = ") ";

		get_lexeme(m_pAgentSoar);
		read_pattern_and_get_matching_productions (m_pAgentSoar, 
			&current_pf_list,
			options.test(PRODUCTION_FIND_SHOWBINDINGS),
			options.test(PRODUCTION_FIND_ONLY_CHUNKS), 
			options.test(PRODUCTION_FIND_NO_CHUNKS));
		m_pAgentSoar->current_char = ' ';
	}
	if (options.test(PRODUCTION_FIND_INCLUDE_RHS))
	{
		/* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
		/* Soar-Bugs #54 TMH */
		m_pAgentSoar->alternate_input_string = pattern.c_str();
		m_pAgentSoar->alternate_input_suffix = ") ";

		get_lexeme(m_pAgentSoar);
		read_rhs_pattern_and_get_matching_productions (m_pAgentSoar, &current_pf_list,
			options.test(PRODUCTION_FIND_SHOWBINDINGS),
			options.test(PRODUCTION_FIND_ONLY_CHUNKS), 
			options.test(PRODUCTION_FIND_NO_CHUNKS));
		m_pAgentSoar->current_char = ' ';
	}
	if (current_pf_list == NIL) 
	{
		print(m_pAgentSoar, "No matches.\n");
	}

	free_list(m_pAgentSoar, current_pf_list);
	return true;
}

