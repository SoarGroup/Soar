/////////////////////////////////////////////////////////////////
// Sample logging application for SML
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Feb 2006
//
// This application shows an example of how to build a simple
// logging tool.  To use it you would run a Soar kernel somewhere
// (in an environment or inside the debugger for example) and then
// run this logger.  The logger listens for certain events and
// then creates a log file.
//
// The idea is that you could take this simple app and modify it
// to log what you need for your specific application, outputing the
// data in whatever format you want.
//
// This sample is broken into two parts:
// LoggerWinC.cpp contains all of the Windows specific code for putting up a little
//                window to control the logging.
// Logger.cpp contains the SML code for actually doing the logging.
//
// This way if you're not using Windows or wish to add logging to an existing app
// the part you're interested in is Logger.cpp
//
/////////////////////////////////////////////////////////////////

#include "sml_Client.h"
#include "Logger.h"
#include <stdlib.h>
#include <string>

using namespace sml ;

static FILE* gOutputFile = 0 ;
static Kernel* gKernel = 0 ;

/////////////////////////////////////////////////////////////////
//
// This handler is called with parsed XML data from the execution trace.
// There's a lot of interesting stuff in here that could be logged.
// If you need to log things that don't appear in the trace, you can register
// for other events (like production firings) and log extra information.
//
// If you don't want to process the trace but just capture it consider
// listening for the smlEVENT_PRINT event instead which sends the trace just as strings.
//
/////////////////////////////////////////////////////////////////
void MyXMLEventHandler(smlXMLEventId id, void* pUserData, Agent* pAgent, ClientXML* pXML)
{
	// Setting this flag to true will dump the entire XML trace into the log file
	// That can be helpful if you'd like to process it later or you'd like to see what
	// information is available.
	const bool kLogRawXML = false ;

	if (kLogRawXML)
	{
		char* pStr = pXML->GenerateXMLString(true) ;
		fprintf(gOutputFile, "%s\n", pStr) ;
		pXML->DeleteString(pStr) ;
	}

	// This will always succeed.  If this isn't really trace XML
	// the methods checking on tag names etc. will just fail
	ClientTraceXML* pRootXML = pXML->ConvertToTraceXML() ;

	// The root object is just a <trace> tag.  The substance is in the children
	// so we'll get the first child which should exist.
	ClientTraceXML childXML ;
	bool ok = pRootXML->GetChild(&childXML, 0) ;
	ClientTraceXML* pTraceXML = &childXML ;

	if (!ok)
		return ;

	// To figure out the format of the XML you can either look at the ClientTraceXML class
	// (which has methods grouped appropriately) or you can look at the XML output directly.
	// It's designed to be readable, so looking at a few packets you should quickly get the hang
	// of what's going on and what attributes are available to be read.
	// Here are a couple of examples.

	// Check if this is a new state
	if (pTraceXML->IsTagState())
	{
		std::string count = pTraceXML->GetDecisionCycleCount() ;
		std::string stateID = pTraceXML->GetStateID() ;
		std::string impasseObject = pTraceXML->GetImpasseObject() ;
		std::string impasseType = pTraceXML->GetImpasseType() ;

		fprintf(gOutputFile, "New state at decision %s was %s (%s %s)\n", count.c_str(), stateID.c_str(), impasseObject.c_str(), impasseType.c_str()) ;
	}

	// Check if this is a new operator
	if (pTraceXML->IsTagOperator())
	{
		std::string count = pTraceXML->GetDecisionCycleCount() ;
		std::string operatorID = pTraceXML->GetOperatorID() ;
		std::string operatorName = pTraceXML->GetOperatorName() ;

		fprintf(gOutputFile, "Operator at decision %s was %s name %s\n", count.c_str(), operatorID.c_str(), operatorName.c_str()) ;
	}

	// Flushing the file means we can look at it while it's still being formed
	// at the cost of a small reduction in performance.
	fflush(gOutputFile) ;
}

/////////////////////////////////////////////////////////////////
//
// Start logging to pFilename.
// 
// This method connects to an existing kernel and starts capturing logging output.
// If append is true the new logged output is added to the end of the file.
// 
/////////////////////////////////////////////////////////////////
bool StartLogging(char const* pFilename, bool append, std::string* pErrorMsg)
{
	if (append)
		gOutputFile = fopen(pFilename, "a") ;
	else
		gOutputFile = fopen(pFilename, "w") ;

	if (!gOutputFile)
	{
		*pErrorMsg = "Failed to open log file " ;
		*pErrorMsg += pFilename ;

		return false ;
	}

	// Connect to the kernel we wish to capture logging information from.
	gKernel = sml::Kernel::CreateRemoteConnection(true, NULL) ;

	if (gKernel->HadError())
	{
		*pErrorMsg = gKernel->GetLastErrorDescription() ;

		delete gKernel ;
		gKernel = 0 ;
		
		return false ;
	}

	// This logger just logs the first agent's output.
	// It could easily be extended to log an agent whose name we pass in or to log
	// all agents if that's what you need.
	Agent* pAgent = gKernel->GetAgentByIndex(0) ;

	if (!pAgent)
	{
		*pErrorMsg = "Connected to kernel, but there are no agents running" ;
		
		gKernel->Shutdown() ;
		delete gKernel ;
		gKernel = 0 ;

		return false ;
	}

	// Start listening for XML trace events.
	pAgent->RegisterForXMLEvent(smlEVENT_XML_TRACE_OUTPUT, MyXMLEventHandler, NULL) ;

	// We'll also explicitly set the watch level to level 1 here.  This determines how much
	// output is sent out to the event handler.  This setting is for the agent, so if you change it in
	// the debugger (e.g. to watch 0) it will affect what gets logged.  This is a limitation in the kernel.
	pAgent->ExecuteCommandLine("watch 1") ;

	return true ;
}

/////////////////////////////////////////////////////////////////
//
// Stop logging and disconnect from the remote Soar kernel.
// 
/////////////////////////////////////////////////////////////////
bool StopLogging()
{
	if (gOutputFile)
	{
		fclose(gOutputFile) ;
		gOutputFile = 0 ;
	}

	if (gKernel)
	{
		gKernel->Shutdown() ;
		delete gKernel ;
		gKernel = 0 ;
	}

	return true ;
}

bool IsLogging()
{
	return (gOutputFile && gKernel) ;
}

