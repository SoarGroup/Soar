#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParsePreferences(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);

	return DoPreferences();
}

bool CommandLineInterface::DoPreferences() {

	return false;
}

//THIS IS GOING TO BE UGLY UNLESS WE REWRITE THE GSKI SIDE

   //   dumpToStdout(objc,objv);

   //   TCL_CONST char** argv = new TCL_CONST char*[objc];
	  //for(int i = 0; i < objc; i++) {
	  //   argv[i] = Tcl_GetStringFromObj(objv[i], 0);
	  //}

   //   gSKI::IAgent* thisAgent = GetThisAgent(interp);

   //   bool rv = m_kernelHack->Preferences(thisAgent, objc, const_cast<char**>(argv));

	  //delete [] argv;

   //   return rv;
