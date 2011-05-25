#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include "cli_Parser.h"
#include "misc.h"
#include "cli_Options.h"
#include "kernel.h"
#include "sml_Events.h"
#include "cli_Cli.h"

namespace cli 
{
    class AddWMECommand : public cli::ParserCommand
    {
    public:
        AddWMECommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~AddWMECommand() {}
        virtual const char* GetString() const { return "add-wme"; }

        virtual const char* GetSyntax() const 
        { 
            return "Syntax: add-wme id [^]attribute value [+]"; 
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            if (argv.size() < 4) 
                return cli.SetError(GetSyntax());

            unsigned attributeIndex = (argv[2] == "^") ? 3 : 2;

            if (argv.size() < (attributeIndex + 2)) return cli.SetError(GetSyntax());
            if (argv.size() > (attributeIndex + 3)) return cli.SetError(GetSyntax());

            bool acceptable = false;
            if (argv.size() > (attributeIndex + 2)) {
                if (argv[attributeIndex + 2] != "+") 
                    return cli.SetError(GetSyntax());
                acceptable = true;
            }

            return cli.DoAddWME(argv[1], argv[attributeIndex], argv[attributeIndex + 1], acceptable);
        }

    private:
        cli::Cli& cli;

        AddWMECommand& operator=(const AddWMECommand&);
    };

    class AliasCommand : public cli::ParserCommand
    {
    public:
        AliasCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~AliasCommand() {}
        virtual const char* GetString() const { return "alias"; }
        virtual const char* GetSyntax() const
        {
            return "Syntax: alias [name [cmd [args]]]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            if (argv.size() == 1) 
                return cli.DoAlias(); // list all

            argv.erase(argv.begin());

            return cli.DoAlias(&argv);
        }

    private:
        cli::Cli& cli;

        AliasCommand& operator=(const AliasCommand&);
    };

    class AllocateCommand : public cli::ParserCommand
    {
    public:
        AllocateCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~AllocateCommand() {}
        virtual const char* GetString() const { return "allocate"; }
        virtual const char* GetSyntax() const
        { 
            return "Syntax: allocate [pool blocks]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            if (argv.size() == 1)
                return cli.DoAllocate(std::string(), 0);

            if (argv.size() != 3) 
                return cli.SetError(GetSyntax());

            int blocks = 0;
            if (!from_string(blocks, argv[2]))
                return cli.SetError("Expected an integer (number of blocks).");

            if (blocks < 1)
                return cli.SetError("Expected a positive integer (number of blocks).");

            return cli.DoAllocate(argv[1], blocks);
        }

    private:
        cli::Cli& cli;

        AllocateCommand& operator=(const AllocateCommand&);
    };

    class CaptureInputCommand : public cli::ParserCommand
    {
    public:
        CaptureInputCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~CaptureInputCommand() {}
        virtual const char* GetString() const { return "capture-input"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: capture-input --open filename [--flush]\n"
                "capture-input [--query]\n"
                "capture-input --close";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'c', "close", OPTARG_NONE},
                {'f', "flush", OPTARG_NONE},
                {'o', "open", OPTARG_REQUIRED},
                {'q', "query", OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            Cli::eCaptureInputMode mode = Cli::CAPTURE_INPUT_QUERY;
            std::string pathname;

            bool autoflush = false;
            for (;;)
            {
                if (!opt.ProcessOptions(argv, optionsData))
                    return cli.SetError(opt.GetError());

                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'c':
                        mode = Cli::CAPTURE_INPUT_CLOSE;
                        break;
                    case 'f':
                        autoflush = true;
                        break;
                    case 'o':
                        mode = Cli::CAPTURE_INPUT_OPEN;
                        pathname = opt.GetOptionArgument();
                        break;
                    case 'q':
                        mode = Cli::CAPTURE_INPUT_QUERY;
                        break;
                }
            }

            return cli.DoCaptureInput(mode, autoflush, mode == Cli::CAPTURE_INPUT_OPEN ? &pathname : 0);
        }

    private:
        cli::Cli& cli;

        CaptureInputCommand& operator=(const CaptureInputCommand&);
    };

    class CDCommand : public cli::ParserCommand
    {
    public:
        CDCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~CDCommand() {}
        virtual const char* GetString() const { return "cd"; }
        virtual const char* GetSyntax() const 
        { 
            return "Syntax: cd [directory]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // Only takes one optional argument, the directory to change into
            if (argv.size() > 2) 
                return cli.SetError("Only one argument (a directory) is allowed. Paths with spaces should be enclosed in quotes.");

            if (argv.size() > 1) {
                return cli.DoCD(&(argv[1]));
            }
            return cli.DoCD();
        }

    private:
        cli::Cli& cli;

        CDCommand& operator=(const CDCommand&);
    };

    class ChunkNameFormatCommand : public cli::ParserCommand
    {
    public:
        ChunkNameFormatCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~ChunkNameFormatCommand() {}
        virtual const char* GetString() const { return "chunk-name-format"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: chunk-name-format [-sl] -p [prefix]\n"
                "chunk-name-format [-sl] -c [count]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'c', "count",        OPTARG_OPTIONAL},
                {'l', "long",        OPTARG_NONE},
                {'p', "prefix",        OPTARG_OPTIONAL},
                {'s', "short",        OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            bool changeFormat = false;
            bool countFlag = false;
            int64_t count = -1;
            bool patternFlag = false;
            std::string pattern;
            bool longFormat = true;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData))
                    return cli.SetError(opt.GetError());

                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'c': 
                        countFlag = true;
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!from_string(count, opt.GetOptionArgument()) || count < 0)
                                return cli.SetError("Expected non-negative integer.");
                        }
                        break;
                    case 'p': 
                        patternFlag = true;
                        if (opt.GetOptionArgument().size()) {
                            pattern = std::string(opt.GetOptionArgument());
                        }
                        break;
                    case 'l':
                        changeFormat = true;
                        longFormat = true;
                        break;
                    case 's':
                        changeFormat = true;
                        longFormat = false;
                        break;
                }
            }

            if (opt.GetNonOptionArguments()) return cli.SetError(GetSyntax());

            return cli.DoChunkNameFormat(changeFormat ? &longFormat : 0, countFlag ? &count : 0, patternFlag ? &pattern : 0);
        }

    private:
        cli::Cli& cli;

        ChunkNameFormatCommand& operator=(const ChunkNameFormatCommand&);
    };

    class CLogCommand : public cli::ParserCommand
    {
    public:
        CLogCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~CLogCommand() {}
        virtual const char* GetString() const { return "clog"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: clog -[Ae] filename\nclog -a string\nclog [-cdoq]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
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
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
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

            switch (mode) {
                case Cli::LOG_ADD:
                    {
                        std::string toAdd;
                        // no less than one non-option argument
                        if (opt.GetNonOptionArguments() < 1) 
                            return cli.SetError("Provide a string to add.");

                        // move to the first non-option arg
                        std::vector<std::string>::iterator iter = argv.begin();
                        for (int i = 0; i < (opt.GetArgument() - opt.GetNonOptionArguments()); ++i) ++iter;

                        // combine all args
                        while (iter != argv.end()) {
                            toAdd += *iter;
                            toAdd += ' ';
                            ++iter;
                        }
                        return cli.DoCLog(mode, 0, &toAdd);
                    }

                case Cli::LOG_NEW:
                    // no more than one argument, no filename == query
                    if (opt.GetNonOptionArguments() > 1) 
                        return cli.SetError("Filename or nothing expected, enclose filename in quotes if there are spaces in the path.");

                    if (opt.GetNonOptionArguments() == 1) return cli.DoCLog(mode, &argv[1]);
                    break; // no args case handled below

                case Cli::LOG_NEWAPPEND:
                    // exactly one argument
                    if (opt.GetNonOptionArguments() > 1)
                        return cli.SetError("Filename expected, enclose filename in quotes if there are spaces in the path.");

                    if (opt.GetNonOptionArguments() < 1)
                        return cli.SetError("Please provide a filename.");
                    return cli.DoCLog(mode, &argv[1]);

                default:
                case Cli::LOG_CLOSE:
                case Cli::LOG_QUERY:
                    // no arguments
                    if (opt.GetNonOptionArguments()) 
                        return cli.SetError("No arguments when querying log status.");
                    break; // no args case handled below
            }

            // the no args case
            return cli.DoCLog(mode);
        }

    private:
        cli::Cli& cli;

        CLogCommand& operator=(const CLogCommand&);
    };

    class CommandToFileCommand : public cli::ParserCommand
    {
    public:
        CommandToFileCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~CommandToFileCommand() {}
        virtual const char* GetString() const { return "command-to-file"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: command-to-file [-a] filename command [args]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // Not going to use normal option parsing in this case because I do not want to disturb the other command on the line
            if (argv.size() < 3)
                return cli.SetError(GetSyntax());

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
                        append = true;
                    else if (arg[1] == '-') 
                    {
                        if (arg[2] == 'a') 
                            append = true;
                        else 
                            unrecognized = true;
                    } 
                    else 
                        unrecognized = true;
                }
                
                if (unrecognized) 
                    return cli.SetError("Unrecognized option: " + arg);

                if (append) 
                {
                    mode = Cli::LOG_NEWAPPEND;

                    // Index of command in argv:  command-to-file -a filename command ...
                    if (argv.size() < 4) 
                        return cli.SetError(GetSyntax());

                    startOfCommand = 3;

                    // Re-set filename if necessary
                    if (i == 1) 
                        filename = argv[2];

                    break;
                }
            }

            // Restructure argv
            std::vector<std::string> newArgv;
            for (std::vector<int>::size_type i = startOfCommand; i < argv.size(); ++i) 
                newArgv.push_back(argv[i]);

            return cli.DoCommandToFile(mode, filename, newArgv);
        }

    private:
        cli::Cli& cli;

        CommandToFileCommand& operator=(const CommandToFileCommand&);
    };

    class DefaultWMEDepthCommand : public cli::ParserCommand
    {
    public:
        DefaultWMEDepthCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~DefaultWMEDepthCommand() {}
        virtual const char* GetString() const { return "default-wme-depth"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: default-wme-depth [depth]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // n defaults to 0 (query)
            int n = 0;

            if (argv.size() > 2) 
                return cli.SetError(GetSyntax());

            // one argument, figure out if it is a positive integer
            if (argv.size() == 2) {
                from_string(n, argv[1]);
                if (n <= 0) return cli.SetError("Depth argument must be positive.");
            }

            return cli.DoDefaultWMEDepth(n ? &n : 0);
        }

    private:
        cli::Cli& cli;

        DefaultWMEDepthCommand& operator=(const DefaultWMEDepthCommand&);
    };

    class DirsCommand : public cli::ParserCommand
    {
    public:
        DirsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~DirsCommand() {}
        virtual const char* GetString() const { return "dirs"; }
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
        virtual const char* GetString() const { return "echo"; }
        virtual const char* GetSyntax() const
        {
            return "Syntax: echo [--nonewline] [string]"; 
        }
        
        virtual bool Parse(std::vector< std::string >&argv)
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
                    return cli.SetError(opt.GetError());

                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'n':
                        echoNewline = false;
                        break;
                }
            }

            // remove the -n arg
            if (!echoNewline)
                argv.erase(++argv.begin());

            return cli.DoEcho(argv, echoNewline);
        }

    private:
        cli::Cli& cli;

        EchoCommand& operator=(const EchoCommand&);
    };

    class EchoCommandsCommand : public cli::ParserCommand
    {
    public:
        EchoCommandsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~EchoCommandsCommand() {}
        virtual const char* GetString() const { return "echo-commands"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: echo-commands [-yn]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'y', "yes",        OPTARG_NONE},
                {'n', "no",            OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            bool echoCommands = true ;
            bool onlyGetValue = true ;

            for (;;) {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'y':
                        echoCommands = true ;
                        onlyGetValue = false ;
                        break ;
                    case 'n':
                        echoCommands = false ;
                        onlyGetValue = false ;
                        break ;
                }
            }

            if (opt.GetNonOptionArguments())
                return cli.SetError("Format is 'echo-commands [--yes | --no]") ;

            return cli.DoEchoCommands(onlyGetValue, echoCommands);
        }

    private:
        cli::Cli& cli;

        EchoCommandsCommand& operator=(const EchoCommandsCommand&);
    };

    class EditProductionCommand : public cli::ParserCommand
    {
    public:
        EditProductionCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~EditProductionCommand() {}
        virtual const char* GetString() const { return "edit-production"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: edit-production production_name";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            if (argv.size() != 2)
                return cli.SetError("Need to include the name of the production to edit.");

            return cli.DoEditProduction(argv[1]);
        }

    private:
        cli::Cli& cli;

        EditProductionCommand& operator=(const EditProductionCommand&);
    };

    class EpMemCommand : public cli::ParserCommand
    {
    public:
        EpMemCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~EpMemCommand() {}
        virtual const char* GetString() const { return "epmem"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: epmem [options]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] =
            {
				{'b', "backup",       OPTARG_NONE},
                {'c', "close",        OPTARG_NONE},
                {'g', "get",          OPTARG_NONE},
                {'s', "set",          OPTARG_NONE},
                {'S', "stats",        OPTARG_NONE},
                {'t', "timers",       OPTARG_NONE},
                {'v', "viz",          OPTARG_NONE},
                {0, 0, OPTARG_NONE} // null
            };

            char option = 0;

            for (;;)
            {
                if ( !opt.ProcessOptions( argv, optionsData ) )
                    return false;

                if (opt.GetOption() == -1) break;

                if (option != 0)
                    return cli.SetError( "epmem takes only one option at a time." );
                option = static_cast<char>(opt.GetOption());
            }

            switch (option)
            {
            case 0:
            default:
                // no options
                break;

            case 'c':
                // case: close gets no arguments
                {
                    if (!opt.CheckNumNonOptArgs(0, 0)) return false;

                    return cli.DoEpMem( option );
                }

			case 'b':
                // case: backup requires one non-option argument
                if (!opt.CheckNumNonOptArgs(1, 1)) return false;

                return cli.DoEpMem( option, &( argv[2] ) );

            case 'g':
                // case: get requires one non-option argument
                {
                    if (!opt.CheckNumNonOptArgs(1, 1)) return false;

                    return cli.DoEpMem( option, &( argv[2] ) );
                }

            case 's':
                // case: set requires two non-option arguments
                {
                    if (!opt.CheckNumNonOptArgs(2, 2)) return false;

                    return cli.DoEpMem( 's', &( argv[2] ), &( argv[3] ) );
                }

            case 'S':
                // case: stat can do zero or one non-option arguments
                {
                    if (!opt.CheckNumNonOptArgs(0, 1)) return false;

                    if ( opt.GetNonOptionArguments() == 0 )
                        return cli.DoEpMem( option );

                    return cli.DoEpMem( option, &( argv[2] ) );
                }

            case 't':
                // case: timer can do zero or one non-option arguments
                {
                    if (!opt.CheckNumNonOptArgs(0, 1)) return false;

                    if ( opt.GetNonOptionArguments() == 0 )
                        return cli.DoEpMem( option );

                    return cli.DoEpMem( option, &( argv[2] ) );
                }

            case 'v':
                // case: viz takes one non-option argument
                {
                    if (!opt.CheckNumNonOptArgs(1, 1)) return false;

                    std::string temp_str( argv[2] );
                    epmem_time_id memory_id;        

                    if ( !from_string( memory_id, temp_str ) )
                        return cli.SetError( "Invalid attribute." );

                    return cli.DoEpMem( option, 0, 0, memory_id );
                }
            }

            // bad: no option, but more than one argument
            if ( argv.size() > 1 ) 
                return cli.SetError( "Too many arguments, check syntax." );

            // case: nothing = full configuration information
            return cli.DoEpMem();    
        }

    private:
        cli::Cli& cli;

        EpMemCommand& operator=(const EpMemCommand&);
    };

    class ExciseCommand : public cli::ParserCommand
    {
    public:
        ExciseCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~ExciseCommand() {}
        virtual const char* GetString() const { return "excise"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: excise production_name\nexcise options";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'a', "all",        OPTARG_NONE},
                {'c', "chunks",        OPTARG_NONE},
                {'d', "default",    OPTARG_NONE},
                {'r', "rl",            OPTARG_NONE},
                {'t', "task",        OPTARG_NONE},
                {'T', "template",    OPTARG_NONE},
                {'u', "user",        OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            Cli::ExciseBitset options(0);

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'a':
                        options.set(Cli::EXCISE_ALL);
                        break;
                    case 'c':
                        options.set(Cli::EXCISE_CHUNKS);
                        break;
                    case 'd':
                        options.set(Cli::EXCISE_DEFAULT);
                        break;
                    case 'r':
                        options.set(Cli::EXCISE_RL);
                        break;
                    case 't':
                        options.set(Cli::EXCISE_TASK);
                        break;
                    case 'T':
                        options.set(Cli::EXCISE_TEMPLATE);
                        break;
                    case 'u':
                        options.set(Cli::EXCISE_USER);
                        break;
                }
            }

            // If there are options, no additional argument.
            if (options.any()) {
                if (opt.GetNonOptionArguments()) return cli.SetError(GetSyntax());
                return cli.DoExcise(options);
            }

            // If there are no options, there must be only one production name argument
            if (opt.GetNonOptionArguments() < 1) 
                return cli.SetError("Production name is required.");        
            if (opt.GetNonOptionArguments() > 1) 
                return cli.SetError("Only one production name allowed, call excise multiple times to excise more than one specific production.");        

            // Pass the production to the cli.DoExcise function
            return cli.DoExcise(options, &(argv[opt.GetArgument() - opt.GetNonOptionArguments()]));
        }

    private:
        cli::Cli& cli;

        ExciseCommand& operator=(const ExciseCommand&);
    };

    class ExplainBacktracesCommand : public cli::ParserCommand
    {
    public:
        ExplainBacktracesCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~ExplainBacktracesCommand() {}
        virtual const char* GetString() const { return "explain-backtraces"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: explain-backtraces [options] [prod_name]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'c', "condition",    OPTARG_REQUIRED},
                {'f', "full",        OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            int condition = 0;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'f':
                        condition = -1;
                        break;

                    case 'c':
                        if (!from_string(condition, opt.GetOptionArgument()) || (condition <= 0))
                            return cli.SetError("Positive integer expected.");
                        break;
                }
            }

            // never more than one arg
            if (opt.GetNonOptionArguments() > 1) return cli.SetError(GetSyntax());

            // we need a production if full or condition given
            if (condition) 
                if (opt.GetNonOptionArguments() < 1) 
                    return cli.SetError("Production name required for that option.");

            // we have a production
            if (opt.GetNonOptionArguments() == 1) return cli.DoExplainBacktraces(&argv[opt.GetArgument() - opt.GetNonOptionArguments()], condition);
            
            // query
            return cli.DoExplainBacktraces();
        }

    private:
        cli::Cli& cli;

        ExplainBacktracesCommand& operator=(const ExplainBacktracesCommand&);
    };

    class FiringCountsCommand : public cli::ParserCommand
    {
    public:
        FiringCountsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~FiringCountsCommand() {}
        virtual const char* GetString() const { return "firing-counts"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: firing-counts [n]\nfiring-counts production_name";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // The number to list defaults to -1 (list all)
            int numberToList = -1;

            // Production defaults to no production
            std::string* pProduction = 0;

            // no more than 1 arg
            if (argv.size() > 2) 
                return cli.SetError(GetSyntax());

            if (argv.size() == 2) {
                // one argument, figure out if it is a non-negative integer or a production
                if ( from_string( numberToList, argv[1] ) ){
                    if (numberToList < 0) return cli.SetError("Expected non-negative integer (count).");

                } else {
                    numberToList = -1;

                    // non-integer argument, hopfully a production
                    pProduction = &(argv[1]);
                }
            }

            return cli.DoFiringCounts(numberToList, pProduction);
        }

    private:
        cli::Cli& cli;

        FiringCountsCommand& operator=(const FiringCountsCommand&);
    };

    class GDSPrintCommand : public cli::ParserCommand
    {
    public:
        GDSPrintCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~GDSPrintCommand() {}
        virtual const char* GetString() const { return "gds-print"; }
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
        virtual const char* GetString() const { return "gp"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: gp { production_body }";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // One argument
            if (argv.size() < 2) 
                return cli.SetError(GetSyntax());
            if (argv.size() > 2) 
                return cli.SetError(GetSyntax());

            return cli.DoGP(argv[1]);
        }

    private:
        cli::Cli& cli;

        GPCommand& operator=(const GPCommand&);
    };

    class GPMaxCommand : public cli::ParserCommand
    {
    public:
        GPMaxCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~GPMaxCommand() {}
        virtual const char* GetString() const { return "gp-max"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: gp-max [value]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // n defaults to 0 (print current value)
            int n = -1;

            if (argv.size() > 2) return cli.SetError(GetSyntax());

            // one argument, figure out if it is a positive integer
            if (argv.size() == 2) {
                from_string(n, argv[1]);
                if (n < 0) 
                    return cli.SetError("Value must be non-negative.");
            }

            return cli.DoGPMax(n);
        }

    private:
        cli::Cli& cli;

        GPMaxCommand& operator=(const GPMaxCommand&);
    };

    class HelpCommand : public cli::ParserCommand
    {
    public:
        HelpCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~HelpCommand() {}
        virtual const char* GetString() const { return "help"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: help [command]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            return cli.DoHelp(argv);
        }

    private:
        cli::Cli& cli;

        HelpCommand& operator=(const HelpCommand&);
    };

    class IndifferentSelectionCommand : public cli::ParserCommand
    {
    public:
        IndifferentSelectionCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~IndifferentSelectionCommand() {}
        virtual const char* GetString() const { return "indifferent-selection"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: indifferent-selection\n"
                "indifferent-selection [-s]\n"
                "indifferent-selection [-bgfxl]\n"
                "indifferent-selection [-et] [value]\n"
                "indifferent-selection [-p] parameter [reduction_policy]\n"
                "indifferent-selection [-r] parameter reduction_policy [reduction_rate]\n"
                "indifferent-selection [-a] [setting]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                // selection policies
                {'b', "boltzmann",            OPTARG_NONE},
                {'g', "epsilon-greedy",        OPTARG_NONE},
                {'f', "first",                OPTARG_NONE},
                {'l', "last",                OPTARG_NONE},
                //{'u', "random-uniform",        OPTARG_NONE},
                {'x', "softmax",            OPTARG_NONE},

                // selection parameters
                {'e', "epsilon",            OPTARG_NONE},
                {'t', "temperature",        OPTARG_NONE},

                // auto-reduction control
                {'a', "auto-reduce",        OPTARG_NONE},

                // selection parameter reduction
                {'p', "reduction-policy",    OPTARG_NONE},
                {'r', "reduction-rate",        OPTARG_NONE},

                // stats
                {'s', "stats",                OPTARG_NONE},

                {0, 0, OPTARG_NONE} // null
            };

            char option = 0;

            for (;;) 
            {
                if ( !opt.ProcessOptions( argv, optionsData ) ) 
                    return false;

                if (opt.GetOption() == -1) break;

                if (option != 0)
                    return cli.SetError( "indifferent-selection takes only one option at a time." );
                option = static_cast<char>(opt.GetOption());
            }

            switch (option)
            {
            case 0:
            default:
                // no options
                break;

            case 'b':
            case 'g':
            case 'f':
            case 'l':
            case 'x':
                // case: exploration policy takes no non-option arguments
                if (!opt.CheckNumNonOptArgs(0, 0)) return false;
                return cli.DoIndifferentSelection( option );

            case 'e':
            case 't':
                // case: selection parameter can do zero or one non-option arguments
                if (!opt.CheckNumNonOptArgs(0, 1)) return false;

                if ( opt.GetNonOptionArguments() == 0 )
                    return cli.DoIndifferentSelection( option );

                return cli.DoIndifferentSelection( option, &( argv[2] ) );

            case 'a':
                // case: auto reduction control can do zero or one non-option arguments
                if (!opt.CheckNumNonOptArgs(0, 1)) return false;

                if ( opt.GetNonOptionArguments() == 0 )
                    return cli.DoIndifferentSelection( option );

                return cli.DoIndifferentSelection( option, &( argv[2] ) );

            case 'p':
                // case: reduction policy requires one or two non-option arguments
                if (!opt.CheckNumNonOptArgs(1, 2)) return false;

                if ( opt.GetNonOptionArguments() == 1 )
                    return cli.DoIndifferentSelection( option, &( argv[2] ) );

                return cli.DoIndifferentSelection( option, &( argv[2] ), &( argv[3] ) );

            case 'r':
                // case: reduction policy rate requires two or three arguments
                if (!opt.CheckNumNonOptArgs(2, 3)) return false;

                if ( opt.GetNonOptionArguments() == 2 )
                    return cli.DoIndifferentSelection( option, &( argv[2] ), &( argv[3] ) );

                 return cli.DoIndifferentSelection( option, &( argv[2] ), &( argv[3] ), &( argv[4] ) ); 

            case 's':
                // case: stats takes no parameters
                if (!opt.CheckNumNonOptArgs(0, 0)) return false;

                return cli.DoIndifferentSelection( option );
            }

            // bad: no option, but more than one argument
            if ( argv.size() > 1 ) 
                return cli.SetError( "Too many args." );

            // case: nothing = full configuration information
            return cli.DoIndifferentSelection();    
        }

    private:
        cli::Cli& cli;

        IndifferentSelectionCommand& operator=(const IndifferentSelectionCommand&);
    };

    class InitSoarCommand : public cli::ParserCommand
    {
    public:
        InitSoarCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~InitSoarCommand() {}
        virtual const char* GetString() const { return "init-soar"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: init-soar";
        }

        virtual bool Parse(std::vector< std::string >&)
        {
            return cli.DoInitSoar();
        }

    private:
        cli::Cli& cli;

        InitSoarCommand& operator=(const InitSoarCommand&);
    };

    class InternalSymbolsCommand : public cli::ParserCommand
    {
    public:
        InternalSymbolsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~InternalSymbolsCommand() {}
        virtual const char* GetString() const { return "internal-symbols"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: internal-symbols";
        }

        virtual bool Parse(std::vector< std::string >&)
        {
            return cli.DoInternalSymbols();
        }

    private:
        cli::Cli& cli;

        InternalSymbolsCommand& operator=(const InternalSymbolsCommand&);
    };

    class LearnCommand : public cli::ParserCommand
    {
    public:
        LearnCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~LearnCommand() {}
        virtual const char* GetString() const { return "learn"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: learn [-l]\nlearn [-d|E|o]\nlearn [-eabnN]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
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
                {'n', "enable-through-local-negations", OPTARG_NONE},
                {'N', "disable-through-local-negations", OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            Cli::LearnBitset options(0);

            for (;;) {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
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
                }
            }

            // No non-option arguments
            if (opt.GetNonOptionArguments()) 
                return cli.SetError(GetSyntax());

            return cli.DoLearn(options);
        }

    private:
        cli::Cli& cli;

        LearnCommand& operator=(const LearnCommand&);
    };

    class LoadLibraryCommand : public cli::ParserCommand
    {
    public:
        LoadLibraryCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~LoadLibraryCommand() {}
        virtual const char* GetString() const { return "load-library"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: load-library [library_name] [arguments]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // command-name library-name [library-args ...]

            if (argv.size() < 2) 
                return cli.SetError(GetSyntax());

            // strip the command name, combine the rest
            std::string libraryCommand(argv[1]);
            for(std::string::size_type i = 2; i < argv.size(); ++i) {
                libraryCommand += " ";
                libraryCommand += argv[i];
            }

            return cli.DoLoadLibrary(libraryCommand);
        }

    private:
        cli::Cli& cli;

        LoadLibraryCommand& operator=(const LoadLibraryCommand&);
    };

    class LSCommand : public cli::ParserCommand
    {
    public:
        LSCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~LSCommand() {}
        virtual const char* GetString() const { return "ls"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: ls";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // No arguments
            if (argv.size() != 1) {
                return cli.SetError(GetSyntax());
            }
            return cli.DoLS();
        }

    private:
        cli::Cli& cli;

        LSCommand& operator=(const LSCommand&);
    };

    class MatchesCommand : public cli::ParserCommand
    {
    public:
        MatchesCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~MatchesCommand() {}
        virtual const char* GetString() const { return "matches"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: matches [options] production_name\nmatches [options] -[a|r]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'a', "assertions",        OPTARG_NONE},
                {'c', "count",            OPTARG_NONE},
                {'n', "names",            OPTARG_NONE},
                {'r', "retractions",    OPTARG_NONE},
                {'t', "timetags",        OPTARG_NONE},
                {'w', "wmes",            OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            Cli::eWMEDetail detail = Cli::WME_DETAIL_NONE;
            Cli::eMatchesMode mode = Cli::MATCHES_ASSERTIONS_RETRACTIONS;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'n':
                    case 'c':
                        detail = Cli::WME_DETAIL_NONE;
                        break;
                    case 't':
                        detail = Cli::WME_DETAIL_TIMETAG;
                        break;
                    case 'w':
                        detail = Cli::WME_DETAIL_FULL;
                        break;
                    case 'a':
                        mode = Cli::MATCHES_ASSERTIONS;
                        break;
                    case 'r':
                        mode = Cli::MATCHES_RETRACTIONS;
                        break;
                }
            }

            // Max one additional argument and it is a production
            if (opt.GetNonOptionArguments() > 1) 
                return cli.SetError(GetSyntax());        

            if (opt.GetNonOptionArguments() == 1) 
            {
                if (mode != Cli::MATCHES_ASSERTIONS_RETRACTIONS) 
                    return cli.SetError(GetSyntax());
                return cli.DoMatches(Cli::MATCHES_PRODUCTION, detail, &argv[opt.GetArgument() - opt.GetNonOptionArguments()]);
            }

            return cli.DoMatches(mode, detail);
        }

    private:
        cli::Cli& cli;

        MatchesCommand& operator=(const MatchesCommand&);
    };

    class MaxChunksCommand : public cli::ParserCommand
    {
    public:
        MaxChunksCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~MaxChunksCommand() {}
        virtual const char* GetString() const { return "max-chunks"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: max-chunks [n]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // n defaults to 0 (print current value)
            int n = 0;

            if (argv.size() > 2) return cli.SetError(GetSyntax());

            // one argument, figure out if it is a positive integer
            if (argv.size() == 2) 
            {
                from_string(n, argv[1]);
                if (n <= 0) return cli.SetError("Expected positive integer.");
            }

            return cli.DoMaxChunks(n);
        }

    private:
        cli::Cli& cli;

        MaxChunksCommand& operator=(const MaxChunksCommand&);
    };

    class MaxDCTimeCommand : public cli::ParserCommand
    {
    public:
        MaxDCTimeCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~MaxDCTimeCommand() {}
        virtual const char* GetString() const { return "max-dc-time"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: max-dc-time [--seconds] [n]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'s', "seconds", OPTARG_NONE},
                {'d', "disable", OPTARG_NONE},
                {'o', "off", OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            bool seconds = false;

            // n defaults to 0 (print current value)
            int n = 0;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 's':
                        seconds = true;
                        break;
                    case 'd':
                    case 'o':
                        n = -1;
                        break;
                }
            }

            if (opt.GetNonOptionArguments() > 1) 
                return cli.SetError(GetSyntax());       
            
            if (opt.GetNonOptionArguments() == 1)
            {
                int index = opt.GetArgument() - opt.GetNonOptionArguments();

                if (seconds)
                {
                    double nsec = 0;

                    if (!from_string(nsec, argv[index]))
                        return cli.SetError(GetSyntax());   

                    n = static_cast<int>(nsec * 1000000);
                }
                else
                {
                    from_string(n, argv[index]);
                }

                if (n <= 0) 
                    return cli.SetError("Expected positive value.");
            }

            return cli.DoMaxDCTime(n);
        }

    private:
        cli::Cli& cli;

        MaxDCTimeCommand& operator=(const MaxDCTimeCommand&);
    };

    class MaxElaborationsCommand : public cli::ParserCommand
    {
    public:
        MaxElaborationsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~MaxElaborationsCommand() {}
        virtual const char* GetString() const { return "max-elaborations"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: max-elaborations [n]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // n defaults to 0 (print current value)
            int n = 0;

            if (argv.size() > 2) return cli.SetError(GetSyntax());

            // one argument, figure out if it is a positive integer
            if (argv.size() == 2) 
            {
                from_string(n, argv[1]);
                if (n <= 0) return cli.SetError("Expected positive integer.");
            }

            return cli.DoMaxElaborations(n);
        }

    private:
        cli::Cli& cli;

        MaxElaborationsCommand& operator=(const MaxElaborationsCommand&);
    };

    class MaxGoalDepthCommand : public cli::ParserCommand
    {
    public:
        MaxGoalDepthCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~MaxGoalDepthCommand() {}
        virtual const char* GetString() const { return "max-goal-depth"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: max-goal-depth [n]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // n defaults to 0 (print current value)
            int n = 0;

            if (argv.size() > 2) return cli.SetError(GetSyntax());

            // one argument, figure out if it is a positive integer
            if (argv.size() == 2) {
                from_string(n, argv[1]);
                if (n <= 0) return cli.SetError("Expected positive integer.");
            }

            return cli.DoMaxGoalDepth(n);
        }

    private:
        cli::Cli& cli;

        MaxGoalDepthCommand& operator=(const MaxGoalDepthCommand&);
    };

    class MaxMemoryUsageCommand : public cli::ParserCommand
    {
    public:
        MaxMemoryUsageCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~MaxMemoryUsageCommand() {}
        virtual const char* GetString() const { return "max-memory-usage"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: max-memory-usage [n]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // n defaults to 0 (print current value)
            int n = 0;

            if (argv.size() > 2) 
                return cli.SetError(GetSyntax());

            // one argument, figure out if it is a positive integer
            if (argv.size() == 2) 
            {
                from_string(n, argv[1]);
                if (n <= 0) return cli.SetError("Expected positive integer.");
            }

            return cli.DoMaxMemoryUsage(n);
        }

    private:
        cli::Cli& cli;

        MaxMemoryUsageCommand& operator=(const MaxMemoryUsageCommand&);
    };

    class MaxNilOutputCyclesCommand : public cli::ParserCommand
    {
    public:
        MaxNilOutputCyclesCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~MaxNilOutputCyclesCommand() {}
        virtual const char* GetString() const { return "max-nil-output-cycles"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: max-nil-output-cycles [n]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // n defaults to 0 (print current value)
            int n = 0;

            if (argv.size() > 2) 
                return cli.SetError(GetSyntax());

            // one argument, figure out if it is a positive integer
            if (argv.size() == 2)
            {
                from_string(n, argv[1]);
                if (n <= 0) return cli.SetError("Expected positive integer.");
            }

            return cli.DoMaxNilOutputCycles(n);
        }

    private:
        cli::Cli& cli;

        MaxNilOutputCyclesCommand& operator=(const MaxNilOutputCyclesCommand&);
    };

    class MemoriesCommand : public cli::ParserCommand
    {
    public:
        MemoriesCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~MemoriesCommand() {}
        virtual const char* GetString() const { return "memories"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: memories [options] [number]\nmemories production_name";
        }

        virtual bool Parse(std::vector< std::string >&argv)
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

            Cli::MemoriesBitset options(0);

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'c':
                        options.set(Cli::MEMORIES_CHUNKS);
                        break;
                    case 'd':
                        options.set(Cli::MEMORIES_DEFAULT);
                        break;
                    case 'j':
                        options.set(Cli::MEMORIES_JUSTIFICATIONS);
                        break;
                    case 'T':
                        options.set(Cli::MEMORIES_TEMPLATES);
                        break;
                    case 'u':
                        options.set(Cli::MEMORIES_USER);
                        break;
                }
            }

            // Max one additional argument
            if (opt.GetNonOptionArguments() > 1) 
                return cli.SetError(GetSyntax());        

            // It is either a production or a number
            int n = 0;
            if (opt.GetNonOptionArguments() == 1) {
                int optind = opt.GetArgument() - opt.GetNonOptionArguments();
                if ( from_string( n, argv[optind] ) ) {
                    // number
                    if (n <= 0) return cli.SetError("Expected positive integer.");
                } else {
                    // production
                    if (options.any()) return cli.SetError("Do not specify production type when specifying a production name.");
                    return cli.DoMemories(options, 0, &argv[optind]);
                }
            }

            // Default to all types when no production and no type specified
            if (options.none()) options.flip();

            // handle production/number cases
            return cli.DoMemories(options, n);
        }

    private:
        cli::Cli& cli;

        MemoriesCommand& operator=(const MemoriesCommand&);
    };

    class MultiAttributesCommand : public cli::ParserCommand
    {
    public:
        MultiAttributesCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~MultiAttributesCommand() {}
        virtual const char* GetString() const { return "multi-attributes"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: multi-attributes [symbol [n]]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // No more than three arguments
            if (argv.size() > 3) return cli.SetError(GetSyntax());

            int n = 0;
            // If we have 3 arguments, third one is an integer
            if (argv.size() > 2) 
            {
                if ( !from_string( n, argv[2] ) || (n <= 0) ) 
                    return cli.SetError("Expected non-negative integer.");
            }

            // If we have two arguments, second arg is an attribute/identifer/whatever
            if (argv.size() > 1) return cli.DoMultiAttributes(&argv[1], n);

            return cli.DoMultiAttributes();
        }

    private:
        cli::Cli& cli;

        MultiAttributesCommand& operator=(const MultiAttributesCommand&);
    };

    class NumericIndifferentModeCommand : public cli::ParserCommand
    {
    public:
        NumericIndifferentModeCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~NumericIndifferentModeCommand() {}
        virtual const char* GetString() const { return "numeric-indifferent-mode"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: numeric-indifferent-mode [-as]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'a', "average",    OPTARG_NONE},
                {'a', "avg",        OPTARG_NONE},
                {'s', "sum",        OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            ni_mode mode = NUMERIC_INDIFFERENT_MODE_AVG;
            bool query = true;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'a':
                        mode = NUMERIC_INDIFFERENT_MODE_AVG;
                        query = false;
                        break;
                    case 's':
                        mode = NUMERIC_INDIFFERENT_MODE_SUM;
                        query = false;
                        break;
                }
            }

            // No additional arguments
            if (opt.GetNonOptionArguments()) return cli.SetError(GetSyntax());        

            return cli.DoNumericIndifferentMode( query, mode );
        }

    private:
        cli::Cli& cli;

        NumericIndifferentModeCommand& operator=(const NumericIndifferentModeCommand&);
    };

    class OSupportModeCommand : public cli::ParserCommand
    {
    public:
        OSupportModeCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~OSupportModeCommand() {}
        virtual const char* GetString() const { return "o-support-mode"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: o-support-mode [n]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            if (argv.size() > 2) return cli.SetError(GetSyntax());

            int mode = -1;
            if (argv.size() == 2) {
                if (!isdigit(argv[1][0])) 
                    return cli.SetError("Expected an integer 0, 2, 3, or 4.");
                from_string(mode, argv[1]);
                if (mode < 0 || mode > 4 || mode == 1) 
                    return cli.SetError("Expected an integer 0, 2, 3, or 4.");
            }

            return cli.DoOSupportMode(mode);
        }

    private:
        cli::Cli& cli;

        OSupportModeCommand& operator=(const OSupportModeCommand&);
    };

    class PopDCommand : public cli::ParserCommand
    {
    public:
        PopDCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~PopDCommand() {}
        virtual const char* GetString() const { return "popd"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: popd";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // No arguments
            if (argv.size() != 1)
                return cli.SetError(GetSyntax());
            return cli.DoPopD();
        }

    private:
        cli::Cli& cli;

        PopDCommand& operator=(const PopDCommand&);
    };

    class PortCommand : public cli::ParserCommand
    {
    public:
        PortCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~PortCommand() {}
        virtual const char* GetString() const { return "port"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: port";
        }

        virtual bool Parse(std::vector< std::string >&)
        {
            return cli.DoPort();
        }

    private:
        cli::Cli& cli;

        PortCommand& operator=(const PortCommand&);
    };

    class PredictCommand : public cli::ParserCommand
    {
    public:
        PredictCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~PredictCommand() {}
        virtual const char* GetString() const { return "predict"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: predict";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // No arguments to predict next operator
            if ( argv.size() != 1 ) 
                return cli.SetError( "predict takes no arguments." );
            
            return cli.DoPredict( );
        }

    private:
        cli::Cli& cli;

        PredictCommand& operator=(const PredictCommand&);
    };

    class PreferencesCommand : public cli::ParserCommand
    {
    public:
        PreferencesCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~PreferencesCommand() {}
        virtual const char* GetString() const { return "preferences"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: preferences [options] [identifier [attribute]]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
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
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
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
                return cli.SetError(GetSyntax());

            int optind = opt.GetArgument() - opt.GetNonOptionArguments();
            if (opt.GetNonOptionArguments() == 2) {
                // id & attribute
                return cli.DoPreferences(detail, object, &argv[optind], &argv[optind + 1]);
            }
            if (opt.GetNonOptionArguments() == 1) {
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
        virtual const char* GetString() const { return "print"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: print [options] [production_name]\nprint [options] identifier|timetag|pattern";
        }

        virtual bool Parse(std::vector< std::string >&argv)
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

            for (;;) {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'a':
                        options.set(Cli::PRINT_ALL);
                        break;
                    case 'c':
                        options.set(Cli::PRINT_CHUNKS);
                        break;
                    case 'd':
                        options.set(Cli::PRINT_DEPTH);
                        if ( !from_string( depth, opt.GetOptionArgument() ) || (depth < 0) ) 
                            return cli.SetError("Non-negative depth expected.");
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
            if (options.test(Cli::PRINT_OPERATORS) || options.test(Cli::PRINT_STATES)) {
                if (!options.test(Cli::PRINT_STACK)) return cli.SetError("Options --operators (-o) and --states (-S) are only valid when printing the stack.");
            }

            if (opt.GetNonOptionArguments() == 0) {
                // d and t options require an argument
                if (options.test(Cli::PRINT_TREE) || options.test(Cli::PRINT_DEPTH)) return cli.SetError(GetSyntax());
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
                return cli.SetError("No depth/tree flags allowed when printing exact.");

            std::string arg;
            for (int i = opt.GetArgument() - opt.GetNonOptionArguments(); i < argv.size(); ++i)
            {
                if (!arg.empty())
                    arg.push_back(' ');
                arg.append(argv[i]);
            }
            return cli.DoPrint(options, depth, &arg);
        }

    private:
        cli::Cli& cli;

        PrintCommand& operator=(const PrintCommand&);
    };

    class ProductionFindCommand : public cli::ParserCommand
    {
    public:
        ProductionFindCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~ProductionFindCommand() {}
        virtual const char* GetString() const { return "production-find"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: production-find [-lrs[n|c]] pattern";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'c', "chunks",            OPTARG_NONE},
                {'l', "lhs",                OPTARG_NONE},
                {'n', "nochunks",        OPTARG_NONE},
                {'r', "rhs",                OPTARG_NONE},
                {'s', "show-bindings",    OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            Cli::ProductionFindBitset options(0);

            for (;;) {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'c':
                        options.set(Cli::PRODUCTION_FIND_ONLY_CHUNKS);
                        options.reset(Cli::PRODUCTION_FIND_NO_CHUNKS);
                        break;
                    case 'l':
                        options.set(Cli::PRODUCTION_FIND_INCLUDE_LHS);
                        break;
                    case 'n':
                        options.set(Cli::PRODUCTION_FIND_NO_CHUNKS);
                        options.reset(Cli::PRODUCTION_FIND_ONLY_CHUNKS);
                        break;
                    case 'r':
                        options.set(Cli::PRODUCTION_FIND_INCLUDE_RHS);
                        break;
                    case 's':
                        options.set(Cli::PRODUCTION_FIND_SHOWBINDINGS);
                        break;
                }
            }

            if (!opt.GetNonOptionArguments())
                return cli.SetError(GetSyntax());

            if (options.none()) options.set(Cli::PRODUCTION_FIND_INCLUDE_LHS);

            std::string pattern;
            for (unsigned i = opt.GetArgument() - opt.GetNonOptionArguments(); i < argv.size(); ++i) {
                pattern += argv[i];
                pattern += ' ';
            }
            pattern = pattern.substr(0, pattern.length() - 1);

            return cli.DoProductionFind(options, pattern);
        }

    private:
        cli::Cli& cli;

        ProductionFindCommand& operator=(const ProductionFindCommand&);
    };

    class PushDCommand : public cli::ParserCommand
    {
    public:
        PushDCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~PushDCommand() {}
        virtual const char* GetString() const { return "pushd"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: pushd directory";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // Only takes one argument, the directory to change into
            if (argv.size() < 2) 
                return cli.SetError(GetSyntax());
            if (argv.size() > 2)
                return cli.SetError("Expected on argument (directory). Enclose directory in quotes if there are spaces in the path.");
            return cli.DoPushD(argv[1]);
        }

    private:
        cli::Cli& cli;

        PushDCommand& operator=(const PushDCommand&);
    };

    class PWatchCommand : public cli::ParserCommand
    {
    public:
        PWatchCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~PWatchCommand() {}
        virtual const char* GetString() const { return "pwatch"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: pwatch [-d|e] [production name]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'d', "disable",    OPTARG_NONE},
                {'e', "enable",        OPTARG_NONE},
                {'d', "off",        OPTARG_NONE},
                {'e', "on",            OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            bool setting = true;
            bool query = true;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'd':
                        setting = false;
                        query = false;
                        break;
                    case 'e':
                        setting = true;
                        break;
                }
            }
            if (opt.GetNonOptionArguments() > 1) 
                return cli.SetError(GetSyntax());

            if (opt.GetNonOptionArguments() == 1) 
                return cli.DoPWatch(false, &argv[opt.GetArgument() - opt.GetNonOptionArguments()], setting);
            return cli.DoPWatch(query, 0);
        }

    private:
        cli::Cli& cli;

        PWatchCommand& operator=(const PWatchCommand&);
    };

    class PWDCommand : public cli::ParserCommand
    {
    public:
        PWDCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~PWDCommand() {}
        virtual const char* GetString() const { return "pwd"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: pwd";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // No arguments to print working directory
            if (argv.size() != 1) 
                return cli.SetError(GetSyntax());
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
        virtual const char* GetString() const { return "rand"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: rand\nrand n\nrand --integer\nrand --integer n";
        }

        virtual bool Parse(std::vector< std::string >&argv)
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
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'i':
                        integer = true;
                        break;
                }
            }

            if ( opt.GetNonOptionArguments() > 1 ) 
                return cli.SetError( GetSyntax() );
            else if ( opt.GetNonOptionArguments() == 1 ) 
            {
                unsigned optind = opt.GetArgument() - opt.GetNonOptionArguments();
                return cli.DoRand( integer, &(argv[optind]) );
            }

            return cli.DoRand( integer, 0 );
        }

    private:
        cli::Cli& cli;

        RandCommand& operator=(const RandCommand&);
    };

    class RemoveWMECommand : public cli::ParserCommand
    {
    public:
        RemoveWMECommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~RemoveWMECommand() {}
        virtual const char* GetString() const { return "remove-wme"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: remove-wme timetag";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // Exactly one argument
            if (argv.size() < 2)
                return cli.SetError(GetSyntax());
            if (argv.size() > 2) 
                return cli.SetError(GetSyntax());

            uint64_t timetag = 0;
            from_string(timetag, argv[1]);
            if (!timetag) 
                return cli.SetError("Timetag must be positive.");

            return cli.DoRemoveWME(timetag);
        }

    private:
        cli::Cli& cli;

        RemoveWMECommand& operator=(const RemoveWMECommand&);
    };

    class ReplayInputCommand : public cli::ParserCommand
    {
    public:
        ReplayInputCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~ReplayInputCommand() {}
        virtual const char* GetString() const { return "replay-input"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: replay-input --open filename\nreplay-input [--query]\nreplay-input --close";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] =
            {
                {'c', "close", OPTARG_NONE},
                {'o', "open", OPTARG_REQUIRED},
                {'q', "query", OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            Cli::eReplayInputMode mode = Cli::REPLAY_INPUT_QUERY;
            std::string pathname;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'c':
                        mode = Cli::REPLAY_INPUT_CLOSE;
                        break;
                    case 'o':
                        mode = Cli::REPLAY_INPUT_OPEN;
                        pathname = opt.GetOptionArgument();
                        break;
                    case 'q':
                        mode = Cli::REPLAY_INPUT_QUERY;
                        break;
                }
            }

            return cli.DoReplayInput(mode, mode == Cli::REPLAY_INPUT_OPEN ? &pathname : 0);
        }

    private:
        cli::Cli& cli;

        ReplayInputCommand& operator=(const ReplayInputCommand&);
    };

    class ReteNetCommand : public cli::ParserCommand
    {
    public:
        ReteNetCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~ReteNetCommand() {}
        virtual const char* GetString() const { return "rete-net"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: rete-net -s|l filename";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'l', "load",        OPTARG_REQUIRED},
                {'r', "restore",    OPTARG_REQUIRED},
                {'s', "save",        OPTARG_REQUIRED},
                {0, 0, OPTARG_NONE}
            };

            bool save = false;
            bool load = false;
            std::string filename;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption())
                {
                    case 'l':
                    case 'r':
                        load = true;
                        save = false;
                        filename = opt.GetOptionArgument();
                        break;
                    case 's':
                        save = true;
                        load = false;
                        filename = opt.GetOptionArgument();
                        break;
                }
            }

            // Must have a save or load operation
            if (!save && !load)
                return cli.SetError(GetSyntax());
            if (opt.GetNonOptionArguments())
                return cli.SetError(GetSyntax());

            return cli.DoReteNet(save, filename);
        }

    private:
        cli::Cli& cli;

        ReteNetCommand& operator=(const ReteNetCommand&);
    };

    class RLCommand : public cli::ParserCommand
    {
    public:
        RLCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~RLCommand() {}
        virtual const char* GetString() const { return "rl"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: rl [options parameter|statstic]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'g', "get",    OPTARG_NONE},
                {'s', "set",    OPTARG_NONE},
                {'S', "stats",    OPTARG_NONE},
                {0, 0, OPTARG_NONE} // null
            };

            char option = 0;

            for (;;) 
            {
                if ( !opt.ProcessOptions( argv, optionsData ) ) 
                    return false;

                if (opt.GetOption() == -1) break;

                if (option != 0)
                    return cli.SetError( "rl takes only one option at a time." );
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
                    if (!opt.CheckNumNonOptArgs(1, 1)) return false;

                    return cli.DoRL( option, &( argv[2] ) );        
                }

            case 's':
                // case: set requires two non-option arguments
                {
                    if (!opt.CheckNumNonOptArgs(2, 2)) return false;

                    return cli.DoRL( option, &( argv[2] ), &( argv[3] ) );
                }

            case 'S':
                // case: stat can do zero or one non-option arguments
                {
                    if (!opt.CheckNumNonOptArgs(0, 1)) return false;

                    if ( opt.GetNonOptionArguments() == 0 )
                        return cli.DoRL( option );

                    return cli.DoRL( option, &( argv[2] ) );
                }
            }

            // bad: no option, but more than one argument
            if ( argv.size() > 1 ) 
                return cli.SetError( "Invalid syntax." );

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
        virtual const char* GetString() const { return "run"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: run  [-f|count]\nrun -[d|e|o|p][s][un][g] [f|count]\nrun -[d|e|o|p][un] count [-i e|p|d|o]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'d', "decision",        OPTARG_NONE},
                {'e', "elaboration",    OPTARG_NONE},
                {'f', "forever",        OPTARG_NONE},
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
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'd':
                        options.set(Cli::RUN_DECISION);
                        break;
                    case 'e':
                        options.set(Cli::RUN_ELABORATION);
                        break;
                    case 'f':
                        options.set(Cli::RUN_FOREVER);
                        break;
                    case 'g':
                        options.set(Cli::RUN_GOAL);
                        break;
                    case 'i':
                        options.set(Cli::RUN_INTERLEAVE);
                        interleaveMode = ParseRunInterleaveOptarg(opt);
                        if (interleaveMode == Cli::RUN_INTERLEAVE_DEFAULT) 
                            return false; // error set in parse function
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
                return cli.SetError(GetSyntax());

            // Decide if we explicitly indicated how to run
            bool specifiedType = (options.test(Cli::RUN_FOREVER) || options.test(Cli::RUN_ELABORATION) || options.test(Cli::RUN_DECISION) || options.test(Cli::RUN_PHASE) || options.test(Cli::RUN_OUTPUT)) ;

            // Count defaults to -1
            int count = -1;
            if (opt.GetNonOptionArguments() == 1) {
                int optind = opt.GetArgument() - opt.GetNonOptionArguments();
                if ( !from_string( count, argv[optind] ) ) 
                    return cli.SetError("Integer count expected.");
                // Allow "run 0" for decisions -- which means run agents to the current stop-before phase
                if (count < 0 || (count == 0 && specifiedType && !options.test(Cli::RUN_DECISION))) 
                    return cli.SetError("Count must be positive.");
            } 

            return cli.DoRun(options, count, interleaveMode);
        }

    private:
        cli::Cli& cli;

        Cli::eRunInterleaveMode ParseRunInterleaveOptarg(cli::Options& opt)
        {
            if (opt.GetOptionArgument() == "d") 
                return Cli::RUN_INTERLEAVE_DECISION;
            else if (opt.GetOptionArgument() == "e")
                return Cli::RUN_INTERLEAVE_ELABORATION;
            else if (opt.GetOptionArgument() == "o") 
                return Cli::RUN_INTERLEAVE_OUTPUT;
            else if (opt.GetOptionArgument() == "p")
                return Cli::RUN_INTERLEAVE_PHASE;

            cli.SetError("Invalid interleave switch: " + opt.GetOptionArgument());
            return Cli::RUN_INTERLEAVE_DEFAULT;
        }

        RunCommand& operator=(const RunCommand&);
    };

    class SaveBacktracesCommand : public cli::ParserCommand
    {
    public:
        SaveBacktracesCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~SaveBacktracesCommand() {}
        virtual const char* GetString() const { return "save-backtraces"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: save-backtraces [-ed]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] =
            {
                {'d', "disable",    OPTARG_NONE},
                {'e', "enable",        OPTARG_NONE},
                {'d', "off",        OPTARG_NONE},
                {'e', "on",            OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            bool setting = true;
            bool query = true;

            for (;;) {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'd':
                        setting = false;
                        query = false;
                        break;
                    case 'e':
                        setting = true;
                        query = false;
                        break;
                }
            }
            if (opt.GetNonOptionArguments()) 
                return cli.SetError(GetSyntax());
            return cli.DoSaveBacktraces(query ? 0 : &setting);
        }

    private:
        cli::Cli& cli;

        SaveBacktracesCommand& operator=(const SaveBacktracesCommand&);
    };

    class SelectCommand : public cli::ParserCommand
    {
    public:
        SelectCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~SelectCommand() {}
        virtual const char* GetString() const { return "select"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: select id";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // At most one argument to select the next operator
            if ( argv.size() > 2 ) 
                return cli.SetError( GetSyntax() );
            
            if ( argv.size() == 2 )
                return cli.DoSelect( &( argv[1] ) );
            
            return cli.DoSelect( );
        }

    private:
        cli::Cli& cli;

        SelectCommand& operator=(const SelectCommand&);
    };

    class SetLibraryLocationCommand : public cli::ParserCommand
    {
    public:
        SetLibraryLocationCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~SetLibraryLocationCommand() {}
        virtual const char* GetString() const { return "set-library-location"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: set-library-location [directory]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            if (argv.size() > 2) 
                return cli.SetError("Expected a path, please enclose in quotes if there are spaces in the path.");

            if (argv.size() == 2)
                return cli.DoSetLibraryLocation(&(argv[1]));
            return cli.DoSetLibraryLocation();
        }

    private:
        cli::Cli& cli;

        SetLibraryLocationCommand& operator=(const SetLibraryLocationCommand&);
    };

    class SetStopPhaseCommand : public cli::ParserCommand
    {
    public:
        SetStopPhaseCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~SetStopPhaseCommand() {}
        virtual const char* GetString() const { return "set-stop-phase"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: set-stop-phase -[ABadiop]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] =
            {
                {'B', "before",        OPTARG_NONE},    // optional (defaults to before)
                {'A', "after",        OPTARG_NONE},    // optional
                {'i', "input",        OPTARG_NONE},    // requires one of these
                {'p', "proposal",    OPTARG_NONE},
                {'d', "decision",    OPTARG_NONE},
                {'a', "apply",        OPTARG_NONE},
                {'o', "output",        OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            sml::smlPhase phase = sml::sml_INPUT_PHASE ;
            int countPhaseArgs = 0 ;
            bool before = true ;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'B':
                        before = true ;
                        break ;
                    case 'A':
                        before = false ;
                        break ;
                    case 'i':
                        phase = sml::sml_INPUT_PHASE ;
                        countPhaseArgs++ ;
                        break;
                    case 'p':
                        phase = sml::sml_PROPOSAL_PHASE ;
                        countPhaseArgs++ ;
                        break;
                    case 'd':
                        phase = sml::sml_DECISION_PHASE ;
                        countPhaseArgs++ ;
                        break;
                    case 'a':
                        phase = sml::sml_APPLY_PHASE ;
                        countPhaseArgs++ ;
                        break;
                    case 'o':
                        phase = sml::sml_OUTPUT_PHASE ;
                        countPhaseArgs++ ;
                        break;
                }
            }

            if (opt.GetNonOptionArguments() || countPhaseArgs > 1)
                return cli.SetError("Format is 'set-stop-phase [--Before | --After] <phase>' where <phase> is --input | --proposal | --decision | --apply | --output\ne.g. set-stop-phase --before --input") ;

            return cli.DoSetStopPhase(countPhaseArgs == 1, before, phase);
        }

    private:
        cli::Cli& cli;

        SetStopPhaseCommand& operator=(const SetStopPhaseCommand&);
    };

    class SMemCommand : public cli::ParserCommand
    {
    public:
        SMemCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~SMemCommand() {}
        virtual const char* GetString() const { return "smem"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: smem [options]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] =
            {
                {'a', "add",        OPTARG_NONE},
				{'b', "backup",		OPTARG_NONE},
                {'g', "get",        OPTARG_NONE},
                {'i', "init",       OPTARG_NONE},
				{'p', "print",      OPTARG_NONE},
                {'s', "set",        OPTARG_NONE},
                {'S', "stats",      OPTARG_NONE},
                {'t', "timers",     OPTARG_NONE},
                {'v', "viz",        OPTARG_NONE},
                {0, 0, OPTARG_NONE} // null
            };

            char option = 0;

            for (;;)
            {
                if ( !opt.ProcessOptions( argv, optionsData ) )
                    return false;

                if (opt.GetOption() == -1) break;

                if (option != 0)
                    return cli.SetError( "smem takes only one option at a time." );

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
                if (!opt.CheckNumNonOptArgs(1, 1)) return false;

                return cli.DoSMem( option, &( argv[2] ) );

			case 'b':
                // case: backup requires one non-option argument
                if (!opt.CheckNumNonOptArgs(1, 1)) return false;

                return cli.DoSMem( option, &( argv[2] ) );

            case 'g':
                {
                    // case: get requires one non-option argument
                    if (!opt.CheckNumNonOptArgs(1, 1)) return false;

                    return cli.DoSMem( option, &( argv[2] ) );
                }

            case 'i':
                // case: init takes no arguments
                if (!opt.CheckNumNonOptArgs(0, 0)) return false;

                return cli.DoSMem( option );

			case 'p':
                {
                    // case: print does zero or 1/2 non-option arguments
                    if (!opt.CheckNumNonOptArgs(0, 2)) return false;

                    if ( opt.GetNonOptionArguments() == 0 )
                        return cli.DoSMem( option );

                    if (opt.GetNonOptionArguments() == 1)
                        return cli.DoSMem( option, &(argv[2]), 0 );

                    return cli.DoSMem( option, &(argv[2]), &(argv[3]) );
                }

            case 's':
                {
                    // case: set requires two non-option arguments
                    if (!opt.CheckNumNonOptArgs(2, 2)) return false;

                    return cli.DoSMem( option, &( argv[2] ), &( argv[3] ) );
                }

            case 'S':
                {
                    // case: stat can do zero or one non-option arguments
                    if (!opt.CheckNumNonOptArgs(0, 1)) return false;

                    if ( opt.GetNonOptionArguments() == 0 )
                        return cli.DoSMem( 'S' );

                    return cli.DoSMem( option, &( argv[2] ) );
                }

            case 't':
                {
                    // case: timer can do zero or one non-option arguments
                    if (!opt.CheckNumNonOptArgs(0, 1)) return false;

                    if ( opt.GetNonOptionArguments() == 0 )
                        return cli.DoSMem( 't' );

                    return cli.DoSMem( option, &( argv[2] ) );
                }

            case 'v':
                {
                    // case: viz does zero or 1/2 non-option arguments
                    if (!opt.CheckNumNonOptArgs(0, 2)) return false;

                    if ( opt.GetNonOptionArguments() == 0 )
                        return cli.DoSMem( option );

                    if (opt.GetNonOptionArguments() == 1)
                        return cli.DoSMem( option, &(argv[2]), 0 );

                    return cli.DoSMem( option, &(argv[2]), &(argv[3]) );
                }
            }

            // bad: no option, but more than one argument
            if ( argv.size() > 1 ) 
                return cli.SetError( "Too many arguments." );

            // case: nothing = full configuration information
            return cli.DoSMem();
        }

    private:
        cli::Cli& cli;

        SMemCommand& operator=(const SMemCommand&);
    };

    class SoarNewsCommand : public cli::ParserCommand
    {
    public:
        SoarNewsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~SoarNewsCommand() {}
        virtual const char* GetString() const { return "soarnews"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: soarnews";
        }

        virtual bool Parse(std::vector< std::string >&)
        {
            return cli.DoSoarNews();
        }

    private:
        cli::Cli& cli;

        SoarNewsCommand& operator=(const SoarNewsCommand&);
    };

    class SourceCommand : public cli::ParserCommand
    {
    public:
        SourceCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~SourceCommand() {}
        virtual const char* GetString() const { return "source"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: source [options] filename";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'a', "all",            OPTARG_NONE},
                {'d', "disable",        OPTARG_NONE},
                {'v', "verbose",        OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            Cli::SourceBitset options(0);

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                case 'd':
                    options.set(Cli::SOURCE_DISABLE);
                    break;
                case 'a':
                    options.set(Cli::SOURCE_ALL);
                    break;
                case 'v':
                    options.set(Cli::SOURCE_VERBOSE);
                    break;
                }
            }

            if (opt.GetNonOptionArguments() < 1) 
                return cli.SetError(GetSyntax());
            else if (opt.GetNonOptionArguments() > 2) 
                return cli.SetError("Please supply one file to source. If there are spaces in the path, enclose it in quotes.");

            return cli.DoSource(argv[opt.GetArgument() - opt.GetNonOptionArguments()], &options);
        }

    private:
        cli::Cli& cli;

        SourceCommand& operator=(const SourceCommand&);
    };

    class SPCommand : public cli::ParserCommand
    {
    public:
        SPCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~SPCommand() {}
        virtual const char* GetString() const { return "sp"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: sp {production_body}";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // One argument (the stuff in the brackets, minus the brackets
            if (argv.size() < 2) 
                return cli.SetError(GetSyntax());
            if (argv.size() > 2) 
                return cli.SetError(GetSyntax());

            return cli.DoSP(argv[1]);
        }

    private:
        cli::Cli& cli;

        SPCommand& operator=(const SPCommand&);
    };

    class SRandCommand : public cli::ParserCommand
    {
    public:
        SRandCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~SRandCommand() {}
        virtual const char* GetString() const { return "srand"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: srand [seed]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            if (argv.size() < 2) return cli.DoSRand();

            if (argv.size() > 2) return cli.SetError(GetSyntax());

            uint32_t seed = 0;
            sscanf(argv[1].c_str(), "%u", &seed);
            return cli.DoSRand(&seed);
        }

    private:
        cli::Cli& cli;

        SRandCommand& operator=(const SRandCommand&);
    };

    class StatsCommand : public cli::ParserCommand
    {
    public:
        StatsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~StatsCommand() {}
        virtual const char* GetString() const { return "stats"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: stats [options]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
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
                {0, 0, OPTARG_NONE}
            };

            Cli::StatsBitset options(0);
            int sort = 0;

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

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
                        if (!from_string(sort, opt.GetOptionArgument())) {
                            return cli.SetError("Integer expected");
                        }
                        break;
                }
            }

            // No arguments
            if (opt.GetNonOptionArguments()) 
                return cli.SetError(GetSyntax());

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
        virtual const char* GetString() const { return "stop-soar"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: stop-soar [-s] [reason string]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
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
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption())
                {
                    case 's':
                        self = true;
                        break;
                }
            }

            // Concatinate remaining args for 'reason'
            if (opt.GetNonOptionArguments()) {
                std::string reasonForStopping;
                unsigned int optind = opt.GetArgument() - opt.GetNonOptionArguments();
                while (optind < argv.size()) reasonForStopping += argv[optind++] + ' ';
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
        virtual const char* GetString() const { return "time"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: time command [arguments]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // There must at least be a command
            if (argv.size() < 2) 
                return cli.SetError(GetSyntax());

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
        virtual const char* GetString() const { return "timers"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: timers [options]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
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
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

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
                return cli.SetError(GetSyntax());

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
        virtual const char* GetString() const { return "unalias"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: unalias name";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            // Need exactly one argument
            if (argv.size() < 2) 
                return cli.SetError(GetSyntax());
            if (argv.size() > 2)
                return cli.SetError(GetSyntax());

            argv.erase(argv.begin());
            return cli.DoUnalias(argv);
        }

    private:
        cli::Cli& cli;

        UnaliasCommand& operator=(const UnaliasCommand&);
    };

    class VerboseCommand : public cli::ParserCommand
    {
    public:
        VerboseCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~VerboseCommand() {}
        virtual const char* GetString() const { return "verbose"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: verbose [-ed]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] =
            {
                {'d', "disable",    OPTARG_NONE},
                {'e', "enable",        OPTARG_NONE},
                {'d', "off",        OPTARG_NONE},
                {'e', "onn",        OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            bool setting = false;
            bool query = true;

            for (;;) {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption())
                {
                    case 'd':
                        setting = false;
                        query = false;
                        break;
                    case 'e':
                        setting = true;
                        query = false;
                        break;
                }
            }

            if (opt.GetNonOptionArguments()) 
                return cli.SetError(GetSyntax());

            return cli.DoVerbose(query ? 0 : &setting);
        }

    private:
        cli::Cli& cli;

        VerboseCommand& operator=(const VerboseCommand&);
    };

    class VersionCommand : public cli::ParserCommand
    {
    public:
        VersionCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~VersionCommand() {}
        virtual const char* GetString() const { return "version"; }
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

    class WaitSNCCommand : public cli::ParserCommand
    {
    public:
        WaitSNCCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~WaitSNCCommand() {}
        virtual const char* GetString() const { return "waitsnc"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: waitsnc -[e|d]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'d', "disable",    OPTARG_NONE},
                {'e', "enable",        OPTARG_NONE},
                {'d', "off",        OPTARG_NONE},
                {'e', "on",            OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            bool query = true;
            bool enable = false;

            for (;;)
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption())
                {
                    case 'd':
                        query = false;
                        enable = false;
                        break;
                    case 'e':
                        query = false;
                        enable = true;
                        break;
                }
            }

            // No additional arguments
            if (opt.GetNonOptionArguments()) 
                return cli.SetError(GetSyntax());        

            return cli.DoWaitSNC(query ? 0 : &enable);
        }

    private:
        cli::Cli& cli;

        WaitSNCCommand& operator=(const WaitSNCCommand&);
    };

    class WarningsCommand : public cli::ParserCommand
    {
    public:
        WarningsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~WarningsCommand() {}
        virtual const char* GetString() const { return "warnings"; }
        virtual const char* GetSyntax() const 
        {
            return 
                "Syntax: warnings [options]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'e', "enable",        OPTARG_NONE},
                {'d', "disable",    OPTARG_NONE},
                {'e', "on",            OPTARG_NONE},
                {'d', "off",        OPTARG_NONE},
                {0, 0, OPTARG_NONE}
            };

            bool query = true;
            bool setting = true;

            for (;;)
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'e':
                        setting = true;
                        query = false;
                        break;
                    case 'd':
                        setting = false;
                        query = false;
                        break;
                }
            }

            if (opt.GetNonOptionArguments()) 
                cli.SetError(GetSyntax());

            return cli.DoWarnings(query ? 0 : &setting);
        }

    private:
        cli::Cli& cli;

        WarningsCommand& operator=(const WarningsCommand&);
    };

    class WatchCommand : public cli::ParserCommand
    {
    public:
        WatchCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~WatchCommand() {}
        virtual const char* GetString() const { return "watch"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: watch [options]\nwatch [level]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'b',"backtracing",                OPTARG_OPTIONAL},
                {'c',"chunks",                    OPTARG_OPTIONAL},
                {'d',"decisions",                OPTARG_OPTIONAL},
                {'D',"default-productions",        OPTARG_OPTIONAL},
                {'e',"epmem",                    OPTARG_OPTIONAL},
                {'f',"fullwmes",                OPTARG_NONE},
                {'g',"gds",                        OPTARG_OPTIONAL},
                {'i',"indifferent-selection",    OPTARG_OPTIONAL},
                {'j',"justifications",            OPTARG_OPTIONAL},
                {'L',"learning",                OPTARG_REQUIRED},
                {'l',"level",                    OPTARG_REQUIRED},
                {'N',"none",                    OPTARG_NONE},
                {'n',"nowmes",                    OPTARG_NONE},
                {'p',"phases",                    OPTARG_OPTIONAL},
                {'P',"productions",                OPTARG_OPTIONAL},
                {'r',"preferences",                OPTARG_OPTIONAL},
                {'R',"rl",                        OPTARG_OPTIONAL},
                {'s',"smem",                    OPTARG_OPTIONAL},
                {'t',"timetags",                OPTARG_NONE},
                {'T',"template",                OPTARG_OPTIONAL},
                {'u',"user-productions",        OPTARG_OPTIONAL},
                {'w',"wmes",                    OPTARG_OPTIONAL},
                {'W',"waterfall",                OPTARG_OPTIONAL}, // TODO: document. note: added to watch 5
                {0, 0, OPTARG_NONE}
            };
                     
            Cli::WatchBitset options(0);
            Cli::WatchBitset settings(0);
            int learnSetting = 0;
            int wmeSetting = 0;

            for (;;)
            {
                if (!opt.ProcessOptions(argv, optionsData))
                    return cli.SetError(opt.GetError());

                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) {
                    case 'b':
                        options.set(Cli::WATCH_BACKTRACING);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_BACKTRACING);
                        } 
                        else 
                            settings.set(Cli::WATCH_BACKTRACING);
                        break;

                    case 'c':
                        options.set(Cli::WATCH_CHUNKS);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_CHUNKS);
                        } 
                        else 
                            settings.set(Cli::WATCH_CHUNKS);
                        break;

                    case 'd':
                        options.set(Cli::WATCH_DECISIONS);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_DECISIONS);
                        } 
                        else 
                            settings.set(Cli::WATCH_DECISIONS);
                        break;

                    case 'D':
                        options.set(Cli::WATCH_DEFAULT);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_DEFAULT);
                        } 
                        else 
                            settings.set(Cli::WATCH_DEFAULT);
                        break;
                        
                    case 'e':
                        options.set(Cli::WATCH_EPMEM);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_EPMEM);
                        } 
                        else 
                            settings.set(Cli::WATCH_EPMEM);
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
                                return false;
                            settings.reset(Cli::WATCH_GDS);
                        } 
                        else 
                            settings.set(Cli::WATCH_GDS);
                        break;

                    case 'i':
                        options.set(Cli::WATCH_INDIFFERENT);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_INDIFFERENT);
                        } 
                        else
                            settings.set(Cli::WATCH_INDIFFERENT);
                        break;

                    case 'j':
                        options.set(Cli::WATCH_JUSTIFICATIONS);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_JUSTIFICATIONS);
                        } 
                        else 
                            settings.set(Cli::WATCH_JUSTIFICATIONS);
                        break;

                    case 'L':
                        options.set(Cli::WATCH_LEARNING);
                        learnSetting = ParseLearningOptarg(opt);
                        if (learnSetting == -1) return false; 

                    case 'l':
                        {
                            int level = 0;
                            if (!from_string( level, opt.GetOptionArgument())) 
                                return cli.SetError("Integer argument expected.");

                            if (!ProcessWatchLevelSettings(level, options, settings, wmeSetting, learnSetting)) 
                                return false; 
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
                        if (opt.GetOptionArgument().size()) {
                            if (!CheckOptargRemoveOrZero(opt))
                                return false; 
                            settings.reset(Cli::WATCH_PHASES);
                        }
                        else 
                            settings.set(Cli::WATCH_PHASES);
                        break;

                    case 'P': // productions (all)
                        options.set(Cli::WATCH_DEFAULT);
                        options.set(Cli::WATCH_USER);
                        options.set(Cli::WATCH_CHUNKS);
                        options.set(Cli::WATCH_JUSTIFICATIONS);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false;
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
                                return false; 
                            settings.reset(Cli::WATCH_PREFERENCES);
                        } 
                        else 
                            settings.set(Cli::WATCH_PREFERENCES);
                        break;

                    case 'R':
                        options.set(Cli::WATCH_RL);
                        if (opt.GetOptionArgument().size())
                        {
                            if (!CheckOptargRemoveOrZero(opt))
                                return false; 
                            settings.reset(Cli::WATCH_RL);
                        } 
                        else 
                            settings.set(Cli::WATCH_RL);
                        break;

                    case 's':
                        options.set(Cli::WATCH_SMEM);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_SMEM);
                        } 
                        else 
                            settings.set(Cli::WATCH_SMEM);
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
                                return false; 
                            settings.reset(Cli::WATCH_TEMPLATES);
                        }
                        else 
                            settings.set(Cli::WATCH_TEMPLATES);
                        break;

                    case 'u':
                        options.set(Cli::WATCH_USER);
                        if (opt.GetOptionArgument().size())
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_USER);
                        } 
                        else
                            settings.set(Cli::WATCH_USER);
                        break;
                    case 'w'://wmes
                        options.set(Cli::WATCH_WMES);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt))
                                return false; 
                            settings.reset(Cli::WATCH_WMES);
                        }
                        else
                            settings.set(Cli::WATCH_WMES);
                        break;
                    case 'W'://waterfall
                        options.set(Cli::WATCH_WATERFALL);
                        if (opt.GetOptionArgument().size()) 
                        {
                            if (!CheckOptargRemoveOrZero(opt)) 
                                return false; 
                            settings.reset(Cli::WATCH_WATERFALL);
                        }
                        else 
                            settings.set(Cli::WATCH_WATERFALL);
                        break;
                }
            }

            if (opt.GetNonOptionArguments() > 1)
                return cli.SetError("Only non option argument allowed is watch level.");

            // Allow watch level by itself
            if (opt.GetNonOptionArguments() == 1) {
                int optind = opt.GetArgument() - opt.GetNonOptionArguments();
                int level = 0;
                if (!from_string(level, argv[optind])) return cli.SetError("Integer argument expected.");
                if (!ProcessWatchLevelSettings(level, options, settings, wmeSetting, learnSetting)) 
                    return false; 
            }

            return cli.DoWatch(options, settings, wmeSetting, learnSetting);
        }

    private:
        cli::Cli& cli;

        bool ProcessWatchLevelSettings(const int level, Cli::WatchBitset& options, Cli::WatchBitset& settings, int& wmeSetting, int& learnSetting)
        {
            if (level < 0)  
                return cli.SetError("Expected watch level from 0 to 5.");

            if (level > 5) 
                return cli.SetError("Expected watch level from 0 to 5.");

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
                return 0;
            if (opt.GetOptionArgument() == "print"     || opt.GetOptionArgument() == "1") 
                return 1;
            if (opt.GetOptionArgument() == "fullprint" || opt.GetOptionArgument() == "2") 
                return 2;

            cli.SetError("Invalid learn setting, expected noprint, print, fullprint, or 0-2. Got: " + opt.GetOptionArgument());
            return -1;
        }

        bool CheckOptargRemoveOrZero(cli::Options& opt) 
        {
            if (opt.GetOptionArgument() == "remove" || opt.GetOptionArgument() == "0") 
                return true;

            return cli.SetError("Invalid argument, expected remove or 0. Got: " + opt.GetOptionArgument());
        }

        WatchCommand& operator=(const WatchCommand&);
    };

    class WatchWMEsCommand : public cli::ParserCommand
    {
    public:
        WatchWMEsCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~WatchWMEsCommand() {}
        virtual const char* GetString() const { return "watch-wmes"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: watch-wmes -[a|r]  -t type  pattern\nwatch-wmes -[l|R] [-t type]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] =
            {
                {'a', "add-filter",        OPTARG_NONE},
                {'r', "remove-filter",    OPTARG_NONE},
                {'l', "list-filter",    OPTARG_NONE},
                {'R', "reset-filter",    OPTARG_NONE},
                {'t', "type",            OPTARG_REQUIRED},
                {0, 0, OPTARG_NONE}
            };

            Cli::eWatchWMEsMode mode = Cli::WATCH_WMES_LIST;
            Cli::WatchWMEsTypeBitset type(0);

            for (;;) 
            {
                if (!opt.ProcessOptions(argv, optionsData)) return false;
                if (opt.GetOption() == -1) break;

                switch (opt.GetOption()) 
                {
                    case 'a':
                        mode = Cli::WATCH_WMES_ADD;
                        break;
                    case 'r':
                        mode = Cli::WATCH_WMES_REMOVE;
                        break;
                    case 'l':
                        mode = Cli::WATCH_WMES_LIST;
                        break;
                    case 'R':
                        mode = Cli::WATCH_WMES_RESET;
                        break;
                    case 't':
                        {
                            std::string typeString = opt.GetOptionArgument();
                            if (typeString == "adds") {
                                type.set(Cli::WATCH_WMES_TYPE_ADDS);
                            } else if (typeString == "removes") {
                                type.set(Cli::WATCH_WMES_TYPE_REMOVES);
                            } else if (typeString == "both") {
                                type.set(Cli::WATCH_WMES_TYPE_ADDS);
                                type.set(Cli::WATCH_WMES_TYPE_REMOVES);
                            } else {
                                return cli.SetError("Invalid wme filter type, got: " + typeString);
                            }
                        }
                        break;
                }
            }
            
            if (mode == Cli::WATCH_WMES_ADD || mode == Cli::WATCH_WMES_REMOVE) 
            {
                // type required
                if (type.none()) 
                    return cli.SetError("Wme type required.");
            
                // check for too few/many args
                if (opt.GetNonOptionArguments() > 3) 
                    return cli.SetError(GetSyntax());
                if (opt.GetNonOptionArguments() < 3)
                    return cli.SetError(GetSyntax());

                int optind = opt.GetArgument() - opt.GetNonOptionArguments();
                return cli.DoWatchWMEs(mode, type, &argv[optind], &argv[optind + 1], &argv[optind + 2]);
            }

            // no additional arguments
            if (opt.GetNonOptionArguments())
                return cli.SetError(GetSyntax());

            return cli.DoWatchWMEs(mode, type);
        }

    private:
        cli::Cli& cli;

        WatchWMEsCommand& operator=(const WatchWMEsCommand&);
    };

    class WMACommand : public cli::ParserCommand
    {
    public:
        WMACommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
        virtual ~WMACommand() {}
        virtual const char* GetString() const { return "wma"; }
        virtual const char* GetSyntax() const 
        {
            return "Syntax: wma [options]";
        }

        virtual bool Parse(std::vector< std::string >&argv)
        {
            cli::Options opt;
            OptionsData optionsData[] = 
            {
                {'g', "get",		OPTARG_NONE},
				{'h', "history",	OPTARG_NONE},
                {'s', "set",		OPTARG_NONE},
                {'S', "stats",		OPTARG_NONE},
				{'t', "timers",		OPTARG_NONE},
                {0, 0, OPTARG_NONE} // null
            };

            char option = 0;

            for (;;) 
            {
                if ( !opt.ProcessOptions( argv, optionsData ) ) 
                    return false;

                if (opt.GetOption() == -1) break;

                if (option != 0)
                    return cli.SetError( "wma takes only one option at a time." );

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
                    if (!opt.CheckNumNonOptArgs(1, 1)) return false;

                    return cli.DoWMA( option, &( argv[2] ) );
                }

			case 'h':
				// case: history requires one non-option argument
				{
					if (!opt.CheckNumNonOptArgs(1, 1)) return false;

                    return cli.DoWMA( option, &( argv[2] ) );
				}

            case 's':
                // case: set requires two non-option arguments
                {
                    if (!opt.CheckNumNonOptArgs(2, 2)) return false;

                    return cli.DoWMA( option, &( argv[2] ), &( argv[3] ) );
                }

            case 'S':
                // case: stat can do zero or one non-option arguments
                {
                    if (!opt.CheckNumNonOptArgs(0, 1)) return false;

                    if ( opt.GetNonOptionArguments() == 0 )
                        return cli.DoWMA( 'S' );

                    return cli.DoWMA( option, &( argv[2] ) );
                }

			case 't':
                // case: timer can do zero or one non-option arguments
                {
                    if (!opt.CheckNumNonOptArgs(0, 1)) return false;

                    if ( opt.GetNonOptionArguments() == 0 )
                        return cli.DoWMA( option );

                    return cli.DoWMA( option, &( argv[2] ) );
                }
            }

            // bad: no option, but more than one argument
            if ( argv.size() > 1 ) 
                return cli.SetError( "Too many args." );

            // case: nothing = full configuration information
            return cli.DoWMA();    
        }

    private:
        cli::Cli& cli;

        WMACommand& operator=(const WMACommand&);
    };

}

#endif // CLI_COMMANDS_H
