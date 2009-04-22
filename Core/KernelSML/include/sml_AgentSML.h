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
#include "sml_OutputListener.h"
#include "sml_InputListener.h"

#include "callback.h"


// Forward definitions for kernel
typedef struct agent_struct agent;
typedef union symbol_union Symbol;
typedef struct cons_struct list;
typedef struct wme_struct wme;

#include <map>
#include <list>
#include <string>
#include <fstream>

namespace sml {

// Forward declarations for SML
class OutputListener ;
class KernelSML ;
class RhsFunction ;
class AgentRunCallback ;

// badbad: shouldn't we be using hash_maps for these?

// Map from a client side identifier to a kernel side one (e.g. "o3" => "O5")
typedef std::map< std::string, std::string >	IdentifierMap ;
typedef IdentifierMap::iterator				IdentifierMapIter ;
typedef IdentifierMap::const_iterator		IdentifierMapConstIter ;

// Keep track of instances of client side ids
typedef std::map< std::string, int >		IdentifierRefMap ;
typedef IdentifierRefMap::iterator			IdentifierRefMapIter ;
typedef IdentifierRefMap::const_iterator	IdentifierRefMapConstIter ;

// Map from client side time tag to a kernel time tag
typedef std::map< long, unsigned long >		CKTimeMap ;
typedef CKTimeMap::iterator					CKTimeMapIter ;

// Map from kernel side time tag to client time tag
typedef std::map< unsigned long, long >		KCTimeMap ;
typedef KCTimeMap::iterator					KCTimeMapIter ;

// Map from client side time tag to client time tag, for replay
typedef std::map< long, long >				CCTimeMap ;
typedef CCTimeMap::iterator					CCTimeMapIter ;

// List of input messages waiting for the next input phase callback from the kernel
typedef std::list<soarxml::ElementXML*>		PendingInputList ;
typedef PendingInputList::iterator			PendingInputListIter ;

// Map of kernel time tags to kernel wmes for input
typedef std::map< unsigned long, wme* >				WmeMap;
typedef std::map< unsigned long, wme* >::iterator	WmeMapIter;

class AgentSML
{
	friend class KernelSML ;
	friend class InputListener ;

public:
	static void InputPhaseCallback( agent* agent, soar_callback_event_id eventid, soar_callback_data callbackdata, soar_call_data calldata );

protected:
   // This function actually creates a wme in Soar (and the associated mappings between client and kernel side timetags)
   // Currently, only the functions below call this, so it's protected
   bool AddInputWME(char const* pID, char const* pAttribute, Symbol* pValueSymbol, long clientTimeTag);
public:
   // These functions convert values into Symbols so the above function can be called
   // These functions are called directly in "optimized" mode (set when the client-side kernel is created)
   bool AddStringInputWME(char const* pID, char const* pAttribute, char const* pValue, long clientTimeTag);
   bool AddIntInputWME(char const* pID, char const* pAttribute, int value, long clientTimeTag);
   bool AddDoubleInputWME(char const* pID, char const* pAttribute, double value, long clientTimeTag);

    // This function converts string values into typed values so the above functions can be called
    // This function is called by InputListener and passed info taken from XML objects
    bool AddIdInputWME(char const* pID, char const* pAttribute, char const* pValue, long clientTimeTag);

	bool AddInputWME(char const* pID, char const* pAttribute, char const* pValue, char const* pType, char const* pTimeTag) ;
	bool RemoveInputWME(long timeTag) ;
    bool RemoveInputWME(char const* pTimeTag) ;
   
protected:

	// A reference to the underlying kernel agent object
	agent*			m_agent ;

	// Pointer back to the owning kernel SML object
	KernelSML*		m_pKernelSML ;

	// Map from client side identifiers to kernel side ones
	IdentifierMap	m_IdentifierMap ;

	// For cleanup we also need a map from kernel side identifiers to client side ones (for cleanup)
	IdentifierMap	m_ToClientIdentifierMap ;

	// Keep track of number of instances of client side identifiers
	IdentifierRefMap m_IdentifierRefMap;

	// Map from client side time tags to kernel side timetags, and back (for cleanup
	CKTimeMap		m_CKTimeMap ;
	KCTimeMap		m_KCTimeMap ;

	// Used to listen for kernel events that are agent specific
	ProductionListener	m_ProductionListener;
	RunListener			m_RunListener;
	PrintListener		m_PrintListener;
	XMLListener			m_XMLListener ;

	// This is a callback we can register to listen for changes to the output-link
	OutputListener		m_OutputListener ;

	// This listener is called during the kernel's input phase callback
	InputListener		m_InputListener ;

	// Input changes waiting to be processed at next input phase callback
	PendingInputList	m_PendingInput ;

	// Used to listen for a before removed event
	//class AgentBeforeDestroyedListener ;
	//AgentBeforeDestroyedListener*	m_pBeforeDestroyedListener ;

	bool m_SuppressRunEndsEvent ;

	// Used to control runs
	bool m_ScheduledToRun ;
	bool m_WasOnRunList;
	bool m_OnStepList;
	//unsigned long	m_InitialStepCount ;
	unsigned long	m_InitialRunCount ;
	smlRunResult	m_ResultOfLastRun ;
    unsigned long	m_localRunCount;
    unsigned long	m_localStepCount;
	smlRunState		m_runState;          // Current agent run state
	unsigned long	m_interruptFlags;    // Flags indicating an interrupt request

	  // Used for update world events
	bool m_CompletedOutputPhase ;
	bool m_GeneratedOutput ;
	unsigned long m_OutputCounter ;

	RhsFunction*	m_pRhsInterrupt ;
	RhsFunction*	m_pRhsConcat ;
	RhsFunction*	m_pRhsExec ;
	RhsFunction*	m_pRhsCmd ;

	AgentRunCallback*	m_pAgentRunCallback ;

	WmeMap			m_KernelTimeTagToWmeMap ; // Kernel time tag to kernel wme

	void AddWmeToWmeMap( long clientTimeTag, wme* w );
	wme* FindWmeFromKernelTimetag( unsigned long timetag );
	static void InputWmeGarbageCollectedHandler( agent* pAgent, int eventID, void* pData, void* pCallData ) ;

	~AgentSML() ;

	// Capture/Replay input
private:
	std::fstream*		m_pCaptureFile;
	bool				m_CaptureAutoflush;
	bool				m_ReplayInput;
    CCTimeMap			m_ReplayTimetagMap;
	struct CapturedActionAdd
	{
		std::string id;
		std::string attr;
		std::string value;
		const char* type;
	};
	struct CapturedAction
	{
		CapturedAction() { add = 0; }
		CapturedAction(const CapturedAction& other) 
			: dc(other.dc)
			, timetag(other.timetag)
			, add(0)
		{
			if (other.add) add = new CapturedActionAdd(*other.add);
		}
		~CapturedAction() { if (add) delete add; }
		CapturedAction& operator=(const CapturedAction& other)
		{
			dc = other.dc;
			timetag = other.timetag;
			if (other.add) 
			{
				add = new CapturedActionAdd(*other.add);
			}
			else
			{
				add = 0;
			}
			return *this;
		}

		unsigned long dc;
		long timetag;
		void CreateAdd() { if (!add) add = new CapturedActionAdd(); }
		CapturedActionAdd* Add() const { return add; }

	private:
		CapturedActionAdd* add;
	};
	std::queue< CapturedAction > m_CapturedActions;
	static const std::string CAPTURE_SEPERATOR;
	bool				CaptureInputWME(const CapturedAction& ca);
	void				ResetCaptureReplay();

public: 
	bool				StartCaptureInput(const std::string& pathname, bool autoflush, unsigned long seed);
	bool				StopCaptureInput();
	inline bool			CaptureQuery() { return m_pCaptureFile ? true : false; }

	bool				StartReplayInput(const std::string& pathname);
	bool				StopReplayInput();
	inline bool			ReplayQuery()  { return m_ReplayInput; }
	size_t				NumberOfCapturedActions() { return m_CapturedActions.size(); }

	void				ReplayInputWMEs();
	// end Capture/Replay input

public:
	void RemoveWmeFromWmeMap( wme* w );

	AgentSML(KernelSML* pKernelSML, agent* pAgent) ;

	void InitListeners() ;
	void Init() ;

	// Release any objects or other data we are keeping.  We do this just
	// prior to deleting AgentSML
	void Clear(bool deletingThisAgent) ;

	void DeleteSelf() ;

	void RegisterForBeforeAgentDestroyedEvent() ;

	// Release all of the WMEs that we currently have references to
	// It's a little less severe than clear() which releases everything we own, not just wmes.
	void ReleaseAllWmes(bool flushPendingRemoves = true) ;

	// Explicitly reinitialize the agent as part of init-soar
	bool Reinitialize() ;

	agent* GetSoarAgent()	  { return m_agent ; }
	KernelSML* GetKernelSML() { return m_pKernelSML ; }

	char const* GetName() ;

	OutputListener* GetOutputListener()							{ return &m_OutputListener ; }
	InputListener* GetInputListener()							{ return &m_InputListener ; }

	void SetSuppressRunEndsEvent(bool state) { m_SuppressRunEndsEvent = state ; }
	bool GetSuppressRunEndsEvent()			 { return m_SuppressRunEndsEvent ; }

	void AddProductionListener(smlProductionEventId eventID, Connection* pConnection)	{ m_ProductionListener.AddListener(eventID, pConnection) ; }
	void RemoveProductionListener(smlProductionEventId eventID, Connection* pConnection) { m_ProductionListener.RemoveListener(eventID, pConnection) ; }	
	void AddRunListener(smlRunEventId eventID, Connection* pConnection)	{ m_RunListener.AddListener(eventID, pConnection) ; }
	void RemoveRunListener(smlRunEventId eventID, Connection* pConnection) { m_RunListener.RemoveListener(eventID, pConnection) ; }	
	void AddPrintListener(smlPrintEventId eventID, Connection* pConnection)	{ m_PrintListener.AddListener(eventID, pConnection) ; }
	void RemovePrintListener(smlPrintEventId eventID, Connection* pConnection) { m_PrintListener.RemoveListener(eventID, pConnection) ; }	
	void AddXMLListener(smlXMLEventId eventID, Connection* pConnection) { m_XMLListener.AddListener(eventID, pConnection) ; }
	void RemoveXMLListener(smlXMLEventId eventID, Connection* pConnection) { m_XMLListener.RemoveListener(eventID, pConnection) ; }

	// Echo the list of wmes received back to any listeners
	void FireInputReceivedEvent(soarxml::ElementXML const* pCommands) { m_XMLListener.FireInputReceivedEvent(pCommands) ; }

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
	* @brief	During an init-soar, the agent generates print callbacks
	*			that need to be flushed before the trace is printed during
	*			the first run.
	*************************************************************/
	void FlushPrintOutput() { m_PrintListener.FlushOutput( NULL, smlEVENT_PRINT ); }

	/*************************************************************
	* @brief	Trigger an "echo" event.  This event allows one client
	*			to issue a command and have another client (typically the debugger)
	*			listen in on the output.
	*************************************************************/
	void FireEchoEvent(Connection* pConnection, char const* pMessage) 
	{ 
		m_PrintListener.OnEvent(smlEVENT_ECHO, this, pMessage) ; 
		m_PrintListener.FlushOutput(pConnection, smlEVENT_ECHO) ; 
	}

	/*************************************************************
	* @brief	This is the same as the regular FireEchoEvent, except
	*			it broadcasts to all listeners, even the originator of the command
	*			because the self field is not set in this case.
	*
	*			We shouldn't use this extensively and may choose to
	*			remove this and replace it with a different event later.
	*************************************************************/
	void FireEchoEventIncludingSelf(char const* pMessage) 
	{ 
		m_PrintListener.OnEvent(smlEVENT_ECHO, this, pMessage) ; 
		m_PrintListener.FlushOutput(NULL, smlEVENT_ECHO) ; 
	}

	/*************************************************************
	* @brief	Send XML and print events for a simple string message
	*************************************************************/
	void FireSimpleXML(char const* pMsg) ;

	/*************************************************************
	* @brief	Send a run event
	*************************************************************/
	void FireRunEvent(smlRunEventId eventId) ;

	/*************************************************************
	* @brief	Converts an id from a client side value to a kernel side value.
	*			We need to be able to do this because the client is adding a collection
	*			of wmes at once, so it makes up the ids for those objects.
	*			But the kernel will assign them a different value when the
	*			wme is actually added in the kernel.
	*
	*			RecordIDMapping should be called whenever an identifier is created 
	*			or linked to on the client side because it increments reference counts
	*			to that id so that shared ids work correctly
	*************************************************************/
	bool ConvertID(char const* pClientID, std::string* pKernelID) ;
	void RecordIDMapping(char const* pClientID, char const* pKernelID) ;
	void RemoveID(char const* pKernelID) ;  // BADBAD: this function only referenced in commented out code

	/*************************************************************
	* @brief	Converts a time tag from a client side value to
	*			a kernel side one.
	*************************************************************/
	wme* ConvertKernelTimeTag(char const* pTimeTag) ;
    unsigned long ConvertTime(long clientTimeTag);
	unsigned long ConvertTime(char const* pTimeTag) ;

    void RecordTime(long clientTimeTag, long kernelTimeTag) ;
	void RemoveKernelTime(unsigned long kernelTimeTag);
	long GetClientTimetag( unsigned long kernelTimeTag );

	// Register a RHS function with the Soar kernel
	void RegisterRHSFunction(RhsFunction* pFunction) ;
	void RemoveRHSFunction(RhsFunction* pFunction) ;

	// Utility function (note it's static) for converting a symbol to a string
	static char const* GetValueType(int type);

	// Execute a command line function (through the CLI processor)
	std::string ExecuteCommandLine(std::string const& commmandLine) ;

	/*************************************************************
	* @brief	Used to select which agents run on the next run command.
	*************************************************************/
	void ScheduleAgentToRun(bool state) ;
	void RemoveAgentFromRunList()       { m_ScheduledToRun = false ;}
	bool IsAgentScheduledToRun()		{ return m_ScheduledToRun ; }
	void PutAgentOnStepList(bool state) { m_OnStepList = state; }
	bool IsAgentOnStepList()		    { return m_OnStepList ; }
	bool WasAgentOnRunList()            { return m_WasOnRunList ; }

	smlRunResult	GetResultOfLastRun()		  { return m_ResultOfLastRun ; }
	void SetResultOfRun(smlRunResult runResult) { m_ResultOfLastRun = runResult ; }

	//void SetInitialStepCount(unsigned long count)	{ m_InitialStepCount = count ; }
	void SetInitialRunCount(unsigned long count)	{ m_InitialRunCount = count ; }
	//unsigned long GetInitialStepCount()				{ return m_InitialStepCount ; }
	unsigned long GetInitialRunCount()				{ return m_InitialRunCount ; }
	void ResetLocalRunCounters()                    { m_localRunCount = 0 ; m_localStepCount = 0 ; }
	void IncrementLocalRunCounter()                 { m_localRunCount++ ; }
	void IncrementLocalStepCounter()                { m_localStepCount++ ; }
	bool CompletedRunType(unsigned long count)      { 
		//std::cout << std::endl << GetName() << ": (count" << count << " > (m_InitialRunCount" << m_InitialRunCount << " + m_localRunCount" << m_localRunCount << "))";
		return (count > (m_InitialRunCount + m_localRunCount)) ; 
	}
	void SetCompletedOutputPhase(bool state)		{ m_CompletedOutputPhase = state ; }
	bool HasCompletedOutputPhase()					{ return m_CompletedOutputPhase ; }

	void SetGeneratedOutput(bool state)				{ m_GeneratedOutput = state ; }
	bool HasGeneratedOutput() 						{ return m_GeneratedOutput ; }

	void SetInitialOutputCount(unsigned long count)	{ m_OutputCounter = count ; }
	unsigned long GetInitialOutputCount()			{ return m_OutputCounter ; }

	unsigned long GetNumDecisionsExecuted() ;
	unsigned long GetNumDecisionCyclesExecuted() ;
	unsigned long GetNumElaborationsExecuted() ;
	unsigned long GetNumPhasesExecuted() ;
	unsigned long GetNumOutputsGenerated() ;
	unsigned long GetLastOutputCount() ;
	void ResetLastOutputCount() ;
	smlPhase GetCurrentPhase() ;
	AgentRunCallback* GetAgentRunCallback() { return m_pAgentRunCallback ; }

	unsigned long GetRunCounter(smlRunStepSize runStepSize) ;

	// Request that the agent stop soon.
	void Interrupt(smlStopLocationFlags stopLoc) ;
	void ClearInterrupts() ;
	smlRunResult StepInClientThread(smlRunStepSize  stepSize) ;
	smlRunResult Step(smlRunStepSize stepSize) ;

	unsigned long GetInterruptFlags()		{ return m_interruptFlags ; }
	smlRunState GetRunState()				{ return m_runState ; }
	void SetRunState(smlRunState state)	{ m_runState = state ; }

protected:
	void InitializeRuntimeState() ;

	/*************************************************************
	* @brief	Add an input message to the pending input list
	*			-- it will be processed on the next input phase callback from the kernel.
	*			This agent takes ownership of the input message by adding a reference to it.
	*************************************************************/
	void AddToPendingInputList(ElementXML_Handle hInputMsgHandle) ;

	PendingInputList*	GetPendingInputList() { return &m_PendingInput ; }
} ;

class AgentRunCallback : public KernelCallback
{
	// This is the actual callback for the event
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void*)
	{
		if (smlEVENT_AFTER_OUTPUT_PHASE == eventID) 	
		{
			pAgentSML->SetCompletedOutputPhase(true) ;
			// If the number of outputs generated by this agent has changed record it as
			// having generated output and possibly fire a notification event.
			// InitialOutputCount is updated when the OutputGenerated event is fired.
			unsigned long count = pAgentSML->GetNumOutputsGenerated() ;
			if (count != pAgentSML->GetInitialOutputCount())
			{
				pAgentSML->SetGeneratedOutput(true) ;
			}
		}
	}
} ;

}

#endif // SML_AGENT_SML_H
