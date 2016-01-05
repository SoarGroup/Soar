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
                return "Syntax: explain [--enable | --disable | --on | --off]\n"
                       "        explain [chunk name | rule name]"
                       "        explain [instantiation | condition] [id]\n";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "all",           OPTARG_NONE},
                    {'b', "backtrace",     OPTARG_NONE},
                    {'c', "constraints",   OPTARG_NONE},
                    {'i', "identity",      OPTARG_NONE},
                    {'s', "specific",      OPTARG_NONE},
                    {'t', "time",          OPTARG_REQUIRED},
                    {'?', "stats",         OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                Cli::ExplainBitset options(0);
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

                        case 's':
                            options.set(Cli::EXPLAIN_SPECIFIC);
                            break;

                        case 't':
                            options.set(Cli::EXPLAIN_TIME);
                            break;

                        case 'b':
                            options.set(Cli::EXPLAIN_BACKTRACE);
                            break;

                        case 'c':
                            options.set(Cli::EXPLAIN_CONSTRAINTS);
                            break;

                        case 'i':
                            options.set(Cli::EXPLAIN_IDENTITY_SETS);
                            break;
                        case '?':
                            options.set(Cli::EXPLAIN_STATS);
                            break;
                    }
                }
                int NonOptionArguments = opt.GetNonOptionArguments();
                if (options.test(Cli::EXPLAIN_TIME) && (opt.GetNonOptionArguments() != 1) && opt.GetOptionArgument().empty())
                {
                    return cli.SetError("Please specify the interval's start and end times.");
                }
                if ((options.count() != 1) &&
                    (options.test(Cli::EXPLAIN_ALL) ||
                     options.test(Cli::EXPLAIN_SPECIFIC) ||
                     options.test(Cli::EXPLAIN_TIME)))
                {
                    return cli.SetError("The explain command options --all --specific and --time cannot be used with other options.");
                }

                std::string arg, arg2;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;
                if (num_args > 2)
                {
                    return cli.SetError("The explain command cannot take that many arguments.");
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
