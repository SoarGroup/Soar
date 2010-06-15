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

////

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


}// namespace
