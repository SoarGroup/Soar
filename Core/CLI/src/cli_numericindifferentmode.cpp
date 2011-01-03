/////////////////////////////////////////////////////////////////
// numeric-indifferent-mode command file.
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

bool CommandLineInterface::DoNumericIndifferentMode( bool query, const ni_mode mode ) 
{
    agent* agnt = m_pAgentSML->GetSoarAgent();
	if ( query )
	{
		if (m_RawOutput) {
			m_Result << "Current numeric indifferent mode: ";

			switch (agnt->numeric_indifferent_mode) {
                default:
				case NUMERIC_INDIFFERENT_MODE_AVG:
					m_Result << "average";
					break;
				case NUMERIC_INDIFFERENT_MODE_SUM:
					m_Result << "sum";
					break;
			}
		}
		else
		{
			std::stringstream modeString;
			modeString << static_cast< int >( agnt->numeric_indifferent_mode );
			AppendArgTagFast(sml_Names::kParamNumericIndifferentMode, sml_Names::kTypeInt, modeString.str() );
		}
	}
	else // !query
	{
		agnt->numeric_indifferent_mode = mode;
	}

	return true;
}
