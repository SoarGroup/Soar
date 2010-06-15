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
#include "cli_CLIError.h"

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
						return SetError(CLIError::kInvalidWMEFilterType);
					}
				}
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}
	
	if (mode == WATCH_WMES_ADD || mode == WATCH_WMES_REMOVE) {
		// type required
		if (type.none()) return SetError(CLIError::kTypeRequired);
	
		// check for too few/many args
		if (m_NonOptionArguments > 3) return SetError(CLIError::kTooManyArgs);
		if (m_NonOptionArguments < 3) return SetError(CLIError::kTooFewArgs);

		int optind = m_Argument - m_NonOptionArguments;
		return DoWatchWMEs(mode, type, &argv[optind], &argv[optind + 1], &argv[optind + 2]);
	}

	// no additional arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoWatchWMEs(mode, type);
}

typedef struct wme_filter_struct {
	Symbol *id;
	Symbol *attr;
	Symbol *value;
	bool adds;
	bool removes;
} wme_filter;

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
					xml_begin_tag(pSoarAgent, soar_TraceNames::kTagVerbose);
					xml_att_val(pSoarAgent, soar_TraceNames::kTypeString, buf);
					print_wme(pSoarAgent, pWme);
					xml_end_tag(pSoarAgent, soar_TraceNames::kTagVerbose);
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

int AddWMEFilter(AgentSML* pAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes)
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

int RemoveWMEFilter(AgentSML* pAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes)
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

bool ResetWMEFilters(AgentSML* pAgent, bool adds, bool removes)
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

void ListWMEFilters(AgentSML* pAgent, bool adds, bool removes)
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

bool CommandLineInterface::DoWatchWMEs(const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString, const std::string* pAttributeString, const std::string* pValueString) {
	int ret = 0;
	bool retb = false;
	switch (mode) {
		case WATCH_WMES_ADD:
			if (!pIdString || !pAttributeString || !pValueString) return SetError(CLIError::kFilterExpected);
			ret = AddWMEFilter(m_pAgentSML, pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
			if (ret == -1) {
				SetErrorDetail("Got: " + *pIdString);
				return SetError(CLIError::kInvalidID);
			}
			if (ret == -2) {
				SetErrorDetail("Got: " + *pAttributeString);
				return SetError(CLIError::kInvalidAttribute);
			}
			if (ret == -3) {
				SetErrorDetail("Got: " + *pValueString);
				return SetError(CLIError::kInvalidValue);
			}
			if (ret == -4) return SetError(CLIError::kDuplicateWMEFilter);
			break;

		case WATCH_WMES_REMOVE:
			if (!pIdString || !pAttributeString || !pValueString) return SetError(CLIError::kFilterExpected);
			ret = RemoveWMEFilter(m_pAgentSML, pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
			if (ret == -1) {
				SetErrorDetail("Got: " + *pIdString);
				return SetError(CLIError::kInvalidID);
			}
			if (ret == -2) {
				SetErrorDetail("Got: " + *pAttributeString);
				return SetError(CLIError::kInvalidAttribute);
			}
			if (ret == -3) {
				SetErrorDetail("Got: " + *pValueString);
				return SetError(CLIError::kInvalidValue);
			}
			if (ret == -4) return SetError(CLIError::kWMEFilterNotFound);
			break;

		case WATCH_WMES_LIST:
			if (type.none()) type.flip();

			ListWMEFilters(m_pAgentSML, type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
			break;

		case WATCH_WMES_RESET:
			if (type.none()) type.flip();

			retb = ResetWMEFilters(m_pAgentSML, type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));

			if (!retb) return SetError(CLIError::kWMEFilterNotFound);
			break;

		default:
			return SetError(CLIError::kInvalidMode);
	}

	return true;
}
