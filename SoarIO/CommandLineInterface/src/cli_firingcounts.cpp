#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParseFiringCounts(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	// The number to list defaults to 0 (list all)
	int numberToList = 0;

	// Production defaults to no production
	std::string* pProduction = 0;

	if (argv.size() == 2) {
		// one argument, figure out if it is a positive integer
		if (argv[1][0] == '-') {
			return HandleSyntaxError(Constants::kCLIFiringCounts, "Integer argument must be positive");
		}
		if (isdigit(argv[1][0])) {
			// integer argument, set numberToList
			numberToList = atoi(argv[1].c_str());
			if (numberToList <= 0) {
				return HandleSyntaxError(Constants::kCLIFiringCounts, "Integer argument must be positive");
			}
		} else {
			// non-integer argument, hopfully a production
			pProduction = &(argv[1]);
		}
	} else if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLIFiringCounts, Constants::kCLITooManyArgs);
	}

	return DoFiringCounts(pAgent, pProduction, numberToList);
}

bool CommandLineInterface::DoFiringCounts(gSKI::IAgent* pAgent, std::string* pProduction, int numberToList) {
	if (!RequireAgent(pAgent)) return false;

	unused(pProduction);
	unused(numberToList);

	return HandleError("Not implemented yet.");
}

