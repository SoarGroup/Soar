/////////////////////////////////////////////////////////////////
// chunk-name-format command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "gsysparam.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoChunkNameFormat(const bool* pLongFormat, const int64_t* pCount, const std::string* pPrefix)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (!pLongFormat && !pCount && !pPrefix)
    {
        if (m_RawOutput)
        {
            m_Result << "Using " << (thisAgent->sysparams[USE_LONG_CHUNK_NAMES] ? "long" : "short") << " chunk format.";
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamChunkLongFormat, sml_Names::kTypeBoolean, thisAgent->sysparams[USE_LONG_CHUNK_NAMES] ? sml_Names::kTrue : sml_Names::kFalse);
        }
        return true;
    }
    
    if (pLongFormat)
    {
        set_sysparam(thisAgent, USE_LONG_CHUNK_NAMES, *pLongFormat);
    }
    
    if (pCount)
    {
        if (*pCount >= 0)
        {
            if (*pCount >= thisAgent->sysparams[MAX_CHUNKS_SYSPARAM])
            {
                return SetError("Cannot set count greater than the max-chunks sysparam.");
            }
            
            if (static_cast<uint64_t>(*pCount) < thisAgent->chunk_count)
            {
                return SetError("Cannot set chunk count less than the current number of chunks.");
            }
            
            thisAgent->chunk_count = static_cast<uint64_t>(*pCount);
        }
        else
        {
            // query
            if (m_RawOutput)
            {
                m_Result << "Chunk count: " << thisAgent->chunk_count;
            }
            else
            {
                std::string temp;
                AppendArgTagFast(sml_Names::kParamChunkCount, sml_Names::kTypeInt, to_string(thisAgent->chunk_count, temp));
            }
        }
    }
    
    if (pPrefix)
    {
        if (pPrefix->size())
        {
            if (strchr(pPrefix->c_str(), '*'))
            {
                return SetError("Failed to set prefix (does it contain a '*'?).");
            }
            
            strcpy(thisAgent->chunk_name_prefix, pPrefix->c_str());
            
        }
        else
        {
            // query
            if (m_RawOutput)
            {
                if (pCount && *pCount < 0)
                {
                    m_Result << "\n";
                }
                m_Result << "Prefix: " << thisAgent->chunk_name_prefix << "\n";
            }
            else
            {
                AppendArgTagFast(sml_Names::kParamChunkNamePrefix, sml_Names::kTypeString, thisAgent->chunk_name_prefix);
            }
        }
    }
    
    return true;
}
