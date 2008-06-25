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
		{'g', "get",	0},
		{'p', "print",  0},
		{'s', "set",	0},
		{'S', "stats",	0},		
		{0, 0, 0} // null
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
		if ( wma_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
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
		if ( wma_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
		{
			switch ( wma_get_parameter_type( m_pAgentSoar, argv[2].c_str() ) )
			{
				case wma_param_constant:
					if ( !wma_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), argv[3].c_str() ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoWMA( 's', &( argv[2] ), &( argv[3] ) );
					break;
					
				case wma_param_number:
					double temp;
					from_string( temp, argv[3] );
					if ( !wma_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), temp ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoWMA( 's', &( argv[2] ), &( argv[3] ) );
					break;
					
				case wma_param_string:
					if ( !wma_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), argv[3].c_str() ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoWMA( 's', &( argv[2] ), &( argv[3] ) );
					break;
					
				case wma_param_invalid:
					return SetError( CLIError::kInvalidAttribute );
					break;
			}
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
			if ( wma_valid_stat( m_pAgentSoar, argv[2].c_str() ) )
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
		std::string *temp2;
		double temp_val;
		
		temp = "WMA activation: ";
		temp += wma_get_parameter( m_pAgentSoar, (const long) WMA_PARAM_ACTIVATION, WMA_RETURN_STRING );
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
		temp_val = wma_get_parameter( m_pAgentSoar, WMA_PARAM_DECAY_RATE );
		temp2 = to_string( temp_val );
		temp += (*temp2);
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "criteria: ";
		temp += wma_get_parameter( m_pAgentSoar, (const long) WMA_PARAM_CRITERIA, WMA_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "forgetting: ";
		temp += wma_get_parameter( m_pAgentSoar, (const long) WMA_PARAM_FORGETTING, WMA_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "i-support: ";
		temp += wma_get_parameter( m_pAgentSoar, (const long) WMA_PARAM_I_SUPPORT, WMA_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "persistence: ";
		temp += wma_get_parameter( m_pAgentSoar, (const long) WMA_PARAM_PERSISTENCE, WMA_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "precision: ";
		temp += wma_get_parameter( m_pAgentSoar, (const long) WMA_PARAM_PRECISION, WMA_RETURN_STRING );
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
		std::string output = "";
		double temp;
		std::string *temp2;
		const char *tag_type = sml_Names::kTypeString;
		
		switch ( wma_get_parameter_type( m_pAgentSoar, pAttr->c_str() ) )
		{
			case wma_param_constant:
				output += wma_get_parameter( m_pAgentSoar, pAttr->c_str(), WMA_RETURN_STRING );
				break;
				
			case wma_param_number:
				temp = wma_get_parameter( m_pAgentSoar, pAttr->c_str() );
				temp2 = to_string( temp );
				output += (*temp2);
				delete temp2;
				tag_type = sml_Names::kTypeDouble;
				break;
				
			case wma_param_string:
				output += wma_get_parameter( m_pAgentSoar, pAttr->c_str(), WMA_RETURN_STRING );
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
		bool result = false;
		bool invalid = false;
		
		switch ( wma_get_parameter_type( m_pAgentSoar, pAttr->c_str() ) )
		{
			case wma_param_constant:
				result = wma_set_parameter( m_pAgentSoar, pAttr->c_str(), pVal->c_str() );
				break;
				
			case wma_param_number:
				double temp;
				from_string( temp, *pVal );
				result = wma_set_parameter( m_pAgentSoar, pAttr->c_str(), temp );				
				break;
				
			case wma_param_string:
				result = wma_set_parameter( m_pAgentSoar, pAttr->c_str(), pVal->c_str() );
				break;
				
			case wma_param_invalid:
				invalid = true;
				break;
		}

		// since parameter name and value have been validated,
		// this can only mean the parameter is protected
		if ( !invalid && !result )
		{
			const char *msg = "ERROR: this parameter is protected while WMA is on.";
			const char *tag_type = sml_Names::kTypeString;			
			
			if ( m_RawOutput )
				m_Result << msg;
			else
				AppendArgTagFast( sml_Names::kParamValue, tag_type, msg );
		}

		return result;
	}
	else if ( pOp == 'S' )
	{
		if ( !pAttr )
		{
			double temp;
			std::string output;
			std::string *temp_str;	
			
			output = "Dummy: ";
			temp = wma_get_stat( m_pAgentSoar, (const long) WMA_STAT_DUMMY );
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
			double temp = wma_get_stat( m_pAgentSoar, pAttr->c_str() );
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
