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
#include "cli_Cli.h"
#include "misc.h"
#include "sml_Events.h"

namespace cli
{

    class ChunkCommand : public cli::ParserCommand
    {
        public:
            ChunkCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~ChunkCommand() {}
            virtual const char* GetString() const
            {
                return "chunk";
            }
            virtual const char* GetSyntax() const
            {
                return  "chunk <command> [parameter value]    (leave empty to see current value)\n"
                    "      ============= When to chunk ============== Value ===\n"
                    "      always | never | only | all-except\n"
                    "      bottom-only                             [ on | off ]\n"
                    "      =============== Settings ================= Value ===\n"
                    "      naming-style                     [ numbered | rule ]\n"
                    "      max-chunks                                 <maximum>\n"
                    "      max-dupe-chunks                            <maximum>\n"
                    "      =========== Debugging Commands =========== Value ===\n"
                    "      ? | help | history | stats \n"
                    "      interrupt                               [ on | off ]\n"
                    "      record-utility                          [ on | off ]\n"
                    "      ============= EBC Mechanisms ============= Value ===\n"
                    "      variablize-identity                     [ on | off ]\n"
                    "      enforce-constraints                     [ on | off ]\n"
                    "      add-osk                                 [ on | off ]\n"
                    "      variablize-rhs-funcs                    [ on | off ]\n"
                    "      repair-rhs                              [ on | off ]\n"
                    "      repair-lhs                              [ on | off ]\n"
                    "      repair-rhs-promotion                    [ on | off ]\n"
                    "      merge                                   [ on | off ]\n"
                    "      user-singletons                         [ on | off ]\n"
                    "      ========== Correctness Filters =========== Value ===\n"
                    "      allow-local-negations                   [ on | off ]\n"
                    "      allow-missing-osk                       [ on | off ]\n"
                    "      allow-opaque                            [ on | off ]\n"
                    "      allow-uncertain-operators               [ on | off ]\n"
                    "      allow-multiple-prefs                    [ on | off ]\n"
                    "      allow-pre-existing-ltm                  [ on | off ]\n"
                    "      allow-local-promotion                   [ on | off ]\n";
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
                        cli.SetError("chunk takes only one option at a time.");
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
                            return cli.DoChunk('G', &(argv[1]));
                        }
                        if (opt.CheckNumNonOptArgs(2, 2))
                        {
                            return cli.DoChunk('S', &(argv[1]), &(argv[2]));
                        }
                        break;
                }

                // bad: no option, but more than two argument
                if (argv.size() > 3)
                {
                    return cli.SetError("Too many arguments.");
                }

                // case: nothing = full configuration information
                return cli.DoChunk();
            }

        private:
            cli::Cli& cli;

            ChunkCommand& operator=(const ChunkCommand&);
    };
}



#endif /* CLI_CHUNK_H_ */
