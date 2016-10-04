/////////////////////////////////////////////////////////////////
// learn command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"
#include "cli_soar.h"

#include "sml_AgentSML.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"
#include "sml_Utils.h"

#include "agent.h"
#include "cmd_settings.h"
#include "condition.h"
#include "lexer.h"
#include "mem.h"
#include "memory_manager.h"
#include "output_manager.h"
#include "parser.h"
#include "print.h"
#include "production.h"
#include "reinforcement_learning.h"
#include "rete.h"
#include "rhs.h"
#include "run_soar.h"
#include "symbol_manager.h"
#include "symbol.h"
#include "symbol.h"
#include "test.h"

#include <algorithm>
#include <assert.h>

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoProduction(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->production_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->production_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid production command.  Use 'production ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->production_params->excise_cmd)
    {
        return ParseExcise(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->firing_counts_cmd)
    {
        return ParseFC(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->matches_cmd)
    {
        return ParseMatches(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->multi_attributes_cmd)
    {
        return ParseMultiAttributes(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->break_cmd)
    {
        return ParsePBreak(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->find_cmd)
    {
        return ParsePFind(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->watch_cmd)
    {
        return ParsePWatch(argv);
    }
    else if ((my_param == thisAgent->command_params->production_params->help_cmd) || (my_param == thisAgent->command_params->production_params->qhelp_cmd))
    {
        thisAgent->command_params->production_params->print_settings(thisAgent);
    }
    return false;
}
bool CommandLineInterface::ParseExcise(std::vector< std::string >& argv)
{
    cli::Options opt;
    OptionsData optionsData[] =
    {
        {'a', "all",            OPTARG_NONE},
        {'c', "chunks",         OPTARG_NONE},
        {'d', "default",        OPTARG_NONE},
        {'n', "never-fired",    OPTARG_NONE},
        {'r', "rl",             OPTARG_NONE},
        {'t', "task",           OPTARG_NONE},
        {'T', "template",       OPTARG_NONE},
        {'u', "user",           OPTARG_NONE},
        {0, 0,                  OPTARG_NONE}
    };

    cli::ExciseBitset options(0);

    for (;;)
    {
        if (!opt.ProcessOptions(argv, optionsData))
        {
            return SetError(opt.GetError().c_str());
        }
        ;
        if (opt.GetOption() == -1)
        {
            break;
        }

        switch (opt.GetOption())
        {
            case 'a':
                options.set(cli::EXCISE_ALL);
                break;
            case 'c':
                options.set(cli::EXCISE_CHUNKS);
                break;
            case 'd':
                options.set(cli::EXCISE_DEFAULT);
                break;
            case 'n':
                options.set(cli::EXCISE_NEVER_FIRED);
                break;
            case 'r':
                options.set(cli::EXCISE_RL);
                break;
            case 't':
                options.set(cli::EXCISE_TASK);
                break;
            case 'T':
                options.set(cli::EXCISE_TEMPLATE);
                break;
            case 'u':
                options.set(cli::EXCISE_USER);
                break;
        }
    }

    // If there are options, no additional argument.
    if (options.any())
    {
        if (!opt.CheckNumNonOptArgs(1, 1))
        {
            return SetError("Invalid additional arguments.");
        }
        return DoExcise(options);
    }

    // If there are no options, there must be only one production name argument
    if (opt.GetNonOptionArguments() < 2)
    {
        return SetError("Production name is required.");
    }
    if (opt.GetNonOptionArguments() > 2)
    {
        return SetError("Only one production name allowed, call excise multiple times to excise more than one specific production.");
    }

    // Pass the production to the DoExcise function
    return DoExcise(options, &(argv[opt.GetArgument() - opt.GetNonOptionArguments() + 1]));
}
bool CommandLineInterface::ParseFC(std::vector< std::string >& argv)
{
    cli::Options opt;
    OptionsData optionsData[] =
    {
        {'a', "all",            OPTARG_NONE},
        {'c', "chunks",            OPTARG_NONE},
        {'d', "defaults",        OPTARG_NONE},
        {'j', "justifications",    OPTARG_NONE},
        {'r', "rl",                OPTARG_NONE},
        {'T', "template",        OPTARG_NONE},
        {'u', "user",            OPTARG_NONE},
        {0, 0, OPTARG_NONE}
    };

    // The number to list defaults to -1 (list all)
    int numberToList = -1;

    // Production defaults to no production
    std::string pProduction;

    // We're using a subset of the print options, so
    // we'll just use the same data structure
    cli::PrintBitset options(0);
    bool hasOptions = false;

    for (;;)
    {
        if (!opt.ProcessOptions(argv, optionsData))
        {
            return SetError(opt.GetError().c_str());
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
                hasOptions = true;
                break;
            case 'c':
                options.set(cli::PRINT_CHUNKS);
                hasOptions = true;
                break;
            case 'd':
                options.set(cli::PRINT_DEFAULTS);
                hasOptions = true;
                break;
            case 'j':
                options.set(cli::PRINT_JUSTIFICATIONS);
                hasOptions = true;
                break;
            case 'r':
                options.set(cli::PRINT_RL);
                hasOptions = true;
                break;
            case 't':
                options.set(cli::PRINT_TEMPLATE);
                hasOptions = true;
                break;
            case 'u':
                options.set(cli::PRINT_USER);
                hasOptions = true;
                break;
        }
    }

    if (opt.GetNonOptionArguments() > 1)
    {
        if (opt.GetNonOptionArguments() > 2)
        {
            return SetError("Too many parameters.");
        } else {
            /* This might not be needed since we can only get a single argument */
            for (size_t i = opt.GetArgument() - opt.GetNonOptionArguments() + 1; i < argv.size(); ++i)
            {
                if (!pProduction.empty())
                {
                    pProduction.push_back(' ');
                }
                pProduction.append(argv[i]);
            }
            // one argument, figure out if it is a non-negative integer or a production
            if (from_string(numberToList, pProduction))
            {
                if (numberToList < 0)
                {
                    return SetError("Expected non-negative integer (count).");
                } else {
                    pProduction.clear();
                }
            }
            else
            {
                // non-integer argument, hopfully a production
                numberToList = -1;
                if (hasOptions)
                {
                    SetError("Ignoring unexpected options when printing firing count for a single production.\n");
                }
            }
        }
    }

    return DoFiringCounts(options, numberToList, &pProduction);
}
bool CommandLineInterface::ParseMatches(std::vector< std::string >& argv)
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

    cli::eWMEDetail detail = cli::WME_DETAIL_NONE;
    cli::eMatchesMode mode = cli::MATCHES_ASSERTIONS_RETRACTIONS;

    for (;;)
    {
        if (!opt.ProcessOptions(argv, optionsData))
        {
            return SetError(opt.GetError().c_str());
        }
        ;
        if (opt.GetOption() == -1)
        {
            break;
        }

        switch (opt.GetOption())
        {
            case 'n':
            case 'c':
                detail = cli::WME_DETAIL_NONE;
                break;
            case 't':
                detail = cli::WME_DETAIL_TIMETAG;
                break;
            case 'w':
                detail = cli::WME_DETAIL_FULL;
                break;
            case 'a':
                mode = cli::MATCHES_ASSERTIONS;
                break;
            case 'r':
                mode = cli::MATCHES_RETRACTIONS;
                break;
        }
    }

    // Max one additional argument and it is a production
    if (opt.GetNonOptionArguments() > 2)
    {
        return SetError("Error.");
    }

    if (opt.GetNonOptionArguments() == 2)
    {
        if (mode != cli::MATCHES_ASSERTIONS_RETRACTIONS)
        {
            return SetError("Error.");
        }
        return DoMatches(cli::MATCHES_PRODUCTION, detail, &argv[opt.GetArgument() - opt.GetNonOptionArguments() + 1]);
    }

    return DoMatches(mode, detail);
}
bool CommandLineInterface::ParseMultiAttributes(std::vector< std::string >& argv)
{
    // No more than three arguments
    if (argv.size() > 4)
    {
        return SetError("Too many parameters");
    }

    int n = 0;
    // If we have 3 arguments, third one is an integer
    if (argv.size() > 3)
    {
        if (!from_string(n, argv[3]) || (n <= 0))
        {
            return SetError("Expected non-negative integer.");
        }
    }

    // If we have two arguments, second arg is an attribute/identifer/whatever
    if (argv.size() > 2)
    {
        return DoMultiAttributes(&argv[2], n);
    }

    return DoMultiAttributes();
}
bool CommandLineInterface::ParsePBreak(std::vector< std::string >& argv)
{
    cli::Options opt;
    OptionsData optionsData[] =
    {
        {'c', "clear", OPTARG_NONE},
        {'p', "print", OPTARG_NONE},
        {'s', "set",   OPTARG_NONE},
        {0, 0, OPTARG_NONE} // null
    };

    char option = 0;

    for (;;)
    {
        if (!opt.ProcessOptions(argv, optionsData))
        {
            return SetError(opt.GetError().c_str());
        }

        if (opt.GetOption() == -1)
        {
            break;
        }

        if (option != 0)
        {
            return SetError("pbreak takes only one option at a time.");
        }
        option = static_cast<char>(opt.GetOption());
    }

    switch (option)
    {
        case 'c':
        case 's':
            if (argv.size() != 4)
            {
                return SetError("pbreak --set/--clear takes exactly one argument.");
            }

            // case: clear the interrupt flag on the production
            return DoPbreak(option, argv[opt.GetArgument() - opt.GetNonOptionArguments() + 1]);

        case 'p':
            if (argv.size() != 3)
            {
                return SetError("pbreak --print takes no arguments.");
            }

            // case: set the interrupt flag on the production
            return DoPbreak('p', "");

        default:
            if (argv.size() == 2)
            {
                return DoPbreak('p', "");
            }
            else if (argv.size() == 3)
            {
                return DoPbreak('s', argv[opt.GetArgument() - opt.GetNonOptionArguments() + 1]);
            }
            else
            {
                return SetError("pbreak used incorrectly.");
            }
    }

    // bad: no option, but more than one argument
    return SetError("pbreak takes exactly one argument.");
}
bool CommandLineInterface::ParsePFind(std::vector< std::string >& argv)
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

    cli::ProductionFindBitset options(0);

    for (;;)
    {
        if (!opt.ProcessOptions(argv, optionsData))
        {
            return SetError(opt.GetError().c_str());
        }
        ;
        if (opt.GetOption() == -1)
        {
            break;
        }

        switch (opt.GetOption())
        {
            case 'c':
                options.set(cli::PRODUCTION_FIND_ONLY_CHUNKS);
                options.reset(cli::PRODUCTION_FIND_NO_CHUNKS);
                break;
            case 'l':
                options.set(cli::PRODUCTION_FIND_INCLUDE_LHS);
                break;
            case 'n':
                options.set(cli::PRODUCTION_FIND_NO_CHUNKS);
                options.reset(cli::PRODUCTION_FIND_ONLY_CHUNKS);
                break;
            case 'r':
                options.set(cli::PRODUCTION_FIND_INCLUDE_RHS);
                break;
            case 's':
                options.set(cli::PRODUCTION_FIND_SHOWBINDINGS);
                break;
        }
    }

    if (opt.CheckNumNonOptArgs(1, 1))
    {
        return SetError("No pattern specified.");
    }

    if (options.none())
    {
        options.set(cli::PRODUCTION_FIND_INCLUDE_LHS);
    }

    std::string pattern;
    for (unsigned i = opt.GetArgument() - opt.GetNonOptionArguments() + 1; i < argv.size(); ++i)
    {
        pattern += argv[i];
        pattern += ' ';
    }
    pattern = pattern.substr(0, pattern.length() - 1);

    return DoProductionFind(options, pattern);
}
bool CommandLineInterface::ParsePWatch(std::vector< std::string >& argv)
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
        if (!opt.ProcessOptions(argv, optionsData))
        {
            return SetError(opt.GetError().c_str());
        }
        ;
        if (opt.GetOption() == -1)
        {
            break;
        }

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
    if (opt.GetNonOptionArguments() > 2)
    {
        return SetError("Too many parameters");
    }

    if (opt.GetNonOptionArguments() == 2)
    {
        return DoPWatch(false, &argv[opt.GetArgument() - opt.GetNonOptionArguments() + 1], setting);
    }
    return DoPWatch(query, 0);
}

bool CommandLineInterface::DoPbreak(const char& mode, const std::string& production)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (mode == 's' || mode == 'c')
    {
        Symbol* sym = thisAgent->symbolManager->find_str_constant(production.c_str());
        rete_node* prod = (sym && sym->sc->production) ? sym->sc->production->p_node : 0;

        if (!prod)
        {
            return SetError("Production not found: " + production);
        }

        if (mode == 's' && !sym->sc->production->interrupt)
        {
            sym->sc->production->interrupt = true;
            sym->sc->production->interrupt_break = true;
        }
        else if (mode == 'c' && sym->sc->production->interrupt)
        {
            sym->sc->production->interrupt = false;
            sym->sc->production->interrupt_break = false;
        }
    }
    else
    {
        assert(mode == 'p');

        for (int i = 0; i != NUM_PRODUCTION_TYPES; ++i)
        {
            for (struct production_struct* prod = thisAgent->all_productions_of_type[i]; prod; prod = prod->next)
            {
                if (prod->interrupt_break)
                {
                    m_Result << prod->name->sc->name << std::endl;
                }
            }
        }
    }

    // Transfer the result from m_XMLResult into pResponse
    // We pass back the name of the command we just executed which becomes the tag name
    // used in the resulting XML.
    if (!m_RawOutput)
    {
        XMLResultToResponse("break") ;
    }

    return true;
}
bool CommandLineInterface::DoExcise(const ExciseBitset& options, const std::string* pProduction)
{
    int64_t exciseCount = 0;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::string lCmd("init");
    production* nextProd = NULL;

    // Process the general options
    if (options.test(EXCISE_ALL))
    {
        exciseCount += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];

        excise_all_productions(thisAgent, false);
    }
    if (options.test(EXCISE_CHUNKS))
    {
        exciseCount += thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE];

        excise_all_productions_of_type(thisAgent, CHUNK_PRODUCTION_TYPE, false);
        excise_all_productions_of_type(thisAgent, JUSTIFICATION_PRODUCTION_TYPE, false);
    }
    if (options.test(EXCISE_DEFAULT))
    {
        exciseCount += thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];

        excise_all_productions_of_type(thisAgent, DEFAULT_PRODUCTION_TYPE, false);
    }
    if (options.test(EXCISE_RL))
    {
        for (production* prod = thisAgent->all_productions_of_type[DEFAULT_PRODUCTION_TYPE]; prod != NIL; prod = nextProd)
        {
            nextProd = prod->next;
            if (prod->rl_rule)
            {
                exciseCount++;
                excise_production(thisAgent, prod, thisAgent->sysparams[TRACE_LOADING_SYSPARAM] != 0);
            }
        }

        for (production* prod = thisAgent->all_productions_of_type[USER_PRODUCTION_TYPE]; prod != NIL; prod = nextProd)
        {
            nextProd = prod->next;
            if (prod->rl_rule)
            {
                exciseCount++;
                excise_production(thisAgent, prod, thisAgent->sysparams[TRACE_LOADING_SYSPARAM] != 0);
            }
        }

        for (production* prod = thisAgent->all_productions_of_type[CHUNK_PRODUCTION_TYPE]; prod != NIL; prod = nextProd)
        {
            nextProd = prod->next;
            if (prod->rl_rule)
            {
                exciseCount++;
                excise_production(thisAgent, prod, thisAgent->sysparams[TRACE_LOADING_SYSPARAM] != 0);
            }
        }

        rl_initialize_template_tracking(thisAgent);
    }
    if (options.test(EXCISE_NEVER_FIRED))
    {
        for (int i = 0; i < NUM_PRODUCTION_TYPES; i++)
        {
            for (production* prod = thisAgent->all_productions_of_type[i]; prod != NIL; prod = nextProd)
            {
                nextProd = prod->next;
                if (!prod->firing_count)
                {
                    exciseCount++;
                    excise_production(thisAgent, prod, thisAgent->sysparams[TRACE_LOADING_SYSPARAM] != 0);
                }
            }
        }
    }
    if (options.test(EXCISE_TASK))
    {
        exciseCount += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];

        excise_all_productions_of_type(thisAgent, USER_PRODUCTION_TYPE, false);
        excise_all_productions_of_type(thisAgent, DEFAULT_PRODUCTION_TYPE, false);
    }
    if (options.test(EXCISE_TEMPLATE))
    {
        exciseCount += thisAgent->num_productions_of_type[TEMPLATE_PRODUCTION_TYPE];

        excise_all_productions_of_type(thisAgent, TEMPLATE_PRODUCTION_TYPE, false);
    }
    if (options.test(EXCISE_USER))
    {
        exciseCount += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];

        excise_all_productions_of_type(thisAgent, USER_PRODUCTION_TYPE, false);
    }

    // Excise specific production
    if (pProduction)
    {
        Symbol* sym = thisAgent->symbolManager->find_str_constant(pProduction->c_str());

        if (!sym || !(sym->sc->production))
        {
            return SetError("Production not found.");
        }

        if (!m_RawOutput)
        {
            // Save the name for the structured response
            AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, *pProduction);
        }

        // Increment the count for the structured response
        ++exciseCount;

        excise_production(thisAgent, sym->sc->production, false);
    }

    if (m_RawOutput)
    {
        m_Result << exciseCount << " production" << (exciseCount == 1 ? " " : "s ") << "excised.\n";
    }
    else
    {
        // Add the count tag to the front
        std::string temp;
        PrependArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, to_string(exciseCount, temp));
    }

    return true;
}
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
        Symbol* sym = thisAgent->symbolManager->find_str_constant(pProduction->c_str());

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
bool CommandLineInterface::DoMatches(const eMatchesMode mode, const eWMEDetail detail, const std::string* pProduction)
{
    wme_trace_type wtt = 0;
    switch (detail)
    {
        case WME_DETAIL_NONE:
            wtt = NONE_WME_TRACE;
            break;
        case WME_DETAIL_TIMETAG:
            wtt = TIMETAG_WME_TRACE;
            break;
        case WME_DETAIL_FULL:
            wtt = FULL_WME_TRACE;
            break;
        default:
            assert(false);
    }

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (mode == MATCHES_PRODUCTION)
    {
        if (!pProduction)
        {
            return SetError("Production required.");
        }

        Symbol* sym = thisAgent->symbolManager->find_str_constant(pProduction->c_str());
        rete_node* prod = (sym && sym->sc->production) ? sym->sc->production->p_node : 0;

        if (!prod)
        {
            return SetError("Production not found: " + *pProduction);
        }

        if (m_RawOutput)
        {
            print_partial_match_information(thisAgent, prod, wtt);
        }
        else
        {
            xml_partial_match_information(thisAgent, prod, wtt);
        }

    }
    else
    {
        ms_trace_type mst = MS_ASSERT_RETRACT;
        if (mode == MATCHES_ASSERTIONS)
        {
            mst = MS_ASSERT;
        }
        if (mode == MATCHES_RETRACTIONS)
        {
            mst = MS_RETRACT;
        }

        if (m_RawOutput)
        {
            print_match_set(thisAgent, wtt, mst);
        }
        else
        {
            xml_match_set(thisAgent, wtt, mst);
        }
    }

    // Transfer the result from m_XMLResult into pResponse
    // We pass back the name of the command we just executed which becomes the tag name
    // used in the resulting XML.
    if (!m_RawOutput)
    {
        XMLResultToResponse("matches") ;
    }

    return true;
}

bool CommandLineInterface::DoMultiAttributes(const std::string* pAttribute, int n)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    multi_attribute* maList = thisAgent->multi_attributes;

    if (!pAttribute && !n)
    {

        // No args, print current setting
        int count = 0;

        if (!maList)
        {
            m_Result << "No multi-attributes found.";
        }

        std::stringstream buffer;

        if (m_RawOutput)
        {
            m_Result << "Value\tSymbol\n";
        }

        while (maList)
        {
            // Arbitrary buffer and size
            char attributeName[1024];
            maList->symbol->to_string(true, attributeName, 1024);

            if (m_RawOutput)
            {
                m_Result  << maList->value << "\t" << maList->symbol->to_string(true, attributeName, 1024) << std::endl;

            }
            else
            {
                buffer << maList->value;
                // Value
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, buffer.str());
                buffer.clear();

                // Symbol
                AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, attributeName);
            }

            ++count;

            maList = maList->next;
        }

        buffer << count;
        if (!m_RawOutput)
        {
            PrependArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, buffer.str());
        }
        return true;
    }

    // Setting defaults to 10
    if (!n)
    {
        n = 10;
    }

    // Set it
    Symbol* s = thisAgent->symbolManager->make_str_constant(pAttribute->c_str());

    while (maList)
    {
        if (maList->symbol == s)
        {
            maList->value = n;
            thisAgent->symbolManager->symbol_remove_ref(&s);
            return true;
        }

        maList = maList->next;
    }

    /* sym wasn't in the table if we get here, so add it */
    maList = static_cast<multi_attribute*>(thisAgent->memoryManager->allocate_memory(sizeof(multi_attribute), MISCELLANEOUS_MEM_USAGE));
    assert(maList);

    maList->value = n;
    maList->symbol = s;
    maList->next = thisAgent->multi_attributes;
    thisAgent->multi_attributes = maList;

    return true;
}

bool CommandLineInterface::DoPWatch(bool query, const std::string* pProduction, bool setting)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    // check for query or not production
    if (query || !pProduction)
    {
        // list all productions currently being traced
        production* pSoarProduction = 0;
        int productionCount = 0;
        for (unsigned int i = 0; i < NUM_PRODUCTION_TYPES; ++i)
        {
            for (pSoarProduction = thisAgent->all_productions_of_type[i];
                    pSoarProduction != 0;
                    pSoarProduction = pSoarProduction->next)
            {
                // is it being watched
                if (pSoarProduction->trace_firings)
                {

                    if (query)
                    {
                        ++productionCount;
                        if (m_RawOutput)
                        {
                            m_Result << '\n' << pSoarProduction->name->sc->name;
                        }
                        else
                        {
                            AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, pSoarProduction->name->sc->name);
                        }
                    }
                    else
                    {
                        // not querying, shut it off
                        remove_pwatch(thisAgent, pSoarProduction);
                    }
                }

            }
        }

        if (query)
        {
            // we're querying, summarize
            if (m_RawOutput)
            {
                if (!productionCount)
                {
                    m_Result << "No watched productions found.";
                }
            }
            else if (!m_RawOutput)
            {
                std::stringstream buffer;
                buffer << productionCount;
                PrependArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, buffer.str());
            }
        }

        return true;
    }

    Symbol* sym = thisAgent->symbolManager->find_str_constant(pProduction->c_str());

    if (!sym || !(sym->sc->production))
    {
        return SetError("Production not found.");
    }

    // we have a production
    if (setting)
    {
        add_pwatch(thisAgent, sym->sc->production);
    }
    else
    {
        remove_pwatch(thisAgent, sym->sc->production);
    }
    return true;
}

void free_binding_list(agent* thisAgent, cons* bindings)
{
    cons* c;

    for (c = bindings; c != NIL; c = c->rest)
    {
        thisAgent->memoryManager->free_memory(c->first, MISCELLANEOUS_MEM_USAGE);
    }
    free_list(thisAgent, bindings);
}

void print_binding_list(agent* thisAgent, cons* bindings)
{
    cons* c;

    for (c = bindings ; c != NIL ; c = c->rest)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "   (%y -> %y)\n", static_cast<Binding*>(c->first)->from, static_cast<Binding*>(c->first)->to);
    }
}

void reset_old_binding_point(agent* thisAgent, cons** bindings, cons** current_binding_point)
{
    cons* c, *c_next;

    c = *bindings;
    while (c != *current_binding_point)
    {
        c_next = c->rest;
        thisAgent->memoryManager->free_memory(c->first, MISCELLANEOUS_MEM_USAGE);
        free_cons(thisAgent, c);
        c = c_next;
    }

    bindings = current_binding_point;
}

Symbol* get_binding(Symbol* f, cons* bindings)
{
    cons* c;

    for (c = bindings; c != NIL; c = c->rest)
    {
        if (static_cast<Binding*>(c->first)->from == f)
        {
            return static_cast<Binding*>(c->first)->to;
        }
    }
    return NIL;
}

bool symbols_are_equal_with_bindings(agent* thisAgent, Symbol* s1, Symbol* s2, cons** bindings)
{
    Binding* b;
    Symbol* bvar;

    if ((s1 == s2) && (s1->symbol_type != VARIABLE_SYMBOL_TYPE))
    {
        return true;
    }

    /* "*" matches everything. */
    if ((s1->symbol_type == STR_CONSTANT_SYMBOL_TYPE) &&
            (!strcmp(s1->sc->name, "*")))
    {
        return true;
    }
    if ((s2->symbol_type == STR_CONSTANT_SYMBOL_TYPE) &&
            (!strcmp(s2->sc->name, "*")))
    {
        return true;
    }


    if ((s1->symbol_type != VARIABLE_SYMBOL_TYPE) ||
            (s2->symbol_type != VARIABLE_SYMBOL_TYPE))
    {
        return false;
    }
    /* Both are variables */
    bvar = get_binding(s1, *bindings);
    if (bvar == NIL)
    {
        b = static_cast<Binding*>(thisAgent->memoryManager->allocate_memory(sizeof(Binding), MISCELLANEOUS_MEM_USAGE));
        b->from = s1;
        b->to = s2;
        push(thisAgent, b, *bindings);
        return true;
    }
    else if (bvar == s2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool actions_are_equal_with_bindings(agent* thisAgent, action* a1, action* a2, cons** bindings)
{
    //         if (a1->type == FUNCALL_ACTION)
    //         {
    //            if ((a2->type == FUNCALL_ACTION))
    //            {
    //               if (funcalls_match(rhs_value_to_funcall_list(a1->value),
    //                  rhs_value_to_funcall_list(a2->value)))
    //               {
    //                     return true;
    //               }
    //               else return false;
    //            }
    //            else return false;
    //         }
    if (a2->type == FUNCALL_ACTION)
    {
        return false;
    }

    /* Both are make_actions. */

    if (a1->preference_type != a2->preference_type)
    {
        return false;
    }

    if (!symbols_are_equal_with_bindings(thisAgent, rhs_value_to_symbol(a1->id),
                                         rhs_value_to_symbol(a2->id),
                                         bindings))
    {
        return false;
    }

    if ((rhs_value_is_symbol(a1->attr)) && (rhs_value_is_symbol(a2->attr)))
    {
        if (!symbols_are_equal_with_bindings(thisAgent, rhs_value_to_symbol(a1->attr),
                                             rhs_value_to_symbol(a2->attr), bindings))
        {
            return false;
        }
    }
    else
    {
        //            if ((rhs_value_is_funcall(a1->attr)) && (rhs_value_is_funcall(a2->attr)))
        //            {
        //               if (!funcalls_match(rhs_value_to_funcall_list(a1->attr),
        //                  rhs_value_to_funcall_list(a2->attr)))
        //               {
        //                  return false;
        //               }
        //            }
    }

    /* Values are different. They are rhs_value's. */

    if ((rhs_value_is_symbol(a1->value)) && (rhs_value_is_symbol(a2->value)))
    {
        if (symbols_are_equal_with_bindings(thisAgent, rhs_value_to_symbol(a1->value),
                                            rhs_value_to_symbol(a2->value), bindings))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    if ((rhs_value_is_funcall(a1->value)) && (rhs_value_is_funcall(a2->value)))
    {
        //            if (funcalls_match(rhs_value_to_funcall_list(a1->value),
        //               rhs_value_to_funcall_list(a2->value)))
        //            {
        //               return true;
        //            }
        //            else
        {
            return false;
        }
    }
    return false;
}


#define dealloc_and_return(thisAgent,x,y) { deallocate_test(thisAgent, x) ; return (y) ; }

bool tests_are_equal_with_bindings(agent* thisAgent, test t1, test test2, cons** bindings)
{
    cons* c1, *c2;
    bool goal_test, impasse_test;

    /* DJP 4/3/96 -- The problem here is that sometimes test2 was being copied      */
    /*               and sometimes it wasn't.  If it was copied, the copy was never */
    /*               deallocated.  There's a few choices about how to fix this.  I  */
    /*               decided to just create a copy always and then always           */
    /*               deallocate it before returning.  Added a macro to do that.     */

    test t2;

    /* t1 is from the pattern given to "pf"; t2 is from a production's condition list. */
    if (!t1)
    {
        return (test2 != 0);
    }

    /* If the pattern doesn't include "(state", but the test from the
    * production does, strip it out of the production's.
    */
    if ((!test_includes_goal_or_impasse_id_test(t1, true, false)) &&
            test_includes_goal_or_impasse_id_test(test2, true, false))
    {
        goal_test = false;
        impasse_test = false;
        t2 = copy_test_removing_goal_impasse_tests(thisAgent, test2, &goal_test, &impasse_test);
    }
    else
    {
        t2 = copy_test(thisAgent, test2) ; /* DJP 4/3/96 -- Always make t2 into a copy */
    }
    if (t1->type == EQUALITY_TEST)
    {
        if (!(t2 && (t2->type == EQUALITY_TEST)))
        {
            dealloc_and_return(thisAgent, t2, false);
        }
        else
        {
            if (symbols_are_equal_with_bindings(thisAgent, t1->data.referent, t2->data.referent, bindings))
            {
                dealloc_and_return(thisAgent, t2, true);
            }
            else
            {
                dealloc_and_return(thisAgent, t2, false);
            }
        }
    }

    if (t1->type != t2->type)
    {
        dealloc_and_return(thisAgent, t2, false);
    }

    switch (t1->type)
    {
        case GOAL_ID_TEST:
            dealloc_and_return(thisAgent, t2, true);
            break;
        case IMPASSE_ID_TEST:
            dealloc_and_return(thisAgent, t2, true);
            break;
        case SMEM_LINK_UNARY_TEST:
            dealloc_and_return(thisAgent, t2, true);
            break;
        case SMEM_LINK_UNARY_NOT_TEST:
            dealloc_and_return(thisAgent, t2, true);
            break;
        case DISJUNCTION_TEST:
            for (c1 = t1->data.disjunction_list, c2 = t2->data.disjunction_list;
                    ((c1 != NIL) && (c2 != NIL));
                    c1 = c1->rest, c2 = c2->rest)
            {
                if (c1->first != c2->first)
                {
                    dealloc_and_return(thisAgent, t2, false)
                }
            }
            if (c1 == c2)
            {
                dealloc_and_return(thisAgent, t2, true); /* make sure they both hit end-of-list */
            }
            else
            {
                dealloc_and_return(thisAgent, t2, false);
            }
            break;
        case CONJUNCTIVE_TEST:
            for (c1 = t1->data.conjunct_list, c2 = t2->data.conjunct_list;
                    ((c1 != NIL) && (c2 != NIL)); c1 = c1->rest, c2 = c2->rest)
            {
                if (!tests_are_equal_with_bindings(thisAgent, static_cast<test>(c1->first), static_cast<test>(c2->first), bindings))
                    dealloc_and_return(thisAgent, t2, false)
                }
            if (c1 == c2)
            {
                dealloc_and_return(thisAgent, t2, true); /* make sure they both hit end-of-list */
            }
            else
            {
                dealloc_and_return(thisAgent, t2, false);
            }
            break;
        default:  /* relational tests other than equality */
            if (symbols_are_equal_with_bindings(thisAgent, t1->data.referent, t2->data.referent, bindings))
            {
                dealloc_and_return(thisAgent, t2, true);
            }
            else
            {
                dealloc_and_return(thisAgent, t2, false);
            }
            break;
    }
    return false;
}

bool conditions_are_equal_with_bindings(agent* thisAgent, condition* c1, condition* c2, cons** bindings)
{
    if (c1->type != c2->type)
    {
        return false;
    }
    switch (c1->type)
    {
        case POSITIVE_CONDITION:
        case NEGATIVE_CONDITION:
            if (! tests_are_equal_with_bindings(thisAgent, c1->data.tests.id_test,
                                                c2->data.tests.id_test, bindings))
            {
                return false;
            }
            if (! tests_are_equal_with_bindings(thisAgent, c1->data.tests.attr_test,
                                                c2->data.tests.attr_test, bindings))

            {
                return false;
            }
            if (! tests_are_equal_with_bindings(thisAgent, c1->data.tests.value_test,
                                                c2->data.tests.value_test, bindings))
            {
                return false;
            }
            if (c1->test_for_acceptable_preference != c2->test_for_acceptable_preference)
            {
                return false;
            }
            return true;

        case CONJUNCTIVE_NEGATION_CONDITION:
            for (c1 = c1->data.ncc.top, c2 = c2->data.ncc.top;
                    ((c1 != NIL) && (c2 != NIL));
                    c1 = c1->next, c2 = c2->next)
                if (! conditions_are_equal_with_bindings(thisAgent, c1, c2, bindings))
                {
                    return false;
                }
            if (c1 == c2)
            {
                return true;    /* make sure they both hit end-of-list */
            }
            return false;
    }
    return false; /* unreachable, but without it, gcc -Wall warns here */
}

void read_pattern_and_get_matching_productions(agent* thisAgent,
        const char* lhs_str,
        cons** current_pf_list,
        bool show_bindings,
        bool just_chunks,
        bool no_chunks)
{
    condition* c, *clist, *top, *bottom, *pc;
    int i;
    production* prod;
    cons* bindings, *current_binding_point;
    bool match, match_this_c;


    bindings = NIL;
    current_binding_point = NIL;

    /*  print("Parsing as a lhs...\n"); */
    soar::Lexer lexer(thisAgent, lhs_str);
    lexer.get_lexeme();
    clist = parse_lhs(thisAgent, &lexer);
    if (!clist)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Error: not a valid condition list.\n");
        current_pf_list = NIL;
        return;
    }
    /*
    print("Valid condition list:\n");
    print_condition_list(clist,0,false);
    print("\nMatches:\n");
    */

    /* For the moment match against productions of all types (user,chunk,default, justification).     Later on the type should be a parameter.
    */

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
        if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) ||
                (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
            for (prod = thisAgent->all_productions_of_type[i]; prod != NIL; prod = prod->next)
            {

                /* Now the complicated part. */
                /* This is basically a full graph-match.  Like the rete.  Yikes! */
                /* Actually it's worse, because there are so many different KINDS of
                conditions (negated, >/<, acc prefs, ...) */
                /* Is there some way I could *USE* the rete for this?  -- for lhs
                positive conditions, possibly.  Put some fake stuff into WM
                (i.e. with make-wme's), see what matches all of them, and then
                yank out the fake stuff.  But that won't work for RHS or for
                negateds.       */

                /* Also note that we need bindings for every production.  Very expensive
                (but don't necc. need to save them -- maybe can just print them as we go). */

                match = true;
                p_node_to_conditions_and_rhs(thisAgent, prod->p_node, NIL, NIL, &top, &bottom, NIL);

                free_binding_list(thisAgent, bindings);
                bindings = NIL;

                for (c = clist; c != NIL; c = c->next)
                {
                    match_this_c = false;
                    current_binding_point = bindings;

                    for (pc = top; pc != NIL; pc = pc->next)
                    {
                        if (conditions_are_equal_with_bindings(thisAgent, c, pc, &bindings))
                        {
                            match_this_c = true;
                            break;
                        }
                        else
                        {
                            /* Remove new, incorrect bindings. */
                            reset_old_binding_point(thisAgent, &bindings, &current_binding_point);
                            bindings = current_binding_point;
                        }
                    }
                    if (!match_this_c)
                    {
                        match = false;
                        break;
                    }
                }
                deallocate_condition_list(thisAgent, top);  /* DJP 4/3/96 -- Never dealloced */
                if (match)
                {
                    push(thisAgent, prod, (*current_pf_list));
                    if (show_bindings)
                    {
                        thisAgent->outputManager->printa_sf(thisAgent, "%y, with bindings:\n", prod->name);
                        print_binding_list(thisAgent, bindings);
                    }
                    else
                    {
                        thisAgent->outputManager->printa_sf(thisAgent, "%y\n", prod->name);
                    }
                }
            }
    if (bindings)
    {
        free_binding_list(thisAgent, bindings);    /* DJP 4/3/96 -- To catch the last production */
    }
}

void read_rhs_pattern_and_get_matching_productions(agent* thisAgent,
        const char* rhs_string,
        cons** current_pf_list,
        bool show_bindings,
        bool just_chunks,
        bool no_chunks)
{

    action* a, *alist, *pa;
    int i;
    production* prod;
    cons* bindings, *current_binding_point;
    bool match, match_this_a, parsed_ok;
    action* rhs;
    condition* top_cond, *bottom_cond;

    bindings = NIL;
    current_binding_point = NIL;

    /*  print("Parsing as a rhs...\n"); */
    soar::Lexer lexer(thisAgent, rhs_string);
    lexer.get_lexeme();
    parsed_ok = (parse_rhs(thisAgent, &lexer, &alist) == true);
    if (!parsed_ok)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "Error: not a valid rhs.\n");
        current_pf_list = NIL;
        return;
    }

    /*
    print("Valid RHS:\n");
    print_action_list(alist,0,false);
    print("\nMatches:\n");
    */

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
    {
        if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) || (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
        {
            for (prod = thisAgent->all_productions_of_type[i]; prod != NIL; prod = prod->next)
            {
                match = true;

                free_binding_list(thisAgent, bindings);
                bindings = NIL;

                p_node_to_conditions_and_rhs(thisAgent, prod->p_node, NIL, NIL, &top_cond, &bottom_cond, &rhs);
                deallocate_condition_list(thisAgent, top_cond);
                for (a = alist; a != NIL; a = a->next)
                {
                    match_this_a = false;
                    current_binding_point = bindings;

                    for (pa = rhs; pa != NIL; pa = pa->next)
                    {
                        if (actions_are_equal_with_bindings(thisAgent, a, pa, &bindings))
                        {
                            match_this_a = true;
                            break;
                        }
                        else
                        {
                            /* Remove new, incorrect bindings. */
                            reset_old_binding_point(thisAgent, &bindings, &current_binding_point);
                            bindings = current_binding_point;
                        }
                    }
                    if (!match_this_a)
                    {
                        match = false;
                        break;
                    }
                }

                deallocate_action_list(thisAgent, rhs);
                if (match)
                {
                    push(thisAgent, prod, (*current_pf_list));
                    if (show_bindings)
                    {
                        thisAgent->outputManager->printa_sf(thisAgent, "%y, with bindings:\n", prod->name);
                        print_binding_list(thisAgent, bindings);
                    }
                    else
                    {
                        thisAgent->outputManager->printa_sf(thisAgent, "%y\n", prod->name);
                    }
                }
            }
        }
    }
    if (bindings)
    {
        free_binding_list(thisAgent, bindings); /* DJP 4/3/96 -- To catch the last production */
    }
}

bool CommandLineInterface::DoProductionFind(const ProductionFindBitset& options, const std::string& pattern)
{
    cons* current_pf_list = 0;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (options.test(PRODUCTION_FIND_INCLUDE_LHS))
    {
        /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
        read_pattern_and_get_matching_productions (thisAgent,
                pattern.c_str(),
                &current_pf_list,
                options.test(PRODUCTION_FIND_SHOWBINDINGS),
                options.test(PRODUCTION_FIND_ONLY_CHUNKS),
                options.test(PRODUCTION_FIND_NO_CHUNKS));
    }
    if (options.test(PRODUCTION_FIND_INCLUDE_RHS))
    {
        /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
        /* Soar-Bugs #54 TMH */
        read_pattern_and_get_matching_productions (thisAgent,
                pattern.c_str(),
                &current_pf_list,
                options.test(PRODUCTION_FIND_SHOWBINDINGS),
                options.test(PRODUCTION_FIND_ONLY_CHUNKS),
                options.test(PRODUCTION_FIND_NO_CHUNKS));
    }
    if (current_pf_list == NIL)
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "No matches.\n");
    }

    free_list(thisAgent, current_pf_list);
    return true;
}

