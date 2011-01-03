/////////////////////////////////////////////////////////////////
// replay-input command file.
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


using namespace cli;
using namespace sml;

bool CommandLineInterface::DoReplayInput(eReplayInputMode mode, std::string* pathname) {
	switch (mode) {
		case REPLAY_INPUT_CLOSE:
			if (!m_pAgentSML->ReplayQuery()) return SetError("File is not open.");
			if (!m_pAgentSML->StopReplayInput())
			{
				return SetError("File close operation failed.");
			} 
			break;

		case REPLAY_INPUT_OPEN:
			if (m_pAgentSML->ReplayQuery()) return SetError("File is already open.");
			if (!pathname) return SetError("No filename given.");
			if (!pathname->size()) return SetError("No filename given.");

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

