/////////////////////////////////////////////////////////////////
// numeric-indifferent-mode command file.
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
#include "sml_StringOps.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseNumericIndifferentMode(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	static struct GetOpt::option longOptions[] = {
		{"average",	0, 0, 'a'},
		{"avg",		0, 0, 'a'},
		{"sum",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	eNumericIndifferentMode mode = NUMERIC_INDIFFERENT_QUERY;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "as", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'a':
				mode = NUMERIC_INDIFFERENT_AVERAGE;
				break;
			case 's':
				mode = NUMERIC_INDIFFERENT_SUM;
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

	return DoNumericIndifferentMode(pAgent, mode);
}

bool CommandLineInterface::DoNumericIndifferentMode(gSKI::IAgent* pAgent, const eNumericIndifferentMode mode) {
	if (!RequireAgent(pAgent)) return false;

	switch (mode) {
		case 0:
			break;
		case NUMERIC_INDIFFERENT_AVERAGE:
			pAgent->SetNumericIndifferentMode(gSKI_NUMERIC_INDIFFERENT_MODE_AVG);
			return true;
		case NUMERIC_INDIFFERENT_SUM:
			pAgent->SetNumericIndifferentMode(gSKI_NUMERIC_INDIFFERENT_MODE_SUM);
			return true;
		default:
			return SetError(CLIError::kInvalidNumericIndifferentMode);
	}
	
	char buf[kMinBufferSize];
	if (m_RawOutput) {
		m_Result << "Current numeric indifferent mode: ";
	}

	switch (pAgent->GetNumericIndifferentMode()) {
		case gSKI_NUMERIC_INDIFFERENT_MODE_AVG:
			if (m_RawOutput) {
				m_Result << "average";
			} else {
				AppendArgTagFast(sml_Names::kParamNumericIndifferentMode, sml_Names::kTypeInt, Int2String((int)gSKI_NUMERIC_INDIFFERENT_MODE_AVG, buf, kMinBufferSize));
			}
			break;
		case gSKI_NUMERIC_INDIFFERENT_MODE_SUM:
			if (m_RawOutput) {
				m_Result << "sum";
			} else {
				AppendArgTagFast(sml_Names::kParamNumericIndifferentMode, sml_Names::kTypeInt, Int2String((int)gSKI_NUMERIC_INDIFFERENT_MODE_SUM, buf, kMinBufferSize));
			}
			break;
		default:
			return SetError(CLIError::kInvalidNumericIndifferentMode);
	}
	return true;
}
