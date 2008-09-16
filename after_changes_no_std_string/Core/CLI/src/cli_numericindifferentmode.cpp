/////////////////////////////////////////////////////////////////
// numeric-indifferent-mode command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"
#include "cli_CLIError.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseNumericIndifferentMode(std::vector<std::string>& argv) {

	Options optionsData[] = 
	{
		{'a', "average",	0},
		{'a', "avg",		0},
		{'s', "sum",		0},
		{0, 0, 0}
	};

	ni_mode mode = NUMERIC_INDIFFERENT_MODE_AVG;
	bool query = true;

	for (;;) 
	{
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) 
		{
			case 'a':
				mode = NUMERIC_INDIFFERENT_MODE_AVG;
				query = false;
				break;
			case 's':
				mode = NUMERIC_INDIFFERENT_MODE_SUM;
				query = false;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No additional arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);		

	return DoNumericIndifferentMode( query, mode );
}

bool CommandLineInterface::DoNumericIndifferentMode( bool query, const ni_mode mode ) 
{
	if ( query )
	{
		if (m_RawOutput) {
			m_Result << "Current numeric indifferent mode: ";

			switch (m_pAgentSoar->numeric_indifferent_mode) {
				case NUMERIC_INDIFFERENT_MODE_AVG:
					m_Result << "average";
					break;
				case NUMERIC_INDIFFERENT_MODE_SUM:
					m_Result << "sum";
					break;
				default:
					m_Result << "unknown";
					assert( false );
					return SetError(CLIError::kInvalidNumericIndifferentMode);
			}
		}
		else
		{
			std::stringstream modeString;
			modeString << static_cast< int >( m_pAgentSoar->numeric_indifferent_mode );
			AppendArgTagFast(sml_Names::kParamNumericIndifferentMode, sml_Names::kTypeInt, modeString.str().c_str() );
		}
	}
	else // !query
	{
		m_pAgentSoar->numeric_indifferent_mode = mode;
	}

	return true;
}
