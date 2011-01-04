/////////////////////////////////////////////////////////////////
// time command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include <time.h>

#include "cli_Commands.h"

#include "sml_Names.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoTime(std::vector<std::string>& argv) {

    soar_wallclock_timer real;
    soar_process_timer proc;

    proc.start();
    real.start();

    // Execute command
    bool ret = m_Parser.handle_command(argv);

    real.stop();
    proc.stop();

    double realElapsed = real.get_usec() / 1000000.0;
    double procElapsed = proc.get_usec() / 1000000.0;

    // Print elapsed time and return
    if (m_RawOutput) {
        m_Result << "\n(" << procElapsed << "s) proc" << "\n(" << realElapsed << "s) real";
    } else {
        std::string temp;
        AppendArgTagFast(sml_Names::kParamRealSeconds, sml_Names::kTypeDouble, to_string(realElapsed, temp));
        AppendArgTagFast(sml_Names::kParamProcSeconds, sml_Names::kTypeDouble, to_string(procElapsed, temp));
    }
    return ret;
}

