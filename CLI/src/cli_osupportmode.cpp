/////////////////////////////////////////////////////////////////
// o-support-mode command file.
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

#include "gSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseOSupportMode(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	int mode = -1;
	if (argv.size() == 2) {
		if (!isdigit(argv[1][0])) {
			SetErrorDetail("Expected an integer from 0 to 4.");
			return SetError(CLIError::kIntegerOutOfRange);
		}
		mode = atoi(argv[1].c_str());
		if (mode < 0 || mode > 4) {
			SetErrorDetail("Expected an integer from 0 to 4.");
			return SetError(CLIError::kIntegerOutOfRange);
		}
	}

	return DoOSupportMode(pAgent, mode);
}

bool CommandLineInterface::DoOSupportMode(gSKI::Agent* pAgent, int mode) {

	if (!RequireAgent(pAgent)) return false;

	if (mode < 0) {
		switch (pAgent->GetOSupportMode()) {
			case gSKI_O_SUPPORT_MODE_0:
				mode = 0;
				break;
			case gSKI_O_SUPPORT_MODE_2:
				mode = 2;
				break;
			case gSKI_O_SUPPORT_MODE_3:
				mode = 3;
				break;
			case gSKI_O_SUPPORT_MODE_4:
				mode = 4;
				break;
			default:
				return SetError(CLIError::kInvalidOSupportMode);
		}

		if (m_RawOutput) {
			m_Result << mode;
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(mode, buf, kMinBufferSize));
		}
	} else {

		egSKIOSupportMode oSupportMode = gSKI_O_SUPPORT_MODE_3;
		switch (mode) {
			case 0:
				oSupportMode = gSKI_O_SUPPORT_MODE_0;
				break;
			case 2:
				oSupportMode = gSKI_O_SUPPORT_MODE_2;
				break;
			case 3:
				oSupportMode = gSKI_O_SUPPORT_MODE_3;
				break;
			case 4:
				oSupportMode = gSKI_O_SUPPORT_MODE_4;
				break;
			default:
				return SetError(CLIError::kInvalidOSupportMode);
		}
		pAgent->SetOSupportMode(oSupportMode);
	}

	return true;
}

