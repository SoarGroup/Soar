/*
 * cli_wm.h
 *
 *  Created on: Oct 1, 2016
 *      Author: mazzin
 */

#ifndef CORE_CLI_SRC_CLI_WM_H_
#define CORE_CLI_SRC_CLI_WM_H_

#include "cli_Parser.h"
#include "cli_Options.h"
#include "cli_Cli.h"
//#include "misc.h"
//#include "sml_Events.h"

namespace cli
{

    class WMCommand : public cli::ParserCommand
    {
        public:
            WMCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~WMCommand() {}
            virtual const char* GetString() const
            {
                return "wm";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'wm ?' or 'help wm' to learn more about the wm command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "add-filter",     OPTARG_NONE},
                    {'c', "chunks",         OPTARG_NONE},
                    {'d', "default",        OPTARG_NONE},
                    {'g', "get",            OPTARG_NONE},
                    {'h', "history",        OPTARG_NONE},
                    {'j', "justifications", OPTARG_NONE},
                    {'l', "list-filter",    OPTARG_NONE},
                    {'r', "remove-filter",  OPTARG_NONE},
                    {'R', "reset-filter",   OPTARG_NONE},
                    {'s', "set",            OPTARG_NONE},
                    {'S', "stats",          OPTARG_NONE},
                    {'T', "template",       OPTARG_NONE},
                    {'x', "timers",         OPTARG_NONE},
                    {'t', "type",           OPTARG_REQUIRED},
                    {'u', "user",           OPTARG_NONE},
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
                    return cli.SetError("The 'wm' commands contains commands and settings related to working memomory: add, remove, activation and watch.\n\n"
                        "Use 'wm ?' to see an overview of the command or 'help wm' to read the manual page.");
                }
                std::string lCmd;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;
                lCmd = argv[start_arg_position];
                if (num_args > 0)
                {
                    return cli.DoWM(argv, lCmd);
                }
                return cli.DoWM(argv, lCmd);
            }

        private:
            cli::Cli& cli;

            WMCommand& operator=(const WMCommand&);
    };
}




#endif /* CORE_CLI_SRC_CLI_WM_H_ */
