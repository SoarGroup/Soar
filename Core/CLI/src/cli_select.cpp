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

#include "gSKI_Agent.h"
#include "sml_Names.h"

#include "decision_manipulation.h"

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

	// get soar kernel agent - bad gSKI!
	agent *my_agent = pAgent->GetSoarAgent();

	if ( !pOp )
	{
		const char *my_selection = get_selected_operator( my_agent );
		
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
		select_next_operator( my_agent, pOp->c_str() );
	
	return false;
}
