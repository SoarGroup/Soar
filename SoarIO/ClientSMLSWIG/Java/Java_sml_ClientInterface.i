/* File : sml_ClientInterface.i */
%module sml

%javaconst(1); // strongly recommended by SWIG manual section 19.3.5.1

%include "../sml_ClientInterface.i"

%{
#include "JavaCallbackByHand.h"
%}

%pragma(java) jniclasscode=%{
  public final static native int Agent_RegisterForRunEventByHand(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForAgentEventByHand(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForProductionEventByHand(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_RegisterForSystemEventByHand(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);

  public final static native void Agent_UnregisterForRunEventByHand(long jarg1, int jarg2, int jarg3);
  public final static native void Agent_UnregisterForAgentEventByHand(long jarg1, int jarg2, int jarg3);
  public final static native void Agent_UnregisterForProductionEventByHand(long jarg1, int jarg2, int jarg3);
  public final static native void Kernel_UnregisterForSystemEventByHand(long jarg1, int jarg2, int jarg3);
%}