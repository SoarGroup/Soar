#include <portability.h>

/*************************************************************************
* PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
* FOR LICENSE AND COPYRIGHT INFORMATION. 
*************************************************************************/

/********************************************************************
* @file gski_donottouch.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/

#include "sml_KernelHelpers.h"

#include "sml_Utils.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "XMLTrace.h"

#include "KernelHeaders.h"

#include "gdatastructs.h"
#include "rete.h"
#include "trace.h"
#include "parser.h"
#include "rhsfun.h"
#include "explain.h"
#include "soar_rand.h"
#include "xml.h"
#include "soar_TraceNames.h"
#include "utilities.h"
#include "tempmem.h"


using namespace sml ;
using namespace soarxml ;
using namespace soar_TraceNames ;


namespace sml 
{

typedef struct wme_filter_struct {
	Symbol *id;
	Symbol *attr;
	Symbol *value;
	bool adds;
	bool removes;
} wme_filter;

void KernelHelpers::PrintStackTrace(AgentSML* agent, bool print_states, bool print_operators)
{
	Symbol *g;

	// We don't want to keep printing forever (in case we're in a state no change cascade).
	const int maxStates = 500 ;
	int stateCount = 0 ;

	for (g = agent->GetSoarAgent()->top_goal; g != NIL; g = g->id.lower_goal) 
	{
		stateCount++ ;

		if (stateCount > maxStates)
			continue ;

		if (print_states)
		{
			print_stack_trace (agent->GetSoarAgent(),g, g, FOR_STATES_TF, false);
			print (agent->GetSoarAgent(), "\n");
		}
		if (print_operators && g->id.operator_slot->wmes) 
		{
			print_stack_trace (agent->GetSoarAgent(), g->id.operator_slot->wmes->value,
				g, FOR_OPERATORS_TF, false);
			print (agent->GetSoarAgent(), "\n");
		}
	}

	if (stateCount > maxStates)
	{
		print (agent->GetSoarAgent(), "...Stack goes on for another %d states\n", stateCount - maxStates);
	}
}

void do_print_for_production (agent* agnt, 
	production *prod, 
	bool internal, 
	bool print_filename, 
	bool full_prod) 
{
	if (!full_prod) 
	{
		print_with_symbols(agnt, "%y  ",prod->name);

		if ( prod->rl_rule )
		{
			print_with_symbols( agnt, "%y  ", make_float_constant( agnt, prod->rl_update_count ) );
			print_with_symbols( agnt, "%y", rhs_value_to_symbol( prod->action_list->referent ) );
		}
	}
	if (print_filename) 
	{
		print_string(agnt, "# sourcefile : ");
		if (prod->filename) 
		{
			print_string(agnt, prod->filename);
		} 
		else 
		{
			print_string(agnt, " _unknown_ ");
		}
	}
	print(agnt, "\n");
	if (full_prod) 
	{
		print_production (agnt, prod, internal);
		print(agnt, "\n");
	}
}

// compare_attr is used for cstdlib::qsort below. 
int compare_attr (const void * e1, const void * e2)
{
	wme **p1, **p2;

	char s1[MAX_LEXEME_LENGTH*2+20], s2[MAX_LEXEME_LENGTH*2+20];

	p1 = (wme **) e1;
	p2 = (wme **) e2;

	// passing null thisAgent is OK as long as dest is guaranteed != 0
	symbol_to_string (0, (*p1)->attr, TRUE, s1, MAX_LEXEME_LENGTH*2+20);
	symbol_to_string (0, (*p2)->attr, TRUE, s2, MAX_LEXEME_LENGTH*2+20);

	return strcmp (s1, s2);
}

/* This should probably be in the Soar kernel interface. */
#define NEATLY_PRINT_BUF_SIZE 10000
void neatly_print_wme_augmentation_of_id (agent* thisAgent, wme *w, int indentation) {
	char buf[NEATLY_PRINT_BUF_SIZE], *ch;

	xml_object( thisAgent, w );

	strcpy (buf, " ^");
	ch = buf;
	while (*ch) ch++;
	symbol_to_string (thisAgent, w->attr, TRUE, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf)); while (*ch) ch++;
	*(ch++) = ' ';
	symbol_to_string (thisAgent, w->value, TRUE, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf)); while (*ch) ch++;
	if (w->acceptable) { strcpy (ch, " +"); while (*ch) ch++; }

	if (get_printer_output_column(thisAgent) + (ch - buf) >= 80) {
		print (thisAgent, "\n");
		print_spaces (thisAgent, indentation+6);
	}
	print_string (thisAgent, buf);
}

/* RPM 4/07 bug 988
	This function traverses the ids we are going to print and marks each with its shallowest depth
	That is, if an id can be reached by multiple paths, this will find the shortest one and save
		the depth of that path on the id.  Thus, when we print, the wmes will be indented properly,
		making it much easier to read, and avoiding bugs (see bug 988).
*/
void mark_depths_augs_of_id (agent* agnt,
	Symbol *id, 
	int depth,
	tc_number tc) 
{
	slot *s;
	wme *w;

	/* AGR 652  The plan is to go through the list of WMEs and find out how
	many there are.  Then we malloc an array of that many pointers.
	Then we go through the list again and copy all the pointers to that array.
	Then we qsort the array and print it out.  94.12.13 */

	if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) return;
	if (id->id.tc_num==tc && id->id.depth >= depth) return;  // this has already been printed at an equal-or-lower depth, RPM 4/07 bug 988
	
	id->id.depth = depth; // set the depth of this id
	id->id.tc_num = tc;

	/* --- if depth<=1, we're done --- */
	if (depth<=1) return;

	/* --- call this routine recursively --- */
	for (w=id->id.input_wmes; w!=NIL; w=w->next) {
		mark_depths_augs_of_id (agnt, w->attr, depth-1, tc);
		mark_depths_augs_of_id (agnt, w->value, depth-1, tc);
	}
	for (w=id->id.impasse_wmes; w!=NIL; w=w->next) {
		mark_depths_augs_of_id (agnt, w->attr, depth-1, tc);
		mark_depths_augs_of_id (agnt, w->value, depth-1, tc);
	}
	for (s=id->id.slots; s!=NIL; s=s->next) {
		for (w=s->wmes; w!=NIL; w=w->next) {
			mark_depths_augs_of_id (agnt, w->attr, depth-1, tc);
			mark_depths_augs_of_id (agnt, w->value, depth-1, tc);
		}
		for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) {
			mark_depths_augs_of_id (agnt, w->attr, depth-1, tc);
			mark_depths_augs_of_id (agnt, w->value, depth-1, tc);
		}
	}
}

// RPM 4/07: Note, mark_depths_augs_of_id must be called before the root call to print_augs_of_id
//           Thus, this should probably only be called from do_print_for_identifier
void print_augs_of_id (agent* agnt,
	Symbol *id, 
	int depth,
	int maxdepth,
	bool internal,
	bool tree,
	tc_number tc) 
{
	slot *s;
	wme *w;

	wme **list;    /* array of WME pointers, AGR 652 */
	int num_attr;  /* number of attributes, AGR 652 */
	int attr;      /* attribute index, AGR 652 */

	/* AGR 652  The plan is to go through the list of WMEs and find out how
	many there are.  Then we malloc an array of that many pointers.
	Then we go through the list again and copy all the pointers to that array.
	Then we qsort the array and print it out.  94.12.13 */

	if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) return;
	if (id->id.tc_num==tc) return;  // this has already been printed, so return RPM 4/07 bug 988
	if (id->id.depth > depth) return;  // this can be reached via an equal or shorter path, so return without printing RPM 4/07 bug 988

	// if we're here, then we haven't printed this id yet, so print it

	depth = id->id.depth; // set the depth to the depth via the shallowest path, RPM 4/07 bug 988
	int indent = (maxdepth-depth)*2; // set the indent based on how deep we are, RPM 4/07 bug 988

	id->id.tc_num = tc;  // mark id as printed

	/* --- first, count all direct augmentations of this id --- */
	num_attr = 0;
	for (w=id->id.impasse_wmes; w!=NIL; w=w->next) num_attr++;
	for (w=id->id.input_wmes; w!=NIL; w=w->next) num_attr++;
	for (s=id->id.slots; s!=NIL; s=s->next) {
		for (w=s->wmes; w!=NIL; w=w->next) num_attr++;
		for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) num_attr++;
	}

	/* --- next, construct the array of wme pointers and sort them --- */
	list = (wme**)allocate_memory(agnt, num_attr*sizeof(wme *), MISCELLANEOUS_MEM_USAGE);
	attr = 0;
	for (w=id->id.impasse_wmes; w!=NIL; w=w->next)
		list[attr++] = w;
	for (w=id->id.input_wmes; w!=NIL; w=w->next)
		list[attr++] = w;
	for (s=id->id.slots; s!=NIL; s=s->next) {
		for (w=s->wmes; w!=NIL; w=w->next)
			list[attr++] = w;
		for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next)
			list[attr++] = w;
	}
	qsort (list, num_attr, sizeof (wme *), compare_attr); 


	/* --- finally, print the sorted wmes and deallocate the array --- */

	// RPM 4/07 If this is a tree print, then for each wme in the list, print it and its children
	if(tree) {
		for (attr=0; attr < num_attr; attr++) {
			w = list[attr];
			print_spaces (agnt, indent);
			if (internal) print_wme (agnt, w);
			else print_wme_without_timetag (agnt, w);

			if (depth>1) { // we're not done yet
				/* --- call this routine recursively --- */
				print_augs_of_id (agnt, w->attr, depth-1, maxdepth, internal, tree, tc);
				print_augs_of_id (agnt, w->value, depth-1, maxdepth, internal, tree, tc);
			}
		}
	// RPM 4/07 This is not a tree print, so for each wme in the list, print it
	// Then, after all wmes have been printed, print the children
	} else {
		for (attr=0; attr < num_attr; attr++) {
			w = list[attr];
			print_spaces (agnt, indent);

			if(internal) {
				print_wme (agnt, w);
			} else {
				print_with_symbols (agnt, "(%y", id);

				// XML format of an <id> followed by a series of <wmes> each of which shares the original ID.
				// <id id="s1"><wme tag="123" attr="foo" attrtype="string" val="123" valtype="string"></wme><wme attr="bar" ...></wme></id>
				xml_begin_tag(agnt, kWME_Id);
				xml_att_val(agnt, kWME_Id, id);

				for (attr=0; attr < num_attr; attr++) {
					w = list[attr];
					neatly_print_wme_augmentation_of_id (agnt, w, indent);
				}
				
				xml_end_tag(agnt, kWME_Id);

				print (agnt, ")\n");
			}
		}

		// If there is still depth left, recurse
		if (depth > 1) {
			for (attr=0; attr < num_attr; attr++) {
				w = list[attr];
				/* --- call this routine recursively --- */
				print_augs_of_id (agnt, w->attr, depth-1, maxdepth, internal, tree, tc);
				print_augs_of_id (agnt, w->value, depth-1, maxdepth, internal, tree, tc);
			}
		}
	}

	// deallocate the array
	free_memory(agnt, list, MISCELLANEOUS_MEM_USAGE);
}

void do_print_for_identifier (agent* agnt, Symbol *id, int depth, bool internal, bool tree) {
	tc_number tc;

	// RPM 4/07: first mark the nodes with their shallowest depth
	//           then print them at their shallowest depth
	tc = get_new_tc_number(agnt);
	mark_depths_augs_of_id (agnt, id, depth, tc);
	tc = get_new_tc_number(agnt);
	print_augs_of_id (agnt, id, depth, depth, internal, tree, tc);
}

void do_print_for_production_name (agent* agnt,
	char *prod_name, 
	bool internal,
	bool print_filename, 
	bool full_prod) 
{
	Symbol *sym;

	sym = find_sym_constant (agnt, agnt->lexeme.string);
	if (sym && sym->sc.production) {
		/* kjh CUSP(B11) begin */  /* also Soar-Bugs #161 */
		if (print_filename) {
			if (full_prod) 
				print_string(agnt, "# sourcefile : ");
			print (agnt, "%s\n", sym->sc.production->filename);
		}
		/* KJC added so get at least some output for any named productions */
		if ((full_prod) || (!print_filename)) {
			print_production (agnt, sym->sc.production, internal);
			print (agnt, "\n");
		} /* end CUSP B11 kjh */
	} else {
		print (agnt, "No production named %s\n", prod_name);
		print_location_of_most_recent_lexeme(agnt);
	}
}

void do_print_for_wme (agent* agnt, wme *w, int depth, bool internal, bool tree) {
	if (internal && (depth==0)) {
		print_wme (agnt, w);
		print (agnt, "\n");
	} else {
		do_print_for_identifier(agnt, w->id, depth, internal, tree);
	}
}

/* --- Read and consume one pattern element.  Return 0 if error, 1 if "*",
otherwise return 2 and set dest_sym to find_symbol() result. --- */
int read_pattern_component (agent* agnt, Symbol **dest_sym) 
{
	if (strcmp(agnt->lexeme.string,"*") == 0) return 1;
	switch (agnt->lexeme.type) 
	{
	case SYM_CONSTANT_LEXEME:
		*dest_sym = find_sym_constant (agnt, agnt->lexeme.string); return 2;
	case INT_CONSTANT_LEXEME:
		*dest_sym = find_int_constant (agnt, agnt->lexeme.int_val); return 2;
	case FLOAT_CONSTANT_LEXEME:
		*dest_sym = find_float_constant (agnt, agnt->lexeme.float_val); return 2;
	case IDENTIFIER_LEXEME:
		*dest_sym = find_identifier (agnt, agnt->lexeme.id_letter, agnt->lexeme.id_number); return 2;
	case VARIABLE_LEXEME:
		*dest_sym = read_identifier_or_context_variable(agnt);
		if (*dest_sym) return 2;
		return 0;
	default:
		print (agnt, "Expected identifier or constant in wme pattern\n");
		print_location_of_most_recent_lexeme(agnt);
		return 0;
	}
}



list *read_pattern_and_get_matching_wmes (agent* agnt) 
{
	int parentheses_level;
	list *wmes;
	wme *w;
	Symbol *id, *attr, *value;
	int id_result, attr_result, value_result;
	bool acceptable;

	if (agnt->lexeme.type!=L_PAREN_LEXEME) {
		print (agnt, "Expected '(' to begin wme pattern not string '%s' or char '%c'\n", 
			agnt->lexeme.string, agnt->current_char);
		print_location_of_most_recent_lexeme(agnt);
		return NIL;
	}
	parentheses_level = current_lexer_parentheses_level(agnt);

	get_lexeme(agnt);
	id_result = read_pattern_component (agnt, &id);
	if (! id_result) {
		skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
		return NIL;
	}
	get_lexeme(agnt);
	if (agnt->lexeme.type!=UP_ARROW_LEXEME) {
		print (agnt, "Expected ^ in wme pattern\n");
		print_location_of_most_recent_lexeme(agnt);
		skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
		return NIL;
	}
	get_lexeme(agnt);
	attr_result = read_pattern_component (agnt, &attr);
	if (! attr_result) {
		skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
		return NIL;
	}
	get_lexeme(agnt);
	value_result = read_pattern_component (agnt, &value);
	if (! value_result) {
		skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
		return NIL;
	}
	get_lexeme(agnt);
	if (agnt->lexeme.type==PLUS_LEXEME) {
		acceptable = TRUE;
		get_lexeme(agnt);
	} else {
		acceptable = FALSE;
	}
	if (agnt->lexeme.type!=R_PAREN_LEXEME) {
		print (agnt, "Expected ')' to end wme pattern\n");
		print_location_of_most_recent_lexeme(agnt);
		skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
		return NIL;
	}

	wmes = NIL;
	for (w=agnt->all_wmes_in_rete; w!=NIL; w=w->rete_next) {
		if ((id_result==1) || (id==w->id))
			if ((attr_result==1) || (attr==w->attr))
				if ((value_result==1) || (value==w->value))
					if (acceptable == (w->acceptable == TRUE))
						push (agnt, w, wmes);
	}
	return wmes;  
}


void  soar_alternate_input(agent *ai_agent,
	const char  *ai_string, 
	const char  *ai_suffix, 
	bool   ai_exit   )
{
	ai_agent->alternate_input_string = ai_string;
	ai_agent->alternate_input_suffix = ai_suffix;
	ai_agent->current_char = ' ';
	ai_agent->alternate_input_exit = ai_exit;
	return;
}


void KernelHelpers::PrintSymbol(AgentSML* thisAgent,
	char*       arg, 
	bool        name_only, 
	bool        print_filename, 
	bool        internal,
	bool		tree,
	bool        full_prod,
	int         depth,
	bool		exact)
{
	cons *c;
	Symbol *id;
	bool output_arg;
	wme* w;
	list* wmes;

	agent* agnt = thisAgent->GetSoarAgent();

	get_lexeme_from_string(agnt, arg);

	switch (agnt->lexeme.type) 
	{
	case SYM_CONSTANT_LEXEME:
		output_arg = true; /* Soar-Bugs #161 */
		if (!name_only || print_filename) 
		{
			/* kjh CUSP(B11) begin */
			do_print_for_production_name (agnt, arg, internal, 
				print_filename, full_prod);
		} else 
		{
			print(agnt, "%s\n",arg);
		}
		break;

	case INT_CONSTANT_LEXEME:
		output_arg = true; /* Soar-Bugs #161 */
		for (w=thisAgent->GetSoarAgent()->all_wmes_in_rete; w!=NIL; w=w->rete_next)
			// RDF (08282002) Added the following cast to get rid of warning
			// message
			if (w->timetag == static_cast<unsigned long>(thisAgent->GetSoarAgent()->lexeme.int_val))
				break;
		if (w) 
		{
			do_print_for_wme (agnt, w, depth, internal, tree);
		} else 
		{
			print(agnt, "No wme %ld in working memory.", agnt->lexeme.int_val);
			return;
		}
		break;

	case IDENTIFIER_LEXEME:
	case VARIABLE_LEXEME:
		output_arg = true; /* Soar-Bugs #161 */
		id = read_identifier_or_context_variable(agnt);
		if (id) 
			do_print_for_identifier (agnt, id, depth, internal, tree);
		break;

	case QUOTED_STRING_LEXEME:
		output_arg = true; /* Soar-Bugs #161 */
		/* Soar-Bugs #54 TMH */
		soar_alternate_input(agnt, arg, ") ",true);
		/* ((agent *)clientData)->alternate_input_string = argv[next_arg];
		* ((agent *)clientData)->alternate_input_suffix = ") ";
		*/
		get_lexeme(agnt);
		wmes = read_pattern_and_get_matching_wmes (agnt);
		soar_alternate_input(agnt, NIL, NIL, FALSE); 
		agnt->current_char = ' ';
		if (exact)
		{
			// When printing exact, we want to list only those wmes who match.
			// Group up the wmes in objects (id ^attr value ^attr value ...)
			std::map< Symbol*, std::list< wme* > > objects;
			for (c = wmes; c != NIL; c = c->rest) {
				wme* current = static_cast<wme *>(c->first);
				objects[current->id].push_back(current);
			}
			// Loop through objects and print its wmes
			std::map< Symbol*, std::list<wme*> >::iterator iter = objects.begin();
			while ( iter != objects.end() ) 
			{
				std::list<wme*> wmelist = iter->second;
				std::list<wme*>::iterator wmeiter = wmelist.begin();

				// If we're printing internally, we just print the wme
				// taken from print_wme_without_timetag
				if (!internal) {
					print_with_symbols(agnt, "(%y", iter->first);
				}

				while ( wmeiter != wmelist.end() ) {
					wme* w = *wmeiter;
					if (internal) {
						// This does everything for us in the internal case, including xml
						print_wme(agnt, w);
					} else {
						// taken from print_wme_without_timetag
						print_with_symbols (agnt, " ^%y %y", w->attr, w->value);
						if (w->acceptable) {
							print_string (agnt, " +");
						}
						
						// this handles xml case for the wme
						xml_object( agnt, w, XML_WME_NO_TIMETAG );
					}
					++wmeiter;
				}

				if (!internal) {
					print_string(agnt, ")\n");
				}
				++iter;
			}
		} 
		else {
			for (c = wmes; c != NIL; c = c->rest) {
				do_print_for_wme (agnt, static_cast<wme *>(c->first), depth, internal, tree);
			}
		}
		free_list (agnt, wmes);
		break;

	default:
		//            sprintf(interp->result,
		//               "Unrecognized argument to %s command: %s",
		//               print, arg);
		//            return TCL_ERROR;
		return;
	} /* end of switch statement */
	output_arg = true;  /* Soar-bugs #161 */
}

void KernelHelpers::PrintUser(AgentSML*       thisAgent,
	char*         /*arg*/,
	bool          internal,
	bool          print_filename,
	bool          full_prod,
	unsigned int  productionType)
{
	//bool output_arg = true; /* TEST for Soar-Bugs #161 */
	for (production* prod=thisAgent->GetSoarAgent()->all_productions_of_type[productionType];  
		prod != NIL; prod = prod->next)
	{
		/* CUSP B11 kjh */
		do_print_for_production(thisAgent->GetSoarAgent(), prod,internal,
			print_filename,full_prod);
	}
}




bool string_match_up_to (char * string1, 
	const char * string2, 
	size_t positions)
{
	size_t i,num;

	/*  what we really want is to require a match over the length of
	the shorter of the two strings, with positions being a minimum */

	num = strlen(string1);
	if (num > strlen(string2)) num = strlen(string2);
	if (positions < num)  positions = num;

	for (i = 0; i < positions; i++)
	{
		if (string1[i] != string2[i])
			return false;
	}

	return true;  
}


bool KernelHelpers::GDSPrint(AgentSML* thisAgent)
{
	agent* agnt = thisAgent->GetSoarAgent();

	wme *w;
	Symbol *goal;


	print(agnt, "********************* Current GDS **************************\n");
	print(agnt, "stepping thru all wmes in rete, looking for any that are in a gds...\n");
	for (w=agnt->all_wmes_in_rete; w!=NIL; w=w->rete_next) 
	{
		if (w->gds)
		{
			if (w->gds->goal) 
			{
				print_with_symbols (agnt, "  For Goal  %y  ", w->gds->goal);
			} 
			else 
			{
				print(agnt, "  Old GDS value ");
			}
			print (agnt, "(%lu: ", w->timetag);
			print_with_symbols (agnt, "%y ^%y %y", w->id, w->attr, w->value);
			if (w->acceptable) 
			{
				print_string (agnt, " +");
			}
			print_string (agnt, ")");
			print (agnt, "\n");
		}
	}
	print(agnt, "************************************************************\n");
	for (goal=agnt->top_goal; goal!=NIL; goal=goal->id.lower_goal)
	{
		print_with_symbols (agnt, "  For Goal  %y  ", goal);
		if (goal->id.gds){
			/* Loop over all the WMEs in the GDS */
			print (agnt, "\n");
			for (w=goal->id.gds->wmes_in_gds; w!=NIL; w=w->gds_next)
			{
				print (agnt, "                (%lu: ", w->timetag);
				print_with_symbols (agnt, "%y ^%y %y", w->id, w->attr, w->value);
				if (w->acceptable) 
				{
					print_string (agnt, " +");
				}
				print_string (agnt, ")");
				print (agnt, "\n");
			}

		} 
		else 
		{
			print(agnt, ": No GDS for this goal.\n");
		}
	}

	print(agnt, "************************************************************\n");
	return true;
}////

void KernelHelpers::GetForceLearnStates(AgentSML* pAgent, std::stringstream& res) {
	agent* pSoarAgent = pAgent->GetSoarAgent();

	cons *c;
	char buff[1024];

	for (c = pSoarAgent->chunky_problem_spaces; c != NIL; c = c->rest) {
		symbol_to_string(pSoarAgent, static_cast<Symbol *>(c->first), TRUE, buff, 1024);
		res << buff;
	}
}

void KernelHelpers::GetDontLearnStates(AgentSML* pAgent, std::stringstream& res) {
	agent* pSoarAgent = pAgent->GetSoarAgent();

	cons *c;
	char buff[1024];

	for (c = pSoarAgent->chunk_free_problem_spaces; c != NIL; c = c->rest) {
		symbol_to_string(pSoarAgent, static_cast<Symbol *>(c->first), TRUE, buff, 1024);
		res << buff;
	}
}

void KernelHelpers::SetVerbosity(AgentSML* pAgent, bool setting) {
	agent* pSoarAgent = pAgent->GetSoarAgent();

	pSoarAgent->soar_verbose_flag = setting;

}

bool KernelHelpers::GetVerbosity(AgentSML* pAgent) {
	agent* pSoarAgent = pAgent->GetSoarAgent();

	return pSoarAgent->soar_verbose_flag ? true : false;
}

bool KernelHelpers::BeginTracingProduction(AgentSML* pAgent, const char* pProductionName) {
	agent* pSoarAgent = pAgent->GetSoarAgent();

	Symbol *sym;
	sym = find_sym_constant(pSoarAgent, pProductionName);

	if (!sym || !(sym->sc.production))
		return false;

	add_pwatch(pSoarAgent, sym->sc.production);
	return true;
}

bool KernelHelpers::StopTracingProduction(AgentSML* pAgent, const char* pProductionName) {
	agent* pSoarAgent = pAgent->GetSoarAgent();

	Symbol *sym;
	sym = find_sym_constant(pSoarAgent, pProductionName);

	if (!sym || !(sym->sc.production))
		return false;

	remove_pwatch(pSoarAgent, sym->sc.production);
	return true;
}

int RemoveWme(agent* pSoarAgent, wme* pWme)
{
	//	wme *w, *w2;
	//	Symbol *id;
	//	slot *s;

	//	w = (wme *) the_wme;

	Symbol* pId = pWme->id;

	// remove w from whatever list of wmes it's on
	wme* pWme2;
	for (pWme2 = pId->id.input_wmes; pWme2 != NIL; pWme2 = pWme2->next)
		if (pWme == pWme2)
			break;

	if (pWme2) remove_from_dll(pId->id.input_wmes, pWme, next, prev);

	for (pWme2 = pId->id.impasse_wmes; pWme2 != NIL; pWme2 = pWme2->next)
		if (pWme == pWme2)
			break;

	if (pWme2) remove_from_dll(pId->id.impasse_wmes, pWme, next, prev);

	slot* s;
	for (s = pId->id.slots; s != NIL; s = s->next) {

		for (pWme2 = s->wmes; pWme2 != NIL; pWme2 = pWme2->next)
			if (pWme == pWme2)
				break;

		if (pWme2)
			remove_from_dll(s->wmes, pWme, next, prev);

		for (pWme2 = s->acceptable_preference_wmes; pWme2 != NIL; pWme2 = pWme2->next)
			if (pWme == pWme2)
				break;

		if (pWme2)
			remove_from_dll(s->acceptable_preference_wmes, pWme, next, prev);
	}

#ifdef USE_CAPTURE_REPLAY
	// TODO: ommitted
#endif // USE_CAPTURE_REPLAY

	/* REW: begin 09.15.96 */
	if (pWme->gds) {
		if (pWme->gds->goal != NIL) {
			if (pSoarAgent->soar_verbose_flag || pSoarAgent->sysparams[TRACE_WM_CHANGES_SYSPARAM])
				{
					print(pSoarAgent, "\nremove_input_wme: Removing state S%d because element in GDS changed.", pWme->gds->goal->id.level);
					print(pSoarAgent, " WME: "); 

					char buf[256];
					SNPRINTF(buf, 254, "remove_input_wme: Removing state S%d because element in GDS changed.", pWme->gds->goal->id.level);
					xml_begin_tag(pSoarAgent, kTagVerbose);
					xml_att_val(pSoarAgent, kTypeString, buf);
					print_wme(pSoarAgent, pWme);
					xml_end_tag(pSoarAgent, kTagVerbose);
				}

			gds_invalid_so_remove_goal(pSoarAgent, pWme);
			/* NOTE: the call to remove_wme_from_wm will take care of checking if
			GDS should be removed */
		}
	}
	/* REW: end   09.15.96 */

	// now remove w from working memory
	remove_wme_from_wm(pSoarAgent, pWme);

	/* REW: begin 28.07.96 */
	/* See AddWme for description of what's going on here */

	if (pSoarAgent->current_phase != INPUT_PHASE) {
#ifndef NO_TIMING_STUFF
		pSoarAgent->timers_kernel.start();
#ifndef KERNEL_TIME_ONLY
		pSoarAgent->timers_phase.start();
#endif // KERNEL_TIME_ONLY
#endif // NO_TIMING_STUFF

		/* do_buffered_wm_and_ownership_changes(); */

#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
		pSoarAgent->timers_phase.stop();
		pSoarAgent->timers_decision_cycle_phase[pSoarAgent->current_phase].update(pSoarAgent->timers_phase);
		pSoarAgent->timers_decision_cycle.update(pSoarAgent->timers_phase);
#endif // KERNEL_TIME_ONLY
		pSoarAgent->timers_kernel.stop();
		pSoarAgent->timers_total_kernel_time.update(pSoarAgent->timers_kernel);
		pSoarAgent->timers_kernel.start();
#endif // NO_TIMING_STUFF
	}

	/* note: 
	*  See note at the NO_TOP_LEVEL_REFS flag in soar_cAddWme
	*/

#ifndef NO_TOP_LEVEL_REFS
	do_buffered_wm_and_ownership_changes(pSoarAgent);
#endif // NO_TOP_LEVEL_REFS

	return 0;
}

bool read_wme_filter_component(agent* pSoarAgent, const char *s, Symbol ** sym)
{
	get_lexeme_from_string(pSoarAgent, const_cast<char*>(s));
	if (pSoarAgent->lexeme.type == IDENTIFIER_LEXEME) {
		if ((*sym = find_identifier(pSoarAgent, pSoarAgent->lexeme.id_letter, pSoarAgent->lexeme.id_number)) == NIL) {
			return false;          /* Identifier does not exist */
		}
	} else {
		*sym = make_symbol_for_current_lexeme(pSoarAgent, false);
	}
	// Added by voigtjr because if this function can 
	// legally return success with *sym == 0, my logic in AddWmeFilter will be broken.
	assert(*sym);
	return true;
}

int KernelHelpers::AddWMEFilter(AgentSML* pAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes)
{
	agent* pSoarAgent = pAgent->GetSoarAgent();

	Symbol* pId = 0;
	if (!read_wme_filter_component(pSoarAgent, pIdString, &pId)) {
		return -1;
	}

	Symbol* pAttr = 0;
	if (!read_wme_filter_component(pSoarAgent, pAttrString, &pAttr)) {
		symbol_remove_ref(pSoarAgent, pId);
		return -2;
	}

	Symbol* pValue = 0;
	if (!read_wme_filter_component(pSoarAgent, pValueString, &pValue)) {
		symbol_remove_ref(pSoarAgent, pId);
		symbol_remove_ref(pSoarAgent, pAttr);
		return -3;
	}

	/* check to see if such a filter has already been added: */
	cons *c;
	wme_filter* existing_wf;
	for (c = pSoarAgent->wme_filter_list; c != NIL; c = c->rest) {

		existing_wf = static_cast<wme_filter*>(c->first);

		// check for duplicate
		if ((existing_wf->adds == adds) 
			&& (existing_wf->removes == removes)
			&& (existing_wf->id == pId) 
			&& (existing_wf->attr == pAttr)
			&& (existing_wf->value == pValue)) 
		{
			symbol_remove_ref(pSoarAgent, pId);
			symbol_remove_ref(pSoarAgent, pAttr);
			symbol_remove_ref(pSoarAgent, pValue);
			return -4; // Filter already exists
		}
	}

	wme_filter* wf = static_cast<wme_filter*>(allocate_memory(pSoarAgent, sizeof(wme_filter), MISCELLANEOUS_MEM_USAGE));
	wf->id = pId;
	wf->attr = pAttr;
	wf->value = pValue;
	wf->adds = adds;
	wf->removes = removes;

	/* Rather than add refs for the new filter symbols and then remove refs 
	* for the identical symbols created from the string parameters, skip
	* the two nullifying steps altogether and just return immediately
	* after pushing the new filter:
	*/
	push(pSoarAgent, wf, pSoarAgent->wme_filter_list);     
	return 0;
}

int KernelHelpers::RemoveWMEFilter(AgentSML* pAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes)
{
	agent* pSoarAgent = pAgent->GetSoarAgent();

	Symbol* pId = 0;
	if (!read_wme_filter_component(pSoarAgent, pIdString, &pId)) {
		return -1;
	}

	Symbol* pAttr = 0;
	if (!read_wme_filter_component(pSoarAgent, pAttrString, &pAttr)) {
		symbol_remove_ref(pSoarAgent, pId);
		return -2;
	}

	Symbol* pValue = 0;
	if (!read_wme_filter_component(pSoarAgent, pValueString, &pValue)) {
		symbol_remove_ref(pSoarAgent, pId);
		symbol_remove_ref(pSoarAgent, pAttr);
		return -3;
	}

	cons* c;
	cons** prev_cons_rest = &pSoarAgent->wme_filter_list;
	for (c = pSoarAgent->wme_filter_list; c != NIL; c = c->rest) {
		wme_filter* wf = static_cast<wme_filter*>(c->first);

		// check for duplicate
		if ((wf->adds == adds) 
			&& (wf->removes == removes)
			&& (wf->id == pId) 
			&& (wf->attr == pAttr)
			&& (wf->value == pValue)) 
		{
			*prev_cons_rest = c->rest;
			symbol_remove_ref(pSoarAgent, pId);
			symbol_remove_ref(pSoarAgent, pAttr);
			symbol_remove_ref(pSoarAgent, pValue);
			free_memory(pSoarAgent, wf, MISCELLANEOUS_MEM_USAGE);
			free_cons(pSoarAgent, c);
			return 0; /* assume that AddWMEFilter did not add duplicates */
		}
		prev_cons_rest = &(c->rest);
	}
	assert(!c);
	symbol_remove_ref(pSoarAgent, pId);
	symbol_remove_ref(pSoarAgent, pAttr);
	symbol_remove_ref(pSoarAgent, pValue);
	return -4;
}

bool KernelHelpers::ResetWMEFilters(AgentSML* pAgent, bool adds, bool removes)
{
	agent* pSoarAgent = pAgent->GetSoarAgent();

	cons*c;
	bool didRemoveSome = false;
	cons** prev_cons_rest = &pSoarAgent->wme_filter_list;
	for (c = pSoarAgent->wme_filter_list; c != NIL; c = c->rest) {

		wme_filter* wf = static_cast<wme_filter*>(c->first);
		if ((adds && wf->adds) || (removes && wf->removes)) {
			*prev_cons_rest = c->rest;
			print_with_symbols(pSoarAgent, "Removed: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
			print(pSoarAgent, "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
			symbol_remove_ref(pSoarAgent, wf->id);
			symbol_remove_ref(pSoarAgent, wf->attr);
			symbol_remove_ref(pSoarAgent, wf->value);
			free_memory(pSoarAgent, wf, MISCELLANEOUS_MEM_USAGE);
			free_cons(pSoarAgent, c);
			didRemoveSome = true;
		}
		prev_cons_rest = &(c->rest);
	}
	return didRemoveSome;
}

void KernelHelpers::ListWMEFilters(AgentSML* pAgent, bool adds, bool removes)
{
	agent* pSoarAgent = pAgent->GetSoarAgent();

	cons *c;
	for (c = pSoarAgent->wme_filter_list; c != NIL; c = c->rest) {
		wme_filter* wf = static_cast<wme_filter*>(c->first);

		if ((adds && wf->adds) || (removes && wf->removes)) {
			print_with_symbols(pSoarAgent, "wme filter: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
			print(pSoarAgent, "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
		}
	}
}

void KernelHelpers::ExplainListChunks(AgentSML* pAgent)
{
	agent* pSoarAgent = pAgent->GetSoarAgent();

	explain_chunk_str *chunk;

	chunk = pSoarAgent->explain_chunk_list;

	if (!chunk) {
		print(pSoarAgent, "No chunks/justifications built yet!\n");
	} else {
		print(pSoarAgent, "List of all explained chunks/justifications:\n");
		while (chunk != NULL) {
			print(pSoarAgent, "Have explanation for %s\n", chunk->name);
			chunk = chunk->next_chunk;
		}
	}
}

bool KernelHelpers::ExplainChunks(AgentSML* pAgent, const char* pProduction, int mode)
{
	agent* pSoarAgent = pAgent->GetSoarAgent();

	// mode == -1 full
	// mode == 0 name
	// mode > 0 condition

	get_lexeme_from_string(pSoarAgent, const_cast<char*>(pProduction));

	if (pSoarAgent->lexeme.type != SYM_CONSTANT_LEXEME) {
		return false; // invalid production
	}

	switch (mode) {
		case -1: // full
			{
				explain_chunk_str* chunk = find_chunk(pSoarAgent, pSoarAgent->explain_chunk_list, pSoarAgent->lexeme.string);
				if (chunk) explain_trace_chunk(pSoarAgent, chunk);
			}
			break;
		case 0:
			{
				explain_chunk_str* chunk = find_chunk(pSoarAgent, pSoarAgent->explain_chunk_list, pSoarAgent->lexeme.string);
				if (!chunk) return false;

				/* First print out the production in "normal" form */
				print(pSoarAgent, "(sp %s\n  ", chunk->name);
				print_condition_list(pSoarAgent, chunk->conds, 2, FALSE);
				print(pSoarAgent, "\n-->\n   ");
				print_action_list(pSoarAgent, chunk->actions, 3, FALSE);
				print(pSoarAgent, ")\n\n");

				/* Then list each condition and the associated "ground" WME */
				int i = 0;
				condition* ground = chunk->all_grounds;

				for (condition* cond = chunk->conds; cond != NIL; cond = cond->next) {
					i++;
					print(pSoarAgent, " %2d : ", i);
					print_condition(pSoarAgent, cond);

					while (get_printer_output_column(pSoarAgent) < COLUMNS_PER_LINE - 40)
						print(pSoarAgent, " ");

					print(pSoarAgent, " Ground :");
					print_condition(pSoarAgent, ground);
					print(pSoarAgent, "\n");
					ground = ground->next;
				}
			}
			break;
		default:
			{
				explain_chunk_str* chunk = find_chunk(pSoarAgent, pSoarAgent->explain_chunk_list, pSoarAgent->lexeme.string);
				if (!chunk) return false;

				condition* ground = find_ground(pSoarAgent, chunk, mode);
				if (!ground) return false;

				explain_trace(pSoarAgent, pSoarAgent->lexeme.string, chunk->backtrace, ground);
			}
			break;
	}
	return true;
}

void KernelHelpers::print_rl_rules( agent* thisAgent, char * /*arg*/,bool internal, bool print_filename, bool full_prod)
{
	assert (thisAgent);

	for ( production *prod = thisAgent->all_productions_of_type[ DEFAULT_PRODUCTION_TYPE ]; prod != NIL; prod = prod->next )
	{
		if ( prod->rl_rule )
			do_print_for_production( thisAgent, prod, internal, print_filename, full_prod );
	}
	
	for ( production *prod = thisAgent->all_productions_of_type[ USER_PRODUCTION_TYPE ]; prod != NIL; prod = prod->next )
	{
		if ( prod->rl_rule )
			do_print_for_production( thisAgent, prod, internal, print_filename, full_prod );
	}
	
	for ( production *prod = thisAgent->all_productions_of_type[ CHUNK_PRODUCTION_TYPE ]; prod != NIL; prod = prod->next )
	{
		if ( prod->rl_rule )
			do_print_for_production( thisAgent, prod, internal, print_filename, full_prod );
	}
}


}// namespace
