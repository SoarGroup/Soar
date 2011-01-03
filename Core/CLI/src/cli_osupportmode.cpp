/////////////////////////////////////////////////////////////////
// o-support-mode command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoOSupportMode(int mode) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
	if (mode < 0) {
		mode = agnt->o_support_calculation_type;

		if (m_RawOutput) {
			m_Result << mode;
		} else {
			std::stringstream buffer;
			buffer << mode;
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, buffer.str());
		}
	} else {
		assert(mode != 1);
		agnt->o_support_calculation_type = mode;
	}

	return true;
}

