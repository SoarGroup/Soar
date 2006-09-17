/////////////////////////////////////////////////////////////////
// CSharp callback support methods
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2005
//
// Some handcoded methods to support registering callbacks for
// events through CSharp.  This is the part of the Soar/SML interface
// that SWIG can't auto generate.
//

typedef int	agentPtr ;
typedef int	CallbackDataPtr ;
typedef int kernelPtr ;

/* Callback for deleting GCHandle objects from within C#, so we don't leak them. */
typedef void (SWIGSTDCALL* CSharpHandleHelperCallback)(unsigned int);
static CSharpHandleHelperCallback SWIG_csharp_deletehandle_callback = NULL;

SWIGEXPORT void SWIGSTDCALL CSharp_Kernel_RegisterHandleHelper(CSharpHandleHelperCallback callback) {
  SWIG_csharp_deletehandle_callback = callback;
}

/* Callback for allocating new C# WMElement objects that we need to create (to pass back as parameters) */
typedef unsigned int (SWIGSTDCALL* CSharpAllocateWMElementCallback)(unsigned int);
static CSharpAllocateWMElementCallback SWIG_csharp_allocateWMElement_callback = NULL;

SWIGEXPORT void SWIGSTDCALL CSharp_Kernel_RegisterAllocateWMElementHelper(CSharpAllocateWMElementCallback callback) {
  SWIG_csharp_allocateWMElement_callback = callback;
}

/* Callback for allocating new C# ClientXML objects that we need to create (to pass back as parameters) */
typedef unsigned int (SWIGSTDCALL* CSharpAllocateClientXMLCallback)(unsigned int);
static CSharpAllocateClientXMLCallback SWIG_csharp_allocateClientXML_callback = NULL;

SWIGEXPORT void SWIGSTDCALL CSharp_Kernel_RegisterAllocateClientXMLHelper(CSharpAllocateClientXMLCallback callback) {
  SWIG_csharp_allocateClientXML_callback = callback;
}

class CSharpCallbackData
{
// Making these public as this is basically just a struct.
public:
	int				m_EventID ;
	agentPtr		m_Agent ;
	kernelPtr		m_Kernel ;
	void*			m_CallbackFunction ;
	CallbackDataPtr	m_CallbackData ;
	int				m_CallbackID ;

public:
	CSharpCallbackData(agentPtr jagent, kernelPtr jkernel, int eventID, void* callbackFunction, unsigned int callbackData)
	{
		m_Agent = jagent ;
		m_Kernel = jkernel ;
		m_EventID = eventID ;
		m_CallbackFunction = callbackFunction ;
		m_CallbackData = callbackData ;
		m_CallbackID = 0 ;
	}

	~CSharpCallbackData()
	{
		// Free the GCHandles created when the callback was registered
		if(m_Agent != NULL) SWIG_csharp_deletehandle_callback(m_Agent) ;
		if(m_Kernel != NULL) SWIG_csharp_deletehandle_callback(m_Kernel) ;
		SWIG_csharp_deletehandle_callback(m_CallbackData) ;
	}
} ;

std::list<CSharpCallbackData*> callbackdatas;

void ReleaseCallbackData(CSharpCallbackData* pData) {
	// Release callback data and remove from collection of those we need to release at shutdown
	std::list<CSharpCallbackData*>::iterator itr = find(callbackdatas.begin(), callbackdatas.end(), pData);
	if(itr != callbackdatas.end()) {
		callbackdatas.erase(itr);
		delete pData;
	}
}

bool IsValidCallbackData(CSharpCallbackData* pData) {
	std::list<CSharpCallbackData*>::iterator itr = find(callbackdatas.begin(), callbackdatas.end(), pData);
	if(itr == callbackdatas.end()) {
		return false;
	} else {
		return true;
	}
}

static CSharpCallbackData* CreateCSharpCallbackDataAgent(agentPtr jagent, int eventID, unsigned int callbackFunction, CallbackDataPtr callbackData)
{
	CSharpCallbackData* pData = new CSharpCallbackData(jagent, NULL, eventID, (void *)callbackFunction, callbackData) ;

	// Save the callback data so we can free it later
	callbackdatas.push_back(pData);

	return pData;
}

static CSharpCallbackData* CreateCSharpCallbackDataKernel(kernelPtr jkernel, int eventID, unsigned int callbackFunction, CallbackDataPtr callbackData)
{
	CSharpCallbackData* pData = new CSharpCallbackData(NULL, jkernel, eventID, (void *)callbackFunction, callbackData) ;
	
	// Save the callback data so we can free it later
	callbackdatas.push_back(pData);

	return pData;
}

//////////////////////////////////////////////////////////////////////////////////
//
// RunEvent
//
//////////////////////////////////////////////////////////////////////////////////

// The callback we want to support
// typedef void (*RunEventHandler)(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase);

// The C# callback equivalent that we'll eventually call
typedef void (__stdcall *RunEventCallback)(int eventID, CallbackDataPtr callbackData, agentPtr jagent, int phase) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void RunEventHandler(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	RunEventCallback callback = (RunEventCallback)pData->m_CallbackFunction ;

	// Now try to call back to CSharp
	callback(pData->m_EventID, pData->m_CallbackData, pData->m_Agent, phase) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Agent_RegisterForRunEvent(void * jarg1, int jarg2, agentPtr jagent, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlRunEventId arg2 = (sml::smlRunEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataAgent(jagent, jarg2, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForRunEvent(arg2, &RunEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Agent_UnregisterForRunEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForRunEvent(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// OutputNotification
//
//////////////////////////////////////////////////////////////////////////////////

// The callback we want to support
// This is a simpler notification event -- it just tells you that some output was received for this agent.
// You then call to the other client side methods to determine what has changed.
// typedef void (*OutputNotificationHandler)(void* pUserData, Agent* pAgent) ;

// The C# callback equivalent that we'll eventually call
typedef void (__stdcall *OutputNotificationCallback)(CallbackDataPtr callbackData, agentPtr jagent) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void OutputNotificationHandler(void* pUserData, sml::Agent* pAgent)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	OutputNotificationCallback callback = (OutputNotificationCallback)pData->m_CallbackFunction ;

	// Now try to call back to CSharp
	callback(pData->m_CallbackData, pData->m_Agent) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Agent_RegisterForOutputNotification(void * jarg1, agentPtr jagent, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	// sml::smlOutputNotificationId arg2 = (sml::smlOutputNotificationId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataAgent(jagent, 0, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForOutputNotification(&OutputNotificationHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Agent_UnregisterForOutputNotification(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForOutputNotification(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// OutputEvent
//
//////////////////////////////////////////////////////////////////////////////////

// The callback we want to support
// You register a specific attribute name (e.g. "move") and when this attribute appears on the output link (^io.output-link.move M3)
// you are passed the working memory element ((I3 ^move M3) in this case) in the callback.  This mimics gSKI's output producer model.
// typedef void (*OutputEventHandler)(void* pUserData, Agent* pAgent, char const* pCommandName, WMElement* pOutputWme) ;

// The C# callback equivalent that we'll eventually call
typedef void (__stdcall *OutputEventCallback)(CallbackDataPtr callbackData, agentPtr jagent, char const* pCommandName, unsigned int outputWME) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void OutputEventHandler(void* pUserData, sml::Agent* pAgent, char const* pCommandName, sml::WMElement* pOutputWME)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	OutputEventCallback callback = (OutputEventCallback)pData->m_CallbackFunction ;

	// Create a C# string which we can return
	char* csharpProdName = SWIG_csharp_string_callback(pCommandName);

	// Create a C# object that wraps the C++ one, without taking ownership of it
	// (This is done by calling to the C# constructor for the SWIG object)
	unsigned int csharpOutputWME = SWIG_csharp_allocateWMElement_callback((unsigned int)pOutputWME) ;

	// Now try to call back to CSharp
	callback(pData->m_CallbackData, pData->m_Agent, pCommandName, csharpOutputWME) ;

	// After the callback has completed, I think we need to release the GCHandle that wrapped the C# object
	// (This wrapping and deleting may be unnecessary, but it makes me feel more comfortable while passing objects
	// between C# and C++).
	SWIG_csharp_deletehandle_callback(csharpOutputWME) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Agent_AddOutputHandler(void * jarg1, agentPtr jagent, char const* pAttributeName, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	//sml::smlOutputEventId arg2 = (sml::smlOutputEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataAgent(jagent, 0, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->AddOutputHandler(pAttributeName, &OutputEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Agent_RemoveOutputHandler(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->RemoveOutputHandler(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// XMLEvent
//
//////////////////////////////////////////////////////////////////////////////////

// Handler for XML events.  The data for the event is passed back in pXML.
// NOTE: To keep a copy of the ClientXML* you are passed use ClientXML* pMyXML = new ClientXML(pXML) to create
// a copy of the object.  This is very efficient and just adds a reference to the underlying XML message object.
// You need to delete ClientXML objects you create and you should not delete the pXML object you are passed.
// typedef void (*XMLEventHandler)(smlXMLEventId id, void* pUserData, Agent* pAgent, ClientXML* pXML) ;

// The C# callback equivalent that we'll eventually call
typedef void (__stdcall *XMLEventCallback)(int eventID, CallbackDataPtr callbackData, agentPtr jagent, unsigned int pXML) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void XMLEventHandler(sml::smlXMLEventId id, void* pUserData, sml::Agent* pAgent, sml::ClientXML* pXML)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	XMLEventCallback callback = (XMLEventCallback)pData->m_CallbackFunction ;

	// Create a C# object that wraps the C++ one, without taking ownership of it
	// (This is done by calling to the C# constructor for the SWIG object)
	unsigned int csharpXML = SWIG_csharp_allocateClientXML_callback((unsigned int)pXML) ;

	// Now try to call back to CSharp
	callback(id, pData->m_CallbackData, pData->m_Agent, csharpXML) ;

	// After the callback has completed, I think we need to release the GCHandle that wrapped the C# object
	// (This wrapping and deleting may be unnecessary, but it makes me feel more comfortable while passing objects
	// between C# and C++).
	SWIG_csharp_deletehandle_callback(csharpXML) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Agent_RegisterForXMLEvent(void * jarg1, int jarg2, agentPtr jagent, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlXMLEventId arg2 = (sml::smlXMLEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataAgent(jagent, jarg2, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForXMLEvent(arg2, &XMLEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Agent_UnregisterForXMLEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForXMLEvent(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// ProductionEvent
//
//////////////////////////////////////////////////////////////////////////////////

// The callback we want to support
// typedef void (*ProductionEventHandler)(smlProductionEventId id, void* pUserData, Agent* pAgent, char const* pProdName, char const* pInstantion) ;

// The C# callback equivalent that we'll eventually call
typedef void (__stdcall *ProductionEventCallback)(int eventID, CallbackDataPtr callbackData, agentPtr jagent, char* prodName, char* instantiation) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void ProductionEventHandler(sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	ProductionEventCallback callback = (ProductionEventCallback)pData->m_CallbackFunction ;

	// Create a C# string which we can return
	char* csharpProdName = SWIG_csharp_string_callback(pProdName); 
	char* csharpInstantiation = SWIG_csharp_string_callback(pInstantiation) ;

	// Now try to call back to CSharp
	callback(pData->m_EventID, pData->m_CallbackData, pData->m_Agent, csharpProdName, csharpInstantiation) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Agent_RegisterForProductionEvent(void * jarg1, int jarg2, agentPtr jagent, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlProductionEventId arg2 = (sml::smlProductionEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataAgent(jagent, jarg2, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForProductionEvent(arg2, &ProductionEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Agent_UnregisterForProductionEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForProductionEvent(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// PrintEvent
//
//////////////////////////////////////////////////////////////////////////////////

// The callback we want to support
// Handler for Print events.
//typedef void (*PrintEventHandler)(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage) ;

// The C# callback equivalent that we'll eventually call
typedef void (__stdcall *PrintEventCallback)(int eventID, CallbackDataPtr callbackData, agentPtr jagent, char* pMessage) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void PrintEventHandler(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	PrintEventCallback callback = (PrintEventCallback)pData->m_CallbackFunction ;

	// Create a C# string which we can return
	char* csharpMessage = SWIG_csharp_string_callback(pMessage); 

	// Now try to call back to CSharp
	callback(pData->m_EventID, pData->m_CallbackData, pData->m_Agent, csharpMessage) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Agent_RegisterForPrintEvent(void * jarg1, int jarg2, agentPtr jagent, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlPrintEventId arg2 = (sml::smlPrintEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataAgent(jagent, jarg2, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForPrintEvent(arg2, &PrintEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Agent_UnregisterForPrintEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForPrintEvent(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// SystemEvent
//
//////////////////////////////////////////////////////////////////////////////////

// The callback we want to support
// Handler for System events.
// typedef void (*SystemEventHandler)(smlSystemEventId id, void* pUserData, Kernel* pKernel) ;

// The C# callback equivalent that we'll eventually call 
typedef void (__stdcall *SystemEventCallback)(int eventID, CallbackDataPtr callbackData, kernelPtr jKernel) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void SystemEventHandler(sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	SystemEventCallback callback = (SystemEventCallback)pData->m_CallbackFunction ;

	// Now try to call back to CSharp
	callback(pData->m_EventID, pData->m_CallbackData, pData->m_Kernel) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Kernel_RegisterForSystemEvent(void * jarg1, int jarg2, kernelPtr jkernel, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlSystemEventId arg2 = (sml::smlSystemEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataKernel(jkernel, jarg2, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForSystemEvent(arg2, &SystemEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Kernel_UnregisterForSystemEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForSystemEvent(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// UpdateEvent
//
//////////////////////////////////////////////////////////////////////////////////

// The callback we want to support
// Handler for Update events.
// typedef void (*UpdateEventHandler)(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags) ;

// The C# callback equivalent that we'll eventually call 
typedef void (__stdcall *UpdateEventCallback)(int eventID, CallbackDataPtr callbackData, kernelPtr jKernel, int runFlags) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void UpdateEventHandler(sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	UpdateEventCallback callback = (UpdateEventCallback)pData->m_CallbackFunction ;

	// Now try to call back to CSharp
	callback(pData->m_EventID, pData->m_CallbackData, pData->m_Kernel, runFlags) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Kernel_RegisterForUpdateEvent(void * jarg1, int jarg2, kernelPtr jkernel, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlUpdateEventId arg2 = (sml::smlUpdateEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataKernel(jkernel, jarg2, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForUpdateEvent(arg2, &UpdateEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Kernel_UnregisterForUpdateEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForUpdateEvent(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// StringEvent
//
//////////////////////////////////////////////////////////////////////////////////
// The callback we want to support
// Handler for string based events.
// typedef void (*StringEventHandler)(smlStringEventId id, void* pUserData, Kernel* pKernel, char const* pString) ;

// The C# callback equivalent that we'll eventually call 
typedef void (__stdcall *StringEventCallback)(int eventID, CallbackDataPtr callbackData, kernelPtr jKernel, char const* pString) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void StringEventHandler(sml::smlStringEventId id, void* pUserData, sml::Kernel* pKernel, char const* pString)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	StringEventCallback callback = (StringEventCallback)pData->m_CallbackFunction ;

	// Now try to call back to CSharp
	callback(pData->m_EventID, pData->m_CallbackData, pData->m_Kernel, pString) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Kernel_RegisterForStringEvent(void * jarg1, int jarg2, kernelPtr jkernel, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlStringEventId arg2 = (sml::smlStringEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataKernel(jkernel, jarg2, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForStringEvent(arg2, &StringEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Kernel_UnregisterForStringEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForStringEvent(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// RhsEvent and ClientMessageHandler
//
//////////////////////////////////////////////////////////////////////////////////

// The callback we want to support
// Handler for RHS (right hand side) function firings
// pFunctionName and pArgument define the RHS function being called (the client may parse pArgument to extract other values)
// The return value is a string which allows the RHS function to create a symbol: e.g. ^att (exec plus 2 2) producting ^att 4
// typedef std::string (*RhsEventHandler)(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) ;

// Handler for a generic "client message".  The content is determined by the client sending this data.
// The message is sent as a simple string and the response is also a string.  The string can contain data that is intended to be parsed,
// such as a simple series of integers up to a complete XML message.
//typedef std::string (*ClientMessageHandler)(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pClientName, char const* pMessage) ;

// The C# callback equivalent that we'll eventually call (we pass back the name of the agent so we don't have to locate the C# Agent object)
typedef char const* (__stdcall *RhsFunction)(int eventID, CallbackDataPtr callbackData, kernelPtr jKernel, char const* agentName, char const* pFunctionName, char const* pArgument) ;
typedef char const* (__stdcall *ClientMessageCallback)(int eventID, CallbackDataPtr callbackData, kernelPtr jKernel, char const* agentName, char const* pClientName, char const* pMessage) ;

// This is a bit ugly.  We compile this header with extern "C" around it so that the public methods can be
// exposed in a DLL with C naming (not C++ mangled names).  However, RhsEventHandler (below) returns a std::string
// which won't compile under "C"...even though it's a static function and hence won't appear in the DLL anyway.
// The solution is to turn off extern "C" for this method and turn it back on afterwards.  Here is where we turn off extern "C".
#ifdef __cplusplus
}
#endif

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static std::string RhsEventHandler(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pFunctionName, char const* pArgument)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	RhsFunction callback = (RhsFunction)pData->m_CallbackFunction ;

	// Create a C# string which we can return
	char* csharpAgentName	 = SWIG_csharp_string_callback(pAgent->GetAgentName()); 
	char* csharpFunctionName = SWIG_csharp_string_callback(pFunctionName); 
	char* csharpArgument	 = SWIG_csharp_string_callback(pArgument); 

	// Now try to call back to CSharp
	return callback(pData->m_EventID, pData->m_CallbackData, pData->m_Kernel, csharpAgentName, csharpFunctionName, csharpArgument) ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static std::string ClientEventHandler(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pClientName, char const* pMessage)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	ClientMessageCallback callback = (ClientMessageCallback)pData->m_CallbackFunction ;

	// Create a C# string which we can return
	char* csharpAgentName	= SWIG_csharp_string_callback(pAgent->GetAgentName()); 
	char* csharpClientName	= SWIG_csharp_string_callback(pClientName); 
	char* csharpMessage		= SWIG_csharp_string_callback(pMessage); 

	// Now try to call back to CSharp
	return callback(pData->m_EventID, pData->m_CallbackData, pData->m_Kernel, csharpAgentName, csharpClientName, csharpMessage) ;
}

// This is a bit ugly.  We compile this header with extern "C" around it so that the public methods can be
// exposed in a DLL with C naming (not C++ mangled names).  However, RhsEventHandler (above) returns a std::string
// which won't compile under "C"...even though it's a static function and hence won't appear in the DLL anyway.
// The solution is to turn off extern "C" for this method and turn it back on afterwards.  Here is where we turn extern "C" back on.
#ifdef __cplusplus
extern "C" {
#endif

SWIGEXPORT int SWIGSTDCALL CSharp_Kernel_AddRhsFunction(void * jarg1, char const* pFunctionName, kernelPtr jkernel, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	// sml::smlRhsEventId arg2 = (sml::smlRhsEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataKernel(jkernel, 0, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->AddRhsFunction(pFunctionName, &RhsEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Kernel_RemoveRhsFunction(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->RemoveRhsFunction(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Kernel_RegisterForClientMessageEvent(void * jarg1, char const* pClientName, kernelPtr jkernel, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	// sml::smlRhsEventId arg2 = (sml::smlRhsEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataKernel(jkernel, 0, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForClientMessageEvent(pClientName, &ClientEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Kernel_UnregisterForClientMessageEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForClientMessageEvent(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

//////////////////////////////////////////////////////////////////////////////////
//
// AgentEvent
//
//////////////////////////////////////////////////////////////////////////////////

// The callback we want to support
// Handler for Agent events (such as creation/destruction etc.).
//typedef void (*AgentEventHandler)(smlAgentEventId id, void* pUserData, Agent* pAgent) ;

// The C# callback equivalent that we'll eventually call (we pass back the name of the agent so we don't have to locate the C# Agent object)
typedef void (__stdcall *AgentEventCallback)(int eventID, CallbackDataPtr callbackData, kernelPtr jKernel, char* agentName) ;

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void AgentEventHandler(sml::smlAgentEventId id, void* pUserData, sml::Agent* pAgent)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	AgentEventCallback callback = (AgentEventCallback)pData->m_CallbackFunction ;

	// Create a C# string which we can return
	char* csharpAgentName = SWIG_csharp_string_callback(pAgent->GetAgentName()); 

	// Now try to call back to CSharp
	callback(pData->m_EventID, pData->m_CallbackData, pData->m_Kernel, csharpAgentName) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Kernel_RegisterForAgentEvent(void * jarg1, int jarg2, kernelPtr jkernel, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlAgentEventId arg2 = (sml::smlAgentEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackDataKernel(jkernel, jarg2, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForAgentEvent(arg2, &AgentEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}

SWIGEXPORT bool SWIGSTDCALL CSharp_Kernel_UnregisterForAgentEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Don't try to release invalid data
	if(!IsValidCallbackData(pData)) return false;

	// Unregister our handler.
	bool result = arg1->UnregisterForAgentEvent(pData->m_CallbackID) ;

	// Release callback data and remove from collection of those we need to remove at shutdown
	ReleaseCallbackData(pData);

	return result ;
}

