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

        if (option != 0)
        {  
            SetErrorDetail( "wma takes only one option at a time." );
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

    case 'g':
        // case: get requires one non-option argument
        {
            if (!CheckNumNonOptArgs(1, 1)) return false;

            // check attribute name here
            soar_module::param *my_param = m_pAgentSoar->wma_params->get( argv[2].c_str() );
            if ( !my_param )
                return SetError( kInvalidAttribute );

            return DoWMA( option, &( argv[2] ) );
        }

    case 's':
        // case: set requires two non-option arguments
        {
            if (!CheckNumNonOptArgs(2, 2)) return false;

            // check attribute name/potential vals here
            soar_module::param *my_param = m_pAgentSoar->wma_params->get( argv[2].c_str() );
            if ( !my_param )
                return SetError( kInvalidAttribute );

            if ( !my_param->validate_string( argv[3].c_str() ) )
                return SetError( kInvalidValue );

            return DoWMA( option, &( argv[2] ), &( argv[3] ) );
        }

    case 'S':
        // case: stat can do zero or one non-option arguments
        {
            if (!CheckNumNonOptArgs(0, 1)) return false;

            if ( m_NonOptionArguments == 0 )
                return DoWMA( 'S' );

            // check attribute name
            soar_module::stat *my_stat = m_pAgentSoar->wma_stats->get( argv[2].c_str() );
            if ( !my_stat )
                return SetError( kInvalidAttribute );

            return DoWMA( option, &( argv[2] ) );
        }
    }

    // bad: no option, but more than one argument
    if ( argv.size() > 1 ) 
        return SetError( kTooManyArgs );

    // case: nothing = full configuration information
    return DoWMA();	
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
            SetError( kWmaError );
            SetErrorDetail( "ERROR: this parameter is protected while WMA is on." );
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

    return SetError( kCommandNotImplemented );
}
