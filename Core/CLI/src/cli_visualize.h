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
                return "Syntax: visualize [ last | instantiations | contributors]  (from explain command analysis)\n"
                       "        visualize [ wm | smem | epmem] [id]       (from current state of memory)\n\n"
                       "                  options:  --architectural-links (default off)"
                       "                            --depth <number>\n"
                       "                            --editor-launch       (default off)\n"
                       "                            --filename <path>     (default \"$SOAR_HOME\\soar_visualization.svg\")\n"
                       "                            --generate-image      (default off)\n"
                       "                            --line-style          (default polyline)\n"
                       "                            --object-style        (simple/complex, default simple)\n"
                       "                            --print               (default off)\n"
                       "                            --use-same-file       (default on)\n"
                       "                            --viewer-launch       (default on)\n";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "architectural-links",        OPTARG_NONE},
                    {'d', "depth",                      OPTARG_REQUIRED},
                    {'e', "editor-launch",              OPTARG_NONE},
                    {'f', "filename",                   OPTARG_REQUIRED},
                    {'g', "generate-image",             OPTARG_NONE},
                    {'l', "line-style",                 OPTARG_REQUIRED},
                    {'o', "object-style",               OPTARG_REQUIRED},
                    {'p', "print",                      OPTARG_NONE},
                    {'u', "use-same-file",              OPTARG_NONE},
                    {'v', "viewer-launch",              OPTARG_NONE},
                    {0, 0,                              OPTARG_NONE}
                };

                Cli::VisualizeBitset options(0);
                std::string lfileName, lLineStyle, lObjectStyle;
                int lDepth;

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
                        case 'a':
                            options.set(Cli::VISUALIZE_ARCH_SHOW);
                            break;

                        case 'd':
                            if (!from_string(lDepth, opt.GetOptionArgument()) || (lDepth < 0))
                            {
                                cli.SetError("Invalid depth value.");
                                return cli.AppendError(GetSyntax());
                            }
                            options.set(Cli::VISUALIZE_DEPTH);
                            break;

                        case 'e':
                            options.set(Cli::VISUALIZE_LAUNCH_EDITOR);
                            break;

                        case 'f':
                            options.set(Cli::VISUALIZE_FILENAME);
                            lfileName = opt.GetOptionArgument();
                            break;

                        case 'g':
                            options.set(Cli::VISUALIZE_GENERATE_IMAGE);
                            break;

                        case 'l':
                            options.set(Cli::VISUALIZE_STYLE_LINE);
                            lLineStyle = opt.GetOptionArgument();
                            break;

                        case 'o':
                            options.set(Cli::VISUALIZE_STYLE_OBJECT);
                            lObjectStyle = opt.GetOptionArgument();
                            break;

                        case 'p':
                            options.set(Cli::VISUALIZE_PRINT_TO_SCREEN);
                            break;

                        case 'u':
                            options.set(Cli::VISUALIZE_USE_SAME_FILE);
                            break;

                        case 'v':
                            options.set(Cli::VISUALIZE_LAUNCH_VIEWER);
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
//                if (options.test(Cli::VISUALIZE_FILENAME))
//                {
//                    if (options.count() != 1)
//                    {
//                        return cli.SetError("That visualize option cannot be changed while issuing a visualize command.");
//                    }
//                    return cli.DoVisualize(options, &arg, &arg2);
//                }
//
//                if (options.test(Cli::VISUALIZE_FILENAME) ||
//                    options.test(Cli::VISUALIZE_STYLE_OBJECT) ||
//                    options.test(Cli::VISUALIZE_PRINT_TO_SCREEN) ||
//                    options.test(Cli::VISUALIZE_STYLE_LINE) ||
//                    options.test(Cli::VISUALIZE_LAUNCH_EDITOR))
//                {
//                    if (num_args > 0)
//                    {
//                        cli.SetError("That visualize options cannot take additional arguments.\n");
//                    	return cli.AppendError(GetSyntax());
//                    }
//                    return cli.DoVisualize(options, &arg, &arg2);
//                }
//
//                if (options.test(Cli::VISUALIZE_FILENAME))
//                {
//                    if ((options.count() != 1) || (num_args > 0))
//                    {
//                        cli.SetError("Please specify only a filename, for example 'visualize -f myFile'.");
//                    	return cli.AppendError(GetSyntax());
//                    }
//                    return cli.DoVisualize(options, &lfileName, &arg2);
//                }

                if (num_args > 0)
                {
                    arg = argv[start_arg_position];
                }
                if (num_args > 1)
                {
                    arg2 = argv[start_arg_position+1];
                }
                if (!cli.DoVisualize(options, &arg, &arg2, &lfileName, &lLineStyle, &lObjectStyle))
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
