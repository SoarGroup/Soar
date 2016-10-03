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
#include "sml_Events.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"
#include "sml_StringOps.h"
#include "sml_Utils.h"

#include "agent.h"
#include "cmd_settings.h"
#include "misc.h"
#include "lexer.h"
#include "output_manager.h"
#include "parser.h"
#include "print.h"
#include "production.h"
#include "soar_rand.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "rete.h"

#include <algorithm>
#include <assert.h>
#include <fstream>

using namespace cli;
using namespace sml;
using namespace std;

bool CommandLineInterface::DoLoad(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->load_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->load_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid load command.  Use 'load ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->load_params->input_cmd)
    {
        return ParseReplayInput(argv);
    }
    else if (my_param == thisAgent->command_params->load_params->file_cmd)
    {
        return ParseSource(argv);
    }
    else if (my_param == thisAgent->command_params->load_params->rete_cmd)
    {
        return ParseReteLoad(argv);
    }
    else if (my_param == thisAgent->command_params->load_params->library_cmd)
    {
        return ParseLoadLibrary(argv);
    }
    else if ((my_param == thisAgent->command_params->load_params->help_cmd) || (my_param == thisAgent->command_params->load_params->qhelp_cmd))
    {
        thisAgent->command_params->load_params->print_settings(thisAgent);
    }
    return false;
}

bool CommandLineInterface::DoSave(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->save_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->save_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid save command.  Use 'save ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->save_params->input_cmd)
    {
        return ParseCaptureInput(argv);
    }
    else if (my_param == thisAgent->command_params->save_params->rete_cmd)
    {
        return ParseReteSave(argv);
    }
    else if ((my_param == thisAgent->command_params->save_params->help_cmd) || (my_param == thisAgent->command_params->save_params->qhelp_cmd))
    {
        thisAgent->command_params->save_params->print_settings(thisAgent);
    }
    return false;
}
bool CommandLineInterface::ParseReplayInput(std::vector< std::string >& argv)
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

    return DoReplayInput(mode, mode == Cli::REPLAY_INPUT_OPEN ? &pathname : 0);
}
bool CommandLineInterface::ParseSource(std::vector< std::string >& argv)
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

    if (opt.GetNonOptionArguments() < 2)
    {
        return SetError("Syntax: load file [--all | --disable | --verbose] <filename>");
    }
    else if (opt.GetNonOptionArguments() > 3)
    {
        return SetError("Please supply one file to source. If there are spaces in the path, enclose it in quotes.");
    }

    return DoSource(argv[opt.GetArgument() - opt.GetNonOptionArguments() + 1], &options);
}
bool CommandLineInterface::ParseReteLoad(std::vector< std::string >& argv)
{
    cli::Options opt;
    OptionsData optionsData[] =
    {
        {'l', "load",        OPTARG_REQUIRED},
        {'r', "restore",    OPTARG_REQUIRED},
        {'s', "save",        OPTARG_REQUIRED},
        {0, 0, OPTARG_NONE}
    };

    bool load = false;
    std::string filename;

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
            case 'l':
            case 'r':
                load = true;
                filename = opt.GetOptionArgument();
                break;
            case 's':
                return ParseReteSave(argv);
                break;
        }
    }

    // Must have a save or load operation
    if (!load)
    {
        return SetError("Syntax: load rete-net --load <filename>");
    }

    // No additional arguments
    if (!opt.CheckNumNonOptArgs(1, 1))
    {
        return SetError(opt.GetError().c_str());
    }

    return DoReteNet(false, filename);
}

bool CommandLineInterface::ParseLoadLibrary(std::vector< std::string >& argv)
{
    // command-name library-name [library-args ...]

    if (argv.size() < 2)
    {
        return SetError("Syntax: load library <filename>");
    }

    // strip the command name, combine the rest
    std::string libraryCommand(argv[2]);
    for (std::string::size_type i = 3; i < argv.size(); ++i)
    {
        libraryCommand += " ";
        libraryCommand += argv[i];
    }

    return DoLoadLibrary(libraryCommand);
}
bool CommandLineInterface::ParseCaptureInput(std::vector< std::string >& argv)
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
        {
            return SetError(opt.GetError());
        }

        if (opt.GetOption() == -1)
        {
            break;
        }

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

    return DoCaptureInput(mode, autoflush, mode == Cli::CAPTURE_INPUT_OPEN ? &pathname : 0);
}
bool CommandLineInterface::ParseReteSave(std::vector< std::string >& argv)
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
    std::string filename;

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
            case 'r':
            case 'l':
                return ParseReteLoad(argv);
                break;
            case 's':
                save = true;
                filename = opt.GetOptionArgument();
                break;
        }
    }

    // Must have a save or load operation
    if (!save)
    {
        return SetError("Syntax: save rete-network --save <filename>");
    }
    // No additional arguments
    if (!opt.CheckNumNonOptArgs(1, 1))
    {
        return SetError(opt.GetError().c_str());
    }

    return DoReteNet(true, filename);
}
bool CommandLineInterface::DoReplayInput(eReplayInputMode mode, std::string* pathname)
{
    switch (mode)
    {
        case REPLAY_INPUT_CLOSE:
            if (!m_pAgentSML->ReplayQuery())
            {
                return SetError("File is not open.");
            }
            if (!m_pAgentSML->StopReplayInput())
            {
                return SetError("File close operation failed.");
            }
            break;

        case REPLAY_INPUT_OPEN:
            if (m_pAgentSML->ReplayQuery())
            {
                return SetError("File is already open.");
            }
            if (!pathname)
            {
                return SetError("No filename given.");
            }
            if (!pathname->size())
            {
                return SetError("No filename given.");
            }

            if (!m_pAgentSML->StartReplayInput(*pathname))
            {
                return SetError("Open file failed.");
            }
            m_Result << "Loaded " << m_pAgentSML->NumberOfCapturedActions() << " actions.";
            break;

        case REPLAY_INPUT_QUERY:
            m_Result << (m_pAgentSML->ReplayQuery() ? "open" : "closed");
            break;
    }

    return true;
}

bool CommandLineInterface::DoCaptureInput(eCaptureInputMode mode, bool autoflush, std::string* pathname)
{
    switch (mode)
    {
        case CAPTURE_INPUT_CLOSE:
            if (!m_pAgentSML->CaptureQuery())
            {
                return SetError("File is not open.");
            }
            if (!m_pAgentSML->StopCaptureInput())
            {
                return SetError("Error closing file.");
            }
            break;

        case CAPTURE_INPUT_OPEN:
        {
            if (m_pAgentSML->CaptureQuery())
            {
                return SetError("File is already open.");
            }
            if (!pathname || !pathname->size())
            {
                return SetError("File name required.");
            }

            uint32_t seed = SoarRandInt();

            if (!m_pAgentSML->StartCaptureInput(*pathname, autoflush, seed))
            {
                return SetError("Error opening file.");
            }

            m_Result << "Capturing input with random seed: " << seed;
        }
        break;

        case CAPTURE_INPUT_QUERY:
            m_Result << (m_pAgentSML->CaptureQuery() ? "open" : "closed");
            break;
    }

    return true;
}

bool CommandLineInterface::DoReteNet(bool save, std::string filename)
{
    if (!filename.size())
    {
        return SetError("Missing file name.");
    }

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (save)
    {
        FILE* file = fopen(filename.c_str(), "wb");

        if (file == 0)
        {
            return SetError("Open file failed.");
        }

        if (! save_rete_net(thisAgent, file, true))
        {
            // TODO: additional error information
            return SetError("Rete save operation failed.");
        }

        fclose(file);

    }
    else
    {
        FILE* file = fopen(filename.c_str(), "rb");

        if (file == 0)
        {
            return SetError("Open file failed.");
        }

        if (! load_rete_net(thisAgent, file))
        {
            // TODO: additional error information
            return SetError("Rete load operation failed.");
        }

        fclose(file);
    }

    return true;
}

bool CommandLineInterface::DoLoadLibrary(const std::string& libraryCommand)
{

    std::string result = this->m_pKernelSML->FireLoadLibraryEvent(libraryCommand.c_str());

    // zero length is success
    if (result.size() == 0)
    {
        return true;
    }

    return SetError("load library failed: " + result);
}

void CommandLineInterface::PrintSourceSummary(int sourced, const std::list< std::string >& excised, int ignored)
{
    if (!m_SourceFileStack.empty())
    {
        AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, m_SourceFileStack.top());
    }
    std::string temp;
    AppendArgTag(sml_Names::kParamSourcedProductionCount, sml_Names::kTypeInt, to_string(sourced, temp));
    AppendArgTag(sml_Names::kParamExcisedProductionCount, sml_Names::kTypeInt, to_string(excised.size(), temp));
    AppendArgTag(sml_Names::kParamIgnoredProductionCount, sml_Names::kTypeInt, to_string(ignored, temp));

    if (!excised.empty())
    {
        std::list< std::string >::const_iterator iter = excised.begin();
        while (iter != excised.end())
        {
            AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, *iter);
            ++iter;
        }
    }

    if (m_RawOutput)
    {
        if (m_SourceFileStack.empty())
        {
            m_Result << "Total";
        }
        else
        {
            m_Result << m_SourceFileStack.top();
        }
        m_Result << ": " << sourced << " production" << ((sourced == 1) ? " " : "s ") << "sourced.";

        if (!excised.empty())
        {
            m_Result << " " << excised.size() << " production" << ((excised.size() == 1) ? " " : "s ") << "excised.";
            if (m_pSourceOptions && m_pSourceOptions->test(SOURCE_VERBOSE))
            {
                // print excised production names
                m_Result << "\nExcised productions:";

                std::list< std::string >::const_iterator iter = excised.begin();
                while (iter != excised.end())
                {
                    m_Result << "\n\t" << (*iter);
                    ++iter;
                }
            }
        }
        if (ignored)
        {
            m_Result << " " << ignored << " production" << ((ignored == 1) ? " " : "s ") << "ignored.";
        }
        m_Result << "\n";
    }
}

bool CommandLineInterface::DoSource(std::string path, SourceBitset* pOptions)
{
    if (m_SourceFileStack.size() >= 100)
    {
        return SetError("Source depth (100) exceeded, possible recursive source.");
    }

    normalize_separators(path);

    // Separate the path out of the filename if any
    std::string filename;
    std::string folder;
    std::string::size_type lastSeparator = path.rfind('/');
    if (lastSeparator == std::string::npos)
    {
        filename.assign(path);
    }
    else
    {
        ++lastSeparator;
        if (lastSeparator < path.length())
        {
            folder = path.substr(0, lastSeparator);
            filename.assign(path.substr(lastSeparator, path.length() - lastSeparator));
        }
    }

    if (!folder.empty()) if (!DoPushD(folder))
        {
            return false;
        }

    FILE* pFile = fopen(filename.c_str() , "rb");
    if (!pFile)
    {
        if (!folder.empty())
        {
            DoPopD();
        }
        return SetError("Failed to open file for reading: " + path);
    }

    // obtain file size:
    fseek(pFile, 0, SEEK_END);
    long lSize = ftell(pFile);
    rewind(pFile);

    // allocate memory to contain the whole file:
    char* buffer = reinterpret_cast<char*>(malloc(sizeof(char) * (lSize + 1)));
    if (!buffer)
    {
        if (!folder.empty())
        {
            DoPopD();
        }
        path.insert(0, "Memory allocation failed: ");
        fclose(pFile);
        return SetError("Failed to open file for reading: " + path);
    }

    // copy the file into the buffer:
    size_t result = fread(buffer, 1, lSize, pFile);
    if (result != lSize)
    {
        free(buffer);
        if (!folder.empty())
        {
            DoPopD();
        }
        path.insert(0, "Read failed: ");
        fclose(pFile);
        return SetError("Failed to open file for reading: " + path);
    }
    buffer[lSize] = 0;

    // close file
    fclose(pFile);

    if (m_SourceFileStack.empty())
    {
        m_pSourceOptions = pOptions;

        m_NumProductionsSourced = 0;
        m_ExcisedDuringSource.clear();
        m_NumProductionsIgnored = 0;

        m_NumTotalProductionsSourced = 0;
        m_TotalExcisedDuringSource.clear();
        m_NumTotalProductionsIgnored = 0;

        // Need to listen for excise callbacks
        if (m_pAgentSML)
        {
            this->RegisterWithKernel(smlEVENT_BEFORE_PRODUCTION_REMOVED);
        }
    }

    std::string temp;
    GetCurrentWorkingDirectory(temp);
    temp.push_back('/');
    temp.append(filename);
    m_SourceFileStack.push(temp);

    if (m_pSourceOptions && m_pSourceOptions->test(SOURCE_VERBOSE))
    {
        if (m_RawOutput)
        {
            m_Result << "Sourcing " << filename.c_str() << ".\n";
        }
        else
        {
            std::string outString;
            outString.assign("Sourcing ");
            outString.append(filename);
            outString.append(".\n");
            AppendArgTagFast(sml_Names::kParamChunkNamePrefix, sml_Names::kTypeString, outString);
        }
    }
    bool ret = Source(buffer, true);

    if (m_pSourceOptions && m_pSourceOptions->test(SOURCE_ALL))
    {
        PrintSourceSummary(m_NumProductionsSourced, m_ExcisedDuringSource, m_NumProductionsIgnored);
    }

    m_SourceFileStack.pop();

    if ((m_NumProductionsSourced + m_NumProductionsIgnored) > 0)
    {
        m_Result << "\n";
    }
    m_NumTotalProductionsSourced += m_NumProductionsSourced;
    m_TotalExcisedDuringSource.insert(m_TotalExcisedDuringSource.end(), m_ExcisedDuringSource.begin(), m_ExcisedDuringSource.end());
    m_NumTotalProductionsIgnored += m_NumProductionsIgnored;

    m_NumProductionsSourced = 0;
    m_ExcisedDuringSource.clear();
    m_NumProductionsIgnored = 0;

    if (m_SourceFileStack.empty())
    {
        if (m_pAgentSML)
        {
            this->UnregisterWithKernel(smlEVENT_BEFORE_PRODUCTION_REMOVED);
        }
        agent* thisAgent = m_pAgentSML->GetSoarAgent();
        if (m_pSourceOptions && !m_pSourceOptions->test(SOURCE_DISABLE))
        {
            PrintSourceSummary(m_NumTotalProductionsSourced, m_TotalExcisedDuringSource, m_NumTotalProductionsIgnored);
        }

        m_pSourceOptions = 0;
    }

    if (!folder.empty())
    {
        DoPopD();
    }

    free(buffer);
    return ret;
}

bool CommandLineInterface::Source(const char* buffer, bool printFileStack)
{
    soar::tokenizer tokenizer;
    tokenizer.set_handler(&m_Parser);
    if (tokenizer.evaluate(buffer))
    {
        return true;
    }

    int line = tokenizer.get_command_line_number();
    int offset = -1;

    std::string sourceError;
    if (m_LastError.empty())
    {
        if (!m_Parser.GetError().empty())
        {
            sourceError = m_Parser.GetError();
        }
        else if (tokenizer.get_error_string())
        {
            sourceError = tokenizer.get_error_string();
            line = tokenizer.get_current_line_number();
            offset = tokenizer.get_offset();
        }
    }
    if (printFileStack)
    {
        std::string temp;
        sourceError.append("\n\t");
        sourceError.append(m_SourceFileStack.top());
        sourceError.append(":");
        sourceError.append(to_string(line, temp));
        if (offset > 0)
        {
            sourceError.append(":");
            sourceError.append(to_string(line, temp));
        }
    }
    AppendError(sourceError);
    return false;
}
