/////////////////////////////////////////////////////////////////
// capture-input command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2008
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_AgentSML.h"
#include "soar_rand.h"


using namespace cli;
using namespace sml;

// capture-input --open pathname [--flush]
// capture-input [--query]
// capture-input --close
bool CommandLineInterface::ParseCaptureInput(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'c', "close", OPTARG_NONE},
		{'f', "flush", OPTARG_NONE},
		{'o', "open", OPTARG_REQUIRED},
		{'q', "query", OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	eCaptureInputMode mode = CAPTURE_INPUT_QUERY;
	std::string pathname;

	bool autoflush = false;
	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'c':
				mode = CAPTURE_INPUT_CLOSE;
				break;
			case 'f':
				autoflush = true;
				break;
			case 'o':
				mode = CAPTURE_INPUT_OPEN;
				pathname = m_OptionArgument;
				break;
			case 'q':
				mode = CAPTURE_INPUT_QUERY;
				break;
			default:
				return SetError(kGetOptError);
		}
	}

	return DoCaptureInput(mode, autoflush, mode == CAPTURE_INPUT_OPEN ? &pathname : 0);
}

bool CommandLineInterface::DoCaptureInput(eCaptureInputMode mode, bool autoflush, std::string* pathname) {
	switch (mode) {
		case CAPTURE_INPUT_CLOSE:
			if (!m_pAgentSML->CaptureQuery()) return SetError(kFileNotOpen);
			if (!m_pAgentSML->StopCaptureInput())
			{
				return SetError(kCloseFileFail);
			} 
			break;

		case CAPTURE_INPUT_OPEN:
			{
				if (m_pAgentSML->CaptureQuery()) return SetError(kFileOpen);
				if (!pathname) return SetError(kMissingFilenameArg);
				if (!pathname->size()) return SetError(kMissingFilenameArg);

				uint32_t seed = SoarRandInt();

				if (!m_pAgentSML->StartCaptureInput(*pathname, autoflush, seed))
				{
					return SetError(kOpenFileFail);
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

