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
#include "cli_CLIError.h"

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "sml_KernelHelpers.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "gsysparam.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseChunkNameFormat(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'c', "count",		OPTARG_OPTIONAL},
		{'l', "long",		OPTARG_NONE},
		{'p', "prefix",		OPTARG_OPTIONAL},
		{'s', "short",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	bool changeFormat = false;
	bool countFlag = false;
	int count = -1;
	bool patternFlag = false;
	std::string pattern;
	bool longFormat = true;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'c': 
				countFlag = true;
				if (m_OptionArgument.size()) {
					if (!IsInteger(m_OptionArgument)) return SetError(CLIError::kIntegerExpected);
					count = atoi(m_OptionArgument.c_str());
					if (count < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
				}
				break;
			case 'p': 
				patternFlag = true;
				if (m_OptionArgument.size()) {
					pattern = std::string(m_OptionArgument);
				}
				break;
			case 'l':
				changeFormat = true;
				longFormat = true;
				break;
			case 's':
				changeFormat = true;
				longFormat = false;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoChunkNameFormat(changeFormat ? &longFormat : 0, countFlag ? &count : 0, patternFlag ? &pattern : 0);
}

bool CommandLineInterface::DoChunkNameFormat(const bool* pLongFormat, const int* pCount, const std::string* pPrefix) {
	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;

	if (!pLongFormat && !pCount && !pPrefix) {
		if (m_RawOutput) {
			m_Result << "Using " << (pKernelHack->GetSysparam(m_pAgentSML, USE_LONG_CHUNK_NAMES) ? "long" : "short") << " chunk format.";
		} else {
			AppendArgTagFast(sml_Names::kParamChunkLongFormat, sml_Names::kTypeBoolean, pKernelHack->GetSysparam(m_pAgentSML, USE_LONG_CHUNK_NAMES) ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	if (pLongFormat) pKernelHack->SetSysparam(m_pAgentSML, USE_LONG_CHUNK_NAMES, *pLongFormat);

	if (pCount) {
		if (*pCount >= 0) {
			if (*pCount >= pKernelHack->GetSysparam(m_pAgentSML, MAX_CHUNKS_SYSPARAM)) return SetError(CLIError::kCountGreaterThanMaxChunks);
			if (static_cast<unsigned long>(*pCount) < m_pAgentSML->GetSoarAgent()->chunk_count ) return SetError(CLIError::kCountLessThanChunks);
			m_pAgentSML->GetSoarAgent()->chunk_count = *pCount;
		} else {
			// query
			if (m_RawOutput) {
				m_Result << "Chunk count: " << m_pAgentSML->GetSoarAgent()->chunk_count;
			} else {
				char buf[kMinBufferSize];
				AppendArgTagFast(sml_Names::kParamChunkCount, sml_Names::kTypeInt, Int2String(m_pAgentSML->GetSoarAgent()->chunk_count, buf, kMinBufferSize));
			}
		}
	}

	if (pPrefix) {
		if (pPrefix->size()) {
			if ( strchr(pPrefix->c_str(), '*') ) 
			{
				return SetError(CLIError::kInvalidPrefix);
			}
			
			strcpy( m_pAgentSML->GetSoarAgent()->chunk_name_prefix, pPrefix->c_str() );

		} else {
			// query
			if (m_RawOutput) {
				if (pCount && *pCount < 0) m_Result << "\n";
				m_Result << "Prefix: " << m_pAgentSML->GetSoarAgent()->chunk_name_prefix;
			} else {
				AppendArgTagFast(sml_Names::kParamChunkNamePrefix, sml_Names::kTypeString, m_pAgentSML->GetSoarAgent()->chunk_name_prefix);
			}
		}
	}

	return true;
}
