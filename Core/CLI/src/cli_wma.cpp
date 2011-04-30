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

#include "agent.h"
#include "wmem.h"
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

		if ( m_RawOutput )
            m_Result << "\n";
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
        }


		temp = "Forgetting";
        if ( m_RawOutput )
            m_Result << temp << "\n";
        else
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        temp = "----------";
        if ( m_RawOutput )
            m_Result << temp << "\n";
        else
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );

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

		temp = "forget-wme: ";
        temp2 = agnt->wma_params->forget_wme->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
            m_Result << temp << "\n";
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

		if ( m_RawOutput )
            m_Result << "\n";
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
        }

		temp = "Performance";
        if ( m_RawOutput )
            m_Result << temp << "\n";
        else
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        temp = "-----------";
        if ( m_RawOutput )
            m_Result << temp << "\n";
        else
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );

		temp = "timers: ";
        temp2 = agnt->wma_params->timers->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
            m_Result << temp << "\n";
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

		if ( m_RawOutput )
            m_Result << "\n";
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
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
	else if ( pOp == 'h' )
	{
		uint64_t timetag;
		if ( !from_string( timetag, *pAttr ) || ( timetag == 0 ) )
			return SetError( "Invalid timetag." );
		
		wme* pWme = NULL;
		agent* agnt = m_pAgentSML->GetSoarAgent();

		for ( pWme = agnt->all_wmes_in_rete; pWme; pWme=pWme->rete_next )
		{
			if ( pWme->timetag == timetag )
			{
				break;
			}
		}

		if ( pWme )
		{
			std::string output;
			wma_get_wme_history( agnt, pWme, output );

			if ( m_RawOutput )
                m_Result << output;
            else
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
		}
		
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

            output = "Forgotten WMEs: ";
            temp2 = agnt->wma_stats->forgotten_wmes->get_string();
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
	else if ( pOp == 't' )
	{
		if ( !pAttr )
        {
            struct foo: public soar_module::accumulator< soar_module::timer* >
            {				
            private:
                bool raw;
                cli::CommandLineInterface* this_cli;
                std::ostringstream& m_Result;

                foo& operator= (const foo&) { return *this; }

            public:				
                foo( bool m_RawOutput, cli::CommandLineInterface *new_cli, std::ostringstream& m_Result ): raw( m_RawOutput ), this_cli( new_cli ), m_Result( m_Result ) {};


                void operator() ( soar_module::timer* t )
                {
                    std::string output( t->get_name() );
                    output += ": ";

                    char *temp = t->get_string();
                    output += temp;
                    delete temp;

                    if ( raw )
                    {
                        m_Result << output << "\n";
                    }
                    else
                    {
                        this_cli->AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
                    }
                }
            } bar( m_RawOutput, this, m_Result );

            agnt->wma_timers->for_each( bar );
        }
        else
        {
            // check attribute name
            soar_module::timer* my_timer = agnt->wma_timers->get( pAttr->c_str() );
            if ( !my_timer )
                return SetError( "Invalid timer." );

            char *temp2 = my_timer->get_string();
            std::string output( temp2 );
            delete temp2;			

            if ( m_RawOutput )
            {
                m_Result << output;
            }
            else
            {
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
            }
        }

        return true;
	}

    return SetError( "Unknown option." );
}
