/////////////////////////////////////////////////////////////////
// predict command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, 
//         Nate Derbinsky, nlderbin@umich.edu
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "gSKI_Agent.h"
#include "sml_Names.h"

#include "decision_manipulation.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePredict( gSKI::Agent* pAgent, std::vector<std::string>& argv ) 
{
	// No arguments to predict next operator
	if ( argv.size() != 1 ) 
	{
		SetErrorDetail( "predict takes no arguments." );
		return SetError( CLIError::kTooManyArgs );
	}
	
	return DoPredict( pAgent );
}

bool CommandLineInterface::DoPredict( gSKI::Agent* pAgent ) 
{
	if ( !RequireAgent( pAgent ) ) 
		return false;

	// get soar kernel agent - bad gSKI!
	agent *my_agent = pAgent->GetSoarAgent();

	const char *prediction_result = get_prediction( my_agent );

	if ( m_RawOutput )
		m_Result << prediction_result;
	else
		AppendArgTagFast( sml_Names::kParamMessage, sml_Names::kTypeString, prediction_result );

	return true;
}
