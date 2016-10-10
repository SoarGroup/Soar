/*
 * cli_explain.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mazzin
 */

#ifndef CLI_EXPLAIN_H_
#define CLI_EXPLAIN_H_

#include "cli_Parser.h"
#include "cli_Options.h"

#include "misc.h"
#include "sml_Events.h"

namespace cli
{

    class ExplainCommand : public cli::ParserCommand
    {
        public:
            ExplainCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~ExplainCommand() {}
            virtual const char* GetString() const
            {
                return "explain";
            }
            virtual const char* GetSyntax() const
            {
                return  "Use 'explain ?' or 'help explain' to learn more about the explain command.";
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

                std::string arg, arg2;
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
                    return cli.SetError("Too many arguments for the 'explain' command.");
                }

                if (num_args == 1)
                {
                    return cli.DoExplain(&arg);
                }
                if (num_args == 2)
                {
                    return cli.DoExplain(&arg, &arg2);
                }

                // case: nothing = full configuration information
                return cli.DoExplain();
            }

        private:
            cli::CommandLineInterface& cli;

            ExplainCommand& operator=(const ExplainCommand&);
    };
}



#endif /* CLI_EXPLAIN_H_ */
