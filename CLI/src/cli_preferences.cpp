/////////////////////////////////////////////////////////////////
// preferences command file.
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

#include "sml_KernelSML.h"
#include "sml_KernelHelpers.h"
#include "gdatastructs.h"
#include "utilities.h"
#include "print.h"
#include "tempmem.h"
#include "trace.h"
#include "wmem.h"
#include "decide.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePreferences(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'0', "none",		OPTARG_NONE},
		{'n', "names",		OPTARG_NONE},
		{'1', "names",		OPTARG_NONE},
		{'N', "names",		OPTARG_NONE},
		{'2', "timetags",	OPTARG_NONE},
		{'t', "timetags",	OPTARG_NONE},
		{'3', "wmes",		OPTARG_NONE},
		{'w', "wmes",		OPTARG_NONE},
		{'o', "object",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	ePreferencesDetail detail = PREFERENCES_ONLY;
	bool object = false;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case '0':
				detail = PREFERENCES_ONLY;
				break;
			case '1':
			case 'n':
			case 'N':
				detail = PREFERENCES_NAMES;
				break;
			case '2':
			case 't':
				detail = PREFERENCES_TIMETAGS;
				break;
			case '3':
			case 'w':
				detail = PREFERENCES_WMES;
				break;
				
			case 'o':
			case 'O':
				object = true;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Up to two non-option arguments allowed, id/attribute
	if (m_NonOptionArguments > 2) return SetError(CLIError::kTooManyArgs);

	int optind = m_Argument - m_NonOptionArguments;
	if (m_NonOptionArguments == 2) {
		// id & attribute
		return DoPreferences(detail, object, &argv[optind], &argv[optind + 1]);
	}
	if (m_NonOptionArguments == 1) {
		// id
		return DoPreferences(detail, object, &argv[optind]);
	}

	return DoPreferences(detail, object);
}

/*
*	This procedure parses a string to determine if it is a
*      lexeme for an existing attribute.
*
* Side effects:
*	None.
*
===============================
*/

bool read_attribute_from_string (agent* agnt, Symbol *id, char * the_lexeme, Symbol * * attr)
{
	Symbol *attr_tmp;
	slot *s;

	/* skip optional '^' if present.  KJC added to Ken's code */
	if (*the_lexeme == '^')
	{
		the_lexeme++;
	}

	get_lexeme_from_string(agnt, the_lexeme);

	switch (agnt->lexeme.type) 
	{
	case SYM_CONSTANT_LEXEME:
		attr_tmp = find_sym_constant (agnt, agnt->lexeme.string);
		break;
	case INT_CONSTANT_LEXEME:
		attr_tmp = find_int_constant (agnt, agnt->lexeme.int_val);
		break;
	case FLOAT_CONSTANT_LEXEME:
		attr_tmp = find_float_constant (agnt, agnt->lexeme.float_val);
		break;
	case IDENTIFIER_LEXEME:
		attr_tmp = find_identifier (agnt, agnt->lexeme.id_letter,
			agnt->lexeme.id_number);
		break;
	case VARIABLE_LEXEME:
		attr_tmp = read_identifier_or_context_variable(agnt);
		if (!attr_tmp)
			return false;
		break;
	default:
		return false;
	}
	s = find_slot (id, attr_tmp);
	if (s) {
		*attr = attr_tmp;
		return true;
	} else						
		*attr = attr_tmp;
		return false;
}

/*
*
*	This procedure prints a preference and the production
*      which is the source of the preference.
*
* Results:
*	Tcl status code.
*
* Side effects:
*	Prints the preference and its source production.
*
* NOTE:  The called of this routine should be stepping thru slots only, 
*        (not stepping thru WMEs) and therefore input wmes and arch-wmes
*        are already excluded and we can print :I when o_support is FALSE.
* 
===============================
*/
void print_preference_and_source (agent* agnt, preference *pref,
								  bool print_source,
								  wme_trace_type wtt,
								  double* selection_probability = 0) 
{
	print_string (agnt, "  ");
	if (pref->attr == agnt->operator_symbol) {
		print_object_trace (agnt, pref->value);
	} else {					
		print_with_symbols (agnt, "(%y ^%y %y) ", pref->id, pref->attr, pref->value);
	} 	
	if (pref->attr == agnt->operator_symbol) {
		print (agnt, " %c", preference_type_indicator (agnt, pref->type));
	}
	if (preference_is_binary(pref->type)) print_object_trace (agnt, pref->referent);
	if (pref->o_supported) print (agnt, " :O "); else print (agnt, " :I ");
	if (selection_probability) print (agnt, "(%.1f%%)", (*selection_probability) * 100.0);
	print (agnt, "\n");
	if (print_source) {
		print (agnt, "    From ");
		print_instantiation_with_wmes (agnt, pref->inst, wtt, -1);
		print (agnt, "\n");
	}
}

int soar_ecPrintPreferences(agent* soarAgent, char *szId, char *szAttr, bool object, bool print_prod, wme_trace_type wtt)
{

	Symbol *id, *attr = 0;
	slot *s = 0 ;
	preference *p;
	wme *w;
	int i;

	if (!read_id_or_context_var_from_string(soarAgent, szId, &id)) {
		print(soarAgent, "Could not find the id '%s'\n", szId);
		return -1;
	}

	/// This is badbad style using the literal string, but it all has to
	/// change soon, so I'll cheat for now.  If we have an ID that isn't a
	/// state (goal), then don't use the default ^operator.  Instead, search
	/// for wmes with that ID as a value.  See below
	if (!id->id.isa_goal && !strcmp(szAttr, "operator")) {
		attr = NIL;
	} else {
		if ( szAttr && !object) { // default ^attr is ^operator, unless specified --object on cmdline
			if (!read_attribute_from_string(soarAgent, id, szAttr, &attr)) {		
				// NOT tested:  but here goes...
				// This is code to determine whether ^attr arg is misspelled 
				// or an arch-wme.  Had to modify read_attribute_from_string() to  
				// always set the attr:  symbol exists but no slot, or attr = NIL.
				if (attr) {
					print (soarAgent,"  This is probably an io- or arch-wme and does not have preferences\n");							
					return 0;										
				}
				print(soarAgent, "Could not find prefs for the id,attribute pair: %s %s\n", szId, szAttr);
				return -2;
			}			
			s = find_slot(id, attr);
			if (!s && !object) {
				// Should we check for input wmes and arch-wmes ?? ...covered above...
				print(soarAgent, "Could not find preferences for %s ^%s.", szId, szAttr);
				return -3;
			}
		}
	}

	// We have one of three cases now, as of v8.6.3
	//     1.  --object is specified:  return prefs for all wmes comprising object ID
	//                    (--depth not yet implemented...)
	//     2.  non-state ID is given:  return prefs for wmes whose <val> is ID
	//     3.  default (no args):  return prefs of slot (id, attr)  <s> ^operator

	if (object) {
		// step thru dll of slots for ID, printing prefs for each one
		for (s = id->id.slots; s != NIL; s = s->next ) {		
			if (s->attr == soarAgent->operator_symbol)
				print_with_symbols(soarAgent, "Preferences for %y ^%y:", s->id, s->attr);				
			else 
				print_with_symbols(soarAgent, "Support for %y ^%y:\n", s->id, s->attr);				
			for (i = 0; i < NUM_PREFERENCE_TYPES; i++) {
				if (s->preferences[i]) {
					if (s->isa_context_slot) print(soarAgent, "\n%ss:\n", preference_name[i]);
					for (p = s->preferences[i]; p; p = p->next) {
						print_preference_and_source(soarAgent, p, print_prod, wtt);
					}
				}
			}
		}
		if (id->id.impasse_wmes)
			print_with_symbols(soarAgent, "Arch-created wmes for %y :\n", id);				
		for (w=id->id.impasse_wmes; w!=NIL; w=w->next)   {
			print_wme(soarAgent, w);
		}
		if (id->id.input_wmes)
			print_with_symbols(soarAgent, "Input (IO) wmes for %y :\n", id);				
		for (w=id->id.input_wmes; w!=NIL; w=w->next) {
			print_wme(soarAgent, w);
		}

		return 0;
	} else if (!id->id.isa_goal && !attr ) {  
		// find wme(s?) whose value is <ID> and print prefs if they exist
		// ??? should write print_prefs_for_id(soarAgent, id, print_prod, wtt);
		// return;					
		for (w=soarAgent->all_wmes_in_rete; w!=NIL; w=w->rete_next) {				
			if (w->value == id )  {				
				if (w->value == soarAgent->operator_symbol)
					print (soarAgent, "Preferences for (%lu: ", w->timetag);					
				else 
					print (soarAgent, "Support for (%lu: ", w->timetag);					
				print_with_symbols (soarAgent, "%y ^%y %y)\n", w->id, w->attr, w->value);
				if (w->preference) {
					s = find_slot(w->id, w->attr);
					if (!s) {
						print (soarAgent, "    This is an arch-wme and has no prefs.\n");
					} else {		
						for (i = 0; i < NUM_PREFERENCE_TYPES; i++) {
							if (s->preferences[i]) {			
								// print(soarAgent, "\n%ss:\n", preference_name[i]);			
								for (p = s->preferences[i]; p; p = p->next) {			
									if (p->value == id) print_preference_and_source(soarAgent, p, print_prod, wtt);				
								}			
							}		
						}
					}
					//print it
				} else {
					print (soarAgent, "    This is an input-wme and has no prefs.\n");
				}
			}
		}
		return 0;			
	}

	//print prefs for specified slot
	print_with_symbols(soarAgent, "\nPreferences for %y ^%y:\n", id, attr);

	for (i = 0; i < NUM_PREFERENCE_TYPES; i++) 
	{
		if (s->preferences[i]) 
		{
			print(soarAgent, "\n%ss:\n", preference_name[i]);
			for (p = s->preferences[i]; p; p = p->next) 
				print_preference_and_source(soarAgent, p, print_prod, wtt);
		}
	}

	// voigtjr march 2010
	// print selection probabilities re: issue 18
	// run preference semantics "read only" via _consistency_check
	// returns a list of candidates without deciding which one in the event of indifference
	preference* cand = 0;
	byte impasse_type = run_preference_semantics_for_consistency_check(soarAgent, s, &cand);

	// if the impasse isn't NONE_IMPASSE_TYPE, there's an impasse and we don't want to print anything
	// if we have no candidates, we don't want to print anything
	if ((impasse_type == NONE_IMPASSE_TYPE) && cand)
	{
		print(soarAgent, "\nselection probabilities:\n");

		// some of this following code is redundant with code in exploration.cpp
		// see exploration_choose_according_to_policy
		// see exploration_compute_value_of_candidate
		// see exploration_probabilistically_select
		int count = 0;
		double total_probability = 0;
		// add up positive numeric values, count candidates
		for (p = cand; p; p = p->next_candidate) 
		{
			exploration_compute_value_of_candidate(soarAgent, p, s);
			++count;
			if ( p->numeric_value > 0 )
				total_probability += p->numeric_value;
		}
		assert (count != 0);
		for (p = cand; p; p = p->next_candidate) 
		{
			// if total probability is zero, fall back to random
			double prob = total_probability > 0.0 ? p->numeric_value / total_probability : 1.0 / count;
			print_preference_and_source(soarAgent, p, print_prod, wtt, &prob);
		}
	}

	return 0;
}

bool CommandLineInterface::DoPreferences(const ePreferencesDetail detail, bool object, const std::string* pId, const std::string* pAttribute) {
	static const int PREFERENCES_BUFFER_SIZE = 128;

	char id[PREFERENCES_BUFFER_SIZE];
	char attr[PREFERENCES_BUFFER_SIZE];

	const char* idString = pId ? pId->c_str() : 0;
	const char* attrString = pAttribute ? pAttribute->c_str() : 0;

	// Establish defaults
	symbol_to_string(m_pAgentSoar, m_pAgentSoar->bottom_goal, TRUE, id, PREFERENCES_BUFFER_SIZE);
	symbol_to_string(m_pAgentSoar, m_pAgentSoar->operator_symbol, TRUE, attr, PREFERENCES_BUFFER_SIZE);

	// Override defaults
	if (idString) {
		strncpy(id, idString, PREFERENCES_BUFFER_SIZE);

		// Notice: attrString arg ignored if no idString passed
		if (attrString) {
			strncpy(attr, attrString, PREFERENCES_BUFFER_SIZE);
		} 
	}

	bool print_productions = false;
	wme_trace_type wtt = NONE_WME_TRACE;

	switch (detail) {
		case PREFERENCES_ONLY:
			print_productions = false;
			wtt = NONE_WME_TRACE;
			break;
		case PREFERENCES_NAMES:
			print_productions = true;
			wtt = NONE_WME_TRACE;
			break;
		case PREFERENCES_TIMETAGS:
			print_productions = true;
			wtt = TIMETAG_WME_TRACE;
			break;
		default:
			// falls through
		case PREFERENCES_WMES:
			print_productions = true;
			wtt = FULL_WME_TRACE;
			break;
	}

	if (soar_ecPrintPreferences(m_pAgentSoar, id, attr, object, print_productions, wtt)) {
		print(m_pAgentSoar, "An Error occured trying to print the prefs.");
		return SetError(CLIError::kPreferencesError);
	}
	return true;
}
