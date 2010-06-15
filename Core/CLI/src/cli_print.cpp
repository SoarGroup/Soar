/////////////////////////////////////////////////////////////////
// print command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "agent.h"

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "gsysparam.h"
#include "xml.h"
#include "print.h"
#include "trace.h"
#include "wmem.h"
#include "rhsfun.h"
#include "utilities.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePrint(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "all",			OPTARG_NONE},
		{'c', "chunks",			OPTARG_NONE},
		{'d', "depth",			OPTARG_REQUIRED},
		{'D', "defaults",		OPTARG_NONE},
		{'e', "exact",			OPTARG_NONE},
		{'f', "full",			OPTARG_NONE},
		{'F', "filename",		OPTARG_NONE},
		{'i', "internal",		OPTARG_NONE},
		{'j', "justifications",	OPTARG_NONE},
		{'n', "name",			OPTARG_NONE},
		{'o', "operators",		OPTARG_NONE},
		{'r', "rl",				OPTARG_NONE},
		{'s', "stack",			OPTARG_NONE},
		{'S', "states",			OPTARG_NONE},
		{'t', "tree",			OPTARG_NONE},
		{'T', "template",		OPTARG_NONE},
		{'u', "user",			OPTARG_NONE},
		{'v', "varprint",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	int depth = m_pAgentSoar->default_wme_depth;
	PrintBitset options(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'a':
				options.set(PRINT_ALL);
				break;
			case 'c':
				options.set(PRINT_CHUNKS);
				break;
			case 'd':
				options.set(PRINT_DEPTH);
				if ( !from_string( depth, m_OptionArgument ) ) return SetError(CLIError::kIntegerExpected);
				if (depth < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
				break;
			case 'D':
				options.set(PRINT_DEFAULTS);
				break;
			case 'e':
				options.set(PRINT_EXACT);
				break;
			case 'f':
				options.set(PRINT_FULL);
				break;
			case 'F':
				options.set(PRINT_FILENAME);
				break;
			case 'i':
				options.set(PRINT_INTERNAL);
				break;
			case 'j':
				options.set(PRINT_JUSTIFICATIONS);
				break;
			case 'n':
				options.set(PRINT_NAME);
				break;
			case 'o':
				options.set(PRINT_OPERATORS);
				break;
			case 'r':
				options.set(PRINT_RL);
				break;
			case 's':
				options.set(PRINT_STACK);
				break;
			case 'S':
				options.set(PRINT_STATES);
				break;
			case 't':
				options.set(PRINT_TREE);
				break;
			case 'T':
				options.set(PRINT_TEMPLATE);
				break;
			case 'u':
				options.set(PRINT_USER);
				break;
			case 'v':
				options.set(PRINT_VARPRINT);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// STATES and OPERATORS are sub-options of STACK
	if (options.test(PRINT_OPERATORS) || options.test(PRINT_STATES)) {
		if (!options.test(PRINT_STACK)) return SetError(CLIError::kPrintSubOptionsOfStack);
	}

	switch (m_NonOptionArguments) {
		case 0:  // no argument
			// d and t options require an argument
			if (options.test(PRINT_TREE) || options.test(PRINT_DEPTH)) return SetError(CLIError::kTooFewArgs);
			return DoPrint(options, depth);

		case 1: 
			// the acDjus options don't allow an argument
			if (options.test(PRINT_ALL) 
				|| options.test(PRINT_CHUNKS) 
				|| options.test(PRINT_DEFAULTS) 
				|| options.test(PRINT_JUSTIFICATIONS)
				|| options.test(PRINT_RL)
				|| options.test(PRINT_TEMPLATE)
				|| options.test(PRINT_USER) 
				|| options.test(PRINT_STACK)) 
			{
				SetErrorDetail("No argument allowed when printing all/chunks/defaults/justifications/rl/template/user/stack.");
				return SetError(CLIError::kTooManyArgs);
			}
			if (options.test(PRINT_EXACT) && (options.test(PRINT_DEPTH) || options.test(PRINT_TREE))) 
			{
				SetErrorDetail("No depth/tree flags allowed when printing exact.");
				return SetError(CLIError::kTooManyArgs);
			}
			return DoPrint(options, depth, &(argv[m_Argument - m_NonOptionArguments]));

		default: // more than 1 arg
			break;
	}

	return SetError(CLIError::kTooManyArgs);
}

void PrintStackTrace(AgentSML* agent, bool print_states, bool print_operators)
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

void PrintUser(AgentSML*       thisAgent,
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

void print_rl_rules( agent* thisAgent, bool internal, bool print_filename, bool full_prod)
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
				xml_begin_tag(agnt, soar_TraceNames::kWME_Id);
				xml_att_val(agnt, soar_TraceNames::kWME_Id, id);

				for (attr=0; attr < num_attr; attr++) {
					w = list[attr];
					neatly_print_wme_augmentation_of_id (agnt, w, indent);
				}
				
				xml_end_tag(agnt, soar_TraceNames::kWME_Id);

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

void PrintSymbol(AgentSML* thisAgent,
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

bool CommandLineInterface::DoPrint(PrintBitset options, int depth, const std::string* pArg) {
	// Strip any surrounding "{"
	/*
	std::string local = *pArg ;
	if (local.length() > 2)
	{
		if (local[0] == '{') local = local.substr(1) ;
		if (local[local.length()-1] == '}') local = local.substr(0, local.length()-1) ;
	}
	*/

	// Check for stack print
	if (options.test(PRINT_STACK)) {

		// if neither states option nor operators option are set, set them both
		if (!options.test(PRINT_STATES) && !options.test(PRINT_OPERATORS)) {
			options.set(PRINT_STATES);
			options.set(PRINT_OPERATORS);
		}

		// Structured output through structured output callback
		PrintStackTrace(m_pAgentSML, (options.test(PRINT_STATES)) ? true : false, (options.test(PRINT_OPERATORS)) ? true : false);
		return true;
	}

	// Cache the flags since it makes function calls huge
	bool internal = options.test(PRINT_INTERNAL);
	bool tree = options.test(PRINT_TREE);
	bool filename = options.test(PRINT_FILENAME);
	bool full = options.test(PRINT_FULL);
	bool name = options.test(PRINT_NAME);
	bool exact = options.test(PRINT_EXACT);

	// Check for the five general print options (all, chunks, defaults, justifications, user)
	if (options.test(PRINT_ALL)) {
        PrintUser(m_pAgentSML, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        PrintUser(m_pAgentSML, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        PrintUser(m_pAgentSML, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        PrintUser(m_pAgentSML, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
        PrintUser(m_pAgentSML, 0, internal, filename, full, TEMPLATE_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_CHUNKS)) {
        PrintUser(m_pAgentSML, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_DEFAULTS)) {
        PrintUser(m_pAgentSML, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_JUSTIFICATIONS)) {
        PrintUser(m_pAgentSML, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_USER)) {
        PrintUser( m_pAgentSML, 0, internal, filename, full, USER_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_RL)) {
		print_rl_rules( m_pAgentSoar, internal, filename, full );
		return true;
	}
	if (options.test(PRINT_TEMPLATE)) {
        PrintUser( m_pAgentSML, 0, internal, filename, full, TEMPLATE_PRODUCTION_TYPE );
		return true;
	}

	// Default to symbol print if there is an arg, otherwise print all
	if (options.test(PRINT_VARPRINT)) {
		m_VarPrint = true;
	}
	if (pArg) {
		PrintSymbol(m_pAgentSML, const_cast<char*>(pArg->c_str()), name, filename, internal, tree, full, depth, exact);
	} else {
        PrintUser(m_pAgentSML, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        PrintUser(m_pAgentSML, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        PrintUser(m_pAgentSML, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        PrintUser(m_pAgentSML, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
        PrintUser(m_pAgentSML, 0, internal, filename, full, TEMPLATE_PRODUCTION_TYPE);
	}
	m_VarPrint = false;

	return true;
}

