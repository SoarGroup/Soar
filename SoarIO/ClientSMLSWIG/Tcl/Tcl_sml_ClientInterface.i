/* File : Tcl_sml_ClientInterface.i */
%module Tcl_sml_ClientInterface

%typemap(in) Tcl_Obj* {
	$1 = Tcl_DuplicateObj($input);
}

%typemap(in) Tcl_Interp* {
	$1 = interp;
}

%{
	namespace sml {
	class Agent;
	}
	#include "sml_ClientEvents.h" 
	
	struct TclUserData {
		Tcl_Interp* interp;
		Tcl_Obj* script;
	};

	void TclAgentEventCallBack(sml::smlEventId id, void* pUserData, sml::Agent* agent)
	{
		// we can ignore the agent parameter because it's already in the script (from when we registered it)
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_EvalObjEx(tud->interp, tud->script, 0);
	}
%}

%include "../sml_ClientInterface.i"

%extend sml::Agent {
	void RegisterForAgentEvent(Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, Tcl_Interp* interp) {
		TclUserData* tud = new TclUserData();
	    
	    tud->interp = interp;
	    // put all of the arguments together so we can just execute this as a single script later
	    // put spaces between the arguments (I'm not entirely sure this is required)
	    tud->script = proc;
	    Tcl_AppendStringsToObj(tud->script, " ", NULL);
	    Tcl_AppendObjToObj(tud->script, id);
	    Tcl_AppendStringsToObj(tud->script, " ", NULL);
	    Tcl_AppendObjToObj(tud->script, userData);
	    Tcl_AppendStringsToObj(tud->script, " ", NULL);
	    Tcl_AppendObjToObj(tud->script, SWIG_NewInstanceObj((void *) self, SWIGTYPE_p_sml__Agent,0));
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    self->RegisterForAgentEvent(sml::smlEventId(lid), TclAgentEventCallBack, (void*)tud);
    };
}