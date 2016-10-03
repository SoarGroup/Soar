#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include "cli_Parser.h"
#include "cli_Options.h"
#include "cli_Cli.h"

#include "misc.h"
#include "sml_Events.h"

namespace cli
{
    class AliasCommand : public cli::ParserCommand
    {
        public:
            AliasCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~AliasCommand() {}
            virtual const char* GetString() const
            {
                return "alias";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: alias [name [cmd [args]]]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                if (argv.size() == 1)
                {
                    return cli.DoAlias();    // list all
                }

                argv.erase(argv.begin());

                return cli.DoAlias(&argv);
            }

        private:
            cli::Cli& cli;

            AliasCommand& operator=(const AliasCommand&);
    };

    class CDCommand : public cli::ParserCommand
    {
        public:
            CDCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            CDCommand& operator=(const CDCommand&);
    };

    class CLogCommand : public cli::ParserCommand
    {
        public:
            CLogCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~CLogCommand() {}
            virtual const char* GetString() const
            {
                return "clog";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: clog -[Ae] filename\nclog -a string\nclog [-cdoq]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "add",        OPTARG_NONE},
                    {'A', "append",        OPTARG_NONE},
                    {'c', "close",        OPTARG_NONE},
                    {'d', "disable",    OPTARG_NONE},
                    {'e', "existing",    OPTARG_NONE},
                    {'d', "off",        OPTARG_NONE},
                    {'q', "query",        OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                Cli::eLogMode mode = Cli::LOG_NEW;

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
                            mode = Cli::LOG_ADD;
                            break;
                        case 'c':
                        case 'd':
                        case 'o':
                            mode = Cli::LOG_CLOSE;
                            break;
                        case 'e':
                        case 'A':
                            mode = Cli::LOG_NEWAPPEND;
                            break;
                        case 'q':
                            mode = Cli::LOG_QUERY;
                            break;
                    }
                }

                switch (mode)
                {
                    case Cli::LOG_ADD:
                    {
                        std::string toAdd;
                        // no less than one non-option argument
                        if (opt.GetNonOptionArguments() < 1)
                        {
                            return cli.SetError("Provide a string to add.");
                        }

                        // move to the first non-option arg
                        std::vector<std::string>::iterator iter = argv.begin();
                        for (int i = 0; i < (opt.GetArgument() - opt.GetNonOptionArguments()); ++i)
                        {
                            ++iter;
                        }

                        // combine all args
                        while (iter != argv.end())
                        {
                            toAdd += *iter;
                            toAdd += ' ';
                            ++iter;
                        }
                        return cli.DoCLog(mode, 0, &toAdd);
                    }

                    case Cli::LOG_NEW:
                        // no more than one argument, no filename == query
                        if (opt.GetNonOptionArguments() > 1)
                        {
                            return cli.SetError("Filename or nothing expected, enclose filename in quotes if there are spaces in the path.");
                        }

                        if (opt.GetNonOptionArguments() == 1)
                        {
                            return cli.DoCLog(mode, &argv[1]);
                        }
                        break; // no args case handled below

                    case Cli::LOG_NEWAPPEND:
                        // exactly one argument
                        if (opt.GetNonOptionArguments() > 1)
                        {
                            return cli.SetError("Filename expected, enclose filename in quotes if there are spaces in the path.");
                        }

                        if (opt.GetNonOptionArguments() < 1)
                        {
                            return cli.SetError("Please provide a filename.");
                        }
                        return cli.DoCLog(mode, &argv[1]);

                    default:
                    case Cli::LOG_CLOSE:
                    case Cli::LOG_QUERY:
                        // no arguments
                        if (opt.GetNonOptionArguments())
                        {
                            return cli.SetError("No arguments when querying log status.");
                        }
                        break; // no args case handled below
                }

                // the no args case
                return cli.DoCLog(mode);
            }

        private:
            cli::Cli& cli;

            CLogCommand& operator=(const CLogCommand&);
    };

    class TclCommand : public cli::ParserCommand
    {

        public:
            TclCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~TclCommand() {}
            virtual const char* GetString() const
            {
                return "tcl";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: tcl [ on | off ]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                if (argv.size() < 2)
                {
                    return cli.SetError(GetSyntax());
                }

                std::string concat_message(argv[1]);
                for (std::vector<int>::size_type i = 2; i < argv.size(); ++i)
                {
                    concat_message += ' ' ;
                    concat_message += argv[i] ;
                }
                return cli.DoTclCommand(concat_message);
            }

        private:
            cli::Cli& cli;

            TclCommand& operator=(const TclCommand&);
    };

    class CommandToFileCommand : public cli::ParserCommand
    {
        public:
            CommandToFileCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~CommandToFileCommand() {}
            virtual const char* GetString() const
            {
                return "command-to-file";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: command-to-file [-a] filename command [args]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                // Not going to use normal option parsing in this case because I do not want to disturb the other command on the line
                if (argv.size() < 3)
                {
                    return cli.SetError(GetSyntax());
                }

                // Index of command in argv:  command-to-file filename command ...
                // Unless append option is present, which is handled later.
                int startOfCommand = 2;
                Cli::eLogMode mode = Cli::LOG_NEW;
                std::string filename = argv[1];

                // Parse out option.
                for (int i = 1; i < 3; ++i)
                {
                    bool append = false;
                    bool unrecognized = false;
                    std::string arg = argv[i];
                    if (arg[0] == '-')
                    {
                        if (arg[1] == 'a')
                        {
                            append = true;
                        }
                        else if (arg[1] == '-')
                        {
                            if (arg[2] == 'a')
                            {
                                append = true;
                            }
                            else
                            {
                                unrecognized = true;
                            }
                        }
                        else
                        {
                            unrecognized = true;
                        }
                    }

                    if (unrecognized)
                    {
                        return cli.SetError("Unrecognized option: " + arg);
                    }

                    if (append)
                    {
                        mode = Cli::LOG_NEWAPPEND;

                        // Index of command in argv:  command-to-file -a filename command ...
                        if (argv.size() < 4)
                        {
                            return cli.SetError(GetSyntax());
                        }

                        startOfCommand = 3;

                        // Re-set filename if necessary
                        if (i == 1)
                        {
                            filename = argv[2];
                        }

                        break;
                    }
                }

                // Restructure argv
                std::vector<std::string> newArgv;
                for (std::vector<int>::size_type i = startOfCommand; i < argv.size(); ++i)
                {
                    newArgv.push_back(argv[i]);
                }

                return cli.DoCommandToFile(mode, filename, newArgv);
            }

        private:
            cli::Cli& cli;

            CommandToFileCommand& operator=(const CommandToFileCommand&);
    };

    class DebugCommand : public cli::ParserCommand
    {
        public:
            DebugCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~DebugCommand() {}
            virtual const char* GetString() const
            {
                return "debug";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: debug [internal-symbols | port | ? ] [arguments*]";
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
            cli::Cli& cli;
            DebugCommand& operator=(const DebugCommand&);
    };


    class DirsCommand : public cli::ParserCommand
    {
        public:
            DirsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            DirsCommand& operator=(const DirsCommand&);
    };

    class EchoCommand : public cli::ParserCommand
    {
        public:
            EchoCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            EchoCommand& operator=(const EchoCommand&);
    };

    class EpMemCommand : public cli::ParserCommand
    {
        public:
            EpMemCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
                    return cli.SetError("Too many arguments, check syntax.");
                }

                // case: nothing = full configuration information
                return cli.DoEpMem();
            }

        private:
            cli::Cli& cli;

            EpMemCommand& operator=(const EpMemCommand&);
    };

    class GDSPrintCommand : public cli::ParserCommand
    {
        public:
            GDSPrintCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~GDSPrintCommand() {}
            virtual const char* GetString() const
            {
                return "gds-print";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: gds-print";
            }

            virtual bool Parse(std::vector< std::string >&)
            {
                return cli.DoGDSPrint();
            }

        private:
            cli::Cli& cli;

            GDSPrintCommand& operator=(const GDSPrintCommand&);
    };

    class GPCommand : public cli::ParserCommand
    {
        public:
            GPCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            GPCommand& operator=(const GPCommand&);
    };

    class HelpCommand : public cli::ParserCommand
    {
        public:
            HelpCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            HelpCommand& operator=(const HelpCommand&);
    };


    class LearnCommand : public cli::ParserCommand
    {
        public:
            LearnCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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

                Cli::LearnBitset options(0);

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
                            options.set(Cli::LEARN_ALL_LEVELS);
                            break;
                        case 'b':
                            options.set(Cli::LEARN_BOTTOM_UP);
                            break;
                        case 'd':
                            options.set(Cli::LEARN_DISABLE);
                            break;
                        case 'e':
                            options.set(Cli::LEARN_ENABLE);
                            break;
                        case 'E':
                            options.set(Cli::LEARN_EXCEPT);
                            break;
                        case 'l':
                            options.set(Cli::LEARN_LIST);
                            break;
                        case 'o':
                            options.set(Cli::LEARN_ONLY);
                            break;
                        case 'n':
                            options.set(Cli::LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS);
                            break;
                        case 'N':
                            options.set(Cli::LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS);
                            break;
                        case 'p':
                            options.set(Cli::LEARN_ENABLE_THROUGH_EVALUATION_RULES);
                            break;
                        case 'P':
                            options.set(Cli::LEARN_DISABLE_THROUGH_EVALUATION_RULES);
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
            cli::Cli& cli;

            LearnCommand& operator=(const LearnCommand&);
    };

    class LSCommand : public cli::ParserCommand
    {
        public:
            LSCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            LSCommand& operator=(const LSCommand&);
    };

    class PopDCommand : public cli::ParserCommand
    {
        public:
            PopDCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            PopDCommand& operator=(const PopDCommand&);
    };

    class PreferencesCommand : public cli::ParserCommand
    {
        public:
            PreferencesCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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

                Cli::ePreferencesDetail detail = Cli::PREFERENCES_ONLY;
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
                            detail = Cli::PREFERENCES_ONLY;
                            break;
                        case '1':
                        case 'n':
                        case 'N':
                            detail = Cli::PREFERENCES_NAMES;
                            break;
                        case '2':
                        case 't':
                            detail = Cli::PREFERENCES_TIMETAGS;
                            break;
                        case '3':
                        case 'w':
                            detail = Cli::PREFERENCES_WMES;
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
            cli::Cli& cli;

            PreferencesCommand& operator=(const PreferencesCommand&);
    };

    class PrintCommand : public cli::ParserCommand
    {
        public:
            PrintCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
                    {'c', "chunks",            OPTARG_NONE},
                    {'d', "depth",            OPTARG_REQUIRED},
                    {'D', "defaults",        OPTARG_NONE},
                    {'e', "exact",            OPTARG_NONE},
                    {'f', "full",            OPTARG_NONE},
                    {'F', "filename",        OPTARG_NONE},
                    {'i', "internal",        OPTARG_NONE},
                    {'j', "justifications",    OPTARG_NONE},
                    {'n', "name",            OPTARG_NONE},
                    {'o', "operators",        OPTARG_NONE},
                    {'r', "rl",                OPTARG_NONE},
                    {'s', "stack",            OPTARG_NONE},
                    {'S', "states",            OPTARG_NONE},
                    {'t', "tree",            OPTARG_NONE},
                    {'T', "template",        OPTARG_NONE},
                    {'u', "user",            OPTARG_NONE},
                    {'v', "varprint",        OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                int depth = -1;
                Cli::PrintBitset options(0);

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
                            options.set(Cli::PRINT_ALL);
                            break;
                        case 'c':
                            options.set(Cli::PRINT_CHUNKS);
                            break;
                        case 'd':
                            options.set(Cli::PRINT_DEPTH);
                            if (!from_string(depth, opt.GetOptionArgument()) || (depth < 0))
                            {
                                return cli.SetError("Non-negative depth expected.");
                            }
                            break;
                        case 'D':
                            options.set(Cli::PRINT_DEFAULTS);
                            break;
                        case 'e':
                            options.set(Cli::PRINT_EXACT);
                            break;
                        case 'f':
                            options.set(Cli::PRINT_FULL);
                            break;
                        case 'F':
                            options.set(Cli::PRINT_FILENAME);
                            break;
                        case 'i':
                            options.set(Cli::PRINT_INTERNAL);
                            break;
                        case 'j':
                            options.set(Cli::PRINT_JUSTIFICATIONS);
                            break;
                        case 'n':
                            options.set(Cli::PRINT_NAME);
                            break;
                        case 'o':
                            options.set(Cli::PRINT_OPERATORS);
                            break;
                        case 'r':
                            options.set(Cli::PRINT_RL);
                            break;
                        case 's':
                            options.set(Cli::PRINT_STACK);
                            break;
                        case 'S':
                            options.set(Cli::PRINT_STATES);
                            break;
                        case 't':
                            options.set(Cli::PRINT_TREE);
                            break;
                        case 'T':
                            options.set(Cli::PRINT_TEMPLATE);
                            break;
                        case 'u':
                            options.set(Cli::PRINT_USER);
                            break;
                        case 'v':
                            options.set(Cli::PRINT_VARPRINT);
                            break;
                    }
                }

                // STATES and OPERATORS are sub-options of STACK
                if (options.test(Cli::PRINT_OPERATORS) || options.test(Cli::PRINT_STATES))
                {
                    if (!options.test(Cli::PRINT_STACK))
                    {
                        return cli.SetError("Options --operators (-o) and --states (-S) are only valid when printing the stack.");
                    }
                }

                if (opt.GetNonOptionArguments() == 0)
                {
                    // d and t options require an argument
                    if (options.test(Cli::PRINT_TREE) || options.test(Cli::PRINT_DEPTH))
                    {
                        return cli.SetError(GetSyntax());
                    }
                    return cli.DoPrint(options, depth);
                }

                // the acDjus options don't allow an argument
                if (options.test(Cli::PRINT_ALL)
                        || options.test(Cli::PRINT_CHUNKS)
                        || options.test(Cli::PRINT_DEFAULTS)
                        || options.test(Cli::PRINT_JUSTIFICATIONS)
                        || options.test(Cli::PRINT_RL)
                        || options.test(Cli::PRINT_TEMPLATE)
                        || options.test(Cli::PRINT_USER)
                        || options.test(Cli::PRINT_STACK))
                {
                    return cli.SetError("No argument allowed when printing all/chunks/defaults/justifications/rl/template/user/stack.");
                }
                if (options.test(Cli::PRINT_EXACT) && (options.test(Cli::PRINT_DEPTH) || options.test(Cli::PRINT_TREE)))
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
            cli::Cli& cli;

            PrintCommand& operator=(const PrintCommand&);
    };

    class PushDCommand : public cli::ParserCommand
    {
        public:
            PushDCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            PushDCommand& operator=(const PushDCommand&);
    };

    class PWDCommand : public cli::ParserCommand
    {
        public:
            PWDCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            PWDCommand& operator=(const PWDCommand&);
    };

    class RandCommand : public cli::ParserCommand
    {
        public:
            RandCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~RandCommand() {}
            virtual const char* GetString() const
            {
                return "rand";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: rand\nrand n\nrand --integer\nrand --integer n";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'i', "integer", OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                bool integer(false);

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
                        case 'i':
                            integer = true;
                            break;
                    }
                }

                if (opt.GetNonOptionArguments() > 1)
                {
                    return cli.SetError(GetSyntax());
                }
                else if (opt.GetNonOptionArguments() == 1)
                {
                    unsigned optind = opt.GetArgument() - opt.GetNonOptionArguments();
                    return cli.DoRand(integer, &(argv[optind]));
                }

                return cli.DoRand(integer, 0);
            }

        private:
            cli::Cli& cli;

            RandCommand& operator=(const RandCommand&);
    };

    class RLCommand : public cli::ParserCommand
    {
        public:
            RLCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            RLCommand& operator=(const RLCommand&);
    };

    class RunCommand : public cli::ParserCommand
    {
        public:
            RunCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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

                Cli::RunBitset options(0);
                Cli::eRunInterleaveMode interleaveMode = Cli::RUN_INTERLEAVE_DEFAULT;

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
                            options.set(Cli::RUN_DECISION);
                            break;
                        case 'e':
                            options.set(Cli::RUN_ELABORATION);
                            break;
                        case 'g':
                            options.set(Cli::RUN_GOAL);
                            break;
                        case 'i':
                            options.set(Cli::RUN_INTERLEAVE);
                            interleaveMode = ParseRunInterleaveOptarg(opt);
                            if (interleaveMode == Cli::RUN_INTERLEAVE_DEFAULT)
                            {
                                return cli.SetError(opt.GetError().c_str());    // error set in parse function
                            }
                            break;
                        case 'o':
                            options.set(Cli::RUN_OUTPUT);
                            break;
                        case 'p':
                            options.set(Cli::RUN_PHASE);
                            break;
                        case 's':
                            options.set(Cli::RUN_SELF);
                            break;
                        case 'u':
                            options.set(Cli::RUN_UPDATE) ;
                            break ;
                        case 'n':
                            options.set(Cli::RUN_NO_UPDATE) ;
                            break ;
                    }
                }

                // Only one non-option argument allowed, count
                if (opt.GetNonOptionArguments() > 1)
                {
                    return cli.SetError(GetSyntax());
                }

                // Decide if we explicitly indicated how to run
                bool specifiedType = (options.test(Cli::RUN_ELABORATION) || options.test(Cli::RUN_DECISION) || options.test(Cli::RUN_PHASE) || options.test(Cli::RUN_OUTPUT)) ;

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
                    if (count < 0 || (count == 0 && specifiedType && !options.test(Cli::RUN_DECISION)))
                    {
                        return cli.SetError("Count must be positive.");
                    }
                }

                return cli.DoRun(options, count, interleaveMode);
            }

        private:
            cli::Cli& cli;

            Cli::eRunInterleaveMode ParseRunInterleaveOptarg(cli::Options& opt)
            {
                if (opt.GetOptionArgument() == "d")
                {
                    return Cli::RUN_INTERLEAVE_DECISION;
                }
                else if (opt.GetOptionArgument() == "e")
                {
                    return Cli::RUN_INTERLEAVE_ELABORATION;
                }
                else if (opt.GetOptionArgument() == "o")
                {
                    return Cli::RUN_INTERLEAVE_OUTPUT;
                }
                else if (opt.GetOptionArgument() == "p")
                {
                    return Cli::RUN_INTERLEAVE_PHASE;
                }

                cli.SetError("Invalid interleave switch: " + opt.GetOptionArgument());
                return Cli::RUN_INTERLEAVE_DEFAULT;
            }

            RunCommand& operator=(const RunCommand&);
    };

    class SMemCommand : public cli::ParserCommand
    {
        public:
            SMemCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
                    {'d', "disable",    OPTARG_NONE},
                    {'d', "off",        OPTARG_NONE},
                    {'e', "enable",     OPTARG_NONE},
                    {'e', "on",         OPTARG_NONE},
                    {'g', "get",        OPTARG_NONE},
                    {'h', "history",    OPTARG_NONE},//Testing/unstable - 23-7-2014
                    {'i', "init",       OPTARG_NONE},
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
                    return cli.SetError("Too many arguments.");
                }

                // case: nothing = full configuration information
                return cli.DoSMem();
            }

        private:
            cli::Cli& cli;

            SMemCommand& operator=(const SMemCommand&);
    };

    class SPCommand : public cli::ParserCommand
    {
        public:
            SPCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;

            SPCommand& operator=(const SPCommand&);
    };

    class StatsCommand : public cli::ParserCommand
    {
        public:
            StatsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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

                Cli::StatsBitset options(0);
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
                            options.set(Cli::STATS_DECISION);
                            break;
                        case 'm':
                            options.set(Cli::STATS_MEMORY);
                            break;
                        case 'M':
                            options.set(Cli::STATS_MAX);
                            break;
                        case 'r':
                            options.set(Cli::STATS_RETE);
                            break;
                        case 'R':
                            options.set(Cli::STATS_RESET);
                            break;
                        case 's':
                            options.set(Cli::STATS_SYSTEM);
                            break;
                        case 't':
                            options.set(Cli::STATS_TRACK);
                            break;
                        case 'T':
                            options.set(Cli::STATS_STOP_TRACK);
                            break;
                        case 'c':
                            options.set(Cli::STATS_CYCLE);
                            break;
                        case 'C':
                            options.set(Cli::STATS_CSV);
                            break;
                        case 'S':
                            options.set(Cli::STATS_CYCLE);
                            if (!from_string(sort, opt.GetOptionArgument()))
                            {
                                return cli.SetError("Integer expected");
                            }
                            break;
                        case 'a':
                            options.set(Cli::STATS_AGENT);
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
            cli::Cli& cli;

            StatsCommand& operator=(const StatsCommand&);
    };

    class StopSoarCommand : public cli::ParserCommand
    {
        public:
            StopSoarCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~StopSoarCommand() {}
            virtual const char* GetString() const
            {
                return "stop-soar";
            }
            virtual const char* GetSyntax() const
            {
                return
                    "Syntax: stop-soar [-s] [reason string]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'s', "self",        OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                bool self = false;

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
                        case 's':
                            self = true;
                            break;
                    }
                }

                // Concatinate remaining args for 'reason'
                if (opt.GetNonOptionArguments())
                {
                    std::string reasonForStopping;
                    unsigned int optind = opt.GetArgument() - opt.GetNonOptionArguments();
                    while (optind < argv.size())
                    {
                        reasonForStopping += argv[optind++] + ' ';
                    }
                    return cli.DoStopSoar(self, &reasonForStopping);
                }
                return cli.DoStopSoar(self);
            }

        private:
            cli::Cli& cli;

            StopSoarCommand& operator=(const StopSoarCommand&);
    };

    class TimeCommand : public cli::ParserCommand
    {
        public:
            TimeCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~TimeCommand() {}
            virtual const char* GetString() const
            {
                return "time";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: time command [arguments]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                // There must at least be a command
                if (argv.size() < 2)
                {
                    return cli.SetError(GetSyntax());
                }

                std::vector<std::string>::iterator iter = argv.begin();
                argv.erase(iter);

                return cli.DoTime(argv);
            }

        private:
            cli::Cli& cli;

            TimeCommand& operator=(const TimeCommand&);
    };

    class TimersCommand : public cli::ParserCommand
    {
        public:
            TimersCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~TimersCommand() {}
            virtual const char* GetString() const
            {
                return "timers";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: timers [options]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'e', "enable",        OPTARG_NONE},
                    {'d', "disable",    OPTARG_NONE},
                    {'d', "off",        OPTARG_NONE},
                    {'e', "on",            OPTARG_NONE},
                    {0, 0, OPTARG_NONE}
                };

                bool print = true;
                bool setting = false;    // enable or disable timers, default of false ignored

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
                        case 'e':
                            print = false;
                            setting = true; // enable timers
                            break;
                        case 'd':
                            print = false;
                            setting = false; // disable timers
                            break;
                    }
                }

                // No non-option arguments
                if (opt.GetNonOptionArguments())
                {
                    return cli.SetError(GetSyntax());
                }

                return cli.DoTimers(print ? 0 : &setting);
            }

        private:
            cli::Cli& cli;

            TimersCommand& operator=(const TimersCommand&);
    };

    class UnaliasCommand : public cli::ParserCommand
    {
        public:
            UnaliasCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~UnaliasCommand() {}
            virtual const char* GetString() const
            {
                return "unalias";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: unalias name";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                // Need exactly one argument
                if (argv.size() < 2)
                {
                    return cli.SetError(GetSyntax());
                }
                if (argv.size() > 2)
                {
                    return cli.SetError(GetSyntax());
                }

                argv.erase(argv.begin());
                return cli.DoUnalias(argv);
            }

        private:
            cli::Cli& cli;

            UnaliasCommand& operator=(const UnaliasCommand&);
    };

    class VersionCommand : public cli::ParserCommand
    {
        public:
            VersionCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~VersionCommand() {}
            virtual const char* GetString() const
            {
                return "version";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: version";
            }

            virtual bool Parse(std::vector< std::string >&)
            {
                return cli.DoVersion();
            }

        private:
            cli::Cli& cli;

            VersionCommand& operator=(const VersionCommand&);
    };

    class WatchCommand : public cli::ParserCommand
    {
        public:
            WatchCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
            virtual ~WatchCommand() {}
            virtual const char* GetString() const
            {
                return "watch";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: watch [options]\nwatch [level]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                OptionsData optionsData[] =
                {
                    {'a', "wma",                         OPTARG_OPTIONAL},
                    {'b', "backtracing",                 OPTARG_OPTIONAL},
                    {'c', "chunks",                      OPTARG_OPTIONAL},
                    {'d', "decisions",                   OPTARG_OPTIONAL},
                    {'D', "default-productions",         OPTARG_OPTIONAL},
                    {'e', "epmem",                       OPTARG_OPTIONAL},
                    {'f', "fullwmes",                    OPTARG_NONE},
                    {'g', "gds",                         OPTARG_OPTIONAL},
                    {'i', "indifferent-selection",       OPTARG_OPTIONAL},
                    {'j', "justifications",              OPTARG_OPTIONAL},
                    {'L', "learning",                    OPTARG_REQUIRED},
                    {'l', "level",                       OPTARG_REQUIRED},
                    {'N', "none",                        OPTARG_NONE},
                    {'n', "nowmes",                      OPTARG_NONE},
                    {'p', "phases",                      OPTARG_OPTIONAL},
                    {'P', "productions",                 OPTARG_OPTIONAL},
                    {'r', "preferences",                 OPTARG_OPTIONAL},
                    {'R', "rl",                          OPTARG_OPTIONAL},
                    {'s', "smem",                        OPTARG_OPTIONAL},
                    {'t', "timetags",                    OPTARG_NONE},
                    {'T', "template",                    OPTARG_OPTIONAL},
                    {'u', "user-productions",            OPTARG_OPTIONAL},
                    {'w', "wmes",                        OPTARG_OPTIONAL},
                    {'W', "waterfall",                   OPTARG_OPTIONAL}, // TODO: document. note: added to watch 5
                    {0, 0, OPTARG_NONE}
                };

                Cli::WatchBitset options(0);
                Cli::WatchBitset settings(0);
                int learnSetting = 0;
                int wmeSetting = 0;

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
                            options.set(Cli::WATCH_WMA);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_WMA);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_WMA);
                            }
                            break;

                        case 'b':
                            options.set(Cli::WATCH_BACKTRACING);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_BACKTRACING);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_BACKTRACING);
                            }
                            break;

                        case 'c':
                            options.set(Cli::WATCH_CHUNKS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_CHUNKS);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_CHUNKS);
                            }
                            break;

                        case 'd':
                            options.set(Cli::WATCH_DECISIONS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_DECISIONS);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_DECISIONS);
                            }
                            break;

                        case 'D':
                            options.set(Cli::WATCH_DEFAULT);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_DEFAULT);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_DEFAULT);
                            }
                            break;

                        case 'e':
                            options.set(Cli::WATCH_EPMEM);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_EPMEM);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_EPMEM);
                            }
                            break;

                        case 'f': // fullwmes
                            options.set(Cli::WATCH_WME_DETAIL);
                            wmeSetting = 2;
                            break;

                        case 'g':
                            options.set(Cli::WATCH_GDS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_GDS);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_GDS);
                            }
                            break;

                        case 'i':
                            options.set(Cli::WATCH_INDIFFERENT);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_INDIFFERENT);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_INDIFFERENT);
                            }
                            break;

                        case 'j':
                            options.set(Cli::WATCH_JUSTIFICATIONS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_JUSTIFICATIONS);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_JUSTIFICATIONS);
                            }
                            break;

                        case 'L':
                            options.set(Cli::WATCH_LEARNING);
                            learnSetting = ParseLearningOptarg(opt);
                            if (learnSetting == -1)
                            {
                                return cli.SetError(opt.GetError().c_str());
                            }
                            break;

                        case 'l':
                        {
                            int level = 0;
                            if (!from_string(level, opt.GetOptionArgument()))
                            {
                                return cli.SetError("Integer argument expected.");
                            }

                            if (!ProcessWatchLevelSettings(level, options, settings, wmeSetting, learnSetting))
                            {
                                return cli.SetError(opt.GetError().c_str());
                            }
                        }
                        break;

                        case 'N': // none
                            options.reset();
                            options.flip();
                            settings.reset();
                            learnSetting = 0;
                            wmeSetting = 0;
                            break;

                        case 'n': // nowmes
                            options.set(Cli::WATCH_WME_DETAIL);
                            wmeSetting = 0;
                            break;

                        case 'p':
                            options.set(Cli::WATCH_PHASES);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_PHASES);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_PHASES);
                            }
                            break;

                        case 'P': // productions (all)
                            options.set(Cli::WATCH_DEFAULT);
                            options.set(Cli::WATCH_USER);
                            options.set(Cli::WATCH_CHUNKS);
                            options.set(Cli::WATCH_JUSTIFICATIONS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_DEFAULT);
                                settings.reset(Cli::WATCH_USER);
                                settings.reset(Cli::WATCH_CHUNKS);
                                settings.reset(Cli::WATCH_JUSTIFICATIONS);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_DEFAULT);
                                settings.set(Cli::WATCH_USER);
                                settings.set(Cli::WATCH_CHUNKS);
                                settings.set(Cli::WATCH_JUSTIFICATIONS);
                            }
                            break;

                        case 'r':
                            options.set(Cli::WATCH_PREFERENCES);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_PREFERENCES);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_PREFERENCES);
                            }
                            break;

                        case 'R':
                            options.set(Cli::WATCH_RL);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_RL);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_RL);
                            }
                            break;

                        case 's':
                            options.set(Cli::WATCH_SMEM);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_SMEM);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_SMEM);
                            }
                            break;

                        case 't'://timetags
                            options.set(Cli::WATCH_WME_DETAIL);
                            wmeSetting = 1;
                            break;

                        case 'T':
                            options.set(Cli::WATCH_TEMPLATES);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_TEMPLATES);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_TEMPLATES);
                            }
                            break;

                        case 'u':
                            options.set(Cli::WATCH_USER);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_USER);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_USER);
                            }
                            break;
                        case 'w'://wmes
                            options.set(Cli::WATCH_WMES);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_WMES);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_WMES);
                            }
                            break;
                        case 'W'://waterfall
                            options.set(Cli::WATCH_WATERFALL);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt))
                                {
                                    return cli.SetError(opt.GetError().c_str());
                                }
                                settings.reset(Cli::WATCH_WATERFALL);
                            }
                            else
                            {
                                settings.set(Cli::WATCH_WATERFALL);
                            }
                            break;
                    }
                }

                if (opt.GetNonOptionArguments() > 1)
                {
                    return cli.SetError("Only non option argument allowed is watch level.");
                }

                // Allow watch level by itself
                if (opt.GetNonOptionArguments() == 1)
                {
                    int optind = opt.GetArgument() - opt.GetNonOptionArguments();
                    int level = 0;
                    if (!from_string(level, argv[optind]))
                    {
                        return cli.SetError("Integer argument expected.");
                    }
                    if (!ProcessWatchLevelSettings(level, options, settings, wmeSetting, learnSetting))
                    {
                        return cli.SetError(opt.GetError().c_str());
                    }
                }

                return cli.DoWatch(options, settings, wmeSetting, learnSetting);
            }

        private:
            cli::Cli& cli;

            bool ProcessWatchLevelSettings(const int level, Cli::WatchBitset& options, Cli::WatchBitset& settings, int& wmeSetting, int& learnSetting)
            {
                if (level < 0)
                {
                    return cli.SetError("Expected watch level from 0 to 5.");
                }

                if (level > 5)
                {
                    return cli.SetError("Expected watch level from 0 to 5.");
                }

                // All of these are going to change
                options.set(Cli::WATCH_PREFERENCES);
                options.set(Cli::WATCH_WMES);
                options.set(Cli::WATCH_DEFAULT);
                options.set(Cli::WATCH_USER);
                options.set(Cli::WATCH_CHUNKS);
                options.set(Cli::WATCH_JUSTIFICATIONS);
                options.set(Cli::WATCH_TEMPLATES);
                options.set(Cli::WATCH_PHASES);
                options.set(Cli::WATCH_DECISIONS);
                options.set(Cli::WATCH_WATERFALL);
                options.set(Cli::WATCH_GDS);

                // Start with all off, turn on as appropriate
                settings.reset(Cli::WATCH_PREFERENCES);
                settings.reset(Cli::WATCH_WMES);
                settings.reset(Cli::WATCH_DEFAULT);
                settings.reset(Cli::WATCH_USER);
                settings.reset(Cli::WATCH_CHUNKS);
                settings.reset(Cli::WATCH_JUSTIFICATIONS);
                settings.reset(Cli::WATCH_TEMPLATES);
                settings.reset(Cli::WATCH_PHASES);
                settings.reset(Cli::WATCH_DECISIONS);
                settings.reset(Cli::WATCH_WATERFALL);
                settings.reset(Cli::WATCH_GDS);

                switch (level)
                {
                    case 0:// none
                        options.reset();
                        options.flip();
                        settings.reset();
                        learnSetting = 0;
                        wmeSetting = 0;
                        break;

                    case 5:// preferences, waterfall
                        settings.set(Cli::WATCH_PREFERENCES);
                        settings.set(Cli::WATCH_WATERFALL);
                    // falls through
                    case 4:// wmes
                        settings.set(Cli::WATCH_WMES);
                    // falls through
                    case 3:// productions (default, user, chunks, justifications, templates)
                        settings.set(Cli::WATCH_DEFAULT);
                        settings.set(Cli::WATCH_USER);
                        settings.set(Cli::WATCH_CHUNKS);
                        settings.set(Cli::WATCH_JUSTIFICATIONS);
                        settings.set(Cli::WATCH_TEMPLATES);
                    // falls through
                    case 2:// phases, gds
                        settings.set(Cli::WATCH_PHASES);
                        settings.set(Cli::WATCH_GDS);
                    // falls through
                    case 1:// decisions
                        settings.set(Cli::WATCH_DECISIONS);
                        break;
                }
                return true;
            }

            int ParseLearningOptarg(cli::Options& opt)
            {
                if (opt.GetOptionArgument() == "noprint"   || opt.GetOptionArgument() == "0")
                {
                    return 0;
                }
                if (opt.GetOptionArgument() == "print"     || opt.GetOptionArgument() == "1")
                {
                    return 1;
                }
                if (opt.GetOptionArgument() == "fullprint" || opt.GetOptionArgument() == "2")
                {
                    return 2;
                }

                cli.SetError("Invalid learn setting, expected noprint, print, fullprint, or 0-2. Got: " + opt.GetOptionArgument());
                return -1;
            }

            bool CheckOptargRemoveOrZero(cli::Options& opt)
            {
                if (opt.GetOptionArgument() == "remove" || opt.GetOptionArgument() == "0")
                {
                    return true;
                }

                return cli.SetError("Invalid argument, expected remove or 0. Got: " + opt.GetOptionArgument());
            }

            WatchCommand& operator=(const WatchCommand&);
    };

    class SVSCommand : public cli::ParserCommand
    {
        public:
            SVSCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
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
            cli::Cli& cli;
    };
}

#endif // CLI_COMMANDS_H
