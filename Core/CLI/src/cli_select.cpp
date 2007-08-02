/////////////////////////////////////////////////////////////////
// select command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, 
//         Nate Derbinsky, nlderbin@umich.edu
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSelect( gSKI::Agent* pAgent, std::vector<std::string>& argv ) 
{
	// At most one argument to select the next operator
	if ( argv.size() > 2 ) 
		return SetError( CLIError::kTooManyArgs );
	
	if ( argv.size() == 2 )
		return DoSelect( pAgent, &( argv[1] ) );
	
	return DoSelect( pAgent );
}

bool CommandLineInterface::DoSelect( gSKI::Agent* pAgent, const std::string* pOp ) 
{
	if ( !RequireAgent( pAgent ) ) 
		return false;

	if ( !pOp )
		m_Result << "current op";
	else
		m_Result << "select: " << *pOp;
	
	return SetError( CLIError::kCommandNotImplemented );
}
