/////////////////////////////////////////////////////////////////
// chunk-name-format command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "gSKI_DoNotTouch.h"
#include "gSKI_Kernel.h"
#include "gsysparam.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseChunkNameFormat(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'c', "count",		2},
		{'l', "long",		0},
		{'p', "prefix",		2},
		{'s', "short",		0},
		{0, 0, 0}
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

	return DoChunkNameFormat(pAgent, changeFormat ? &longFormat : 0, countFlag ? &count : 0, patternFlag ? &pattern : 0);
}

bool CommandLineInterface::DoChunkNameFormat(gSKI::Agent* pAgent, const bool* pLongFormat, const int* pCount, const std::string* pPrefix) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!pLongFormat && !pCount && !pPrefix) {
		if (m_RawOutput) {
			m_Result << "Using " << (pKernelHack->GetSysparam(pAgent, USE_LONG_CHUNK_NAMES) ? "long" : "short") << " chunk format.";
		} else {
			AppendArgTagFast(sml_Names::kParamChunkLongFormat, sml_Names::kTypeBoolean, pKernelHack->GetSysparam(pAgent, USE_LONG_CHUNK_NAMES) ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	if (pLongFormat) pKernelHack->SetSysparam(pAgent, USE_LONG_CHUNK_NAMES, *pLongFormat);

	if (pCount) {
		if (*pCount >= 0) {
			if (*pCount >= pKernelHack->GetSysparam(pAgent, MAX_CHUNKS_SYSPARAM)) return SetError(CLIError::kCountGreaterThanMaxChunks);
			if (static_cast<unsigned long>(*pCount) < pKernelHack->GetChunkCount(pAgent)) return SetError(CLIError::kCountLessThanChunks);
			pKernelHack->SetChunkCount(pAgent, *pCount);
		} else {
			// query
			if (m_RawOutput) {
				m_Result << "Chunk count: " << pKernelHack->GetChunkCount(pAgent);
			} else {
				char buf[kMinBufferSize];
				AppendArgTagFast(sml_Names::kParamChunkCount, sml_Names::kTypeInt, Int2String(pKernelHack->GetChunkCount(pAgent), buf, kMinBufferSize));
			}
		}
	}

	if (pPrefix) {
		if (pPrefix->size()) {
			if (!pKernelHack->SetChunkNamePrefix(pAgent, pPrefix->c_str())) return SetError(CLIError::kInvalidPrefix);
		} else {
			// query
			if (m_RawOutput) {
				if (pCount && *pCount < 0) m_Result << "\n";
				m_Result << "Prefix: " << pKernelHack->GetChunkNamePrefix(pAgent);
			} else {
				AppendArgTagFast(sml_Names::kParamChunkNamePrefix, sml_Names::kTypeString, pKernelHack->GetChunkNamePrefix(pAgent));
			}
		}
	}

	return true;
}
