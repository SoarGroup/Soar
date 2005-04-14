#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParseSoar8(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	static struct GetOpt::option longOptions[] = {
		{"on",		0, 0, 'e'},
		{"enable",	0, 0, 'e'},
		{"off",		0, 0, 'd'},
		{"disable", 0, 0, 'd'},
		{0, 0, 0, 0}
	};

	bool query = true;
	bool soar8 = true;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "de", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'd':
				query = false;
				soar8 = false;
				break;
			case 'e':
				query = false;
				soar8 = true;
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

	// No non-option arguments
	if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);

	return DoSoar8(query ? &soar8 : 0);
}

bool CommandLineInterface::DoSoar8(bool* pSoar8) {
	unused(pSoar8);
	SetError(CLIError::kNotImplemented);
	return false;
}

