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
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "decision_manipulation.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePredict( std::vector<std::string>& argv ) 
{
	// No arguments to predict next operator
	if ( argv.size() != 1 ) 
	{
		SetErrorDetail( "predict takes no arguments." );
		return SetError( CLIError::kTooManyArgs );
	}
	
	return DoPredict( );
}

bool CommandLineInterface::DoPredict() 
{
	const char *prediction_result = predict_get( m_pAgentSoar );

	if ( m_RawOutput )
		m_Result << prediction_result;
	else
		AppendArgTagFast( sml_Names::kParamMessage, sml_Names::kTypeString, prediction_result );

	return true;
}
