/////////////////////////////////////////////////////////////////
// wma command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, 
//         Nate Derbinsky, nlderbin@umich.edu
// Date  : 2008
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "wma.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWMA( std::vector<std::string>& argv ) 
{	
	Options optionsData[] = 
	{
		{'g', "get",	OPTARG_NONE},
		{'p', "print",  OPTARG_NONE},
		{'s', "set",	OPTARG_NONE},
		{'S', "stats",	OPTARG_NONE},		
		{0, 0, OPTARG_NONE} // null
	};
	WMABitset options(0);
	
	for (;;) 
	{
		if ( !ProcessOptions( argv, optionsData ) ) 
			return false;
		
		if (m_Option == -1) break;
		
		switch (m_Option) 
		{		
			case 'g':
				options.set( WMA_GET );
				break;

			case 'p':				
				wma_print_activated_wmes( m_pAgentSoar, WMA_MAX_TIMELIST );
				return true;
			
			case 's':
				options.set( WMA_SET );
				break;
				
			case 'S':
				options.set( WMA_STAT );
				break;
				
			default:
				return SetError( CLIError::kGetOptError );
		}
	}
	
	// bad: more than one option
	if ( options.count() > 1 )
	{
		SetErrorDetail( "wma takes only one option at a time." );
		return SetError( CLIError::kTooManyArgs );
	}
	
	// bad: no option, but more than one argument
	if ( !options.count() && ( argv.size() > 1 ) )
		return SetError( CLIError::kTooManyArgs );
	
	// case: nothing = full configuration information
	if ( argv.size() == 1 )
		return DoWMA();	
	
	// case: get requires one non-option argument
	else if ( options.test( WMA_GET ) )
	{
		if ( m_NonOptionArguments < 1 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 1 )
			return SetError( CLIError::kTooManyArgs );
		
		// check attribute name here
		soar_module::param *my_param = m_pAgentSoar->wma_params->get( argv[2].c_str() );
		if ( my_param )
			return DoWMA( 'g', &( argv[2] ) );
		else
			return SetError( CLIError::kInvalidAttribute );
	}
		
	// case: set requires two non-option arguments
	else if ( options.test( WMA_SET ) )
	{
		if ( m_NonOptionArguments < 2 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 2 )
			return SetError( CLIError::kTooManyArgs );
		
		// check attribute name/potential vals here
		soar_module::param *my_param = m_pAgentSoar->wma_params->get( argv[2].c_str() );
		if ( my_param )
		{
			if ( !my_param->validate_string( argv[3].c_str() ) )
				return SetError( CLIError::kInvalidValue );
			else
				return DoWMA( 's', &( argv[2] ), &( argv[3] ) );
		}
		else
			return SetError( CLIError::kInvalidAttribute );
	}
	
	// case: stat can do zero or one non-option arguments
	else if ( options.test( WMA_STAT ) )
	{
		if ( m_NonOptionArguments == 0 )
			return DoWMA( 'S' );
		else if ( m_NonOptionArguments == 1 )
		{
			// check attribute name
			soar_module::stat *my_stat = m_pAgentSoar->wma_stats->get( argv[2].c_str() );
			if ( my_stat )
				return DoWMA( 'S', &( argv[2] ) );
			else
				return SetError( CLIError::kInvalidAttribute );
		}
		else
			return SetError( CLIError::kTooManyArgs );
	}
	
	// not sure why you'd get here
	return false;
}

bool CommandLineInterface::DoWMA( const char pOp, const std::string* pAttr, const std::string* pVal ) 
{
	if ( !pOp )
	{
		std::string temp;
		char *temp2;
		
		temp = "WMA activation: ";
		temp2 = m_pAgentSoar->wma_params->activation->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}

		temp = "Activation";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		temp = "----------";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );

		temp = "decay-rate: ";
		temp2 = m_pAgentSoar->wma_params->decay_rate->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "criteria: ";
		temp2 = m_pAgentSoar->wma_params->criteria->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "forgetting: ";
		temp2 = m_pAgentSoar->wma_params->forgetting->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "i-support: ";
		temp2 = m_pAgentSoar->wma_params->isupport->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "persistence: ";
		temp2 = m_pAgentSoar->wma_params->persistence->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "precision: ";
		temp2 = m_pAgentSoar->wma_params->precision->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		//

		if ( m_RawOutput )
			m_Result << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}
					
		return true;
	}	
	else if ( pOp == 'g' )
	{
		soar_module::param *my_param = m_pAgentSoar->wma_params->get( pAttr->c_str() );
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
		soar_module::param *my_param = m_pAgentSoar->wma_params->get( pAttr->c_str() );
		bool result = my_param->set_string( pVal->c_str() );		
		
		// since parameter name and value have been validated,
		// this can only mean the parameter is protected
		if ( !result )
		{
			const char *msg = "ERROR: this parameter is protected while WMA is on.";			
			
			if ( m_RawOutput )
				m_Result << msg;
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, msg );
		}

		return result;
	}
	else if ( pOp == 'S' )
	{
		if ( !pAttr )
		{			
			std::string output;
			char *temp2;			
			
			output = "Dummy: ";
			temp2 = m_pAgentSoar->wma_stats->dummy->get_string();
			output += temp2;
			delete temp2;
			
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );			
		}
		else
		{
			soar_module::stat *my_stat = m_pAgentSoar->wma_stats->get( pAttr->c_str() );

			char *temp2 = my_stat->get_string();
			std::string output( temp2 );
			delete temp2;			
			
			if ( m_RawOutput )
				m_Result << output;
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
		}
		
		return true;
	}
	
	return SetError( CLIError::kCommandNotImplemented );
}
