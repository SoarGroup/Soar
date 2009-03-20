/////////////////////////////////////////////////////////////////
// smem command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com,
// Date  : 2009
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "semantic_memory.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSMem( std::vector<std::string>& argv )
{
	Options optionsData[] =
	{
		{'c', "close",		OPTARG_NONE},
		{'g', "get",		OPTARG_NONE},
		{'s', "set",		OPTARG_NONE},
		{'S', "stats",		OPTARG_NONE},
		{'t', "timers",		OPTARG_NONE},
		{0, 0, OPTARG_NONE} // null
	};
	SMemBitset options(0);

	for (;;)
	{
		if ( !ProcessOptions( argv, optionsData ) )
			return false;

		if (m_Option == -1) break;

		switch (m_Option)
		{
			case 'c':
				options.set( SMEM_CLOSE );
				break;

			case 'g':
				options.set( SMEM_GET );
				break;

			case 's':
				options.set( SMEM_SET );
				break;

			case 'S':
				options.set( SMEM_STAT );
				break;

			case 't':
				options.set( SMEM_TIMER );
				break;

			default:
				return SetError( CLIError::kGetOptError );
		}
	}

	// bad: more than one option
	if ( options.count() > 1 )
	{
		SetErrorDetail( "smem takes only one option at a time." );
		return SetError( CLIError::kTooManyArgs );
	}

	// bad: no option, but more than one argument
	if ( !options.count() && ( argv.size() > 1 ) )
		return SetError( CLIError::kTooManyArgs );

	// case: nothing = full configuration information
	if ( argv.size() == 1 )
		return DoSMem();

	// case: close gets no arguments
	else if ( options.test( SMEM_CLOSE ) )
	{
		if ( m_NonOptionArguments > 0 )
			return SetError( CLIError::kTooManyArgs );

		return DoSMem( 'c' );
	}

	// case: get requires one non-option argument
	else if ( options.test( SMEM_GET ) )
	{
		if ( m_NonOptionArguments < 1 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 1 )
			return SetError( CLIError::kTooManyArgs );

		// check attribute name here
		if ( smem_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
			return DoSMem( 'g', &( argv[2] ) );
		else
			return SetError( CLIError::kInvalidAttribute );
	}

	// case: set requires two non-option arguments
	else if ( options.test( SMEM_SET ) )
	{
		if ( m_NonOptionArguments < 2 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 2 )
			return SetError( CLIError::kTooManyArgs );

		// check attribute name/potential vals here
		if ( smem_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
		{
			switch ( smem_get_parameter_type( m_pAgentSoar, argv[2].c_str() ) )
			{
				case smem_param_constant:
					if ( !smem_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), argv[3].c_str() ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoSMem( 's', &( argv[2] ), &( argv[3] ) );
					break;

				case smem_param_number:
					double temp;
					from_string( temp, argv[3] );
					if ( !smem_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), temp ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoSMem( 's', &( argv[2] ), &( argv[3] ) );
					break;

				case smem_param_string:
					if ( !smem_valid_parameter_value( m_pAgentSoar, argv[2].c_str(), argv[3].c_str() ) )
						return SetError( CLIError::kInvalidValue );
					else
						return DoSMem( 's', &( argv[2] ), &( argv[3] ) );
					break;

				case smem_param_invalid:
					return SetError( CLIError::kInvalidAttribute );
					break;
			}
		}
		else
			return SetError( CLIError::kInvalidAttribute );
	}
	
	// case: stat can do zero or one non-option arguments
	else if ( options.test( SMEM_STAT ) )
	{
		if ( m_NonOptionArguments == 0 )
			return DoSMem( 'S' );
		else if ( m_NonOptionArguments == 1 )
		{
			// check attribute name
			if ( smem_valid_stat( m_pAgentSoar, argv[2].c_str() ) )
				return DoSMem( 'S', &( argv[2] ) );
			else
				return SetError( CLIError::kInvalidAttribute );
		}
		else
			return SetError( CLIError::kTooManyArgs );
	}

	// case: timer can do zero or one non-option arguments
	else if ( options.test( SMEM_TIMER ) )
	{
		if ( m_NonOptionArguments == 0 )
			return DoSMem( 't' );
		else if ( m_NonOptionArguments == 1 )
		{
			// check attribute name
			if ( smem_valid_timer( m_pAgentSoar, argv[2].c_str() ) )
				return DoSMem( 't', &( argv[2] ) );
			else
				return SetError( CLIError::kInvalidAttribute );
		}
		else
			return SetError( CLIError::kTooManyArgs );
	}

	
	// not sure why you'd get here
	return false;
}

bool CommandLineInterface::DoSMem( const char pOp, const std::string* pAttr, const std::string* pVal )
{	
	if ( !pOp )
	{
		std::string temp;
		std::string *temp2;
		double temp_val;

		temp = "SMem learning: ";
		temp += smem_get_parameter( m_pAgentSoar, (const long) SMEM_PARAM_LEARNING, SMEM_RETURN_STRING );
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
		temp += smem_get_parameter( m_pAgentSoar, (const long) SMEM_PARAM_DB, SMEM_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );

		temp = "commit: ";
		temp_val = smem_get_parameter( m_pAgentSoar, SMEM_PARAM_COMMIT );
		temp2 = to_string( temp_val );
		temp += (*temp2);
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}

		temp = "path: ";
		temp += smem_get_parameter( m_pAgentSoar, (const long) SMEM_PARAM_PATH, SMEM_RETURN_STRING );
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}		

		temp = "timers: ";
		temp += smem_get_parameter( m_pAgentSoar, (const long) SMEM_PARAM_TIMERS, SMEM_RETURN_STRING );
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
		const char *msg = "SMem database closed.";
		const char *tag_type = sml_Names::kTypeString;

		smem_close( m_pAgentSoar );
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

		switch ( smem_get_parameter_type( m_pAgentSoar, pAttr->c_str() ) )
		{
			case smem_param_constant:
				output += smem_get_parameter( m_pAgentSoar, pAttr->c_str(), SMEM_RETURN_STRING );
				break;

			case smem_param_number:
				temp = smem_get_parameter( m_pAgentSoar, pAttr->c_str() );
				temp2 = to_string( temp );
				output += (*temp2);
				delete temp2;
				tag_type = sml_Names::kTypeDouble;
				break;

			case smem_param_string:
				output += smem_get_parameter( m_pAgentSoar, pAttr->c_str(), SMEM_RETURN_STRING );
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

		switch ( smem_get_parameter_type( m_pAgentSoar, pAttr->c_str() ) )
		{
			case smem_param_constant:
				result = smem_set_parameter( m_pAgentSoar, pAttr->c_str(), pVal->c_str() );
				break;

			case smem_param_number:
				double temp;
				from_string( temp, *pVal );
				result = smem_set_parameter( m_pAgentSoar, pAttr->c_str(), temp );
				break;

			case smem_param_string:
				result = smem_set_parameter( m_pAgentSoar, pAttr->c_str(), pVal->c_str() );
				break;

			case smem_param_invalid:
				invalid = true;
				break;
		}

		// since parameter name and value have been validated,
		// this can only mean the parameter is protected
		if ( !invalid && !result )
		{
			const char *msg = "ERROR: this parameter is protected while the SMem database is open.";
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
			EPMEM_TYPE_INT temp;
			std::string output;
			std::string *temp_str;			

			output = "Memory Usage: ";
			temp = smem_get_stat( m_pAgentSoar, (const long) SMEM_STAT_MEM_USAGE );
			temp_str = to_string( temp );
			output += (*temp_str);
			delete temp_str;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );

			output = "Memory Highwater: ";
			temp = smem_get_stat( m_pAgentSoar, (const long) SMEM_STAT_MEM_HIGH );
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
			EPMEM_TYPE_INT temp = smem_get_stat( m_pAgentSoar, pAttr->c_str() );
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
	else if ( pOp == 't' )
	{
		if ( !pAttr )
		{
			double temp;
			std::string output;
			std::string *temp_str;

			for ( int i=0; i<SMEM_TIMERS; i++ )
			{
				output = smem_get_timer_name( m_pAgentSoar, (const long) i );
				output += ": ";
				temp = smem_get_timer_value( m_pAgentSoar, (const long) i );
				temp_str = to_string( temp );
				output += (*temp_str);
				delete temp_str;
				if ( m_RawOutput )
					m_Result << output << "\n";
				else
					AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
			}
		}
		else
		{
			double temp = smem_get_timer_value( m_pAgentSoar, pAttr->c_str() );
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
