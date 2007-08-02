/////////////////////////////////////////////////////////////////
// rl command file.
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

bool CommandLineInterface::ParseRL( gSKI::Agent* pAgent, std::vector<std::string>& argv ) 
{
	Options optionsData[] = 
	{
		{'g', "get",	0},
		{'s', "set",	0},
		{'S', "stat",	0},
		{0, 0, 0} // null
	};
	RLBitset options(0);
	
	for (;;) 
	{
		if ( !ProcessOptions( argv, optionsData ) ) 
			return false;
		
		if (m_Option == -1) break;
		
		switch (m_Option) 
		{
			case 'g':
				options.set( RL_GET );
				break;
			
			case 's':
				options.set( RL_SET );
				break;
				
			case 'S':
				options.set( RL_STAT );
				break;
				
			default:
				return SetError( CLIError::kGetOptError );
		}
	}
	
	// bad: more than one option
	if ( options.count() > 1 )
	{
		SetErrorDetail( "rl takes only one option at a time." );
		return SetError( CLIError::kTooManyArgs );
	}
	
	// bad: no option, but more than one argument
	if ( !options.count() && ( argv.size() > 1 ) )
		return SetError( CLIError::kTooManyArgs );
	
	// case: nothing = full configuration information
	if ( argv.size() == 1 )
		return DoRL( pAgent );
	
	// case: get requires one non-option argument
	else if ( options.test( RL_GET ) )
	{
		if ( m_NonOptionArguments < 1 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 1 )
			return SetError( CLIError::kTooManyArgs );
		
		// check attribute name here
		if ( argv[2].compare( "test" ) == 0 )
			return DoRL( pAgent, 'g', &( argv[2] ) );
		else
			return SetError( CLIError::kInvalidAttribute );
	}
	
	// case: set requires two non-option arguments
	else if ( options.test( RL_SET ) )
	{
		if ( m_NonOptionArguments < 2 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 2 )
			return SetError( CLIError::kTooManyArgs );
		
		// check attribute name/potential vals here
		if ( argv[2].compare( "test" ) == 0 )
			if ( argv[3].compare( "testing" ) == 0 )
				return DoRL( pAgent, 's', &( argv[2] ), &( argv[3] ) );
			else
				return SetError( CLIError::kInvalidValue );
		else
			return SetError( CLIError::kInvalidAttribute );
	}
	
	// case: stat can do zero or one non-option arguments
	else if ( options.test( RL_STAT ) )
	{
		if ( m_NonOptionArguments == 0 )
			return DoRL( pAgent, 'S' );
		else if ( m_NonOptionArguments == 1 )
		{
			// check attribute name
			if ( argv[2].compare( "test" ) == 0 )
				return DoRL( pAgent, 'S', &( argv[2] ) );
			else
				return SetError( CLIError::kInvalidAttribute );
		}
		else
			return SetError( CLIError::kTooManyArgs );
	}
	
	// not sure why you'd get here
	return false;
}

bool CommandLineInterface::DoRL( gSKI::Agent* pAgent, const char pOp, const std::string* pAttr, const std::string* pVal ) 
{
	if ( !RequireAgent( pAgent ) ) 
		return false;

	if ( !pOp )
		m_Result << "all info";
	else if ( pOp == 'g' )
		m_Result << "get: " << *pAttr;
	else if ( pOp == 's' )
		m_Result << "set: " << *pAttr << " = " << *pVal;
	else if ( pOp == 'S' )
	{
		if ( !pAttr )
			m_Result << "all stats";
		else
			m_Result << "stat: " << *pAttr;
	}
	
	return SetError( CLIError::kCommandNotImplemented );
}
