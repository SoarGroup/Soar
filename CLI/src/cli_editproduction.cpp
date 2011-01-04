/////////////////////////////////////////////////////////////////
// edit-production command file.
//
// Author: Douglas Pearson
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_KernelSML.h"

using namespace cli;

bool CommandLineInterface::DoEditProduction(std::string production) {
    m_pKernelSML->FireEditProductionEvent(production.c_str()) ;
    return true;
}

