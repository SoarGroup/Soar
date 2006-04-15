/* File : sml_ClientInterface.i */
%module sml

// uncomment the following line if you're using Java 1.5/5.0 (or later) and want
//	proper Java enums to be generated for the SML enums
//%include "enums.swg"

%javaconst(1); // strongly recommended by SWIG manual section 19.3.5.1
// the previous line causes problems for some enum values, so we have to set them manually here
// the problem only affects those enums whose values are "calculated" based on other values
%javaconstvalue("smlSystemEventId.smlEVENT_LAST_SYSTEM_EVENT.swigValue() + 1") smlEVENT_BEFORE_SMALLEST_STEP;
%javaconstvalue("smlProductionEventId.smlEVENT_LAST_PRODUCTION_EVENT.swigValue() + 1") smlEVENT_AFTER_AGENT_CREATED;
%javaconstvalue("smlPrintEventId.smlEVENT_LAST_PRINT_EVENT.swigValue() + 1") smlEVENT_RHS_USER_FUNCTION;
%javaconstvalue("smlRhsEventId.smlEVENT_LAST_RHS_EVENT.swigValue() + 1") smlEVENT_XML_TRACE_OUTPUT;
%javaconstvalue("smlXMLEventId.smlEVENT_LAST_XML_EVENT.swigValue() + 1") smlEVENT_AFTER_ALL_OUTPUT_PHASES;
%javaconstvalue("smlUpdateEventId.smlEVENT_LAST_UPDATE_EVENT.swigValue() + 1") smlEVENT_EDIT_PRODUCTION;
%javaconstvalue("smlStringEventId.smlEVENT_LAST_STRING_EVENT.swigValue() + 1") smlEVENT_LAST;
%javaconstvalue("smlWorkingMemoryEventId.smlEVENT_LAST_WM_EVENT.swigValue() + 1") smlEVENT_LOG_ERROR;
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

// Add the constructors that SWIG misses
// We do this by adding the constructor to one class and then copying the typemaps for that class to other classes
// This could actually do too much since ALL typemaps are copied, not just the one we added, but it doesn't seem to cause any problems so far
// SWIG should generate this constructor starting in 1.3.28, so at some point in the future we should be able to drop this code
// (e.g. when SWIG 1.3.28 is available on Gentoo Linux)

#if SWIG_VERSION < 0x010328
%typemap(javacode) sml::smlPrintEventId %{
   private $javaclassname(String swigName, $javaclassname enumValue) {
      this.swigName = swigName ; this.swigValue = enumValue.swigValue ; swigNext = this.swigValue+1 ; } %}

%apply sml::smlPrintEventId {	sml::smlSystemEventId,
								sml::smlProductionEventId,
								sml::smlRunEventId,
								sml::smlAgentEventId,
								sml::smlWorkingMemoryEventId,
								sml::smlRhsEventId,
								sml::smlXMLEventId,
								sml::smlUpdateEventId,
								sml::smlStringEventId }
#endif

//
// Doug's custom Java code for registering/unregistering callbacks
//

// in the Java case, we will also provide custom code for unregistering from events
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

// We replace the SWIG generated shutdown with our own version which will call the SWIG generated one.
%rename(ShutdownInternal) sml::Kernel::Shutdown();

%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("Java_sml_ClientInterface");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      throw e ;
    }
  }
  
  public final static native int Agent_RegisterForRunEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Agent_RegisterForProductionEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Agent_RegisterForPrintEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6, boolean jarg7);
  public final static native int Agent_RegisterForXMLEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Agent_AddOutputHandler(long jarg1, String jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Agent_RegisterForOutputNotification(long jarg1, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_RegisterForSystemEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_RegisterForUpdateEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_RegisterForStringEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_RegisterForAgentEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_AddRhsFunction(long jarg1, String jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_RegisterForClientMessageEvent(long jarg1, String jarg2, Object jarg3, Object jarg4, Object jarg6);

  public final static native boolean Agent_UnregisterForRunEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForProductionEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForPrintEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForXMLEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForOutputNotification(long jarg1, int jarg2);
  public final static native boolean Agent_RemoveOutputHandler(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForSystemEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForUpdateEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForStringEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForAgentEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_RemoveRhsFunction(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForClientMessageEvent(long jarg1, int jarg2);
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

  public int RegisterForRunEvent(smlRunEventId id, RunEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForRunEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}
  
  public int RegisterForProductionEvent(smlProductionEventId id, ProductionEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForProductionEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ; }

  public int RegisterForPrintEvent(smlPrintEventId id, PrintEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForPrintEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData, true) ; }

  public int RegisterForPrintEvent(smlPrintEventId id, PrintEventInterface handlerObject, Object callbackData, boolean ignoreOwnEchos)
  { return smlJNI.Agent_RegisterForPrintEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData, ignoreOwnEchos) ; }

  public int RegisterForXMLEvent(smlXMLEventId id, xmlEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForXMLEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ; }
  
  public int RegisterForOutputNotification(OutputNotificationInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForOutputNotification(swigCPtr, this, handlerObject, callbackData) ;}

  public boolean UnregisterForOutputNotification(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForOutputNotification(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForRunEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForRunEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForProductionEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForProductionEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForPrintEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForPrintEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForXMLEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForXMLEvent(swigCPtr, callbackReturnValue) ;}

  public int AddOutputHandler(String attributeName, OutputEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_AddOutputHandler(swigCPtr, attributeName, this, handlerObject, callbackData) ; }

  public boolean RemoveOutputHandler(int callbackReturnValue)
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
  	public void stringEventHandler(int eventID, Object userData, Kernel kernel, String callbackData) ;
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
  
  public int RegisterForSystemEvent(smlSystemEventId id, SystemEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForSystemEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}
 
  public boolean UnregisterForSystemEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForSystemEvent(swigCPtr, callbackReturnValue) ;}

  public int RegisterForUpdateEvent(smlUpdateEventId id, UpdateEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForUpdateEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}

  public boolean UnregisterForUpdateEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForUpdateEvent(swigCPtr, callbackReturnValue) ;}

  public int RegisterForStringEvent(smlStringEventId id, StringEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForStringEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}
 
  public boolean UnregisterForStringEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForStringEvent(swigCPtr, callbackReturnValue) ;}
  
  public int RegisterForAgentEvent(smlAgentEventId id, AgentEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForAgentEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ; }

  public boolean UnregisterForAgentEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForAgentEvent(swigCPtr, callbackReturnValue) ;}

  public int AddRhsFunction(String functionName, RhsFunctionInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_AddRhsFunction(swigCPtr, functionName, this, handlerObject, callbackData) ; }

  public boolean RemoveRhsFunction(int callbackReturnValue)
  { return smlJNI.Kernel_RemoveRhsFunction(swigCPtr, callbackReturnValue) ;}

  public int RegisterForClientMessageEvent(String functionName, ClientMessageInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForClientMessageEvent(swigCPtr, functionName, this, handlerObject, callbackData) ; }

  public boolean UnregisterForClientMessageEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForClientMessageEvent(swigCPtr, callbackReturnValue) ;}

  // In Java we want to explicitly delete the C++ kernel object after calling shutdown so that the user
  // doesn't have to call ".delete()" on their Java object (or wait for the garbage collector to do it which may never run--leading to
  // reports of memory leaks on shutdown).  In C++ users expect to have to delete their kernel pointer but not in Java.
  public void Shutdown() {
    smlJNI.Kernel_ShutdownInternal(swigCPtr);
    delete() ;
  }
  
  // Allow a user to avoid deleting the kernel object immediately, if they have some special reason.
  public void ShutdownNoDelete()
  {
    smlJNI.Kernel_ShutdownInternal(swigCPtr);
  }
%}

// Some handy alternative method names and additional wrappers to help support some legacy code and may be
// useful for future Java apps.  We don't want to add these to the underlying C++ code because the Exception logic etc.
// would be a real pain to support cross-language.
%typemap(javacode) sml::ElementXML %{
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
%}

//
// End custom Java callback code
//

// include stuff common to all languages (i.e. Java and Tcl)
%include "../sml_ClientInterface.i"

// include Doug's custom JNI code for callbacks in the wrapper section
//  so it's in the extern C block
%wrapper %{
#include "JavaCallbackByHand.h"
%}

