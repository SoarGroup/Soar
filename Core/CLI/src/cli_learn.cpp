/////////////////////////////////////////////////////////////////
// learn command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "agent.h"
#include "print.h"
#include "ebc.h"
#include "output_manager.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoLearn(const LearnBitset& options)
{
    // No options means print current settings
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (options.none() || options.test(LEARN_LIST))
    {
        std::string tempstr1(""), tempstr2("");
        DoChunk();
        PrintCLIMessage("Warning:  'learn' has been deprecated.  New corresponding command is: chunk");
        return true;
    }

    if (options.test(LEARN_ONLY))
    {
        std::string tempstr1("learn"), tempstr2("only");
        DoChunk('S', &tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_EXCEPT))
    {
        std::string tempstr1("learn"), tempstr2("all-except");
        DoChunk('S', &tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_ENABLE))
    {
        std::string tempstr1("learn"), tempstr2("on");
        DoChunk('S', &tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_DISABLE))
    {
        std::string tempstr1("learn"), tempstr2("off");
        DoChunk('S', &tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_ALL_LEVELS))
    {
        std::string tempstr1("bottom-only"), tempstr2("off");
        DoChunk('S', &tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_BOTTOM_UP))
    {
        std::string tempstr1("bottom-only"), tempstr2("on");
        DoChunk('S', &tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS))
    {
        std::string tempstr1("allow-local-negations"), tempstr2("on");
        DoChunk('S', &tempstr1, &tempstr2);
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS))
    {
        std::string tempstr1("allow-local-negations"), tempstr2("off");
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_ENABLE_THROUGH_EVALUATION_RULES))
    {
        std::string tempstr1("add-osk"), tempstr2("on");
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    if (options.test(LEARN_DISABLE_THROUGH_EVALUATION_RULES))
    {
        std::string tempstr1("add-osk"), tempstr2("off");
        std::ostringstream tempstrstream;
        tempstrstream << "Warning:  'learn' has been deprecated.  New corresponding command is: chunk " << tempstr1 << " " << tempstr2;
        PrintCLIMessage(tempstrstream.str().c_str());
    }

    thisAgent->ebChunker->ebc_params->update_params(thisAgent->ebChunker->ebc_settings);
    thisAgent->ebChunker->update_learning_on();
    return true;
}

