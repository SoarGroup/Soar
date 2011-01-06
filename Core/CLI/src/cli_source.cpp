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
#include "sml_AgentSML.h"
#include "sml_Events.h"
#include "misc.h"

#include <algorithm>

#include <assert.h>

using namespace cli;
using namespace sml;
using namespace std;

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
        return SetError("Source depth (100) exceeded, possible recursive source.");

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
		return SetError("Failed to open file for reading: " + path);
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
        return SetError("Failed to open file for reading: " + path);
    }

    // copy the file into the buffer:
    size_t result = fread(buffer, 1, lSize, pFile);
    if (result != lSize)
    {
        free(buffer);
        if (!folder.empty()) DoPopD();
        path.insert(0, "Read failed: ");
        return SetError("Failed to open file for reading: " + path);
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

        // Need to listen for excise callbacks
		if (m_pAgentSML)
    		this->RegisterWithKernel(smlEVENT_BEFORE_PRODUCTION_REMOVED);
    }

    std::string temp;
    GetCurrentWorkingDirectory(temp);
    temp.push_back('/');
    temp.append(filename);
    m_SourceFileStack.push(temp);

    soar::tokenizer tokenizer;
    tokenizer.set_handler(&m_Parser);
    bool ret = tokenizer.evaluate(buffer);
    if (!ret)
    {
        int line = tokenizer.get_command_line_number();
        int offset = -1;
		
		if (m_LastError.empty())
		{
			if (!m_Parser.GetError().empty())
				m_LastError = m_Parser.GetError();
			else if (tokenizer.get_error_string())
			{
				m_LastError = tokenizer.get_error_string();
				line = tokenizer.get_current_line_number();
				offset = tokenizer.get_offset();
			}
        }

        m_LastError.append("\n\t");
        m_LastError.append(m_SourceFileStack.top());
        m_LastError.append(":");
        m_LastError.append(to_string(line, temp));
        if (offset > 0)
        {
            m_LastError.append(":");
            m_LastError.append(to_string(line, temp));
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
    }

    if (!folder.empty()) DoPopD();

    free(buffer);
    return ret;
}
