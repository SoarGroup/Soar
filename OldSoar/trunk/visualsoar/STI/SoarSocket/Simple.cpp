// Simple.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "../STI_Interface/STI_Runtime.h"
#include "../STI_Interface/STI_Tool.h"
#include "../STI_Interface/STI_CommonAPI.h"
#include "../SoarSocket/OSspecific.h"	// For sleep command

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		printf("Usage is: simple <runtime | tool>\n") ;
		return 0 ;
	}

	bool bRuntime = (strcmp(argv[1], "runtime") == 0) ;		
	bool bTool    = (strcmp(argv[1], "tool") == 0) ;

	if (!bRuntime && !bTool)
	{
		printf("Usage is: simple <runtime | tool>\n") ;
		return 0 ;
	}

	// By this point we're either a runtime or a tool.
	char* pName = bRuntime ? "Runtime1" : "Tool1" ;

	// Initialize the library
	STI_Handle hServer = STI_InitInterfaceLibrary(pName, bRuntime) ;

	if (!hServer) return 1 ;

	// Initialize the port
	bool ok = STI_InitListenPort(hServer) ;

	if (!ok) return 2 ;

	// See if we can connect to anyone else
	ok = STI_EstablishConnections(hServer, NULL /* local host */, true /* stop on first not found */) ;

	if (!ok) return 3 ;

	// Pump messages for a fixed amount of time.
	long count = 0 ;
	while (count < 30 && ok)
	{
		count++ ;
		printf("About to pump messages %d\n",count) ;

		ok = STI_PumpMessages(hServer, true /* process all pending messages */) ;

		// Wait a while before pumping again, or it'll all flash by on the screen in a moment.
		SleepMillisecs(1000) ;
	}

	ok = STI_TerminateInterfaceLibrary(hServer) ;

	return 0;
}
