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

bool CommandLineInterface::DoRedirectedOutputCommand(std::vector<std::string>& argv, bool& had_error)
{
    if (argv.size() < 3) return false;

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    soar_module::param* my_param = thisAgent->outputManager->m_params->get(argv.at(1).c_str());

    if (!my_param)
    {
        return false;
    }
    else if (my_param == thisAgent->outputManager->m_params->clog)
    {
        argv.erase(argv.begin());
        had_error = !ParseClog(argv);
        return true;
    }
    else if (my_param == thisAgent->outputManager->m_params->ctf)
    {
        argv.erase(argv.begin());
        had_error = !ParseCTF(argv);
        if (!had_error)
        {
            thisAgent->outputManager->printa(thisAgent, "Output of command successfully written to file.\n");
        }
        return true;
    }
    return false;
}

bool CommandLineInterface::DoOutput(std::vector<std::string>& argv, const std::string* pArg1, const std::string* pArg2, const std::string* pArg3)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;

    if (!pArg1)
    {
        thisAgent->outputManager->m_params->print_output_summary(thisAgent);
        return true;
    }

    soar_module::param* my_param = thisAgent->outputManager->m_params->get(pArg1->c_str());

    if (!my_param)
    {
        return SetError("Invalid output sub-command.  Use 'output ?' to see a list of valid sub-commands and settings.");
    }
    else if (my_param == thisAgent->outputManager->m_params->agent_traces)
    {
        if (!pArg2)
        {
            PrintCLIMessage(thisAgent->outputManager->m_params->get_agent_channel_string(thisAgent).c_str());
            return true;
        } else {
            int lTraceLevel;
            if (!pArg3)
            {
                return SetError("Wrong number of arguments to output agent-trace command.");
            }
            if (!my_param->validate_string(pArg3->c_str()))
            {
                return SetError("Agent trace channel setting must be 'on' or 'off'. Use 'output ?' to see a list of valid sub-commands.");
            }
            if (pArg3) {
                if (!from_string(lTraceLevel, (*pArg2)) || (lTraceLevel < 1) || (lTraceLevel > maxAgentTraces))
                {
                    tempStringStream << "Agent trace channel must be an integer between 1 and " << maxAgentTraces << ".";
                    return SetError(tempStringStream.str().c_str());
                }
                thisAgent->output_settings->agent_traces_enabled[lTraceLevel-1] = ((*pArg3) == "on");
            } else {
                tempStringStream << "Agent trace channel " << lTraceLevel << " is now" ;
                PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
            }
        }
    }
    else if ((my_param == thisAgent->outputManager->m_params->help_cmd) || (my_param == thisAgent->outputManager->m_params->qhelp_cmd))
    {
        thisAgent->outputManager->m_params->print_output_settings(thisAgent);
    }
    else {
        if (!pArg2)
        {
            /* Sub-command was a variable setting, so print it's value */
            tempStringStream << my_param->get_name() << " is" ;
            PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
        } else {

            if (!my_param->validate_string(pArg2->c_str()))
            {
                return SetError("Invalid argument for output command. Use 'output ?' to see a list of valid sub-commands.");
            }

            bool result = my_param->set_string(pArg2->c_str());

            if (!result)
            {
                return SetError("The output parameter could not be changed.");
            }
            else
            {
                tempStringStream << my_param->get_name() << " is now " << pArg2->c_str();
                PrintCLIMessage(&tempStringStream);
            }
            if (my_param == thisAgent->outputManager->m_params->print_depth)
            {
                thisAgent->outputManager->m_params->update_int_setting(thisAgent, static_cast<soar_module::integer_param*>(my_param));
            } else {
                thisAgent->outputManager->m_params->update_bool_setting(thisAgent, static_cast<soar_module::boolean_param*>(my_param), m_pKernelSML);
            }
            return result;
        }
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
                mode = cli::LOG_CLOSE;
                break;
            case 'A':
                mode = cli::LOG_NEWAPPEND;
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
            return DoCLog(mode, &argv[opt.GetArgument() - opt.GetNonOptionArguments()]);

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
bool CommandLineInterface::DoCommandToFile(const eLogMode mode, const std::string& filename, std::vector< std::string >& argv)
{
    std::string oldResult(m_Result.str());
    m_Result.str("");

    // Fire off command
    SaveOutputSettings();
    bool ret = m_Parser.handle_command(argv);
    RestoreOutputSettings();

    if (!m_Result.str().empty())
    {
        m_Result << std::endl;
    }

    std::string res = m_Result.str();
    m_Result.str("");
    m_Result << oldResult;

    if (!DoCLog(mode, &filename, 0, true))
    {
        return false;
    }

    if (!DoCLog(LOG_ADD, 0, &res, true))
    {
        return false;
    }

    if (!DoCLog(LOG_CLOSE, 0, 0, true))
    {
        return false;
    }

    return ret;
}

void CommandLineInterface::SaveOutputSettings()
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    m_callbacks_were_enabled = thisAgent->output_settings->callback_mode;
    m_output_was_enabled = thisAgent->output_settings->print_enabled;
    m_console_was_enabled = thisAgent->outputManager->is_printing_to_stdout();

    thisAgent->output_settings->callback_mode = true;
    thisAgent->output_settings->print_enabled = true;
    thisAgent->outputManager->set_printing_to_stdout(false);
    thisAgent->outputManager->m_params->update_params_for_settings(thisAgent);
}

void CommandLineInterface::RestoreOutputSettings()
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    thisAgent->output_settings->callback_mode = m_callbacks_were_enabled;
    thisAgent->output_settings->print_enabled = m_output_was_enabled ;
    thisAgent->outputManager->set_printing_to_stdout(m_console_was_enabled);
    thisAgent->outputManager->m_params->update_params_for_settings(thisAgent);
}

bool CommandLineInterface::DoCLog(const eLogMode mode, const std::string* pFilename, const std::string* pToAdd, bool silent)
{
    std::ios_base::openmode openmode = std::ios_base::out;

    switch (mode)
    {
        case LOG_NEWAPPEND:
            openmode |= std::ios_base::app;
        // falls through

        case LOG_NEW:
            if (!pFilename)
            {
                break;    // handle as just a query
            }

            if (m_pLogFile)
            {
                return SetError("Log already open: " + m_LogFilename);
            }

            {
                std::string filename = *pFilename;

                m_pLogFile = new std::ofstream(filename.c_str(), openmode);
                if (!m_pLogFile)
                {
                    return SetError("Failed to open " + filename);
                }

                m_LogFilename = filename;
            }
            SaveOutputSettings();
            break;

        case LOG_ADD:
            if (!m_pLogFile)
            {
                return SetError("Log is not open.");
            }
            (*m_pLogFile) << *pToAdd << std::endl;
            return true;

        case LOG_CLOSE:
            if (!m_pLogFile)
            {
                return SetError("Log is not open.");
            }

            delete m_pLogFile;
            m_pLogFile = 0;
            m_LogFilename.clear();
            RestoreOutputSettings();
            break;

        default:
        case LOG_QUERY:
            break;
    }

    if (!silent)
    {
        if (m_RawOutput)
        {
            m_Result << "Log file ";
            if (IsLogOpen())
            {
                m_Result << "'" + m_LogFilename + "' open.";
            }
            else
            {
                m_Result << "closed.";
            }

        }
        else
        {
            const char* setting = IsLogOpen() ? sml_Names::kTrue : sml_Names::kFalse;
            AppendArgTagFast(sml_Names::kParamLogSetting, sml_Names::kTypeBoolean, setting);

            if (m_LogFilename.size())
            {
                AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, m_LogFilename);
            }
        }
    }
    return true;
}
