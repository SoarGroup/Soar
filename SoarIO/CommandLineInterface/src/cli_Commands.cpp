#include <string>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <fstream>

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#include <direct.h>

#define snprintf _snprintf

#endif // WIN32

#include "cli_GetOpt.h"	// Not the real getopt, as that one has crazy side effects with windows libraries
#include "cli_Constants.h"

// gSKI includes
#include "gSKI_Structures.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Production.h"
#include "IgSKI_MultiAttribute.h"
#include "IgSKI_AgentPerformanceMonitor.h"

// BADBAD: I think we should be using an error class instead to work with error objects.
#include "../../gSKI/src/gSKI_Error.h"

// SML includes
#include "sml_Connection.h"
#include "sml_StringOps.h"

using namespace std;
using namespace sml;

// Templates for new additions
//bool CommandLineInterface::Parse(std::vector<std::string>& argv) {
//	return Do();
//}
//
//bool CommandLineInterface::Do() {
//	m_Result += "TODO: ";
//	return true;
//}


