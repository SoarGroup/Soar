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

#include "semantic_memory.h"
#include "agent.h"
#include "misc.h"
#include "utilities.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSMem( std::vector<std::string>& argv )
{
    Options optionsData[] =
    {
        {'a', "add",		OPTARG_NONE},
        {'g', "get",		OPTARG_NONE},
        {'i', "init",		OPTARG_NONE},
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
            SetErrorDetail( "smem takes only one option at a time." );
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

    case 'a':
        // case: add requires one non-option argument
        if (!CheckNumNonOptArgs(1, 1)) return false;

        return DoSMem( option, &( argv[2] ) );

    case 'g':
        {
            // case: get requires one non-option argument
            if (!CheckNumNonOptArgs(1, 1)) return false;

            // check attribute name here
            soar_module::param *my_param = m_pAgentSoar->smem_params->get( argv[2].c_str() );
            if ( !my_param )
                return SetError( kInvalidAttribute );

            return DoSMem( option, &( argv[2] ) );
        }

    case 'i':
        // case: init takes no arguments
        if (!CheckNumNonOptArgs(0, 0)) return false;

        return DoSMem( option );

    case 's':
        {
            // case: set requires two non-option arguments
            if (!CheckNumNonOptArgs(2, 2)) return false;

            // check attribute name/potential vals here
            soar_module::param *my_param = m_pAgentSoar->smem_params->get( argv[2].c_str() );
            if ( !my_param )
                return SetError( kInvalidAttribute );

            if ( !my_param->validate_string( argv[3].c_str() ) )
                return SetError( kInvalidValue );

            return DoSMem( option, &( argv[2] ), &( argv[3] ) );
        }

    case 'S':
        {
            // case: stat can do zero or one non-option arguments
            if (!CheckNumNonOptArgs(0, 1)) return false;

            if ( m_NonOptionArguments == 0 )
                return DoSMem( 'S' );

            // check attribute name
            soar_module::stat *my_stat = m_pAgentSoar->smem_stats->get( argv[2].c_str() );
            if ( !my_stat )
                return SetError( kInvalidAttribute );

            return DoSMem( option, &( argv[2] ) );
        }

    case 't':
        {
            // case: timer can do zero or one non-option arguments
            if (!CheckNumNonOptArgs(0, 1)) return false;

            if ( m_NonOptionArguments == 0 )
                return DoSMem( 't' );

            // check attribute name
            soar_module::timer *my_timer = m_pAgentSoar->smem_timers->get( argv[2].c_str() );
            if ( !my_timer )
                return SetError( kInvalidAttribute );

            return DoSMem( option, &( argv[2] ) );
        }

    case 'v':
        {
            // case: viz does zero or 1/2 non-option arguments
            if (!CheckNumNonOptArgs(0, 2)) return false;

            if ( m_NonOptionArguments == 0 )
                return DoSMem( option );

            smem_lti_id lti_id = NIL;
            unsigned int depth = 0;

            get_lexeme_from_string( m_pAgentSoar, argv[2].c_str() );
            if ( m_pAgentSoar->lexeme.type == IDENTIFIER_LEXEME )
            {
                if ( m_pAgentSoar->smem_db->get_status() == soar_module::connected )
                {
                    lti_id = smem_lti_get_id( m_pAgentSoar, m_pAgentSoar->lexeme.id_letter, m_pAgentSoar->lexeme.id_number );

                    if ( ( lti_id != NIL ) && ( m_NonOptionArguments == 2 ) )
                    {
                        from_c_string( depth, argv[3].c_str() );
                    }
                }
            }

            if ( lti_id == NIL )
                return SetError( kInvalidAttribute );

            return DoSMem( option, NIL, NIL, lti_id, depth );
        }
    }

    // bad: no option, but more than one argument
    if ( argv.size() > 1 ) 
        return SetError( kTooManyArgs );

    // case: nothing = full configuration information
    return DoSMem();
}

bool CommandLineInterface::DoSMem( const char pOp, const std::string* pAttr, const std::string* pVal, smem_lti_id lti_id, unsigned int depth )
{
    if ( !pOp )
    {
        std::string temp;
        char *temp2;

        temp = "SMem learning: ";
        temp2 = m_pAgentSoar->smem_params->learning->get_string();
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
        temp2 = m_pAgentSoar->smem_params->database->get_string();
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
        temp2 = m_pAgentSoar->smem_params->path->get_string();
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
        temp2 = m_pAgentSoar->smem_params->lazy_commit->get_string();
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
        temp2 = m_pAgentSoar->smem_params->thresh->get_string();
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

        temp = "cache: ";
        temp2 = m_pAgentSoar->smem_params->cache->get_string();
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
        temp2 = m_pAgentSoar->smem_params->opt->get_string();
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
        temp2 = m_pAgentSoar->smem_params->timers->get_string();
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
        temp2 = m_pAgentSoar->smem_params->merge->get_string();
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
        bool result = smem_parse_chunks( m_pAgentSoar, pAttr->c_str(), &( err ) );

        if ( !result )
        {
            SetError( kSMemError );
            SetErrorDetail( *err );
            delete err;
        }

        return result;
    }
    else if ( pOp == 'g' )
    {
        char *temp2 = m_pAgentSoar->smem_params->get( pAttr->c_str() )->get_string();
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
        smem_close( m_pAgentSoar );

        // production memory (automatic init-soar clears working memory as a result)		
        ExciseBitset options(EXCISE_ALL);
        DoExcise(options, 0);

        return true;
    }
    else if ( pOp == 's' )
    {
        bool result = m_pAgentSoar->smem_params->get( pAttr->c_str() )->set_string( pVal->c_str() );

        // since parameter name and value have been validated,
        // this can only mean the parameter is protected
        if ( !result )
        {
            SetError( kSMemError );
            SetErrorDetail( "ERROR: this parameter is protected while the SMem database is open." );
        }

        return result;
    }
    else if ( pOp == 'S' )
    {
        if ( !pAttr )
        {
            std::string output;
            char *temp2;

            output = "Memory Usage: ";
            temp2 = m_pAgentSoar->smem_stats->mem_usage->get_string();
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
            temp2 = m_pAgentSoar->smem_stats->mem_high->get_string();
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
            temp2 = m_pAgentSoar->smem_stats->expansions->get_string();
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
            temp2 = m_pAgentSoar->smem_stats->cbr->get_string();
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
            temp2 = m_pAgentSoar->smem_stats->stores->get_string();
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
            temp2 = m_pAgentSoar->smem_stats->chunks->get_string();
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
            temp2 = m_pAgentSoar->smem_stats->slots->get_string();
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
            char *temp2 = m_pAgentSoar->smem_stats->get( pAttr->c_str() )->get_string();
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

            m_pAgentSoar->smem_timers->for_each( bar );
        }
        else
        {
            char *temp2 = m_pAgentSoar->smem_timers->get( pAttr->c_str() )->get_string();
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

        if ( lti_id == NIL )
        {
            smem_visualize_store( m_pAgentSoar, &( viz ) );
        }
        else
        {
            smem_visualize_lti( m_pAgentSoar, lti_id, depth, &( viz ) );
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
