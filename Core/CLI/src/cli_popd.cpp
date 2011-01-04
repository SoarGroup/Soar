/////////////////////////////////////////////////////////////////
// popd command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

using namespace cli;

bool CommandLineInterface::DoPopD() {

    // There must be a directory on the stack to pop
    if (m_DirectoryStack.empty()) return SetError("Directory stack is empty.");

    // Change to the directory
    if (!DoCD(&(m_DirectoryStack.top()))) return false;    // error handled in DoCD

    // Pop the directory stack
    m_DirectoryStack.pop();
    return true;
}

