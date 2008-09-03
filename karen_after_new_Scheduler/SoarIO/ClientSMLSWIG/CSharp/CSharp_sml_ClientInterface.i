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
%csconstvalue("smlUntypedEventId.smlEVENT_EDIT_PRODUCTION + 1") smlEVENT_LAST;
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
	public delegate void MyCallback();

	[DllImport("CSharp_sml_ClientInterface")]
	public static extern void CSharp_Kernel_RegisterTestMethod(HandleRef jarg1, MyCallback callback);
	
	public void RegisterTestMethod(MyCallback callback)
	{
		CSharp_Kernel_RegisterTestMethod(swigCPtr, callback);
	}
%}

//typedef void (*RunEventHandler)(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase);

%typemap(cscode) sml::Agent %{
	public delegate void MyRunCallback(IntPtr agent, smlRunEventId eventID);

	// DJP: NOTE!  When changing this code make sure the library smlCSharp.dll is getting
	// updated.  I have had many cases where Visual Studio keeps the library loaded when it shouldn't causing the build
	// to appear to work, but the library is not updated (because it can't be overwritten).
	// The simple test is manually deleting the library from Explorer.  If that fails, close the solution and re-open it in VS
	// which will break the lock.
	[DllImport("CSharp_sml_ClientInterface")]
	public static extern int CSharp_Agent_RegisterForRunEvent(HandleRef jarg1, int eventID, IntPtr jagent, MyRunCallback callbacks);

	public int RegisterForRunEvent(smlRunEventId eventID, MyRunCallback jarg2)
	{
		// This call ensures the garbage collector won't delete the object until we call free on the handle.
		// It also uses an approved way to pass a pointer to unsafe (C++) code and get it back.
		GCHandle agentHandle = GCHandle.Alloc(this) ;
		return CSharp_Agent_RegisterForRunEvent(swigCPtr, (int)eventID, (IntPtr)agentHandle, jarg2) ;
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

