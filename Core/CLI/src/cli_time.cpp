/////////////////////////////////////////////////////////////////
// time command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"

#include <time.h>

#include "cli_Commands.h"

#include "sml_Names.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoTime(std::vector<std::string>& argv)
{

    soar_timer timer;
    
    timer.start();
    
    // Execute command
    bool ret = m_Parser.handle_command(argv);
    
    timer.stop();
    
    double elapsed = timer.get_usec() / 1000000.0;
    
    // Print elapsed time and return
    if (m_RawOutput)
    {
        m_Result << "\n(" << elapsed << "s) real";
    }
    else
    {
        std::string temp;
        AppendArgTagFast(sml_Names::kParamRealSeconds, sml_Names::kTypeDouble, to_string(elapsed, temp));
    }
    return ret;
}

