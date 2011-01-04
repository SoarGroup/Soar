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
#include "sml_AgentSML.h"

#include "agent.h"
#include "sml_Names.h"

#include "exploration.h"
#include "misc.h"

#include <vector>

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoIndifferentSelection( const char pOp, const std::string* p1, const std::string* p2, const std::string* p3 ) 
{
    agent* agnt = m_pAgentSML->GetSoarAgent();
    // show selection policy
    if ( !pOp )
    {
        const char *policy_name = exploration_convert_policy( exploration_get_policy( agnt ) );

        if ( m_RawOutput )
            m_Result << policy_name;
        else
            AppendArgTagFast( sml_Names::kParamIndifferentSelectionMode, sml_Names::kTypeString, policy_name );

        return true;
    }

    // selection policy
    else if ( pOp == 'b' )
        return exploration_set_policy( agnt, "boltzmann" );
    else if ( pOp == 'g' )
        return exploration_set_policy( agnt, "epsilon-greedy" );
    else if ( pOp == 'f' )
        return exploration_set_policy( agnt, "first" );
    else if ( pOp == 'l' )
        return exploration_set_policy( agnt, "last" );
    /*else if ( pOp == 'u' )
    return exploration_set_policy( agnt, "random-uniform" );*/
    else if ( pOp == 'x' )
        return exploration_set_policy( agnt, "softmax" );

    // auto-update control
    else if ( pOp == 'a' )
    {
        if ( !p1 )
        {
            bool setting = exploration_get_auto_update( agnt );

            if ( m_RawOutput )
                m_Result << ( ( setting )?( "on" ):( "off" ) );
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, ( ( setting )?( "on" ):( "off" ) ) );

            return true;
        }
        else
        {	
            if ( *p1 != "on" && *p1 != "off" )
                return SetError( "Invalid value." );

            return exploration_set_auto_update( agnt, ( *p1 == "on" ) );
        }
    }

    // selection policy parameter
    else if ( pOp == 'e' )
    {
        if ( !p1 )
        {
            double param_value = exploration_get_parameter_value( agnt, "epsilon" ); 
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
                return SetError( "Invalid value." );

            if ( !exploration_valid_parameter_value( agnt, "epsilon", new_val ) )
                return SetError( "Invalid value." );

            return exploration_set_parameter_value( agnt, "epsilon", new_val );
        }
    }
    else if ( pOp == 't' )
    {
        if ( !p1 )
        {
            double param_value = exploration_get_parameter_value( agnt, "temperature" ); 
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
                return SetError( "Invalid value." );

            if ( !exploration_valid_parameter_value( agnt, "temperature", new_val ) )
                return SetError( "Invalid value." );

            return exploration_set_parameter_value( agnt, "temperature", new_val );
        }
    }

    // selection parameter reduction policy
    else if ( pOp == 'p' )
    {
        if ( !p2 )
        {
            if ( !exploration_valid_parameter( agnt, p1->c_str() ) )
                return SetError( "Invalid attribute." );

            const char *policy_name = exploration_convert_reduction_policy( exploration_get_reduction_policy( agnt, p1->c_str() ) );

            if ( m_RawOutput )
                m_Result << policy_name;
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, policy_name );

            return true;
        }
        else
        {
            if ( !exploration_valid_reduction_policy( agnt, p1->c_str(), p2->c_str() ) )
                return SetError( "Invalid value." );

            return exploration_set_reduction_policy( agnt, p1->c_str(), p2->c_str() );
        }
    }

    // selection parameter reduction rate
    else if ( pOp == 'r' )
    {
        if ( !exploration_valid_parameter( agnt, p1->c_str() ) )
            return SetError( "Invalid attribute." );

        if ( !exploration_valid_reduction_policy( agnt, p1->c_str(), p2->c_str() ) )
            return SetError( "Invalid attribute." );

        if ( !p3 )
        {
            double reduction_rate = exploration_get_reduction_rate( agnt, p1->c_str(), p2->c_str() );
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
                return SetError( "Invalid value." );

            if ( !exploration_valid_reduction_rate( agnt, p1->c_str(), p2->c_str(), new_val ) )
                return SetError( "Invalid value." );

            return exploration_set_reduction_rate( agnt, p1->c_str(), p2->c_str(), new_val );
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
        temp += exploration_convert_policy( exploration_get_policy( agnt ) );

        if ( m_RawOutput )
            m_Result << temp << "\n"; 
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp );
        }
        temp = "";

        temp = "Automatic Policy Parameter Reduction: ";
        temp += ( ( exploration_get_auto_update( agnt ) )?( "on" ):( "off" ) );

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
            temp = exploration_convert_parameter( agnt, i );
            temp += ": ";
            temp_value = exploration_get_parameter_value( agnt, i ); 
            to_string( temp_value, temp4 );
            temp += temp4;

            if ( m_RawOutput )
                m_Result << temp << "\n"; 
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp );

            // reduction policy
            temp = exploration_convert_parameter( agnt, i );
            temp += " Reduction Policy: ";
            temp += exploration_convert_reduction_policy( exploration_get_reduction_policy( agnt, i ) );
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

                temp_value = exploration_get_reduction_rate( agnt, i, j );
                to_string( temp_value, temp4 );
                temp3 += temp4;
                if ( j != ( EXPLORATION_REDUCTIONS - 1 ) )
                    temp3 += "/";
            }
            temp = exploration_convert_parameter( agnt, i );
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

    return SetError( "Unknown option." );
}

