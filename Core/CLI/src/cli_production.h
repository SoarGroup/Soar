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
#include "cli_Cli.h"
//#include "misc.h"
//#include "sml_Events.h"

namespace cli
{

    class ProductionCommand : public cli::ParserCommand
    {
        public:
            ProductionCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
                OptionsData optionsData[] =
                {
                    {'a', "all",                OPTARG_NONE},
                    {'b', "assertions",         OPTARG_NONE},
                    {'c', "chunks",             OPTARG_NONE},
                    {'e', "clear",              OPTARG_NONE},
                    {'f', "count",              OPTARG_NONE},
                    {'d', "default",            OPTARG_NONE},
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
                    {'T', "template",           OPTARG_NONE},
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
                if (!opt.GetNonOptionArguments())
                {
                    return cli.SetError("Sub-command is required.");
                }
                std::string lCmd;
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
            cli::Cli& cli;

            ProductionCommand& operator=(const ProductionCommand&);
    };
}




#endif /* CORE_CLI_SRC_CLI_PRODUCTION_H_ */
