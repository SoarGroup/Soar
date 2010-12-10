/////////////////////////////////////////////////////////////////
// rl command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, 
//         Nate Derbinsky, nlderbin@umich.edu
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include <vector>
#include <map>

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_Names.h"

#include "agent.h"

#include "reinforcement_learning.h"
#include "misc.h"

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

    char option = 0;

    for (;;) 
    {
        if ( !ProcessOptions( argv, optionsData ) ) 
            return false;

        if (m_Option == -1) break;

        switch (m_Option) 
        {
        case 'g':
        case 's':
        case 'S':
            if (option != 0)
            {
                SetErrorDetail( "rl takes only one option at a time." );
                return SetError( kTooManyArgs );
            }
            option = static_cast<char>(m_Option);
            break;

        default:
            return SetError( kGetOptError );
        }
    }

    switch (option)
    {
    case 0:
    default:
        // no options
        break;

    case 'g':
        // case: get requires one non-option argument
        {
            if (!CheckNumNonOptArgs(1, 1)) return false;

            // check attribute name here
            soar_module::param *my_param = m_pAgentSoar->rl_params->get( argv[2].c_str() );
            if ( !my_param )
                return SetError( kInvalidAttribute );

            return DoRL( option, &( argv[2] ) );		
        }

    case 's':
        // case: set requires two non-option arguments
        {
            if (!CheckNumNonOptArgs(2, 2)) return false;

            // check attribute name/potential vals here
            soar_module::param *my_param = m_pAgentSoar->rl_params->get( argv[2].c_str() );
            if ( !my_param )
                return SetError( kInvalidAttribute );

            if ( !my_param->validate_string( argv[3].c_str() ) )
                return SetError( kInvalidAttribute );

            return DoRL( option, &( argv[2] ), &( argv[3] ) );
        }

    case 'S':
        // case: stat can do zero or one non-option arguments
        {
            if (!CheckNumNonOptArgs(0, 1)) return false;

            if ( m_NonOptionArguments == 0 )
                return DoRL( option );

            // check attribute name
            soar_module::stat *my_stat = m_pAgentSoar->rl_stats->get( argv[2].c_str() );
            if ( !my_stat )
                return SetError( kInvalidAttribute );

            return DoRL( option, &( argv[2] ) );
        }
    }

    // bad: no option, but more than one argument
    if ( argv.size() > 1 ) 
        return SetError( kTooManyArgs );

    // case: nothing = full configuration information
    return DoRL();	
}

inline std::string CLI_DoRL_generate_output( const std::string &name, char * const &param )
{
    const std::string output = name + param;
    delete param;
    return output;
}

inline void CLI_DoRL_print( CommandLineInterface &cli, const bool &RawOutput, std::ostringstream &Result,
                           void (CommandLineInterface::* const AppendArgTagFast)(const char*, const char*, const std::string&),
                           const std::string &text, const bool &newline = true )
{
    if ( RawOutput ) {
        if ( newline )
            Result << text << '\n';
        else
            Result << text;
    }
    else
        (cli.*AppendArgTagFast)( sml_Names::kParamValue, sml_Names::kTypeString, text );
}

bool CommandLineInterface::DoRL( const char pOp, const std::string* pAttr, const std::string* pVal ) 
{
    if ( !pOp )
    {
        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
            CLI_DoRL_generate_output( "Soar-RL learning: ", m_pAgentSoar->rl_params->learning->get_string() ) );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
            CLI_DoRL_generate_output( "temporal-extension: ", m_pAgentSoar->rl_params->temporal_extension->get_string() ) );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "" );
        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "Discount" );
        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "--------" );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
            CLI_DoRL_generate_output( "discount-rate: ", m_pAgentSoar->rl_params->discount_rate->get_string() ) );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "" );
        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "Learning" );
        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "--------" );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
            CLI_DoRL_generate_output( "learning-policy: ", m_pAgentSoar->rl_params->learning_policy->get_string() ) );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
            CLI_DoRL_generate_output( "learning-rate: ", m_pAgentSoar->rl_params->learning_rate->get_string() ) );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
            CLI_DoRL_generate_output( "hrl-discount: ", m_pAgentSoar->rl_params->hrl_discount->get_string() ) );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "" );
        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "Eligibility Traces" );
        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "------------------" );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
            CLI_DoRL_generate_output( "eligibility-trace-decay-rate: ", m_pAgentSoar->rl_params->et_decay_rate->get_string() ) );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
            CLI_DoRL_generate_output( "eligibility-trace-tolerance: ", m_pAgentSoar->rl_params->et_tolerance->get_string() ) );

        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast, "" );

        return true;
    }
    else if ( pOp == 'g' )
    {
        CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
            CLI_DoRL_generate_output( "", m_pAgentSoar->rl_params->get( pAttr->c_str() )->get_string() ), false );

        return true;
    }
    else if ( pOp == 's' )
    {
        soar_module::param *my_param = m_pAgentSoar->rl_params->get( pAttr->c_str() );
        if (!my_param->set_string( pVal->c_str() ))
            return SetError( kRlError );

        return true;
    }
    else if ( pOp == 'S' )
    {
        if ( !pAttr )
        {
            CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
                CLI_DoRL_generate_output( "Error from last update: ", m_pAgentSoar->rl_stats->update_error->get_string() ) );

            CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
                CLI_DoRL_generate_output( "Total reward in last cycle: ", m_pAgentSoar->rl_stats->total_reward->get_string() ) );

            CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
                CLI_DoRL_generate_output( "Global reward since init: ", m_pAgentSoar->rl_stats->global_reward->get_string() ) );
        }
        else
        {
            CLI_DoRL_print( *this, m_RawOutput, m_Result, &CommandLineInterface::AppendArgTagFast,
                CLI_DoRL_generate_output( "", m_pAgentSoar->rl_stats->get( pAttr->c_str() )->get_string() ), false );
        }

        return true;
    }

    return SetError( kCommandNotImplemented );
}
