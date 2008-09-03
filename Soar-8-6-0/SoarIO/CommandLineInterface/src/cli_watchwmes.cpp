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

	eWatchWMEsMode mode = WATCH_WMES_LIST;
	WatchWMEsTypeBitset type(0);

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":arlRt:", longOptions, 0);
		if (option == -1) break;

		switch (option) {
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
					std::string typeString = m_pGetOpt->GetOptArg();
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
			case '?':
				{
					std::string detail;
					if (m_pGetOpt->GetOptOpt()) {
						detail = static_cast<char>(m_pGetOpt->GetOptOpt());
					} else {
						detail = argv[m_pGetOpt->GetOptind() - 1];
					}
					SetErrorDetail("Bad option '" + detail + "'.");
				}
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}
	
	if (mode == WATCH_WMES_ADD || mode == WATCH_WMES_REMOVE) {
		// type required
		if (type.none()) return SetError(CLIError::kTypeRequired);
	
		// check for too few/many args
		if (m_pGetOpt->GetAdditionalArgCount() > 3) return SetError(CLIError::kTooManyArgs);
		if (m_pGetOpt->GetAdditionalArgCount() < 3) return SetError(CLIError::kTooFewArgs);

		int optind = m_pGetOpt->GetOptind();
		return DoWatchWMEs(pAgent, mode, type, &argv[optind], &argv[optind + 1], &argv[optind + 2]);
	}

	// no additional arguments
	if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);

	return DoWatchWMEs(pAgent, mode, type);
}

bool CommandLineInterface::DoWatchWMEs(gSKI::IAgent* pAgent, const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString, const std::string* pAttributeString, const std::string* pValueString) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	int ret = 0;
	bool retb = false;
	switch (mode) {
		case WATCH_WMES_ADD:
			if (!pIdString || !pAttributeString || !pValueString) return SetError(CLIError::kFilterExpected);
			ret = pKernelHack->AddWMEFilter(pAgent, pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
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
			ret = pKernelHack->RemoveWMEFilter(pAgent, pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
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

			this->AddListenerAndDisableCallbacks(pAgent);
			pKernelHack->ListWMEFilters(pAgent, type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
			this->RemoveListenerAndEnableCallbacks(pAgent);
			break;

		case WATCH_WMES_RESET:
			if (type.none()) type.flip();

			this->AddListenerAndDisableCallbacks(pAgent);
			retb = pKernelHack->ResetWMEFilters(pAgent, type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
			this->RemoveListenerAndEnableCallbacks(pAgent);

			if (!retb) return SetError(CLIError::kWMEFilterNotFound);
			break;

		default:
			return SetError(CLIError::kInvalidMode);
	}

	return true;
}
