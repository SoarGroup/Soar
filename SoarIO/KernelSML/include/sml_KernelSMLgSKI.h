/////////////////////////////////////////////////////////////////
// KernelSMLgSKI class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This is a set of commands used to send and receive
// primitive gSKI objects.  Currently these are all deprecated.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_KERNEL_SML_GSKI_H
#define SML_KERNEL_SML_GSKI_H

// This define allows us to pull in and out the Tcl debugger.
// We need this during the early SML days because the Tcl debugger
// needs local access to the gSKI kernel and that only happens here.
//#define USE_TCL_DEBUGGER

// Forward declarations
namespace gSKI {
	class IKernelFactory ;
	class IKernel ;
	class IInputProducer ;
	class IOutputProcessor ;
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
class KernelSMLgSKI ;

// We need a comparator to make the map we're about to define work with char*
struct strCompareKernel
{
  bool operator()(const char* s1, const char* s2) const
  {
    return std::strcmp(s1, s2) < 0;
  }
};

// In Debug mode, we record extra information about the objects
// we store in the ObjectMap.  In Release, it's just the object itself.
#ifdef DEBUG
#define USE_KERNEL_OBJECT_INFO
#endif

class KernelObjectInfo
{
public:
	void const*	m_pObject ;				// The underlying object
	char const*	m_pType ;				// The type of the object
	bool		m_ShouldBeReleased;		// Whether we expect it to be released
	int			m_AllocationCount ;		// Which object this was (so we can break on it later)

public:
	KernelObjectInfo(void const* pObject, char const* pType, bool shouldBeReleased, int allocation)
	{
		m_pObject	= pObject ;
		m_pType		= pType ;
		m_ShouldBeReleased = shouldBeReleased ;
		m_AllocationCount = allocation ;
	}
} ;

// Define the CommandFunction which we'll call to process commands
typedef bool (KernelSMLgSKI::*CommandFunction)(void*, char const*, Connection*, AnalyzeXML*, ElementXML*, gSKI::Error*);

// Used to store a map from command name to function handler for that command
typedef std::map<char const*, CommandFunction, strCompareKernel>	CommandMap ;
typedef CommandMap::iterator										CommandMapIter ;
typedef CommandMap::const_iterator									CommandMapConstIter ;

// Map from id as a hex string to an object
typedef std::map<char*, void*, strCompareKernel>	ObjectMap ;
typedef ObjectMap::iterator							ObjectMapIter ;
typedef ObjectMap::const_iterator					ObjectMapConstIter ;

// List of input producers that we need to delete
typedef std::list<gSKI::IInputProducer*>	InputProducerList_t ;
typedef InputProducerList_t::iterator		InputProducerListIter_t ;

// List of output producers that we need to delete
typedef std::list<gSKI::IOutputProcessor*>	OutputProcessorList_t ;
typedef OutputProcessorList_t::iterator		OutputProcessorListIter_t ;

class KernelSMLgSKI
{
public:
	static const int kMaxIDLen = 20 ;
	static const bool kShouldBeReleased = true ;
	static const bool kShouldNotBeReleased = false ;
	static const int kStopOnAllocation = 0 ;	// Set this to a non-zero value to break on a specific allocation (to find out which object is leaking)

protected:	
	// Count each object as its created and stored, so we can report/break on a specific one in the future.
	int			m_AllocationCounter ;

	// Map from command name to function to handle it
	CommandMap	m_CommandMap ;

	// Map from class and id (hex string) to object
	ObjectMap	m_ObjectMap ;

	// A list of InputProducer objects we have created.
	InputProducerList_t		m_InputProducers ;

	// A list of OutputProcessor objects we have created.
	OutputProcessorList_t	m_OutputProcessors ;

#ifdef USE_TCL_DEBUGGER
	// A hack to allow us access to the Tcl debugger until we have a real one available.
	TgD::TgD* m_Debugger ;
	gSKI::IKernel*  m_DebuggerKernel ;
	gSKI::IKernelFactory* m_DebuggerKernelFactory ;
#endif

   // Command line interface module
   //cli::CommandLineInterface m_CommandLineInterface ;

	// The singleton kernel object
	static KernelSMLgSKI*	s_pKernel ;

	gSKI::IKernelFactory* m_pKernelFactory ;   
	gSKI::IKernel* m_pIKernel ;

public:
	/*************************************************************
	* @brief	Returns the singleton kernel object.
	*************************************************************/
	static KernelSMLgSKI* GetKernel() ;

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
	~KernelSMLgSKI(void);

	/*************************************************************
	* @brief	Takes an incoming SML message and responds with
	*			an appropriate response message.
	*
	* @param pConnection	The connection this message came in on.
	* @param pIncoming		The incoming message
	*************************************************************/
	ElementXML* ProcessIncomingSML(Connection* pConnection, ElementXML* pIncoming) ;

protected:
	KernelSMLgSKI(void);

public:
	/*************************************************************
	* @brief	Record an object so we can look it up later.
	*
	* @param pObject	The object itself.
	* @param pType		A string for the type -- MUST be a static const, we just store the pointer
	* @param shouldBeReleased	True if we expect client to eventually release this object.
	* @returns The ID we created for this object.  Copy this to keep it.
	*************************************************************/
	char const* StoreObject(void const* pObject, const char* pType, bool shouldBeReleased) ;

	/*************************************************************
	* @brief	Removes an object from our table.
	*
	* @param pID The id for this object (e.g. "0x4545")
	* @returns true if we find the object.
	*************************************************************/
	bool DeleteObject(char const* pID) ;

	/*************************************************************
	* @brief	Looks up an object based on a string ID.
	*
	* @param pID		An id for this object (e.g. "0x45ad") (can be NULL, returns NULL)
	* @param pType		The type of this object (can be NULL -- which means we don't care)
	* @returns The object (or NULL if we don't find it in the map).
	*************************************************************/
	void* LookupObject(char const* pID, char const* pType) ;

	/*************************************************************
	* @brief	Generate an ID for an object, but don't store it.
	*
	* @param pObject	The object itself.
	* @param pBuffer	Should be at least 20 bytes.
	* @returns The ID we created for this object.  (Pointer to pBuffer passed in).
	*************************************************************/
	char* GenerateID(void const* pObject, char* pBuffer) ;

protected:
	/*************************************************************
	* @brief	Return an object* to the caller.
	*************************************************************/
	bool ReturnResult(Connection* pConnection, ElementXML* pResponse, void const* pResult, char const* pType, bool shouldBeReleased) ;

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
	bool InvalidArg(Connection* pConnection, ElementXML* pResponse, char const* pCommandName) ;

	void BuildCommandMap() ;

	bool ProcessCommand(char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse) ;

	// Our command handlers

	/*************************************************************
	* @brief	A command handler (SML message->gSKI function call).
	*
	* The method being called is e.g. IKernel::GetAgentManager()
	* The this pointer in this case will be an IKernel* object.
	*
	* @param pThis			The 'this' pointer for this call (NULL if a static method is being called)
	* @param pCommandName	The SML command name (so one handler can handle many incoming calls if we wish)
	* @param pConnection	The connection this command came in on
	* @param pIncoming		The incoming, analyzed message.
	* @param pResponse		The partially formed response.  This handler needs to fill in more of this.
	* @param pError			A gSKI error object, which gSKI will fill in if there are errors.
	* @returns False if we had an error and wish to generate a generic error message (based on the incoming call + pError)
	*          True if the call succeeded or we generated another more specific error already.
	*************************************************************/
	bool KernelSMLgSKI::CreateKernelFactory(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSMLgSKI::IKernelFactory_Create(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSMLgSKI::IKernelFactory_DestroyKernel(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IKernel_GetAgentManager(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSMLgSKI::IAgentManager_AddAgent(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IAgentManager_RemoveAgent(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;

	bool KernelSMLgSKI::IRelease_Release(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;

	bool KernelSMLgSKI::IInputLink_GetInputLinkMemory(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IInputLink_GetRootObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IInputLink_AddInputProducer(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;

	bool KernelSMLgSKI::IWMObject_GetWMEs(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;

	bool KernelSMLgSKI::IWme_GetValue(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::ISymbol_GetObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::ISymbol_GetString(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::ISymbol_GetInt(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;

	bool KernelSMLgSKI::IWorkingMemory_ReplaceStringWme(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IWorkingMemory_ReplaceIntWme(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IWorkingMemory_ReplaceDoubleWme(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;

	bool KernelSMLgSKI::IWorkingMemory_AddWmeString(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IWorkingMemory_AddWmeNewObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IWorkingMemory_AddWmeInt(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;	
	bool KernelSMLgSKI::IWorkingMemory_AddWmeDouble(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IWorkingMemory_RemoveObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IWorkingMemory_GetAgent(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IWorkingMemory_AddWmeObjectLink(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;

	bool KernelSMLgSKI::IAgent_RunInClientThread(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IAgent_GetOutputLink(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IAgent_GetInputLink(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;

	bool KernelSMLgSKI::IIterator_Pointer_IsValid(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IIterator_Pointer_GetVal(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IIterator_Pointer_Next(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;

	bool KernelSMLgSKI::IOutputLink_AddOutputProcessor(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IOutputLink_SetAutomaticUpdate(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IOutputLink_GetRootObject(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;
	bool KernelSMLgSKI::IOutputLink_GetOutputLinkMemory(void* pThis, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error *pError) ;

};

}

#endif // SML_KERNEL_GSKI_H