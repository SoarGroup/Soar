/////////////////////////////////////////////////////////////////
// AgentSML class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class is used to keep track of information needed by SML
// (Soar Markup Language) on an agent by agent basis.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_AGENT_SML_H
#define SML_AGENT_SML_H

//#include "sml_AgentListener.h" 
#include "sml_ProductionListener.h"
#include "sml_RunListener.h"
#include "sml_PrintListener.h"
#include "sml_XMLListener.h"
#include "gSKI_Enumerations.h"

// Forward declarations
namespace gSKI {
	class IAgent ;
	class IWme ;
	class IInputProducer ;
}

#include <map>
#include <list>
#include <string>

namespace sml {

// Forward declarations
class OutputListener ;
class KernelSML ;

// Map from a client side identifier to a kernel side one (e.g. "o3" => "O5")
typedef std::map< std::string, std::string >	IdentifierMap ;
typedef IdentifierMap::iterator				IdentifierMapIter ;
typedef IdentifierMap::const_iterator		IdentifierMapConstIter ;

// Map from a client side time tag (as a string) to a kernel side WME* object
// (Had planned to just map the time tag to a kernel time tag...but it turns out
//  there's no quick way to look up an object in the kernel from its time tag).
typedef std::map< std::string, gSKI::IWme* >	TimeTagMap ;
typedef TimeTagMap::iterator				TimeTagMapIter ;
typedef TimeTagMap::const_iterator			TimeTagMapConstIter ;

class AgentSML
{
protected:
	// This is a callback we can register to listen for changes to the output-link
	OutputListener*	m_pOutputListener ;

	// This is a callback we register so we get called back each input phase (so we can check for incoming commands once per decision)
	gSKI::IInputProducer* m_pInputProducer ;

	// A reference to the underlying gSKI agent object
	gSKI::IAgent*	m_pIAgent ;

	// Pointer back to the owning kernel SML object
	KernelSML*		m_pKernelSML ;

	// Map from client side identifiers to kernel side ones
	IdentifierMap	m_IdentifierMap ;

	// Map from client side time tags (as strings) to kernel side WME* objects
	TimeTagMap		m_TimeTagMap ;

	// For cleanup we also need a map from kernel side identifiers to client side ones (for cleanup)
	IdentifierMap	m_ToClientIdentifierMap ;

	// Used to listen for kernel/gSKI events that are agent specific
	ProductionListener	m_ProductionListener;
	RunListener			m_RunListener;
	PrintListener		m_PrintListener;
	XMLListener			m_XMLListener ;

	// We have to keep pointers to these objects so that we can release then during an init-soar.  Youch!
	gSKI::IWMObject*	m_InputLinkRoot ;
	gSKI::IWMObject*	m_OutputLinkRoot ;

	// Used to listen for a before removed event
	class AgentBeforeDestroyedListener ;
	AgentBeforeDestroyedListener*	m_pBeforeDestroyedListener ;

	bool m_SuppressRunEndsEvent ;

	// Used to control runs
	bool m_ScheduledToRun ;
	bool m_WasOnRunList;
	bool m_OnStepList;
	unsigned long m_InitialStepCount ;
	unsigned long m_InitialRunCount ;
	egSKIRunResult m_ResultOfLastRun ;
    unsigned long m_localRunCount;
    unsigned long m_localStepCount;

	  // Used for update world events
	bool m_CompletedOutputPhase ;
	bool m_GeneratedOutput ;
	unsigned long m_OutputCounter ;

public:
	AgentSML(KernelSML* pKernelSML, gSKI::IAgent* pAgent) ;

	~AgentSML() ;

	// Release any objects or other data we are keeping.  We do this just
	// prior to deleting AgentSML, but before the underlying gSKI agent has been deleted
	void Clear(bool deletingThisAgent) ;

	void RegisterForBeforeAgentDestroyedEvent() ;

	// Release all of the WMEs that we currently have references to
	// It's a little less severe than clear() which releases everything we own, not just wmes.
	// If flushPendingRemoves is true, make sure gSKI removes all wmes from Soar's working memory
	// that have been marked for removal but are still waiting for the next input phase to actually
	// be removed (this should generally be correct so we'll default to true for it).
	void ReleaseAllWmes(bool flushPendingRemoves = true) ;

	gSKI::IAgent* GetIAgent() { return m_pIAgent ; }

	void SetInputLinkRoot(gSKI::IWMObject* pRoot)   { m_InputLinkRoot = pRoot ; }
	gSKI::IWMObject* GetInputLinkRoot()				{ return m_InputLinkRoot ; }

	void SetOutputLinkRoot(gSKI::IWMObject* pRoot)  { m_OutputLinkRoot = pRoot ; }
	gSKI::IWMObject* GetOutputLinkRoot()			{ return m_OutputLinkRoot ; }

//	void SetOutputListener(OutputListener* pListener)			{ m_pOutputListener = pListener ; }
	OutputListener* GetOutputListener()							{ return m_pOutputListener ; }

	void SetInputProducer(gSKI::IInputProducer* pInputProducer)	{ m_pInputProducer = pInputProducer ; }

	void SetSuppressRunEndsEvent(bool state) { m_SuppressRunEndsEvent = state ; }
	bool GetSuppressRunEndsEvent()			 { return m_SuppressRunEndsEvent ; }

	void AddProductionListener(egSKIProductionEventId eventID, Connection* pConnection)	{ m_ProductionListener.AddListener(eventID, pConnection) ; }
	void RemoveProductionListener(egSKIProductionEventId eventID, Connection* pConnection) { m_ProductionListener.RemoveListener(eventID, pConnection) ; }	
	void AddRunListener(egSKIRunEventId eventID, Connection* pConnection)	{ m_RunListener.AddListener(eventID, pConnection) ; }
	void RemoveRunListener(egSKIRunEventId eventID, Connection* pConnection) { m_RunListener.RemoveListener(eventID, pConnection) ; }	
	void AddPrintListener(egSKIPrintEventId eventID, Connection* pConnection)	{ m_PrintListener.AddListener(eventID, pConnection) ; }
	void RemovePrintListener(egSKIPrintEventId eventID, Connection* pConnection) { m_PrintListener.RemoveListener(eventID, pConnection) ; }	
	void AddXMLListener(egSKIXMLEventId eventID, Connection* pConnection) { m_XMLListener.AddListener(eventID, pConnection) ; }
	void RemoveXMLListener(egSKIXMLEventId eventID, Connection* pConnection) { m_XMLListener.RemoveListener(eventID, pConnection) ; }

	// Echo the list of wmes received back to any listeners
	void FireInputReceivedEvent(ElementXML const* pCommands) { m_XMLListener.FireInputReceivedEvent(pCommands) ; }

	void RemoveAllListeners(Connection* pConnection) ;

	/*************************************************************
	* @brief	Sometimes we wish to temporarily disable and then
	*			re-enable the print callback because we use the Kernel's
	*			print callback to report information that isn't really part of the trace.
	*			(One day we should no longer need to do this).
	*			Enabling/disabling affects all connections.
	*************************************************************/
	void DisablePrintCallback() { m_PrintListener.EnablePrintCallback(false) ; m_XMLListener.EnablePrintCallback(false) ; }
	void EnablePrintCallback()  { m_PrintListener.EnablePrintCallback(true) ; m_XMLListener.EnablePrintCallback(true) ; }

	/*************************************************************
	* @brief	Trigger an "echo" event.  This event allows one client
	*			to issue a command and have another client (typically the debugger)
	*			listen in on the output.
	*************************************************************/
	void FireEchoEvent(Connection* pConnection, char const* pMessage) { m_PrintListener.HandleEvent(gSKIEVENT_ECHO, m_pIAgent, pMessage) ; m_PrintListener.FlushOutput(pConnection, gSKIEVENT_ECHO) ; }

	/*************************************************************
	* @brief	Converts an id from a client side value to a kernel side value.
	*			We need to be able to do this because the client is adding a collection
	*			of wmes at once, so it makes up the ids for those objects.
	*			But the kernel will assign them a different value when the
	*			wme is actually added in the kernel.
	*************************************************************/
	bool ConvertID(char const* pClientID, std::string* pKernelID) ;
	void RecordIDMapping(char const* pClientID, char const* pKernelID) ;
	void RemoveID(char const* pKernelID) ;

	/*************************************************************
	* @brief	Converts a time tag from a client side value to
	*			a kernel side one.
	*************************************************************/
	gSKI::IWme* ConvertTimeTag(char const* pTimeTag) ;
	void RecordTimeTag(char const* pTimeTag, gSKI::IWme* pWme) ;
	void RemoveTimeTag(char const* pTimeTag) ;

	void RecordLongTimeTag(long timeTag, gSKI::IWme* pWme) ;
	void RemoveLongTimeTag(long timeTag) ;

	/*************************************************************
	* @brief	Used to select which agents run on the next run command.
	*************************************************************/
	void ScheduleAgentToRun(bool state) ;
	void RemoveAgentFromRunList()       { m_ScheduledToRun = false ;}
	bool IsAgentScheduledToRun()		{ return m_ScheduledToRun ; }
	void PutAgentOnStepList(bool state) { m_OnStepList = state; }
	bool IsAgentOnStepList()		    { return m_OnStepList ; }
	bool WasAgentOnRunList()            { return m_WasOnRunList ; }

	egSKIRunResult	GetResultOfLastRun()		  { return m_ResultOfLastRun ; }
	void SetResultOfRun(egSKIRunResult runResult) { m_ResultOfLastRun = runResult ; }

	void SetInitialStepCount(unsigned long count)	{ m_InitialStepCount = count ; }
	void SetInitialRunCount(unsigned long count)	{ m_InitialRunCount = count ; }
	unsigned long GetInitialStepCount()				{ return m_InitialStepCount ; }
	unsigned long GetInitialRunCount()				{ return m_InitialRunCount ; }
	void ResetLocalRunCounters()                    { m_localRunCount = 0 ; m_localStepCount = 0 ; }
	void IncrementLocalRunCounter()                 { m_localRunCount++ ; }
	void IncrementLocalStepCounter()                { m_localStepCount++ ; }
	bool CompletedRunType(unsigned long count)      { return (count > (m_InitialRunCount + m_localRunCount)) ; }
	void SetCompletedOutputPhase(bool state)		{ m_CompletedOutputPhase = state ; }
	bool HasCompletedOutputPhase()					{ return m_CompletedOutputPhase ; }

	void SetGeneratedOutput(bool state)				{ m_GeneratedOutput = state ; }
	bool HasGeneratedOutput() 						{ return m_GeneratedOutput ; }

	void SetInitialOutputCount(unsigned long count)	{ m_OutputCounter = count ; }
	unsigned long GetInitialOutputCount()			{ return m_OutputCounter ; }
} ;


}

#endif // SML_AGENT_SML_H
