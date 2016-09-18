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
                return "Try 'visualize ?' to learn more about visualize's sub-commands and settings.\n"
                    "(For a detailed article about visualize, use 'help visualize'.)";
            }

            bool validate_yes_no(const std::string& pString, size_t pWhichBit, Cli::VisualizeBitset& enabledBitset, Cli::VisualizeBitset& settingsBitset)
            {
                if (pString.empty()) return false;
                char lChar = pString.at(0);
                if ( (lChar != 'y') && (lChar != 'n'))
                {
                    cli.SetError("Option value must be 'yes' or 'no'.");
                    cli.AppendError(GetSyntax());
                    return false;
                }
                settingsBitset.set(pWhichBit, lChar == 'y');
                enabledBitset.set(pWhichBit);
                return true;
            }
            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "architectural-links",        OPTARG_REQUIRED},
                    {'d', "depth",                      OPTARG_REQUIRED},
                    {'e', "editor-launch",              OPTARG_REQUIRED},
                    {'f', "filename",                   OPTARG_REQUIRED},
                    {'g', "generate-image",             OPTARG_REQUIRED},
                    {'i', "image-type",                 OPTARG_REQUIRED},
                    {'l', "line-style",                 OPTARG_REQUIRED},
                    {'o', "only-show-rule-name",        OPTARG_REQUIRED},
                    {'p', "print",                      OPTARG_REQUIRED},
                    {'u', "use-same-file",              OPTARG_REQUIRED},
                    {'v', "viewer-launch",              OPTARG_REQUIRED},
                    {0, 0,                              OPTARG_NONE}
                };

                Cli::VisualizeBitset options(0), boolSettings(0);
                std::string lfileName, lLineStyle, lImageType;
                int lDepth = 0;

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
                            if (!validate_yes_no(opt.GetOptionArgument(), Cli::VISUALIZE_ARCH_SHOW, options, boolSettings))
                                return false;
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
                            if (!validate_yes_no(opt.GetOptionArgument(), Cli::VISUALIZE_LAUNCH_EDITOR, options, boolSettings))
                                return false;
                            break;

                        case 'f':
                            options.set(Cli::VISUALIZE_FILENAME);
                            lfileName = opt.GetOptionArgument();
                            break;

                        case 'g':
                            if (!validate_yes_no(opt.GetOptionArgument(), Cli::VISUALIZE_GENERATE_IMAGE, options, boolSettings))
                                return false;
                            break;

                        case 'i':
                            options.set(Cli::VISUALIZE_IMAGE_TYPE);
                            lImageType = opt.GetOptionArgument();
                            break;

                        case 'l':
                            options.set(Cli::VISUALIZE_STYLE_LINE);
                            lLineStyle = opt.GetOptionArgument();
                            break;

                        case 'o':
                            if (!validate_yes_no(opt.GetOptionArgument(), Cli::VISUALIZE_ONLY_RULE_NAME, options, boolSettings))
                                return false;
                            break;

                        case 'p':
                            if (!validate_yes_no(opt.GetOptionArgument(), Cli::VISUALIZE_PRINT_TO_SCREEN, options, boolSettings))
                                return false;
                            break;

                        case 'u':
                            if (!validate_yes_no(opt.GetOptionArgument(), Cli::VISUALIZE_USE_SAME_FILE, options, boolSettings))
                                return false;
                            break;

                        case 'v':
                            if (!validate_yes_no(opt.GetOptionArgument(), Cli::VISUALIZE_LAUNCH_VIEWER, options, boolSettings))
                                return false;
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

                if (num_args > 0)
                {
                    arg = argv[start_arg_position];
                }
                if (num_args > 1)
                {
                    arg2 = argv[start_arg_position+1];
                } else {
                    if (arg == "?")
                    {
                        options.reset();
                        boolSettings.reset();
                        std::string dummy1;
                        if (!cli.DoVisualize(options, boolSettings, dummy1, arg2, lfileName, lLineStyle, lImageType, lDepth))
                        {
                            return cli.AppendError(GetSyntax());
                        }
                        return true;
                    }
                }
                if (!num_args && options.none() && boolSettings.none())
                {
                    return cli.AppendError(GetSyntax());
                }
                if (!cli.DoVisualize(options, boolSettings, arg, arg2, lfileName, lLineStyle, lImageType, lDepth))
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
