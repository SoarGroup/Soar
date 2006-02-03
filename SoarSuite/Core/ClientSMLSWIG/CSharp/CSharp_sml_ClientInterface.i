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
%csconstvalue("smlWorkingMemoryEventId.smlEVENT_LAST_WM_EVENT + 1") smlEVENT_LOG_ERROR;
%csconstvalue("smlRunEventId.smlEVENT_LAST_RUN_EVENT + 1") smlEVENT_AFTER_PRODUCTION_ADDED;
%csconstvalue("smlAgentEventId.smlEVENT_LAST_AGENT_EVENT + 1") smlEVENT_OUTPUT_PHASE_CALLBACK;

%ignore sml::Agent::UnregisterForRunEvent(int);
%ignore sml::Agent::UnregisterForProductionEvent(int);
%ignore sml::Agent::UnregisterForPrintEvent(int);
%ignore sml::Agent::UnregisterForXMLEvent(int);
%ignore sml::Agent::UnregisterForOutputNotification(int);
%ignore sml::Agent::RemoveOutputHandler(int);
%ignore sml::Kernel::UnregisterForSystemEvent(int);
%ignore sml::Kernel::UnregisterForUpdateEvent(int);
%ignore sml::Kernel::UnregisterForStringEvent(int);
%ignore sml::Kernel::UnregisterForAgentEvent(int);
%ignore sml::Kernel::RemoveRhsFunction(int);
%ignore sml::Kernel::UnregisterForClientMessageEvent(int);

%typemap(cscode) sml::Kernel %{
	// This class exists to expose the "DeleteHandle" method to the SWIG C++ code, so that we can call back to it to
	// delete a GCHandle.  This code is called to free any GCHandles which were allocated in registering for a callback.
	// All of this, is so that we can pass a pointer into the SWIG/C++ code and ensure that the pointer is not garbage collected
	// until we explicitly indicate we're done with it by calling Free on that pointer.  However, we can't call Free from the C++ code -- 
	// we need to call it from C# and hence the need for this class.
	protected class HandleHelper {

		public delegate void HandleDeletingDelegate(IntPtr intHandle);
		static HandleDeletingDelegate staticHandleDelegate = new HandleDeletingDelegate(DeleteHandle);

		[DllImport("CSharp_sml_ClientInterface")]
		public static extern void CSharp_Kernel_RegisterHandleHelper(HandleDeletingDelegate handleDelegate);

		static void DeleteHandle(IntPtr intHandle)
		{
			GCHandle handle = (GCHandle)intHandle ;
			
			System.Console.Out.WriteLine("Freeing handle" + handle) ;
			handle.Free() ;
		}

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
			System.Console.Out.WriteLine("Created WMElement handle" + handle) ;
			
			return (IntPtr)handle ;
		}

		// This registration method will be called as soon as the parent class (Kernel) is loaded.
		static HandleHelper() {
			CSharp_Kernel_RegisterHandleHelper(staticHandleDelegate);
			CSharp_Kernel_RegisterAllocateWMElementHelper(staticAllocateWMElementDelegate);
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

// include stuff common to all languages (i.e. Java, Tcl, C#)
%include "../sml_ClientInterface.i"

// include Doug's custom custom code for callbacks in the wrapper section
//  so it's in the extern C block
%wrapper %{
#include "CSharpCallbackByHand.h"
%}

