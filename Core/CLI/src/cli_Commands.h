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
            AliasCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~AliasCommand() {}
            virtual const char* GetString() const
            {
                return "alias";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: alias [--remove] [name [cmd args]]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                bool doRemove = false;
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

        private:
            cli::CommandLineInterface& cli;

            AliasCommand& operator=(const AliasCommand&);
    };

    class CDCommand : public cli::ParserCommand
    {
        public:
            CDCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~CDCommand() {}
            virtual const char* GetString() const
            {
                return "cd";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: cd [directory]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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

        private:
            cli::CommandLineInterface& cli;

            CDCommand& operator=(const CDCommand&);
    };

    class DebugCommand : public cli::ParserCommand
    {
        public:
            DebugCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~DebugCommand() {}
            virtual const char* GetString() const
            {
                return "debug";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: debug [ allocate | internal-symbols | port | time | ? ] [arguments*]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                if (argv.size() == 1)
                {
                    return cli.DoDebug();    // list all
                }

                argv.erase(argv.begin());

                return cli.DoDebug(&argv);

            }
        private:
            cli::CommandLineInterface& cli;
            DebugCommand& operator=(const DebugCommand&);
    };


    class DirsCommand : public cli::ParserCommand
    {
        public:
            DirsCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~DirsCommand() {}
            virtual const char* GetString() const
            {
                return "dirs";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: dirs";
            }

            virtual bool Parse(std::vector< std::string >&)
            {
                return cli.DoDirs();
            }

        private:
            cli::CommandLineInterface& cli;

            DirsCommand& operator=(const DirsCommand&);
    };

    class EchoCommand : public cli::ParserCommand
    {
        public:
            EchoCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~EchoCommand() {}
            virtual const char* GetString() const
            {
                return "echo";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: echo [--nonewline] [string]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'n', "no-newline", OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
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

        private:
            cli::CommandLineInterface& cli;

            EchoCommand& operator=(const EchoCommand&);
    };

    class EpMemCommand : public cli::ParserCommand
    {
        public:
            EpMemCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~EpMemCommand() {}
            virtual const char* GetString() const
            {
                return "epmem";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: epmem [options]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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
                    {0, 0, OPTARG_NONE} // null
                };

                char option = 0;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError().c_str());
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
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoEpMem(option);
                    }

                    case 'b':
                        // case: backup requires one non-option argument
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoEpMem(option, &(argv[2]));

                    case 'g':
                        // case: get requires one non-option argument
                    {
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoEpMem(option, &(argv[2]));
                    }

                    case 'p':
                        // case: print takes one non-option argument
                    {
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        std::string temp_str(argv[2]);
                        epmem_time_id memory_id;

                        if (!from_string(memory_id, temp_str))
                        {
                            return cli.SetError("Invalid epmem time tag.");
                        }

                        return cli.DoEpMem(option, 0, 0, memory_id);
                    }

                    case 's':
                        // case: set requires two non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(2, 2))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoEpMem('s', &(argv[2]), &(argv[3]));
                    }

                    case 'S':
                        // case: stat can do zero or one non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(0, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        if (opt.GetNonOptionArguments() == 0)
                        {
                            return cli.DoEpMem(option);
                        }

                        return cli.DoEpMem(option, &(argv[2]));
                    }

                    case 't':
                        // case: timer can do zero or one non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(0, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
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
                            return cli.SetError(opt.GetError().c_str());
                        }

                        std::string temp_str(argv[2]);
                        epmem_time_id memory_id;

                        if (!from_string(memory_id, temp_str))
                        {
                            return cli.SetError("Invalid epmem time tag.");
                        }

                        return cli.DoEpMem(option, 0, 0, memory_id);
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

        private:
            cli::CommandLineInterface& cli;

            EpMemCommand& operator=(const EpMemCommand&);
    };

    class GPCommand : public cli::ParserCommand
    {
        public:
            GPCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~GPCommand() {}
            virtual const char* GetString() const
            {
                return "gp";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: gp { production_body }";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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

        private:
            cli::CommandLineInterface& cli;

            GPCommand& operator=(const GPCommand&);
    };

    class HelpCommand : public cli::ParserCommand
    {
        public:
            HelpCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~HelpCommand() {}
            virtual const char* GetString() const
            {
                return "help";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: help [command]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                return cli.DoHelp(argv);
            }

        private:
            cli::CommandLineInterface& cli;

            HelpCommand& operator=(const HelpCommand&);
    };


    class LearnCommand : public cli::ParserCommand
    {
        public:
            LearnCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~LearnCommand() {}
            virtual const char* GetString() const
            {
                return "learn";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: learn [-abdeElonNpP]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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
                    {0, 0, OPTARG_NONE}
                };

                cli::LearnBitset options(0);

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError().c_str());
                    }
                    ;
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

        private:
            cli::CommandLineInterface& cli;

            LearnCommand& operator=(const LearnCommand&);
    };

    class LSCommand : public cli::ParserCommand
    {
        public:
            LSCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~LSCommand() {}
            virtual const char* GetString() const
            {
                return "ls";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: ls";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                // No arguments
                if (argv.size() != 1)
                {
                    return cli.SetError(GetSyntax());
                }
                return cli.DoLS();
            }

        private:
            cli::CommandLineInterface& cli;

            LSCommand& operator=(const LSCommand&);
    };

    class PopDCommand : public cli::ParserCommand
    {
        public:
            PopDCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~PopDCommand() {}
            virtual const char* GetString() const
            {
                return "popd";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: popd";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                // No arguments
                if (argv.size() != 1)
                {
                    return cli.SetError(GetSyntax());
                }
                return cli.DoPopD();
            }

        private:
            cli::CommandLineInterface& cli;

            PopDCommand& operator=(const PopDCommand&);
    };

    class PreferencesCommand : public cli::ParserCommand
    {
        public:
            PreferencesCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~PreferencesCommand() {}
            virtual const char* GetString() const
            {
                return "preferences";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: preferences [options] [identifier [attribute]]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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
                    {0, 0, OPTARG_NONE}
                };

                cli::ePreferencesDetail detail = cli::PREFERENCES_ONLY;
                bool object = false;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError().c_str());
                    }
                    ;
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

        private:
            cli::CommandLineInterface& cli;

            PreferencesCommand& operator=(const PreferencesCommand&);
    };

    class PrintCommand : public cli::ParserCommand
    {
        public:
            PrintCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~PrintCommand() {}
            virtual const char* GetString() const
            {
                return "print";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: print [options] [production_name]\nprint [options] identifier|timetag|pattern";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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
                    {'T', "template",       OPTARG_NONE},
                    {'u', "user",           OPTARG_NONE},
                    {'v', "varprint",       OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                int depth = -1;
                cli::PrintBitset options(0);

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError().c_str());
                    }
                    ;
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

        private:
            cli::CommandLineInterface& cli;

            PrintCommand& operator=(const PrintCommand&);
    };

    class PushDCommand : public cli::ParserCommand
    {
        public:
            PushDCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~PushDCommand() {}
            virtual const char* GetString() const
            {
                return "pushd";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: pushd directory";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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

        private:
            cli::CommandLineInterface& cli;

            PushDCommand& operator=(const PushDCommand&);
    };

    class PWDCommand : public cli::ParserCommand
    {
        public:
            PWDCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~PWDCommand() {}
            virtual const char* GetString() const
            {
                return "pwd";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: pwd";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                // No arguments to print working directory
                if (argv.size() != 1)
                {
                    return cli.SetError(GetSyntax());
                }
                return cli.DoPWD();
            }

        private:
            cli::CommandLineInterface& cli;

            PWDCommand& operator=(const PWDCommand&);
    };

    class RLCommand : public cli::ParserCommand
    {
        public:
            RLCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~RLCommand() {}
            virtual const char* GetString() const
            {
                return "rl";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: rl [options parameter|statstic]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'g', "get",    OPTARG_NONE},
                    {'s', "set",    OPTARG_NONE},
                    {'t', "trace",    OPTARG_NONE},
                    {'S', "stats",    OPTARG_NONE},
                    {0, 0, OPTARG_NONE} // null
                };

                char option = 0;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError().c_str());
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
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoRL(option, &(argv[2]));
                    }

                    case 's':
                        // case: set requires two non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(2, 2))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoRL(option, &(argv[2]), &(argv[3]));
                    }

                    case 't':
                        // case: trace can do 0-2 non-option arguments
                    {
                        if (!opt.CheckNumNonOptArgs(0, 2))
                        {
                            return cli.SetError(opt.GetError().c_str());
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
                            return cli.SetError(opt.GetError().c_str());
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

        private:
            cli::CommandLineInterface& cli;

            RLCommand& operator=(const RLCommand&);
    };

    class RunCommand : public cli::ParserCommand
    {
        public:
            RunCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~RunCommand() {}
            virtual const char* GetString() const
            {
                return "run";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: run  [-f|count]\nrun -[d|e|o|p][s][un][g] [f|count]\nrun -[d|e|o|p][un] count [-i e|p|d|o]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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
                    {0, 0, OPTARG_NONE}
                };

                cli::RunBitset options(0);
                cli::eRunInterleaveMode interleaveMode = cli::RUN_INTERLEAVE_DEFAULT;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError().c_str());
                    }
                    ;
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
                                return cli.SetError(opt.GetError().c_str());    // error set in parse function
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

            RunCommand& operator=(const RunCommand&);
    };

    class SMemCommand : public cli::ParserCommand
    {
        public:
            SMemCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~SMemCommand() {}
            virtual const char* GetString() const
            {
                return "smem";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: smem [options]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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
                    {0, 0, OPTARG_NONE} // null
                };

                char option = 0;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError().c_str());
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
                        // case: add requires one non-option argument
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoSMem(option, &(argv[2]));

                    case 'b':
                        // case: backup requires one non-option argument
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoSMem(option, &(argv[2]));

                    case 'g':
                    {
                        // case: get requires one non-option argument
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoSMem(option, &(argv[2]));
                    }

                    case 'h':
                    {
                        // case: history only accepts 1 non-option argument
                        if (!opt.CheckNumNonOptArgs(1, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoSMem(option, &(argv[2]), 0);
                    }
                    case 'c':
                    case 'i':
                    case 'e':
                    case 'd':
                        // case: init takes no arguments
                        if (!opt.CheckNumNonOptArgs(0, 0))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoSMem(option);

                    case 'x':
                    {
                        // case: export does 1-2 non-option arguments
                        if (!opt.CheckNumNonOptArgs(1, 2))
                        {
                            return cli.SetError(opt.GetError().c_str());
                        }

                        if (opt.GetNonOptionArguments() == 1)
                        {
                            return cli.DoSMem(option, &(argv[2]), 0);
                        }

                        return cli.DoSMem(option, &(argv[2]), &(argv[3]), 0);
                    }

                    case 'q':
                    {
                        // case: query requires one non-option argument, but can have a depth argument
                        if (!opt.CheckNumNonOptArgs(1, 2))
                        {
                            return cli.SetError(opt.GetError().c_str());
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
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoSMem(option);

                    case 'r':
                    {
                        // case: remove requires one non-option argument, but can have a "force" argument
                        if (!opt.CheckNumNonOptArgs(1, 2))
                        {
                            return cli.SetError(opt.GetError().c_str());
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
                            return cli.SetError(opt.GetError().c_str());
                        }

                        return cli.DoSMem(option, &(argv[2]), &(argv[3]));
                    }

                    case 'S':
                    {
                        // case: stat can do zero or one non-option arguments
                        if (!opt.CheckNumNonOptArgs(0, 1))
                        {
                            return cli.SetError(opt.GetError().c_str());
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
                            return cli.SetError(opt.GetError().c_str());
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

        private:
            cli::CommandLineInterface& cli;

            SMemCommand& operator=(const SMemCommand&);
    };

    class SPCommand : public cli::ParserCommand
    {
        public:
            SPCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~SPCommand() {}
            virtual const char* GetString() const
            {
                return "sp";
            }
            virtual const char* GetSyntax() const
            {
                return
                    "Syntax: sp {production_body}";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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

        private:
            cli::CommandLineInterface& cli;

            SPCommand& operator=(const SPCommand&);
    };

    class StatsCommand : public cli::ParserCommand
    {
        public:
            StatsCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~StatsCommand() {}
            virtual const char* GetString() const
            {
                return "stats";
            }
            virtual const char* GetSyntax() const
            {
                return
                    "Syntax: stats [options]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
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
                    {0, 0, OPTARG_NONE}
                };

                cli::StatsBitset options(0);
                int sort = 0;

                for (;;)
                {
                    if (!opt.ProcessOptions(argv, optionsData))
                    {
                        return cli.SetError(opt.GetError().c_str());
                    }
                    ;
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

        private:
            cli::CommandLineInterface& cli;

            StatsCommand& operator=(const StatsCommand&);
    };

    class SVSCommand : public cli::ParserCommand
    {
        public:
            SVSCommand(cli::CommandLineInterface& cli) : cli(cli), ParserCommand() {}
            virtual ~SVSCommand() {}
            virtual const char* GetString() const
            {
                return "svs";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: svs <elements to inspect>\n"
                       "        svs [--enable | -e | --on | --disable | -d | --off]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                return cli.DoSVS(argv);
            }

        private:
            cli::CommandLineInterface& cli;
    };

}

#endif // CLI_COMMANDS_H
