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
#include "cli_CLIError.h"

#include "sml_KernelHelpers.h"
#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "rete.h"
#include "sml_AgentSML.h"
#include "symtab.h"
#include "production.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseMatches(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "assertions",		OPTARG_NONE},
		{'c', "count",			OPTARG_NONE},
		{'n', "names",			OPTARG_NONE},
		{'r', "retractions",	OPTARG_NONE},
		{'t', "timetags",		OPTARG_NONE},
		{'w', "wmes",			OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	eWMEDetail detail = WME_DETAIL_NONE;
	eMatchesMode mode = MATCHES_ASSERTIONS_RETRACTIONS;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'n':
			case 'c':
				detail = WME_DETAIL_NONE;
				break;
			case 't':
				detail = WME_DETAIL_TIMETAG;
				break;
			case 'w':
				detail = WME_DETAIL_FULL;
				break;
			case 'a':
				mode = MATCHES_ASSERTIONS;
				break;
			case 'r':
				mode = MATCHES_RETRACTIONS;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Max one additional argument and it is a production
	if (m_NonOptionArguments > 1) {
		SetErrorDetail("Expected production name or nothing.");
		return SetError(CLIError::kTooManyArgs);		
	}

	if (m_NonOptionArguments == 1) {
		if (mode != MATCHES_ASSERTIONS_RETRACTIONS) return SetError(CLIError::kTooManyArgs);
		return DoMatches(MATCHES_PRODUCTION, detail, &argv[m_Argument - m_NonOptionArguments]);
	}

	return DoMatches(mode, detail);
}

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

	if (mode == MATCHES_PRODUCTION) {
		if (!pProduction) return SetError(CLIError::kProductionRequired);

		Symbol* sym = find_sym_constant(m_pAgentSoar, pProduction->c_str());
		rete_node* prod = (sym && sym->sc.production) ? sym->sc.production->p_node : 0;

		if (!prod) {
			SetErrorDetail("Production " + *pProduction);
			return SetError(CLIError::kProductionNotFound);
		}

		if (m_RawOutput)
		{
			print_partial_match_information(m_pAgentSML->GetSoarAgent(), prod, wtt);
		}
		else
		{
			xml_partial_match_information(m_pAgentSML->GetSoarAgent(), prod, wtt);
		}

	} else {
		ms_trace_type mst = MS_ASSERT_RETRACT;
		if (mode == MATCHES_ASSERTIONS) mst = MS_ASSERT;
		if (mode == MATCHES_RETRACTIONS) mst = MS_RETRACT;

		if (m_RawOutput)
		{
			print_match_set(m_pAgentSML->GetSoarAgent(), wtt, mst);
		}
		else
		{
			xml_match_set(m_pAgentSML->GetSoarAgent(), wtt, mst);
		}
	}

	// Transfer the result from m_XMLResult into pResponse
	// We pass back the name of the command we just executed which becomes the tag name
	// used in the resulting XML.
	if (!m_RawOutput) XMLResultToResponse(Commands::kCLIMatches) ;

	return true;
}
