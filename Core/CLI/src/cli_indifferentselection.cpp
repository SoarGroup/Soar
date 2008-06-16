/////////////////////////////////////////////////////////////////
// indifferent-selection command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseIndifferentSelection(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "ask",	0},
		{'f', "first",	0},
		{'l', "last",	0},
		{'r', "random",	0},
		{0, 0, 0}
	};

	eIndifferentMode mode = INDIFFERENT_QUERY;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
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
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No additional arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);		


	return DoIndifferentSelection(mode);
}

bool CommandLineInterface::DoIndifferentSelection(eIndifferentMode mode) {
	if (mode == INDIFFERENT_QUERY) {
		// query
		char buf[kMinBufferSize];

		switch (m_pAgentSoar->sysparams[USER_SELECT_MODE_SYSPARAM]) {
			case USER_SELECT_FIRST:
				if (m_RawOutput) {
					m_Result << "first";
				} else {
					AppendArgTagFast(sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeInt, Int2String((int)USER_SELECT_FIRST, buf, kMinBufferSize));
				}
				break;

			case USER_SELECT_LAST:
				if (m_RawOutput) {
					m_Result << "last";
				} else {
					AppendArgTagFast(sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeInt, Int2String((int)USER_SELECT_LAST, buf, kMinBufferSize));
				}
				break;

			case USER_SELECT_ASK:
				if (m_RawOutput) {
					m_Result << "ask";
				} else {
					AppendArgTagFast(sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeInt, Int2String((int)USER_SELECT_ASK, buf, kMinBufferSize));
				}
				break;

			case USER_SELECT_RANDOM:
				if (m_RawOutput) {
					m_Result << "random";
				} else {
					AppendArgTagFast(sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeInt, Int2String((int)USER_SELECT_RANDOM, buf, kMinBufferSize));
				}
				break;

			default:
				return SetError(CLIError::kInvalidIndifferentSelectionMode);
		}
	} else {
		switch (mode) {
			case INDIFFERENT_FIRST:
				m_pAgentSoar->sysparams[USER_SELECT_MODE_SYSPARAM] = USER_SELECT_FIRST;
				break;
			case INDIFFERENT_LAST:
				m_pAgentSoar->sysparams[USER_SELECT_MODE_SYSPARAM] = USER_SELECT_LAST;
				break;
			case INDIFFERENT_ASK:
				return SetError(CLIError::kNotImplemented);
				//m_pAgentSoar->sysparams[USER_SELECT_MODE_SYSPARAM] = USER_SELECT_ASK;
				//break;
			case INDIFFERENT_RANDOM:
				m_pAgentSoar->sysparams[USER_SELECT_MODE_SYSPARAM] = USER_SELECT_RANDOM;
				break;
			default:
				return SetError(CLIError::kInvalidIndifferentSelectionMode);

		}
	}
	return true;
}

