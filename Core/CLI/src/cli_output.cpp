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
#include "ebc.h"
#include "explanation_memory.h"
#include "ebc_settings.h"
#include "output_manager.h"
#include "output_settings.h"
#include "print.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoOutput(std::vector<std::string>& argv, const char pOp, const std::string* pAttr, const std::string* pVal, bool pAppend)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;

    if (!pOp)
    {
        PrintCLIMessage("Output contains settings and sub-commands to control what Soar prints and where it prints it.\n"
            "Use 'output ?' and 'help output' to learn more about the output command.");
        return true;
    }
    else if (pOp == 'G')
    {
        /* Single command argument */
        soar_module::param* my_param = thisAgent->outputManager->m_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid output sub-command.  Use 'output ?' to see a list of valid sub-commands and settings.");
        }
        else if ((my_param == thisAgent->outputManager->m_params->help_cmd) || (my_param == thisAgent->outputManager->m_params->qhelp_cmd))
        {
            thisAgent->outputManager->m_params->print_output_settings(thisAgent);
        }
        else {
            /* Command was a valid ebc_param name, so print it's value */
            tempStringStream << my_param->get_name() << " =" ;
            PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
        }
        return true;
    }
    else if (pOp == 'S')
    {
        soar_module::param* my_param = thisAgent->outputManager->m_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid output command.  Use 'output ?' to see a list of valid sub-commands.");
        }
        else if (my_param == thisAgent->outputManager->m_params->clog)
        {
            argv.erase(argv.begin());
            return ParseClog(argv);
        }
        else if (my_param == thisAgent->outputManager->m_params->ctf)
        {
            argv.erase(argv.begin());
            if (pAppend)
            {
                std::string temp = *(argv.begin());
                argv[0] = argv[1];
                argv[1] = temp;
            }
            return ParseCTF(argv);
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid argument for output command. Use 'output ?' to see a list of valid sub-commands.");
        }

        bool result = my_param->set_string(pVal->c_str());

        if (!result)
        {
            return SetError("The output parameter could not be changed.");
        }
        else
        {
            tempStringStream << my_param->get_name() << " = " << pVal->c_str();
            PrintCLIMessage(&tempStringStream);
        }
        /* The following code assumes that all parameters except learn are boolean */
        if (!strcmp(pAttr->c_str(), "print-depth"))
        {
            thisAgent->outputManager->m_params->update_int_setting(thisAgent, static_cast<soar_module::integer_param*>(my_param));
        } else {
            thisAgent->outputManager->m_params->update_bool_setting(thisAgent, static_cast<soar_module::boolean_param*>(my_param), m_pKernelSML);
        }
        return result;
    }
    else if (pOp == 's')
    {
        thisAgent->explanationMemory->print_global_stats();
        return true;
    }
    else if (pOp == 'h')
    {
        PrintCLIMessage_Header("History", 40);
        return true;
    }

    return true;
}
bool CommandLineInterface::ParseClog(std::vector< std::string >& argv)
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

    cli::eLogMode mode = cli::LOG_NEW;

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
                mode = cli::LOG_ADD;
                break;
            case 'c':
            case 'd':
            case 'o':
                mode = cli::LOG_CLOSE;
                break;
            case 'e':
            case 'A':
                mode = cli::LOG_NEWAPPEND;
                break;
            case 'q':
                mode = cli::LOG_QUERY;
                break;
        }
    }

    switch (mode)
    {
        case cli::LOG_ADD:
        {
            std::string toAdd;
            // no less than one non-option argument
            if (opt.GetNonOptionArguments() < 1)
            {
                return SetError("Provide a string to add.");
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
            return DoCLog(mode, 0, &toAdd);
        }

        case cli::LOG_NEW:
            // no more than one argument, no filename == query
            if (opt.GetNonOptionArguments() > 1)
            {
                return SetError("Filename or nothing expected, enclose filename in quotes if there are spaces in the path.");
            }

            if (opt.GetNonOptionArguments() == 1)
            {
                return DoCLog(mode, &argv[1]);
            }
            break; // no args case handled below

        case cli::LOG_NEWAPPEND:
            // exactly one argument
            if (opt.GetNonOptionArguments() > 1)
            {
                return SetError("Filename expected, enclose filename in quotes if there are spaces in the path.");
            }

            if (opt.GetNonOptionArguments() < 1)
            {
                return SetError("Please provide a filename.");
            }
            return DoCLog(mode, &argv[1]);

        default:
        case cli::LOG_CLOSE:
        case cli::LOG_QUERY:
            // no arguments
            if (opt.GetNonOptionArguments())
            {
                return SetError("No arguments when querying log status.");
            }
            break; // no args case handled below
    }

    // the no args case
    return DoCLog(mode);
}
bool CommandLineInterface::ParseCTF(std::vector< std::string >& argv)
{
    // Not going to use normal option parsing in this case because I do not want to disturb the other command on the line
    if (argv.size() < 3)
    {
        return SetError("Syntax: output command-to-file [-a] <filename> command [args]");
    }

    // Index of command in argv:  command-to-file filename command ...
    // Unless append option is present, which is handled later.
    int startOfCommand = 2;
    cli::eLogMode mode = cli::LOG_NEW;
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
            return SetError("Unrecognized option: " + arg);
        }

        if (append)
        {
            mode = cli::LOG_NEWAPPEND;

            // Index of command in argv:  command-to-file -a filename command ...
            if (argv.size() < 4)
            {
                return SetError("Syntax: output command-to-file [-a] <filename> command [args]");
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

    return DoCommandToFile(mode, filename, newArgv);
}
