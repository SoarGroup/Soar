/////////////////////////////////////////////////////////////////
// indifferent-selection command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
//		   Nate Derbinsky, nlderbin@umich.edu
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

#include "agent.h"
#include "sml_Names.h"

#include "exploration.h"
#include "misc.h"

#include <vector>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseIndifferentSelection(std::vector<std::string>& argv) 
{
	Options optionsData[] = 
	{
		// selection policies
		{'b', "boltzmann",			OPTARG_NONE},
		{'g', "epsilon-greedy",		OPTARG_NONE},
		{'f', "first",				OPTARG_NONE},
		{'l', "last",				OPTARG_NONE},
		//{'u', "random-uniform",		OPTARG_NONE},
		{'x', "softmax",			OPTARG_NONE},

		// selection parameters
		{'e', "epsilon",			OPTARG_NONE},
		{'t', "temperature",		OPTARG_NONE},

		// auto-reduction control
		{'a', "auto-reduce",		OPTARG_NONE},

		// selection parameter reduction
		{'p', "reduction-policy",	OPTARG_NONE},
		{'r', "reduction-rate",		OPTARG_NONE},

		// stats
		{'s', "stats",				OPTARG_NONE},

		{0, 0, OPTARG_NONE} // null
	};
	IndifferentBitset options(0);

	for (;;) 
	{
		if ( !ProcessOptions( argv, optionsData ) ) 
			return false;

		if (m_Option == -1) break;

		switch (m_Option) 
		{
			// selection policies
		case 'b':
			options.set( INDIFFERENT_BOLTZMANN );
			break;
		case 'g':
			options.set( INDIFFERENT_E_GREEDY );
			break;
		case 'f':
			options.set( INDIFFERENT_FIRST );
			break;
		case 'l':
			options.set( INDIFFERENT_LAST );
			break;
			/*case 'u':
			options.set( INDIFFERENT_RANDOM );
			break;*/
		case 'x':
			options.set( INDIFFERENT_SOFTMAX );
			break;

			// selection parameters
		case 'e':
			options.set( INDIFFERENT_EPSILON );
			break;
		case 't':
			options.set( INDIFFERENT_TEMPERATURE );
			break;

			// auto-reduction control
		case 'a':
			options.set( INDIFFERENT_RED_AUTO );
			break;

			// selection parameter reduction
		case 'p':
			options.set( INDIFFERENT_RED_POLICY );
			break;
		case 'r':
			options.set( INDIFFERENT_RED_RATE );
			break;

			// stats
		case 's':
			options.set( INDIFFERENT_STATS );
			break;

		default:
			return SetError( CLIError::kGetOptError );
		}
	}

	// bad: more than one option
	if ( options.count() > 1 )
	{
		SetErrorDetail( "indifferent-selection takes only one option at a time." );
		return SetError( CLIError::kTooManyArgs );
	}

	// bad: no option, but more than one argument
	if ( !options.count() && ( argv.size() > 1 ) )
		return SetError( CLIError::kTooManyArgs );

	// case: nothing = full configuration information
	if ( argv.size() == 1 )
		return DoIndifferentSelection( );

	// case: exploration policy takes no non-option arguments
	else if ( options.test( INDIFFERENT_BOLTZMANN ) ||
		options.test( INDIFFERENT_FIRST ) ||
		options.test( INDIFFERENT_E_GREEDY ) ||
		options.test( INDIFFERENT_LAST ) ||
		//options.test( INDIFFERENT_RANDOM ) ||
		options.test( INDIFFERENT_SOFTMAX ) )
	{
		if ( m_NonOptionArguments )
			return SetError( CLIError::kTooManyArgs );

		// run appropriate policy
		if ( options.test( INDIFFERENT_BOLTZMANN ) )
			return DoIndifferentSelection( 'b' );
		else if ( options.test( INDIFFERENT_E_GREEDY ) )
			return DoIndifferentSelection( 'g' );
		else if ( options.test( INDIFFERENT_FIRST ) )
			return DoIndifferentSelection( 'f' );
		else if ( options.test( INDIFFERENT_LAST ) )
			return DoIndifferentSelection( 'l' );
		/*else if ( options.test( INDIFFERENT_RANDOM ) )
		return DoIndifferentSelection( 'u' );*/
		else if ( options.test( INDIFFERENT_SOFTMAX ) )
			return DoIndifferentSelection( 'x' );
	}

	// case: selection parameter can do zero or one non-option arguments
	else if ( options.test( INDIFFERENT_EPSILON ) ||
		options.test( INDIFFERENT_TEMPERATURE ) )
	{
		// get parameter value
		if ( m_NonOptionArguments == 0 )
		{
			if ( options.test( INDIFFERENT_EPSILON ) )
				return DoIndifferentSelection( 'e' );
			else if ( options.test( INDIFFERENT_TEMPERATURE ) )
				return DoIndifferentSelection( 't' );
		}
		else if ( m_NonOptionArguments == 1 )
		{
			double new_val;
			bool convert = from_string( new_val, argv[2] );

			if ( !convert )
				return SetError( CLIError::kInvalidValue );

			if ( options.test( INDIFFERENT_EPSILON ) )
			{
				if ( exploration_valid_parameter_value( m_pAgentSoar, "epsilon", new_val ) )
					return DoIndifferentSelection( 'e', &( argv[2] ) );
				else
					return SetError( CLIError::kInvalidValue );
			}
			else if ( options.test( INDIFFERENT_TEMPERATURE ) )
				if ( exploration_valid_parameter_value( m_pAgentSoar, "temperature", new_val ) )
					return DoIndifferentSelection( 't', &( argv[2] ) );
				else
					return SetError( CLIError::kInvalidValue );
		}
		else
			return SetError( CLIError::kTooManyArgs );
	}

	// case: auto reduction control can do zero or one non-option arguments
	else if ( options.test( INDIFFERENT_RED_AUTO ) )
	{
		// get value
		if ( m_NonOptionArguments == 0 )
		{
			return DoIndifferentSelection( 'a' );
		}
		else if ( m_NonOptionArguments == 1 )
		{
			if ( ( argv[2].compare("on") == 0 ) || ( argv[2].compare("off") == 0 ) )
				return DoIndifferentSelection( 'a', &( argv[2] ) );
			else
				return SetError( CLIError::kInvalidValue );
		}
		else
			return SetError( CLIError::kTooManyArgs );
	}

	// case: reduction policy requires one or two non-option arguments
	else if ( options.test( INDIFFERENT_RED_POLICY ) )
	{
		if ( m_NonOptionArguments < 1 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 2 )
			return SetError( CLIError::kTooManyArgs );

		// make sure first argument is a valid parameter name
		if ( !exploration_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
			return SetError( CLIError::kInvalidAttribute );

		if ( m_NonOptionArguments == 1 )
			return DoIndifferentSelection( 'p', &( argv[2] ) );
		else if ( m_NonOptionArguments == 2 )
		{
			// validate reduction policy
			if ( exploration_valid_reduction_policy( m_pAgentSoar, argv[2].c_str(), argv[3].c_str() ) )
				return DoIndifferentSelection( 'p', &( argv[2] ), &( argv[3] ) );
			else
				return SetError( CLIError::kInvalidValue );
		}
	}

	// case: reduction policy rate requires two or three arguments
	else if ( options.test( INDIFFERENT_RED_RATE ) )
	{
		if ( m_NonOptionArguments < 2 )
			return SetError( CLIError::kTooFewArgs );
		else if ( m_NonOptionArguments > 3 )
			return SetError( CLIError::kTooManyArgs );

		// make sure first argument is a valid parameter name
		if ( !exploration_valid_parameter( m_pAgentSoar, argv[2].c_str() ) )
			return SetError( CLIError::kInvalidAttribute );

		// make sure second argument is a valid reduction policy
		if ( !exploration_valid_reduction_policy( m_pAgentSoar, argv[2].c_str(), argv[3].c_str() ) )
			return SetError( CLIError::kInvalidAttribute );

		if ( m_NonOptionArguments == 2 )
			return DoIndifferentSelection( 'r', &( argv[2] ), &( argv[3] ) );
		else if ( m_NonOptionArguments == 3 )
		{
			double new_val;
			from_string( new_val, argv[4] );

			// validate setting
			if ( !exploration_valid_reduction_rate( m_pAgentSoar, argv[2].c_str(), argv[3].c_str(), new_val ) )
				return SetError( CLIError::kInvalidValue );
			else
				return DoIndifferentSelection( 'r', &( argv[2] ), &( argv[3] ), &( argv[4] ) ); 
		}
	}

	// case: stats takes no parameters
	else if ( options.test( INDIFFERENT_STATS ) )
	{
		if ( m_NonOptionArguments )
			return SetError( CLIError::kTooManyArgs );

		return DoIndifferentSelection( 's' );
	}

	// not sure why you'd get here
	return false;
}

bool CommandLineInterface::DoIndifferentSelection( const char pOp, const std::string* p1, const std::string* p2, const std::string* p3 ) 
{
	// show selection policy
	if ( !pOp )
	{
		const char *policy_name = exploration_convert_policy( exploration_get_policy( m_pAgentSoar ) );

		if ( m_RawOutput )
			m_Result << policy_name;
		else
			AppendArgTagFast( sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeString, policy_name );

		return true;
	}

	// selection policy
	else if ( pOp == 'b' )
		return exploration_set_policy( m_pAgentSoar, "boltzmann" );
	else if ( pOp == 'g' )
		return exploration_set_policy( m_pAgentSoar, "epsilon-greedy" );
	else if ( pOp == 'f' )
		return exploration_set_policy( m_pAgentSoar, "first" );
	else if ( pOp == 'l' )
		return exploration_set_policy( m_pAgentSoar, "last" );
	/*else if ( pOp == 'u' )
	return exploration_set_policy( m_pAgentSoar, "random-uniform" );*/
	else if ( pOp == 'x' )
		return exploration_set_policy( m_pAgentSoar, "softmax" );

	// auto-update control
	else if ( pOp == 'a' )
	{
		if ( !p1 )
		{
			bool setting = exploration_get_auto_update( m_pAgentSoar );

			if ( m_RawOutput )
				m_Result << ( ( setting )?( "on" ):( "off" ) );
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, ( ( setting )?( "on" ):( "off" ) ) );

			return true;
		}
		else
		{	
			return exploration_set_auto_update( m_pAgentSoar, ( p1->compare("on") == 0 ) );
		}
	}

	// selection policy parameter
	else if ( pOp == 'e' )
	{
		if ( !p1 )
		{
			double param_value = exploration_get_parameter_value( m_pAgentSoar, "epsilon" ); 
			std::string *temp = to_string( param_value );

			if ( m_RawOutput )
				m_Result << (*temp);
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeDouble, temp->c_str() );

			delete temp;
			return true;
		}
		else
		{
			double new_val;
			from_string( new_val, *p1 );

			return exploration_set_parameter_value( m_pAgentSoar, "epsilon", new_val );
		}
	}
	else if ( pOp == 't' )
	{
		if ( !p1 )
		{
			double param_value = exploration_get_parameter_value( m_pAgentSoar, "temperature" ); 
			std::string *temp = to_string( param_value );

			if ( m_RawOutput )
				m_Result << (*temp);
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeDouble, temp->c_str() );

			delete temp;
			return true;
		}
		else
		{
			double new_val;
			from_string( new_val, *p1 );

			return exploration_set_parameter_value( m_pAgentSoar, "temperature", new_val );
		}
	}

	// selection parameter reduction policy
	else if ( pOp == 'p' )
	{
		if ( !p2 )
		{
			const char *policy_name = exploration_convert_reduction_policy( exploration_get_reduction_policy( m_pAgentSoar, p1->c_str() ) );

			if ( m_RawOutput )
				m_Result << policy_name;
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, policy_name );

			return true;
		}
		else
			return exploration_set_reduction_policy( m_pAgentSoar, p1->c_str(), p2->c_str() );
	}

	// selection parameter reduction rate
	else if ( pOp == 'r' )
	{
		if ( !p3 )
		{
			double reduction_rate = exploration_get_reduction_rate( m_pAgentSoar, p1->c_str(), p2->c_str() );
			std::string *temp = to_string( reduction_rate );

			if ( m_RawOutput )
				m_Result << (*temp);
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeDouble, temp->c_str() );

			delete temp;
			return true;
		}
		else
		{
			double new_val;
			from_string( new_val, *p3 );

			return exploration_set_reduction_rate( m_pAgentSoar, p1->c_str(), p2->c_str(), new_val );
		}
	}

	// stats
	else if ( pOp == 's' )
	{
		// used for output
		std::string temp, temp2, temp3;
		std::string *temp4;
		double temp_value;

		temp = "Exploration Policy: ";
		temp += exploration_convert_policy( exploration_get_policy( m_pAgentSoar ) );

		if ( m_RawOutput )
			m_Result << temp << "\n"; 
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}
		temp = "";

		temp = "Automatic Policy Parameter Reduction: ";
		temp += ( ( exploration_get_auto_update( m_pAgentSoar ) )?( "on" ):( "off" ) );

		if ( m_RawOutput )
			m_Result << temp << "\n\n"; 
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
		}
		temp = "";

		for ( int i=0; i<EXPLORATION_PARAMS; i++ )
		{	
			// value
			temp = exploration_convert_parameter( m_pAgentSoar, i );
			temp += ": ";
			temp_value = exploration_get_parameter_value( m_pAgentSoar, i ); 
			temp4 = to_string( temp_value );
			temp += (*temp4);
			delete temp4;

			if ( m_RawOutput )
				m_Result << temp << "\n"; 
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );

			// reduction policy
			temp = exploration_convert_parameter( m_pAgentSoar, i );
			temp += " Reduction Policy: ";
			temp += exploration_convert_reduction_policy( exploration_get_reduction_policy( m_pAgentSoar, i ) );
			if ( m_RawOutput )
				m_Result << temp << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );

			// rates			
			temp2 = "";
			temp3 = "";
			for ( int j=0; j<EXPLORATION_REDUCTIONS; j++ )
			{
				temp2 += exploration_convert_reduction_policy( j );
				if ( j != ( EXPLORATION_REDUCTIONS - 1 ) )
					temp2 += "/";

				temp_value = exploration_get_reduction_rate( m_pAgentSoar, i, j );
				temp4 = to_string( temp_value );
				temp3 += (*temp4);
				delete temp4;
				if ( j != ( EXPLORATION_REDUCTIONS - 1 ) )
					temp3 += "/";
			}
			temp = exploration_convert_parameter( m_pAgentSoar, i );
			temp += " Reduction Rate (";
			temp += temp2;
			temp += "): ";
			temp += temp3;
			if ( m_RawOutput )
				m_Result << temp << "\n\n"; 
			else
			{
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
			}

			temp = "";
		}

		return true;
	}

	return SetError( CLIError::kCommandNotImplemented );
}

