/*
 * cli_explain.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mazzin
 */

#ifndef CLI_EXPLAIN_H_
#define CLI_EXPLAIN_H_

#include "cli_Parser.h"
#include "cli_Options.h"
#include "cli_Cli.h"
#include "misc.h"
#include "kernel.h"
#include "sml_Events.h"

namespace cli
{

    class ExplainCommand : public cli::ParserCommand
    {
        public:
            ExplainCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~ExplainCommand() {}
            virtual const char* GetString() const
            {
                return "explain";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: explain [--all | --only-specific ]\n"
                       "        explain --watch [rule name]"
                       "        explain [chunk name]"
                       "        explain [--dependency-analysis | --constraints | --global-stats | --identity | --stats ]"
                       "        explain [instantiation | condition | chunk] [id number]\n";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "all",                    OPTARG_NONE},
                    {'c', "constraints",            OPTARG_NONE},
                    {'d', "dependency-analysis",    OPTARG_NONE},
                    {'g', "global-stats",           OPTARG_NONE},
                    {'i', "identity",               OPTARG_NONE},
                    {'l', "list",                   OPTARG_NONE},
                    {'o', "only-specific",          OPTARG_NONE},
                    {'s', "stats",                  OPTARG_NONE},
                    {'t', "time",                   OPTARG_REQUIRED},
                    {'w', "watch",                  OPTARG_OPTIONAL},
                    {0, 0,                          OPTARG_NONE}
                };

                Cli::ExplainBitset options(0);
                std::string lWatchArgument;
                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError().c_str());
                    }
                    ;
                    if (opt.GetOption() == -1)
                    {
                        break;
                    }
                    switch (opt.GetOption())
                    {
                        case 'a':
                            options.set(Cli::EXPLAIN_ALL);
                            break;

                        case 'c':
                            options.set(Cli::EXPLAIN_CONSTRAINTS);
                            break;

                        case 'd':
                            options.set(Cli::EXPLAIN_DEPENDENCY);
                            break;

                        case 'g':
                            options.set(Cli::EXPLAIN_GLOBAL_STATS);
                            break;

                        case 'i':
                            options.set(Cli::EXPLAIN_IDENTITY_SETS);
                            break;

                        case 'l':
                            options.set(Cli::EXPLAIN_LIST_ALL);
                            break;

                        case 'o':
                            options.set(Cli::EXPLAIN_ONLY_SPECIFIC);
                            break;

                        case 's':
                            options.set(Cli::EXPLAIN_STATS);
                            break;

                        case 'w':
                            options.set(Cli::EXPLAIN_WATCH);
                            lWatchArgument = opt.GetOptionArgument();
                            break;
                    }
                }
                std::string arg, arg2;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;

                if (num_args > 2)
                {
                    return cli.SetError("The explain command cannot take that many arguments.");
                }
                if (options.test(Cli::EXPLAIN_ALL) ||
                    options.test(Cli::EXPLAIN_ONLY_SPECIFIC) ||
                    options.test(Cli::EXPLAIN_LIST_ALL))
                {
                    if ((options.count() != 1) || (num_args > 0))
                    {
                        return cli.SetError("The explain options --all, --only-specific and --list cannot be used with other options.");
                    }
                    return cli.DoExplain(options, &arg, &arg2);
                }

                if (options.test(Cli::EXPLAIN_DEPENDENCY) ||
                    options.test(Cli::EXPLAIN_CONSTRAINTS) ||
                    options.test(Cli::EXPLAIN_STATS) ||
                    options.test(Cli::EXPLAIN_IDENTITY_SETS))
                {
                    if (num_args > 0)
                    {
                        return cli.SetError("The explain options --dependency-analysis, --constraints, --stats and --identity cannot take additional arguments.");
                    }
                    return cli.DoExplain(options, &arg, &arg2);
                }

                if (options.test(Cli::EXPLAIN_WATCH))
                {
                    if ((options.count() != 1) || (num_args > 0))
                    {
                        return cli.SetError("Please specify only a rule name, for example 'explain -w myRule'.");
                    }
                    return cli.DoExplain(options, &lWatchArgument, &arg2);
                }

                if (num_args > 0)
                {
                    arg = argv[start_arg_position];
                }
                if (num_args > 1)
                {
                    arg2 = argv[start_arg_position+1];
                }
                return cli.DoExplain(options, &arg, &arg2);
            }

        private:
            cli::Cli& cli;

            ExplainCommand& operator=(const ExplainCommand&);
    };
}



#endif /* CLI_EXPLAIN_H_ */
