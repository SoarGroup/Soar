/* File : Tcl_sml_ClientInterface.i */
%module Tcl_sml_ClientInterface

%include "../sml_ClientInterface.i"
/*
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
        
	int TclRegisterForAgentEvent(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[]) {
	    
	    TclUserData* tud = new TclUserData();
	    tud->interp = interp;
	    tud->script = objv[3];
	    
	    int eventId;
	    
	    if (Tcl_GetIntFromObj(interp, objv[2], &eventId) == TCL_ERROR) {
            return TCL_ERROR;
        }
        
        sml::Agent *agent = (sml::Agent *) 0 ;
	    if ((SWIG_ConvertPtr(objv[1], (void **) &agent, SWIGTYPE_p_sml__Agent,SWIG_POINTER_EXCEPTION | 0) != TCL_OK)) SWIG_fail;
	    
	    //create script
	    //form is: procname AgentPtr smlEventId userdata
	    Tcl_AppendStringsToObj(tud->script, " ", (char *)NULL);
	    Tcl_AppendObjToObj(objv[1]);
	    Tcl_AppendStringsToObj(tud->script, " ", (char *)NULL);
	    Tcl_AppendObjToObj(tud->script, objv[2]);
	    Tcl_AppendStringsToObj(tud->script, " ", (char *)NULL);
	    Tcl_AppendObjToObj(tud->script, objv[4]);
	    
	    (agent)->RegisterForAgentEvent((sml::smlEventId )eventId, TclAgentEventCallBack, tud);
	};
}
*/