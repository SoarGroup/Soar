/*
 * cli_load_save.h
 *
 *  Created on: Oct 1, 2016
 *      Author: mazzin
 */

#ifndef CORE_CLI_SRC_CLI_LOAD_SAVE_H_
#define CORE_CLI_SRC_CLI_LOAD_SAVE_H_

#include "cli_Parser.h"
#include "cli_Options.h"
#include "cli_Cli.h"
//#include "misc.h"
//#include "sml_Events.h"

namespace cli
{
    class LoadCommand : public cli::ParserCommand
    {
        public:
            LoadCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~LoadCommand() {}
            virtual const char* GetString() const
            {
                return "load";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'load ?' or 'help load' to learn more about the load command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                std::vector< std::string > argv_orig = argv;
                OptionsData optionsData[] =
                {
                    {'c', "close", OPTARG_NONE},
                    {'f', "flush", OPTARG_NONE},
                    {'o', "open", OPTARG_REQUIRED},
                    {'l', "load",        OPTARG_REQUIRED},
                    {'r', "restore",    OPTARG_REQUIRED},
                    {'s', "save",        OPTARG_REQUIRED},
                    {'a', "all",            OPTARG_NONE},
                    {'d', "disable",        OPTARG_NONE},
                    {'v', "verbose",        OPTARG_NONE},
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
                    return cli.SetError("File type is required. \n\nUse 'load ?' or 'help load' to learn more about the load command.");
                }
                std::string lCmd;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;
                if (num_args > 0)
                {
                    lCmd = argv[start_arg_position];
                }
                return cli.DoLoad(argv_orig, lCmd);
            }

        private:
            cli::Cli& cli;

            LoadCommand& operator=(const LoadCommand&);
    };
    class SaveCommand : public cli::ParserCommand
    {
        public:
            SaveCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~SaveCommand() {}
            virtual const char* GetString() const
            {
                return "save";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'save ?' or 'help save' to learn more about the save command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                std::vector< std::string > argv_orig = argv;
                OptionsData optionsData[] =
                {
                    {'c', "close", OPTARG_NONE},
                    {'f', "flush", OPTARG_NONE},
                    {'o', "open", OPTARG_REQUIRED},
                    {'l', "load",        OPTARG_REQUIRED},
                    {'r', "restore",    OPTARG_REQUIRED},
                    {'s', "save",        OPTARG_REQUIRED},
                    {'a', "all",            OPTARG_NONE},
                    {'d', "disable",        OPTARG_NONE},
                    {'v', "verbose",        OPTARG_NONE},
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
                    return cli.SetError("File type is required. \n\nUse 'save ?' or 'help save' to learn more about the save command.");
                }
                std::string lCmd;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;
                if (num_args > 0)
                {
                    lCmd = argv[start_arg_position];
                }
                return cli.DoSave(argv_orig, lCmd);
            }

        private:
            cli::Cli& cli;

            SaveCommand& operator=(const SaveCommand&);
    };
}




#endif /* CORE_CLI_SRC_CLI_LOAD_SAVE_H_ */
