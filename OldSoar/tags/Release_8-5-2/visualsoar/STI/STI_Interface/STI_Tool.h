
#ifndef STI_TOOL_H
#define STI_TOOL_H

/* The following ifdef block is the standard way of creating macros which make exporting  */
/* from a DLL simpler. All files within this DLL are compiled with the STI_TOOL_EXPORTS */
/* symbol defined on the command line. this symbol should not be defined on any project */
/* that uses this DLL. This way any other project whose source files include this file see  */
/* STI_TOOL_API functions as being imported from a DLL, wheras this DLL sees symbols */
/* defined with this macro as being exported. */
#ifdef _WINDOWS
#ifdef STI_INTERFACE_EXPORTS
#define STI_TOOL_API	__declspec(dllexport)
#else
#define STI_TOOL_API	__declspec(dllimport)
#endif
#else	/* _WINDOWS */
#define STI_TOOL_API
#endif	/* _WINDOWS */

#ifdef __cplusplus
extern "C" {
#endif

#include "STI_CommonAPI.h"

/* Send a production to runtime */
STI_TOOL_API bool	STI_SendProduction(STI_Handle hServer, char const* pProductionName, char const* pProductionBody) ;

/* Send a file of productions to runtime */
STI_TOOL_API bool	STI_SendFile(STI_Handle hServer, char const* pFileName) ;

/* Run the matches command on a production in the runtime */
STI_TOOL_API bool STI_ProductionMatches(STI_Handle hServer, char const* pProductionName) ;

/* Excises (removes) a production from production memory in the runtime */
STI_TOOL_API bool STI_ExciseProduction(STI_Handle hServer, char const* pProductionName) ;

/* Send a raw command string to the runtime where it is executed. */
STI_TOOL_API bool STI_SendRawCommand(STI_Handle hServer, char const* pCommandString) ;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif	/* STI_TOOL_H */


