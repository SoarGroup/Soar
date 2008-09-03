#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

#include <assert.h>

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseMatches(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"assertions",	0, 0, 'a'},
		{"count",		0, 0, 'c'},
		{"names",		0, 0, 'n'},
		{"retractions",	0, 0, 'r'},
		{"timetags",	0, 0, 't'},
		{"wmes",		0, 0, 'w'},
		{0, 0, 0, 0}
	};

	eWMEDetail detail = WME_DETAIL_NONE;
	eMatchesMode mode = MATCHES_ASSERTIONS_RETRACTIONS;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "012acnrtw", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case '0':
			case 'n':
			case 'c':
				detail = WME_DETAIL_NONE;
				break;
			case '1':
			case 't':
				detail = WME_DETAIL_TIMETAG;
				break;
			case '2':
			case 'w':
				detail = WME_DETAIL_FULL;
				break;
			case 'a':
				mode = MATCHES_ASSERTIONS;
				break;
			case 'r':
				mode = MATCHES_RETRACTIONS;
				break;
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Max one additional argument and it is a production
	if (m_pGetOpt->GetAdditionalArgCount() > 1) return SetError(CLIError::kTooManyArgs);		

	if (m_pGetOpt->GetAdditionalArgCount() == 1) return DoMatches(pAgent, MATCHES_PRODUCTION, detail, &argv[m_pGetOpt->GetOptind()]);

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
		if (!prod) return SetError(CLIError::kProductionNotFound);

		AddListenerAndDisableCallbacks(pAgent);		
		pKernelHack->PrintPartialMatchInformation(pAgent, prod, wtt);
		RemoveListenerAndEnableCallbacks(pAgent);

	} else {
		ms_trace_type mst = MS_ASSERT_RETRACT;
		if (mode == MATCHES_ASSERTIONS) mst = MS_ASSERT;
		if (mode == MATCHES_RETRACTIONS) mst = MS_RETRACT;

		AddListenerAndDisableCallbacks(pAgent);		
		pKernelHack->PrintMatchSet(pAgent, wtt, mst);
		RemoveListenerAndEnableCallbacks(pAgent);
	}

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	return true;
}
