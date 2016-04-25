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
#include "kernel.h"
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
                return "Syntax: visualize [options]\n"
                	   "        visualize explanation [options]\n"
                       "        visualize [ wm | smem | epmem] [id] [options]\n"
                 	   "                  id:       [soar-id | time-step]\n"
                       "                  options:  --filename <path>     (default \"$SOAR_HOME\\soar_visualization.svg\")\n"
                       "                            --print               (default off)\n"
                       "                            --image-launch        (default off)\n"
                       "                            --raw-launch          (default off)\n"
                       "                            --chunk               (default off)\n"
                       "                            --simple              (default off)\n";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'c', "chunk",                      OPTARG_NONE},
                    {'f', "filename",                   OPTARG_REQUIRED},
                    {'i', "image-launch",               OPTARG_NONE},
                    {'p', "print",                      OPTARG_NONE},
                    {'r', "raw-launch",                 OPTARG_NONE},
                    {'s', "simple",                     OPTARG_NONE},
                    {0, 0,                              OPTARG_NONE}
                };

                Cli::VisualizeBitset options(0);
                std::string lfileName;
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
                        case 'c':
                            options.set(Cli::VISUALIZE_INCLUDE_CHUNK);
                            lfileName = opt.GetOptionArgument();
                            break;

                        case 'f':
                            options.set(Cli::VISUALIZE_FILENAME);
                            lfileName = opt.GetOptionArgument();
                            break;

                        case 'i':
                            options.set(Cli::VISUALIZE_IMAGE_LAUNCH);
                            break;

                        case 'p':
                            options.set(Cli::VISUALIZE_PRINT_TO_SCREEN);
                            break;

                        case 'r':
                            options.set(Cli::VISUALIZE_RAW_LAUNCH);
                            break;

                        case 's':
                            options.set(Cli::VISUALIZE_SIMPLE);
                            break;
                    }
                }
                std::string arg, arg2;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;

                if (num_args > 2)
                {
                    cli.SetError("The visualize command cannot take that many arguments.");
                	return cli.AppendError(GetSyntax());
                }
                if (options.test(Cli::VISUALIZE_FILENAME))
                {
                    if (options.count() != 1)
                    {
                        return cli.SetError("That visualize option cannot be changed while issuing a visualize command.");
                    }
                    return cli.DoVisualize(options, &arg, &arg2);
                }

                if (options.test(Cli::VISUALIZE_FILENAME) ||
                    options.test(Cli::VISUALIZE_IMAGE_LAUNCH) ||
                    options.test(Cli::VISUALIZE_PRINT_TO_SCREEN) ||
                    options.test(Cli::VISUALIZE_SIMPLE) ||
                    options.test(Cli::VISUALIZE_RAW_LAUNCH))
                {
                    if (num_args > 0)
                    {
                        cli.SetError("That visualize options cannot take additional arguments.\n");
                    	return cli.AppendError(GetSyntax());
                    }
                    return cli.DoVisualize(options, &arg, &arg2);
                }

                if (options.test(Cli::VISUALIZE_FILENAME))
                {
                    if ((options.count() != 1) || (num_args > 0))
                    {
                        cli.SetError("Please specify only a filename, for example 'visualize -f myFile'.");
                    	return cli.AppendError(GetSyntax());
                    }
                    return cli.DoVisualize(options, &lfileName, &arg2);
                }

                if (num_args > 0)
                {
                    arg = argv[start_arg_position];
                }
                if (num_args > 1)
                {
                    arg2 = argv[start_arg_position+1];
                }
                if (!cli.DoVisualize(options, &arg, &arg2))
                {
                	return cli.AppendError(GetSyntax());
                }
                return true;
            }

        private:
            cli::Cli& cli;

            VisualizeCommand& operator=(const VisualizeCommand&);
    };
}



#endif /* CLI_VISUALIZE_H_ */
