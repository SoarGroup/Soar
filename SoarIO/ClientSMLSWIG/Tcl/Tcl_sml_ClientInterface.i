/* File : Tcl_sml_ClientInterface.i */
%module Tcl_sml_ClientInterface

%typemap(in) Tcl_Obj* {
	$1 = Tcl_DuplicateObj($input);
}

%typemap(in) Tcl_Interp* {
	$1 = interp;
}

%include "../sml_ClientInterface.i"

%{
struct TclUserData {
	Tcl_Interp* interp;
	Tcl_Obj* script;
};

// This function matches the prototype of the normal C callback
//   function for our widget. However, we use the pUserData pointer
//   for holding a reference to a Tcl callable object. 

void TclAgentEventCallBack(sml::smlEventId id, void* pUserData, sml::Agent* pAgent)
{
	TclUserData* tud = static_cast<TclUserData*>(pUserData);
	int errorCode = Tcl_EvalObjEx(tud->interp, tud->script, 0);
	
	if(errorCode != TCL_OK) {
	  // not sure what we can do
	}
}
%}

// Attach a new method to Agentfor adding Tcl callback functions
%extend sml::Agent {
    // objv[0] : ???
    // objv[1] : Agent pointer
    // objv[2] : smlEventId (int)
    // obvj[3] : Tcl proc (string)
    // obvj[4] : user data (string?)
    
    void TclRegisterForAgentEvent(Tcl_Interp* interp, sml::smlEventId eventId, Tcl_Obj* proc, Tcl_Obj* userData) {
		TclUserData* tud = new TclUserData();
	    
	    tud->interp = interp;
	    
	    //create script
	    //form is: procname AgentPtr smlEventId userdata
	    tud->script = proc;
	    /*
	    Tcl_AppendStringsToObj(tud->script, " ", (char *)NULL);
	    Tcl_AppendObjToObj(self);
	    Tcl_AppendStringsToObj(tud->script, " ", (char *)NULL);
	    Tcl_AppendObjToObj(tud->script, proc);
	    Tcl_AppendStringsToObj(tud->script, " ", (char *)NULL);
	    Tcl_AppendObjToObj(tud->script, userData);
	    */
	    self->RegisterForAgentEvent((sml::smlEventId )eventId, TclAgentEventCallBack, tud);
    };
}