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

bool CommandLineInterface::DoCD(const std::string* pDirectory) {

    // if directory 0, return SoarLibrary/bin
    if (!pDirectory) 
    {
        std::string binDir(this->m_pKernelSML->GetLibraryLocation());
        binDir.append("bin");

        if (chdir(binDir.c_str())) 
            return SetError("Error changing to " + binDir);
        return true;
    }
   
    std::string dir = *pDirectory;

    // Change to directory
    if (chdir(dir.c_str())) 
        return SetError("Error changing to " + dir);
    return true;
}

