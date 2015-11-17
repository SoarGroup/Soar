/////////////////////////////////////////////////////////////////
// firingcounts command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"

#include <algorithm>

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"

using namespace cli;
using namespace sml;

struct FiringsSort
{
    bool operator()(std::pair< std::string, uint64_t > a, std::pair< std::string, uint64_t > b) const
    {
        return a.second < b.second;
    }
};

void  add_prods_of_type_to_fc_list(agent* thisAgent,
                                   unsigned int productionType, bool printThisType, bool printRL,
                                   const int numberToList,
                                   std::vector< std::pair< std::string, uint64_t > > *firings)
{
    if (printThisType || printRL)
    {
        for (production* pSoarProduction = thisAgent->all_productions_of_type[productionType];
            pSoarProduction != 0;
            pSoarProduction = pSoarProduction->next)
        {
            if (!numberToList)
            {
                if (pSoarProduction->firing_count)
                {
                    // this one has fired, skip it
                    continue;
                }
            }

            if (printThisType || (printRL && pSoarProduction->rl_rule))
            {
                if (!printRL && pSoarProduction->rl_rule)
                {
                    continue;
                }
                // store the name and count
                std::pair< std::string, uint64_t > firing;
                firing.first = pSoarProduction->name->sc->name;
                firing.second = pSoarProduction->firing_count;
                firings->push_back(firing);
            }
        }
    }
}

bool CommandLineInterface::DoFiringCounts(PrintBitset options, const int numberToList, const std::string* pProduction)
{
    std::vector< std::pair< std::string, uint64_t > > firings;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    // if we have a production, just get that one, otherwise get them all
    if (pProduction && !pProduction->empty())
    {
        Symbol* sym = find_str_constant(thisAgent, pProduction->c_str());

        if (!sym || !(sym->sc->production))
        {
            return SetError("Production not found.");
        }

        std::pair< std::string, uint64_t > firing;
        firing.first = *pProduction;
        firing.second = sym->sc->production->firing_count;
        firings.push_back(firing);
    }
    else
    {
        bool foundProduction = false;
        bool printRL = options.test(PRINT_RL);

        // Print all productions if there are no other production flags set
        if (options.test(PRINT_ALL) ||
                (!options.test(PRINT_CHUNKS) &&
                 !options.test(PRINT_DEFAULTS) &&
                 !options.test(PRINT_JUSTIFICATIONS) &&
                 !options.test(PRINT_USER) &&
                 !printRL &&
                 !options.test(PRINT_TEMPLATE)))
        {
            options.set(PRINT_CHUNKS);
            options.set(PRINT_DEFAULTS);
            options.set(PRINT_JUSTIFICATIONS);
            options.set(PRINT_USER);
            options.set(PRINT_TEMPLATE);
            printRL = true;
        }

        add_prods_of_type_to_fc_list(thisAgent, CHUNK_PRODUCTION_TYPE,
            options.test(PRINT_CHUNKS), printRL, numberToList, &firings);
        add_prods_of_type_to_fc_list(thisAgent, DEFAULT_PRODUCTION_TYPE,
            options.test(PRINT_DEFAULTS), printRL, numberToList, &firings);
        add_prods_of_type_to_fc_list(thisAgent, JUSTIFICATION_PRODUCTION_TYPE,
            options.test(PRINT_JUSTIFICATIONS), printRL, numberToList, &firings);
        add_prods_of_type_to_fc_list(thisAgent, USER_PRODUCTION_TYPE,
            options.test(PRINT_USER), printRL, numberToList, &firings);
        add_prods_of_type_to_fc_list(thisAgent, TEMPLATE_PRODUCTION_TYPE,
            options.test(PRINT_TEMPLATE), printRL, numberToList, &firings);

        if (firings.empty())
        {
            return SetError("No productions of that type found.");
        }
    }

    // Sort the list
    FiringsSort s;
    sort(firings.begin(), firings.end(), s);

    // print the list
    int i = 0;
    for (std::vector< std::pair< std::string, uint64_t > >::reverse_iterator j = firings.rbegin();
            j != firings.rend() && (numberToList <= 0 || i < numberToList);
            ++j, ++i)
    {
        if (m_RawOutput)
        {
            m_Result << std::setw(6) << j->second << ":  " << j->first << "\n";
        }
        else
        {
            std::string temp;
            AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, j->first);
            AppendArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, to_string(j->second, temp));
        }
    }
    return true;
}

//class PrintCommand : public cli::ParserCommand
//    {
//        public:
//            PrintCommand(cli::Cli& cli) : cli(cli), ParserCommand() {}
//            virtual ~PrintCommand() {}
//            virtual const char* GetString() const
//            {
//                return "print";
//            }
//            virtual const char* GetSyntax() const
//            {
//                return "Syntax: print [options] [production_name]\nprint [options] identifier|timetag|pattern";
//            }
//
//            virtual bool Parse(std::vector< std::string >& argv)
//            {
//                cli::Options opt;
//                OptionsData optionsData[] =
//                {
//                    {'a', "all",            OPTARG_NONE},
//                    {'c', "chunks",            OPTARG_NONE},
//                    {'d', "depth",            OPTARG_REQUIRED},
//                    {'D', "defaults",        OPTARG_NONE},
//                    {'e', "exact",            OPTARG_NONE},
//                    {'f', "full",            OPTARG_NONE},
//                    {'F', "filename",        OPTARG_NONE},
//                    {'i', "internal",        OPTARG_NONE},
//                    {'j', "justifications",    OPTARG_NONE},
//                    {'n', "name",            OPTARG_NONE},
//                    {'o', "operators",        OPTARG_NONE},
//                    {'r', "rl",                OPTARG_NONE},
//                    {'s', "stack",            OPTARG_NONE},
//                    {'S', "states",            OPTARG_NONE},
//                    {'t', "tree",            OPTARG_NONE},
//                    {'T', "template",        OPTARG_NONE},
//                    {'u', "user",            OPTARG_NONE},
//                    {'v', "varprint",        OPTARG_NONE},
//                    {0, 0, OPTARG_NONE}
//                };
//
//                int depth = -1;
//                Cli::PrintBitset options(0);
//
//                for (;;)
//                {
//                    if (!opt.ProcessOptions(argv, optionsData))
//                    {
//                        return cli.SetError(opt.GetError().c_str());
//                    }
//                    ;
//                    if (opt.GetOption() == -1)
//                    {
//                        break;
//                    }
//
//                    switch (opt.GetOption())
//                    {
//                        case 'a':
//                            options.set(Cli::PRINT_ALL);
//                            break;
//                        case 'c':
//                            options.set(Cli::PRINT_CHUNKS);
//                            break;
//                        case 'd':
//                            options.set(Cli::PRINT_DEPTH);
//                            if (!from_string(depth, opt.GetOptionArgument()) || (depth < 0))
//                            {
//                                return cli.SetError("Non-negative depth expected.");
//                            }
//                            break;
//                        case 'D':
//                            options.set(Cli::PRINT_DEFAULTS);
//                            break;
//                        case 'e':
//                            options.set(Cli::PRINT_EXACT);
//                            break;
//                        case 'f':
//                            options.set(Cli::PRINT_FULL);
//                            break;
//                        case 'F':
//                            options.set(Cli::PRINT_FILENAME);
//                            break;
//                        case 'i':
//                            options.set(Cli::PRINT_INTERNAL);
//                            break;
//                        case 'j':
//                            options.set(Cli::PRINT_JUSTIFICATIONS);
//                            break;
//                        case 'n':
//                            options.set(Cli::PRINT_NAME);
//                            break;
//                        case 'o':
//                            options.set(Cli::PRINT_OPERATORS);
//                            break;
//                        case 'r':
//                            options.set(Cli::PRINT_RL);
//                            break;
//                        case 's':
//                            options.set(Cli::PRINT_STACK);
//                            break;
//                        case 'S':
//                            options.set(Cli::PRINT_STATES);
//                            break;
//                        case 't':
//                            options.set(Cli::PRINT_TREE);
//                            break;
//                        case 'T':
//                            options.set(Cli::PRINT_TEMPLATE);
//                            break;
//                        case 'u':
//                            options.set(Cli::PRINT_USER);
//                            break;
//                        case 'v':
//                            options.set(Cli::PRINT_VARPRINT);
//                            break;
//                    }
//                }
//
//                // STATES and OPERATORS are sub-options of STACK
//                if (options.test(Cli::PRINT_OPERATORS) || options.test(Cli::PRINT_STATES))
//                {
//                    if (!options.test(Cli::PRINT_STACK))
//                    {
//                        return cli.SetError("Options --operators (-o) and --states (-S) are only valid when printing the stack.");
//                    }
//                }
//
//                if (opt.GetNonOptionArguments() == 0)
//                {
//                    // d and t options require an argument
//                    if (options.test(Cli::PRINT_TREE) || options.test(Cli::PRINT_DEPTH))
//                    {
//                        return cli.SetError(GetSyntax());
//                    }
//                    return cli.DoPrint(options, depth);
//                }
//
//                // the acDjus options don't allow an argument
//                if (options.test(Cli::PRINT_ALL)
//                        || options.test(Cli::PRINT_CHUNKS)
//                        || options.test(Cli::PRINT_DEFAULTS)
//                        || options.test(Cli::PRINT_JUSTIFICATIONS)
//                        || options.test(Cli::PRINT_RL)
//                        || options.test(Cli::PRINT_TEMPLATE)
//                        || options.test(Cli::PRINT_USER)
//                        || options.test(Cli::PRINT_STACK))
//                {
//                    return cli.SetError("No argument allowed when printing all/chunks/defaults/justifications/rl/template/user/stack.");
//                }
//                if (options.test(Cli::PRINT_EXACT) && (options.test(Cli::PRINT_DEPTH) || options.test(Cli::PRINT_TREE)))
//                {
//                    return cli.SetError("No depth/tree flags allowed when printing exact.");
//                }
//
//                std::string arg;
//                for (size_t i = opt.GetArgument() - opt.GetNonOptionArguments(); i < argv.size(); ++i)
//                {
//                    if (!arg.empty())
//                    {
//                        arg.push_back(' ');
//                    }
//                    arg.append(argv[i]);
//                }
//                return cli.DoPrint(options, depth, &arg);
//            }
//
//        private:
//            cli::Cli& cli;
//
//            PrintCommand& operator=(const PrintCommand&);
//    };
