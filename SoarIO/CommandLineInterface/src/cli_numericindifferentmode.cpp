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

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	unsigned int mode = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "as", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'a':
				mode = OPTION_NUMERIC_INDIFFERENT_AVERAGE;
				break;
			case 's':
				mode = OPTION_NUMERIC_INDIFFERENT_SUM;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	// No additional arguments
	if (argv.size() != (unsigned)GetOpt::optind) return m_Error.SetError(CLIError::kTooManyArgs);		

	return DoNumericIndifferentMode(pAgent, mode);
}

bool CommandLineInterface::DoNumericIndifferentMode(gSKI::IAgent* pAgent, unsigned int mode) {
	if (!RequireAgent(pAgent)) return false;

	switch (mode) {
		case 0:
			break;
		case OPTION_NUMERIC_INDIFFERENT_AVERAGE:
			pAgent->SetNumericIndifferentMode(gSKI_NUMERIC_INDIFFERENT_MODE_AVG);
			return true;
		case OPTION_NUMERIC_INDIFFERENT_SUM:
			pAgent->SetNumericIndifferentMode(gSKI_NUMERIC_INDIFFERENT_MODE_SUM);
			return true;
		default:
			return m_Error.SetError(CLIError::kInvalidNumericIndifferentMode);
	}
	
	char buf[kMinBufferSize];

	switch (pAgent->GetNumericIndifferentMode()) {
		case gSKI_NUMERIC_INDIFFERENT_MODE_AVG:
			if (m_RawOutput) {
				AppendToResult("Current numeric indifferent mode: average");
			} else {
				AppendArgTag(sml_Names::kParamNumericIndifferentMode, sml_Names::kTypeInt, Int2String((int)gSKI_NUMERIC_INDIFFERENT_MODE_AVG, buf, kMinBufferSize));
			}
			break;
		case gSKI_NUMERIC_INDIFFERENT_MODE_SUM:
			if (m_RawOutput) {
				AppendToResult("Current numeric indifferent mode: sum");
			} else {
				AppendArgTag(sml_Names::kParamNumericIndifferentMode, sml_Names::kTypeInt, Int2String((int)gSKI_NUMERIC_INDIFFERENT_MODE_SUM, buf, kMinBufferSize));
			}
			break;
		default:
			return m_Error.SetError(CLIError::kInvalidNumericIndifferentMode);
	}
	return true;
}
