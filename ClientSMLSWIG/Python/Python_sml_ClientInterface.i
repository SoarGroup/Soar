%module Python_sml_ClientInterface

// This works around a Python "feature" where it wants to link with python24_d.lib when building in debug mode
// (unfortunately, this library is not distributed with the Windows release)
%runtime %{
#ifdef PY_WKAROUND_DEBUG
#	define _DEBUG
#endif
%}

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
	
	struct PythonUserData {
		PyObject* func;
		PyObject* userdata;
	};
	
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
	    
	    return pud;
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

%include "../sml_ClientInterface.i"

%extend sml::Agent {

	int RegisterForRunEvent(sml::smlRunEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
		PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->RegisterForRunEvent(id, PythonRunEventCallback, (void*)pud, addToBack);
	}
    
    int RegisterForProductionEvent(sml::smlProductionEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
		PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->RegisterForProductionEvent(id, PythonProductionEventCallback, (void*)pud, addToBack);
    }

    int RegisterForPrintEvent(sml::smlPrintEventId id, PyObject* func, PyObject* userData, bool ignoreOwnEchos = true, bool addToBack = true) {	    
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->RegisterForPrintEvent(id, PythonPrintEventCallback, (void*)pud, ignoreOwnEchos, addToBack);
    }
   
    int RegisterForXMLEvent(sml::smlXMLEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {	    
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->RegisterForXMLEvent(id, PythonXMLEventCallback, (void*)pud, addToBack);
    }
    
    int AddOutputHandler(char* attributeName, PyObject* func, PyObject* userData, bool addToBack = true) {
		PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->AddOutputHandler(attributeName, PythonOutputEventCallback, (void*)pud, addToBack);
    }
    
    int RegisterForOutputNotification(PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->RegisterForOutputNotification(PythonOutputNotificationEventCallback, (void*)pud, addToBack);
    }
    
}

%extend sml::Kernel {

    int RegisterForSystemEvent(sml::smlSystemEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->RegisterForSystemEvent(id, PythonSystemEventCallback, (void*)pud, addToBack);
    };
    
    int RegisterForUpdateEvent(sml::smlUpdateEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->RegisterForUpdateEvent(id, PythonUpdateEventCallback, (void*)pud, addToBack);
    };
    
    int RegisterForStringEvent(sml::smlStringEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->RegisterForStringEvent(id, PythonStringEventCallback, (void*)pud, addToBack);
    };
    
    int RegisterForAgentEvent(sml::smlAgentEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->RegisterForAgentEvent(id, PythonAgentEventCallback, (void*)pud, addToBack);
    };
    
    int AddRhsFunction(char const* pRhsFunctionName, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    return self->AddRhsFunction(pRhsFunctionName, PythonRhsEventCallback, (void*)pud, addToBack);
    };
    
    int RegisterForClientMessageEvent(char const* pClientName, PyObject* pMessageHandler, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(pMessageHandler, userData);
	    return self->RegisterForClientMessageEvent(pClientName, PythonClientMessageEventCallback, (void*)pud, addToBack);
    };
}

%include "../sml_ClientInterface.i"