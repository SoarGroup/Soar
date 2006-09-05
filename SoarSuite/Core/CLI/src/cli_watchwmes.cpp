/////////////////////////////////////////////////////////////////
// watch-wmes command file.
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

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWatchWMEs(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "add-filter",		0},
		{'r', "remove-filter",	0},
		{'l', "list-filter",	0},
		{'R', "reset-filter",	0},
		{'t', "type",			1},
		{0, 0, 0}
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
		return DoWatchWMEs(pAgent, mode, type, &argv[optind], &argv[optind + 1], &argv[optind + 2]);
	}

	// no additional arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoWatchWMEs(pAgent, mode, type);
}

bool CommandLineInterface::DoWatchWMEs(gSKI::Agent* pAgent, const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString, const std::string* pAttributeString, const std::string* pValueString) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

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
