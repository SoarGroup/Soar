/*
 * cli_memory.h
 *
 *  Created on: Oct 1, 2016
 *      Author: mazzin
 */

#ifndef CORE_CLI_SRC_CLI_MEMORY_H_
#define CORE_CLI_SRC_CLI_MEMORY_H_

#include "cli_Parser.h"
#include "cli_Options.h"
#include "cli_Cli.h"
#include "misc.h"
#include "sml_Events.h"

namespace cli
{

    class MemoryCommand : public cli::ParserCommand
    {
        public:
            MemoryCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~MemoryCommand() {}
            virtual const char* GetString() const
            {
                return "memory";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'memory ?' or 'help memory' to learn more about the memory command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'c', "chunks",            OPTARG_NONE},
                    {'d', "default",        OPTARG_NONE},
                    {'j', "justifications",    OPTARG_NONE},
                    {'T', "template",        OPTARG_NONE},
                    {'u', "user",            OPTARG_NONE},
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
                std::string lCmd;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;
                lCmd = argv[start_arg_position];
                if (num_args > 0)
                {
                    return cli.DoMemory(argv, lCmd);
                }
                return cli.DoMemory(argv, lCmd);
            }

        private:
            cli::Cli& cli;

            MemoryCommand& operator=(const MemoryCommand&);
    };
}


#endif /* CORE_CLI_SRC_CLI_MEMORY_H_ */
