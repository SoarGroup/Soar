#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

#include "IgSKI_Agent.h"

using namespace cli;

bool CommandLineInterface::ParseIndifferentSelection(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"ask",		0, 0, 'a'},
		{"first",	0, 0, 'f'},
		{"last",	0, 0, 'l'},
		{"random",	0, 0, 'r'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	unsigned int mode = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "aflr", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'a':
				mode = OPTION_INDIFFERENT_ASK;
				break;
			case 'f':
				mode = OPTION_INDIFFERENT_FIRST;
				break;
			case 'l':
				mode = OPTION_INDIFFERENT_LAST;
				break;
			case 'r':
				mode = OPTION_INDIFFERENT_RANDOM;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLIIndifferentSelection, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// No additional arguments
	if (argv.size() != (unsigned)GetOpt::optind) return HandleSyntaxError(Constants::kCLIIndifferentSelection, Constants::kCLITooManyArgs);		


	return DoIndifferentSelection(pAgent, mode);
}

bool CommandLineInterface::DoIndifferentSelection(gSKI::IAgent* pAgent, unsigned int mode) {
	if (!RequireAgent(pAgent)) return false;

	if (!mode) {
		// query
		switch (pAgent->GetIndifferentSelection()) {
			case gSKI_USER_SELECT_FIRST:
				AppendToResult("first");
				break;
			case gSKI_USER_SELECT_LAST:
				AppendToResult("last");
				break;
			case gSKI_USER_SELECT_ASK:
				AppendToResult("ask");
				break;
			case gSKI_USER_SELECT_RANDOM:
				AppendToResult("random");
				break;
			default:
				return HandleError("Invalid indifferent selection mode returned from kernel.");
		}
	} else {
		switch (mode) {
			case OPTION_INDIFFERENT_FIRST:
				pAgent->SetIndifferentSelection(gSKI_USER_SELECT_FIRST);
				break;
			case OPTION_INDIFFERENT_LAST:
				pAgent->SetIndifferentSelection(gSKI_USER_SELECT_LAST);
				break;
			case OPTION_INDIFFERENT_ASK:
				pAgent->SetIndifferentSelection(gSKI_USER_SELECT_ASK);
				break;
			case OPTION_INDIFFERENT_RANDOM:
				pAgent->SetIndifferentSelection(gSKI_USER_SELECT_RANDOM);
				break;
			default:
				return HandleError("Invalid indifferent selection mode.");

		}
	}
	return true;
}

