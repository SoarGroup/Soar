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

#include "gSKI_Agent.h"
#include "sml_Names.h"

#include "exploration.h"
#include "misc.h"

#include <vector>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseIndifferentSelection( gSKI::Agent* pAgent, std::vector<std::string>& argv ) 
{
	// get soar kernel agent - bad gSKI!
	agent *my_agent = pAgent->GetSoarAgent();
	
	Options optionsData[] = 
	{
		// selection policies
		{'b', "boltzmann",			0},
		{'g', "epsilon-greedy",		0},
		{'f', "first",				0},
		{'l', "last",				0},
		//{'u', "random-uniform",		0},
		{'x', "softmax",			0},
		
		// selection parameters
		{'e', "epsilon",			0},
		{'t', "temperature",		0},
		
		// auto-reduction control
		{'a', "auto-reduce",		0},
		
		// selection parameter reduction
		{'p', "reduction-policy",	0},
		{'r', "reduction-rate",		0},
		
		// stats
		{'s', "stats",				0},
		
		{0, 0, 0} // null
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
		return DoIndifferentSelection( pAgent );
	
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
			return DoIndifferentSelection( pAgent, 'b' );
		else if ( options.test( INDIFFERENT_E_GREEDY ) )
			return DoIndifferentSelection( pAgent, 'g' );
		else if ( options.test( INDIFFERENT_FIRST ) )
			return DoIndifferentSelection( pAgent, 'f' );
		else if ( options.test( INDIFFERENT_LAST ) )
			return DoIndifferentSelection( pAgent, 'l' );
		/*else if ( options.test( INDIFFERENT_RANDOM ) )
			return DoIndifferentSelection( pAgent, 'u' );*/
		else if ( options.test( INDIFFERENT_SOFTMAX ) )
			return DoIndifferentSelection( pAgent, 'x' );
	}
	
	// case: selection parameter can do zero or one non-option arguments
	else if ( options.test( INDIFFERENT_EPSILON ) ||
			  options.test( INDIFFERENT_TEMPERATURE ) )
	{
		// get parameter value
		if ( m_NonOptionArguments == 0 )
		{
			if ( options.test( INDIFFERENT_EPSILON ) )
				return DoIndifferentSelection( pAgent, 'e' );
			else if ( options.test( INDIFFERENT_TEMPERATURE ) )
				return DoIndifferentSelection( pAgent, 't' );
		}
		else if ( m_NonOptionArguments == 1 )
		{
			double new_val;
			bool convert = from_string( new_val, argv[2] );
			
			if ( !convert )
				return SetError( CLIError::kInvalidValue );
			
			if ( options.test( INDIFFERENT_EPSILON ) )
			{
				if ( valid_parameter_value( my_agent, "epsilon", new_val ) )
					return DoIndifferentSelection( pAgent, 'e', &( argv[2] ) );
				else
					return SetError( CLIError::kInvalidValue );
			}
			else if ( options.test( INDIFFERENT_TEMPERATURE ) )
				if ( valid_parameter_value( my_agent, "temperature", new_val ) )
					return DoIndifferentSelection( pAgent, 't', &( argv[2] ) );
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
			return DoIndifferentSelection( pAgent, 'a' );
		}
		else if ( m_NonOptionArguments == 1 )
		{
			if ( ( argv[2].compare("on") == 0 ) || ( argv[2].compare("off") == 0 ) )
				return DoIndifferentSelection( pAgent, 'a', &( argv[2] ) );
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
		if ( !valid_exploration_parameter( my_agent, argv[2].c_str() ) )
			return SetError( CLIError::kInvalidAttribute );
		
		if ( m_NonOptionArguments == 1 )
			return DoIndifferentSelection( pAgent, 'p', &( argv[2] ) );
		else if ( m_NonOptionArguments == 2 )
		{
			// validate reduction policy
			if ( valid_reduction_policy( my_agent, argv[2].c_str(), argv[3].c_str() ) )
				return DoIndifferentSelection( pAgent, 'p', &( argv[2] ), &( argv[3] ) );
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
		if ( !valid_exploration_parameter( my_agent, argv[2].c_str() ) )
			return SetError( CLIError::kInvalidAttribute );
		
		// make sure second argument is a valid reduction policy
		if ( !valid_reduction_policy( my_agent, argv[2].c_str(), argv[3].c_str() ) )
			return SetError( CLIError::kInvalidAttribute );
		
		if ( m_NonOptionArguments == 2 )
			return DoIndifferentSelection( pAgent, 'r', &( argv[2] ), &( argv[3] ) );
		else if ( m_NonOptionArguments == 3 )
		{
			double new_val;
			from_string( new_val, argv[4] );
			
			// validate setting
			if ( !valid_reduction_rate( my_agent, argv[2].c_str(), argv[3].c_str(), new_val ) )
				return SetError( CLIError::kInvalidValue );
			else
				return DoIndifferentSelection( pAgent, 'r', &( argv[2] ), &( argv[3] ), &( argv[4] ) ); 
		}
	}
	
	// case: stats takes no parameters
	else if ( options.test( INDIFFERENT_STATS ) )
	{
		if ( m_NonOptionArguments )
			return SetError( CLIError::kTooManyArgs );
		
		return DoIndifferentSelection( pAgent, 's' );
	}
	
	// not sure why you'd get here
	return false;
}

bool CommandLineInterface::DoIndifferentSelection( gSKI::Agent* pAgent, const char pOp, const std::string* p1, const std::string* p2, const std::string* p3 ) 
{
	if ( !RequireAgent( pAgent ) ) 
		return false;
	
	// get soar kernel agent - bad gSKI!
	agent *my_agent = pAgent->GetSoarAgent();
	
	// show selection policy
	if ( !pOp )
	{
		const char *policy_name = convert_exploration_policy( get_exploration_policy( my_agent ) );
		
		if ( m_RawOutput )
			m_Result << policy_name;
		else
			AppendArgTagFast( sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeString, policy_name );
		
		return true;
	}
	
	// selection policy
	else if ( pOp == 'b' )
		return set_exploration_policy( my_agent, "boltzmann" );
	else if ( pOp == 'g' )
		return set_exploration_policy( my_agent, "epsilon-greedy" );
	else if ( pOp == 'f' )
		return set_exploration_policy( my_agent, "first" );
	else if ( pOp == 'l' )
		return set_exploration_policy( my_agent, "last" );
	/*else if ( pOp == 'u' )
		return set_exploration_policy( my_agent, "random-uniform" );*/
	else if ( pOp == 'x' )
		return set_exploration_policy( my_agent, "softmax" );
	
	// auto-update control
	else if ( pOp == 'a' )
	{
		if ( !p1 )
		{
			bool setting = get_auto_update_exploration( my_agent );
			
			if ( m_RawOutput )
				m_Result << ( ( setting )?( "on" ):( "off" ) );
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, ( ( setting )?( "on" ):( "off" ) ) );
			
			return true;
		}
		else
		{	
			return set_auto_update_exploration( my_agent, ( p1->compare("on") == 0 ) );
		}
	}
	
	// selection policy parameter
	else if ( pOp == 'e' )
	{
		if ( !p1 )
		{
			double param_value = get_parameter_value( my_agent, "epsilon" ); 
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
			
			return set_parameter_value( my_agent, "epsilon", new_val );
		}
	}
	else if ( pOp == 't' )
	{
		if ( !p1 )
		{
			double param_value = get_parameter_value( my_agent, "temperature" ); 
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
			
			return set_parameter_value( my_agent, "temperature", new_val );
		}
	}
	
	// selection parameter reduction policy
	else if ( pOp == 'p' )
	{
		if ( !p2 )
		{
			const char *policy_name = convert_reduction_policy( get_reduction_policy( my_agent, p1->c_str() ) );
					
			if ( m_RawOutput )
				m_Result << policy_name;
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, policy_name );
			
			return true;
		}
		else
			return set_reduction_policy( my_agent, p1->c_str(), p2->c_str() );
	}
	
	// selection parameter reduction rate
	else if ( pOp == 'r' )
	{
		if ( !p3 )
		{
			double reduction_rate = get_reduction_rate( my_agent, p1->c_str(), p2->c_str() );
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
			
			return set_reduction_rate( my_agent, p1->c_str(), p2->c_str(), new_val );
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
		temp += convert_exploration_policy( get_exploration_policy( my_agent ) );
		
		if ( m_RawOutput )
			m_Result << temp << "\n"; 
		else
		{
			AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
		}
		temp = "";
		
		temp = "Automatic Policy Parameter Reduction: ";
		temp += ( ( get_auto_update_exploration( my_agent ) )?( "on" ):( "off" ) );
		
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
			temp = convert_exploration_parameter( my_agent, i );
			temp += ": ";
			temp_value = get_parameter_value( my_agent, i ); 
			temp4 = to_string( temp_value );
			temp += (*temp4);
			delete temp4;

			if ( m_RawOutput )
				m_Result << temp << "\n"; 
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			
			// reduction policy
			temp = convert_exploration_parameter( my_agent, i );
			temp += " Reduction Policy: ";
			temp += convert_reduction_policy( get_reduction_policy( my_agent, i ) );
			if ( m_RawOutput )
				m_Result << temp << "\n";
			else
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
			
			// rates			
			temp2 = "";
			temp3 = "";
			for ( int j=0; j<EXPLORATION_REDUCTIONS; j++ )
			{
				temp2 += convert_reduction_policy( j );
				if ( j != ( EXPLORATION_REDUCTIONS - 1 ) )
					temp2 += "/";
				
				temp_value = get_reduction_rate( my_agent, i, j );
				temp4 = to_string( temp_value );
				temp3 += (*temp4);
				delete temp4;
				if ( j != ( EXPLORATION_REDUCTIONS - 1 ) )
					temp3 += "/";
			}
			temp = convert_exploration_parameter( my_agent, i );
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

