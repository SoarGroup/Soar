
#ifndef STI_RUNTIME_H
#define STI_RUNTIME_H

/* The following ifdef block is the standard way of creating macros which make exporting  */
/* from a DLL simpler. All files within this DLL are compiled with the STI_RUNTIME_EXPORTS */
/* symbol defined on the command line. this symbol should not be defined on any project */
/* that uses this DLL. This way any other project whose source files include this file see  */
/* STI_RUNTIME_API functions as being imported from a DLL, wheras this DLL sees symbols */
/* defined with this macro as being exported. */
#ifdef _WINDOWS
#ifdef STI_INTERFACE_EXPORTS
#define STI_RUNTIME_API	__declspec(dllexport)
#else
#define STI_RUNTIME_API	__declspec(dllimport)
#endif
#else	/* _WINDOWS */
#define STI_RUNTIME_API
#endif	/* _WINDOWS */

#ifdef __cplusplus
extern "C" {
#endif

#include "STI_CommonAPI.h"

/* Send edit production command to tool */
STI_RUNTIME_API bool			STI_EditProduction(STI_Handle hServer, char const* pProductionName) ;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif	/* STI_RUNTIME_H */
