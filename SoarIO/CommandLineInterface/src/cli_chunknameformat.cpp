#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseChunkNameFormat(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"count",		2, 0, 'c'},
		{"long",		0, 0, 'l'},
		{"prefix",		2, 0, 'p'},
		{"short",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	bool changeFormat = false;
	bool countFlag = false;
	int count = -1;
	bool patternFlag = false;
	std::string pattern;
	bool longFormat = true;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":c::lp::s", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'c': 
				countFlag = true;
				if (m_pGetOpt->GetOptArg()) {
					if (!IsInteger(m_pGetOpt->GetOptArg())) return SetError(CLIError::kIntegerExpected);
					count = atoi(m_pGetOpt->GetOptArg());
					if (count < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
				}
				break;
			case 'p': 
				patternFlag = true;
				if (m_pGetOpt->GetOptArg()) {
					pattern = std::string(m_pGetOpt->GetOptArg());
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
			case ':':
				return SetError(CLIError::kMissingOptionArg);
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);

	return DoChunkNameFormat(pAgent, changeFormat, longFormat, countFlag ? &count : 0, patternFlag ? &pattern : 0);
}

EXPORT bool CommandLineInterface::DoChunkNameFormat(gSKI::IAgent* pAgent, bool changeFormat, bool longFormat, int* pCount, std::string* pPrefix) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!changeFormat && !pCount && !pPrefix) {
		if (m_RawOutput) {
			m_Result << (pKernelHack->GetSysparam(pAgent, USE_LONG_CHUNK_NAMES) ? "Long" : "Short") << "format.";
		} else {
			AppendArgTagFast(sml_Names::kParamChunkLongFormat, sml_Names::kTypeBoolean, pKernelHack->GetSysparam(pAgent, USE_LONG_CHUNK_NAMES) ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	if (changeFormat) {
		pKernelHack->SetSysparam(pAgent, USE_LONG_CHUNK_NAMES, longFormat);
	}

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
