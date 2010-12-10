/////////////////////////////////////////////////////////////////
// watch-wmes command file.
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
#include "wmem.h"
#include "symtab.h"
#include "mem.h"
#include "agent.h"
#include "print.h"
#include "xml.h"
#include "decide.h"
#include "parser.h"
#include "soar_TraceNames.h"
#include "utilities.h"
#include "sml_AgentSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWatchWMEs(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "add-filter",		OPTARG_NONE},
		{'r', "remove-filter",	OPTARG_NONE},
		{'l', "list-filter",	OPTARG_NONE},
		{'R', "reset-filter",	OPTARG_NONE},
		{'t', "type",			OPTARG_REQUIRED},
		{0, 0, OPTARG_NONE}
	};

	eWatchWMEsMode mode = WATCH_WMES_LIST;
	WatchWMEsTypeBitset type(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'a':
				mode = WATCH_WMES_ADD;
				break;
			case 'r':
				mode = WATCH_WMES_REMOVE;
				break;
			case 'l':
				mode = WATCH_WMES_LIST;
				break;
			case 'R':
				mode = WATCH_WMES_RESET;
				break;
			case 't':
				{
					std::string typeString = m_OptionArgument;
					if (typeString == "adds") {
						type.set(WATCH_WMES_TYPE_ADDS);
					} else if (typeString == "removes") {
						type.set(WATCH_WMES_TYPE_REMOVES);
					} else if (typeString == "both") {
						type.set(WATCH_WMES_TYPE_ADDS);
						type.set(WATCH_WMES_TYPE_REMOVES);
					} else {
						SetErrorDetail("Got: " + typeString);
						return SetError(kInvalidWMEFilterType);
					}
				}
				break;
			default:
				return SetError(kGetOptError);
		}
	}
	
	if (mode == WATCH_WMES_ADD || mode == WATCH_WMES_REMOVE) {
		// type required
		if (type.none()) return SetError(kTypeRequired);
	
		// check for too few/many args
		if (m_NonOptionArguments > 3) return SetError(kTooManyArgs);
		if (m_NonOptionArguments < 3) return SetError(kTooFewArgs);

		int optind = m_Argument - m_NonOptionArguments;
		return DoWatchWMEs(mode, type, &argv[optind], &argv[optind + 1], &argv[optind + 2]);
	}

	// no additional arguments
	if (m_NonOptionArguments) return SetError(kTooManyArgs);

	return DoWatchWMEs(mode, type);
}

typedef struct wme_filter_struct {
	Symbol *id;
	Symbol *attr;
	Symbol *value;
	bool adds;
	bool removes;
} wme_filter;

int RemoveWme(agent* thisAgent, wme* pWme)
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
			gds_invalid_so_remove_goal(thisAgent, pWme);
			/* NOTE: the call to remove_wme_from_wm will take care of checking if
			GDS should be removed */
		}
	}
	/* REW: end   09.15.96 */

	// now remove w from working memory
	remove_wme_from_wm(thisAgent, pWme);

	/* REW: begin 28.07.96 */
	/* See AddWme for description of what's going on here */

	if (thisAgent->current_phase != INPUT_PHASE) {
#ifndef NO_TIMING_STUFF
		thisAgent->timers_kernel.start();
#ifndef KERNEL_TIME_ONLY
		thisAgent->timers_phase.start();
#endif // KERNEL_TIME_ONLY
#endif // NO_TIMING_STUFF

		/* do_buffered_wm_and_ownership_changes(); */

#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
		thisAgent->timers_phase.stop();
		thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
#endif // KERNEL_TIME_ONLY
		thisAgent->timers_kernel.stop();
		thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
		thisAgent->timers_kernel.start();
#endif // NO_TIMING_STUFF
	}

	/* note: 
	*  See note at the NO_TOP_LEVEL_REFS flag in soar_cAddWme
	*/

#ifndef NO_TOP_LEVEL_REFS
	do_buffered_wm_and_ownership_changes(thisAgent);
#endif // NO_TOP_LEVEL_REFS

	return 0;
}

bool read_wme_filter_component(agent* thisAgent, const char *s, Symbol ** sym)
{
	get_lexeme_from_string(thisAgent, const_cast<char*>(s));
	if (thisAgent->lexeme.type == IDENTIFIER_LEXEME) {
		if ((*sym = find_identifier(thisAgent, thisAgent->lexeme.id_letter, thisAgent->lexeme.id_number)) == NIL) {
			return false;          /* Identifier does not exist */
		}
	} else {
		*sym = make_symbol_for_current_lexeme(thisAgent, false);
	}
	// Added by voigtjr because if this function can 
	// legally return success with *sym == 0, my logic in AddWmeFilter will be broken.
	assert(*sym);
	return true;
}

int AddWMEFilter(agent* thisAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes)
{
	Symbol* pId = 0;
	if (!read_wme_filter_component(thisAgent, pIdString, &pId)) {
		return -1;
	}

	Symbol* pAttr = 0;
	if (!read_wme_filter_component(thisAgent, pAttrString, &pAttr)) {
		symbol_remove_ref(thisAgent, pId);
		return -2;
	}

	Symbol* pValue = 0;
	if (!read_wme_filter_component(thisAgent, pValueString, &pValue)) {
		symbol_remove_ref(thisAgent, pId);
		symbol_remove_ref(thisAgent, pAttr);
		return -3;
	}

	/* check to see if such a filter has already been added: */
	cons *c;
	wme_filter* existing_wf;
	for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest) {

		existing_wf = static_cast<wme_filter*>(c->first);

		// check for duplicate
		if ((existing_wf->adds == adds) 
			&& (existing_wf->removes == removes)
			&& (existing_wf->id == pId) 
			&& (existing_wf->attr == pAttr)
			&& (existing_wf->value == pValue)) 
		{
			symbol_remove_ref(thisAgent, pId);
			symbol_remove_ref(thisAgent, pAttr);
			symbol_remove_ref(thisAgent, pValue);
			return -4; // Filter already exists
		}
	}

	wme_filter* wf = static_cast<wme_filter*>(allocate_memory(thisAgent, sizeof(wme_filter), MISCELLANEOUS_MEM_USAGE));
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
	push(thisAgent, wf, thisAgent->wme_filter_list);     
	return 0;
}

int RemoveWMEFilter(agent* thisAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes)
{
	Symbol* pId = 0;
	if (!read_wme_filter_component(thisAgent, pIdString, &pId)) {
		return -1;
	}

	Symbol* pAttr = 0;
	if (!read_wme_filter_component(thisAgent, pAttrString, &pAttr)) {
		symbol_remove_ref(thisAgent, pId);
		return -2;
	}

	Symbol* pValue = 0;
	if (!read_wme_filter_component(thisAgent, pValueString, &pValue)) {
		symbol_remove_ref(thisAgent, pId);
		symbol_remove_ref(thisAgent, pAttr);
		return -3;
	}

	cons* c;
	cons** prev_cons_rest = &thisAgent->wme_filter_list;
	for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest) {
		wme_filter* wf = static_cast<wme_filter*>(c->first);

		// check for duplicate
		if ((wf->adds == adds) 
			&& (wf->removes == removes)
			&& (wf->id == pId) 
			&& (wf->attr == pAttr)
			&& (wf->value == pValue)) 
		{
			*prev_cons_rest = c->rest;
			symbol_remove_ref(thisAgent, pId);
			symbol_remove_ref(thisAgent, pAttr);
			symbol_remove_ref(thisAgent, pValue);
			free_memory(thisAgent, wf, MISCELLANEOUS_MEM_USAGE);
			free_cons(thisAgent, c);
			return 0; /* assume that AddWMEFilter did not add duplicates */
		}
		prev_cons_rest = &(c->rest);
	}
	assert(!c);
	symbol_remove_ref(thisAgent, pId);
	symbol_remove_ref(thisAgent, pAttr);
	symbol_remove_ref(thisAgent, pValue);
	return -4;
}

bool ResetWMEFilters(agent* thisAgent, bool adds, bool removes)
{
	cons*c;
	bool didRemoveSome = false;
	cons** prev_cons_rest = &thisAgent->wme_filter_list;
	for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest) {

		wme_filter* wf = static_cast<wme_filter*>(c->first);
		if ((adds && wf->adds) || (removes && wf->removes)) {
			*prev_cons_rest = c->rest;
			print_with_symbols(thisAgent, "Removed: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
			print(thisAgent, "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
			symbol_remove_ref(thisAgent, wf->id);
			symbol_remove_ref(thisAgent, wf->attr);
			symbol_remove_ref(thisAgent, wf->value);
			free_memory(thisAgent, wf, MISCELLANEOUS_MEM_USAGE);
			free_cons(thisAgent, c);
			didRemoveSome = true;
		}
		prev_cons_rest = &(c->rest);
	}
	return didRemoveSome;
}

void ListWMEFilters(agent* thisAgent, bool adds, bool removes)
{
	cons *c;
	for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest) {
		wme_filter* wf = static_cast<wme_filter*>(c->first);

		if ((adds && wf->adds) || (removes && wf->removes)) {
			print_with_symbols(thisAgent, "wme filter: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
			print(thisAgent, "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
		}
	}
}

bool CommandLineInterface::DoWatchWMEs(const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString, const std::string* pAttributeString, const std::string* pValueString) {
	int ret = 0;
	bool retb = false;
	switch (mode) {
		case WATCH_WMES_ADD:
			if (!pIdString || !pAttributeString || !pValueString) return SetError(kFilterExpected);
			ret = AddWMEFilter(m_pAgentSoar, pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
			if (ret == -1) {
				SetErrorDetail("Got: " + *pIdString);
				return SetError(kInvalidID);
			}
			if (ret == -2) {
				SetErrorDetail("Got: " + *pAttributeString);
				return SetError(kInvalidAttribute);
			}
			if (ret == -3) {
				SetErrorDetail("Got: " + *pValueString);
				return SetError(kInvalidValue);
			}
			if (ret == -4) return SetError(kDuplicateWMEFilter);
			break;

		case WATCH_WMES_REMOVE:
			if (!pIdString || !pAttributeString || !pValueString) return SetError(kFilterExpected);
			ret = RemoveWMEFilter(m_pAgentSoar, pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
			if (ret == -1) {
				SetErrorDetail("Got: " + *pIdString);
				return SetError(kInvalidID);
			}
			if (ret == -2) {
				SetErrorDetail("Got: " + *pAttributeString);
				return SetError(kInvalidAttribute);
			}
			if (ret == -3) {
				SetErrorDetail("Got: " + *pValueString);
				return SetError(kInvalidValue);
			}
			if (ret == -4) return SetError(kWMEFilterNotFound);
			break;

		case WATCH_WMES_LIST:
			if (type.none()) type.flip();

			ListWMEFilters(m_pAgentSoar, type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
			break;

		case WATCH_WMES_RESET:
			if (type.none()) type.flip();

			retb = ResetWMEFilters(m_pAgentSoar, type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));

			if (!retb) return SetError(kWMEFilterNotFound);
			break;

		default:
			return SetError(kInvalidMode);
	}

	return true;
}
