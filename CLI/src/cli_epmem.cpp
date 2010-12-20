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

#include "episodic_memory.h"
#include "agent.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseEpMem( std::vector<std::string>& argv )
{
    Options optionsData[] =
    {
        {'c', "close",		OPTARG_NONE},
        {'g', "get",		OPTARG_NONE},
        {'s', "set",		OPTARG_NONE},
        {'S', "stats",		OPTARG_NONE},
        {'t', "timers",		OPTARG_NONE},
        {'v', "viz",		OPTARG_NONE},
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
            SetErrorDetail( "epmem takes only one option at a time." );
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

    case 'c':
        // case: close gets no arguments
        {
            if (!CheckNumNonOptArgs(0, 0)) return false;

            return DoEpMem( option );
        }

    case 'g':
        // case: get requires one non-option argument
        {
            if (!CheckNumNonOptArgs(1, 1)) return false;

            // check attribute name here
            soar_module::param *my_param = m_pAgentSoar->epmem_params->get( argv[2].c_str() );
            if ( !my_param )
                return SetError( kInvalidAttribute );

            return DoEpMem( option, &( argv[2] ) );
        }

    case 's':
        // case: set requires two non-option arguments
        {
            if (!CheckNumNonOptArgs(2, 2)) return false;

            // check attribute name/potential vals here
            soar_module::param *my_param = m_pAgentSoar->epmem_params->get( argv[2].c_str() );
            if ( !my_param )
                return SetError( kInvalidAttribute );

            if ( !my_param->validate_string( argv[3].c_str() ) )
                return SetError( kInvalidValue );

            return DoEpMem( 's', &( argv[2] ), &( argv[3] ) );
        }

    case 'S':
        // case: stat can do zero or one non-option arguments
        {
            if (!CheckNumNonOptArgs(0, 1)) return false;

            if ( m_NonOptionArguments == 0 )
                return DoEpMem( option );

            // check attribute name
            soar_module::stat *my_stat = m_pAgentSoar->epmem_stats->get( argv[2].c_str() );
            if ( !my_stat )
                return SetError( kInvalidAttribute );

            return DoEpMem( option, &( argv[2] ) );
        }

    case 't':
        // case: timer can do zero or one non-option arguments
        {
            if (!CheckNumNonOptArgs(0, 1)) return false;

            if ( m_NonOptionArguments == 0 )
                return DoEpMem( option );

            // check attribute name
            soar_module::timer *my_timer = m_pAgentSoar->epmem_timers->get( argv[2].c_str() );
            if ( !my_timer )
                return SetError( kInvalidAttribute );

            return DoEpMem( option, &( argv[2] ) );
        }

    case 'v':
        // case: viz takes one non-option argument
        {
            if (!CheckNumNonOptArgs(1, 1)) return false;

            std::string temp_str( argv[2] );
            epmem_time_id memory_id;		

            if ( !from_string( memory_id, temp_str ) )
                return SetError( kInvalidAttribute );

            return DoEpMem( option, 0, 0, memory_id );
        }
    }

    // bad: no option, but more than one argument
    if ( argv.size() > 1 ) 
        return SetError( kTooManyArgs );

    // case: nothing = full configuration information
    return DoEpMem();	
}

bool CommandLineInterface::DoEpMem( const char pOp, const std::string* pAttr, const std::string* pVal, epmem_time_id memory_id )
{
    if ( !pOp )
    {
        std::string temp;
        char *temp2;		

        temp = "EpMem learning: ";
        temp2 = m_pAgentSoar->epmem_params->learning->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->phase->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->trigger->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->force->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->exclusions->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->database->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->commit->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->path->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->balance->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->graph_match->get_string();
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

        temp = "cache: ";
        temp2 = m_pAgentSoar->epmem_params->cache->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->opt->get_string();
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
        temp2 = m_pAgentSoar->epmem_params->timers->get_string();
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

        epmem_close( m_pAgentSoar );
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
        char *temp2 = m_pAgentSoar->epmem_params->get( pAttr->c_str() )->get_string();
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
        bool result = m_pAgentSoar->epmem_params->get( pAttr->c_str() )->set_string( pVal->c_str() );

        // since parameter name and value have been validated,
        // this can only mean the parameter is protected
        if ( !result )
        {
            SetError( kEpMemError );
            SetErrorDetail( "ERROR: this parameter is protected while the EpMem database is open." );
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
            temp2 = m_pAgentSoar->epmem_stats->time->get_string();
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
            temp2 = m_pAgentSoar->epmem_stats->mem_usage->get_string();
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
            temp2 = m_pAgentSoar->epmem_stats->mem_high->get_string();
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
            temp2 = m_pAgentSoar->epmem_stats->ncb_wmes->get_string();
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
            temp2 = m_pAgentSoar->epmem_stats->qry_pos->get_string();
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
            temp2 = m_pAgentSoar->epmem_stats->qry_neg->get_string();
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
            temp2 = m_pAgentSoar->epmem_stats->qry_ret->get_string();
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
            temp2 = m_pAgentSoar->epmem_stats->qry_card->get_string();
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
            temp2 = m_pAgentSoar->epmem_stats->qry_lits->get_string();
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
            char *temp2 = m_pAgentSoar->epmem_stats->get( pAttr->c_str() )->get_string();
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

            m_pAgentSoar->epmem_timers->for_each( bar );
        }
        else
        {
            char *temp2 = m_pAgentSoar->epmem_timers->get( pAttr->c_str() )->get_string();
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

        epmem_visualize_episode( m_pAgentSoar, memory_id, &( viz ) );
        if ( viz.empty() )
        {
            viz.assign( "Invalid episode." );
            SetErrorDetail( "epmem viz: Invalid episode." );
            return SetError( kEpMemError );
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

    return SetError( kCommandNotImplemented );
}
