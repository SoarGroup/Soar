/////////////////////////////////////////////////////////////////
// cd command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_KernelSML.h"

using namespace cli;

bool CommandLineInterface::DoCD(const std::string *pDirectory) {
    if (chdir(pDirectory->c_str())) 
        return SetError("Error changing to " + *pDirectory);
    return true;
}

