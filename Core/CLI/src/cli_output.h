/*
 * cli_explain.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mazzin
 */

#ifndef CLI_OUTPUT_H_
#define CLI_OUTPUT_H_

#include "cli_Parser.h"
#include "cli_Options.h"

#include "misc.h"
#include "sml_Events.h"

namespace cli
{

    class OutputCommand : public cli::ParserCommand
    {
        public:
            OutputCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            virtual ~OutputCommand() {}
            virtual const char* GetString() const
            {
                return "output";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'output ?' and 'help output' to learn more about the output command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                std::string subCommandArg;
                bool backwardCompatibleReplacement = false;

                /* Store original argv for CTF and CLOG.  Both sub-commands are still
                 * using their original parsing.  Ideally, we would integrate the logic. */
                std::vector< std::string > argv_orig = argv;
                bool hadError = false;
                bool commandHandled = cli.DoRedirectedOutputCommand(argv_orig, hadError);
                if (commandHandled) {
                    if (hadError) return cli.AppendError(GetSyntax()); else return true;
                }

                OptionsData optionsData[] =
                {
                    {'e', "enable",     OPTARG_NONE},
                    {'d', "disable",    OPTARG_NONE},
                    {'e', "on",         OPTARG_NONE},
                    {'d', "off",        OPTARG_NONE},
                    {'a', "add",        OPTARG_NONE},
                    {'A', "append",        OPTARG_NONE},
                    {'c', "close",        OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                for (;;)
                {
                    /* Ignore bad options in CTF command because they could be options for
                     * command to be executed, which can be anything */
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        cli.SetError(opt.GetError().c_str());
                        return cli.AppendError(GetSyntax());
                    }
                    if (opt.CheckNumNonOptArgs(2, 2))
                    {
                        subCommandArg = argv[2];
                    }
                    if (opt.GetOption() == -1)
                    {
                        break;
                    }
                    /* To maintain backwards compatibility with warnings/verbose/echo-commands */
                    switch (opt.GetOption())
                    {
                        case 'e':
                            subCommandArg = "on";
                            backwardCompatibleReplacement = true;
                            break;
                        case 'd':
                            subCommandArg = "off";
                            backwardCompatibleReplacement = true;
                            break;
                    }
                }
                if (!opt.GetNonOptionArguments())
                {
                    return cli.DoOutput(argv_orig);
                }
                std::string arg, arg2, arg3;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;

                if (num_args > 0)
                {
                    argv_orig.erase(argv_orig.begin());
                    arg = argv[start_arg_position];
                }
                if (num_args > 1)
                {
                    arg2 = argv[start_arg_position+1];
                }
                if (num_args > 2)
                {
                    arg3 = argv[start_arg_position+2];
                }
                if (backwardCompatibleReplacement)
                {
                    return cli.DoOutput(argv_orig, &arg, &subCommandArg);
                } else if (num_args == 1)
                {
                    return cli.DoOutput(argv_orig, &arg);
                } else if (num_args == 2)
                {
                    return cli.DoOutput(argv_orig, &arg, &arg2);
                }
                else if (num_args >= 3)
                {
                    return cli.DoOutput(argv_orig, &arg, &arg2, &arg3);
                }

                return cli.DoOutput(argv_orig);
            }

        private:
            cli::CommandLineInterface& cli;

            OutputCommand& operator=(const OutputCommand&);
    };
}



#endif /* CLI_OUTPUT_H_ */
