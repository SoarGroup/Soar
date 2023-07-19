/* File : sml_ClientInterface.i */
%module sml

// generate proper Java enums (this means we require Java 1.5 or later)
%include "enums.swg"

// handle windows calling convention, __declspec(dllimport), correctly
%include <windows.i>

%javaconst(1); // strongly recommended by SWIG manual section 19.3.5.1
// the previous line causes problems for some enum values, so we have to set them manually here
// the problem only affects those enums whose values are "calculated" based on other values
%javaconstvalue("smlSystemEventId.smlEVENT_LAST_SYSTEM_EVENT.swigValue() + 1") smlEVENT_BEFORE_SMALLEST_STEP;
%javaconstvalue("smlProductionEventId.smlEVENT_LAST_PRODUCTION_EVENT.swigValue() + 1") smlEVENT_AFTER_AGENT_CREATED;
%javaconstvalue("smlPrintEventId.smlEVENT_LAST_PRINT_EVENT.swigValue() + 1") smlEVENT_RHS_USER_FUNCTION;
%javaconstvalue("smlRhsEventId.smlEVENT_LAST_RHS_EVENT.swigValue() + 1") smlEVENT_XML_TRACE_OUTPUT;
%javaconstvalue("smlXMLEventId.smlEVENT_LAST_XML_EVENT.swigValue() + 1") smlEVENT_AFTER_ALL_OUTPUT_PHASES;
%javaconstvalue("smlUpdateEventId.smlEVENT_LAST_UPDATE_EVENT.swigValue() + 1") smlEVENT_TCL_LIBRARY_MESSAGE;
%javaconstvalue("smlStringEventId.smlEVENT_LAST_STRING_EVENT.swigValue() + 1") smlEVENT_LAST;
%javaconstvalue("smlWorkingMemoryEventId.smlEVENT_LAST_WM_EVENT.swigValue() + 1") smlEVENT_ECHO;
%javaconstvalue("smlRunEventId.smlEVENT_LAST_RUN_EVENT.swigValue() + 1") smlEVENT_AFTER_PRODUCTION_ADDED;
%javaconstvalue("smlAgentEventId.smlEVENT_LAST_AGENT_EVENT.swigValue() + 1") smlEVENT_OUTPUT_PHASE_CALLBACK;

// SWIG can't handle enum values that are defined in terms of other values.
// It generates lines like this:
// public final static smlPrintEventId smlEVENT_LAST_PRINT_EVENT = new smlPrintEventId("smlEVENT_LAST_PRINT_EVENT", smlEVENT_PRINT);
// but there's no constructor for smlPrintEventId(String, smlPrintEventId id).
// The solution is either to write a line like this:
//%javaconstvalue("smlPrintEventId.smlEVENT_PRINT.swigValue()") smlEVENT_LAST_PRINT_EVENT;
// or to add a constructor to the class manually.
// I'm choosing to do the latter because the constructor won't change if we change which value is the "last" value in the enum,
// while the javaconstvalue line would need to be updated each time the last changed (and avoiding this is the whole reason for
// having a "last" value in the enum).

//
// Doug's custom Java code for registering/unregistering callbacks
//

// We replace the SWIG generated shutdown with our own version which will call the SWIG generated one.
%rename(ShutdownInternal) sml::Kernel::Shutdown();

// Workaround since SWIG doesn't support %delobj in Java (definition of GetCPtrAndDisown is below)
%typemap(javain) soarxml::ElementXML* pChild "GetCPtrAndDisown($javainput)"

%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("Java_sml_ClientInterface");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      throw e ;
    }
  }

  public final static native long Agent_RegisterForRunEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Agent_RegisterForProductionEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Agent_RegisterForPrintEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6, boolean jarg7);
  public final static native long Agent_RegisterForXMLEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Agent_AddOutputHandler(long jarg1, String jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Agent_RegisterForOutputNotification(long jarg1, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Kernel_RegisterForSystemEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Kernel_RegisterForUpdateEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Kernel_RegisterForStringEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Kernel_RegisterForAgentEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Kernel_AddRhsFunction(long jarg1, String jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native long Kernel_RegisterForClientMessageEvent(long jarg1, String jarg2, Object jarg3, Object jarg4, Object jarg6);

  public final static native boolean Agent_UnregisterForRunEvent(long jarg1, long jarg2);
  public final static native boolean Agent_UnregisterForProductionEvent(long jarg1, long jarg2);
  public final static native boolean Agent_UnregisterForPrintEvent(long jarg1, long jarg2);
  public final static native boolean Agent_UnregisterForXMLEvent(long jarg1, long jarg2);
  public final static native boolean Agent_UnregisterForOutputNotification(long jarg1, long jarg2);
  public final static native boolean Agent_RemoveOutputHandler(long jarg1, long jarg2);
  public final static native boolean Kernel_UnregisterForSystemEvent(long jarg1, long jarg2);
  public final static native boolean Kernel_UnregisterForUpdateEvent(long jarg1, long jarg2);
  public final static native boolean Kernel_UnregisterForStringEvent(long jarg1, long jarg2);
  public final static native boolean Kernel_UnregisterForAgentEvent(long jarg1, long jarg2);
  public final static native boolean Kernel_RemoveRhsFunction(long jarg1, long jarg2);
  public final static native boolean Kernel_UnregisterForClientMessageEvent(long jarg1, long jarg2);
%}

%typemap(javacode) sml::Agent %{
  public interface RunEventInterface {
	public void runEventHandler(int eventID, Object data, Agent agent, int phase) ;
  }

  public interface ProductionEventInterface {
     public void productionEventHandler(int eventID, Object data, Agent agent, String prodName, String instantiation) ;
  }

  public interface PrintEventInterface {
  		public void printEventHandler(int eventID, Object data, Agent agent, String message) ;
  }

  public interface xmlEventInterface {
  		public void xmlEventHandler(int eventID, Object data, Agent agent, ClientXML xml) ;
  }

  public interface OutputEventInterface {
  		public void outputEventHandler(Object data, String agentName, String attributeName, WMElement pWmeAdded) ;
  }

  public interface OutputNotificationInterface {
  		public void outputNotificationHandler(Object data, Agent agent) ;
  }

  public long RegisterForRunEvent(smlRunEventId id, RunEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForRunEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}

  public long RegisterForProductionEvent(smlProductionEventId id, ProductionEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForProductionEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ; }

  public long RegisterForPrintEvent(smlPrintEventId id, PrintEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForPrintEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData, true) ; }

  public long RegisterForPrintEvent(smlPrintEventId id, PrintEventInterface handlerObject, Object callbackData, boolean ignoreOwnEchos)
  { return smlJNI.Agent_RegisterForPrintEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData, ignoreOwnEchos) ; }

  public long RegisterForXMLEvent(smlXMLEventId id, xmlEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForXMLEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ; }

  public long RegisterForOutputNotification(OutputNotificationInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForOutputNotification(swigCPtr, this, handlerObject, callbackData) ;}

  public boolean UnregisterForOutputNotification(long callbackReturnValue)
  { return smlJNI.Agent_UnregisterForOutputNotification(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForRunEvent(long callbackReturnValue)
  { return smlJNI.Agent_UnregisterForRunEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForProductionEvent(long callbackReturnValue)
  { return smlJNI.Agent_UnregisterForProductionEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForPrintEvent(long callbackReturnValue)
  { return smlJNI.Agent_UnregisterForPrintEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForXMLEvent(long callbackReturnValue)
  { return smlJNI.Agent_UnregisterForXMLEvent(swigCPtr, callbackReturnValue) ;}

  public long AddOutputHandler(String attributeName, OutputEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_AddOutputHandler(swigCPtr, attributeName, this, handlerObject, callbackData) ; }

  public boolean RemoveOutputHandler(long callbackReturnValue)
  { return smlJNI.Agent_RemoveOutputHandler(swigCPtr, callbackReturnValue) ;}
%}

%typemap(javacode) sml::Kernel %{
  public interface SystemEventInterface {
     public void systemEventHandler(int eventID, Object data, Kernel kernel) ;
  }

  public interface UpdateEventInterface {
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) ;
  }

  public interface StringEventInterface {
  	public String stringEventHandler(int eventID, Object userData, Kernel kernel, String callbackData) ;
  }

  public interface AgentEventInterface {
  		public void agentEventHandler(int eventID, Object data, String agentName) ;
  }

  public interface RhsFunctionInterface {
  		public String rhsFunctionHandler(int eventID, Object data, String agentName, String functionName, String argument) ;
  }

  public interface ClientMessageInterface {
  		public String clientMessageHandler(int eventID, Object data, String agentName, String functionName, String argument) ;
  }

  public long RegisterForSystemEvent(smlSystemEventId id, SystemEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForSystemEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}

  public boolean UnregisterForSystemEvent(long callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForSystemEvent(swigCPtr, callbackReturnValue) ;}

  public long RegisterForUpdateEvent(smlUpdateEventId id, UpdateEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForUpdateEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}

  public boolean UnregisterForUpdateEvent(long callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForUpdateEvent(swigCPtr, callbackReturnValue) ;}

  public long RegisterForStringEvent(smlStringEventId id, StringEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForStringEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}

  public boolean UnregisterForStringEvent(long callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForStringEvent(swigCPtr, callbackReturnValue) ;}

  public long RegisterForAgentEvent(smlAgentEventId id, AgentEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForAgentEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ; }

  public boolean UnregisterForAgentEvent(long callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForAgentEvent(swigCPtr, callbackReturnValue) ;}

  public long AddRhsFunction(String functionName, RhsFunctionInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_AddRhsFunction(swigCPtr, functionName, this, handlerObject, callbackData) ; }

  public boolean RemoveRhsFunction(long callbackReturnValue)
  { return smlJNI.Kernel_RemoveRhsFunction(swigCPtr, callbackReturnValue) ;}

  public long RegisterForClientMessageEvent(String functionName, ClientMessageInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForClientMessageEvent(swigCPtr, functionName, this, handlerObject, callbackData) ; }

  public boolean UnregisterForClientMessageEvent(long callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForClientMessageEvent(swigCPtr, callbackReturnValue) ;}

  // In Java we want to explicitly delete the C++ kernel object after calling shutdown so that the user
  // doesn't have to call ".delete()" on their Java object (or wait for the garbage collector to do it which may never run--leading to
  // reports of memory leaks on shutdown).  In C++ users expect to have to delete their kernel pointer but not in Java.
  public void Shutdown() {
    smlJNI.Kernel_ShutdownInternal(swigCPtr, this);
    delete() ;
  }

  // Allow a user to avoid deleting the kernel object immediately, if they have some special reason.
  public void ShutdownNoDelete()
  {
    smlJNI.Kernel_ShutdownInternal(swigCPtr, this);
  }
%}

// Some handy alternative method names and additional wrappers to help support some legacy code and may be
// useful for future Java apps.  We don't want to add these to the underlying C++ code because the Exception logic etc.
// would be a real pain to support cross-language.
%typemap(javacode) soarxml::ElementXML %{
   public static final String kClassAttribute   = "Class" ;
   public static final String kVersionAttribute = "Version" ;

  public void addAttribute(String attributeName, String valueName) {
     AddAttribute(attributeName, valueName) ;
  }

	public String getAttributeThrows(String name) throws Exception {
		String value = GetAttribute(name) ;
		if (value == null)
			throw new Exception("Could not find attribute " + name + " while parsing XML document") ;
		return value ;
	}

	public int getAttributeIntThrows(String name) throws Exception {
		String val = getAttributeThrows(name) ;
		int intVal = Integer.parseInt(val) ;
		return intVal ;
	}

	private long GetCPtrAndDisown(ElementXML pChild) {
		pChild.swigCMemOwn = false;
		return ElementXML.getCPtr(pChild);
	}
%}

// Add cleanup code to Shutdown (which is actually renamed to ShutdownInternal)
%exception sml::Kernel::Shutdown {
		$action
		// Release remaining JavaCallbackData's
		std::list<JavaCallbackData*>::iterator itr;
		for(itr=callbackdatas.begin(); itr!=callbackdatas.end(); itr++)
		{
			delete (*itr);
		}
		callbackdatas.clear();
}

//
// End custom Java callback code
//

// include stuff common to all languages (i.e. Java and Tcl)
%include "../sml_ClientInterface.i"

//%newobject sml::Kernel::CreateKernelInCurrentThread;
//%newobject sml::Kernel::CreateKernelInNewThread;
//%newobject sml::Kernel::CreateRemoteConnection;


%{
#include "JavaCallbackByHand.h"
%}
