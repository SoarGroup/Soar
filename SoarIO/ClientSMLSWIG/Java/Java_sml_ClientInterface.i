/* File : sml_ClientInterface.i */
%module sml

%javaconst(1); // strongly recommended by SWIG manual section 19.3.5.1
// the previous line causes problems for some enum values, so we have to set them manually here
// the problem only affects those enums whose values are calculated (as opposed to being constant)
// hopefully this problem will go away in the future if/when we switch to Java 1.5, which has proper enum support
%javaconstvalue("smlSystemEventId.smlEVENT_AFTER_RHS_FUNCTION_EXECUTED.swigValue() + 1") smlEVENT_BEFORE_SMALLEST_STEP;
%javaconstvalue("smlProductionEventId.smlEVENT_BEFORE_PRODUCTION_RETRACTED.swigValue() + 1") smlEVENT_AFTER_AGENT_CREATED;
%javaconstvalue("smlPrintEventId.smlEVENT_PRINT.swigValue() + 1") smlEVENT_LAST;
%javaconstvalue("smlWorkingMemoryEventId.smlEVENT_OUTPUT_PHASE_CALLBACK.swigValue() + 1") smlEVENT_LOG_ERROR;
%javaconstvalue("smlRunEventId.smlEVENT_AFTER_RUNNING.swigValue() + 1") smlEVENT_AFTER_PRODUCTION_ADDED;
%javaconstvalue("smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue() + 1") smlEVENT_OUTPUT_PHASE_CALLBACK;

//
// Doug's custom Java code for registering/unregistering callbacks
//

// in the Java case, we will also provide custom code for unregistering from events
%ignore sml::Agent::UnregisterForRunEvent(smlRunEventId, int);
%ignore sml::Agent::UnregisterForAgentEvent(smlAgentEventId, int);
%ignore sml::Agent::UnregisterForProductionEvent(smlProductionEventId, int);
%ignore sml::Agent::UnregisterForPrintEvent(smlPrintEventId, int);
%ignore sml::Kernel::UnregisterForSystemEvent(smlSystemEventId, int);

%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("Java_sml_ClientInterface");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      System.exit(1);
    }
  }
  
  public final static native int Agent_RegisterForRunEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForAgentEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForProductionEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForPrintEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_RegisterForSystemEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);

  public final static native void Agent_UnregisterForRunEvent(long jarg1, int jarg2, int jarg3);
  public final static native void Agent_UnregisterForAgentEvent(long jarg1, int jarg2, int jarg3);
  public final static native void Agent_UnregisterForProductionEvent(long jarg1, int jarg2, int jarg3);
  public final static native void Agent_UnregisterForPrintEvent(long jarg1, int jarg2, int jarg3);
  public final static native void Kernel_UnregisterForSystemEvent(long jarg1, int jarg2, int jarg3);
%}

%typemap(javacode) sml::Agent %{
  public int RegisterForRunEvent(Agent agent, smlRunEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForRunEvent(swigCPtr, id.swigValue(), agent, handlerObject, handlerMethod, callbackData) ;}
  
  public int RegisterForAgentEvent(Agent agent, smlAgentEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForAgentEvent(swigCPtr, id.swigValue(), agent, handlerObject, handlerMethod, callbackData) ; }

  public int RegisterForProductionEvent(Agent agent, smlProductionEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForProductionEvent(swigCPtr, id.swigValue(), agent, handlerObject, handlerMethod, callbackData) ; }

  public int RegisterForPrintEvent(Agent agent, smlPrintEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForPrintEvent(swigCPtr, id.swigValue(), agent, handlerObject, handlerMethod, callbackData) ; }
  
  public void UnregisterForRunEvent(smlRunEventId id, int callbackReturnValue)
  { smlJNI.Agent_UnregisterForRunEvent(swigCPtr, id.swigValue(), callbackReturnValue) ;}

  public void UnregisterForAgentEvent(smlAgentEventId id, int callbackReturnValue)
  { smlJNI.Agent_UnregisterForAgentEvent(swigCPtr, id.swigValue(), callbackReturnValue) ;}

  public void UnregisterForProductionEvent(smlProductionEventId id, int callbackReturnValue)
  { smlJNI.Agent_UnregisterForProductionEvent(swigCPtr, id.swigValue(), callbackReturnValue) ;}

  public void UnregisterForPrintEvent(smlPrintEventId id, int callbackReturnValue)
  { smlJNI.Agent_UnregisterForPrintEvent(swigCPtr, id.swigValue(), callbackReturnValue) ;}
%}

%typemap(javacode) sml::Kernel %{
  public int RegisterForSystemEvent(Kernel kernel, smlSystemEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Kernel_RegisterForSystemEvent(swigCPtr, id.swigValue(), kernel, handlerObject, handlerMethod, callbackData) ;}
  
  public void UnregisterForSystemEvent(smlSystemEventId id, int callbackReturnValue)
  { smlJNI.Kernel_UnregisterForSystemEvent(swigCPtr, id.swigValue(), callbackReturnValue) ;}
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

