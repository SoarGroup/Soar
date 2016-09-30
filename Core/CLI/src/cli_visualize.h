/*
 * cli_visualize.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mazzin
 */

#ifndef CLI_VISUALIZE_H_
#define CLI_VISUALIZE_H_

#include "cli_Parser.h"
#include "cli_Options.h"
#include "cli_Cli.h"
#include "misc.h"
#include "sml_Events.h"

namespace cli
{

    class VisualizeCommand : public cli::ParserCommand
    {
        public:
            VisualizeCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~VisualizeCommand() {}
            virtual const char* GetString() const
            {
                return "visualize";
            }
            virtual const char* GetSyntax() const
            {
                return "Use 'visualize ?' or 'help soar' to learn more about the visualize command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {0, 0,                              OPTARG_NONE}
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

                std::string arg, arg2, arg3;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;
                if (num_args > 0)
                {
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
                if (num_args > 3)
                {
                    return cli.SetError("Too many arguments for the 'visualize' command.");
                }

                if (num_args == 1)
                {
                    return cli.DoVisualize(&arg);
                }
                if (num_args == 2)
                {
                    return cli.DoVisualize(&arg, &arg2);
                }
                if (num_args == 3)
                {
                    return cli.DoVisualize(&arg, &arg2, &arg3);
                }

                // case: nothing = full configuration information
                return cli.DoVisualize();
            }

        private:
            cli::Cli& cli;

            VisualizeCommand& operator=(const VisualizeCommand&);
    };
}



#endif /* CLI_VISUALIZE_H_ */
