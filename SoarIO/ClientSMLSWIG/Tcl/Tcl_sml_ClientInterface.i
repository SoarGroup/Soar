/* File : Tcl_sml_ClientInterface.i */
%module Tcl_sml_ClientInterface

%typemap(in) Tcl_Obj* {
	$1 = Tcl_DuplicateObj($input);
}

%typemap(in) Tcl_Interp* {
	$1 = interp;
}

// We need to include this stuff before we include sml_ClientInterface.i or else things will be in the wrong
//  order in the generated code and it won't compile
// However, this stuff requires some things that are in sml_ClientInterface.i (i.e. the definition of smlEventId and Agent)
// So I am including the header file for smlEventId here and forward declaring Agent
// Even though the header for smlEventId is included later, this isn't harmful
%{
	namespace sml {
	class Agent;
	}
	#include "sml_ClientEvents.h" 
	
	struct TclUserData {
		Tcl_Interp* interp;
		Tcl_Obj* script;
	};

	void TclAgentEventCallBack(sml::smlAgentEventId id, void* pUserData, sml::Agent* agent)
	{
		// we can ignore the agent parameter because it's already in the script (from when we registered it)
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_EvalObjEx(tud->interp, tud->script, 0);
	}
	
	void TclProductionEventCallBack(sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation)
	{
		// we can ignore the agent parameter because it's already in the script (from when we registered it)
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		Tcl_AppendStringsToObj(script, " \"", pProdName, "\"", NULL);
		
		// we need to check if pInstantiation is null, because if it is it will prematurely end the appending
		//  and we will lose the closing double quote, resulting in a syntax error
		if(pInstantiation) {
			Tcl_AppendStringsToObj(script, " \"", pInstantiation, "\"", NULL);
		} else {
			Tcl_AppendStringsToObj(script, " \"\"", NULL);
		}
		
		Tcl_EvalObjEx(tud->interp, script, 0);
	}
	
	void TclRunEventCallBack(sml::smlRunEventId id, void* pUserData, sml::Agent* agent, sml::smlPhase phase)
	{
		// we can ignore the agent parameter because it's already in the script (from when we registered it)
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_AppendObjToObj(script, Tcl_NewLongObj(long(phase)));
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_EvalObjEx(tud->interp, script, 0);
	}
	
	void TclPrintEventCallBack(sml::smlPrintEventId id, void* pUserData, sml::Agent* agent, char const* pMessage)
	{
		// we can ignore the agent parameter because it's already in the script (from when we registered it)
		// but we still need to append the message (wrapped in quotes in case it has spaces)
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		Tcl_AppendStringsToObj(script, " \"", pMessage, "\"", NULL);
		Tcl_EvalObjEx(tud->interp, script, 0);
	}
	
	TclUserData* CreateTclUserData(sml::Agent* self, Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, Tcl_Interp* interp) {
		TclUserData* tud = new TclUserData();
	    
	    tud->interp = interp;
	    // put all of the arguments together so we can just execute this as a single script later
	    // put spaces between the arguments and wrap the userdata in quotes (in case it has spaces)
	    tud->script = proc;
	    Tcl_AppendStringsToObj(tud->script, " ", NULL);
	    Tcl_AppendObjToObj(tud->script, id);
	    Tcl_AppendStringsToObj(tud->script, " \"", NULL);
	    Tcl_AppendObjToObj(tud->script, userData);
	    Tcl_AppendStringsToObj(tud->script, "\" ", NULL);
	    Tcl_AppendObjToObj(tud->script, SWIG_NewInstanceObj((void *) self, SWIGTYPE_p_sml__Agent,0));
	    
	    return tud;
	}
%}

%include "../sml_ClientInterface.i"

%extend sml::Agent {
	void RegisterForAgentEvent(Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, Tcl_Interp* interp) {
	    TclUserData* tud = CreateTclUserData(self, id, proc, userData, interp);
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    self->RegisterForAgentEvent(sml::smlAgentEventId(lid), TclAgentEventCallBack, (void*)tud);
    };
    
    void UnregisterForAgentEvent(Tcl_Obj* id, Tcl_Obj* callbackID, Tcl_Interp* interp) {
    
    }
    
    void RegisterForProductionEvent(Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, Tcl_Interp* interp) {
	    TclUserData* tud = CreateTclUserData(self, id, proc, userData, interp);
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    self->RegisterForProductionEvent(sml::smlProductionEventId(lid), TclProductionEventCallBack, (void*)tud);
    }
    
    void UnregisterForProductionEvent(Tcl_Obj* id, Tcl_Obj* callbackID, Tcl_Interp* interp) {
    
    }
    
    void RegisterForRunEvent(Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, Tcl_Interp* interp) {
        TclUserData* tud = CreateTclUserData(self, id, proc, userData, interp);
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    self->RegisterForRunEvent(sml::smlRunEventId(lid), TclRunEventCallBack, (void*)tud);
    }
    
    void UnregisterForRunEvent(Tcl_Obj* id, Tcl_Obj* callbackID, Tcl_Interp* interp) {
    
    }

    void RegisterForPrintEvent(Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, Tcl_Interp* interp) {	    
	    TclUserData* tud = CreateTclUserData(self, id, proc, userData, interp);
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    self->RegisterForPrintEvent(sml::smlPrintEventId(lid), TclPrintEventCallBack, (void*)tud);
    }

    void UnregisterForPrintEvent(Tcl_Obj* id, Tcl_Obj* callbackID, Tcl_Interp* interp) {
    
    }
    
}