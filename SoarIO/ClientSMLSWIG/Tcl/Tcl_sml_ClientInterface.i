/* File : Tcl_sml_ClientInterface.i */
%module Tcl_sml_ClientInterface

%typemap(in) Tcl_Obj* {
	$1 = Tcl_DuplicateObj($input);
}

%typemap(in, numinputs=0) Tcl_Interp* {
	$1 = interp;
}

// make sure Tcl_Obj is processed last
// don't bother doing any real type checking because it's not possible for this to be anything else
%typecheck(2000) Tcl_Obj* {
    $1 = 1;
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

	void TclAgentEventCallback(sml::smlAgentEventId id, void* pUserData, sml::Agent* agent)
	{
		// we can ignore the agent parameter because it's already in the script (from when we registered it)
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_EvalObjEx(tud->interp, tud->script, 0);
	}
	
	void TclProductionEventCallback(sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation)
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
	
	void TclRunEventCallback(sml::smlRunEventId id, void* pUserData, sml::Agent* agent, sml::smlPhase phase)
	{
		// we can ignore the agent parameter because it's already in the script (from when we registered it)
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_AppendObjToObj(script, Tcl_NewLongObj(long(phase)));
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_EvalObjEx(tud->interp, script, 0);
	}
	
	void TclPrintEventCallback(sml::smlPrintEventId id, void* pUserData, sml::Agent* agent, char const* pMessage)
	{
		// we can ignore the agent parameter because it's already in the script (from when we registered it)
		// but we still need to append the message (wrapped in quotes in case it has spaces)
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		Tcl_AppendStringsToObj(script, " \"", pMessage, "\"", NULL);
		Tcl_EvalObjEx(tud->interp, script, 0);
	}
	
	void TclSystemEventCallback(sml::smlSystemEventId id, void* pUserData, sml::Kernel* kernel)
	{
		// we can ignore the agent parameter because it's already in the script (from when we registered it)
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_EvalObjEx(tud->interp, tud->script, 0);
	}
	
	TclUserData* CreateTclAgentUserData(sml::Agent* self, Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, Tcl_Interp* interp) {
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
	
	TclUserData* CreateTclSystemUserData(sml::Kernel* self, Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, Tcl_Interp* interp) {
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
	    Tcl_AppendObjToObj(tud->script, SWIG_NewInstanceObj((void *) self, SWIGTYPE_p_sml__Kernel,0));
	    
	    return tud;
	}
%}

%include "../sml_ClientInterface.i"

%extend sml::Agent {
	int RegisterForAgentEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    return self->RegisterForAgentEvent(sml::smlAgentEventId(lid), TclAgentEventCallback, (void*)tud, addToBack);
    };
    
    void UnregisterForAgentEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* callbackID) {
        long lid, lcallbackid;
        Tcl_GetLongFromObj(interp, id, &lid);
        Tcl_GetLongFromObj(interp, callbackID, &lcallbackid);
        self->UnregisterForAgentEvent(sml::smlAgentEventId(lid), lcallbackid);
    }
    
    int RegisterForProductionEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    return self->RegisterForProductionEvent(sml::smlProductionEventId(lid), TclProductionEventCallback, (void*)tud, addToBack);
    }
    
    void UnregisterForProductionEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* callbackID) {
        long lid, lcallbackid;
        Tcl_GetLongFromObj(interp, id, &lid);
        Tcl_GetLongFromObj(interp, callbackID, &lcallbackid);
        self->UnregisterForProductionEvent(sml::smlProductionEventId(lid), lcallbackid);
    }
    
    int RegisterForRunEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, bool addToBack = true) {
        TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    return self->RegisterForRunEvent(sml::smlRunEventId(lid), TclRunEventCallback, (void*)tud, addToBack);
    }
    
    void UnregisterForRunEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* callbackID) {
        long lid, lcallbackid;
        Tcl_GetLongFromObj(interp, id, &lid);
        Tcl_GetLongFromObj(interp, callbackID, &lcallbackid);
        self->UnregisterForRunEvent(sml::smlRunEventId(lid), lcallbackid);
    }

    int RegisterForPrintEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, bool addToBack = true) {	    
	    TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    return self->RegisterForPrintEvent(sml::smlPrintEventId(lid), TclPrintEventCallback, (void*)tud, addToBack);
    }

    void UnregisterForPrintEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* callbackID) {
        long lid, lcallbackid;
        Tcl_GetLongFromObj(interp, id, &lid);
        Tcl_GetLongFromObj(interp, callbackID, &lcallbackid);
        self->UnregisterForPrintEvent(sml::smlPrintEventId(lid), lcallbackid);
    }
}

%extend sml::Kernel {
    int RegisterForSystemEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* proc, Tcl_Obj* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclSystemUserData(self, id, proc, userData, interp);
	    
	    long lid;
	    Tcl_GetLongFromObj(interp, id, &lid);
	    return self->RegisterForSystemEvent(sml::smlSystemEventId(lid), TclSystemEventCallback, (void*)tud, addToBack);
    };
    
    void UnregisterForSystemEvent(Tcl_Interp* interp, Tcl_Obj* id, Tcl_Obj* callbackID) {
        long lid, lcallbackid;
        Tcl_GetLongFromObj(interp, id, &lid);
        Tcl_GetLongFromObj(interp, callbackID, &lcallbackid);
        self->UnregisterForSystemEvent(sml::smlSystemEventId(lid), lcallbackid);
    }
}