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
#include "cli_Cli.h"
#include "misc.h"
#include "sml_Events.h"

namespace cli
{

    class OutputCommand : public cli::ParserCommand
    {
        public:
            OutputCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
                bool appendMode = false;
                std::vector< std::string > argv_orig = argv;

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
                        case 'a':
                            appendMode = true;
                            break;
                    }
                }
                if (!opt.GetNonOptionArguments())
                {
                    return cli.SetError("Sub-command is required.");
                }
                std::string arg;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;
                if (num_args > 0)
                {
                    arg = argv[start_arg_position];
                } else if (num_args > 2)
                {
                    return cli.SetError("Too many arguments for the 'output' command.");
                }

                if ((num_args == 1) && !backwardCompatibleReplacement)
                {
                    return cli.DoOutput(argv_orig, 'G', &arg, NULL);
                }
                if (backwardCompatibleReplacement || (num_args >= 2))
                {
                    return cli.DoOutput(argv_orig, 'S', &arg, &subCommandArg, appendMode);
                }

                // case: nothing = full configuration information
                return cli.DoOutput(argv);
            }

        private:
            cli::Cli& cli;

            OutputCommand& operator=(const OutputCommand&);
    };
}



#endif /* CLI_OUTPUT_H_ */
