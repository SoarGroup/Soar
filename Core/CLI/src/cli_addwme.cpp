/////////////////////////////////////////////////////////////////
// add-wme command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAddWME(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	if (argv.size() < 4) return SetError(CLIError::kTooFewArgs);

	unsigned attributeIndex = (argv[2] == "^") ? 3 : 2;

	if (argv.size() < (attributeIndex + 2)) return SetError(CLIError::kTooFewArgs);
    if (argv.size() > (attributeIndex + 3)) return SetError(CLIError::kTooManyArgs);

	bool acceptable = false;
	if (argv.size() > (attributeIndex + 2)) {
		if (argv[attributeIndex + 2] != "+") {
			SetErrorDetail("Got: " + argv[attributeIndex + 2]);
			return SetError(CLIError::kAcceptableOrNothingExpected);
		}
		acceptable = true;
	}

	return DoAddWME(pAgent, argv[1], argv[attributeIndex], argv[attributeIndex + 1], acceptable);
}

bool CommandLineInterface::DoAddWME(gSKI::Agent* pAgent, const std::string& id, const std::string& attribute, const std::string& value, bool acceptable) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	unsigned long timetag = pKernelHack->AddWme(pAgent, id.c_str(), attribute.c_str(), value.c_str(), acceptable);
	if (timetag < 0) {
		switch (timetag) {
			default:
			case -1:
				SetErrorDetail("Got: " + id);
				return SetError(CLIError::kInvalidID);
			case -2:
				SetErrorDetail("Got: " + attribute);
				return SetError(CLIError::kInvalidAttribute);
			case -3:
				SetErrorDetail("Got: " + value);
				return SetError(CLIError::kInvalidValue);
		}
	}

	if (m_RawOutput) {
		m_Result << "Timetag: " << timetag;
	} else {
		char buf[kMinBufferSize];
		AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(timetag, buf, sizeof(buf)));
	}
	return true;
}



