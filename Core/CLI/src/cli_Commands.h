#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include "cli_Parser.h"
#include "cli_Options.h"

#include "cli_CommandLineInterface.h"

#include "misc.h"
#include "sml_Events.h"

namespace cli
{
    class AliasCommand : public cli::ParserCommand
    {
        public:
            explicit AliasCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~AliasCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "alias";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: alias [--remove] [name [cmd args]]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                bool doRemove = false;
                OptionsData optionsData[] =
                {
                    {'r', "remove",             OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE}
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
                std::string arg;
                size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
                size_t num_args = argv.size() - start_arg_position;

                if (num_args == 0)
                {
                    return cli.DoAlias();    // list all
                } else {
                    argv.erase(argv.begin());
                    if (doRemove)
                    {
                        if (num_args > 1)
                        {
                            return cli.SetError("If your alias has -r or --remove, enclose in quotes.");
                        }
                        argv.erase(argv.begin());
                        return cli.DoAlias(&argv, true);
                    }
                    return cli.DoAlias(&argv);
                }
            }

        AliasCommand& operator=(const AliasCommand&) = delete;

        private:
            cli::CommandLineInterface& cli;
    };

    class CDCommand : public cli::ParserCommand
    {
        public:
            explicit CDCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~CDCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "cd";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: cd [directory]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                // Only takes one optional argument, the directory to change into
                if (argv.size() > 2)
                {
                    return cli.SetError("Only one argument (a directory) is allowed. Paths with spaces should be enclosed in quotes.");
                }

                if (argv.size() > 1)
                {
                    return cli.DoCD(&(argv[1]));
                }
                return cli.DoCD();
            }

        CDCommand& operator=(const CDCommand&) = delete;

        private:
            cli::CommandLineInterface& cli;
    };

    class DebugCommand : public cli::ParserCommand
    {
        public:
            explicit DebugCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~DebugCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "debug";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: debug [ allocate | internal-symbols | port | time | ? ] [arguments*]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                if (argv.size() == 1)
                {
                    return cli.DoDebug();    // list all
                }

                argv.erase(argv.begin());

                return cli.DoDebug(&argv);

            }
            DebugCommand& operator=(const DebugCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;
    };


    class DirsCommand : public cli::ParserCommand
    {
        public:
            explicit DirsCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~DirsCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "dirs";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: dirs";
            }

            bool Parse(std::vector< std::string >&) override
            {
                return cli.DoDirs();
            }

            DirsCommand& operator=(const DirsCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class EchoCommand : public cli::ParserCommand
    {
        public:
            explicit EchoCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~EchoCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "echo";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: echo [--nonewline] [string]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'n', "nonewline", OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE}
                };

                bool echoNewline(true);

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError());
                    }

                    if (opt.GetOption() == -1)
                    {
                        break;
                    }

                    switch (opt.GetOption())
                    {
                        case 'n':
                            echoNewline = false;
                            break;
                    }
                }

                // remove the -n arg
                if (!echoNewline)
                {
                    argv.erase(++argv.begin());
                }

                return cli.DoEcho(argv, echoNewline);
            }

            EchoCommand& operator=(const EchoCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class EpMemCommand : public cli::ParserCommand
    {
        public:
            explicit EpMemCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~EpMemCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "epmem";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: epmem [options]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'b', "backup",       OPTARG_NONE},
                    {'c', "close",        OPTARG_NONE},
                    {'d', "disable",      OPTARG_NONE},
                    {'d', "off",          OPTARG_NONE},
                    {'e', "enable",       OPTARG_NONE},
                    {'e', "on",           OPTARG_NONE},
                    {'i', "init",         OPTARG_NONE},
                    {'g', "get",          OPTARG_NONE},
                    {'p', "print",        OPTARG_NONE},
                    {'s', "set",          OPTARG_NONE},
                    {'S', "stats",        OPTARG_NONE},
                    {'t', "timers",       OPTARG_NONE},
                    {'v', "viz",          OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE} // null
                };

                char option = 0;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError());
                    }

                    if (opt.GetOption() == -1)
                    {
                        break;
                    }

                    if (option != 0)
                    {
                        return cli.SetError("Invalid parameters.");
                    }
                    option = static_cast<char>(opt.GetOption());
                }

                switch (option)
                {
                    case 0:
                    default:
                        // no options
                        break;

                    case 'i':
                    case 'e':
                    case 'd':
                    case 'c':
                        // case: init, close, on and off get no arguments
                    {
                        if (!opt.CheckNumNonOptArgs(0, 0))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoEpMem(option);
                    }

                    case 'b':
                        // case: backup requires one non-option argument
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoEpMem(option, &(argv[2]));

                    case 'g':
                        // case: get requires one non-option argument
                    {
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoEpMem(option, &(argv[2]));
                    }

                    case 'p':
                        // case: print takes one non-option argument
                    {
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        std::string temp_str(argv[2]);
                        epmem_time_id memory_id;

                        if (!from_string(memory_id, temp_str))
                        {
                            return cli.SetError("Invalid epmem time tag.");
                        }

                        return cli.DoEpMem(option, nullptr, nullptr, memory_id);
                    }

                    case 's':
                        // case: set requires two non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(2, 2))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoEpMem('s', &(argv[2]), &(argv[3]));
                    }

                    case 'S':
                    case 't':
                        // case: stat and timer can do zero or one non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(0, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        if (opt.GetNonOptionArguments() == 0)
                        {
                            return cli.DoEpMem(option);
                        }

                        return cli.DoEpMem(option, &(argv[2]));
                    }

                    case 'v':
                        // case: viz takes one non-option argument
                    {
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        std::string temp_str(argv[2]);
                        epmem_time_id memory_id;

                        if (!from_string(memory_id, temp_str))
                        {
                            return cli.SetError("Invalid epmem time tag.");
                        }

                        return cli.DoEpMem(option, nullptr, nullptr, memory_id);
                    }
                }

                // bad: no option, but more than one argument
                if (argv.size() > 1)
                {
                    if (opt.CheckNumNonOptArgs(1, 1) && argv[1][0] == '?')
                    {
                        return cli.DoEpMem('?');
                    }
                    return cli.SetError("Too many arguments, check syntax.");
                }

                // case: nothing = full configuration information
                return cli.DoEpMem();
            }

            EpMemCommand& operator=(const EpMemCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class GPCommand : public cli::ParserCommand
    {
        public:
            explicit GPCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~GPCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "gp";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: gp { production_body }";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                // One argument
                if (argv.size() < 2)
                {
                    return cli.SetError(GetSyntax());
                }
                if (argv.size() > 2)
                {
                    return cli.SetError(GetSyntax());
                }

                return cli.DoGP(argv[1]);
            }

            GPCommand& operator=(const GPCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class HelpCommand : public cli::ParserCommand
    {
        public:
            explicit HelpCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~HelpCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "help";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: help [command]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                return cli.DoHelp(argv);
            }

            HelpCommand& operator=(const HelpCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };


    class LearnCommand : public cli::ParserCommand
    {
        public:
            explicit LearnCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~LearnCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "learn";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: learn [-abdeElonNpP]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "all-levels",    OPTARG_NONE},
                    {'b', "bottom-up",    OPTARG_NONE},
                    {'d', "disable",    OPTARG_NONE},
                    {'d', "off",        OPTARG_NONE},
                    {'e', "enable",        OPTARG_NONE},
                    {'e', "on",            OPTARG_NONE},
                    {'E', "except",        OPTARG_NONE},
                    {'l', "list",        OPTARG_NONE},
                    {'o', "only",        OPTARG_NONE},
                    {'n', "local-negations", OPTARG_NONE},
                    {'N', "no-local-negations", OPTARG_NONE},
                    {'p', "desirability-prefs", OPTARG_NONE},
                    {'P', "no-desirability-prefs", OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE}
                };

                cli::LearnBitset options(0);

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError());
                    }

                    if (opt.GetOption() == -1)
                    {
                        break;
                    }

                    switch (opt.GetOption())
                    {
                        case 'a':
                            options.set(cli::LEARN_ALL_LEVELS);
                            break;
                        case 'b':
                            options.set(cli::LEARN_BOTTOM_UP);
                            break;
                        case 'd':
                            options.set(cli::LEARN_DISABLE);
                            break;
                        case 'e':
                            options.set(cli::LEARN_ENABLE);
                            break;
                        case 'E':
                            options.set(cli::LEARN_EXCEPT);
                            break;
                        case 'l':
                            options.set(cli::LEARN_LIST);
                            break;
                        case 'o':
                            options.set(cli::LEARN_ONLY);
                            break;
                        case 'n':
                            options.set(cli::LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS);
                            break;
                        case 'N':
                            options.set(cli::LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS);
                            break;
                        case 'p':
                            options.set(cli::LEARN_ENABLE_THROUGH_EVALUATION_RULES);
                            break;
                        case 'P':
                            options.set(cli::LEARN_DISABLE_THROUGH_EVALUATION_RULES);
                            break;
                    }
                }

                // No non-option arguments
                if (opt.GetNonOptionArguments())
                {
                    return cli.SetError(GetSyntax());
                }

                return cli.DoLearn(options);
            }

            LearnCommand& operator=(const LearnCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class LSCommand : public cli::ParserCommand
    {
        public:
            explicit LSCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~LSCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "ls";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: ls";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                // No arguments
                if (argv.size() != 1)
                {
                    return cli.SetError(GetSyntax());
                }
                return cli.DoLS();
            }

            LSCommand& operator=(const LSCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class PopDCommand : public cli::ParserCommand
    {
        public:
            explicit PopDCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~PopDCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "popd";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: popd";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                // No arguments
                if (argv.size() != 1)
                {
                    return cli.SetError(GetSyntax());
                }
                return cli.DoPopD();
            }

            PopDCommand& operator=(const PopDCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class PreferencesCommand : public cli::ParserCommand
    {
        public:
            explicit PreferencesCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~PreferencesCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "preferences";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: preferences [options] [identifier [attribute]]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'0', "none",        OPTARG_NONE},
                    {'n', "names",        OPTARG_NONE},
                    {'1', "names",        OPTARG_NONE},
                    {'N', "names",        OPTARG_NONE},
                    {'2', "timetags",    OPTARG_NONE},
                    {'t', "timetags",    OPTARG_NONE},
                    {'3', "wmes",        OPTARG_NONE},
                    {'w', "wmes",        OPTARG_NONE},
                    {'o', "object",        OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE}
                };

                cli::ePreferencesDetail detail = cli::PREFERENCES_ONLY;
                bool object = false;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError());
                    }
                    if (opt.GetOption() == -1)
                    {
                        break;
                    }

                    switch (opt.GetOption())
                    {
                        case '0':
                            detail = cli::PREFERENCES_ONLY;
                            break;
                        case '1':
                        case 'n':
                        case 'N':
                            detail = cli::PREFERENCES_NAMES;
                            break;
                        case '2':
                        case 't':
                            detail = cli::PREFERENCES_TIMETAGS;
                            break;
                        case '3':
                        case 'w':
                            detail = cli::PREFERENCES_WMES;
                            break;

                        case 'o':
                        case 'O':
                            object = true;
                            break;
                    }
                }

                // Up to two non-option arguments allowed, id/attribute
                if (opt.GetNonOptionArguments() > 2)
                {
                    return cli.SetError(GetSyntax());
                }

                int optind = opt.GetArgument() - opt.GetNonOptionArguments();
                if (opt.GetNonOptionArguments() == 2)
                {
                    // id & attribute
                    return cli.DoPreferences(detail, object, &argv[optind], &argv[optind + 1]);
                }
                if (opt.GetNonOptionArguments() == 1)
                {
                    // id
                    return cli.DoPreferences(detail, object, &argv[optind]);
                }

                return cli.DoPreferences(detail, object);
            }

            PreferencesCommand& operator=(const PreferencesCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class PrintCommand : public cli::ParserCommand
    {
        public:
            explicit PrintCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~PrintCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "print";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: print [options] [production_name]\nprint [options] identifier|timetag|pattern";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "all",            OPTARG_NONE},
                    {'c', "chunks",         OPTARG_NONE},
                    {'d', "depth",          OPTARG_REQUIRED},
                    {'D', "defaults",       OPTARG_NONE},
                    {'e', "exact",          OPTARG_NONE},
                    {'f', "full",           OPTARG_NONE},
                    {'F', "filename",       OPTARG_NONE},
                    {'g', "gds",            OPTARG_NONE},
                    {'i', "internal",       OPTARG_NONE},
                    {'j', "justifications", OPTARG_NONE},
                    {'n', "name",           OPTARG_NONE},
                    {'o', "operators",      OPTARG_NONE},
                    {'r', "rl",             OPTARG_NONE},
                    {'s', "stack",          OPTARG_NONE},
                    {'S', "states",         OPTARG_NONE},
                    {'t', "tree",           OPTARG_NONE},
                    {'T', "templates",      OPTARG_NONE},
                    {'u', "user",           OPTARG_NONE},
                    {'v', "varprint",       OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE}
                };

                int depth = -1;
                cli::PrintBitset options(0);

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError());
                    }
                    if (opt.GetOption() == -1)
                    {
                        break;
                    }

                    switch (opt.GetOption())
                    {
                        case 'a':
                            options.set(cli::PRINT_ALL);
                            break;
                        case 'c':
                            options.set(cli::PRINT_CHUNKS);
                            break;
                        case 'd':
                            options.set(cli::PRINT_DEPTH);
                            if (!from_string(depth, opt.GetOptionArgument()) || (depth < 0))
                            {
                                return cli.SetError("Non-negative depth expected.");
                            }
                            break;
                        case 'D':
                            options.set(cli::PRINT_DEFAULTS);
                            break;
                        case 'e':
                            options.set(cli::PRINT_EXACT);
                            break;
                        case 'f':
                            options.set(cli::PRINT_FULL);
                            break;
                        case 'F':
                            options.set(cli::PRINT_FILENAME);
                            break;
                        case 'g':
                            options.set(cli::PRINT_GDS);
                            break;
                        case 'i':
                            options.set(cli::PRINT_INTERNAL);
                            break;
                        case 'j':
                            options.set(cli::PRINT_JUSTIFICATIONS);
                            break;
                        case 'n':
                            options.set(cli::PRINT_NAME);
                            break;
                        case 'o':
                            options.set(cli::PRINT_OPERATORS);
                            break;
                        case 'r':
                            options.set(cli::PRINT_RL);
                            break;
                        case 's':
                            options.set(cli::PRINT_STACK);
                            break;
                        case 'S':
                            options.set(cli::PRINT_STATES);
                            break;
                        case 't':
                            options.set(cli::PRINT_TREE);
                            break;
                        case 'T':
                            options.set(cli::PRINT_TEMPLATE);
                            break;
                        case 'u':
                            options.set(cli::PRINT_USER);
                            break;
                        case 'v':
                            options.set(cli::PRINT_VARPRINT);
                            break;
                    }
                }

                // STATES and OPERATORS are sub-options of STACK
                if (options.test(cli::PRINT_OPERATORS) || options.test(cli::PRINT_STATES))
                {
                    if (!options.test(cli::PRINT_STACK))
                    {
                        return cli.SetError("Options --operators (-o) and --states (-S) are only valid when printing the stack.");
                    }
                }

                if (opt.GetNonOptionArguments() == 0)
                {
                    // d and t options require an argument
                    if (options.test(cli::PRINT_TREE) || options.test(cli::PRINT_DEPTH))
                    {
                        return cli.SetError(GetSyntax());
                    }
                    return cli.DoPrint(options, depth);
                }

                // the acDjus options don't allow an argument
                if (options.test(cli::PRINT_ALL)
                        || options.test(cli::PRINT_CHUNKS)
                        || options.test(cli::PRINT_DEFAULTS)
                        || options.test(cli::PRINT_GDS)
                        || options.test(cli::PRINT_JUSTIFICATIONS)
                        || options.test(cli::PRINT_RL)
                        || options.test(cli::PRINT_TEMPLATE)
                        || options.test(cli::PRINT_USER)
                        || options.test(cli::PRINT_STACK))
                {
                    return cli.SetError("No argument allowed when printing all/chunks/defaults/GDS/justifications/rl/template/user/stack.");
                }
                if (options.test(cli::PRINT_EXACT) && (options.test(cli::PRINT_DEPTH) || options.test(cli::PRINT_TREE)))
                {
                    return cli.SetError("No depth/tree flags allowed when printing exact.");
                }

                std::string arg;
                for (size_t i = opt.GetArgument() - opt.GetNonOptionArguments(); i < argv.size(); ++i)
                {
                    if (!arg.empty())
                    {
                        arg.push_back(' ');
                    }
                    arg.append(argv[i]);
                }
                return cli.DoPrint(options, depth, &arg);
            }

            PrintCommand& operator=(const PrintCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class PushDCommand : public cli::ParserCommand
    {
        public:
            explicit PushDCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~PushDCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "pushd";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: pushd directory";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                // Only takes one argument, the directory to change into
                if (argv.size() < 2)
                {
                    return cli.SetError(GetSyntax());
                }
                if (argv.size() > 2)
                {
                    return cli.SetError("Expected on argument (directory). Enclose directory in quotes if there are spaces in the path.");
                }
                return cli.DoPushD(argv[1]);
            }

            PushDCommand& operator=(const PushDCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class PWDCommand : public cli::ParserCommand
    {
        public:
            explicit PWDCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~PWDCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "pwd";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: pwd";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                // No arguments to print working directory
                if (argv.size() != 1)
                {
                    return cli.SetError(GetSyntax());
                }
                return cli.DoPWD();
            }

            PWDCommand& operator=(const PWDCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class RLCommand : public cli::ParserCommand
    {
        public:
            explicit RLCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~RLCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "rl";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: rl [options parameter|statistic]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'g', "get",    OPTARG_NONE},
                    {'s', "set",    OPTARG_NONE},
                    {'t', "trace",    OPTARG_NONE},
                    {'S', "stats",    OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE} // null
                };

                char option = 0;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError());
                    }

                    if (opt.GetOption() == -1)
                    {
                        break;
                    }

                    if (option != 0)
                    {
                        return cli.SetError("rl takes only one option at a time.");
                    }
                    option = static_cast<char>(opt.GetOption());
                }

                switch (option)
                {
                    case 0:
                    default:
                        // no options
                        break;

                    case 'g':
                        // case: get requires one non-option argument
                    {
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoRL(option, &(argv[2]));
                    }

                    case 's':
                        // case: set requires two non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(2, 2))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoRL(option, &(argv[2]), &(argv[3]));
                    }

                    case 't':
                        // case: trace can do 0-2 non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(0, 2))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        if (opt.GetNonOptionArguments() == 0)
                        {
                            return cli.DoRL(option);
                        }
                        else if (opt.GetNonOptionArguments() == 1)
                        {
                            return cli.DoRL(option, &(argv[2]));
                        }

                        return cli.DoRL(option, &(argv[2]), &(argv[3]));
                    }

                    case 'S':
                        // case: stat and trace can do zero or one non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(0, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        if (opt.GetNonOptionArguments() == 0)
                        {
                            return cli.DoRL(option);
                        }

                        return cli.DoRL(option, &(argv[2]));
                    }
                }

                // bad: no option, but more than one argument
                if (argv.size() > 1)
                {
                    return cli.SetError("Invalid syntax.");
                }

                // case: nothing = full configuration information
                return cli.DoRL();
            }

            RLCommand& operator=(const RLCommand&) = delete;
        private:
            cli::CommandLineInterface& cli;

    };

    class RunCommand : public cli::ParserCommand
    {
        public:
            explicit RunCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~RunCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "run";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: run  [-f|count]\nrun -[d|e|o|p][s][un][g] [f|count]\nrun -[d|e|o|p][un] count [-i e|p|d|o]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'d', "decision",        OPTARG_NONE},
                    {'e', "elaboration",    OPTARG_NONE},
                    {'g', "goal",            OPTARG_NONE},
                    {'i', "interleave",        OPTARG_REQUIRED},
                    {'n', "noupdate",        OPTARG_NONE},
                    {'o', "output",            OPTARG_NONE},
                    {'p', "phase",            OPTARG_NONE},
                    {'s', "self",            OPTARG_NONE},
                    {'u', "update",            OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE}
                };

                cli::RunBitset options(0);
                cli::eRunInterleaveMode interleaveMode = cli::RUN_INTERLEAVE_DEFAULT;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError());
                    }
                    if (opt.GetOption() == -1)
                    {
                        break;
                    }

                    switch (opt.GetOption())
                    {
                        case 'd':
                            options.set(cli::RUN_DECISION);
                            break;
                        case 'e':
                            options.set(cli::RUN_ELABORATION);
                            break;
                        case 'g':
                            options.set(cli::RUN_GOAL);
                            break;
                        case 'i':
                            options.set(cli::RUN_INTERLEAVE);
                            interleaveMode = ParseRunInterleaveOptarg(opt);
                            if (interleaveMode == cli::RUN_INTERLEAVE_DEFAULT)
                            {
                                return cli.SetError(opt.GetError());    // error set in parse function
                            }
                            break;
                        case 'o':
                            options.set(cli::RUN_OUTPUT);
                            break;
                        case 'p':
                            options.set(cli::RUN_PHASE);
                            break;
                        case 's':
                            options.set(cli::RUN_SELF);
                            break;
                        case 'u':
                            options.set(cli::RUN_UPDATE) ;
                            break ;
                        case 'n':
                            options.set(cli::RUN_NO_UPDATE) ;
                            break ;
                    }
                }

                // Only one non-option argument allowed, count
                if (opt.GetNonOptionArguments() > 1)
                {
                    return cli.SetError(GetSyntax());
                }

                // Decide if we explicitly indicated how to run
                bool specifiedType = (options.test(cli::RUN_ELABORATION) || options.test(cli::RUN_DECISION) || options.test(cli::RUN_PHASE) || options.test(cli::RUN_OUTPUT)) ;

                // Count defaults to -1
                int count = -1;
                if (opt.GetNonOptionArguments() == 1)
                {
                    int optind = opt.GetArgument() - opt.GetNonOptionArguments();
                    if (!from_string(count, argv[optind]))
                    {
                        return cli.SetError("Integer count expected.");
                    }
                    // Allow "run 0" for decisions -- which means run agents to the current stop-before phase
                    if (count < 0 || (count == 0 && specifiedType && !options.test(cli::RUN_DECISION)))
                    {
                        return cli.SetError("Count must be positive.");
                    }
                }

                return cli.DoRun(options, count, interleaveMode);
            }

            RunCommand& operator=(const RunCommand&) = delete;

        private:
            cli::CommandLineInterface& cli;

            cli::eRunInterleaveMode ParseRunInterleaveOptarg(cli::Options& opt)
            {
                if (opt.GetOptionArgument() == "d")
                {
                    return cli::RUN_INTERLEAVE_DECISION;
                }
                else if (opt.GetOptionArgument() == "e")
                {
                    return cli::RUN_INTERLEAVE_ELABORATION;
                }
                else if (opt.GetOptionArgument() == "o")
                {
                    return cli::RUN_INTERLEAVE_OUTPUT;
                }
                else if (opt.GetOptionArgument() == "p")
                {
                    return cli::RUN_INTERLEAVE_PHASE;
                }

                cli.SetError("Invalid interleave switch: " + opt.GetOptionArgument());
                return cli::RUN_INTERLEAVE_DEFAULT;
            }

    };

    class SMemCommand : public cli::ParserCommand
    {
        public:
            explicit SMemCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~SMemCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "smem";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: smem [options]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "add",        OPTARG_NONE},
                    {'b', "backup",     OPTARG_NONE},
                    {'c', "clear",      OPTARG_NONE},
                    {'d', "disable",    OPTARG_NONE},
                    {'d', "off",        OPTARG_NONE},
                    {'e', "enable",     OPTARG_NONE},
                    {'e', "on",         OPTARG_NONE},
                    {'g', "get",        OPTARG_NONE},
                    {'h', "history",    OPTARG_NONE},//Testing/unstable - 23-7-2014
                    {'i', "init",       OPTARG_NONE},
                    {'P', "precalculate", OPTARG_NONE},
                    {'q', "query",      OPTARG_NONE},//Testing/unstable - 23-7-2014
                    {'r', "remove",     OPTARG_NONE},//Testing/unstable - 23-7-2014
                    {'s', "set",        OPTARG_NONE},
                    {'S', "stats",      OPTARG_NONE},
                    {'t', "timers",     OPTARG_NONE},
                    {'x', "export",     OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE} // null
                };

                char option = 0;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError());
                    }

                    if (opt.GetOption() == -1)
                    {
                        break;
                    }

                    if (option != 0)
                    {
                        return cli.SetError("smem takes only one option at a time.");
                    }

                    option = static_cast<char>(opt.GetOption());
                }

                switch (option)
                {
                    case 0:
                    default:
                        // no options
                        // case: init takes no arguments
                        break;

                    case 'a':
                    case 'b':
                        // case: add and backup require one non-option argument
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoSMem(option, &(argv[2]));

                    case 'g':
                    {
                        // case: get requires one non-option argument
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoSMem(option, &(argv[2]));
                    }

                    case 'h':
                    {
                        // case: history only accepts 1 non-option argument
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoSMem(option, &(argv[2]), nullptr);
                    }
                    case 'c':
                    case 'i':
                    case 'e':
                    case 'd':
                        // case: init takes no arguments
                        if (!opt.CheckNumNonOptArgs(0, 0))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoSMem(option);

                    case 'x':
                    {
                        // case: export does 1-2 non-option arguments
                        if (!opt.CheckNumNonOptArgs(1, 2))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        if (opt.GetNonOptionArguments() == 1)
                        {
                            return cli.DoSMem(option, &(argv[2]), nullptr);
                        }

                        return cli.DoSMem(option, &(argv[2]), &(argv[3]), nullptr);
                    }

                    case 'q':
                    {
                        // case: query requires one non-option argument, but can have a depth argument
                        if (!opt.CheckNumNonOptArgs(1, 2))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        if (opt.GetNonOptionArguments() == 1)
                        {
                            return cli.DoSMem(option, &(argv[2]));
                        }

                        return cli.DoSMem(option, &(argv[2]), &(argv[3]));// This is the case of "depth".
                    }

                    case 'P':
                        // case: precalculate takes no arguments
                        if (!opt.CheckNumNonOptArgs(0,0))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoSMem(option);

                    case 'r':
                    {
                        // case: remove requires one non-option argument, but can have a "force" argument
                        if (!opt.CheckNumNonOptArgs(1, 2))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        if (opt.GetNonOptionArguments() == 1)
                        {
                            return cli.DoSMem(option, &(argv[2]));
                        }

                        return cli.DoSMem(option, &(argv[2]), &(argv[3]));//
                    }

                    case 's':
                    {
                        // case: set requires two non-option arguments
                        if (!opt.CheckNumNonOptArgs(2, 2))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        return cli.DoSMem(option, &(argv[2]), &(argv[3]));
                    }

                    case 'S':
                    {
                        // case: stat can do zero or one non-option arguments
                        if (!opt.CheckNumNonOptArgs(0, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        if (opt.GetNonOptionArguments() == 0)
                        {
                            return cli.DoSMem('S');
                        }

                        return cli.DoSMem(option, &(argv[2]));
                    }

                    case 't':
                    {
                        // case: timer can do zero or one non-option arguments
                        if (!opt.CheckNumNonOptArgs(0, 1))
                        {
                            return cli.SetError(opt.GetError());
                        }

                        if (opt.GetNonOptionArguments() == 0)
                        {
                            return cli.DoSMem('t');
                        }

                        return cli.DoSMem(option, &(argv[2]));
                    }

                }

                // bad: no option, but more than one argument
                if (argv.size() > 1)
                {
                    if (opt.CheckNumNonOptArgs(1, 1) && argv[1][0] == '?')
                    {
                        return cli.DoSMem('?');
                    }
                    return cli.SetError("Too many arguments.");
                }

                // case: nothing = full configuration information
                return cli.DoSMem();
            }

            SMemCommand& operator=(const SMemCommand&) = delete;

        private:
            cli::CommandLineInterface& cli;
    };

    class SPCommand : public cli::ParserCommand
    {
        public:
            explicit SPCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~SPCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "sp";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return
                    "Syntax: sp {production_body}";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                // One argument (the stuff in the brackets, minus the brackets
                if (argv.size() < 2)
                {
                    return cli.SetError(GetSyntax());
                }
                if (argv.size() > 2)
                {
                    return cli.SetError(GetSyntax());
                }

                return cli.DoSP(argv[1]);
            }

            SPCommand& operator=(const SPCommand&) = delete;

        private:
            cli::CommandLineInterface& cli;
    };

    class StatsCommand : public cli::ParserCommand
    {
        public:
            explicit StatsCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~StatsCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "stats";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return
                    "Syntax: stats [options]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'d', "decision",   OPTARG_NONE},
                    {'l', "learning",   OPTARG_NONE},
                    {'m', "memory",     OPTARG_NONE},
                    {'M', "max",        OPTARG_NONE},
                    {'r', "rete",       OPTARG_NONE},
                    {'s', "system",     OPTARG_NONE},
                    {'R', "reset",      OPTARG_NONE},
                    {'t', "track",      OPTARG_NONE},
                    {'T', "stop-track", OPTARG_NONE},
                    {'c', "cycle",      OPTARG_NONE},
                    {'C', "cycle-csv",  OPTARG_NONE},
                    {'S', "sort",       OPTARG_REQUIRED},
                    {'a', "agent",      OPTARG_NONE},
                    {0, nullptr, OPTARG_NONE}
                };

                cli::StatsBitset options(0);
                int sort = 0;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError());
                    }
                    if (opt.GetOption() == -1)
                    {
                        break;
                    }

                    switch (opt.GetOption())
                    {
                        case 'd':
                            options.set(cli::STATS_DECISION);
                            break;
                        case 'l':
                            options.set(cli::STATS_EBC);
                            break;
                        case 'm':
                            options.set(cli::STATS_MEMORY);
                            break;
                        case 'M':
                            options.set(cli::STATS_MAX);
                            break;
                        case 'r':
                            options.set(cli::STATS_RETE);
                            break;
                        case 'R':
                            options.set(cli::STATS_RESET);
                            break;
                        case 's':
                            options.set(cli::STATS_SYSTEM);
                            break;
                        case 't':
                            options.set(cli::STATS_TRACK);
                            break;
                        case 'T':
                            options.set(cli::STATS_STOP_TRACK);
                            break;
                        case 'c':
                            options.set(cli::STATS_CYCLE);
                            break;
                        case 'C':
                            options.set(cli::STATS_CSV);
                            break;
                        case 'S':
                            options.set(cli::STATS_CYCLE);
                            if (!from_string(sort, opt.GetOptionArgument()))
                            {
                                return cli.SetError("Integer expected");
                            }
                            break;
                        case 'a':
                            options.set(cli::STATS_AGENT);
                            break;
                    }
                }

                // No arguments
                if (opt.GetNonOptionArguments())
                {
                    return cli.SetError(GetSyntax());
                }

                return cli.DoStats(options, sort);
            }

            StatsCommand& operator=(const StatsCommand&) = delete;

        private:
            cli::CommandLineInterface& cli;
    };

    class SVSCommand : public cli::ParserCommand
    {
        public:
            explicit SVSCommand(cli::CommandLineInterface& cli) : ParserCommand(), cli(cli) {}
            ~SVSCommand() override = default;
            [[nodiscard]] const char* GetString() const override
            {
                return "svs";
            }
            [[nodiscard]] const char* GetSyntax() const override
            {
                return "Syntax: svs <elements to inspect>\n"
                       "        svs [--enable | -e | --on | --disable | -d | --off | --enable-in-substates | --disable-in-substates ]";
            }

            bool Parse(std::vector< std::string >& argv) override
            {
                return cli.DoSVS(argv);
            }

        private:
            cli::CommandLineInterface& cli;
    };

}

#endif // CLI_COMMANDS_H
