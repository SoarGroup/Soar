/////////////////////////////////////////////////////////////////
// epmem command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com,
//         Nate Derbinsky, nlderbin@umich.edu
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "episodic_memory.h"
#include "agent.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoEpMem( const char pOp, const std::string* pAttr, const std::string* pVal, epmem_time_id memory_id )
{
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if ( !pOp )
    {
        std::string temp;
        char *temp2;		

        temp = "EpMem learning: ";
        temp2 = agnt->epmem_params->learning->get_string();
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

        temp = "Encoding";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }
        temp = "--------";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

        temp = "phase: ";
        temp2 = agnt->epmem_params->phase->get_string();
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

        temp = "trigger: ";
        temp2 = agnt->epmem_params->trigger->get_string();
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

        temp = "force: ";
        temp2 = agnt->epmem_params->force->get_string();
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

        temp = "exclusions: ";
        temp2 = agnt->epmem_params->exclusions->get_string();
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
        temp2 = agnt->epmem_params->database->get_string();
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

        temp = "commit: ";
        temp2 = agnt->epmem_params->commit->get_string();
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
        temp2 = agnt->epmem_params->path->get_string();
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

        temp = "Retrieval";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }
        temp = "---------";
        if ( m_RawOutput )
        {
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

        temp = "balance: ";
        temp2 = agnt->epmem_params->balance->get_string();
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

        temp = "graph-match: ";
        temp2 = agnt->epmem_params->graph_match->get_string();
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
		
		temp = "page_size: ";
        temp2 = agnt->epmem_params->page_size->get_string();
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
        temp2 = agnt->epmem_params->cache_size->get_string();
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
        temp2 = agnt->epmem_params->opt->get_string();
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
        temp2 = agnt->epmem_params->timers->get_string();
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

		temp = "graph_match_ordering: ";
		temp2 = agnt->epmem_params->gm_ordering->get_string();
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

		temp = "merge: ";
		temp2 = agnt->epmem_params->merge->get_string();
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
    else if ( pOp == 'c' )
    {
        const char *msg = "EpMem database closed.";
        const char *tag_type = sml_Names::kTypeString;

        epmem_close( agnt );
        if ( m_RawOutput )
        {
            m_Result << msg;
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, tag_type, msg );
        }

        return true;
    }
    else if ( pOp == 'g' )
    {
        soar_module::param *my_param = agnt->epmem_params->get( pAttr->c_str() );
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
    else if ( pOp == 's' )
    {
        // check attribute name/potential vals here
        soar_module::param *my_param = agnt->epmem_params->get( pAttr->c_str() );
        if ( !my_param )
            return SetError( "Invalid attribute." );

        if ( !my_param->validate_string( pVal->c_str() ) )
            return SetError( "Invalid value." );

        bool result = my_param->set_string( pVal->c_str() );

        // since parameter name and value have been validated,
        // this can only mean the parameter is protected
        if ( !result )
            SetError( "ERROR: this parameter is protected while the EpMem database is open." );

        return result;
    }
    else if ( pOp == 'S' )
    {
        if ( !pAttr )
        {			
            std::string output;
            char *temp2;

            output = "Time: ";
            temp2 = agnt->epmem_stats->time->get_string();
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

            output = "Memory Usage: ";
            temp2 = agnt->epmem_stats->mem_usage->get_string();
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
            temp2 = agnt->epmem_stats->mem_high->get_string();
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

            output = "Last Retrieval WMEs: ";
            temp2 = agnt->epmem_stats->ncb_wmes->get_string();
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

            output = "Last Query Positive: ";
            temp2 = agnt->epmem_stats->qry_pos->get_string();
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

            output = "Last Query Negative: ";
            temp2 = agnt->epmem_stats->qry_neg->get_string();
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

            output = "Last Query Retrieved: ";
            temp2 = agnt->epmem_stats->qry_ret->get_string();
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

            output = "Last Query Cardinality: ";
            temp2 = agnt->epmem_stats->qry_card->get_string();
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

            output = "Last Query Literals: ";
            temp2 = agnt->epmem_stats->qry_lits->get_string();
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
            // check attribute name
            soar_module::stat *my_stat = agnt->epmem_stats->get( pAttr->c_str() );
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

            agnt->epmem_timers->for_each( bar );
        }
        else
        {
            // check attribute name
            soar_module::timer *my_timer = agnt->epmem_timers->get( pAttr->c_str() );
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
        std::string viz;

        epmem_visualize_episode( agnt, memory_id, &( viz ) );
        if ( viz.empty() )
        {
            viz.assign( "Invalid episode." );
            return SetError( "epmem viz: Invalid episode." );
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

    return SetError( "Unknown option." );
}
