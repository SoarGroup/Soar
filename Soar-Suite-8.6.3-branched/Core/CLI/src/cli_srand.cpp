/////////////////////////////////////////////////////////////////
// srand command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSRand(gSKI::Agent* pAgent, std::vector<std::string>& argv) {

	unused(pAgent);

	if (argv.size() < 2) return DoSRand();

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	unsigned long int seed = 0;
	sscanf(argv[1].c_str(), "%lu", &seed);
	return DoSRand(&seed);
}

bool CommandLineInterface::DoSRand(unsigned long int* pSeed) {

	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	pKernelHack->SeedRandomNumberGenerator(pSeed);
	return true;
}

