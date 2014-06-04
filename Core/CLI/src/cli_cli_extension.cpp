/////////////////////////////////////////////////////////////////
// cli command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, Bob Marinier, rmarinie@umich.edu
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoCLIMessage(const std::string& pMessage) {

    /* -- This function is hardcoded to work for tcl right now.  Should generalize
     * so that command can be sent to a variety of cli extensions -- */

    if ((pMessage == "tcl on") || (pMessage == "tcl off"))
    {
        std::string result = this->m_pKernelSML->FireCliExtensionMessageEvent(pMessage.c_str());

        // zero length is success
        if (result.size() == 0) {
            return true;
        }
        return SetError(result);
    } else {
        return SetError("Illegal CLI command " + pMessage);

    }
}

