/////////////////////////////////////////////////////////////////
// KernelSML class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents the translation point between SML and gSKI.
//
// It will maintain the information needed to communicate with gSKI
// and send and receive messages to the client (a tool or simulation).
//
/////////////////////////////////////////////////////////////////

#ifndef SML_KERNEL_SML_H
#define SML_KERNEL_SML_H

// This define allows us to pull in and out the Tcl debugger.
// We need this during the early SML days because the Tcl debugger
// needs local access to the gSKI kernel and that only happens here.
//#define USE_TCL_DEBUGGER

// Forward declarations
namespace gSKI {
	class IKernelFactory ;
	class IKernel ;
	class IAgent ;
	class IInputProducer ;
	class IOutputProcessor ;
	class OutputListener ;
	class IWme ;
	struct Error ;
}

#include <map>
#include <list>

#ifdef USE_TCL_DEBUGGER
// Forward declaration for the debugger.
namespace TgD
{
	class TgD ;
}
#endif

#include "cli_CommandLineInterface.h"

namespace sml {

// Forward declarations
class ElementXML ;
class Connection ;
class AnalyzeXML ;
class KernelSML ;
class OutputListener ;

// We need a comparator to make the map we're about to define work with char*
struct strCompareKernel
{
  bool operator()(const char* s1, const char* s2) const
  {
    return std::strcmp(s1, s2) < 0;
  }
};

// Define the CommandFunction which we'll call to process commands
typedef bool (KernelSML::*CommandFunction)(gSKI::IAgent*, char const*, Connection*, AnalyzeXML*, ElementXML*, gSKI::Error*);

// Used to store a map from command name to function handler for that command
typedef std::map<char const*, CommandFunction, strCompareKernel>	CommandMap ;
typedef CommandMap::iterator										CommandMapIter ;
typedef CommandMap::const_iterator									CommandMapConstIter ;

// List of input producers that we need to delete
typedef std::list<gSKI::IInputProducer*>	InputProducerList_t ;
typedef InputProducerList_t::iterator		InputProducerListIter_t ;

// List of output producers that we need to delete
typedef std::list<gSKI::IOutputProcessor*>	OutputProcessorList_t ;
typedef OutputProcessorList_t::iterator		OutputProcessorListIter_t ;

// List of output listeners that we need to delete
typedef std::list<OutputListener*>			OutputListenerList_t ;
typedef OutputListenerList_t::iterator		OutputListenerListIter_t ;

// Map from a client side identifier to a kernel side one (e.g. "o3" => "O5")
typedef std::map<std::string, std::string>	IdentifierMap ;
typedef IdentifierMap::iterator				IdentifierMapIter ;
typedef IdentifierMap::const_iterator		IdentifierMapConstIter ;

// Map from a client side time tag (as a string) to a kernel side WME* object
// (Had planned to just map the time tag to a kernel time tag...but it turns out
//  there's no quick way to look up an object in the kernel from its time tag).
typedef std::map<std::string, gSKI::IWme*>	TimeTagMap ;
typedef TimeTagMap::iterator				TimeTagMapIter ;
typedef TimeTagMap::const_iterator			TimeTagMapConstIter ;

// This class is used to keep track of information needed by SML on an agent by agent basis.
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

// Map from agent pointers to information we keep for SML about those agents.
typedef std::map<gSKI::IAgent*, AgentSML*>	AgentMap ;
typedef AgentMap::iterator					AgentMapIter ;
typedef AgentMap::const_iterator			AgentMapConstIter ;

class KernelSML
{
protected:	
	// Map from command name to function to handle it
	CommandMap	m_CommandMap ;

	// Map from gSKI agent pointers to AgentSML objects, where we keep additional information
	// required for SML about each agent.
	AgentMap		m_AgentMap ;

#ifdef USE_TCL_DEBUGGER
	// A hack to allow us access to the Tcl debugger until we have a real one available.
	TgD::TgD* m_Debugger ;
#endif

	// Command line interface module
	cli::CommandLineInterface m_CommandLineInterface ;

	// The singleton kernel object
	static KernelSML*	s_pKernel ;

	gSKI::IKernelFactory* m_pKernelFactory ;   
	gSKI::IKernel* m_pIKernel ;

public:
	/*************************************************************
	* @brief	Returns the singleton kernel object.
	*************************************************************/
	static KernelSML* GetKernelSML() ;

	/*************************************************************
	* @brief	Delete the singleton kernel object
	*			(We only call this in debug mode so we can test memory
	*			 releasing is correct).
	*************************************************************/
	static void DeleteSingleton()
	{
		if (s_pKernel)
			delete s_pKernel ;

		s_pKernel = 0 ;
	}

public:
	~KernelSML(void);

	/*************************************************************
	* @brief	Takes an incoming SML message and responds with
	*			an appropriate response message.
	*
	* @param pConnection	The connection this message came in on.
	* @param pIncoming		The incoming message
	*************************************************************/
	ElementXML* ProcessIncomingSML(Connection* pConnection, ElementXML* pIncoming) ;

protected:
	KernelSML(void);

protected:
	/*************************************************************
	* @brief	Look up our additional SML information for a specific agent.
	*
	*			This will always return an AgentSML object.
	*			If the IAgent* is new, this call will record a new AgentSML
	*			object in the m_AgentMap and return a pointer to it.
	*			We do this, so we can easily support connecting up to
	*			agents that were created before a connection is established
	*			through SML to the kernel (e.g. when attaching a debugger).
	*	
	*************************************************************/
	AgentSML*	GetAgentSML(gSKI::IAgent*) ;

	/*************************************************************
	* @brief	Look up an agent from its name.
	*************************************************************/
	gSKI::IAgent* GetAgent(char const* pAgentName) ;

	/*************************************************************
	* @brief	Get the kernel object.
	*************************************************************/
	gSKI::IKernel* GetKernel() { return m_pIKernel ; }

	/*************************************************************
	* @brief	Get the kernel factory object.
	*************************************************************/
	gSKI::IKernelFactory* GetKernelFactory() { return m_pKernelFactory ; }

	/*************************************************************
	* @brief	Return an object* to the caller.
	*************************************************************/
	bool ReturnResult(Connection* pConnection, ElementXML* pResponse, char const* pResult) ;

	/*************************************************************
	* @brief	Return an integer result to the caller.
	*************************************************************/
	bool ReturnIntResult(Connection* pConnection, ElementXML* pResponse, int result) ;

	/*************************************************************
	* @brief	Return a boolean result to the caller.
	*************************************************************/
	bool ReturnBoolResult(Connection* pConnection, ElementXML* pResponse, bool result) ;

	/*************************************************************
	* @brief	Return an invalid argument error to the caller.
	*************************************************************/
	bool InvalidArg(Connection* pConnection, ElementXML* pResponse, char const* pCommandName, char const* pErrorDescription) ;

	void BuildCommandMap() ;

	bool ProcessCommand(char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse) ;

	// Add a value to working memory
	bool AddInputWME(gSKI::IAgent* pAgent, char const* pID, char const* pAttribute, char const* pValue, char const* pType, char const* pTimeTag, gSKI::Error* pError) ;

	// Remove a value from working memory.  The time tag is the string form of an int.
	bool RemoveInputWME(gSKI::IAgent* pAgent, char const* pTimeTag, gSKI::Error* pError) ;

	// Our command handlers

	/*************************************************************
	* @brief	A command handler (SML message->appropriate gSKI handling).
	*
	* @param pAgent			The agent this command is for (can be NULL if the command is not agent specific)
	* @param pCommandName	The SML command name (so one handler can handle many incoming calls if we wish)
	* @param pConnection	The connection this command came in on
	* @param pIncoming		The incoming, analyzed message.
	* @param pResponse		The partially formed response.  This handler needs to fill in more of this.
	* @param pError			A gSKI error object, which gSKI will fill in if there are errors.
	* @returns False if we had an error and wish to generate a generic error message (based on the incoming call + pError)
	*          True if the call succeeded or we generated another more specific error already.
	*************************************************************/
	bool KernelSML::HandleCreateAgent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleLoadProductions(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleGetInputLink(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleInput(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;

	bool KernelSML::HandleCommandLine(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;

	bool KernelSML::HandleCheckForIncomingCommands(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;

};

}

#endif // SML_KERNEL_H