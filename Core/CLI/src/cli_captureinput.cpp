#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_AgentSML.h"
#include "soar_rand.h"


using namespace cli;
using namespace sml;

bool CommandLineInterface::DoCaptureInput(eCaptureInputMode mode, bool autoflush, std::string* pathname) {
    switch (mode) {
        case CAPTURE_INPUT_CLOSE:
            if (!m_pAgentSML->CaptureQuery()) return SetError("File is not open.");
            if (!m_pAgentSML->StopCaptureInput()) return SetError("Error closing file.");
            break;

        case CAPTURE_INPUT_OPEN:
            {
                if (m_pAgentSML->CaptureQuery()) return SetError("File is already open.");
                if (!pathname || !pathname->size()) return SetError("File name required.");

                uint32_t seed = SoarRandInt();

                if (!m_pAgentSML->StartCaptureInput(*pathname, autoflush, seed))
                    return SetError("Error opening file.");

                m_Result << "Capturing input with random seed: " << seed;
            }
            break;

        case CAPTURE_INPUT_QUERY:
            m_Result << (m_pAgentSML->CaptureQuery() ? "open" : "closed");
            break;
    }

    return true;
}

