/////////////////////////////////////////////////////////////////
// gp command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, Bob Marinier
// Date  : 2008
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "misc.h"
#include "sml_Names.h"

using namespace cli;

bool CommandLineInterface::DoGPMax(const int& maximum) {
    if (maximum < 0) {
        // query
        if (m_RawOutput) {
            m_Result << m_GPMax;
        } else {
            std::string temp;
            AppendArgTagFast(sml::sml_Names::kParamValue, sml::sml_Names::kTypeInt, to_string(m_GPMax, temp));
        }
        return true;
    }

    m_GPMax = static_cast<size_t>(maximum);
    return true;

}
