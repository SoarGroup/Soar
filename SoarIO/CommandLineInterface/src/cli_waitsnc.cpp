/////////////////////////////////////////////////////////////////
// wait-snc command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWaitSNC(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"disable",	0, 0, 'd'},
		{"enable",	0, 0, 'e'},
		{"off",		0, 0, 'd'},
		{"on",		0, 0, 'e'},
		{0, 0, 0, 0}
	};

	bool query = true;
	bool enable = false;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "de", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'd':
				query = false;
				enable = false;
				break;
			case 'e':
				query = false;
				enable = true;
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

	// No additional arguments
	if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);		

	return DoWaitSNC(pAgent, query ? 0 : &enable);
}

bool CommandLineInterface::DoWaitSNC(gSKI::IAgent* pAgent, bool* pSetting) {
	if (!RequireAgent(pAgent)) return false;

	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Current waitsnc setting: " << pAgent->IsWaitingOnStateNoChange() ? "enabled" : "disabled";
		} else {
			AppendArgTagFast(sml_Names::kParamWaitSNC, sml_Names::kTypeBoolean, pAgent->IsWaitingOnStateNoChange() ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	pAgent->SetWaitOnStateNoChange(*pSetting);
	return true;
}
