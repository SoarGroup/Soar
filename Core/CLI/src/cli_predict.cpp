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

#include "sml_Names.h"

#include "decision_manipulation.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoPredict() 
{
    agent* agnt = m_pAgentSML->GetSoarAgent();
	const char *prediction_result = predict_get( agnt );

	if ( m_RawOutput )
		m_Result << prediction_result;
	else
		AppendArgTagFast( sml_Names::kParamMessage, sml_Names::kTypeString, prediction_result );

	return true;
}
