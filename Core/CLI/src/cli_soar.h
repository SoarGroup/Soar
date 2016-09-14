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
                return  "Try 'soar ?' or 'help soar' to learn more about the soar command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {0, 0, OPTARG_NONE}
                };

                char option = 0;

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

                    if (option != 0)
                    {
                        cli.SetError("The 'soar' command takes only one option at a time.");
                        return cli.AppendError(GetSyntax());
                    }

                    option = static_cast<char>(opt.GetOption());
                }

                switch (option)
                {
                    case 0:
                    default:
                        // no options
                        if (opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.DoSoar('G', &(argv[1]));
                        }
                        if (opt.CheckNumNonOptArgs(2, 2))
                        {
                            return cli.DoSoar('S', &(argv[1]), &(argv[2]));
                        }
                        break;
                }

                // bad: no option, but more than two argument
                if (argv.size() > 3)
                {
                    return cli.SetError("Too many arguments for the 'soar' command.");
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
