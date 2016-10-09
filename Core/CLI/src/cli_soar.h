/*
 * cli_explain.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mazzin
 */

#ifndef CLI_SOAR_H
#define CLI_SOAR_H

#include "cli_Parser.h"
#include "cli_Options.h"
#include "cli_Cli.h"
#include "misc.h"
#include "sml_Events.h"

namespace cli
{

    class SoarCommand : public cli::ParserCommand
    {
        public:
            SoarCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~SoarCommand() {}
            virtual const char* GetString() const
            {
                return "soar";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'soar ?' and 'help soar' to learn more about the soar command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                std::string subCommandArg, subCommandArgAlt;
                bool backwardCompatibleReplacement = false;

                OptionsData optionsData[] =
                {
                    {'e', "enable",             OPTARG_NONE},
                    {'e', "on",                 OPTARG_NONE},
                    {'d', "disable",            OPTARG_NONE},
                    {'d', "off",                OPTARG_NONE},
                    {'A', "AfterPhaseIgnored",  OPTARG_NONE},
                    {'B', "BeforePhaseIgnored", OPTARG_NONE},
                    {'a', "apply",              OPTARG_NONE},
                    {'d', "decide",             OPTARG_NONE},
                    {'i', "input",              OPTARG_NONE},
                    {'o', "output",             OPTARG_NONE},
                    {'p', "propose",            OPTARG_NONE},
                    {'s', "self",               OPTARG_NONE},
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
                    /* To maintain backwards compatibility with set-stop-phase/waitsnc */
                    switch (opt.GetOption())
                    {
                        case 'A':
                        case 'B':
                            // Ignored options
                            break;
                        case 'e':
                            subCommandArg = "on";
                            backwardCompatibleReplacement = true;
                            break;
                        case 'a':
                            subCommandArg = "apply";
                            backwardCompatibleReplacement = true;
                            break;
                        case 'd':
                            subCommandArg = "decide";
                            subCommandArgAlt = "off";
                            backwardCompatibleReplacement = true;
                            break;
                        case 'i':
                            subCommandArg = "input";
                            backwardCompatibleReplacement = true;
                            break;
                        case 'o':
                            subCommandArg = "output";
                            backwardCompatibleReplacement = true;
                            break;
                        case 'p':
                            subCommandArg = "propose";
                            backwardCompatibleReplacement = true;
                            break;
                        case 's':
                            std::string sarg("stop"), sarg2("self");
                            return cli.DoSoar('G', &sarg, &sarg2);
                            break;
                    }
                }
                std::string arg;
                std::string reasonForStopping;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;

                if (num_args > 0)
                {
                    arg = argv[start_arg_position];
                } else if (num_args > 2)
                {
                    return cli.SetError("Too many arguments for the 'soar' command.");
                }

                if ((num_args == 1) && !backwardCompatibleReplacement)
                {
                    return cli.DoSoar('G', &arg);
                }
                if (backwardCompatibleReplacement || (num_args == 2))
                {
                    return cli.DoSoar('S', &arg, &subCommandArg, &subCommandArgAlt);
                }

                // case: nothing = full configuration information
                return cli.DoSoar();
            }

        private:
            cli::Cli& cli;

            SoarCommand& operator=(const SoarCommand&);
    };
}



#endif /* CLI_SOAR_H */
