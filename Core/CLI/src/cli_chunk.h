/*
 * cli_explain.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mazzin
 */

#ifndef CLI_CHUNK_H_
#define CLI_CHUNK_H_

#include "cli_Parser.h"
#include "cli_Options.h"

#include "misc.h"
#include "sml_Events.h"

namespace cli
{

    class ChunkCommand : public cli::ParserCommand
    {
        public:
            ChunkCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~ChunkCommand() {}
            virtual const char* GetString() const
            {
                return "chunk";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'chunk ?' or 'help chunk' to learn more about the chunking command.";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                bool doRemove = false;
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'r', "remove",             OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
//                        cli.SetError(opt.GetError().c_str());
//                        return cli.AppendError(GetSyntax());
                    }
                    if (opt.GetOption() == -1)
                    {
                        break;
                    }
                    switch (opt.GetOption())
                    {
                        case 'r':
                            doRemove = true;
                            break;
                    }
                }

                std::string arg, arg2, arg3, arg4;
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
                if ((num_args == 3) || (num_args > 4))
                {
                    return cli.SetError("Wrong number of arguments for the chunk command.");
                }
                if (num_args > 2)
                {
                    arg3 = argv[start_arg_position+2];
                }
                if (num_args > 3)
                {
                    arg4 = argv[start_arg_position+3];
                }

                if (num_args == 1)
                {
                    return cli.DoChunk(&arg);
                }
                if (num_args == 4)
                {
                    return cli.DoChunk(&arg, &arg2, &arg3, &arg4, doRemove);
                }

                // case: nothing = full configuration information
                return cli.DoChunk();
            }

        private:
            cli::CommandLineInterface& cli;

            ChunkCommand& operator=(const ChunkCommand&);
    };
}



#endif /* CLI_CHUNK_H_ */
