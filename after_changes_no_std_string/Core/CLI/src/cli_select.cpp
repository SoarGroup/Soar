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
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "decision_manipulation.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSelect( std::vector<std::string>& argv ) 
{
	// At most one argument to select the next operator
	if ( argv.size() > 2 ) 
		return SetError( CLIError::kTooManyArgs );
	
	if ( argv.size() == 2 )
		return DoSelect( &( argv[1] ) );
	
	return DoSelect( );
}

bool CommandLineInterface::DoSelect( const std::string* pOp ) 
{
	if ( !pOp )
	{
		const char *my_selection = select_get_operator( m_pAgentSoar );
		
		if ( my_selection != NULL )
		{
			if ( m_RawOutput )
				m_Result << my_selection;
			else
				AppendArgTagFast( sml_Names::kOperator_ID, sml_Names::kTypeID, my_selection );
		}
		else
		{
			if ( m_RawOutput )
				m_Result << "No operator selected.";
			else
				AppendArgTagFast( sml_Names::kParamMessage, sml_Names::kTypeString, "No operator selected." );
		}
	}
	else
		select_next_operator( m_pAgentSoar, pOp->c_str() );
	
	return false;
}
