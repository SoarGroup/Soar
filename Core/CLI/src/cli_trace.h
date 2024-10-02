/*
 * cli_trace.h
 *
 *  Created on: Nov 25, 2016
 *      Author: mazzin
 */

#ifndef CORE_CLI_SRC_CLI_TRACE_H_
#define CORE_CLI_SRC_CLI_TRACE_H_

#include "cli_Parser.h"
#include "cli_Options.h"

#include "cli_CommandLineInterface.h"

#include "misc.h"
#include "sml_Events.h"

namespace cli
{

    class TraceCommand : public cli::ParserCommand
    {
        public:
            TraceCommand(cli::CommandLineInterface& cli) : ParserCommand(),  cli(cli) {}
            virtual ~TraceCommand() {}
            virtual const char* GetString() const
            {
                return "trace";
            }
            virtual const char* GetSyntax() const
            {
                return "Syntax: trace [options]";
            }

            virtual bool Parse(std::vector< std::string >& argv)
            {
                cli::Options opt;
                bool fromWatch = false;

                OptionsData optionsData[] =
                {
                    {'a', "wma",                         OPTARG_OPTIONAL},
                    {'A', "assertions",                  OPTARG_OPTIONAL},
                    {'b', "backtracing",                 OPTARG_OPTIONAL},
                    {'c', "chunks",                      OPTARG_OPTIONAL},
                    {'C', "chunk-warnings",              OPTARG_OPTIONAL},
                    {'d', "decisions",                   OPTARG_OPTIONAL},
                    {'D', "default-productions",         OPTARG_OPTIONAL},
                    {'e', "epmem",                       OPTARG_OPTIONAL},
                    {'f', "fullwmes",                    OPTARG_NONE},
                    {'g', "gds",                         OPTARG_OPTIONAL},
                    {'G', "gds-wmes",                    OPTARG_OPTIONAL},
                    {'i', "indifferent-selection",       OPTARG_OPTIONAL},
                    {'j', "justifications",              OPTARG_OPTIONAL},
                    {'L', "learning",                    OPTARG_REQUIRED},
                    {'l', "level",                       OPTARG_REQUIRED},
                    {'N', "none",                        OPTARG_NONE},
                    {'n', "nowmes",                      OPTARG_NONE},
                    {'o', "consistency",                 OPTARG_OPTIONAL},
                    {'p', "phases",                      OPTARG_OPTIONAL},
                    {'P', "productions",                 OPTARG_OPTIONAL},
                    {'r', "preferences",                 OPTARG_OPTIONAL},
                    {'R', "rl",                          OPTARG_OPTIONAL},
                    {'s', "smem",                        OPTARG_OPTIONAL},
                    {'t', "timetags",                    OPTARG_NONE},
                    {'T', "templates",                   OPTARG_OPTIONAL},
                    {'u', "user-productions",            OPTARG_OPTIONAL},
                    {'w', "wmes",                        OPTARG_OPTIONAL},
                    {'W', "waterfall",                   OPTARG_OPTIONAL},
                    {0, 0, OPTARG_NONE}
                };

                cli::WatchBitset options(0);
                cli::WatchBitset settings(0);
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
                            options.set(cli::WATCH_WMA);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_WMA);
                            }
                            else
                            {
                                settings.set(cli::WATCH_WMA);
                            }
                            break;

                        case 'A':
                            options.set(cli::WATCH_ASSERTIONS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_ASSERTIONS);
                            }
                            else
                            {
                                settings.set(cli::WATCH_ASSERTIONS);
                            }
                            break;

                        case 'b':
                            options.set(cli::WATCH_BACKTRACING);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_BACKTRACING);
                            }
                            else
                            {
                                settings.set(cli::WATCH_BACKTRACING);
                            }
                            break;

                        case 'c':
                            options.set(cli::WATCH_CHUNKS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_CHUNKS);
                            }
                            else
                            {
                                settings.set(cli::WATCH_CHUNKS);
                            }
                            break;

                        case 'C':
                            options.set(cli::WATCH_CHUNK_WARNINGS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_CHUNK_WARNINGS);
                            }
                            else
                            {
                                settings.set(cli::WATCH_CHUNK_WARNINGS);
                            }
                            break;

                        case 'd':
                            options.set(cli::WATCH_DECISIONS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_DECISIONS);
                            }
                            else
                            {
                                settings.set(cli::WATCH_DECISIONS);
                            }
                            break;

                        case 'D':
                            options.set(cli::WATCH_DEFAULT);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_DEFAULT);
                            }
                            else
                            {
                                settings.set(cli::WATCH_DEFAULT);
                            }
                            break;

                        case 'e':
                            options.set(cli::WATCH_EPMEM);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_EPMEM);
                            }
                            else
                            {
                                settings.set(cli::WATCH_EPMEM);
                            }
                            break;

                        case 'f': // fullwmes
                            options.set(cli::WATCH_WME_DETAIL);
                            wmeSetting = 2;
                            break;

                        case 'g':
                            options.set(cli::WATCH_GDS_STATE_REMOVAL);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_GDS_STATE_REMOVAL);
                            }
                            else
                            {
                                settings.set(cli::WATCH_GDS_STATE_REMOVAL);
                            }
                            break;

                        case 'G':
                            options.set(cli::WATCH_GDS_WMES);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_GDS_WMES);
                            }
                            else
                            {
                                settings.set(cli::WATCH_GDS_WMES);
                            }
                            break;

                        case 'i':
                            options.set(cli::WATCH_INDIFFERENT);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_INDIFFERENT);
                            }
                            else
                            {
                                settings.set(cli::WATCH_INDIFFERENT);
                            }
                            break;

                        case 'j':
                            options.set(cli::WATCH_JUSTIFICATIONS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_JUSTIFICATIONS);
                            }
                            else
                            {
                                settings.set(cli::WATCH_JUSTIFICATIONS);
                            }
                            break;

                        case 'L':
                            options.set(cli::WATCH_LEARNING);
                            learnSetting = ParseLearningOptarg(opt);
                            if (learnSetting == -1)
                            {
                                return cli.SetError(opt.GetError().c_str());
                            }
                            break;
                        case 'l':
                        {
                            int level = 0;
                            if (!from_string(level, opt.GetOptionArgument())) return cli.SetError(GetSyntax());

                            if (!ProcessWatchLevelSettings(level, options, settings, wmeSetting, learnSetting))
                            {
                                return cli.SetError(opt.GetError().c_str());
                            }
                            fromWatch = true;
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
                            options.set(cli::WATCH_WME_DETAIL);
                            wmeSetting = 0;
                            break;

                        case 'o':
                            options.set(cli::WATCH_CONSISTENCY);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_CONSISTENCY);
                            }
                            else
                            {
                                settings.set(cli::WATCH_CONSISTENCY);
                            }
                            break;

                        case 'p':
                            options.set(cli::WATCH_PHASES);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_PHASES);
                            }
                            else
                            {
                                settings.set(cli::WATCH_PHASES);
                            }
                            break;

                        case 'P': // productions (all)
                            options.set(cli::WATCH_DEFAULT);
                            options.set(cli::WATCH_USER);
                            options.set(cli::WATCH_CHUNKS);
                            options.set(cli::WATCH_JUSTIFICATIONS);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_DEFAULT);
                                settings.reset(cli::WATCH_USER);
                                settings.reset(cli::WATCH_CHUNKS);
                                settings.reset(cli::WATCH_JUSTIFICATIONS);
                            }
                            else
                            {
                                settings.set(cli::WATCH_DEFAULT);
                                settings.set(cli::WATCH_USER);
                                settings.set(cli::WATCH_CHUNKS);
                                settings.set(cli::WATCH_JUSTIFICATIONS);
                            }
                            break;

                        case 'r':
                            options.set(cli::WATCH_PREFERENCES);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_PREFERENCES);
                            }
                            else
                            {
                                settings.set(cli::WATCH_PREFERENCES);
                            }
                            break;

                        case 'R':
                            options.set(cli::WATCH_RL);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_RL);
                            }
                            else
                            {
                                settings.set(cli::WATCH_RL);
                            }
                            break;

                        case 's':
                            options.set(cli::WATCH_SMEM);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_SMEM);
                            }
                            else
                            {
                                settings.set(cli::WATCH_SMEM);
                            }
                            break;

                        case 't'://timetags
                            options.set(cli::WATCH_WME_DETAIL);
                            wmeSetting = 1;
                            break;

                        case 'T':
                            options.set(cli::WATCH_TEMPLATES);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_TEMPLATES);
                            }
                            else
                            {
                                settings.set(cli::WATCH_TEMPLATES);
                            }
                            break;

                        case 'u':
                            options.set(cli::WATCH_USER);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_USER);
                            }
                            else
                            {
                                settings.set(cli::WATCH_USER);
                            }
                            break;
                        case 'w'://wmes
                            options.set(cli::WATCH_WMES);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_WMES);
                            }
                            else
                            {
                                settings.set(cli::WATCH_WMES);
                            }
                            break;
                        case 'W'://waterfall
                            options.set(cli::WATCH_WATERFALL);
                            if (opt.GetOptionArgument().size())
                            {
                                if (!CheckOptargRemoveOrZero(opt)) return cli.SetError(opt.GetError().c_str());
                                settings.reset(cli::WATCH_WATERFALL);
                            }
                            else
                            {
                                settings.set(cli::WATCH_WATERFALL);
                            }
                            break;
                    }
                }

                if (opt.GetNonOptionArguments() > 1)
                {
                    return cli.SetError("Only non option argument allowed is watch level.");
                }

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
                    fromWatch = true;
                }

                return cli.DoTrace(options, settings, wmeSetting, learnSetting, fromWatch);
            }

        private:
            cli::CommandLineInterface& cli;

            bool ProcessWatchLevelSettings(const int level, cli::WatchBitset& options, cli::WatchBitset& settings, int& wmeSetting, int& learnSetting)
            {
                if (level < 0)
                {
                    return cli.SetError("Expected trace level from 0 to 5.");
                }

                if (level > 5)
                {
                    return cli.SetError("Expected trace level from 0 to 5.");
                }

                // All of these are going to change
                options.set(cli::WATCH_PREFERENCES);
                options.set(cli::WATCH_WMES);
                options.set(cli::WATCH_DEFAULT);
                options.set(cli::WATCH_USER);
                options.set(cli::WATCH_CHUNKS);
                options.set(cli::WATCH_CHUNK_WARNINGS);
                options.set(cli::WATCH_JUSTIFICATIONS);
                options.set(cli::WATCH_TEMPLATES);
                options.set(cli::WATCH_PHASES);
                options.set(cli::WATCH_DECISIONS);
                options.set(cli::WATCH_WATERFALL);
                options.set(cli::WATCH_GDS_STATE_REMOVAL);
                options.set(cli::WATCH_CONSISTENCY);

                // Start with all off, turn on as appropriate
                settings.reset(cli::WATCH_PREFERENCES);
                settings.reset(cli::WATCH_WMES);
                settings.reset(cli::WATCH_DEFAULT);
                settings.reset(cli::WATCH_USER);
                settings.reset(cli::WATCH_CHUNKS);
                settings.reset(cli::WATCH_CHUNK_WARNINGS);
                settings.reset(cli::WATCH_JUSTIFICATIONS);
                settings.reset(cli::WATCH_TEMPLATES);
                settings.reset(cli::WATCH_PHASES);
                settings.reset(cli::WATCH_DECISIONS);
                settings.reset(cli::WATCH_WATERFALL);
                settings.reset(cli::WATCH_GDS_STATE_REMOVAL);
                settings.reset(cli::WATCH_CONSISTENCY);

                switch (level)
                {
                    case 0:// none
                    options.reset();
                    options.flip();
                    settings.reset();
                    learnSetting = 0;
                    wmeSetting = 0;
                    cli.PrintCLIMessage("Trace level 0 enabled:  All trace messages disabled.");
                    break;
                    case 5:// preferences, waterfall, gds wme additions
                        cli.PrintCLIMessage("Trace level 5 enabled: Preferences");
                        settings.set(cli::WATCH_PREFERENCES);
                        // falls through
                    case 4:
                        cli.PrintCLIMessage("Trace level 4 enabled:  Working memory element additions and removals");
                        settings.set(cli::WATCH_WMES);
                        // falls through
                    case 3:// productions (default, user, chunks, justifications, templates)
                        cli.PrintCLIMessage("Trace level 3 enabled:  All rule firings");
                        settings.set(cli::WATCH_DEFAULT);
                        settings.set(cli::WATCH_USER);
                        settings.set(cli::WATCH_CHUNKS);
                        settings.set(cli::WATCH_JUSTIFICATIONS);
                        settings.set(cli::WATCH_TEMPLATES);
                        settings.set(cli::WATCH_WATERFALL);
                        // falls through
                    case 2:// phases, gds
                        cli.PrintCLIMessage("Trace level 2 enabled:  All phases, state removals caused by operator selection\n"
                                            "                        or a GDS violation, and any learning issues detected");
                        settings.set(cli::WATCH_PHASES);
                        settings.set(cli::WATCH_CONSISTENCY);
                        settings.set(cli::WATCH_GDS_STATE_REMOVAL);
                        settings.set(cli::WATCH_CHUNK_WARNINGS);

                        // falls through
                    case 1:// decisions
                        cli.PrintCLIMessage("Trace level 1 enabled:  Decision cycles, state creation and operator selection");
                        settings.set(cli::WATCH_DECISIONS);
                        break;
                }
                cli.PrintCLIMessage("\nFor a full list of trace options, use 'trace' (no arguments)");
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

            TraceCommand& operator=(const TraceCommand&);
    };
}
#endif /* CORE_CLI_SRC_CLI_TRACE_H_ */
