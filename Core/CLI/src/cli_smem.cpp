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
    std::ostringstream tempString;

    if ( !pOp )
    {   // Print SMem Settings
        PrintCLIMessage_Header("Semantic Memory Settings", 40);
        PrintCLIMessage_Item("learning:", agnt->smem_params->learning, 40);
        PrintCLIMessage_Section("Storage", 40);
        PrintCLIMessage_Item("database:", agnt->smem_params->database, 40);
        PrintCLIMessage_Item("append-database:", agnt->smem_params->append_db, 40);
        PrintCLIMessage_Item("path:", agnt->smem_params->path, 40);
        PrintCLIMessage_Item("lazy-commit:", agnt->smem_params->lazy_commit, 40);
        PrintCLIMessage_Section("Activation", 40);
        PrintCLIMessage_Item("activation-mode:", agnt->smem_params->activation_mode, 40);
        PrintCLIMessage_Item("activate-on-query:", agnt->smem_params->activate_on_query, 40);
        PrintCLIMessage_Item("base-decay:", agnt->smem_params->base_decay, 40);
        PrintCLIMessage_Item("base-update-policy:", agnt->smem_params->base_update, 40);
        PrintCLIMessage_Item("base-incremental-threshes:", agnt->smem_params->base_incremental_threshes, 40);
        PrintCLIMessage_Item("thresh:", agnt->smem_params->thresh, 40);
        PrintCLIMessage_Section("Performance", 40);
        PrintCLIMessage_Item("page-size:", agnt->smem_params->page_size, 40);
        PrintCLIMessage_Item("cache-size:", agnt->smem_params->cache_size, 40);
        PrintCLIMessage_Item("optimization:", agnt->smem_params->opt, 40);
        PrintCLIMessage_Item("timers:", agnt->smem_params->timers, 40);
        PrintCLIMessage_Section("Experimental", 40);
        PrintCLIMessage_Item("merge:", agnt->smem_params->merge, 40);
        PrintCLIMessage_Item("mirroring:", agnt->smem_params->mirroring, 40);
        PrintCLIMessage("");

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
        } else {
        	PrintCLIMessage("SMem| Knowledge added to semantic memory.");
        }

        return result;
    }
	else if ( pOp == 'b' )
    {
        std::string err;
		bool result = smem_backup_db( agnt, pAttr->c_str(), &( err ) );

        if ( !result )
            SetError( "Error while backing up database: " + err );
        else {
           	tempString << "SMem| Database backed up to " << pAttr->c_str();
           	PrintCLIMessage(&tempString);
        }

        return result;
    }
    else if ( pOp == 'e' )
    {
       bool result = agnt->smem_params->learning->set_string("on");

        if ( !result )
            SetError( "This parameter is protected while the semantic memory database is open." );
        else
     	   PrintCLIMessage( "SMem| learning = on");

        return result;
    }
    else if ( pOp == 'd' )
    {
       bool result = agnt->smem_params->learning->set_string("off");

       if ( !result )
           SetError( "This parameter is protected while the semantic memory database is open." );
       else
    	   PrintCLIMessage( "SMem| learning = off");

        return result;
    }
    else if ( pOp == 'g' )
    {
        soar_module::param *my_param = agnt->smem_params->get( pAttr->c_str() );
        if ( !my_param )
            return SetError( "Invalid attribute." );

        PrintCLIMessage_Item("", my_param, 0);
        return true;
    }
    else if ( pOp == 'i' )
    {
        // Because of LTIs, re-initializing requires all other memories to be reinitialized.
        // epmem - close before working/production memories to get re-init benefits
        // smem - close before working/production memories to prevent id counter mess-ups
        // production memory (automatic init-soar clears working memory as a result)

        epmem_close(agnt);
        smem_close( agnt );

        ExciseBitset options(0);
		options.set( EXCISE_ALL, true );
        DoExcise( options, 0 );

    	PrintCLIMessage( "SMem| Semantic memory system re-initialized.");
        return true;
    }
	else if ( pOp == 'p' )
    {
        smem_lti_id lti_id = NIL;
        unsigned int depth = 1;

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
            smem_print_store( agnt, &( viz ) );
        }
        else
        {
            smem_print_lti( agnt, lti_id, depth, &( viz ) );
        }
        if (viz.empty())
        	return SetError("SMem| Semantic memory is empty.");

        PrintCLIMessage_Header("Semantic Memory", 40);
        PrintCLIMessage(&viz);
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

        if ( !result )
            SetError( "This parameter is protected while the semantic memory database is open." );
        else {
        	tempString << "SMem| "<< pAttr->c_str() << " = " << pVal->c_str();
        	PrintCLIMessage(&tempString);
        }
        return result;
    }
    else if ( pOp == 'S' )
    {
        if ( !pAttr )
        {   // Print SMem Settings
            PrintCLIMessage_Header("Semantic Memory Statistics", 40);
            PrintCLIMessage_Item("SQLite Version:", agnt->smem_stats->db_lib_version, 40);
            PrintCLIMessage_Item("Memory Usage:", agnt->smem_stats->mem_usage, 40);
            PrintCLIMessage_Item("Memory Highwater:", agnt->smem_stats->mem_high, 40);
            PrintCLIMessage_Item("Retrieves:", agnt->smem_stats->expansions, 40);
            PrintCLIMessage_Item("Queries:", agnt->smem_stats->cbr, 40);
            PrintCLIMessage_Item("Stores:", agnt->smem_stats->stores, 40);
            PrintCLIMessage_Item("Activation Updates:", agnt->smem_stats->act_updates, 40);
            PrintCLIMessage_Item("Mirrors:", agnt->smem_stats->mirrors, 40);
            PrintCLIMessage_Item("Nodes:", agnt->smem_stats->chunks, 40);
            PrintCLIMessage_Item("Edges:", agnt->smem_stats->slots, 40);
        }
        else
        {
            soar_module::stat *my_stat = agnt->smem_stats->get( pAttr->c_str() );
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

            PrintCLIMessage_Header("Semantic Memory Timers", 40);
            agnt->smem_timers->for_each( bar );
        }
        else
        {
            soar_module::timer *my_timer = agnt->smem_timers->get( pAttr->c_str() );
            if ( !my_timer )
                return SetError( "Invalid timer." );

            PrintCLIMessage_Item("", my_timer, 0);
        }

        return true;
    }
    else if ( pOp == 'v' )
    {
        smem_lti_id lti_id = NIL;
        unsigned int depth = 1;

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

        if ( viz.empty() ) return SetError( "Nothing to visualize." );
        PrintCLIMessage(&viz);

        return true;
    }

    return SetError( "Unknown option." );
}
