/////////////////////////////////////////////////////////////////
// matches command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include <assert.h>

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "rete.h"
#include "sml_AgentSML.h"
#include "symtab.h"
#include "production.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoMatches(const eMatchesMode mode, const eWMEDetail detail, const std::string* pProduction) {
	wme_trace_type wtt = 0;
	switch (detail) {
		case WME_DETAIL_NONE:
			wtt = NONE_WME_TRACE;
			break;
		case WME_DETAIL_TIMETAG:
			wtt = TIMETAG_WME_TRACE;
			break;
		case WME_DETAIL_FULL:
			wtt = FULL_WME_TRACE;
			break;
		default:
			assert(false);
	}

    agent* agnt = m_pAgentSML->GetSoarAgent();
	if (mode == MATCHES_PRODUCTION) {
		if (!pProduction) return SetError("Production required.");

		Symbol* sym = find_sym_constant(agnt, pProduction->c_str());
		rete_node* prod = (sym && sym->sc.production) ? sym->sc.production->p_node : 0;

		if (!prod) 
            return SetError("Production not found: " + *pProduction);

		if (m_RawOutput)
			print_partial_match_information(agnt, prod, wtt);
		else
			xml_partial_match_information(agnt, prod, wtt);

	} else {
		ms_trace_type mst = MS_ASSERT_RETRACT;
		if (mode == MATCHES_ASSERTIONS) mst = MS_ASSERT;
		if (mode == MATCHES_RETRACTIONS) mst = MS_RETRACT;

		if (m_RawOutput)
			print_match_set(agnt, wtt, mst);
		else
			xml_match_set(agnt, wtt, mst);
	}

	// Transfer the result from m_XMLResult into pResponse
	// We pass back the name of the command we just executed which becomes the tag name
	// used in the resulting XML.
	if (!m_RawOutput) XMLResultToResponse("matches") ;

	return true;
}
