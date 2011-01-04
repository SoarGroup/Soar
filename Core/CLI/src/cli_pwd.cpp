/////////////////////////////////////////////////////////////////
// pwd command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoPWD() {

    std::string directory;
    bool ret = GetCurrentWorkingDirectory(directory);

    if (directory.size()) {
        if (m_RawOutput) {
            m_Result << directory;
        } else {
            AppendArgTagFast(sml_Names::kParamDirectory, sml_Names::kTypeString, directory);
        }
    }

    return ret;
}

