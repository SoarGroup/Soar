//////////////////////////////////////////////////////////////////////
// ClientSML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class owns the Connection* used to communicate with the Soar
// kernel and could retain pointers to other objects (e.g. IKernel)
// if that proves useful.
//
//////////////////////////////////////////////////////////////////////

#include "sml_ClientSML.h"
#include "sml_Connection.h"
#include "sml_ClientIInputProducer.h"
#include "sml_ClientIOutputProcessor.h"
#include "sml_ClientWMObject.h"
#include "sml_ClientWorkingMemory.h"

using namespace sml ;

ClientSML::ClientSML(void)
{
	m_Connection = 0 ;
}

ClientSML::~ClientSML(void)
{
	// We don't currently own these objects, so we clear out the maps (so they don't delete their contents).
	// If we later decide this client should take ownership we can remove these.
	m_InputProducers.clear(false) ;
	m_OutputProcessors.clear(false) ;

	// We own the list of working memory objects, so we want
	// to delete them.
	m_WorkingMemories.clear(true) ;
}

void ClientSML::InputProducerUpdate(Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Get the id of the input producer this message is for
	char const* pThisID		= pIncoming->GetArgValue(sml_Names::kParamThis) ;
	char const* pWMID		= pIncoming->GetArgValue(sml_Names::kParamWorkingMemory) ;
	char const* pWMObjectID = pIncoming->GetArgValue(sml_Names::kParamWmeObject) ;

	// Find the input producer object (on the client side)
	IInputProducer* pInputProducer = (IInputProducer*)m_InputProducers.lookup(pThisID) ;

	// Check that we found the producer
	if (!pInputProducer)
		pConnection->AddErrorToSMLResponse(pResponse, "InputProducer update message for an unknown input producer", -1) ;

	// BUGBUG: We need to decide if gSKI creates this object or not.
	// If it does, then correct users will release it and delete it.
	// If gSKI doesn't create this object, then our users will need to explicitly
	// delete (*not* release) the object in their input processors.
	WMObject* pWMObject = new WMObject(pWMObjectID, this) ;

	// See if we've already created this WorkingMemory object
	IWorkingMemory* pWM = (IWorkingMemory*)m_WorkingMemories.lookup(pWMID) ;
	if (!pWM)
	{
		pWM = new WorkingMemory(pWMID, this) ;
		RecordWorkingMemory(pWM) ;
	}

	pInputProducer->Update(pWM, pWMObject) ;
}

void ClientSML::OutputProcessorUpdate(Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Get the id of the input producer this message is for
	char const* pThisID		= pIncoming->GetArgValue(sml_Names::kParamThis) ;
	char const* pWMID		= pIncoming->GetArgValue(sml_Names::kParamWorkingMemory) ;
	char const* pWMObjectID = pIncoming->GetArgValue(sml_Names::kParamWmeObject) ;

	// Find the output processor object (on the client side)
	IOutputProcessor* pOutputProcessor = (IOutputProcessor*)m_OutputProcessors.lookup(pThisID) ;

	// Check that we found the producer
	if (!pOutputProcessor)
		pConnection->AddErrorToSMLResponse(pResponse, "OutputProcessor process output message for an unknown output processor", -1) ;

	// BUGBUG: We need to decide if gSKI creates this object or not.
	// If it does, then correct users will release it and delete it.
	// If gSKI doesn't create this object, then our users will need to explicitly
	// delete (*not* release) the object in their input processors.
	WMObject* pWMObject = new WMObject(pWMObjectID, this) ;

	// See if we've already created this WorkingMemory object
	IWorkingMemory* pWM = (IWorkingMemory*)m_WorkingMemories.lookup(pWMID) ;
	if (!pWM)
	{
		pWM = new WorkingMemory(pWMID, this) ;
		RecordWorkingMemory(pWM) ;
	}

	pOutputProcessor->ProcessOutput(pWM, pWMObject) ;
}

ElementXML* ClientSML::ReceivedCallFromKernel(Connection* pConnection, ElementXML* pIncomingMsg)
{
	AnalyzeXML incoming ;
	incoming.Analyze(pIncomingMsg) ;

	// Create the reply.
	ElementXML* pResponse = pConnection->CreateSMLResponse(pIncomingMsg) ;

	// See what type of message this is from the kernel
	if (strcmp(incoming.GetCommandName(), sml_Names::kgSKI_IInputProducer_Update) == 0)
	{
		// Input producer callback
		InputProducerUpdate(pConnection, &incoming, pResponse) ;
	}
	else if (strcmp(incoming.GetCommandName(), sml_Names::kgSKI_IOutputProcessor_ProcessOutput) == 0)
	{
		// Output processor callback
		OutputProcessorUpdate(pConnection, &incoming, pResponse) ;
	}

	return pResponse ;
}

void ClientSML::RecordInputProducer(IInputProducer* pInput)
{
	m_InputProducers.add(pInput) ;
}

void ClientSML::RecordOutputProcessor(IOutputProcessor* pOutput)
{
	m_OutputProcessors.add(pOutput) ;
}

void ClientSML::RecordWorkingMemory(IWorkingMemory* pWM)
{
	m_WorkingMemories.add(pWM) ;
}
