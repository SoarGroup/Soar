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
#include "cli_CLIError.h"

#include "cli_Commands.h"
#include "sml_KernelSML.h"
#include "soar_rand.h"


using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSRand(std::vector<std::string>& argv) {

	if (argv.size() < 2) return DoSRand();

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	uint32_t seed = 0;
	sscanf(argv[1].c_str(), "%u", &seed);
	return DoSRand(&seed);
}

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

