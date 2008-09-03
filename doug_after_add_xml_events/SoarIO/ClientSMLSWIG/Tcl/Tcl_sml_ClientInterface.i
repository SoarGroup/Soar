/* File : Tcl_sml_ClientInterface.i */
%module Tcl_sml_ClientInterface

// this includes support for wrapping Tcl_Interp*, which we need for our custom callback code
%include typemaps.i

// We need to include this stuff before we include sml_ClientInterface.i or else things will be in the wrong
//  order in the generated code and it won't compile
// However, this stuff requires some things that are in sml_ClientInterface.i (i.e. the definition of smlEventId and Agent)
// So I am including the header file for smlEventId here and forward declaring Agent
// Even though the header for smlEventId is included later, this isn't harmful
%{
	// helps quell warnings
	#ifndef unused
	#define unused(x) (void)(x)
	#endif
	
	#include <string>
	
	namespace sml {
	class Agent;
	}
	#include "sml_ClientEvents.h" 
	
	struct TclUserData {
		Tcl_Interp* interp;
		Tcl_Obj* script;
	};
	
	std::string TclRhsEventCallback(sml::smlRhsEventId, void* pUserData, sml::Agent* pAgent, char const* pFunctionName,
	                    char const* pArgument)
	{
	    TclUserData* tud = static_cast<TclUserData*>(pUserData);
	    Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
	    Tcl_AppendObjToObj(script, SWIG_Tcl_NewInstanceObj(tud->interp, (void *) pAgent, SWIGTYPE_p_sml__Agent,0));
	    Tcl_AppendStringsToObj(script, " ", pFunctionName, " \"", pArgument, "\"", NULL);
	    Tcl_EvalObjEx(tud->interp, script, 0);
	    Tcl_Obj* res = Tcl_GetObjResult(tud->interp);
	    
	    return Tcl_GetString(res);
	}
	
	void TclAgentEventCallback(sml::smlAgentEventId id, void* pUserData, sml::Agent* agent)
	{
		// we can ignore the id parameter because it's already in the script (from when we registered it)
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
	    Tcl_AppendObjToObj(script, SWIG_Tcl_NewInstanceObj(tud->interp, (void *) agent, SWIGTYPE_p_sml__Agent,0));
		Tcl_EvalObjEx(tud->interp, script, 0);
	}
	
	void TclProductionEventCallback(sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation)
	{
		// we can ignore the agent and id parameters because they're already in the script (from when we registered it)
		unused(pAgent);
		unused(id);
		
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
		// we can ignore the agent and id parameters because they're already in the script (from when we registered it)
		unused(agent);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_AppendObjToObj(script, Tcl_NewLongObj(long(phase)));
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_EvalObjEx(tud->interp, script, 0);
	}
	
	void TclPrintEventCallback(sml::smlPrintEventId id, void* pUserData, sml::Agent* agent, char const* pMessage)
	{
		// we can ignore these parameters because they're already in the script (from when we registered it)
		unused(agent);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		// wrap the message in quotes in case it has spaces
		Tcl_AppendStringsToObj(script, " \"", pMessage, "\"", NULL);
		Tcl_EvalObjEx(tud->interp, script, 0);
	}
	
	void TclSystemEventCallback(sml::smlSystemEventId id, void* pUserData, sml::Kernel* kernel)
	{
		// we can ignore these parameters because they're already in the script (from when we registered it)
		unused(kernel);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		Tcl_EvalObjEx(tud->interp, tud->script, 0);
	}
	
	TclUserData* CreateTclAgentUserData(sml::Agent* self, int id, char* proc, char* userData, Tcl_Interp* interp) {
		TclUserData* tud = new TclUserData();
	    
	    tud->interp = interp;
	    // put all of the arguments together so we can just execute this as a single script later
	    // put spaces between the arguments and wrap the userdata in quotes (in case it has spaces)
	    tud->script = Tcl_NewObj();
	    Tcl_AppendStringsToObj(tud->script, proc, " ", NULL);
	    Tcl_AppendObjToObj(tud->script, Tcl_NewLongObj(id));
	    Tcl_AppendStringsToObj(tud->script, " \"", userData, "\" ", NULL);
	    Tcl_AppendObjToObj(tud->script, SWIG_NewInstanceObj((void *) self, SWIGTYPE_p_sml__Agent,0));
	    
	    return tud;
	}
	
	TclUserData* CreateTclSystemUserData(sml::Kernel* self, int id, const char* proc, const char* userData, Tcl_Interp* interp) {
		TclUserData* tud = new TclUserData();
	    
	    tud->interp = interp;
	    // put all of the arguments together so we can just execute this as a single script later
	    // put spaces between the arguments and wrap the userdata in quotes (in case it has spaces)
	    tud->script = Tcl_NewObj();
	    Tcl_AppendStringsToObj(tud->script, proc, " ", NULL);
	    Tcl_AppendObjToObj(tud->script, Tcl_NewLongObj(id));
	    Tcl_AppendStringsToObj(tud->script, " \"", userData, "\" ", NULL);
	    Tcl_AppendObjToObj(tud->script, SWIG_NewInstanceObj((void *) self, SWIGTYPE_p_sml__Kernel,0));
	    
	    return tud;
	}
	
	TclUserData* CreateTclUserData(int id, const char* proc, const char* userData, Tcl_Interp* interp) {
		TclUserData* tud = new TclUserData();
	    
	    tud->interp = interp;
	    // put all of the arguments together so we can just execute this as a single script later
	    // put spaces between the arguments and wrap the userdata in quotes (in case it has spaces)
	    tud->script = Tcl_NewObj();
	    Tcl_AppendStringsToObj(tud->script, proc, " ", NULL);
	    Tcl_AppendObjToObj(tud->script, Tcl_NewLongObj(id));
	    Tcl_AppendStringsToObj(tud->script, " \"", userData, "\" ", NULL);
	    
	    return tud;
	}
%}

%include "../sml_ClientInterface.i"

%extend sml::Agent {

    int RegisterForProductionEvent(Tcl_Interp* interp, sml::smlProductionEventId id, char* proc, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    return self->RegisterForProductionEvent(id, TclProductionEventCallback, (void*)tud, addToBack);
    }
  
    int RegisterForRunEvent(Tcl_Interp* interp, sml::smlRunEventId id, char* proc, char* userData, bool addToBack = true) {
        TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    return self->RegisterForRunEvent(id, TclRunEventCallback, (void*)tud, addToBack);
    }

    int RegisterForPrintEvent(Tcl_Interp* interp, sml::smlPrintEventId id, char* proc, char* userData, bool addToBack = true) {	    
	    TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    return self->RegisterForPrintEvent(id, TclPrintEventCallback, (void*)tud, addToBack);
    }
}

%extend sml::Kernel {

    int RegisterForSystemEvent(Tcl_Interp* interp, sml::smlSystemEventId id, char* proc, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclSystemUserData(self, id, proc, userData, interp);
	    return self->RegisterForSystemEvent(id, TclSystemEventCallback, (void*)tud, addToBack);
    };
    
    int RegisterForAgentEvent(Tcl_Interp* interp, sml::smlAgentEventId id, char* proc, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclUserData(id, proc, userData, interp);
	    return self->RegisterForAgentEvent(id, TclAgentEventCallback, (void*)tud, addToBack);
    };
    
    int AddRhsFunction(Tcl_Interp* interp, char const* pRhsFunctionName, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclUserData(sml::smlEVENT_RHS_USER_FUNCTION, pRhsFunctionName, userData, interp);
	    return self->AddRhsFunction(pRhsFunctionName, TclRhsEventCallback, (void*)tud, addToBack);
    };
}