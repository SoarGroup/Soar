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

// Forward declarations
namespace gSKI {
	class IAgent ;
	class IWme ;
}

#include <map>
#include <list>
#include <string>

namespace sml {

// Forward declarations
class OutputListener ;

// Map from a client side identifier to a kernel side one (e.g. "o3" => "O5")
typedef std::map<std::string, std::string>	IdentifierMap ;
typedef IdentifierMap::iterator				IdentifierMapIter ;
typedef IdentifierMap::const_iterator		IdentifierMapConstIter ;

// Map from a client side time tag (as a string) to a client side identifier (e.g. "35" => "o3")
// This one's only needed so we can do cleanup of our identifier map during removal of WMEs.
// There may be some way to avoid it but I don't see how (and be reasonably fast).
typedef std::map<std::string, std::string>	TimeIdentifierMap ;
typedef TimeIdentifierMap::iterator			TimeIdentifierMapIter ;
typedef TimeIdentifierMap::const_iterator	TimeIdentifierMapConstIter ;

// Map from a client side time tag (as a string) to a kernel side WME* object
// (Had planned to just map the time tag to a kernel time tag...but it turns out
//  there's no quick way to look up an object in the kernel from its time tag).
typedef std::map<std::string, gSKI::IWme*>	TimeTagMap ;
typedef TimeTagMap::iterator				TimeTagMapIter ;
typedef TimeTagMap::const_iterator			TimeTagMapConstIter ;

class AgentSML
{
protected:
	// This is a callback we can register to listen for changes to the output-link
	OutputListener*	m_pOutputListener ;

	// A reference to the underlying gSKI agent object
	gSKI::IAgent*	m_pIAgent ;

	// Map from client side identifiers to kernel side ones
	IdentifierMap	m_IdentifierMap ;

	// Map from client side time tags (as strings) to kernel side WME* objects
	TimeTagMap		m_TimeTagMap ;

public:
	AgentSML(gSKI::IAgent* pAgent) { m_pIAgent = pAgent ; m_pOutputListener = NULL ; }

	~AgentSML() ;

	void SetOutputListener(OutputListener* pListener)	{ m_pOutputListener = pListener ; }
	bool SetStopOnOutput(bool state) ;

	/*************************************************************
	* @brief	Converts an id from a client side value to a kernel side value.
	*			We need to be able to do this because the client is adding a collection
	*			of wmes at once, so it makes up the ids for those objects.
	*			But the kernel will assign them a different value when the
	*			wme is actually added in the kernel.
	*************************************************************/
	bool ConvertID(char const* pClientID, std::string* pKernelID) ;
	void RecordIDMapping(char const* pClientID, char const* pKernelID) ;

	/*************************************************************
	* @brief	Converts a time tag from a client side value to
	*			a kernel side one.
	*************************************************************/
	gSKI::IWme* ConvertTimeTag(char const* pTimeTag) ;
	void RecordTimeTag(char const* pTimeTag, gSKI::IWme* pWme) ;
	void RemoveTimeTag(char const* pTimeTag) ;
} ;


}

#endif // SML_AGENT_SML_H