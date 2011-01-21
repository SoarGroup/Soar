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

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "semantic_memory.h"
#include "agent.h"
#include "misc.h"
#include "utilities.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSMem( const char pOp, const std::string* pAttr, const std::string* pVal )
{
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if ( !pOp )
    {
        std::string temp;
        char *temp2;

        temp = "SMem learning: ";
        temp2 = agnt->smem_params->learning->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << "\n" << temp << "\n\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
        }

        temp = "Storage";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }
        temp = "-------";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

        temp = "database: ";
        temp2 = agnt->smem_params->database->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

        temp = "path: ";
        temp2 = agnt->smem_params->path->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
        }

        temp = "lazy-commit: ";
        temp2 = agnt->smem_params->lazy_commit->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
        }


        temp = "Performance";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }
        temp = "-----------";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

        temp = "thresh: ";
        temp2 = agnt->smem_params->thresh->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );			
        }

        temp = "page_size: ";
        temp2 = agnt->smem_params->page_size->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );			
        }
		
		temp = "cache_size: ";
        temp2 = agnt->smem_params->cache_size->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );			
        }

        temp = "optimization: ";
        temp2 = agnt->smem_params->opt->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );			
        }

        temp = "timers: ";
        temp2 = agnt->smem_params->timers->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, "" );
        }

        temp = "Experimental";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }
        temp = "------------";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

        temp = "merge: ";
        temp2 = agnt->smem_params->merge->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }
		
		temp = "activate_on_query: ";
        temp2 = agnt->smem_params->activate_on_query->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }
		
		temp = "activation_mode: ";
        temp2 = agnt->smem_params->activation_mode->get_string();
        temp += temp2;
        delete temp2;
        if ( m_RawOutput )
        {
            m_Result << temp << "\n\n";
        }
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
        bool result = smem_parse_chunks( agnt, pAttr->c_str(), &( err ) );

        if ( !result )
        {
            SetError( *err );
            delete err;
        }

        return result;
    }
    else if ( pOp == 'g' )
    {
        soar_module::param *my_param = agnt->smem_params->get( pAttr->c_str() );
        if ( !my_param )
            return SetError( "Invalid attribute." );

        char *temp2 = my_param->get_string();
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

        return true;
    }
    else if ( pOp == 'i' )
    {
        // Because of LTIs, re-initializing requires all other memories to be reinitialized.		

        // epmem - close before working/production memories to get re-init benefits
        DoEpMem( 'c' );

        // smem - close before working/production memories to prevent id counter mess-ups
        smem_close( agnt );

        // production memory (automatic init-soar clears working memory as a result)		
        ExciseBitset options(EXCISE_ALL);
        DoExcise(options, 0);

        return true;
    }
    else if ( pOp == 's' )
    {
        soar_module::param *my_param = agnt->smem_params->get( pAttr->c_str() );
        if ( !my_param )
            return SetError( "Invalid attribute." );

        if ( !my_param->validate_string( pVal->c_str() ) )
            return SetError( "Invalid value." );

        bool result = my_param->set_string( pVal->c_str() );

        // since parameter name and value have been validated,
        // this can only mean the parameter is protected
        if ( !result )
            SetError( "ERROR: this parameter is protected while the SMem database is open." );

        return result;
    }
    else if ( pOp == 'S' )
    {
        if ( !pAttr )
        {
            std::string output;
            char *temp2;

            output = "Memory Usage: ";
            temp2 = agnt->smem_stats->mem_usage->get_string();
            output += temp2;
            delete temp2;
            if ( m_RawOutput )
            {
                m_Result << output << "\n";
            }
            else
            {
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
            }

            output = "Memory Highwater: ";
            temp2 = agnt->smem_stats->mem_high->get_string();
            output += temp2;
            delete temp2;
            if ( m_RawOutput )
            {
                m_Result << output << "\n";
            }
            else
            {
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
            }

            output = "Retrieves: ";
            temp2 = agnt->smem_stats->expansions->get_string();
            output += temp2;
            delete temp2;
            if ( m_RawOutput )
            {
                m_Result << output << "\n";
            }
            else
            {
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
            }

            output = "Queries: ";
            temp2 = agnt->smem_stats->cbr->get_string();
            output += temp2;
            delete temp2;
            if ( m_RawOutput )
            {
                m_Result << output << "\n";
            }
            else
            {
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
            }

            output = "Stores: ";
            temp2 = agnt->smem_stats->stores->get_string();
            output += temp2;
            delete temp2;
            if ( m_RawOutput )
            {
                m_Result << output << "\n";
            }
            else
            {
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
            }

            output = "Nodes: ";
            temp2 = agnt->smem_stats->chunks->get_string();
            output += temp2;
            delete temp2;
            if ( m_RawOutput )
            {
                m_Result << output << "\n";
            }
            else
            {
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
            }

            output = "Edges: ";
            temp2 = agnt->smem_stats->slots->get_string();
            output += temp2;
            delete temp2;
            if ( m_RawOutput )
            {
                m_Result << output << "\n";
            }
            else
            {
                AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, output.c_str() );
            }
        }
        else
        {
            soar_module::stat *my_stat = agnt->smem_stats->get( pAttr->c_str() );
            if ( !my_stat )
                return SetError( "Invalid statistic." );

            char *temp2 = my_stat->get_string();
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
    else if ( pOp == 't' )
    {
        if ( !pAttr )
        {
            struct foo: public soar_module::accumulator< soar_module::timer * >
            {
            private:
                bool raw;
                cli::CommandLineInterface *this_cli;
                std::ostringstream& m_Result;

                foo& operator=(const foo&) { return *this; }

            public:
                foo( bool m_RawOutput, cli::CommandLineInterface *new_cli, std::ostringstream& m_Result ): raw( m_RawOutput ), this_cli( new_cli ), m_Result( m_Result ) {};

                void operator() ( soar_module::timer *t )
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

            agnt->smem_timers->for_each( bar );
        }
        else
        {
            soar_module::timer *my_timer = agnt->smem_timers->get( pAttr->c_str() );
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
    else if ( pOp == 'v' )
    {
        smem_lti_id lti_id = NIL;
        unsigned int depth = 0;

		if ( pAttr )
		{
			get_lexeme_from_string( agnt, pAttr->c_str() );
			if ( agnt->lexeme.type == IDENTIFIER_LEXEME )
			{
				if ( agnt->smem_db->get_status() == soar_module::connected )
				{
					lti_id = smem_lti_get_id( agnt, agnt->lexeme.id_letter, agnt->lexeme.id_number );

					if ( ( lti_id != NIL ) && pVal )
					{
						from_c_string( depth, pVal->c_str() );
					}
				}
			}

			if ( lti_id == NIL )
				return SetError( "Invalid attribute." );
		}

        std::string viz;

        if ( lti_id == NIL )
        {
            smem_visualize_store( agnt, &( viz ) );
        }
        else
        {
            smem_visualize_lti( agnt, lti_id, depth, &( viz ) );
        }

        if ( m_RawOutput )
        {
            m_Result << viz;
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, viz.c_str() );
        }

        return true;
    }

    return SetError( "Option not implemented." );
}
