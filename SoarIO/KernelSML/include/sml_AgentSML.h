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

// Map from a client side time tag (as a string) to a client side identifier (e.g. "-35" => "o3")
// This one's only needed so we can do cleanup of our identifier map during removal of WMEs.
// There may be some way to avoid it but I don't see how (and be reasonably fast).
typedef std::map< std::string, std::string >	TimeIdentifierMap ;
typedef TimeIdentifierMap::iterator			TimeIdentifierMapIter ;
typedef TimeIdentifierMap::const_iterator	TimeIdentifierMapConstIter ;

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

	// Used to listen for kernel events that are agent specific
	ProductionListener m_ProductionListener;
	RunListener m_RunListener;
	PrintListener m_PrintListener;
	

	// We have to keep pointers to these objects so that we can release then during an init-soar.  Youch!
	gSKI::IWMObject*	m_InputLinkRoot ;
	gSKI::IWMObject*	m_OutputLinkRoot ;

	// Used to listen for a before removed event
	class AgentBeforeDestroyedListener ;
	AgentBeforeDestroyedListener*	m_pBeforeDestroyedListener ;

public:
	AgentSML(KernelSML* pKernelSML, gSKI::IAgent* pAgent) ;

	~AgentSML() ;

	// Release any objects or other data we are keeping.  We do this just
	// prior to deleting AgentSML, but before the underlying gSKI agent has been deleted
	void Clear() ;

	void RegisterForBeforeAgentDestroyedEvent() ;

	// Release all of the WMEs that we currently have references to
	// It's a little less severe than clear() which releases everything we own, not just wmes.
	void ReleaseAllWmes() ;

	gSKI::IAgent* GetIAgent() { return m_pIAgent ; }

	void SetInputLinkRoot(gSKI::IWMObject* pRoot)   { m_InputLinkRoot = pRoot ; }
	gSKI::IWMObject* GetInputLinkRoot()				{ return m_InputLinkRoot ; }

	void SetOutputLinkRoot(gSKI::IWMObject* pRoot)  { m_OutputLinkRoot = pRoot ; }
	gSKI::IWMObject* GetOutputLinkRoot()			{ return m_OutputLinkRoot ; }

//	void SetOutputListener(OutputListener* pListener)			{ m_pOutputListener = pListener ; }
	OutputListener* GetOutputListener()							{ return m_pOutputListener ; }

	void SetInputProducer(gSKI::IInputProducer* pInputProducer)	{ m_pInputProducer = pInputProducer ; }

	bool SetStopOnOutput(bool state) ;

	void AddProductionListener(egSKIProductionEventId eventID, Connection* pConnection)	{ m_ProductionListener.AddListener(eventID, pConnection) ; }
	void RemoveProductionListener(egSKIProductionEventId eventID, Connection* pConnection) { m_ProductionListener.RemoveListener(eventID, pConnection) ; }	
	void AddRunListener(egSKIRunEventId eventID, Connection* pConnection)	{ m_RunListener.AddListener(eventID, pConnection) ; }
	void RemoveRunListener(egSKIRunEventId eventID, Connection* pConnection) { m_RunListener.RemoveListener(eventID, pConnection) ; }	
	void AddPrintListener(egSKIPrintEventId eventID, Connection* pConnection)	{ m_PrintListener.AddListener(eventID, pConnection) ; }
	void RemovePrintListener(egSKIPrintEventId eventID, Connection* pConnection) { m_PrintListener.RemoveListener(eventID, pConnection) ; }	

	void RemoveAllListeners(Connection* pConnection) ;

	/*************************************************************
	* @brief	Sometimes we wish to temporarily disable and then
	*			re-enable the print callback because we use the Kernel's
	*			print callback to report information that isn't really part of the trace.
	*			(One day we should no longer need to do this).
	*			Enabling/disabling affects all connections.
	*************************************************************/
	void DisablePrintCallback() { m_PrintListener.EnablePrintCallback(false) ; }
	void EnablePrintCallback()  { m_PrintListener.EnablePrintCallback(true) ; }

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
} ;


}

#endif // SML_AGENT_SML_H
