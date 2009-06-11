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
#include "cli_CLIError.h"
#include "misc.h"
#include "sml_Names.h"

using namespace cli;

bool CommandLineInterface::ParseGPMax(std::vector<std::string>& argv) {
	// n defaults to 0 (print current value)
	long n = -1;

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	// one argument, figure out if it is a positive integer
	if (argv.size() == 2) {
		from_string(n, argv[1]);
		if (n < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
	}

	return DoGPMax(n);
}

bool CommandLineInterface::DoGPMax(const long& maximum) {
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

	m_GPMax = maximum;
	return true;

}
