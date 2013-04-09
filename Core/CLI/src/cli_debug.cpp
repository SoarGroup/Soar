/*------------------------------------------------------------------
					   set cli command

   @author Mazin Assanie

   @date 2007

------------------------------------------------------------------ */


#include <portability.h>

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "agent.h"
#include "misc.h"
#include "soar_instance.h"
#include "debug.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoDebug( std::vector< std::string >* argv)
{
#ifdef SOAR_DEBUG_UTILITIES
#include "debug.h"

  agent* agnt = m_pAgentSML->GetSoarAgent();
	bool result = false;
	int	numArgs = 0;
	std::ostringstream tempString;
	std::string err, sub_command;

	if (!argv)
	{
        struct print_function: public soar_module::accumulator< soar_module::param * >
        {
        private:
            cli::CommandLineInterface *this_cli;

            print_function& operator=(const print_function&) { return *this; }

        public:
            print_function( cli::CommandLineInterface *new_cli): this_cli( new_cli ) {};

            void operator() ( soar_module::param *p )
            {
                std::string output( p->get_name() );
                output += ":";
                this_cli->PrintCLIMessage_Item(output.c_str(), p, 40);
            }
        } print_function_accumulator(this);


        PrintCLIMessage_Header("Debug", 40);
        PrintCLIMessage_Section("Settings", 40);
        agnt->debug_params->for_each( print_function_accumulator );

		result = true;
		goto print_syntax;
	}

	numArgs = argv->size()-1;
	sub_command = argv->front();

	if (numArgs == 1)
	{
		if (sub_command[0] == 'g')
		{
			std::string parameter_name = argv->at(1);
	        soar_module::param *my_param = agnt->debug_params->get( parameter_name.c_str() );
	        if ( !my_param ) {
				tempString.str("");
	            tempString << "Debug| Invalid parameter: " << parameter_name;
	        	SetError( tempString.str().c_str() );
				goto print_syntax;
	        }
			tempString.str("");
        	tempString << "Debug| "<< parameter_name << " = " << my_param->get_string();
			PrintCLIMessage(&tempString);
	        return true;
		}
	}
	else if (numArgs == 2)
	{
		if (sub_command[0] == 's')
		{
			std::string parameter_name = argv->at(1);
			std::string parameter_value = argv->at(2);

			soar_module::param *my_param = agnt->debug_params->get( parameter_name.c_str() );
			if ( !my_param ) {
				tempString.str("");
				tempString << "Debug| Invalid parameter: " << parameter_name;
				SetError( tempString.str().c_str() );
				goto print_syntax;
			}
			if ( !my_param->validate_string( parameter_value.c_str() ) )
			{
				tempString.str("");
				tempString << "Debug| Invalid value: " << parameter_value;
				SetError( tempString.str().c_str() );
				goto print_syntax;
			}

			bool result = my_param->set_string( parameter_value.c_str() );

			if ( !result )
			{
				SetError( "Debug| Could not set parameter!" );
				goto print_syntax;
			}
			else
			{
				tempString << "Debug| "<< parameter_name.c_str() << " = " << parameter_value.c_str();
				PrintCLIMessage(&tempString);
				return true;
			}
		}
		else if (sub_command[0] == 'p')
		{
			std::string database_name = argv->at(1);
			std::string table_name = argv->at(2);
			if (database_name[0] == 'e')
			{
//				debug_print_epmem_table(agnt, table_name.c_str());
			}
			else if (database_name[0] == 's')
			{
//				debug_print_smem_table(agnt, table_name.c_str());
			}
			else
			{
				tempString.str("");
				tempString << "Debug| Invalid print database parameter: " << sub_command << ".";
				SetError( tempString.str().c_str() );
				goto print_syntax;
			}
		}

		else
		{
			tempString.str("");
			tempString << "Debug| Invalid command: " << sub_command << ".";
			SetError( tempString.str().c_str() );
			goto print_syntax;
		}

		return result;
	}
	else if (numArgs == 0)
	{
if (sub_command[0] == 'd')
		{
//			debug_print_db_err(agnt);
		}
		else
		{
			tempString.str("");
			tempString << "Debug| Invalid command: " << sub_command << ".";
			SetError( tempString.str().c_str() );
			goto print_syntax;
		}

		return result;
	}

	tempString.str("");
	tempString << "Debug| Invalid number of parameters (" << numArgs << ") to command " << sub_command << ".";
	SetError( tempString.str().c_str() );

	print_syntax:

	PrintCLIMessage("\nSyntax: Debug [init|dberr]");
	PrintCLIMessage("        Debug [print] [epmem|smem] [table-name]");
	PrintCLIMessage("        Debug [set|get] [epmem|smem|sql] [on|off]");
	return result;
#else
  PrintCLIMessage("\nDebug| This version of Soar does not have the debug command compiled in.");
  return false;
#endif
}

/**
 * @brief A utility function to run the agent for a certain number of
 *        decision cycles.  Used so that consolidate can make sure it
 *        has the episodes it needs to do an issued experiment.  (i.e.
 *        just saves us the hassle of having to run
 *
 * @param agnt			Agent to run
 * @param run_count		Number of decision cycles to run for
 */
void CommandLineInterface::Run_DC(agent* agnt, int run_count)
{
	std::ostringstream tempString;
	tempString << "MemCon| Running for " << run_count << " decision cycles.\n";
	PrintCLIMessage(&tempString);
	cli::Options opt;
	cli::OptionsData optionsData[] =
	{
			{'d', "decision",        cli::OPTARG_NONE},
			{'e', "elaboration",    cli::OPTARG_NONE},
			{'g', "goal",            cli::OPTARG_NONE},
			{'i', "interleave",        cli::OPTARG_REQUIRED},
			{'n', "noupdate",        cli::OPTARG_NONE},
			{'o', "output",            cli::OPTARG_NONE},
			{'p', "phase",            cli::OPTARG_NONE},
			{'s', "self",            cli::OPTARG_NONE},
			{'u', "update",            cli::OPTARG_NONE},
			{0, 0, cli::OPTARG_NONE}
	};

	cli::Cli::RunBitset options(0);
	DoRun(options, run_count, cli::Cli::RUN_INTERLEAVE_DEFAULT);

}
