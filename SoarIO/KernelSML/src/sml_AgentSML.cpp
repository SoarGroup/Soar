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

#include "IgSKI_WME.h"
#include "IgSKI_Agent.h"
#include "IgSKI_InputProducer.h"

#include <assert.h>

using namespace sml ;

AgentSML::~AgentSML()
{
	// Release any objects we still own
	Clear() ;

	// If we have an output listener object, delete it now.
	// NOTE: At this point we're assuming AgentSML objects live as long as the underlying gSKI IAgent object.
	// If not, we need to unregister this listener, but we shouldn't do that here as the IAgent object may
	// be invalid by the time this destructor is called.
	delete m_pOutputListener ;

	delete m_pInputProducer ;
}

// Release any objects or other data we are keeping.  We do this just
// prior to deleting AgentSML, but before the underlying gSKI agent has been deleted
void AgentSML::Clear()
{
	// Release any WME objects we still own.
	for (TimeTagMapIter mapIter = m_TimeTagMap.begin() ; mapIter != m_TimeTagMap.end() ; mapIter++)
	{
		gSKI::IWme* pWme = mapIter->second ;
		pWme->Release() ;
	}

	m_TimeTagMap.clear() ;

	m_AgentListener.Clear() ;
}

void AgentSML::RemoveAllListeners(Connection* pConnection)
{
	m_AgentListener.RemoveAllListeners(pConnection) ;
	m_pOutputListener->RemoveAllListeners(pConnection) ; 
}

/*************************************************************
* @brief	When set, this flag will cause Soar to break when
*			output is next generated during a run.
*************************************************************/
bool AgentSML::SetStopOnOutput(bool state)
{
	if (!m_pOutputListener)
		return false ;

	m_pOutputListener->SetStopOnOutput(state) ;

	return true ;
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
	m_TimeTagMap[pTimeTag] = pWME ;
}

void AgentSML::RemoveTimeTag(char const* pTimeTag)
{
	m_TimeTagMap.erase(pTimeTag) ;
}
