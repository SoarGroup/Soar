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

#include "sml_AgentSML.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"
#include "sml_Utils.h"

#include "agent.h"
#include "cmd_memory_settings.h"
#include "output_manager.h"
#include "print.h"
#include "production.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "rete.h"

#include <algorithm>

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoMemory(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->memory_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->memory_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid memory command.  Use 'memory ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->memory_params->allocate_cmd)
    {
        return ParseAllocate(argv);

    }
    else if (my_param == thisAgent->command_params->memory_params->memories_cmd)
    {
        return ParseMemories(argv);
    }
    else if ((my_param == thisAgent->command_params->memory_params->help_cmd) || (my_param == thisAgent->command_params->memory_params->qhelp_cmd))
    {
        thisAgent->command_params->memory_params->print_settings(thisAgent);
    }
    return false;
}

bool CommandLineInterface::ParseAllocate(std::vector< std::string >& argv)
{
    if (argv.size() == 2)
    {
        return DoAllocate(std::string(), 0);
    }

    if (argv.size() != 4)
    {
        return SetError("Syntax: allocate [pool blocks]");
    }

    int blocks = 0;
    if (!from_string(blocks, argv[3]))
    {
        return SetError("Expected an integer (number of blocks).");
    }

    if (blocks < 1)
    {
        return SetError("Expected a positive integer (number of blocks).");
    }

    return DoAllocate(argv[2], blocks);
}

bool CommandLineInterface::DoAllocate(const std::string& pool, int blocks)
{
    if (pool.empty())
    {
        GetMemoryPoolStatistics(); // cli_stats.cpp
        return true;
    }

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (thisAgent->memoryManager->add_block_to_memory_pool_by_name(pool, blocks))
    {
        m_Result << pool << " blocks increased by " << blocks;
        return true;
    }

    SetError("Could not allocate memory.  Probably a bad pool name: " + pool);
    return false;
}

struct MemoriesSort
{
    bool operator()(std::pair< std::string, uint64_t > a, std::pair< std::string, uint64_t > b) const
    {
        return a.second < b.second;
    }
};

bool CommandLineInterface::DoMemories(const MemoriesBitset options, int n, const std::string* pProduction)
{
    std::vector< std::pair< std::string, uint64_t > > memories;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    // get either one production or all of them
    if (options.none())
    {
        if (!pProduction)
        {
            return SetError("Production required.");
        }

        Symbol* sym = thisAgent->symbolManager->find_str_constant(pProduction->c_str());

        if (!sym || !(sym->sc->production))
        {
            return SetError("Production not found.");
        }

        // save the tokens/name pair
        std::pair< std::string, uint64_t > memory;
        memory.first = *pProduction;
        memory.second = count_rete_tokens_for_production(thisAgent, sym->sc->production);
        memories.push_back(memory);

    }
    else
    {
        bool foundProduction = false;

        for (unsigned int i = 0; i < NUM_PRODUCTION_TYPES; ++i)
        {
            // if filter is set, skip types that are not specified
            if (!options.none())
            {
                switch (i)
                {
                    case USER_PRODUCTION_TYPE:
                        if (!options.test(MEMORIES_USER))
                        {
                            continue;
                        }
                        break;

                    case DEFAULT_PRODUCTION_TYPE:
                        if (!options.test(MEMORIES_DEFAULT))
                        {
                            continue;
                        }
                        break;

                    case CHUNK_PRODUCTION_TYPE:
                        if (!options.test(MEMORIES_CHUNKS))
                        {
                            continue;
                        }
                        break;

                    case JUSTIFICATION_PRODUCTION_TYPE:
                        if (!options.test(MEMORIES_JUSTIFICATIONS))
                        {
                            continue;
                        }
                        break;

                    case TEMPLATE_PRODUCTION_TYPE:
                        if (!options.test(MEMORIES_TEMPLATES))
                        {
                            continue;
                        }
                        break;

                    default:
                        assert(false);
                        break;
                }
            }

            for (production* pSoarProduction = thisAgent->all_productions_of_type[i];
                    pSoarProduction != 0;
                    pSoarProduction = pSoarProduction->next)
            {
                foundProduction = true;

                // save the tokens/name pair
                std::pair< std::string, uint64_t > memory;
                memory.first = pSoarProduction->name->sc->name;
                memory.second = count_rete_tokens_for_production(thisAgent, pSoarProduction);
                memories.push_back(memory);
            }
        }

        if (!foundProduction)
        {
            return SetError("Production not found.");
        }
    }

    // sort them
    MemoriesSort s;
    sort(memories.begin(), memories.end(), s);

    // print them
    int i = 0;
    for (std::vector< std::pair< std::string, uint64_t > >::reverse_iterator j = memories.rbegin();
            j != memories.rend() && (n == 0 || i < n);
            ++j, ++i)
    {
        if (m_RawOutput)
        {
            m_Result  << std::setw(6) << j->second << ":  " << j->first << "\n";
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

bool CommandLineInterface::ParseMemories(std::vector< std::string >& argv)
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

    assert(opt.GetNonOptionArguments() > 0);
    // Max one additional argument
    if (opt.GetNonOptionArguments() > 2)
    {
        return SetError("Syntax: memories [options] [number]\nmemories production_name");
    }

    // It is either a production or a number
    int n = 0;
    if (opt.GetNonOptionArguments() == 2)
    {
        int optind = opt.GetArgument() - opt.GetNonOptionArguments() + 1;
        if (from_string(n, argv[optind]))
        {
            // number
            if (n <= 0)
            {
                return SetError("Expected positive integer.");
            }
        }
        else
        {
            // production
            if (options.any())
            {
                return SetError("Do not specify production type when specifying a production name.");
            }
            return DoMemories(options, 0, &argv[optind]);
        }
    }

    // Default to all types when no production and no type specified
    if (options.none())
    {
        options.flip();
    }

    // handle production/number cases
    return DoMemories(options, n);
}
