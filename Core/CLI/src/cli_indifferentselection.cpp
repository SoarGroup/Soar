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
#include "sml_Names.h"

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

    char option = 0;

    for (;;) 
    {
        if ( !ProcessOptions( argv, optionsData ) ) 
            return false;

        if (m_Option == -1) break;

        if (option != 0)
        {  
            SetErrorDetail( "indifferent-selection takes only one option at a time." );
            return SetError( kTooManyArgs );
        }
        option = static_cast<char>(m_Option);
    }

    switch (option)
    {
    case 0:
    default:
        // no options
        break;

    case 'b':
    case 'g':
    case 'f':
    case 'l':
    case 'x':
        // case: exploration policy takes no non-option arguments
        if (!CheckNumNonOptArgs(0, 0)) return false;
        return DoIndifferentSelection( option );

    case 'e':
    case 't':
        // case: selection parameter can do zero or one non-option arguments
        if (!CheckNumNonOptArgs(0, 1)) return false;

        if ( m_NonOptionArguments == 0 )
            return DoIndifferentSelection( option );

        return DoIndifferentSelection( option, &( argv[2] ) );

    case 'a':
        // case: auto reduction control can do zero or one non-option arguments
        if (!CheckNumNonOptArgs(0, 1)) return false;

        if ( m_NonOptionArguments == 0 )
            return DoIndifferentSelection( option );

        return DoIndifferentSelection( option, &( argv[2] ) );

    case 'p':
        // case: reduction policy requires one or two non-option arguments
        if (!CheckNumNonOptArgs(1, 2)) return false;

        if ( m_NonOptionArguments == 1 )
            return DoIndifferentSelection( option, &( argv[2] ) );

        return DoIndifferentSelection( option, &( argv[2] ), &( argv[3] ) );

    case 'r':
        // case: reduction policy rate requires two or three arguments
        if (!CheckNumNonOptArgs(2, 3)) return false;

        if ( m_NonOptionArguments == 2 )
            return DoIndifferentSelection( option, &( argv[2] ), &( argv[3] ) );

         return DoIndifferentSelection( option, &( argv[2] ), &( argv[3] ), &( argv[4] ) ); 

    case 's':
        // case: stats takes no parameters
        if (!CheckNumNonOptArgs(0, 0)) return false;

        return DoIndifferentSelection( option );
    }

    // bad: no option, but more than one argument
    if ( argv.size() > 1 ) 
        return SetError( kTooManyArgs );

    // case: nothing = full configuration information
    return DoIndifferentSelection();	
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
            if ( *p1 != "on" && *p1 != "off" )
                return SetError( kInvalidValue );

            return exploration_set_auto_update( m_pAgentSoar, ( *p1 == "on" ) );
        }
    }

    // selection policy parameter
    else if ( pOp == 'e' )
    {
        if ( !p1 )
        {
            double param_value = exploration_get_parameter_value( m_pAgentSoar, "epsilon" ); 
            std::string temp;
            to_string( param_value, temp );

            if ( m_RawOutput )
                m_Result << temp;
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeDouble, temp );

            return true;
        }
        else
        {
            double new_val;
            if ( !from_string( new_val, *p1 ))
                return SetError( kInvalidValue );

            if ( !exploration_valid_parameter_value( m_pAgentSoar, "epsilon", new_val ) )
                return SetError( kInvalidValue );

            return exploration_set_parameter_value( m_pAgentSoar, "epsilon", new_val );
        }
    }
    else if ( pOp == 't' )
    {
        if ( !p1 )
        {
            double param_value = exploration_get_parameter_value( m_pAgentSoar, "temperature" ); 
            std::string temp;
            to_string( param_value, temp );

            if ( m_RawOutput )
                m_Result << temp;
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeDouble, temp );

            return true;
        }
        else
        {
            double new_val;
            if ( !from_string( new_val, *p1 ))
                return SetError( kInvalidValue );

            if ( !exploration_valid_parameter_value( m_pAgentSoar, "temperature", new_val ) )
                return SetError( kInvalidValue );

            return exploration_set_parameter_value( m_pAgentSoar, "temperature", new_val );
        }
    }

    // selection parameter reduction policy
    else if ( pOp == 'p' )
    {
        if ( !p2 )
        {
            if ( !exploration_valid_parameter( m_pAgentSoar, p1->c_str() ) )
                return SetError( kInvalidAttribute );

            const char *policy_name = exploration_convert_reduction_policy( exploration_get_reduction_policy( m_pAgentSoar, p1->c_str() ) );

            if ( m_RawOutput )
                m_Result << policy_name;
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, policy_name );

            return true;
        }
        else
        {
            if ( !exploration_valid_reduction_policy( m_pAgentSoar, p1->c_str(), p2->c_str() ) )
                return SetError( kInvalidValue );

            return exploration_set_reduction_policy( m_pAgentSoar, p1->c_str(), p2->c_str() );
        }
    }

    // selection parameter reduction rate
    else if ( pOp == 'r' )
    {
        if ( !exploration_valid_parameter( m_pAgentSoar, p1->c_str() ) )
            return SetError( kInvalidAttribute );

        if ( !exploration_valid_reduction_policy( m_pAgentSoar, p1->c_str(), p2->c_str() ) )
            return SetError( kInvalidAttribute );

        if ( !p3 )
        {
            double reduction_rate = exploration_get_reduction_rate( m_pAgentSoar, p1->c_str(), p2->c_str() );
            std::string temp;
            to_string( reduction_rate, temp );

            if ( m_RawOutput )
                m_Result << temp;
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeDouble, temp );

            return true;
        }
        else
        {
            double new_val;
            if ( !from_string( new_val, *p3 ) )
                return SetError( kInvalidValue );

            if ( !exploration_valid_reduction_rate( m_pAgentSoar, p1->c_str(), p2->c_str(), new_val ) )
                return SetError( kInvalidValue );

            return exploration_set_reduction_rate( m_pAgentSoar, p1->c_str(), p2->c_str(), new_val );
        }
    }

    // stats
    else if ( pOp == 's' )
    {
        // used for output
        std::string temp, temp2, temp3;
        std::string temp4;
        double temp_value;

        temp = "Exploration Policy: ";
        temp += exploration_convert_policy( exploration_get_policy( m_pAgentSoar ) );

        if ( m_RawOutput )
            m_Result << temp << "\n"; 
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp );
        }
        temp = "";

        temp = "Automatic Policy Parameter Reduction: ";
        temp += ( ( exploration_get_auto_update( m_pAgentSoar ) )?( "on" ):( "off" ) );

        if ( m_RawOutput )
            m_Result << temp << "\n\n"; 
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp );
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
        }
        temp = "";

        for ( int i=0; i<EXPLORATION_PARAMS; i++ )
        {	
            // value
            temp = exploration_convert_parameter( m_pAgentSoar, i );
            temp += ": ";
            temp_value = exploration_get_parameter_value( m_pAgentSoar, i ); 
            to_string( temp_value, temp4 );
            temp += temp4;

            if ( m_RawOutput )
                m_Result << temp << "\n"; 
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp );

            // reduction policy
            temp = exploration_convert_parameter( m_pAgentSoar, i );
            temp += " Reduction Policy: ";
            temp += exploration_convert_reduction_policy( exploration_get_reduction_policy( m_pAgentSoar, i ) );
            if ( m_RawOutput )
                m_Result << temp << "\n";
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp );

            // rates			
            temp2 = "";
            temp3 = "";
            for ( int j=0; j<EXPLORATION_REDUCTIONS; j++ )
            {
                temp2 += exploration_convert_reduction_policy( j );
                if ( j != ( EXPLORATION_REDUCTIONS - 1 ) )
                    temp2 += "/";

                temp_value = exploration_get_reduction_rate( m_pAgentSoar, i, j );
                to_string( temp_value, temp4 );
                temp3 += temp4;
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
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp );
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
            }

            temp = "";
        }

        return true;
    }

    return SetError( kCommandNotImplemented );
}

