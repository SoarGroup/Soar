#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;

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

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int wmeDetail = 0;
	unsigned int matches = OPTION_MATCHES_ASSERTIONS_RETRACTIONS;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "012acnrtw", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case '0':
			case 'n':
			case 'c':
				wmeDetail = 0;
				break;
			case '1':
			case 't':
				wmeDetail = 1;
				break;
			case '2':
			case 'w':
				wmeDetail = 2;
				break;
			case 'a':
				matches = OPTION_MATCHES_ASSERTIONS;
				break;
			case 'r':
				matches = OPTION_MATCHES_RETRACTIONS;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	// Max one additional argument and it is a production
	if ((argv.size() - GetOpt::optind) > 1) return m_Error.SetError(CLIError::kTooManyArgs);		

	std::string production;
	if ((argv.size() - GetOpt::optind) == 1) {
		matches = OPTION_MATCHES_PRODUCTION;
		production = argv[GetOpt::optind];
	}

	return DoMatches(pAgent, matches, wmeDetail, production);
}

bool CommandLineInterface::DoMatches(gSKI::IAgent* pAgent, unsigned int matches, int wmeDetail, const std::string& production) {

	if (!RequireAgent(pAgent)) return false;
	if (!RequireKernel()) return false;

	wme_trace_type wtt = 0;
	switch (wmeDetail) {
		case 0:
			wtt = NONE_WME_TRACE;
			break;
		case 1:
			wtt = TIMETAG_WME_TRACE;
			break;
		case 2:
			wtt = FULL_WME_TRACE;
			break;
		default:
			return m_Error.SetError(CLIError::kInvalidWMEDetail);
	}

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (matches == OPTION_MATCHES_PRODUCTION) {
		rete_node* prod = 0;

		prod = pKernelHack->NameToProduction(pAgent, const_cast<char*>(production.c_str()));

		if (!prod) return m_Error.SetError(CLIError::kProductionNotFound);

		pKernelHack->PrintPartialMatchInformation(pAgent, prod, wtt);
		return true;
	}

    ms_trace_type mst = MS_ASSERT_RETRACT;
	if (matches == OPTION_MATCHES_ASSERTIONS) mst = MS_ASSERT;
	if (matches == OPTION_MATCHES_RETRACTIONS) mst = MS_RETRACT;

    pKernelHack->PrintMatchSet(pAgent, wtt, mst);
	return true;
}
