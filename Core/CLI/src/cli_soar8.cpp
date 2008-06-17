/////////////////////////////////////////////////////////////////
// soar8 command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "cli_CLIError.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSoar8(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'e', "on",			0},
		{'e', "enable",		0},
		{'d', "off",		0},
		{'d', "disable",	0},
		{0, 0, 0}
	};

	bool query = true;
	bool soar8 = true;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				query = false;
				soar8 = false;
				break;
			case 'e':
				query = false;
				soar8 = true;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoSoar8(query ? 0 : &soar8);
}

bool CommandLineInterface::DoSoar8(bool* pSoar8) {
	if (!pSoar8) {
		// query
		if (m_RawOutput) {
			m_Result << "Soar 8 mode is " << (m_pAgentSoar->operand2_mode ? "on." : "off.");
		}
		else 
		{
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, m_pAgentSoar->operand2_mode ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	// Check that working memory is empty
	if (m_pAgentSoar->all_wmes_in_rete) 
	{
		return SetError( CLIError::kWorkingMemoryNotEmpty );
	}
	// Check that production memory is empty
	for ( unsigned i = 0 ; i < NUM_PRODUCTION_TYPES; ++i )
	{
		if ( m_pAgentSoar->num_productions_of_type[ i ] ) 
		{
			return SetError( CLIError::kProductionMemoryNotEmpty );
		}
	}

	if (*pSoar8) {
		// Turn Soar8 mode ON
		m_pAgentSoar->operand2_mode = true;

		// o-support-mode 4
		DoOSupportMode(4);
		
		// init-soar
		if (!DoInitSoar()) return false;
	} else {
		// Turn Soar8 mode OFF
		m_pAgentSoar->operand2_mode = false;

		// o-support-mode 0
		DoOSupportMode(0);

		// init-soar
		if (!DoInitSoar()) return false;
	}
	return true;
}

