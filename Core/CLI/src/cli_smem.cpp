/////////////////////////////////////////////////////////////////
// smem command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com,
//         Nate Derbinsky, nlderbin@umich.edu
// Date  : 2009
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "semantic_memory.h"
#include "agent.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSMem( std::vector<std::string>& argv )
{
	Options optionsData[] =
	{		
		{'a', "add",		OPTARG_NONE},
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
			case 'a':
				options.set( SMEM_ADD );
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
	{
		return DoSMem();
	}
	
	// case: add requires one non-option argument
	else if ( options.test( SMEM_ADD ) )
	{
		if ( m_NonOptionArguments < 1 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 1 )
			return SetError( CLIError::kTooManyArgs );
		
		return DoSMem( 'a', &( argv[2] ) );
	}
	
	// case: get requires one non-option argument
	else if ( options.test( SMEM_GET ) )
	{
		if ( m_NonOptionArguments < 1 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 1 )
			return SetError( CLIError::kTooManyArgs );

		// check attribute name here
		soar_module::param *my_param = m_pAgentSoar->smem_params->get( argv[2].c_str() );
		if ( my_param )
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
		soar_module::param *my_param = m_pAgentSoar->smem_params->get( argv[2].c_str() );
		if ( my_param )
		{
			if ( !my_param->validate_string( argv[3].c_str() ) )
				return SetError( CLIError::kInvalidValue );
			else
				return DoSMem( 's', &( argv[2] ), &( argv[3] ) );
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
			soar_module::stat *my_stat = m_pAgentSoar->smem_stats->get( argv[2].c_str() );
			if ( my_stat )
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
			soar_module::timer *my_timer = m_pAgentSoar->smem_timers->get( argv[2].c_str() );
			if ( my_timer )
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
		char *temp2;		

		temp = "SMem learning: ";
		temp2 = m_pAgentSoar->smem_params->learning->get_string();
		temp += temp2;
		delete temp2;
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
		temp2 = m_pAgentSoar->smem_params->database->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );		

		temp = "path: ";
		temp2 = m_pAgentSoar->smem_params->path->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n";
		else
		{			
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}

		temp = "lazy-commit: ";
		temp2 = m_pAgentSoar->smem_params->lazy_commit->get_string();
		temp += temp2;
		delete temp2;
		if ( m_RawOutput )
			m_Result << temp << "\n\n";
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}


		temp = "timers: ";
		temp2 = m_pAgentSoar->smem_params->timers->get_string();
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
	else if ( pOp == 'a' )
	{
		std::string *err = NULL;
		bool result = smem_parse_chunks( m_pAgentSoar, pAttr, &( err ) );

		if ( !result )
		{
			if ( m_RawOutput )
			{
				m_Result << (*err);
			}
			else
			{
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, err->c_str() );
			}

			delete err;
		}

		return result;
	}
	else if ( pOp == 'g' )
	{
		char *temp2 = m_pAgentSoar->smem_params->get( pAttr->c_str() )->get_string();
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
		bool result = m_pAgentSoar->smem_params->get( pAttr->c_str() )->set_string( pVal->c_str() );

		// since parameter name and value have been validated,
		// this can only mean the parameter is protected
		if ( !result )
		{
			const char *msg = "ERROR: this parameter is protected while the SMem database is open.";			

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

			output = "Memory Usage: ";
			temp2 = m_pAgentSoar->smem_stats->mem_usage->get_string();
			output += temp2;
			delete temp2;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );

			output = "Memory Highwater: ";
			temp2 = m_pAgentSoar->smem_stats->mem_high->get_string();
			output += temp2;
			delete temp2;
			if ( m_RawOutput )
				m_Result << output << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
		}
		else
		{
			char *temp2 = m_pAgentSoar->smem_stats->get( pAttr->c_str() )->get_string();
			std::string output( temp2 );
			delete temp2;			

			if ( m_RawOutput )
				m_Result << output;
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
		}

		return true;
	}
	else if ( pOp == 't' )
	{
		if ( !pAttr )
		{
			struct foo: public soar_module::accumulator< soar_module::timer * >
			{				
				private:
					bool raw;
					cli::CommandLineInterface *this_cli;

				public:				
					foo( bool m_RawOutput, cli::CommandLineInterface *new_cli ): raw( m_RawOutput ), this_cli( new_cli ) {};
					
					void operator() ( soar_module::timer *t )
					{
						std::string output( t->get_name() );
						output += ": ";
						
						char *temp = t->get_string();
						output += temp;
						delete temp;

						if ( raw )
							m_Result << output << "\n";
						else
							this_cli->AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
					}
			} bar( m_RawOutput, this );
			
			m_pAgentSoar->smem_timers->for_each( bar );
		}
		else
		{
			char *temp2 = m_pAgentSoar->smem_timers->get( pAttr->c_str() )->get_string();
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
