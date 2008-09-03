/* File : sml_ClientInterface.i */
%module sml

// uncomment the following line if you're using Java 1.5/5.0 (or later) and want
//	proper Java enums to be generated for the SML enums
//%include "enums.swg"

%javaconst(1); // strongly recommended by SWIG manual section 19.3.5.1
// the previous line causes problems for some enum values, so we have to set them manually here
// the problem only affects those enums whose values are "calculated" based on other values
%javaconstvalue("smlSystemEventId.smlEVENT_AFTER_RHS_FUNCTION_EXECUTED.swigValue() + 1") smlEVENT_BEFORE_SMALLEST_STEP;
%javaconstvalue("smlProductionEventId.smlEVENT_BEFORE_PRODUCTION_RETRACTED.swigValue() + 1") smlEVENT_AFTER_AGENT_CREATED;
%javaconstvalue("smlPrintEventId.smlEVENT_PRINT.swigValue() + 1") smlEVENT_RHS_USER_FUNCTION;
%javaconstvalue("smlRhsEventId.smlEVENT_RHS_USER_FUNCTION.swigValue() + 1") smlEVENT_XML_TRACE_OUTPUT;
%javaconstvalue("smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT.swigValue() + 1") smlEVENT_AFTER_ALL_OUTPUT_PHASES;
%javaconstvalue("smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT.swigValue() + 1") smlEVENT_LAST;
%javaconstvalue("smlWorkingMemoryEventId.smlEVENT_OUTPUT_PHASE_CALLBACK.swigValue() + 1") smlEVENT_LOG_ERROR;
%javaconstvalue("smlRunEventId.smlEVENT_AFTER_RUNNING.swigValue() + 1") smlEVENT_AFTER_PRODUCTION_ADDED;
%javaconstvalue("smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue() + 1") smlEVENT_OUTPUT_PHASE_CALLBACK;

//
// Doug's custom Java code for registering/unregistering callbacks
//

// in the Java case, we will also provide custom code for unregistering from events
%ignore sml::Agent::UnregisterForRunEvent(int);
%ignore sml::Agent::UnregisterForProductionEvent(int);
%ignore sml::Agent::UnregisterForPrintEvent(int);
%ignore sml::Agent::UnregisterForXMLEvent(int);
%ignore sml::Kernel::UnregisterForSystemEvent(int);
%ignore sml::Kernel::UnregisterForUpdateEvent(int);
%ignore sml::Kernel::UnregisterForAgentEvent(int);
%ignore sml::Kernel::RemoveRhsFunction(int);

%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("Java_sml_ClientInterface");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      throw e ;
    }
  }
  
  public final static native int Agent_RegisterForRunEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForProductionEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForPrintEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForXMLEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_RegisterForSystemEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_RegisterForUpdateEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_RegisterForAgentEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_AddRhsFunction(long jarg1, String jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);

  public final static native boolean Agent_UnregisterForRunEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForProductionEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForPrintEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForXMLEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForSystemEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForUpdateEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForAgentEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_RemoveRhsFunction(long jarg1, int jarg2);
%}

%typemap(javacode) sml::Agent %{
  public int RegisterForRunEvent(smlRunEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForRunEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ;}
  
  public int RegisterForProductionEvent(smlProductionEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForProductionEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ; }

  public int RegisterForPrintEvent(smlPrintEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForPrintEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ; }

  public int RegisterForXMLEvent(smlXMLEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForXMLEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ; }
  
  public boolean UnregisterForRunEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForRunEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForProductionEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForProductionEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForPrintEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForPrintEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForXMLEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForXMLEvent(swigCPtr, callbackReturnValue) ;}
%}

%typemap(javacode) sml::Kernel %{
  public int RegisterForSystemEvent(smlSystemEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Kernel_RegisterForSystemEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ;}
 
  public boolean UnregisterForSystemEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForSystemEvent(swigCPtr, callbackReturnValue) ;}

  public int RegisterForUpdateEvent(smlUpdateEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Kernel_RegisterForUpdateEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ;}
 
  public boolean UnregisterForUpdateEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForUpdateEvent(swigCPtr, callbackReturnValue) ;}
  
  public int RegisterForAgentEvent(smlAgentEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Kernel_RegisterForAgentEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ; }

  public boolean UnregisterForAgentEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForAgentEvent(swigCPtr, callbackReturnValue) ;}

  public int AddRhsFunction(String functionName, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Kernel_AddRhsFunction(swigCPtr, functionName, this, handlerObject, handlerMethod, callbackData) ; }

  public boolean RemoveRhsFunction(int callbackReturnValue)
  { return smlJNI.Kernel_RemoveRhsFunction(swigCPtr, callbackReturnValue) ;}

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

