#ifndef STI_COMMONAPI_H
#define STI_COMMONAPI_H

/* The following ifdef block is the standard way of creating macros which make exporting */
/* from a DLL simpler. All files within this DLL are compiled with the STI_TOOL_EXPORTS */
/* symbol defined on the command line. this symbol should not be defined on any project */
/* that uses this DLL. This way any other project whose source files include this file see */
/* STI_COMMON_API functions as being imported from a DLL, wheras this DLL sees symbols */
/* defined with this macro as being exported. */
#ifdef _WINDOWS

#ifdef STI_INTERFACE_EXPORTS
#define STI_COMMON_API	__declspec(dllexport)
#else
#define STI_COMMON_API	__declspec(dllimport)
#endif

#else	// _WINDOWS
#define STI_COMMON_API
#endif	// _WINDOWS

#ifdef __cplusplus
extern "C" {
#endif

/* Define an STI_Handle as a pointer to this empty structure to give users */
/* a typesafe way to pass STI_Handle's in and out.  In fact it's a pointer */
/* to a class, but that's not the client's business--to the client this is an */
/* arbitrary handle.  If this is ever a problem, it can be typedef'd to a long. */
typedef struct STI_InterfaceStructTag
{
	unsigned int n;	/* So that we compiles under 'C' */
} STI_InterfaceStruct, *STI_Handle ;

/* Call once at startup.  Returns handle to server object. */
STI_COMMON_API STI_Handle	STI_InitInterfaceLibrary(char const* pName, bool bIsRuntime) ;

/* Call this to initialize the listening port */
STI_COMMON_API bool			STI_InitListenPort(STI_Handle hServer);

/* Call to connect to other interface libraries */
STI_COMMON_API bool			STI_EstablishConnections(STI_Handle hServer, char const* pRemoteIPAddress, bool bStopOnFirstNotFound) ;

/* Get the number of current connections to other libraries */
STI_COMMON_API long			STI_GetNumberConnections(STI_Handle hServer) ;

/* Get the name of a specific connection */
/* Returns null if connection out of range. */
STI_COMMON_API char const*	STI_GetConnectionName(STI_Handle hServer, long index) ;

/* Get the port of a specific connection */
/* Returns -1 if connection out of range */
STI_COMMON_API long			STI_GetConnectionPort(STI_Handle hServer, long index) ;

/* Get the port we're listening on */
STI_COMMON_API long			STI_GetListenPort(STI_Handle hServer) ;

/* Get the index for a connection from its name */
STI_COMMON_API long			STI_GetConnectionIndexByName(STI_Handle hServer, char const* pName) ;

/* Get the index for a connection from its port */
STI_COMMON_API long			STI_GetConnectionIndexByPort(STI_Handle hServer, long port) ;

/* Enable or disable a specific connection */
STI_COMMON_API void			STI_EnableConnectionByIndex(STI_Handle hServer, long index, bool bEnable) ;
STI_COMMON_API void			STI_EnableConnectionByName(STI_Handle hServer, char const* pName, bool bEnable) ;

/* Returns whether or not a specific connection is enabled */
STI_COMMON_API bool			STI_IsConnectionEnabledByIndex(STI_Handle hServer, long index) ;
STI_COMMON_API bool			STI_IsConnectionEnabledByName(STI_Handle hServer, char const* pName) ;

/* Enable or disable all connections */
STI_COMMON_API void			STI_EnableAllConnections(STI_Handle hServer, bool bEnable) ;

/* Call regularly to send and receive connection requests/messages */
/* This could be put into a separate thread later. */
/* It sends messages that have been queued and reads messages waiting to come in. */
STI_COMMON_API bool			STI_PumpMessages(STI_Handle hServer, bool bProcessAllPendingMessages) ;

/* Call after pumping messages when you are ready to process new commands */
STI_COMMON_API bool			STI_IsIncomingCommandAvailable(STI_Handle hServer) ;

/* Get the latest incoming command numeric values. */
/* Returns false if there is no incoming command available. */
STI_COMMON_API bool			STI_GetIncomingCommand1(STI_Handle hServer, long* pCommandID, long* pCommandFlags,
													long* pDataSize, long* pSystemMsg,
													long* pParam1, long* pParam2, long* pParam3,
													long* pParam4, long* pParam5, long* pParam6) ;

/* Returns NULL if there is no incoming command available. */
/* The returned buffer is not owned by the caller--copy to keep it. */
STI_COMMON_API char const*	STI_GetIncomingCommandStringParam1(STI_Handle hServer) ;

/* Returns NULL if there is no incoming command available. */
/* The returned buffer is not owned by the caller--copy to keep it. */
/* Use memcpy to copy with the size returned in long *pDataSize of GetIncomingCommand1 */
/* to support embedded null's in the stream. */
STI_COMMON_API char const*	STI_GetIncomingCommandData(STI_Handle hServer) ;

/* Remove the top incoming command from the queue */
STI_COMMON_API void			STI_PopIncomingCommand(STI_Handle hServer) ;

/* Send a command by putting it in the queue.  It is sent later */
/* when PumpMessages is called. */
STI_COMMON_API bool			STI_SendCommandAsynch1(STI_Handle hServer, long commandID, long commandFlags, long dataSize,
									   			   long param1, long param2, long param3, long param4,
									   			   long param5, long param6, char const* pStringParam1,
									   			   char const* pDataBuffer) ;

/* Send a command now and wait for a response. */
STI_COMMON_API bool			STI_SendCommandSynch1(STI_Handle hServer, long commandID, long commandFlags, long dataSize,
									  			  long param1, long param2, long param3, long param4,
									  		  	  long param5, long param6, char const* pStringParam1,
									  			  char const* pDataBuffer) ;
/* Call once at shutdown */
STI_COMMON_API bool			STI_TerminateInterfaceLibrary(STI_Handle hServer) ;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif	/* STI_COMMONAPI_H */


