/////////////////////////////////////////////////////////////////
// source command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include <fstream>

#include "cli_Commands.h"
#include "sml_StringOps.h"
#include "sml_Names.h"
#include "sml_Events.h"
#include "misc.h"

#include <algorithm>

#include <assert.h>

using namespace cli;
using namespace sml;
using namespace std;

bool CommandLineInterface::ParseSource(std::vector<std::string>& argv) 
{
    Options optionsData[] = 
    {
        {'a', "all",			OPTARG_NONE},
        {'d', "disable",		OPTARG_NONE},
        {'v', "verbose",		OPTARG_NONE},
        {0, 0, OPTARG_NONE}
    };

    SourceBitset options(0);

    for (;;) 
    {
        if (!ProcessOptions(argv, optionsData)) return false;
        if (m_Option == -1) break;

        switch (m_Option) 
        {
        case 'd':
            options.set(SOURCE_DISABLE);
            break;
        case 'a':
            options.set(SOURCE_ALL);
            break;
        case 'v':
            options.set(SOURCE_VERBOSE);
            break;
        default:
            return SetError(kGetOptError);
        }
    }

    if (m_NonOptionArguments < 1) 
    {
        SetErrorDetail("Please supply one file to source.");
        return SetError(kTooFewArgs);

    } 
    else if (m_NonOptionArguments > 2) 
    {
        SetErrorDetail("Please supply one file to source. If there are spaces in the path, enclose it in quotes.");
        return SetError(kSourceOnlyOneFile);
    }

    return DoSource(argv[m_Argument - m_NonOptionArguments], &options);
}

void CommandLineInterface::PrintSourceSummary(int sourced, const std::list< std::string >& excised, int ignored)
{
    if (!m_SourceFileStack.empty())
        AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, m_SourceFileStack.top());
    std::string temp;
    AppendArgTag(sml_Names::kParamSourcedProductionCount, sml_Names::kTypeInt, to_string(sourced, temp));
    AppendArgTag(sml_Names::kParamExcisedProductionCount, sml_Names::kTypeInt, to_string(excised.size(), temp));
    AppendArgTag(sml_Names::kParamIgnoredProductionCount, sml_Names::kTypeInt, to_string(ignored, temp));

    if (!excised.empty())
    {
        std::list< std::string >::const_iterator iter = excised.begin();
        while (iter != excised.end()) 
        {
            AppendArgTagFast( sml_Names::kParamName, sml_Names::kTypeString, *iter );
            ++iter;
        }
    }

    if (m_RawOutput) 
    {
        if (m_SourceFileStack.empty())
            m_Result << "Total";
        else
            m_Result << m_SourceFileStack.top();
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
            m_Result << " " << ignored << " production" << ((ignored == 1) ? " " : "s ") << "ignored.";
        m_Result << "\n";
    }
}

bool CommandLineInterface::DoSource(std::string path, SourceBitset* pOptions) 
{
    if (m_SourceFileStack.size() >= 100)
        return SetError(kSourceDepthExceeded);

    normalize_separators(path);

    // Separate the path out of the filename if any
    std::string filename;
    std::string folder;
    std::string::size_type lastSeparator = path.rfind('/');
    if (lastSeparator == std::string::npos) 
        filename.assign(path);
    else
    {
        ++lastSeparator;
        if (lastSeparator < path.length()) 
        {
            folder = path.substr(0, lastSeparator);
            filename.assign(path.substr(lastSeparator, path.length() - lastSeparator));
        }
    }

    if (!folder.empty()) if (!DoPushD(folder)) return false;

    FILE* pFile = fopen ( filename.c_str() , "rb" );
    if (!pFile) 
    {
        if (!folder.empty()) DoPopD();
        SetErrorDetail(path);
        return SetError(kOpenFileFail);
    }

    // obtain file size:
    fseek(pFile, 0, SEEK_END);
    long lSize = ftell(pFile);
    rewind(pFile);

    // allocate memory to contain the whole file:
    char* buffer = reinterpret_cast<char*>(malloc(sizeof(char)*(lSize + 1)));
    if (!buffer) 
    {
        if (!folder.empty()) DoPopD();
        path.insert(0, "Memory allocation failed: ");
        SetErrorDetail(path);
        return SetError(kOpenFileFail);
    }

    // copy the file into the buffer:
    size_t result = fread(buffer, 1, lSize, pFile);
    if (result != lSize)
    {
        free(buffer);
        if (!folder.empty()) DoPopD();
        path.insert(0, "Read failed: ");
        SetErrorDetail(path);
        return SetError(kOpenFileFail);
    }
    buffer[lSize] = 0;

    // close file
    fclose (pFile);

    if (m_SourceFileStack.empty())
    {
        m_pSourceOptions = pOptions;

        m_NumProductionsSourced = 0;
        m_ExcisedDuringSource.clear();
        m_NumProductionsIgnored = 0;

        m_NumTotalProductionsSourced = 0;
        m_TotalExcisedDuringSource.clear();
        m_NumTotalProductionsIgnored = 0;

        m_SourceErrorDetail.clear();

        // Need to listen for excise callbacks
		if (m_pAgentSML)
    		this->RegisterWithKernel(smlEVENT_BEFORE_PRODUCTION_REMOVED);
    }

    std::string temp;
    GetCurrentWorkingDirectory(temp);
    temp.push_back('/');
    temp.append(filename);
    m_SourceFileStack.push(temp);

    cli::Tokenizer tokenizer;
    tokenizer.SetHandler(this);
    bool ret = tokenizer.Evaluate(buffer);
    if (!ret)
    {
        int line = tokenizer.GetCommandLineNumber();
        int offset = -1;
        if (m_LastError == kNoError)
        {
            SetError(kParseError);
            SetErrorDetail(tokenizer.GetErrorString());
            line = tokenizer.GetCurrentLineNumber();
            offset = tokenizer.GetOffset();
        }

        m_SourceErrorDetail.append("\n\t");
        m_SourceErrorDetail.append(m_SourceFileStack.top());
        m_SourceErrorDetail.append(":");
        m_SourceErrorDetail.append(to_string(line, temp));
        if (offset > 0)
        {
            m_SourceErrorDetail.append(":");
            m_SourceErrorDetail.append(to_string(line, temp));
        }
    }

    if (m_pSourceOptions && m_pSourceOptions->test(SOURCE_ALL))
        PrintSourceSummary(m_NumProductionsSourced, m_ExcisedDuringSource, m_NumProductionsIgnored);

    m_SourceFileStack.pop();

    m_NumTotalProductionsSourced += m_NumProductionsSourced;
    m_TotalExcisedDuringSource.insert(m_TotalExcisedDuringSource.end(), m_ExcisedDuringSource.begin(), m_ExcisedDuringSource.end());
    m_NumTotalProductionsIgnored += m_NumProductionsIgnored;

    m_NumProductionsSourced = 0;
    m_ExcisedDuringSource.clear();
    m_NumProductionsIgnored = 0;

    if (m_SourceFileStack.empty())
    {
		if (m_pAgentSML)
			this->UnregisterWithKernel(smlEVENT_BEFORE_PRODUCTION_REMOVED);

        if (m_pSourceOptions && !m_pSourceOptions->test(SOURCE_DISABLE))
            PrintSourceSummary(m_NumTotalProductionsSourced, m_TotalExcisedDuringSource, m_NumTotalProductionsIgnored);

        m_pSourceOptions = 0;
        if (!m_SourceErrorDetail.empty())
            m_LastErrorDetail.append(m_SourceErrorDetail);
    }

    if (!folder.empty()) DoPopD();

    free(buffer);
    return ret;
}
