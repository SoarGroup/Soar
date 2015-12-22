/*
 * cli_explain.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mazzin
 */

#ifndef CLI_EXPLAIN_H_
#define CLI_EXPLAIN_H_

#include "cli_Parser.h"
#include "misc.h"
#include "cli_Options.h"
#include "kernel.h"
#include "sml_Events.h"
#include "cli_Cli.h"

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
                       "        explain [options] [chunk name | object-id]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'d', "disable",       OPTARG_NONE},
                    {'d', "off",           OPTARG_NONE},
                    {'e', "enable",        OPTARG_NONE},
                    {'e', "on",            OPTARG_NONE},
                    {'b', "backtrace",     OPTARG_NONE},
                    {'c', "constraints",   OPTARG_NONE},
                    {'i', "identity",      OPTARG_NONE},
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
                        case 'e':
                            options.set(Cli::EXPLAIN_ENABLE);
                            break;

                        case 'd':
                            options.set(Cli::EXPLAIN_DISABLE);
                            break;

                        case 'b':
                            options.set(Cli::EXPLAIN_DEPENDENCY);
                            break;

                        case 'c':
                            options.set(Cli::EXPLAIN_CONSTRAINTS);
                            break;

                        case 'i':
                            options.set(Cli::EXPLAIN_IDENTITY_SETS);
                            break;
                    }
                }

                std::string arg;
                for (size_t i = opt.GetArgument() - opt.GetNonOptionArguments(); i < argv.size(); ++i)
                {
                    if (!arg.empty() && ((argv.size() - i) > 1))
                    {
                        arg.push_back(' ');
                    }
                    arg.append(argv[i]);
                }
                if (arg.empty())
                {
                    if (options.test(Cli::EXPLAIN_DEPENDENCY))
                    {
                        return cli.SetError("Please specify the name of chunk for which you want to see the dependency analysis.");
                    }
                    if (options.test(Cli::EXPLAIN_CONSTRAINTS))
                    {
                        return cli.SetError("Please specify the name of chunk for which you want to see the constraint list.");
                    }
                    if (options.test(Cli::EXPLAIN_IDENTITY_SETS))
                    {
                        return cli.SetError("Please specify the name of chunk for which you want to see the identity set mappings.");
                    }
                }
                return cli.DoExplain(options, &arg);
            }

        private:
            cli::Cli& cli;

            ExplainCommand& operator=(const ExplainCommand&);
    };
}



#endif /* CLI_EXPLAIN_H_ */
