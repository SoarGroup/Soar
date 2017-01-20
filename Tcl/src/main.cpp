/*
 * main.c
 *
 *  Created on: Oct 29, 2013
 *      Author: mazzin
 */

#include "portability.h"
#include "Export.h"
#include "TclSoarLib.h"
#include <string.h>

#define libName "TclSoarLib"

using namespace sml;

// Might need this for windows dll.  Might also need some includes.
#ifdef _WIN32

BOOL APIENTRY DllMain(HANDLE hModule,          \
                      DWORD  ul_reason_for_call, \
                      LPVOID lpReserved)         \
{
    \
    return TRUE;
    \
}

#endif // _WIN32

namespace TclSoar
{
    static TclSoarLib* gTclLib = NULL;
}

#ifdef __cplusplus
extern "C"
#endif
{
#include <iostream>

RHS_EXPORT void* sml_LibraryMessage(const char* pMessage, void* pMessageData)
{
    std::cout << "TclSoarLib::sml_LibraryMessage(" << pMessage << ")" << std::endl;

    /* -- create and delete should not be directly accessed by the user.  Ideally
     *    we should pass in a parameter indicating whether the command originated
     *    from the user or the soar code. -- */
    if (!strcmp(pMessage, "delete"))
    {
        if (TclSoar::gTclLib)
        {
            delete TclSoar::gTclLib;
            TclSoar::gTclLib = NULL;
            return ((void*) true);
        }
    }
    else if (!strcmp(pMessage, "on"))
    {
        if (TclSoar::gTclLib)
        {
			return ((void*) TclSoar::gTclLib->turnOn());
        }
    }
    else if (!strcmp(pMessage, "off"))
    {
        if (TclSoar::gTclLib)
        {
            return ((void*) TclSoar::gTclLib->turnOff());
        }
    }
    
    return NULL;
}

RHS_EXPORT const char* sml_InitLibrary(Kernel* pKernel, int argc, char** argv)
{
    if (!TclSoar::gTclLib)
    {
        TclSoar::gTclLib = new TclSoarLib(pKernel);
        Soar_Instance::Get_Soar_Instance().Register_Library(pKernel, libName, sml_LibraryMessage);
    }
    return "";
}


} // endif extern "C"

