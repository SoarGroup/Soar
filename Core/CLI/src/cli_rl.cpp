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
		soar_module::param *my_param = m_pAgentSoar->rl_params->get( argv[2].c_str() );
		if ( my_param )
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
		soar_module::param *my_param = m_pAgentSoar->rl_params->get( argv[2].c_str() );
		if ( my_param )
		{
			if ( !my_param->validate_string( argv[3].c_str() ) )
				return SetError( CLIError::kInvalidAttribute );
			else
				return DoRL( 's', &( argv[2] ), &( argv[3] ) );
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
			soar_module::stat *my_stat = m_pAgentSoar->rl_stats->get( argv[2].c_str() );

			if ( my_stat )
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
		char *temp2;
		
		temp = "Soar-RL learning: ";
		temp2 = m_pAgentSoar->rl_params->learning->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "temporal-extension: ";
		temp2 = m_pAgentSoar->rl_params->temporal_extension->get_string();
		temp += temp2;
		delete temp2;
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
		temp2 = m_pAgentSoar->rl_params->discount_rate->get_string();
		temp += temp2;
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
		temp2 = m_pAgentSoar->rl_params->learning_policy->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "learning-rate: ";
		temp2 = m_pAgentSoar->rl_params->learning_rate->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );			
		}
		
		temp = "hrl-discount: ";
		temp2 = m_pAgentSoar->rl_params->hrl_discount->get_string();
		temp += temp2;
		delete temp2;
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
		temp2 = m_pAgentSoar->rl_params->et_decay_rate->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "eligibility-trace-tolerance: ";
		temp2 = m_pAgentSoar->rl_params->et_tolerance->get_string();
		temp += temp2;
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
		soar_module::param *my_param = m_pAgentSoar->rl_params->get( pAttr->c_str() );
		char *temp2 = my_param->get_string();
		std::string output( temp2 );
		delete temp2;
					
		if ( m_RawOutput )
			m_Result << output;
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
		
		return true;
	}
	else if ( pOp == 's' )
	{
		soar_module::param *my_param = m_pAgentSoar->rl_params->get( pAttr->c_str() );
		return my_param->set_string( pVal->c_str() );
	}
	else if ( pOp == 'S' )
	{
		if ( !pAttr )
		{
			std::string output;
			const char *temp;
			
			output = "Error from last update: ";
			temp = m_pAgentSoar->rl_stats->update_error->get_string();			
			output += temp;
			delete temp;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
			
			output = "Total reward in last cycle: ";
			temp = m_pAgentSoar->rl_stats->total_reward->get_string();			
			output += temp;
			delete temp;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
			
			output = "Global reward since init: ";
			temp = m_pAgentSoar->rl_stats->global_reward->get_string();			
			output += temp;
			delete temp;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
		}
		else
		{
			soar_module::stat *my_stat = m_pAgentSoar->rl_stats->get( pAttr->c_str() );
			char *temp = my_stat->get_string();
			std::string output( temp );
			delete temp;			
			
			if ( m_RawOutput )
				m_Result << output;
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeDouble, output.c_str() );
		}
		
		return true;
	}
	
	return SetError( CLIError::kCommandNotImplemented );
}
