#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

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

#include "sml_AgentSML.h"
#include "sml_OutputListener.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"

#include "gSKI_Events.h"
#include "IgSKI_Wme.h"
#include "gSKI_Agent.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_Symbol.h"
#include "gSKI_AgentManager.h"
#include "gSKI_Kernel.h"
#include "IgSKI_InputLink.h"
#include "IgSKI_WorkingMemory.h"

#ifdef _DEBUG
// Comment this in to debug init-soar and inputwme::update calls
//#define DEBUG_UPDATE
#endif

#ifdef DEBUG_UPDATE
#include "sock_Debug.h"	// For PrintDebugFormat
#endif

#include <assert.h>

using namespace sml ;

AgentSML::AgentSML(KernelSML* pKernelSML, gSKI::Agent* pAgent) : /*m_AgentListener(pKernelSML, pAgent)*/ m_ProductionListener(pKernelSML, pAgent), m_RunListener(pKernelSML, pAgent), m_PrintListener(pKernelSML, pAgent), m_XMLListener(pKernelSML, pAgent)
{
	m_pKernelSML = pKernelSML ;
	m_pIAgent = pAgent ;
	m_pInputProducer = NULL ;
	m_InputLinkRoot = NULL ;
	m_OutputLinkRoot = NULL ;
	m_SuppressRunEndsEvent = false ;
	m_WasOnRunList = false;
	m_ScheduledToRun = false ;
	m_OnStepList = false;
	m_ResultOfLastRun = gSKI_RUN_COMPLETED ;
	m_InitialStepCount = 0 ;
	m_InitialRunCount = 0 ;
	m_CompletedOutputPhase = false ;
	m_GeneratedOutput = false ;
	m_OutputCounter = 0 ;
    m_localRunCount = 0 ;
    m_localStepCount= 0 ;

	m_pBeforeDestroyedListener = NULL ;

	// Create a listener for output events and other events we listen for
	m_pOutputListener = new OutputListener(pKernelSML, pAgent) ;

	// For KernelSML (us) to work correctly we need to listen for certain events, independently of what any client is interested in
	// Currently:
	// Listen for output callback events so we can send this output over to the clients
	// Listen for "before" init-soar events (we need to know when these happen so we can release all WMEs on the input link, otherwise gSKI will fail to re-init the kernel correctly.)
	// Listen for "after" init-soar events (we need to know when these happen so we can resend the output link over to the client)
	m_pOutputListener->RegisterForKernelSMLEvents() ;
}

AgentSML::~AgentSML()
{
	// Release any objects we still own
	Clear(true) ;

	// If we have an output listener object, delete it now.
	// NOTE: At this point we're assuming AgentSML objects live as long as the underlying gSKI Agent object.
	// If not, we need to unregister this listener, but we shouldn't do that here as the Agent object may
	// be invalid by the time this destructor is called.
	delete m_pOutputListener ;

	delete m_pInputProducer ;
}

// Release any objects or other data we are keeping.  We do this just
// prior to deleting AgentSML, but before the underlying gSKI agent has been deleted
// 'deletingThisAgent' should only be true when we're actually in the destructor.
void AgentSML::Clear(bool deletingThisAgent)
{
#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML::Clear start %s", deletingThisAgent ? "deleting this agent." : "not deleting this agent.") ;
#endif

	// Release any WME objects we still own.
	// (Don't flush removes in this case as we're shutting down rather than just doing an init-soar).
	ReleaseAllWmes(!deletingThisAgent) ;

	m_ProductionListener.Clear();
	m_RunListener.Clear();
	m_PrintListener.Clear();
	m_pOutputListener->Clear() ;
	m_XMLListener.Clear() ;

#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML::Clear end %s", deletingThisAgent ? "deleting this agent." : "not deleting this agent.") ;
#endif
}

// Release all of the WMEs that we currently have references to
// It's a little less severe than clear() which releases everything we own, not just wmes.
// If flushPendingRemoves is true, make sure gSKI removes all wmes from Soar's working memory
// that have been marked for removal but are still waiting for the next input phase to actually
// be removed (this should generally be correct so we'll default to true for it).
void AgentSML::ReleaseAllWmes(bool flushPendingRemoves)
{
#ifdef DEBUG_UPDATE
	PrintDebugFormat("****************************************************") ;
	PrintDebugFormat("%s AgentSML::ReleaseAllWmes start %s", this->GetIAgent()->GetName(), flushPendingRemoves ? "flush pending removes." : "do not flush pending removes.") ;
	PrintDebugFormat("****************************************************") ;
#endif

	if (flushPendingRemoves)
	{
		bool forceAdds = false ;	// It doesn't matter if we do these or not as we're about to release everything.  Seems best to not start things up.
		bool forceRemoves = true ;	// SML may have deleted a wme but gSKI has yet to act on this.  As SML has removed its object we have no way to free the gSKI object w/o doing this update.
		this->GetIAgent()->GetInputLink()->GetInputLinkMemory()->Update(forceAdds, forceRemoves) ;
	}

	// Release any WME objects we still own.
	for (TimeTagMapIter mapIter = m_TimeTagMap.begin() ; mapIter != m_TimeTagMap.end() ; mapIter++)
	{
		gSKI::IWme* pWme = mapIter->second ;
		std::string value = pWme->GetValue()->GetString() ;

		bool deleted = pWme->Release() ;

		if (!deleted)
		{
			// Can put a break point here to see if any wmes aren't being deleted when they are released.
			// Unforunately, we can't assert or do more because that can be valid (others places might validly have a reference to pWme).
			int x = 1 ;
			unused(x) ;
		}
	}

	if (m_InputLinkRoot)
	{
		m_InputLinkRoot->Release() ;
	}
	m_InputLinkRoot = NULL ;

	m_TimeTagMap.clear() ;
	m_ToClientIdentifierMap.clear() ;
	m_IdentifierMap.clear() ;

#ifdef DEBUG_UPDATE
	PrintDebugFormat("****************************************************") ;
	PrintDebugFormat("%s AgentSML::ReleaseAllWmes end %s", this->GetIAgent()->GetName(), flushPendingRemoves ? "flush pending removes." : "do not flush pending removes.") ;
	PrintDebugFormat("****************************************************") ;
#endif
}

void AgentSML::RemoveAllListeners(Connection* pConnection)
{
	m_ProductionListener.RemoveAllListeners(pConnection);
	m_RunListener.RemoveAllListeners(pConnection);
	m_PrintListener.RemoveAllListeners(pConnection);
	m_pOutputListener->RemoveAllListeners(pConnection) ; 
	m_XMLListener.RemoveAllListeners(pConnection) ;
}

class AgentSML::AgentBeforeDestroyedListener: public gSKI::IAgentListener
{
public:
	// This handler is called right before the agent is actually deleted
	// inside gSKI.  We need to clean up any object we own now.
	virtual void HandleEvent(egSKIAgentEventId, gSKI::Agent* pAgent)
	{
#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML::AgentBeforeDestroyedListener start.") ;
#endif
		KernelSML* pKernelSML = KernelSML::GetKernelSML() ;

		// Release any wmes or other objects we're keeping
		AgentSML* pAgentSML = pKernelSML->GetAgentSML(pAgent) ;
		pAgentSML->Clear(false) ;

		// Remove the listeners that KernelSML uses for this agent.
		// This is important.  Otherwise if we create a new agent using the same kernel object
		// the listener will still exist inside gSKI and will crash when an agent event is next generated.
		pAgentSML->GetOutputListener()->UnRegisterForKernelSMLEvents() ;

		// Unregister ourselves (this is important for the same reasons as listed above)
		pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, this) ;

		// Then delete our matching agent sml information
		pKernelSML->DeleteAgentSML(pAgent) ;

		// Do self clean-up of this object as it's just called
		// prior to deleting the AgentSML structure.
		delete this ;

#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML::AgentBeforeDestroyedListener end.") ;
#endif
	}
};

void AgentSML::RegisterForBeforeAgentDestroyedEvent()
{
	// We should do this immediately before we delete the agent.
	// We shouldn't do it earlier or we can't be sure it'll be last on the list of listeners which is where we
	// need it to be (so that we clear our information about the gSKI agent *after* we've notified any of our listeners
	// about this event).
	m_pBeforeDestroyedListener = new AgentBeforeDestroyedListener() ;
	m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, m_pBeforeDestroyedListener) ;
}

void AgentSML::ScheduleAgentToRun(bool state) 
{ 
	if (m_pIAgent->GetRunState() != gSKI_RUNSTATE_HALTED) 
	{
		m_ScheduledToRun = state ; 
		m_WasOnRunList = state; 
	}
}

/*************************************************************
* @brief	Converts an id from a client side value to a kernel side value.
*			We need to be able to do this because the client is adding a collection
*			of wmes at once, so it makes up the ids for those objects.
*			But the kernel will assign them a different value when the
*			wme is actually added in the kernel.
*************************************************************/
bool AgentSML::ConvertID(char const* pClientID, std::string* pKernelID)
{
	if (pClientID == NULL)
		return false ;

	IdentifierMapIter iter = m_IdentifierMap.find(pClientID) ;

	if (iter == m_IdentifierMap.end())
	{
		// If the client id is not in the map, then we may have been
		// passed a kernel id (this will happen at times).
		// So return the value we were passed
		*pKernelID = pClientID ;
		return false ;
	}
	else
	{
		// If we found a mapping, return the mapped value
		*pKernelID = iter->second ;
		return true ;
	}
}

void AgentSML::RecordIDMapping(char const* pClientID, char const* pKernelID)
{
	m_IdentifierMap[pClientID] = pKernelID ;

	// Record in both directions, so we can clean up (at which time we only know the kernel side ID).
	m_ToClientIdentifierMap[pKernelID] = pClientID ;
}

void AgentSML::RemoveID(char const* pKernelID)
{
	IdentifierMapIter iter = m_ToClientIdentifierMap.find(pKernelID) ;

	// This identifier should have been in the table
	assert (iter != m_ToClientIdentifierMap.end()) ;
	if (iter == m_ToClientIdentifierMap.end())
		return ;

	// Delete this mapping from both tables
	std::string clientID = iter->second ;
	m_IdentifierMap.erase(clientID) ;
	m_ToClientIdentifierMap.erase(pKernelID) ;
}

/*************************************************************
* @brief	Converts a time tag from a client side value to
*			a kernel side object
*************************************************************/
gSKI::IWme* AgentSML::ConvertTimeTag(char const* pTimeTag)
{
	if (pTimeTag == NULL)
		return NULL ;

	TimeTagMapIter iter = m_TimeTagMap.find(pTimeTag) ;

	if (iter == m_TimeTagMap.end())
	{
		return NULL ;
	}
	else
	{
		// If we found a mapping, return the mapped value
		gSKI::IWme* result = iter->second ;
		return result ;
	}
}

/*************************************************************
* @brief	Maps from a client side time tag to a kernel side WME.
*************************************************************/
void AgentSML::RecordTimeTag(char const* pTimeTag, gSKI::IWme* pWME)
{
#ifdef _DEBUG
	// I believe it correct that a time tag should never be re-used in this context
	// so I'm including this assert.  However, it's possible this assumption is wrong (in particular after an init-soar?)
	// so I'm only including it in debug builds and if the assert fails, check the context and make sure that this re-use
	// in indeed a mistake.
	// If you fail to call commit() after creating a new input wme and then issue an init-soar this assert may fire.
	// If so, the fix is to call commit().
	assert (m_TimeTagMap.find(pTimeTag) == m_TimeTagMap.end()) ;
#endif

	m_TimeTagMap[pTimeTag] = pWME ;
}

void AgentSML::RecordLongTimeTag(long timeTag, gSKI::IWme* pWME)
{
	// Make sure it's a valid time tag
	assert(timeTag != 0) ;

	char str[kMinBufferSize] ;
	Int2String(timeTag, str, sizeof(str)) ;

	RecordTimeTag(str, pWME) ;
}

void AgentSML::RemoveTimeTag(char const* pTimeTag)
{
	m_TimeTagMap.erase(pTimeTag) ;
}

void AgentSML::RemoveLongTimeTag(long timeTag)
{
	char str[kMinBufferSize] ;
	Int2String(timeTag, str, sizeof(str)) ;

	m_TimeTagMap.erase(str) ;
}
