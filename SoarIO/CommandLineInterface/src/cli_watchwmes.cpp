#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWatchWMEs(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"add-filter",		0, 0, 'a'},
		{"remove-filter",	0, 0, 'r'},
		{"list-filter",		0, 0, 'l'},
		{"reset-filter",	0, 0, 'R'},
		{"type",			1, 0, 't'},
		{0, 0, 0, 0}
	};

	unsigned int mode = 0;
	bool adds = false;
	bool removes = false;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":arlRt:", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'a':
				mode = OPTION_WATCH_WMES_MODE_ADD;
				break;
			case 'r':
				mode = OPTION_WATCH_WMES_MODE_REMOVE;
				break;
			case 'l':
				mode = OPTION_WATCH_WMES_MODE_LIST;
				break;
			case 'R':
				mode = OPTION_WATCH_WMES_MODE_RESET;
				break;
			case 't':
				{
					std::string typeString = m_pGetOpt->GetOptArg();
					if (typeString == "adds") {
						adds = true;
					} else if (typeString == "removes") {
						removes = true;
					} else if (typeString == "both") {
						adds = true;
						removes = true;
					} else {
						return m_Error.SetError(CLIError::kInvalidWMEFilterType);
					}
				}
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	if (mode == 0) return m_Error.SetError(CLIError::kTooFewArgs);
	
	if (mode == OPTION_WATCH_WMES_MODE_ADD || mode == OPTION_WATCH_WMES_MODE_REMOVE) {
		// type required
		if (!adds && !removes) return m_Error.SetError(CLIError::kTypeRequired);
	
		// check for too few/many args
		if (m_pGetOpt->GetAdditionalArgCount() > 3) return m_Error.SetError(CLIError::kTooManyArgs);
		if (m_pGetOpt->GetAdditionalArgCount() < 3) return m_Error.SetError(CLIError::kTooFewArgs);

		int optind = m_pGetOpt->GetOptind();
		return DoWatchWMEs(pAgent, mode, adds, removes, &argv[optind], &argv[optind + 1], &argv[optind + 2]);
	}

	// no additional arguments
	if (m_pGetOpt->GetAdditionalArgCount()) return m_Error.SetError(CLIError::kTooManyArgs);

	return DoWatchWMEs(pAgent, mode, adds, removes);
}

bool CommandLineInterface::DoWatchWMEs(gSKI::IAgent* pAgent, unsigned int mode, bool adds, bool removes, std::string* pIdString, std::string* pAttributeString, std::string* pValueString) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	int ret = 0;
	bool retb = false;
	switch (mode) {
		case OPTION_WATCH_WMES_MODE_ADD:
			if (!pIdString || !pAttributeString || !pValueString) return m_Error.SetError(CLIError::kFilterExpected);
			ret = pKernelHack->AddWMEFilter(pAgent, pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), adds, removes);
			if (ret == -1) return m_Error.SetError(CLIError::kInvalidID);
			if (ret == -2) return m_Error.SetError(CLIError::kInvalidAttribute);
			if (ret == -3) return m_Error.SetError(CLIError::kInvalidValue);
			if (ret == -4) return m_Error.SetError(CLIError::kDuplicateWMEFilter);
			break;

		case OPTION_WATCH_WMES_MODE_REMOVE:
			if (!pIdString || !pAttributeString || !pValueString) return m_Error.SetError(CLIError::kFilterExpected);
			ret = pKernelHack->RemoveWMEFilter(pAgent, pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), adds, removes);
			if (ret == -1) return m_Error.SetError(CLIError::kInvalidID);
			if (ret == -2) return m_Error.SetError(CLIError::kInvalidAttribute);
			if (ret == -3) return m_Error.SetError(CLIError::kInvalidValue);
			if (ret == -4) return m_Error.SetError(CLIError::kWMEFilterNotFound);
			break;

		case OPTION_WATCH_WMES_MODE_LIST:
			if (!adds && !removes) adds = removes = true;
			this->AddListenerAndDisableCallbacks(pAgent);
			pKernelHack->ListWMEFilters(pAgent, adds, removes);
			this->RemoveListenerAndEnableCallbacks(pAgent);
			break;

		case OPTION_WATCH_WMES_MODE_RESET:
			if (!adds && !removes) adds = removes = true;
			this->AddListenerAndDisableCallbacks(pAgent);
			retb = pKernelHack->ResetWMEFilters(pAgent, adds, removes);
			this->RemoveListenerAndEnableCallbacks(pAgent);
			if (!retb) return m_Error.SetError(CLIError::kWMEFilterNotFound);
			break;

		default:
			return m_Error.SetError(CLIError::kInvalidMode);
	}

	return true;
}
