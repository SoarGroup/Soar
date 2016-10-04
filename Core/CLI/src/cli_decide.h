/*
 * cli_decide.h
 *
 *  Created on: Oct 1, 2016
 *      Author: mazzin
 */

#ifndef CORE_CLI_SRC_CLI_DECIDE_H_
#define CORE_CLI_SRC_CLI_DECIDE_H_

#include "cli_Parser.h"
#include "cli_Options.h"
#include "cli_Cli.h"
//#include "misc.h"
//#include "sml_Events.h"

namespace cli
{

    class DecideCommand : public cli::ParserCommand
    {
        public:
            DecideCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~DecideCommand() {}
            virtual const char* GetString() const
            {
                return "decide";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'decide ?' or 'help decide' to learn more about the decide command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    // Dummy parameters.  Real parsing in cli_decide.cpp
                    {'a', "auto-reduce",        OPTARG_NONE},
                    {'b', "boltzmann",          OPTARG_NONE},
                    {'e', "epsilon",            OPTARG_NONE},
                    {'f', "first",              OPTARG_NONE},
                    {'g', "epsilon-greedy",     OPTARG_NONE},
                    {'l', "last",               OPTARG_NONE},
                    {'p', "reduction-policy",   OPTARG_NONE},
                    {'q', "average",            OPTARG_NONE},
                    {'r', "reduction-rate",     OPTARG_NONE},
                    {'s', "stats",              OPTARG_NONE},
                    {'t', "temperature",        OPTARG_NONE},
                    {'v', "avg",                OPTARG_NONE},
                    {'x', "softmax",            OPTARG_NONE},
                    {'z', "sum",                OPTARG_NONE},

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
                    return cli.SetError("Sub-command is required. Use 'decide ?' or 'help decide' to learn more about the decide command.");
                }
                std::string lCmd;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;
                lCmd = argv[start_arg_position];
                if (num_args > 0)
                {
                    return cli.DoDecide(argv, lCmd);
                }
                return cli.DoDecide(argv, lCmd);
            }

        private:
            cli::Cli& cli;

            DecideCommand& operator=(const DecideCommand&);
    };
}




#endif /* CORE_CLI_SRC_CLI_DECIDE_H_ */
