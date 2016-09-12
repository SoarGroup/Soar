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
                return  "Try 'output ?' or 'help output' to learn more about the output command.";
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
                        cli.SetError("output takes only one option at a time.");
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
                            return cli.DoOutput('G', &(argv[1]));
                        }
                        if (opt.CheckNumNonOptArgs(2, 2))
                        {
                            return cli.DoOutput('S', &(argv[1]), &(argv[2]));
                        }
                        break;
                }

                // bad: no option, but more than two argument
                if (argv.size() > 3)
                {
                    return cli.SetError("Too many arguments.");
                }

                // case: nothing = full configuration information
                return cli.DoOutput();
            }

        private:
            cli::Cli& cli;

            OutputCommand& operator=(const OutputCommand&);
    };
}



#endif /* CLI_OUTPUT_H_ */
