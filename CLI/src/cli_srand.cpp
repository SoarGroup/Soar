/////////////////////////////////////////////////////////////////
// srand command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_KernelSML.h"
#include "soar_rand.h"


using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSRand(uint32_t* pSeed) {
    if (pSeed) 
    {
        SoarSeedRNG( *pSeed );
    }
    else
    {
        SoarSeedRNG();
    }

    return true;
}

