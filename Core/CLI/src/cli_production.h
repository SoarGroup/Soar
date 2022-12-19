/*
 * cli_production.h
 *
 *  Created on: Oct 1, 2016
 *      Author: mazzin
 */

#ifndef CORE_CLI_SRC_CLI_PRODUCTION_H_
#define CORE_CLI_SRC_CLI_PRODUCTION_H_

#include "cli_Parser.h"
#include "cli_Options.h"

//#include "misc.h"
//#include "sml_Events.h"

namespace cli
{

    class ProductionCommand : public cli::ParserCommand
    {
        public:
            ProductionCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~ProductionCommand() {}
            virtual const char* GetString() const
            {
                return "production";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'production ?' or 'help production' to learn more about the production command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                // Maintainer Note: because we parse the options once here and once in DoProduction, we have to declare all options
                // used by subcommands here as well.
                // TODO: that's double-maintenance. We should only have to declare options to the top-level command here.
                OptionsData optionsData[] =
                {
                    {'a', "all",                OPTARG_NONE},
                    {'b', "assertions",         OPTARG_NONE},
                    {'c', "chunks",             OPTARG_NONE},
                    // TODO: it's actually -c for count, but -c is taken by chunks already,
                    // and we have to declare this somewhere for the subcommands to use it.
                    {'z', "count",              OPTARG_NONE},
                    {'e', "clear",              OPTARG_NONE},
                    {'f', "fired",              OPTARG_NONE},
                    {'d', "defaults",           OPTARG_NONE},
                    {'j', "justifications",     OPTARG_NONE},
                    {'l', "lhs",                OPTARG_NONE},
                    {'n', "names",              OPTARG_NONE},
                    {'o', "never-fired",        OPTARG_NONE},
                    {'q', "nochunks",           OPTARG_NONE},
                    {'p', "print",              OPTARG_NONE},
                    {'r', "retractions",        OPTARG_NONE},
                    {'v', "rhs",                OPTARG_NONE},
                    {'r', "rl",                 OPTARG_NONE},
                    {'s', "set",                OPTARG_NONE},
                    {'x', "show-bindings",      OPTARG_NONE},
                    {'t', "task",               OPTARG_NONE},
                    {'T', "templates",          OPTARG_NONE},
                    {'y', "timetags",           OPTARG_NONE},
                    {'u', "user",               OPTARG_NONE},
                    {'w', "wmes",               OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        cli.SetError(opt.GetError().c_str());
                        return cli.AppendError(GetSyntax());
                    }
                    if (opt.GetOption() == -1)
                    {
                        break;
                    }
                }
                std::string lCmd;
                if (!opt.GetNonOptionArguments())
                {
                    return cli.DoProduction(argv, lCmd);
//                    return cli.SetError("The 'production' commands contains commands to manipulate Soar rules and analyze their usage.\n\n"
//                        "Use 'production ?' to see an overview of the command or 'help production' to read the manual page.");
                }
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;
                lCmd = argv[start_arg_position];
                if (num_args > 0)
                {
                    return cli.DoProduction(argv, lCmd);
                }
                return cli.DoProduction(argv, lCmd);
            }

        private:
            cli::CommandLineInterface& cli;

            ProductionCommand& operator=(const ProductionCommand&);
    };
}




#endif /* CORE_CLI_SRC_CLI_PRODUCTION_H_ */
