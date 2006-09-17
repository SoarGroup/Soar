%module Python_sml_ClientInterface

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
	
	struct PythonUserData {
		PyObject* func;
		PyObject* userdata;
		int callbackid;
		~PythonUserData () {
			Py_DECREF(userdata);
		}
	};
	
	std::list<PythonUserData*> callbackdatas;
	
	void PythonProductionEventCallback(sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation)
	{
		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOOss)", id, pud->userdata, SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0), pProdName, pInstantiation);
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}
	
	void PythonRunEventCallback(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase)
	{
		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOOi)", id, pud->userdata, SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0), phase);
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}
	
	void PythonPrintEventCallback(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage)
	{	
		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOOs)", id, pud->userdata, SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0), pMessage);
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}
	
	void PythonXMLEventCallback(sml::smlXMLEventId id, void* pUserData, sml::Agent* pAgent, sml::ClientXML* pXML)
	{
		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOOO)", id, pud->userdata, SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0), SWIG_NewInstanceObj((void *) pXML, SWIGTYPE_p_sml__ClientXML,0));
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}
	
	void PythonOutputEventCallback(void* pUserData, sml::Agent* pAgent, char const* commandName, sml::WMElement* pOutputWme)
	{
		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(OsO)", pud->userdata, SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0), commandName, SWIG_NewInstanceObj((void *) pOutputWme, SWIGTYPE_p_sml__WMElement,0));
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}
	
	void PythonOutputNotificationEventCallback(void* pUserData, sml::Agent* pAgent)
	{
		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(OO)", pud->userdata, SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0));
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}
	
	void PythonSystemEventCallback(sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel)
	{
		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOO)", id, pud->userdata, SWIG_NewInstanceObj((void *) pKernel, SWIGTYPE_p_sml__Kernel,0));
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}

	void PythonUpdateEventCallback(sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags)
	{
	    PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOOi)", id, pud->userdata, SWIG_NewInstanceObj((void *) pKernel, SWIGTYPE_p_sml__Kernel,0), runFlags);
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}
	
	void PythonStringEventCallback(sml::smlStringEventId id, void* pUserData, sml::Kernel* pKernel, char const* pData)
	{
		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOOs)", id, pud->userdata, SWIG_NewInstanceObj((void *) pKernel, SWIGTYPE_p_sml__Kernel,0), pData);
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}
	
	void PythonAgentEventCallback(sml::smlAgentEventId id, void* pUserData, sml::Agent* pAgent)
	{
		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOO)", id, pud->userdata, SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0));
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result!=0) Py_DECREF(result);
	}
	
	std::string PythonRhsEventCallback(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pFunctionName, char const* pArgument)
	{
	    PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOOss)", id, pud->userdata, SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0), pFunctionName, pArgument);
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result==0 || !PyString_Check(result))
			return "";
		
		std::string res = PyString_AsString(result);
		Py_DECREF(result);
		
		return res;
	}

	std::string PythonClientMessageEventCallback(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pClientName, char const* pMessage)
	{
	    PythonUserData* pud = static_cast<PythonUserData*>(pUserData);
		PyObject* args = Py_BuildValue("(iOOss)", id, pud->userdata, SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0), pClientName, pMessage);
		PyObject* result = PyEval_CallObject(pud->func, args);
		
		Py_DECREF(args);
		if(result==0 || !PyString_Check(result))
			return "";
		
		std::string res = PyString_AsString(result);
		Py_DECREF(result);
		
		return res;
	}
	
	PythonUserData* CreatePythonUserData(PyObject* func, PyObject* userData) {
		PythonUserData* pud = new PythonUserData();
	    
	    pud->func = func;
	    pud->userdata = userData;
	    
	    // Save the callback data so we can free it later
		callbackdatas.push_back(pud);
	    
	    return pud;
	}
	
	void ReleaseCallbackData(PythonUserData* pud) {
		// Release callback data and remove from collection of those we need to release at shutdown
		std::list<PythonUserData*>::iterator itr = find(callbackdatas.begin(), callbackdatas.end(), pud);
		if(itr != callbackdatas.end()) {
			callbackdatas.erase(itr);
			delete pud;
		}
    }
    
    bool IsValidCallbackData(PythonUserData* pud) {
		std::list<PythonUserData*>::iterator itr = find(callbackdatas.begin(), callbackdatas.end(), pud);
		if(itr == callbackdatas.end()) {
			return false;
		} else {
			return true;
		}
    }

%}

// Make sure that the PyObject we get for the func is a callable object
%typemap(check) PyObject* func {
	if (!PyCallable_Check($1)) {
		PyErr_SetString(PyExc_TypeError, "Need a callable object!");
		return NULL;
	}
}

%apply PyObject* func { PyObject* pMessageHandler }

//%include "../sml_ClientInterface.i"

%extend sml::Agent {

	int RegisterForRunEvent(sml::smlRunEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
		PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForRunEvent(id, PythonRunEventCallback, (void*)pud, addToBack);
	    return (int)pud;
	}
    
    int RegisterForProductionEvent(sml::smlProductionEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
		PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForProductionEvent(id, PythonProductionEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }

    int RegisterForPrintEvent(sml::smlPrintEventId id, PyObject* func, PyObject* userData, bool ignoreOwnEchos = true, bool addToBack = true) {	    
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForPrintEvent(id, PythonPrintEventCallback, (void*)pud, ignoreOwnEchos, addToBack);
	    return (int)pud;
    }
   
    int RegisterForXMLEvent(sml::smlXMLEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {	    
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForXMLEvent(id, PythonXMLEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }
    
    int AddOutputHandler(char* attributeName, PyObject* func, PyObject* userData, bool addToBack = true) {
		PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->AddOutputHandler(attributeName, PythonOutputEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }
    
    int RegisterForOutputNotification(PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForOutputNotification(PythonOutputNotificationEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }
    
    bool UnregisterForRunEvent(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForRunEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool UnregisterForProductionEvent(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForProductionEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool UnregisterForPrintEvent(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForPrintEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool UnregisterForXMLEvent(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForXMLEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool UnregisterForOutputNotification(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForOutputNotification(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool RemoveOutputHandler(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->RemoveOutputHandler(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
};

%extend sml::Kernel {

    int RegisterForSystemEvent(sml::smlSystemEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForSystemEvent(id, PythonSystemEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }
    
    int RegisterForUpdateEvent(sml::smlUpdateEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForUpdateEvent(id, PythonUpdateEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }
    
    int RegisterForStringEvent(sml::smlStringEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForStringEvent(id, PythonStringEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }
    
    int RegisterForAgentEvent(sml::smlAgentEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForAgentEvent(id, PythonAgentEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }
    
    int AddRhsFunction(char const* pRhsFunctionName, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->AddRhsFunction(pRhsFunctionName, PythonRhsEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }
    
    int RegisterForClientMessageEvent(char const* pClientName, PyObject* pMessageHandler, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(pMessageHandler, userData);
	    pud->callbackid = self->RegisterForClientMessageEvent(pClientName, PythonClientMessageEventCallback, (void*)pud, addToBack);
	    return (int)pud;
    }
    
    bool UnregisterForSystemEvent(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForSystemEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool UnregisterForUpdateEvent(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForUpdateEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool UnregisterForStringEvent(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForStringEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool UnregisterForAgentEvent(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForAgentEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool RemoveRhsFunction(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->RemoveRhsFunction(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
    
    bool UnregisterForClientMessageEvent(int id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForClientMessageEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }
/*    
    void Shutdown() {
		self->Shutdown();
		// Release remaining PythonUserData's
		std::list<PythonUserData*>::iterator itr;
		for(itr=callbackdatas.begin(); itr!=callbackdatas.end(); itr++)
		{
			delete (*itr);
		}
		callbackdatas.clear();
    }
*/
};

// Add cleanup code to Shutdown
%exception Shutdown {
		$action
		// Release remaining PythonUserData's
		std::list<PythonUserData*>::iterator itr;
		for(itr=callbackdatas.begin(); itr!=callbackdatas.end(); itr++)
		{
			delete (*itr);
		}
		callbackdatas.clear();
}

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

%include "../sml_ClientInterface.i"