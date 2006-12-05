#include "sml_Client.h"
#include "Export.h"
//#include "interface.h"

#include <string>
#include <sstream>

using namespace sml;
using namespace std;

string myRHSTest(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	return "Success!";
}

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void sml_InitLibrary(Kernel* pKernel, void* pUserData) {
	int callbackId = pKernel->AddRhsFunction("test", myRHSTest, 0);
}

#ifdef __cplusplus
} // extern "C"
#endif