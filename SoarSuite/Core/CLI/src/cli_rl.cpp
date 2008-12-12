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
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "reinforcement_learning.h"
#include "misc.h"

#include <vector>
#include <map>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRL( std::vector<std::string>& argv ) 
{
	Options optionsData[] = 
	{
		{'g', "get",	OPTARG_NONE},
		{'s', "set",	OPTARG_NONE},
		{'S', "stats",	OPTARG_NONE},
		{0, 0, OPTARG_NONE} // null
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
		return DoRL( );
	
	// case: get requires one non-option argument
	else if ( options.test( RL_GET ) )
	{
		if ( m_NonOptionArguments < 1 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 1 )
			return SetError( CLIError::kTooManyArgs );
		
		// check attribute name here
		if ( rl_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
			return DoRL( 'g', &( argv[2] ) );
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
		if ( rl_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
		{
			switch ( rl_get_parameter_type( m_pAgentSoar, argv[2].c_str() ) )
			{
				case rl_param_string:
					if ( !rl_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), argv[3].c_str() ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoRL( 's', &( argv[2] ), &( argv[3] ) );
					break;
					
				case rl_param_number:
					double temp;
					from_string( temp, argv[3] );
					if ( !rl_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), temp ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoRL( 's', &( argv[2] ), &( argv[3] ) );
					break;
					
				case rl_param_invalid:
					return SetError( CLIError::kInvalidAttribute );
					break;
			}
		}
		else
			return SetError( CLIError::kInvalidAttribute );
	}
	
	// case: stat can do zero or one non-option arguments
	else if ( options.test( RL_STAT ) )
	{
		if ( m_NonOptionArguments == 0 )
			return DoRL( 'S' );
		else if ( m_NonOptionArguments == 1 )
		{
			// check attribute name
			if ( rl_valid_stat( m_pAgentSoar, argv[2].c_str() ) )
				return DoRL( 'S', &( argv[2] ) );
			else
				return SetError( CLIError::kInvalidAttribute );
		}
		else
			return SetError( CLIError::kTooManyArgs );
	}
	
	// not sure why you'd get here
	return false;
}

bool CommandLineInterface::DoRL( const char pOp, const std::string* pAttr, const std::string* pVal ) 
{
	if ( !pOp )
	{
		std::string temp;
		std::string *temp2;
		double temp_val;
		
		temp = "Soar-RL learning: ";
		temp += rl_get_parameter( m_pAgentSoar, (const long) RL_PARAM_LEARNING, RL_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "temporal-extension: ";
		temp += rl_get_parameter( m_pAgentSoar, RL_PARAM_TEMPORAL_EXTENSION, RL_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}
						
		temp = "Discount";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		temp = "--------";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
						
		temp = "discount-rate: ";
		temp_val = rl_get_parameter( m_pAgentSoar, RL_PARAM_DISCOUNT_RATE );
		temp2 = to_string( temp_val );
		temp += (*temp2);
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}
		
				
		temp = "Learning";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		temp = "--------";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "learning-policy: ";
		temp += rl_get_parameter( m_pAgentSoar, RL_PARAM_LEARNING_POLICY, RL_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "learning-rate: ";
		temp_val = rl_get_parameter( m_pAgentSoar, RL_PARAM_LEARNING_RATE );
		temp2 = to_string( temp_val );
		temp += (*temp2);
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );			
		}
		
		temp = "hrl-discount: ";
		temp += rl_get_parameter( m_pAgentSoar, RL_PARAM_HRL_DISCOUNT, RL_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}
		
		temp = "Eligibility Traces";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		temp = "------------------";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "eligibility-trace-decay-rate: ";
		temp_val = rl_get_parameter( m_pAgentSoar, RL_PARAM_ET_DECAY_RATE );
		temp2 = to_string( temp_val );
		temp += (*temp2);
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "eligibility-trace-tolerance: ";
		temp_val = rl_get_parameter( m_pAgentSoar, RL_PARAM_ET_TOLERANCE );
		temp2 = to_string( temp_val );
		temp += (*temp2);
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}
		
		return true;
	}
	else if ( pOp == 'g' )
	{
		std::string output = "";
		std::string *temp2;
		const char *tag_type = sml_Names::kTypeString;
		
		switch ( rl_get_parameter_type( m_pAgentSoar, pAttr->c_str() ) )
		{
			case rl_param_string:
				output += rl_get_parameter( m_pAgentSoar, pAttr->c_str(), RL_RETURN_STRING );
				break;
				
			case rl_param_number:
				double temp = rl_get_parameter( m_pAgentSoar, pAttr->c_str() );
				temp2 = to_string( temp );
				output += (*temp2);
				delete temp2;
				tag_type = sml_Names::kTypeDouble;
				break;
		}
					
		if ( m_RawOutput )
			m_Result << output;
		else
			AppendArgTagFast( sml_Names::kParamValue, tag_type, output.c_str() );
		
		return true;
	}
	else if ( pOp == 's' )
	{
		switch ( rl_get_parameter_type( m_pAgentSoar, pAttr->c_str() ) )
		{
			case rl_param_string:
				return rl_set_parameter( m_pAgentSoar, pAttr->c_str(), pVal->c_str() );
				break;
				
			case rl_param_number:
				double temp;
				from_string( temp, *pVal );
				return rl_set_parameter( m_pAgentSoar, pAttr->c_str(), temp );
				
				break;
				
			case rl_param_invalid:
				return false;
				break;
		}
	}
	else if ( pOp == 'S' )
	{
		if ( !pAttr )
		{
			double temp;
			std::string output;
			std::string *temp_str;
			
			output = "Error from last update: ";
			temp = rl_get_stat( m_pAgentSoar, (const long) RL_STAT_UPDATE_ERROR );
			temp_str = to_string( temp );
			output += (*temp_str);
			delete temp_str;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
			
			output = "Total reward in last cycle: ";
			temp = rl_get_stat( m_pAgentSoar, RL_STAT_TOTAL_REWARD );
			temp_str = to_string( temp );
			output += (*temp_str);
			delete temp_str;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
			
			output = "Global reward since init: ";
			temp = rl_get_stat( m_pAgentSoar, RL_STAT_GLOBAL_REWARD );
			temp_str = to_string( temp );
			output += (*temp_str);
			delete temp_str;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
		}
		else
		{
			double temp = rl_get_stat( m_pAgentSoar, pAttr->c_str() );
			std::string *temp_str = to_string( temp );
			std::string output = (*temp_str);
			delete temp_str;
			
			if ( m_RawOutput )
				m_Result << output;
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeDouble, output.c_str() );
		}
		
		return true;
	}
	
	return SetError( CLIError::kCommandNotImplemented );
}
