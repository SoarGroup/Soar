/* File : sml_ClientInterface.i */
%module sml

//%javaconst(1); // strongly recommended by SWIG manual section 19.3.5.1

//
// Doug's custom Java code for registering/unregistering callbacks
//

%pragma(java) jniclasscode=%{
  public final static native int Agent_RegisterForRunEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForAgentEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForProductionEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_RegisterForSystemEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);

  public final static native void Agent_UnregisterForRunEvent(long jarg1, int jarg2, int jarg3);
  public final static native void Agent_UnregisterForAgentEvent(long jarg1, int jarg2, int jarg3);
  public final static native void Agent_UnregisterForProductionEvent(long jarg1, int jarg2, int jarg3);
  public final static native void Kernel_UnregisterForSystemEvent(long jarg1, int jarg2, int jarg3);
%}

%typemap(javacode) sml::Agent %{
  public int RegisterForRunEvent(Agent agent, smlRunEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForRunEvent(swigCPtr, id.swigValue(), agent, handlerObject, handlerMethod, callbackData) ;}

  public int RegisterForAgentEvent(Agent agent, smlAgentEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForAgentEvent(swigCPtr, id.swigValue(), agent, handlerObject, handlerMethod, callbackData) ; }

  public int RegisterForProductionEvent(Agent agent, smlProductionEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForProductionEvent(swigCPtr, id.swigValue(), agent, handlerObject, handlerMethod, callbackData) ; }

  public void UnregisterForRunEvent(smlRunEventId id, int callbackReturnValue)
  { smlJNI.Agent_UnregisterForRunEvent(swigCPtr, id.swigValue(), callbackReturnValue) ;}

  public void UnregisterForAgentEvent(smlAgentEventId id, int callbackReturnValue)
  { smlJNI.Agent_UnregisterForAgentEvent(swigCPtr, id.swigValue(), callbackReturnValue) ;}

  public void UnregisterForProductionEvent(smlProductionEventId id, int callbackReturnValue)
  { smlJNI.Agent_UnregisterForProductionEvent(swigCPtr, id.swigValue(), callbackReturnValue) ;}
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

