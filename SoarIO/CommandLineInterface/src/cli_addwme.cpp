#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParseAddWME(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
    if (argv.size() < 4) return HandleSyntaxError(Constants::kCLIAddWME, Constants::kCLITooFewArgs);

	unsigned attributeIndex = (argv[2] == "^") ? 3 : 2;

    if (argv.size() < (attributeIndex + 2)) return HandleSyntaxError(Constants::kCLIAddWME, Constants::kCLITooFewArgs);
    if (argv.size() > (attributeIndex + 3)) return HandleSyntaxError(Constants::kCLIAddWME, Constants::kCLITooManyArgs);

	bool acceptable = false;
	if (argv.size() > (attributeIndex + 2)) {
		if (argv[attributeIndex + 2] != "+") return HandleSyntaxError(Constants::kCLIAddWME, "Expected acceptable preference (+) or nothing, got '" + argv[attributeIndex + 2] + "'.");
		acceptable = true;
	}

	return DoAddWME(pAgent, argv[1], argv[attributeIndex], argv[attributeIndex + 1], acceptable);
}

bool CommandLineInterface::DoAddWME(gSKI::IAgent* pAgent, std::string id, std::string attribute, std::string value, bool acceptable) {
	unused(pAgent);
	unused(id);
	unused(attribute);
	unused(value);
	unused(acceptable);

	return false;
}


    //if (soar_cAddWme(argv[1], argv[attr_index], argv[attr_index + 1], acceptable, &psw) <= 0) {
    //    return SOAR_ERROR;
    //}

    ///* SW NOTE
    // * The old way to do this is commented out below
    // * the reason for the change is that print_wme_for_tcl
    // * in only used here, and moreover, we don't want to 
    // * use a print call back since wmes added using this method
    // * will get added into the log file which really doesn't
    // * make a whole lot of sense.
    // */
    ///*
    //   soar_cPushCallback((soar_callback_agent) soar_agent, 
    //   PRINT_CALLBACK,
    //   (soar_callback_fn) cb_soarResult_AppendResult, 
    //   (soar_callback_data) res,
    //   (soar_callback_free_fn) NULL);

    //   print_wme_for_tcl((wme *)psw);
    //   soar_cPopCallback((soar_callback_agent) soar_agent, PRINT_CALLBACK);
    // */

    //setSoarResultResult(res, "%lu: ", ((wme *) psw)->timetag);
    //appendSymbolsToSoarResultResult(res, "%y ^%y %y", ((wme *) psw)->id, ((wme *) psw)->attr, ((wme *) psw)->value);
    //if (((wme *) psw)->acceptable)
    //    appendSoarResultResult(res, " +");

    //return SOAR_OK;
