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
                return "Syntax: explain [ --all | --only-specific ]\n"
                       "        explain --record <rule-name>\n"
                       "        explain [instantiation | condition | chunk] <id number>\n"
                       "        explain <chunk name>\n"
                       "        explain [ explanation-trace | wme-trace ]\n"
                       "        explain [--formation | --constraints | --global-stats | --identity | --stats ]\n";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "all",                    OPTARG_NONE},
                    {'c', "constraints",            OPTARG_NONE},
                    {'f', "formation",              OPTARG_NONE},
                    {'e', "explanation-trace",      OPTARG_NONE},
                    {'g', "global-stats",           OPTARG_NONE},
                    {'i', "identity",               OPTARG_NONE},
                    {'l', "list",                   OPTARG_NONE},
                    {'o', "only-specific",          OPTARG_NONE},
                    {'r', "record",                 OPTARG_OPTIONAL},
                    {'s', "stats",                  OPTARG_NONE},
                    {'w', "wme-trace",              OPTARG_OPTIONAL},
                    {0, 0,                          OPTARG_NONE}
                };

                Cli::ExplainBitset options(0);
                std::string lWatchArgument;
                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        cli.SetError(opt.GetError().c_str());
                        return cli.AppendError(GetSyntax());
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

                        case 'f':
                            options.set(Cli::EXPLAIN_FORMATION);
                            break;

                        case 'e':
                            options.set(Cli::EXPLAIN_EXPLANATION_TRACE);
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
                            options.set(Cli::EXPLAIN_WME_TRACE);
                            break;

                        case 'r':
                            options.set(Cli::EXPLAIN_RECORD);
                            lWatchArgument = opt.GetOptionArgument();
                            break;
                    }
                }
                std::string arg, arg2;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;

                if (num_args > 2)
                {
                    cli.SetError("The explain command cannot take that many arguments.");
                	return cli.AppendError(GetSyntax());
                }
                if (options.test(Cli::EXPLAIN_ALL) ||
                    options.test(Cli::EXPLAIN_ONLY_SPECIFIC) ||
                    options.test(Cli::EXPLAIN_LIST_ALL))
                {
                    if ((options.count() != 1) || (num_args > 0))
                    {
                        cli.SetError("That explain option cannot be used with other options.");
                    	return cli.AppendError(GetSyntax());
                    }
                    return cli.DoExplain(options, &arg, &arg2);
                }

                if (options.test(Cli::EXPLAIN_FORMATION) ||
                    options.test(Cli::EXPLAIN_CONSTRAINTS) ||
                    options.test(Cli::EXPLAIN_STATS) ||
                    options.test(Cli::EXPLAIN_IDENTITY_SETS) ||
                    options.test(Cli::EXPLAIN_EXPLANATION_TRACE) ||
                    options.test(Cli::EXPLAIN_WME_TRACE))
                {
                    if (num_args > 0)
                    {
                        cli.SetError("That explain option cannot take additional arguments.");
                    	return cli.AppendError(GetSyntax());
                    }
                    return cli.DoExplain(options, &arg, &arg2);
                }

                if (options.test(Cli::EXPLAIN_RECORD))
                {
                    if ((options.count() != 1) || (num_args > 0))
                    {
                        cli.SetError("Please specify only a rule name, for example 'explain -r myRule'.");
                    	return cli.AppendError(GetSyntax());
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
                if (!cli.DoExplain(options, &arg, &arg2))
                {
                	return cli.AppendError(GetSyntax());
                }
                return true;
            }

        private:
            cli::Cli& cli;

            ExplainCommand& operator=(const ExplainCommand&);
    };
}



#endif /* CLI_EXPLAIN_H_ */
