/* File : Tcl_sml_ClientInterface.i */
%module Tcl_sml_ClientInterface

// this includes support for wrapping Tcl_Interp*, which we need for our custom callback code
%include typemaps.i

%ignore sml::Agent::UnregisterForRunEvent(int);
%ignore sml::Agent::UnregisterForProductionEvent(int);
%ignore sml::Agent::UnregisterForPrintEvent(int);
%ignore sml::Agent::UnregisterForXMLEvent(int);
%ignore sml::Agent::UnregisterForOutputNotification(int);
%ignore sml::Agent::RemoveOutputHandler(int);
%ignore sml::Kernel::UnregisterForSystemEvent(int);
%ignore sml::Kernel::UnregisterForUpdateEvent(int);
%ignore sml::Kernel::UnregisterForStringEvent(int);
%ignore sml::Kernel::UnregisterForAgentEvent(int);
%ignore sml::Kernel::RemoveRhsFunction(int);
%ignore sml::Kernel::UnregisterForClientMessageEvent(int);

%ignore sml::Kernel::Shutdown();


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
	#include <list>
	#include <algorithm>
	
	namespace sml {
	class Agent;
	}
	#include "sml_ClientEvents.h" 
	
	struct TclUserData {
	    Tcl_ThreadId threadId;
		Tcl_Interp* interp;
		Tcl_Obj* script;
		int callbackid;
	};
	
	std::list<TclUserData*> callbackdatas;
	
	typedef struct ThreadEventResult {
		Tcl_Condition done;         /* Signaled when the script completes */
		int code;                   /* Return value of Tcl_Eval */
		char *result;               /* Result from the script */
		char *errorInfo;            /* Copy of errorInfo variable */
		char *errorCode;            /* Copy of errorCode variable */
		Tcl_ThreadId srcThreadId;   /* Id of sending thread, in case it dies */
		Tcl_ThreadId dstThreadId;   /* Id of target thread, in case it dies */
		struct ThreadEvent *eventPtr;       /* Back pointer */
		struct ThreadEventResult *nextPtr;  /* List for cleanup */
		struct ThreadEventResult *prevPtr;
	} ThreadEventResult;

	typedef struct ThreadEvent {
		Tcl_Event event; /* Must be first */
		Tcl_Obj *script; /* script to run */
		ThreadEventResult *resultPtr; /* To communicate the result back */
	} ThreadEvent;

	Tcl_Interp *dispinterp;

	static void ThreadFreeProc(ClientData clientData)
	{
		if (clientData) {
			ckfree((char *) clientData);
		}
	}

	// evPtr is really a ThreadEvent*
	static int ThreadEventProc(Tcl_Event *evPtr, int mask)
	{
		ThreadEvent *threadEventPtr = (ThreadEvent *)evPtr;
		ThreadEventResult *resultPtr = threadEventPtr->resultPtr;
		Tcl_Interp *interp = dispinterp;
		int code;
		char const* result;
		char const* errorCode;
		char const* errorInfo;

		// Check which thread we're on.
		// I hope this is the thread I asked to be part of.	
		Tcl_ThreadId currentThread = Tcl_GetCurrentThread() ;

		if (interp == NULL) {
			code = TCL_ERROR;
			result = "no target interp!";
			errorCode = "THREAD";
			errorInfo = "";
		} else {
			Tcl_Preserve((ClientData) interp);
			Tcl_ResetResult(interp);
			Tcl_CreateThreadExitHandler(ThreadFreeProc,
					(ClientData) threadEventPtr->script);
			// since this script isn't going to be reused, there's no point in compiling to bytecode (hence the TCL_EVAL_DIRECT flag)
			code = Tcl_EvalObjEx(interp, threadEventPtr->script, TCL_EVAL_DIRECT);
			Tcl_DeleteThreadExitHandler(ThreadFreeProc,
					(ClientData) threadEventPtr->script);
			if (code != TCL_OK) {
				errorCode = Tcl_GetVar(interp, "errorCode", TCL_GLOBAL_ONLY);
				errorInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
			} else {
				errorCode = errorInfo = NULL;
			}
			result = Tcl_GetStringResult(interp);
		}
		if (interp != NULL) {
			Tcl_Release((ClientData) interp);
		}
		return 1;
	}

	int tcl_thread_send(Tcl_Interp* interp, Tcl_ThreadId id, Tcl_Obj* script)
	{
		ThreadEvent *threadEventPtr;
		ThreadEventResult *resultPtr;
		Tcl_ThreadId threadId = (Tcl_ThreadId) id;

		// Cache the interpreter so we can find it again later
		dispinterp = interp ;

		/*
		* Create the event for its event queue.
		*/

		threadEventPtr = (ThreadEvent *) ckalloc(sizeof(ThreadEvent));
		threadEventPtr->script = script;
		resultPtr = threadEventPtr->resultPtr = NULL;

		/*
		* Queue the event and poke the other thread's notifier.
		*/

		threadEventPtr->event.proc = ThreadEventProc;
		Tcl_ThreadQueueEvent(threadId, (Tcl_Event *)threadEventPtr,
				TCL_QUEUE_TAIL);
		Tcl_ThreadAlert(threadId);

		// Pump the event queue so the event is handled synchronously
		// Without this the call because asynchronous which is trouble.
		// BUGBUG? Should this be while (DoOneEvent() == 1) { } - i.e. clear the queue.
		Tcl_DoOneEvent(TCL_DONT_WAIT) ;

		return TCL_OK;
	}
	
	// Embedded $, [, \ and " are replaced with \$ \[ and \" to stop them being evaluated by
	// Tcl on the way through the callback.
	void EscapeSpecial(std::string* s) {
	    int pos = s->find_first_of("[$\"\\");
	    while(pos != std::basic_string<char*>::npos) {
	        s->insert(pos, "\\");
	        pos = s->find_first_of("[$\"\\", pos+2);
	    }	    
	}
	
	void TclOutputNotificationEventCallback(void* pUserData, sml::Agent* pAgent)
	{
	    TclUserData* tud = static_cast<TclUserData*>(pUserData);
	    // this script will never change, so we don't make a copy and we allow it to be compiled into bytecode (i.e. we don't pass TCL_EVAL_DIRECT)
	    Tcl_EvalObjEx(tud->interp, tud->script, 0);

		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;
	}
	
	std::string TclRhsEventCallback(sml::smlRhsEventId, void* pUserData, sml::Agent* pAgent, char const* pFunctionName,
	                    char const* pArgument)
	{
	    TclUserData* tud = static_cast<TclUserData*>(pUserData);
	    // this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
	    Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
	    Tcl_AppendObjToObj(script, SWIG_Tcl_NewInstanceObj(tud->interp, (void *) pAgent, SWIGTYPE_p_sml__Agent,0));
	    
	    // the argument could have embedded quotes, so we need to escape them
	    std::string argument = pArgument;
	    EscapeSpecial(&argument);
	    
	    Tcl_AppendStringsToObj(script, " ", pFunctionName, " \"", argument.c_str(), "\"", NULL);
	    // since we're returning a value, we need to clear out any old values
		Tcl_ResetResult(tud->interp);
		// since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
	    Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);

		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;

		Tcl_Obj* res = Tcl_GetObjResult(tud->interp);
		
		return Tcl_GetString(res);
	}

	std::string TclClientMessageEventCallback(sml::smlRhsEventId, void* pUserData, sml::Agent* pAgent, char const* pClientName,
	                    char const* pMessage)
	{
	    TclUserData* tud = static_cast<TclUserData*>(pUserData);
	    // this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
	    Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
	    Tcl_AppendObjToObj(script, SWIG_Tcl_NewInstanceObj(tud->interp, (void *) pAgent, SWIGTYPE_p_sml__Agent,0));
	    
	    // the message could have embedded quotes, so we need to escape them
	    std::string message = pMessage;
	    EscapeSpecial(&message);
	    
	    Tcl_AppendStringsToObj(script, " ", pClientName, " \"", message.c_str(), "\"", NULL);
	    // since we're returning a value, we need to clear out any old values
		Tcl_ResetResult(tud->interp);
		// since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
	    Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);

		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;

		Tcl_Obj* res = Tcl_GetObjResult(tud->interp);
		
		return Tcl_GetString(res);
	}
	
	void TclAgentEventCallback(sml::smlAgentEventId id, void* pUserData, sml::Agent* agent)
	{
		// we can ignore the id parameter because it's already in the script (from when we registered it)
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		// this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
	    Tcl_AppendObjToObj(script, SWIG_Tcl_NewInstanceObj(tud->interp, (void *) agent, SWIGTYPE_p_sml__Agent,0));
	    // since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
		Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);
		
		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;
	}
	
	void TclProductionEventCallback(sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation)
	{
		// we can ignore the agent and id parameters because they're already in the script (from when we registered it)
		unused(pAgent);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		// this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		Tcl_AppendStringsToObj(script, " \"", pProdName, "\"", NULL);
		
		// we need to check if pInstantiation is null, because if it is it will prematurely end the appending
		//  and we will lose the closing double quote, resulting in a syntax error
		if(pInstantiation) {
			Tcl_AppendStringsToObj(script, " \"", pInstantiation, "\"", NULL);
		} else {
			Tcl_AppendStringsToObj(script, " \"\"", NULL);
		}
		
		// since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
		Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);
		
		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;
	}
	
	void TclRunEventCallback(sml::smlRunEventId id, void* pUserData, sml::Agent* agent, sml::smlPhase phase)
	{
		// we can ignore the agent and id parameters because they're already in the script (from when we registered it)
		unused(agent);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		// this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_AppendObjToObj(script, Tcl_NewLongObj(long(phase)));
		// since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
		Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);
		
		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;
	}
	
	void TclPrintEventCallback(sml::smlPrintEventId id, void* pUserData, sml::Agent* agent, char const* pMessage)
	{
		// we can ignore these parameters because they're already in the script (from when we registered it)
		unused(agent);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		// this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		
		// the message could have embedded quotes, so we need to escape them
	    std::string message = pMessage;
	    EscapeSpecial(&message);
	    
		// wrap the message in quotes in case it has spaces
		Tcl_AppendStringsToObj(script, " \"", message.c_str(), "\"", NULL);
		// since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
		Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);
		
		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;
	}
	
	void TclXMLEventCallback(sml::smlXMLEventId id, void* pUserData, sml::Agent* agent, sml::ClientXML* pXML)
	{
		// we can ignore these parameters because they're already in the script (from when we registered it)
		unused(agent);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		// this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		// add a space to separate the args
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_AppendObjToObj(script, SWIG_Tcl_NewInstanceObj(tud->interp, (void *) pXML, SWIGTYPE_p_sml__ClientXML,0));
		// since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
		Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);
		
		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;
	}
	
	void TclOutputEventCallback(void* pUserData, sml::Agent* agent, char const* commandName, sml::WMElement* pOutputWme)
	{
		unused(agent);
		unused(commandName);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		// this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		// add a space to separate the args
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_AppendObjToObj(script, SWIG_Tcl_NewInstanceObj(tud->interp, (void *) pOutputWme, SWIGTYPE_p_sml__WMElement,0));
		// since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
		Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);
		
		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;
		
	}
	
	void TclSystemEventCallback(sml::smlSystemEventId id, void* pUserData, sml::Kernel* kernel)
	{
		// we can ignore these parameters because they're already in the script (from when we registered it)
		unused(kernel);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		// this script will never change, so we don't make a copy and we allow it to be compiled into bytecode (i.e. we don't pass TCL_EVAL_DIRECT)
		Tcl_EvalObjEx(tud->interp, tud->script, 0);
		
		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, tud->script) ;
	}

	void TclUpdateEventCallback(sml::smlUpdateEventId id, void* pUserData, sml::Kernel* kernel, sml::smlRunFlags runFlags)
	{
	    // we can ignore these parameters because they're already in the script (from when we registered it)
		unused(kernel);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		// this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		Tcl_AppendStringsToObj(script, " ", NULL);
		Tcl_AppendObjToObj(script, Tcl_NewLongObj(runFlags));
		// since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
		Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);

		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, script) ;
	}
	
	void TclStringEventCallback(sml::smlStringEventId id, void* pUserData, sml::Kernel* kernel, char const* pData)
	{
		// we can ignore these parameters because they're already in the script (from when we registered it)
		unused(kernel);
		unused(id);
		
		TclUserData* tud = static_cast<TclUserData*>(pUserData);
		// this beginning part of the script will never change, but the parts we add will, so we make a copy of the beginning part so we can reuse it next time
		Tcl_Obj* script = Tcl_DuplicateObj(tud->script);
		
		// the message could have embedded quotes, so we need to escape them
	    std::string data = pData;
	    EscapeSpecial(&data);
	    
		Tcl_AppendStringsToObj(script, " \"", data.c_str(), "\"", NULL);
		// since this script will never be executed again, we use TCL_EVAL_DIRECT, which skips the compilation step
		Tcl_EvalObjEx(tud->interp, script, TCL_EVAL_DIRECT);
		
		// Send the event to the given interpreter using the given thread
//		tcl_thread_send(tud->interp, tud->threadId, tud->script) ;
	}
	
	TclUserData* CreateTclUserData(int id, const char* proc, const char* userData, Tcl_Interp* interp) {
		TclUserData* tud = new TclUserData();
	    
	    tud->threadId = Tcl_GetCurrentThread();
	    tud->interp = interp;
	    // put all of the arguments together so we can just execute this as a single script later
	    // put spaces between the arguments and wrap the userdata in quotes (in case it has spaces)
	    tud->script = Tcl_NewObj();
	    Tcl_AppendStringsToObj(tud->script, proc, " ", NULL);
	    Tcl_AppendObjToObj(tud->script, Tcl_NewLongObj(id));
	    
	    // the user data could have embedded quotes, so we need to escape them
	    std::string ud = userData;
	    EscapeSpecial(&ud);
	    
	    Tcl_AppendStringsToObj(tud->script, " \"", ud.c_str(), "\" ", NULL);
		
		// Save the callback data so we can free it later
		callbackdatas.push_back(tud);
	    
	    return tud;
	}
	
	TclUserData* CreateTclOutputNotificationUserData(const char* proc, const char* userData, sml::Agent* self, Tcl_Interp* interp) {
		TclUserData* tud = new TclUserData();
	    
	    tud->threadId = Tcl_GetCurrentThread();
	    tud->interp = interp;
	    // put all of the arguments together so we can just execute this as a single script later
	    // put spaces between the arguments and wrap the userdata in quotes (in case it has spaces)
	    tud->script = Tcl_NewObj();
	    
	    // the user data could have embedded quotes, so we need to escape them
	    std::string ud = userData;
	    EscapeSpecial(&ud);
	    
	    Tcl_AppendStringsToObj(tud->script, proc, " \"", ud.c_str(), "\"", NULL);
	    Tcl_AppendObjToObj(tud->script, SWIG_NewInstanceObj((void *) self, SWIGTYPE_p_sml__Agent,0));
	    
	    return tud;
	}
	
	TclUserData* CreateTclAgentUserData(sml::Agent* self, int id, const char* proc, const char* userData, Tcl_Interp* interp) {
	    TclUserData* tud = CreateTclUserData(id, proc, userData, interp);
	    Tcl_AppendObjToObj(tud->script, SWIG_NewInstanceObj((void *) self, SWIGTYPE_p_sml__Agent,0));
	    
	    return tud;
	}
	
	TclUserData* CreateTclSystemUserData(sml::Kernel* self, int id, const char* proc, const char* userData, Tcl_Interp* interp) {
	    TclUserData* tud = CreateTclUserData(id, proc, userData, interp);
	    Tcl_AppendObjToObj(tud->script, SWIG_NewInstanceObj((void *) self, SWIGTYPE_p_sml__Kernel,0));
	    
	    return tud;
	}
	
	TclUserData* CreateTclOutputUserData(sml::Agent* self, const char* commandName, const char* proc, const char* userData, Tcl_Interp* interp) {
		TclUserData* tud = new TclUserData();
	    
	    tud->threadId = Tcl_GetCurrentThread();
	    tud->interp = interp;
	    // put all of the arguments together so we can just execute this as a single script later
	    // put spaces between the arguments and wrap the userdata in quotes (in case it has spaces)
	    tud->script = Tcl_NewObj();
	    
	    // the user data could have embedded quotes, so we need to escape them
	    std::string ud = userData;
	    EscapeSpecial(&ud);
	    
	    Tcl_AppendStringsToObj(tud->script, proc, " \"", ud.c_str(), "\" ", NULL);
	    Tcl_AppendObjToObj(tud->script, SWIG_NewInstanceObj((void *) self, SWIGTYPE_p_sml__Agent,0));
	    Tcl_AppendStringsToObj(tud->script, " ", commandName, " ", NULL);
	    
	    return tud;
	}

    void ReleaseCallbackData(TclUserData* tud) {
		// Release callback data and remove from collection of those we need to release at shutdown
		std::list<TclUserData*>::iterator itr = find(callbackdatas.begin(), callbackdatas.end(), tud);
		if(itr != callbackdatas.end()) {
			callbackdatas.erase(itr);
			delete tud;
		}
    }
    
    bool IsValidCallbackData(TclUserData* tud) {
		std::list<TclUserData*>::iterator itr = find(callbackdatas.begin(), callbackdatas.end(), tud);
		if(itr == callbackdatas.end()) {
			return false;
		} else {
			return true;
		}
    }
    
%}

%include "../sml_ClientInterface.i"

%extend sml::Agent {

	int RegisterForRunEvent(Tcl_Interp* interp, sml::smlRunEventId id, char* proc, char* userData, bool addToBack = true) {
        TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    tud->callbackid = self->RegisterForRunEvent(id, TclRunEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }
    
    int RegisterForProductionEvent(Tcl_Interp* interp, sml::smlProductionEventId id, char* proc, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    tud->callbackid = self->RegisterForProductionEvent(id, TclProductionEventCallback, (void*)tud, addToBack);
	    return (int)tud;
    }

    int RegisterForPrintEvent(Tcl_Interp* interp, sml::smlPrintEventId id, char* proc, char* userData, bool ignoreOwnEchos = true, bool addToBack = true) {	    
	    TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    tud->callbackid = self->RegisterForPrintEvent(id, TclPrintEventCallback, (void*)tud, ignoreOwnEchos, addToBack);
		return (int)tud;
    }
    
    int RegisterForXMLEvent(Tcl_Interp* interp, sml::smlXMLEventId id, char* proc, char* userData, bool addToBack = true) {	    
	    TclUserData* tud = CreateTclAgentUserData(self, id, proc, userData, interp);
	    tud->callbackid = self->RegisterForXMLEvent(id, TclXMLEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }
    
    int AddOutputHandler(Tcl_Interp* interp, char* attributeName, char* proc, char* userData, bool addToBack = true) {
		TclUserData* tud = CreateTclOutputUserData(self, attributeName, proc, userData, interp);
		tud->callbackid = self->AddOutputHandler(attributeName, TclOutputEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }
    
    int RegisterForOutputNotification(Tcl_Interp* interp, char* proc, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclOutputNotificationUserData(proc, userData, self, interp);
	    tud->callbackid = self->RegisterForOutputNotification(TclOutputNotificationEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }
    
    bool UnregisterForRunEvent(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForRunEvent(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool UnregisterForProductionEvent(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForProductionEvent(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool UnregisterForPrintEvent(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForPrintEvent(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool UnregisterForXMLEvent(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForXMLEvent(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool UnregisterForOutputNotification(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForOutputNotification(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool RemoveOutputHandler(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->RemoveOutputHandler(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
};

%extend sml::Kernel {
	
    int RegisterForSystemEvent(Tcl_Interp* interp, sml::smlSystemEventId id, char* proc, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclSystemUserData(self, id, proc, userData, interp);
	    tud->callbackid = self->RegisterForSystemEvent(id, TclSystemEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }
    
    int RegisterForUpdateEvent(Tcl_Interp* interp, sml::smlUpdateEventId id, char* proc, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclSystemUserData(self, id, proc, userData, interp);
	    tud->callbackid = self->RegisterForUpdateEvent(id, TclUpdateEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }
    
    int RegisterForStringEvent(Tcl_Interp* interp, sml::smlStringEventId id, char* proc, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclSystemUserData(self, id, proc, userData, interp);
	    tud->callbackid = self->RegisterForStringEvent(id, TclStringEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }
    
    int RegisterForAgentEvent(Tcl_Interp* interp, sml::smlAgentEventId id, char* proc, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclUserData(id, proc, userData, interp);
	    tud->callbackid = self->RegisterForAgentEvent(id, TclAgentEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }
    
    // Note that we're not exposing both the function name and callback as separate parameters, because it doesn't make much sense
    // to have different names in Tcl (and they are both strings -- in C++ they're not the same type; hence the separation there)
    int AddRhsFunction(Tcl_Interp* interp, char const* pRhsFunctionName, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclUserData(sml::smlEVENT_RHS_USER_FUNCTION, pRhsFunctionName, userData, interp);
	    tud->callbackid = self->AddRhsFunction(pRhsFunctionName, TclRhsEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }
    
    int RegisterForClientMessageEvent(Tcl_Interp* interp, char const* pClientName, char const* pMessageHandler, char* userData, bool addToBack = true) {
	    TclUserData* tud = CreateTclUserData(sml::smlEVENT_RHS_USER_FUNCTION, pMessageHandler, userData, interp);
	    tud->callbackid = self->RegisterForClientMessageEvent(pClientName, TclClientMessageEventCallback, (void*)tud, addToBack);
		return (int)tud;
    }

	bool UnregisterForSystemEvent(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForSystemEvent(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool UnregisterForUpdateEvent(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForUpdateEvent(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool UnregisterForStringEvent(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForStringEvent(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool UnregisterForAgentEvent(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForAgentEvent(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool RemoveRhsFunction(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->RemoveRhsFunction(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    bool UnregisterForClientMessageEvent(Tcl_Interp* interp, int id) {
		TclUserData* tud = (TclUserData *)id;
		if(!IsValidCallbackData(tud)) return false;
		self->UnregisterForClientMessageEvent(tud->callbackid);
		ReleaseCallbackData(tud);
		return true;
    }
    
    void Shutdown(Tcl_Interp* interp) {
		self->Shutdown();
		// Release remaining TclUserData's
		std::list<TclUserData*>::iterator itr;
		for(itr=callbackdatas.begin(); itr!=callbackdatas.end(); itr++)
		{
			delete (*itr);
		}
		callbackdatas.clear();
    }
};

