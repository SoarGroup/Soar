/////////////////////////////////////////////////////////////////
// epmem command file.
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

#include "episodic_memory.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseEpMem( std::vector<std::string>& argv ) 
{	
	Options optionsData[] = 
	{
		{'g', "get",	0},
		{'c', "close",0},
		{'s', "set",	0},
		{'S', "stats",	0},
		{0, 0, 0} // null
	};
	EpMemBitset options(0);
	
	for (;;) 
	{
		if ( !ProcessOptions( argv, optionsData ) ) 
			return false;
		
		if (m_Option == -1) break;
		
		switch (m_Option) 
		{
			case 'c':
				options.set( EPMEM_CLOSE );
				break;
		
			case 'g':
				options.set( EPMEM_GET );
				break;		
			
			case 's':
				options.set( EPMEM_SET );
				break;
				
			case 'S':
				options.set( EPMEM_STAT );
				break;
				
			default:
				return SetError( CLIError::kGetOptError );
		}
	}
	
	// bad: more than one option
	if ( options.count() > 1 )
	{
		SetErrorDetail( "epmem takes only one option at a time." );
		return SetError( CLIError::kTooManyArgs );
	}
	
	// bad: no option, but more than one argument
	if ( !options.count() && ( argv.size() > 1 ) )
		return SetError( CLIError::kTooManyArgs );
	
	// case: nothing = full configuration information
	if ( argv.size() == 1 )
		return DoEpMem();
	
	// case: close gets no arguments
	else if ( options.test( EPMEM_CLOSE ) )
	{		
		if ( m_NonOptionArguments > 0 )
			return SetError( CLIError::kTooManyArgs );

		return DoEpMem( 'c' );
	}
	
	// case: get requires one non-option argument
	else if ( options.test( EPMEM_GET ) )
	{
		if ( m_NonOptionArguments < 1 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 1 )
			return SetError( CLIError::kTooManyArgs );
		
		// check attribute name here
		if ( epmem_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
			return DoEpMem( 'g', &( argv[2] ) );
		else
			return SetError( CLIError::kInvalidAttribute );
	}
		
	// case: set requires two non-option arguments
	else if ( options.test( EPMEM_SET ) )
	{
		if ( m_NonOptionArguments < 2 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 2 )
			return SetError( CLIError::kTooManyArgs );
		
		// check attribute name/potential vals here
		if ( epmem_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
		{
			switch ( epmem_get_parameter_type( m_pAgentSoar, argv[2].c_str() ) )
			{
				case epmem_param_constant:
					if ( !epmem_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), argv[3].c_str() ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoEpMem( 's', &( argv[2] ), &( argv[3] ) );
					break;
					
				case epmem_param_number:
					double temp;
					from_string( temp, argv[3] );
					if ( !epmem_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), temp ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoEpMem( 's', &( argv[2] ), &( argv[3] ) );
					break;
					
				case epmem_param_string:
					if ( !epmem_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), argv[3].c_str() ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoEpMem( 's', &( argv[2] ), &( argv[3] ) );
					break;
					
				case epmem_param_invalid:
					return SetError( CLIError::kInvalidAttribute );
					break;
			}
		}
		else
			return SetError( CLIError::kInvalidAttribute );
	}
	
	// case: stat can do zero or one non-option arguments
	else if ( options.test( EPMEM_STAT ) )
	{
		if ( m_NonOptionArguments == 0 )
			return DoEpMem( 'S' );
		else if ( m_NonOptionArguments == 1 )
		{
			// check attribute name
			if ( epmem_valid_stat( m_pAgentSoar, argv[2].c_str() ) )
				return DoEpMem( 'S', &( argv[2] ) );
			else
				return SetError( CLIError::kInvalidAttribute );
		}
		else
			return SetError( CLIError::kTooManyArgs );
	}
	
	// not sure why you'd get here
	return false;
}

bool CommandLineInterface::DoEpMem( const char pOp, const std::string* pAttr, const std::string* pVal ) 
{
	if ( !pOp )
	{
		std::string temp;
		std::string *temp2;
		double temp_val;
		
		temp = "EpMem learning: ";
		temp += epmem_get_parameter( m_pAgentSoar, (const long) EPMEM_PARAM_LEARNING, EPMEM_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}
		
		temp = "Storage";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		temp = "-------";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "database: ";
		temp += epmem_get_parameter( m_pAgentSoar, (const long) EPMEM_PARAM_DB, EPMEM_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
				
		temp = "path: ";
		temp += epmem_get_parameter( m_pAgentSoar, (const long) EPMEM_PARAM_PATH, EPMEM_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}
		
		temp = "Representation";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		temp = "--------------";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
				
		temp = "indexing: ";
		temp += epmem_get_parameter( m_pAgentSoar, (const long) EPMEM_PARAM_INDEXING, EPMEM_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "provenance: ";
		temp += epmem_get_parameter( m_pAgentSoar, (const long) EPMEM_PARAM_PROVENANCE, EPMEM_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}
		
		temp = "Space";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		temp = "-----";
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		
		temp = "trigger: ";
		temp += epmem_get_parameter( m_pAgentSoar, (const long) EPMEM_PARAM_TRIGGER, EPMEM_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );			
		}

		temp = "balance: ";
		temp_val = epmem_get_parameter( m_pAgentSoar, EPMEM_PARAM_BALANCE );
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
	else if ( pOp == 'c' )
	{
		const char *msg = "EpMem database closed.";
		const char *tag_type = sml_Names::kTypeString;
		
		epmem_end( m_pAgentSoar );
		if ( m_RawOutput )
			m_Result << msg;
		else
			AppendArgTagFast( sml_Names::kParamValue, tag_type, msg );

		return true;
	}
	else if ( pOp == 'g' )
	{
		std::string output = "";
		double temp;
		std::string *temp2;
		const char *tag_type = sml_Names::kTypeString;
		
		switch ( epmem_get_parameter_type( m_pAgentSoar, pAttr->c_str() ) )
		{
			case epmem_param_constant:
				output += epmem_get_parameter( m_pAgentSoar, pAttr->c_str(), EPMEM_RETURN_STRING );
				break;
				
			case epmem_param_number:
				temp = epmem_get_parameter( m_pAgentSoar, pAttr->c_str() );
				temp2 = to_string( temp );
				output += (*temp2);
				delete temp2;
				tag_type = sml_Names::kTypeDouble;
				break;
				
			case epmem_param_string:
				output += epmem_get_parameter( m_pAgentSoar, pAttr->c_str(), EPMEM_RETURN_STRING );
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
		
		switch ( epmem_get_parameter_type( m_pAgentSoar, pAttr->c_str() ) )
		{
			case epmem_param_constant:
				result = epmem_set_parameter( m_pAgentSoar, pAttr->c_str(), pVal->c_str() );
				break;
				
			case epmem_param_number:
				double temp;
				from_string( temp, *pVal );
				result = epmem_set_parameter( m_pAgentSoar, pAttr->c_str(), temp );				
				break;
				
			case epmem_param_string:
				result = epmem_set_parameter( m_pAgentSoar, pAttr->c_str(), pVal->c_str() );
				break;
				
			case epmem_param_invalid:
				invalid = true;
				break;
		}

		// since parameter name and value have been validated,
		// this can only mean the parameter is protected
		if ( !invalid && !result )
		{
			const char *msg = "ERROR: this parameter is protected while the EpMem database is open.";
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
			
			output = "Time: ";
			temp = epmem_get_stat( m_pAgentSoar, (const long) EPMEM_STAT_TIME );
			temp_str = to_string( temp );
			output += (*temp_str);
			delete temp_str;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
			
			output = "Memory Usage: ";
			temp = epmem_get_stat( m_pAgentSoar, (const long) EPMEM_STAT_MEM_USAGE );
			temp_str = to_string( temp );
			output += (*temp_str);
			delete temp_str;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
			
			output = "Memory Highwater: ";
			temp = epmem_get_stat( m_pAgentSoar, (const long) EPMEM_STAT_MEM_HIGH );
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
			double temp = epmem_get_stat( m_pAgentSoar, pAttr->c_str() );
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
