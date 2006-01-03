/* File : sml_ClientInterface.i */
%module sml

%csconst(1); // strongly recommended by SWIG manual section 19.3.5.1
// the previous line causes problems for some enum values, so we have to set them manually here
// the problem only affects those enums whose values are "calculated" based on other values
%csconstvalue("smlSystemEventId.smlEVENT_AFTER_RHS_FUNCTION_EXECUTED + 1") smlEVENT_BEFORE_SMALLEST_STEP;
%csconstvalue("smlProductionEventId.smlEVENT_BEFORE_PRODUCTION_RETRACTED + 1") smlEVENT_AFTER_AGENT_CREATED;
%csconstvalue("smlPrintEventId.smlEVENT_PRINT + 1") smlEVENT_RHS_USER_FUNCTION;
%csconstvalue("smlRhsEventId.smlEVENT_RHS_USER_FUNCTION + 1") smlEVENT_XML_TRACE_OUTPUT;
%csconstvalue("smlXMLEventId.smlEVENT_XML_INPUT_RECEIVED + 1") smlEVENT_AFTER_ALL_OUTPUT_PHASES;
%csconstvalue("smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT + 1") smlEVENT_EDIT_PRODUCTION;
%csconstvalue("smlStringEventId.smlEVENT_EDIT_PRODUCTION + 1") smlEVENT_LAST;
%csconstvalue("smlWorkingMemoryEventId.smlEVENT_OUTPUT_PHASE_CALLBACK + 1") smlEVENT_LOG_ERROR;
%csconstvalue("smlRunEventId.smlEVENT_AFTER_RUNNING + 1") smlEVENT_AFTER_PRODUCTION_ADDED;
%csconstvalue("smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED + 1") smlEVENT_OUTPUT_PHASE_CALLBACK;

%ignore sml::Agent::UnregisterForRunEvent(int);
%ignore sml::Agent::UnregisterForProductionEvent(int);
%ignore sml::Agent::UnregisterForPrintEvent(int);
%ignore sml::Agent::UnregisterForXMLEvent(int);
%ignore sml::Agent::RemoveOutputHandler(int);
%ignore sml::Kernel::UnregisterForSystemEvent(int);
%ignore sml::Kernel::UnregisterForUpdateEvent(int);
%ignore sml::Kernel::UnregisterForUntypedEvent(int);
%ignore sml::Kernel::UnregisterForAgentEvent(int);
%ignore sml::Kernel::RemoveRhsFunction(int);

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

		// This registration method will be called as soon as the parent class (Kernel) is loaded.
		static HandleHelper() {
			CSharp_Kernel_RegisterHandleHelper(staticHandleDelegate);
		}
	}

	static protected HandleHelper staticHandleHelper = new HandleHelper();
%}

// DJP: NOTE!  When changing this code make sure the library smlCSharp.dll is getting
// updated.  I have had many cases where Visual Studio keeps the library loaded when it shouldn't causing the build
// to appear to work, but the library is not updated (because it can't be overwritten).
// The simple test is manually deleting the library from Explorer.  If that fails, close the solution and re-open it in VS
// which will break the lock.
%typemap(cscode) sml::Agent %{
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
%}

/*
%typemap(cscode) sml::smlPINVOKE %{
	[DllImport("CSharp_sml_ClientInterface")]
	public static extern void MyFunction(sml.Kernel.MyCallback callback);
%}
*/

// include stuff common to all languages (i.e. Java, Tcl, C#)
%include "../sml_ClientInterface.i"

// include Doug's custom custom code for callbacks in the wrapper section
//  so it's in the extern C block
%wrapper %{
#include "CSharpCallbackByHand.h"
%}

