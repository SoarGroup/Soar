/* File : sml_ClientInterface.i */
%module sml

%csconst(1); // strongly recommended by SWIG manual section 19.3.5.1
// the previous line causes problems for some enum values, so we have to set them manually here
// the problem only affects those enums whose values are "calculated" based on other values
%csconstvalue("smlSystemEventId.smlEVENT_LAST_SYSTEM_EVENT + 1") smlEVENT_BEFORE_SMALLEST_STEP;
%csconstvalue("smlProductionEventId.smlEVENT_LAST_PRODUCTION_EVENT + 1") smlEVENT_AFTER_AGENT_CREATED;
%csconstvalue("smlPrintEventId.smlEVENT_LAST_PRINT_EVENT + 1") smlEVENT_RHS_USER_FUNCTION;
%csconstvalue("smlRhsEventId.smlEVENT_LAST_RHS_EVENT + 1") smlEVENT_XML_TRACE_OUTPUT;
%csconstvalue("smlXMLEventId.smlEVENT_LAST_XML_EVENT + 1") smlEVENT_AFTER_ALL_OUTPUT_PHASES;
%csconstvalue("smlUpdateEventId.smlEVENT_LAST_UPDATE_EVENT + 1") smlEVENT_EDIT_PRODUCTION;
%csconstvalue("smlStringEventId.smlEVENT_LAST_STRING_EVENT + 1") smlEVENT_LAST;
%csconstvalue("smlWorkingMemoryEventId.smlEVENT_LAST_WM_EVENT + 1") smlEVENT_ECHO;
%csconstvalue("smlRunEventId.smlEVENT_LAST_RUN_EVENT + 1") smlEVENT_AFTER_PRODUCTION_ADDED;
%csconstvalue("smlAgentEventId.smlEVENT_LAST_AGENT_EVENT + 1") smlEVENT_OUTPUT_PHASE_CALLBACK;

// We replace the SWIG generated shutdown with our own version which will call the SWIG generated one.
%rename(ShutdownInternal) sml::Kernel::Shutdown();

%typemap(cscode) sml::Kernel %{
	// This class exists to expose the "DeleteHandle" method to the SWIG C++ code, so that we can call back to it to
	// delete a GCHandle.  This code is called to free any GCHandles which were allocated in registering for a callback.
	// All of this, is so that we can pass a pointer into the SWIG/C++ code and ensure that the pointer is not garbage collected
	// until we explicitly indicate we're done with it by calling Free on that pointer.  However, we can't call Free from the C++ code -- 
	// we need to call it from C# and hence the need for this class.
	protected class HandleHelper {

		// This chunk of code allows us to call "DeleteHandle" from within C++.
		public delegate void HandleDeletingDelegate(IntPtr intHandle);
		static HandleDeletingDelegate staticHandleDelegate = new HandleDeletingDelegate(DeleteHandle);

		[DllImport("CSharp_sml_ClientInterface")]
		public static extern void CSharp_Kernel_RegisterHandleHelper(HandleDeletingDelegate handleDelegate);

		static void DeleteHandle(IntPtr intHandle)
		{
			GCHandle handle = (GCHandle)intHandle ;
			
			//System.Console.Out.WriteLine("Freeing handle" + handle) ;
			handle.Free() ;
		}

		// This chunk of code allows us to call "AllocateWMElement" from within C++
		public delegate IntPtr AllocateWMElementDelegate(IntPtr intHandle);
		static AllocateWMElementDelegate staticAllocateWMElementDelegate = new AllocateWMElementDelegate(AllocateWMElement);

		[DllImport("CSharp_sml_ClientInterface")]
		public static extern void CSharp_Kernel_RegisterAllocateWMElementHelper(AllocateWMElementDelegate theDelegate);

		static IntPtr AllocateWMElement(IntPtr intCPtr)
		{
			// Creates a new C# object to wrap the C++ one, without taking ownership of the underlying object
			// (as we don't own the C++ object either when we call here).
			// To keep this the user would need to copy it.
			WMElement wmElement = new WMElement(intCPtr, false) ;
			
			GCHandle handle = GCHandle.Alloc(wmElement) ;
			//System.Console.Out.WriteLine("Created WMElement handle" + handle) ;
			
			return (IntPtr)handle ;
		}

		// This chunk of code allows us to call "AllocateClientXML" from within C++
		public delegate IntPtr AllocateClientXMLDelegate(IntPtr intHandle);
		static AllocateClientXMLDelegate staticAllocateClientXMLDelegate = new AllocateClientXMLDelegate(AllocateClientXML);

		[DllImport("CSharp_sml_ClientInterface")]
		public static extern void CSharp_Kernel_RegisterAllocateClientXMLHelper(AllocateClientXMLDelegate theDelegate);

		static IntPtr AllocateClientXML(IntPtr intCPtr)
		{
			// Creates a new C# object to wrap the C++ one, without taking ownership of the underlying object
			// (as we don't own the C++ object either when we call here).
			// To keep this the user would need to copy it.
			ClientXML xml = new ClientXML(intCPtr, false) ;
			
			GCHandle handle = GCHandle.Alloc(xml) ;
			//System.Console.Out.WriteLine("Created ClientXML handle" + handle) ;
			
			return (IntPtr)handle ;
		}

		// This registration method will be called as soon as the parent class (Kernel) is loaded
		// and passes the delegates (callbacks) down to the C++ code by calling the registration methods
		// (which are in C++).  It's complicated stuff, but all we're actually doing is passing a pointer
		// to the 3 C# methods we want to call down to the C++ code.
		static HandleHelper() {
			CSharp_Kernel_RegisterHandleHelper(staticHandleDelegate);
			CSharp_Kernel_RegisterAllocateWMElementHelper(staticAllocateWMElementDelegate);
			CSharp_Kernel_RegisterAllocateClientXMLHelper(staticAllocateClientXMLDelegate);
		}
	}

	static protected HandleHelper staticHandleHelper = new HandleHelper();

	//////////////////////////////////////////////////////////////////////////////////
	//
	// SystemEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// typedef void (*SystemEventHandler)(smlSystemEventId id, void* pUserData, Kernel* pKernel) ;
	public delegate void SystemEventCallback(smlSystemEventId eventID, IntPtr callbackData, IntPtr kernel);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Kernel_RegisterForSystemEvent(HandleRef jarg1, int eventID, IntPtr jkernel, SystemEventCallback callback, IntPtr callbackData);

	public int RegisterForSystemEvent(smlSystemEventId eventID, SystemEventCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle kernelHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Kernel_RegisterForSystemEvent(swigCPtr, (int)eventID, (IntPtr)kernelHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Kernel_UnregisterForSystemEvent(HandleRef jarg1, int callbackID);

	public bool UnregisterForSystemEvent(int jarg2)
	{
		return CSharp_Kernel_UnregisterForSystemEvent(swigCPtr, jarg2) ;
	}

	//////////////////////////////////////////////////////////////////////////////////
	//
	// UpdateEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// Handler for Update events.
	// typedef void (*UpdateEventHandler)(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags) ;
	public delegate void UpdateEventCallback(smlUpdateEventId eventID, IntPtr callbackData, IntPtr kernel, smlRunFlags runFlags);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Kernel_RegisterForUpdateEvent(HandleRef jarg1, int eventID, IntPtr jkernel, UpdateEventCallback callback, IntPtr callbackData);

	public int RegisterForUpdateEvent(smlUpdateEventId eventID, UpdateEventCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle kernelHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Kernel_RegisterForUpdateEvent(swigCPtr, (int)eventID, (IntPtr)kernelHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Kernel_UnregisterForUpdateEvent(HandleRef jarg1, int callbackID);

	public bool UnregisterForUpdateEvent(int jarg2)
	{
		return CSharp_Kernel_UnregisterForUpdateEvent(swigCPtr, jarg2) ;
	}

	//////////////////////////////////////////////////////////////////////////////////
	//
	// StringEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// Handler for Update events.
	// typedef void (*StringEventHandler)(smlStringEventId id, void* pUserData, Kernel* pKernel, char const* pString) ;
	public delegate void StringEventCallback(smlStringEventId eventID, IntPtr callbackData, IntPtr kernel, String str);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Kernel_RegisterForStringEvent(HandleRef jarg1, int eventID, IntPtr jkernel, StringEventCallback callback, IntPtr callbackData);

	public int RegisterForStringEvent(smlStringEventId eventID, StringEventCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle kernelHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Kernel_RegisterForStringEvent(swigCPtr, (int)eventID, (IntPtr)kernelHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Kernel_UnregisterForStringEvent(HandleRef jarg1, int callbackID);

	public bool UnregisterForStringEvent(int jarg2)
	{
		return CSharp_Kernel_UnregisterForStringEvent(swigCPtr, jarg2) ;
	}
	
	//////////////////////////////////////////////////////////////////////////////////
	//
	// RhsEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// Handler for RHS (right hand side) function firings
	// pFunctionName and pArgument define the RHS function being called (the client may parse pArgument to extract other values)
	// The return value is a string which allows the RHS function to create a symbol: e.g. ^att (exec plus 2 2) producting ^att 4
	// typedef std::string (*RhsEventHandler)(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) ;
	public delegate String RhsFunction(smlRhsEventId eventID, IntPtr callbackData, IntPtr kernel, String agentName, String functionName, String argument);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Kernel_AddRhsFunction(HandleRef jarg1, String functionName, IntPtr jkernel, RhsFunction callback, IntPtr callbackData);

	public int AddRhsFunction(String functionName, RhsFunction jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle kernelHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Kernel_AddRhsFunction(swigCPtr, functionName, (IntPtr)kernelHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Kernel_RemoveRhsFunction(HandleRef jarg1, int callbackID);

	public bool RemoveRhsFunction(int jarg2)
	{
		return CSharp_Kernel_RemoveRhsFunction(swigCPtr, jarg2) ;
	}	

	//////////////////////////////////////////////////////////////////////////////////
	//
	// ClientMessageEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// Handler for a generic "client message".  The content is determined by the client sending this data.
	// The message is sent as a simple string and the response is also a string.  The string can contain data that is intended to be parsed,
	// such as a simple series of integers up to a complete XML message.
	// typedef std::string (*ClientMessageHandler)(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pClientName, char const* pMessage) ;
	public delegate String ClientMessageCallback(smlRhsEventId eventID, IntPtr callbackData, IntPtr kernel, String agentName, String clientName, String message);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Kernel_RegisterForClientMessageEvent(HandleRef jarg1, String clientName, IntPtr jkernel, ClientMessageCallback callback, IntPtr callbackData);

	public int RegisterForClientMessageEvent(String clientName, ClientMessageCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle kernelHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Kernel_RegisterForClientMessageEvent(swigCPtr, clientName, (IntPtr)kernelHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Kernel_UnregisterForClientMessageEvent(HandleRef jarg1, int callbackID);

	public bool UnregisterForClientMessageEvent(int jarg2)
	{
		return CSharp_Kernel_UnregisterForClientMessageEvent(swigCPtr, jarg2) ;
	}	
	
	//////////////////////////////////////////////////////////////////////////////////
	//
	// AgentEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// typedef void (*AgentEventHandler)(smlAgentEventId id, void* pUserData, Agent* pAgent);
	public delegate void AgentEventCallback(smlAgentEventId eventID, IntPtr callbackData, IntPtr kernel, String agentName);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Kernel_RegisterForAgentEvent(HandleRef jarg1, int eventID, IntPtr jkernel, AgentEventCallback callback, IntPtr callbackData);

	public int RegisterForAgentEvent(smlAgentEventId eventID, AgentEventCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle kernelHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Kernel_RegisterForAgentEvent(swigCPtr, (int)eventID, (IntPtr)kernelHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Kernel_UnregisterForAgentEvent(HandleRef jarg1, int callbackID);

	public bool UnregisterForAgentEvent(int jarg2)
	{
		return CSharp_Kernel_UnregisterForAgentEvent(swigCPtr, jarg2) ;
	}
	
	// In C# we want to explicitly delete the C++ kernel object after calling shutdown so that the user
	// doesn't have to call ".Dispose()" on their C# object (or wait for the garbage collector to do it which may never run--leading to
	// reports of memory leaks on shutdown).  In C++ users expect to have to delete their kernel pointer but not in C#.
	public void Shutdown() {
		ShutdownInternal();
		Dispose() ;
	}

%}

// DJP: NOTE!  When changing this code make sure the library smlCSharp.dll is getting
// updated.  I have had many cases where Visual Studio keeps the library loaded when it shouldn't causing the build
// to appear to work, but the library is not updated (because it can't be overwritten).
// The simple test is manually deleting the library from Explorer.  If that fails, close the solution and re-open it in VS
// which will break the lock.
%typemap(cscode) sml::Agent %{
	//////////////////////////////////////////////////////////////////////////////////
	//
	// RunEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// typedef void (*RunEventHandler)(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase);
	public delegate void RunEventCallback(smlRunEventId eventID, IntPtr callbackData, IntPtr agent, smlPhase phase);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Agent_RegisterForRunEvent(HandleRef jarg1, int eventID, IntPtr jagent, RunEventCallback callback, IntPtr callbackData);

	public int RegisterForRunEvent(smlRunEventId eventID, RunEventCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle agentHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Agent_RegisterForRunEvent(swigCPtr, (int)eventID, (IntPtr)agentHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Agent_UnregisterForRunEvent(HandleRef jarg1, int callbackID);

	public bool UnregisterForRunEvent(int jarg2)
	{
		return CSharp_Agent_UnregisterForRunEvent(swigCPtr, jarg2) ;
	}

	//////////////////////////////////////////////////////////////////////////////////
	//
	// OutputNotification
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// This is a simpler notification event -- it just tells you that some output was received for this agent.
	// You then call to the other client side methods to determine what has changed.
	// typedef void (*OutputNotificationHandler)(void* pUserData, Agent* pAgent) ;
	public delegate void OutputNotificationCallback(IntPtr callbackData, IntPtr agent);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Agent_RegisterForOutputNotification(HandleRef jarg1, IntPtr jagent, OutputNotificationCallback callback, IntPtr callbackData);

	public int RegisterForOutputNotification(OutputNotificationCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle agentHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Agent_RegisterForOutputNotification(swigCPtr, (IntPtr)agentHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Agent_UnregisterForOutputNotification(HandleRef jarg1, int callbackID);

	public bool UnregisterForOutputNotification(int jarg2)
	{
		return CSharp_Agent_UnregisterForOutputNotification(swigCPtr, jarg2) ;
	}

	//////////////////////////////////////////////////////////////////////////////////
	//
	// OutputEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// You register a specific attribute name (e.g. "move") and when this attribute appears on the output link (^io.output-link.move M3)
	// you are passed the working memory element ((I3 ^move M3) in this case) in the callback.  This mimics gSKI's output producer model.
	//typedef void (*OutputEventHandler)(void* pUserData, Agent* pAgent, char const* pCommandName, WMElement* pOutputWme) ;
	public delegate void OutputEventCallback(IntPtr callbackData, IntPtr agent, String commandName, IntPtr outputWME);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Agent_AddOutputHandler(HandleRef jarg1, IntPtr jagent, String attributeName, OutputEventCallback callback, IntPtr callbackData);

	public int AddOutputHandler(String attributeName, OutputEventCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle agentHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Agent_AddOutputHandler(swigCPtr, (IntPtr)agentHandle, attributeName, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Agent_RemoveOutputHandler(HandleRef jarg1, int callbackID);

	public bool RemoveOutputHandler(int jarg2)
	{
		return CSharp_Agent_RemoveOutputHandler(swigCPtr, jarg2) ;
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
	public delegate void XMLEventCallback(smlXMLEventId eventID, IntPtr callbackData, IntPtr agent, IntPtr pXML);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Agent_RegisterForXMLEvent(HandleRef jarg1, int eventID, IntPtr jagent, XMLEventCallback callback, IntPtr callbackData);

	public int RegisterForXMLEvent(smlXMLEventId eventID, XMLEventCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle agentHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Agent_RegisterForXMLEvent(swigCPtr, (int)eventID, (IntPtr)agentHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Agent_UnregisterForXMLEvent(HandleRef jarg1, int callbackID);

	public bool UnregisterForXMLEvent(int jarg2)
	{
		return CSharp_Agent_UnregisterForXMLEvent(swigCPtr, jarg2) ;
	}
	
	//////////////////////////////////////////////////////////////////////////////////
	//
	// ProductionEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// typedef void (*ProductionEventHandler)(smlProductionEventId id, void* pUserData, Agent* pAgent, char const* pProdName, char const* pInstantiation);
	public delegate void ProductionEventCallback(smlProductionEventId eventID, IntPtr callbackData, IntPtr agent, String prodName, String instantiation);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Agent_RegisterForProductionEvent(HandleRef jarg1, int eventID, IntPtr jagent, ProductionEventCallback callback, IntPtr callbackData);

	public int RegisterForProductionEvent(smlProductionEventId eventID, ProductionEventCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle agentHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Agent_RegisterForProductionEvent(swigCPtr, (int)eventID, (IntPtr)agentHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Agent_UnregisterForProductionEvent(HandleRef jarg1, int callbackID);

	public bool UnregisterForProductionEvent(int jarg2)
	{
		return CSharp_Agent_UnregisterForProductionEvent(swigCPtr, jarg2) ;
	}

	//////////////////////////////////////////////////////////////////////////////////
	//
	// PrintEvent
	//
	//////////////////////////////////////////////////////////////////////////////////
	// C++ equivalent:
	// typedef void (*PrintEventHandler)(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage) ;
	public delegate void PrintEventCallback(smlPrintEventId eventID, IntPtr callbackData, IntPtr agent, String message);

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Agent_RegisterForPrintEvent(HandleRef jarg1, int eventID, IntPtr jagent, PrintEventCallback callback, IntPtr callbackData);

	public int RegisterForPrintEvent(smlPrintEventId eventID, PrintEventCallback jarg2, Object callbackData)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It's also an approved way to pass a pointer to unsafe (C++) code and get it back.
		// Also, somewhat remarkably, we can pass null to GCHandle.Alloc() and get back a valid object, so no need to special case that.
		GCHandle agentHandle = GCHandle.Alloc(this) ;
		GCHandle callbackDataHandle = GCHandle.Alloc(callbackData) ;
		
		return CSharp_Agent_RegisterForPrintEvent(swigCPtr, (int)eventID, (IntPtr)agentHandle, jarg2, (IntPtr)callbackDataHandle) ;
	}

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern bool CSharp_Agent_UnregisterForPrintEvent(HandleRef jarg1, int callbackID);

	public bool UnregisterForPrintEvent(int jarg2)
	{
		return CSharp_Agent_UnregisterForPrintEvent(swigCPtr, jarg2) ;
	}

%}

// Add cleanup code to Shutdown (which is actually renamed to ShutdownInternal)
%exception sml::Kernel::Shutdown {
		$action
		// Release remaining CSharpCallbackData's
		std::list<CSharpCallbackData*>::iterator itr;
		for(itr=callbackdatas.begin(); itr!=callbackdatas.end(); itr++)
		{
			delete (*itr);
		}
		callbackdatas.clear();
}

// include stuff common to all languages (i.e. Java, Tcl, C#)
%include "../sml_ClientInterface.i"

%{
#ifdef __cplusplus
extern "C" {
#endif
#include "CSharpCallbackByHand.h"
#ifdef __cplusplus
}
#endif
%}

