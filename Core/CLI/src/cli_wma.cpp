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
#include "sml_AgentSML.h"

#include "wma.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoWMA( const char pOp, const std::string* pAttr, const std::string* pVal ) 
{
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if ( !pOp )
    {
        std::string temp;
        char *temp2;

        temp = "WMA activation: ";
        temp2 = agnt->wma_params->activation->get_string();
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
        temp2 = agnt->wma_params->decay_rate->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
            m_Result << temp << "\n";
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

		temp = "decay-thresh: ";
        temp2 = agnt->wma_params->decay_thresh->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
            m_Result << temp << "\n";
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

		temp = "petrov-approx: ";
        temp2 = agnt->wma_params->petrov_approx->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
            m_Result << temp << "\n";
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

        temp = "forgetting: ";
        temp2 = agnt->wma_params->forgetting->get_string();
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
        soar_module::param *my_param = agnt->wma_params->get( pAttr->c_str() );
        if ( !my_param )
            return SetError( "Invalid attribute." );

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
        soar_module::param *my_param = agnt->wma_params->get( pAttr->c_str() );
        if ( !my_param )
            return SetError( "Invalid attribute." );

        if ( !my_param->validate_string( pVal->c_str() ) )
            return SetError( "Invalid value." );

        bool result = my_param->set_string( pVal->c_str() );		

        // since parameter name and value have been validated,
        // this can only mean the parameter is protected
        if ( !result )
            SetError( "ERROR: this parameter is protected while WMA is on." );

        return result;
    }
    else if ( pOp == 'S' )
    {
        if ( !pAttr )
        {			
            std::string output;
            char *temp2;			

            output = "Dummy: ";
            temp2 = agnt->wma_stats->dummy->get_string();
            output += temp2;
            delete temp2;

            if ( m_RawOutput )
                m_Result << output << "\n";
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );			
        }
        else
        {
            soar_module::stat *my_stat = agnt->wma_stats->get( pAttr->c_str() );
            if ( !my_stat )
                return SetError( "Invalid statistic." );

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

    return SetError( "Unknown option." );
}
