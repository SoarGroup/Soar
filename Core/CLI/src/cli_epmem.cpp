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
   	std::ostringstream tempString;

   	if ( !pOp )
    {   // Print Epmem Settings
        PrintCLIMessage_Header("Episodic Memory Settings", 40);
        PrintCLIMessage_Item("learning:", agnt->epmem_params->learning, 40);
        PrintCLIMessage_Section("Encoding", 40);
        PrintCLIMessage_Item("phase:", agnt->epmem_params->phase, 40);
        PrintCLIMessage_Item("trigger:", agnt->epmem_params->trigger, 40);
        PrintCLIMessage_Item("force:", agnt->epmem_params->force, 40);
        PrintCLIMessage_Item("exclusions:", agnt->epmem_params->exclusions, 40);
        PrintCLIMessage_Section("Storage", 40);
        PrintCLIMessage_Item("database:", agnt->epmem_params->database, 40);
        PrintCLIMessage_Item("append-database:", agnt->epmem_params->append_db, 40);
        PrintCLIMessage_Item("path:", agnt->epmem_params->path, 40);
        PrintCLIMessage_Item("lazy-commit:", agnt->epmem_params->lazy_commit, 40);
        PrintCLIMessage_Section("Retrieval", 40);
        PrintCLIMessage_Item("balance:", agnt->epmem_params->balance, 40);
        PrintCLIMessage_Item("graph-match:", agnt->epmem_params->graph_match, 40);
        PrintCLIMessage_Item("graph-match-ordering:", agnt->epmem_params->gm_ordering, 40);
        PrintCLIMessage_Section("Performance", 40);
        PrintCLIMessage_Item("page-size:", agnt->epmem_params->page_size, 40);
        PrintCLIMessage_Item("cache-size:", agnt->epmem_params->cache_size, 40);
        PrintCLIMessage_Item("optimization:", agnt->epmem_params->opt, 40);
        PrintCLIMessage_Item("timers:", agnt->epmem_params->timers, 40);
        PrintCLIMessage_Section("Experimental", 40);
        PrintCLIMessage_Item("merge:", agnt->epmem_params->merge, 40);
        PrintCLIMessage("");

		//

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

		//

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

        temp = "lazy-commit: ";
        temp2 = agnt->epmem_params->lazy_commit->get_string();
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

		//

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
            m_Result << temp << "\n";
        }
        else
        {
            AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, temp.c_str() );
        }

		temp = "graph-match-ordering: ";
		temp2 = agnt->epmem_params->gm_ordering->get_string();
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

		//

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

		temp = "page-size: ";
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

        temp = "cache-size: ";
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
	else if ( pOp == 'b' )
    {
        std::string err;

        bool result = epmem_backup_db( agnt, pAttr->c_str(), &( err ) );

        if ( !result )
            SetError( "Error while backing up database: " + err );
        else {
           	tempString << "EpMem| Database backed up to " << pAttr->c_str();
           	PrintCLIMessage(&tempString);
        }

        return result;
    }
    else if ( pOp == 'e' )
    {
       bool result = agnt->epmem_params->learning->set_string("on");

        if ( !result )
            SetError( "This parameter is protected while the episodic memory database is open." );
        else
     	   PrintCLIMessage( "EpMem| learning = on");

        return result;
    }
    else if ( pOp == 'd' )
    {
       bool result = agnt->epmem_params->learning->set_string("off");

       if ( !result )
           SetError( "This parameter is protected while the episodic memory database is open." );
       else
    	   PrintCLIMessage( "EpMem| learning = off");

        return result;
    }
    else if ( pOp == 'g' )
    {
        soar_module::param *my_param = agnt->epmem_params->get( pAttr->c_str() );
        if ( !my_param )
            return SetError( "Invalid attribute." );

        PrintCLIMessage_Item("", my_param, 0);
        return true;
    }
    else if ( pOp == 'i' )
    {
    	epmem_reinit( agnt );
    	PrintCLIMessage( "EpMem| Episodic memory system re-initialized.");
    	if ((agnt->epmem_params->append_db->get_value() == soar_module::on) &&
    		(agnt->epmem_params->database->get_value() != epmem_param_container::memory))
    	{
        	PrintCLIMessage( "EpMem|   Note: There was no effective change to memory contents because append mode is on and path set to file.");
    	}
    	return true;
    }
	else if ( pOp == 'p' )
    {
        std::string viz;

        epmem_print_episode( agnt, memory_id, &( viz ) );
        if ( viz.empty() )
        {
            return SetError( "Invalid episode." );
        }
        tempString << "Episode " << memory_id;
        PrintCLIMessage_Header(tempString.str().c_str(), 40);
        PrintCLIMessage(&viz);
        return true;
    }
    else if ( pOp == 's' )
    {
        soar_module::param *my_param = agnt->epmem_params->get( pAttr->c_str() );
        if ( !my_param )
            return SetError( "Invalid attribute." );

        if ( !my_param->validate_string( pVal->c_str() ) )
            return SetError( "Invalid value." );

        bool result = my_param->set_string( pVal->c_str() );

        if ( !result )
            SetError( "This parameter is protected while the episodic memory database is open." );
        else {
        	tempString << "EpMem| "<< pAttr->c_str() << " = " << pVal->c_str();
        	PrintCLIMessage(&tempString);
        }
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

			output = "SQLite Version: ";
            temp2 = agnt->epmem_stats->db_lib_version->get_string();
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

			output = "Retrievals: ";
            temp2 = agnt->epmem_stats->ncbr->get_string();
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
            temp2 = agnt->epmem_stats->cbr->get_string();
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

			output = "Nexts: ";
            temp2 = agnt->epmem_stats->nexts->get_string();
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

			output = "Prevs: ";
            temp2 = agnt->epmem_stats->prevs->get_string();
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

            PrintCLIMessage_Item("", my_stat, 0);
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
                    output += ":";
                    this_cli->PrintCLIMessage_Item(output.c_str(), t, 40);
                }
            } bar( m_RawOutput, this, m_Result );

            PrintCLIMessage_Header("Episodic Memory Timers", 40);
            agnt->epmem_timers->for_each( bar );
        }
        else
        {
            // check attribute name
            soar_module::timer *my_timer = agnt->epmem_timers->get( pAttr->c_str() );
            if ( !my_timer )
                return SetError( "Invalid timer." );

            PrintCLIMessage_Item("", my_timer, 0);
        }

        return true;
    }
    else if ( pOp == 'v' )
    {
        std::string viz;

        epmem_visualize_episode( agnt, memory_id, &( viz ) );

        if ( viz.empty() ) return SetError( "Invalid episode." );
        PrintCLIMessage(&viz);

        return true;
    }

    return SetError( "Unknown option." );
}
