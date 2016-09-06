/////////////////////////////////////////////////////////////////
// internal-symbols command file.
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

#include "agent.h"
#include "symbol.h"
#include "symbol_manager.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoInternalSymbols()
{
    m_pAgentSML->GetSoarAgent()->symbolManager->print_internal_symbols();
    return true;
}

