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

bool CommandLineInterface::ParseIndifferentSelection(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"ask",		0, 0, 'a'},
		{"first",	0, 0, 'f'},
		{"last",	0, 0, 'l'},
		{"random",	0, 0, 'r'},
		{0, 0, 0, 0}
	};

	eIndifferentMode mode = INDIFFERENT_QUERY;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "aflr", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'a':
				mode = INDIFFERENT_ASK;
				break;
			case 'f':
				mode = INDIFFERENT_FIRST;
				break;
			case 'l':
				mode = INDIFFERENT_LAST;
				break;
			case 'r':
				mode = INDIFFERENT_RANDOM;
				break;
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No additional arguments
	if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);		


	return DoIndifferentSelection(pAgent, mode);
}

bool CommandLineInterface::DoIndifferentSelection(gSKI::IAgent* pAgent, eIndifferentMode mode) {
	if (!RequireAgent(pAgent)) return false;

	if (mode == INDIFFERENT_QUERY) {
		// query
		char buf[kMinBufferSize];

		switch (pAgent->GetIndifferentSelection()) {
			case gSKI_USER_SELECT_FIRST:
				if (m_RawOutput) {
					m_Result << "first";
				} else {
					AppendArgTagFast(sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeInt, Int2String((int)gSKI_USER_SELECT_FIRST, buf, kMinBufferSize));
				}
				break;

			case gSKI_USER_SELECT_LAST:
				if (m_RawOutput) {
					m_Result << "last";
				} else {
					AppendArgTagFast(sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeInt, Int2String((int)gSKI_USER_SELECT_LAST, buf, kMinBufferSize));
				}
				break;

			case gSKI_USER_SELECT_ASK:
				if (m_RawOutput) {
					m_Result << "ask";
				} else {
					AppendArgTagFast(sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeInt, Int2String((int)gSKI_USER_SELECT_ASK, buf, kMinBufferSize));
				}
				break;

			case gSKI_USER_SELECT_RANDOM:
				if (m_RawOutput) {
					m_Result << "random";
				} else {
					AppendArgTagFast(sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeInt, Int2String((int)gSKI_USER_SELECT_RANDOM, buf, kMinBufferSize));
				}
				break;

			default:
				return SetError(CLIError::kInvalidIndifferentSelectionMode);
		}
	} else {
		switch (mode) {
			case INDIFFERENT_FIRST:
				pAgent->SetIndifferentSelection(gSKI_USER_SELECT_FIRST);
				break;
			case INDIFFERENT_LAST:
				pAgent->SetIndifferentSelection(gSKI_USER_SELECT_LAST);
				break;
			case INDIFFERENT_ASK:
				pAgent->SetIndifferentSelection(gSKI_USER_SELECT_ASK);
				break;
			case INDIFFERENT_RANDOM:
				pAgent->SetIndifferentSelection(gSKI_USER_SELECT_RANDOM);
				break;
			default:
				return SetError(CLIError::kInvalidIndifferentSelectionMode);

		}
	}
	return true;
}

