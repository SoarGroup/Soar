// This is a simple example of a library that Soar can load at runtime via the `load library` command.
#include "portability.h"

#include "sml_Utils.h"
#include "sml_Client.h"
#include "sml_Names.h"
#include "Export.h"

#include <string>
#include <sstream>
#include <iostream>

using namespace sml;

// C linkage is important for making the library portable across compiler versions
#ifdef __cplusplus
extern "C"
{
#endif

    // Soar calls this when the library is loaded
    RHS_EXPORT const char *sml_InitLibrary(Kernel *pKernel, int argc, char **argv)
    {
        int callbackId = pKernel->AddRhsFunction("test", [](smlRhsEventId id, Agent *pAgent, char const *pFunctionName, char const *pArgument)
                                                 { return "myLibTest"; });
        return "";
    }

#ifdef __cplusplus
} // extern "C"
#endif

//
// Stuff to use the library as a standard client (i.e. as an executable)
//

void MyShutdownHandler(smlSystemEventId id, void *pUserData, Kernel *pKernel)
{
    pKernel->SetConnectionInfo("RHSemotion", sml_Names::kStatusClosing, sml_Names::kStatusClosing);
    exit(0);
}

int main(int argc, char **argv)
{
    sml::Kernel *pKernel = sml::Kernel::CreateRemoteConnection();
    sml_InitLibrary(pKernel, 0, 0);

    assert(pKernel);
    if (pKernel->HadError())
    {
        std::cout << "Error: " << pKernel->GetLastErrorDescription() << std::endl;
        exit(1);
    }

    // Listen for when to shutdown
    pKernel->RegisterForSystemEvent(smlEVENT_BEFORE_SHUTDOWN, MyShutdownHandler, NULL);

    pKernel->SetConnectionInfo("TestExternalLibraryLib", sml_Names::kStatusReady, sml_Names::kStatusReady);

    std::cout << std::endl
              << "This will automatically close when the remote kernel shuts down." << std::endl;
    // stay open until the remote Soar is shutdown
    for (;;)
    {
        sml::Sleep(100000, 0);
    }

    pKernel->SetConnectionInfo("RHSemotion", sml_Names::kStatusClosing, sml_Names::kStatusClosing);
    pKernel->Shutdown();
    delete pKernel;

    return 0;
}
