/////////////////////////////////////////////////////////////////
// matches command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include <assert.h>

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseMatches(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "assertions",		0},
		{'c', "count",			0},
		{'n', "names",			0},
		{'r', "retractions",	0},
		{'t', "timetags",		0},
		{'w', "wmes",			0},
		{0, 0, 0}
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
		return DoMatches(pAgent, MATCHES_PRODUCTION, detail, &argv[m_Argument - m_NonOptionArguments]);
	}

	return DoMatches(pAgent, mode, detail);
}

bool CommandLineInterface::DoMatches(gSKI::IAgent* pAgent, const eMatchesMode mode, const eWMEDetail detail, const std::string* pProduction) {

	if (!RequireAgent(pAgent)) return false;

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

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (mode == MATCHES_PRODUCTION) {
		if (!pProduction) return SetError(CLIError::kProductionRequired);
		rete_node* prod = pKernelHack->NameToProduction(pAgent, const_cast<char*>(pProduction->c_str()));
		if (!prod) {
			SetErrorDetail("Production " + *pProduction);
			return SetError(CLIError::kProductionNotFound);
		}

		if (m_RawOutput)
		{
			AddListenerAndDisableCallbacks(pAgent);		
			pKernelHack->PrintPartialMatchInformation(pAgent, prod, wtt);
			RemoveListenerAndEnableCallbacks(pAgent);
		}
		else
		{
			pKernelHack->XMLPartialMatchInformation(pAgent, prod, wtt) ;
		}

	} else {
		ms_trace_type mst = MS_ASSERT_RETRACT;
		if (mode == MATCHES_ASSERTIONS) mst = MS_ASSERT;
		if (mode == MATCHES_RETRACTIONS) mst = MS_RETRACT;

		if (m_RawOutput)
		{
			AddListenerAndDisableCallbacks(pAgent);		
			pKernelHack->PrintMatchSet(pAgent, wtt, mst);
			RemoveListenerAndEnableCallbacks(pAgent);
		}
		else
		{
			pKernelHack->XMLMatchSet(pAgent, wtt, mst) ;
		}
	}

	// Transfer the result from m_XMLResult into pResponse
	if (!m_RawOutput) XMLResultToResponse("matches") ;

	return true;
}
