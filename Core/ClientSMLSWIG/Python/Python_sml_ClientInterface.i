%begin %{
/* This will make the linker link against python2x.lib instead of python2x_d.lib */
#if defined _MSC_VER && defined _DEBUG
#undef _DEBUG
#endif
%}

%module Python_sml_ClientInterface
%feature("autodoc","3");

// handle windows calling convention, __declspec(dllimport), correctly
%include <windows.i>

%{
	// helps quell warnings
	#ifndef unused
	#define unused(x) (void)(x)
	#endif

	#include <string>
	#include <list>
	#include <algorithm>
	#include <iostream>
    #include <string>

	namespace sml {
	class Agent;
	}
	#include "sml_ClientEvents.h"

	struct PythonUserData {
		PyObject* func;
		PyObject* userdata;
		int callbackid;
		~PythonUserData () {
			PyGILState_STATE gstate;
			gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */
			Py_DECREF(userdata);
			PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
		}
	};

	std::list<PythonUserData*> callbackdatas;

	void show_exception_and_exit(const char *type, int id) {
		PyTraceBack_Here(PyEval_GetFrame());
        std::cerr << "Uncaught Python exception in " << type << " callback id = " << id << ". Exiting." << std::endl;
		PyObject *excType, *excValue, *excTraceback;
		PyErr_Fetch(&excType, &excValue, &excTraceback);
		PyErr_NormalizeException(&excType, &excValue, &excTraceback);
		PyTraceBack_Print(excTraceback, PySys_GetObject("stderr"));
		exit(1);
	}

	void PythonProductionEventCallback(sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation)
	{
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* agent = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0);
		PyObject* args = Py_BuildValue("(iOOss)", id, pud->userdata, agent, pProdName, pInstantiation);
		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(agent);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("production event", id);
		} else {
			Py_DECREF(result);
		}

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
	}

	void PythonRunEventCallback(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase)
	{
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* agent = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0);
		PyObject* args = Py_BuildValue("(iOOi)", id, pud->userdata, agent, phase);
		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(agent);
		Py_DECREF(args);
		if (!result) {
			show_exception_and_exit("run event", id);
		} else {
			Py_DECREF(result);
		}

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
	}

	void PythonPrintEventCallback(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage)
	{
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* agent = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0);
		PyObject* args = Py_BuildValue("(iOOs)", id, pud->userdata, agent, pMessage);
		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(agent);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("print event", id);
		} else {
			Py_DECREF(result);
		}

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
	}

	void PythonXMLEventCallback(sml::smlXMLEventId id, void* pUserData, sml::Agent* pAgent, sml::ClientXML* pXML)
	{
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* agent = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0);
		PyObject* xml = SWIG_NewInstanceObj((void *) pXML, SWIGTYPE_p_sml__ClientXML,0);
		PyObject* args = Py_BuildValue("(iOOO)", id, pud->userdata, agent, xml);
        PyObject *result = PyObject_Call(pud->func, args, NULL);

        Py_DECREF(agent);
		Py_DECREF(xml);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("XML event", id);
		} else {
			Py_DECREF(result);
		}

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
	}

	void PythonOutputEventCallback(void* pUserData, sml::Agent* pAgent, char const* commandName, sml::WMElement* pOutputWme)
	{
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* agent = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0);
		PyObject* wme = SWIG_NewInstanceObj((void *) pOutputWme, SWIGTYPE_p_sml__WMElement,0);
		PyObject* args = Py_BuildValue("(OOsO)", pud->userdata, agent, commandName, wme);
        PyObject *result = PyObject_Call(pud->func, args, NULL);

        Py_DECREF(agent);
		Py_DECREF(wme);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("output event", -1);
		} else {
			Py_DECREF(result);
		}

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
	}

	void PythonOutputNotificationEventCallback(void* pUserData, sml::Agent* pAgent)
	{
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* agent = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0);
		PyObject* args = Py_BuildValue("(OO)", pud->userdata, agent);
		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(agent);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("output notification event", -1);
		} else {
			Py_DECREF(result);
		}

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
	}

	void PythonSystemEventCallback(sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel)
	{
        std::cerr << "PythonSystemEventCallback1: " << id << std::endl;
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* kernel = SWIG_NewInstanceObj((void *) pKernel, SWIGTYPE_p_sml__Kernel,0);
		PyObject* args = Py_BuildValue("(iOO)", id, pud->userdata, kernel);
		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(kernel);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("system event", id);
		} else {
			Py_DECREF(result);
		}

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
	}

	void PythonUpdateEventCallback(sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags)
	{
	    PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* kernel = SWIG_NewInstanceObj((void*) pKernel, SWIGTYPE_p_sml__Kernel, 0);
		PyObject* args = Py_BuildValue("(iOOi)", id, pud->userdata, kernel, runFlags);
		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(kernel);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("update event", id);
		} else {
			Py_DECREF(result);
		}

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
	}

	std::string PythonStringEventCallback(sml::smlStringEventId id, void* pUserData, sml::Kernel* pKernel, char const* pData)
	{
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* kernel = SWIG_NewInstanceObj((void *) pKernel, SWIGTYPE_p_sml__Kernel,0);
		PyObject* args = Py_BuildValue("(iOOs)", id, pud->userdata, kernel, pData);
		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(kernel);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("string event", id);
		} else if (!PyUnicode_Check(result)) {
			return "";
		}

		std::string res = PyUnicode_AsUTF8 (result);
		Py_DECREF(result);

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */

		return res;
	}

	void PythonAgentEventCallback(sml::smlAgentEventId id, void* pUserData, sml::Agent* pAgent)
	{
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* agent = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0);
		PyObject* args = Py_BuildValue("(iOO)", id, pud->userdata, agent);
		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(agent);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("agent event", id);
		} else {
			Py_DECREF(result);
		}

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */
	}

	const std::string PythonRhsEventCallback(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pFunctionName, char const* pArgument)
	{
	    PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* agent = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0);
		PyObject* args = Py_BuildValue("(iOOss)", id, pud->userdata, agent, pFunctionName, pArgument);

		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(agent);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("RHS event", id);
		} else if (!PyUnicode_Check(result)) {
			return "";
		}

		std::string res = PyUnicode_AsUTF8 (result);
		Py_DECREF(result);

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */

        return res;
	}

    const sml::RhsEventHandlerCpp getPythonRhsEventCallback(void *pUserData)
    {
        return [pUserData](sml::smlRhsEventId id, sml::Agent *pAgent, char const *pFunctionName, char const *pArgument) -> const std::string
        {
            return PythonRhsEventCallback(id, pUserData, pAgent, pFunctionName, pArgument);
        };
    }

    const char *PythonClientMessageEventCallback(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pClientName, char const* pMessage, int *bufSize, char *buf)
	{
        // Previous result was cached, meaning client should be calling again to get it
        // return that result and clear the cache
        static std::string prevResult;
        if ( !prevResult.empty() )
        {
            strncpy( buf, prevResult.c_str(), *bufSize );

            prevResult = "";

            return buf;
        }

	    PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */

		PythonUserData* pud = static_cast<PythonUserData*>(pUserData);

		PyObject* agent = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0);
		PyObject* args = Py_BuildValue("(iOOss)", id, pud->userdata, agent, pClientName, pMessage);
		PyObject* result = PyObject_Call(pud->func, args, NULL);

		Py_DECREF(agent);
		Py_DECREF(args);
		if(!result) {
			show_exception_and_exit("client message event", id);
		} else if (!PyUnicode_Check(result)) {
			return "";
		}

		std::string res = PyUnicode_AsUTF8 (result);
		Py_DECREF(result);

		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */

        // Too long to fit in the buffer; cache result and signal client with
        // NULL return value to call again with a larger buffer
        if ( res.length() + 1 > *bufSize )
        {
            *bufSize = res.length() + 1;
            prevResult = res;
            return NULL;
        }
        strcpy( buf, res.c_str() );

        return buf;
	}

	PythonUserData* CreatePythonUserData(PyObject* func, PyObject* userData) {
		PythonUserData* pud = new PythonUserData();
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure(); /* Get the thread.  No Python API allowed before this point. */
		Py_INCREF(userData);
		PyGILState_Release(gstate); /* Release the thread. No Python API allowed beyond this point. */

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

	long RegisterForRunEvent(sml::smlRunEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
		PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForRunEvent(id, PythonRunEventCallback, (void*)pud, addToBack);
	    return (long)pud;
	}

    long RegisterForProductionEvent(sml::smlProductionEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
		PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForProductionEvent(id, PythonProductionEventCallback, (void*)pud, addToBack);
	    return (long)pud;
    }

    long RegisterForPrintEvent(sml::smlPrintEventId id, PyObject* func, PyObject* userData, bool ignoreOwnEchos = true, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForPrintEvent(id, PythonPrintEventCallback, (void*)pud, ignoreOwnEchos, addToBack);
	    return (long)pud;
    }

    long RegisterForXMLEvent(sml::smlXMLEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForXMLEvent(id, PythonXMLEventCallback, (void*)pud, addToBack);
	    return (long)pud;
    }

    long AddOutputHandler(char* attributeName, PyObject* func, PyObject* userData, bool addToBack = true) {
		PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->AddOutputHandler(attributeName, PythonOutputEventCallback, (void*)pud, addToBack);
	    return (long)pud;
    }

    long RegisterForOutputNotification(PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForOutputNotification(PythonOutputNotificationEventCallback, (void*)pud, addToBack);
	    return (long)pud;
    }

    bool UnregisterForRunEvent(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForRunEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool UnregisterForProductionEvent(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForProductionEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool UnregisterForPrintEvent(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForPrintEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool UnregisterForXMLEvent(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForXMLEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool UnregisterForOutputNotification(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForOutputNotification(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool RemoveOutputHandler(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->RemoveOutputHandler(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

};

%extend sml::Kernel {

    long RegisterForSystemEvent(sml::smlSystemEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
        std::cerr << "RegisterForSystemEvent: " << id << std::endl;
        PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForSystemEvent(id, PythonSystemEventCallback, (void*)pud, addToBack);
	    return (long)pud;
    }

    long RegisterForUpdateEvent(sml::smlUpdateEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForUpdateEvent(id, PythonUpdateEventCallback, (void*)pud, addToBack);
	    return (long)pud;
    }

    long RegisterForStringEvent(sml::smlStringEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForStringEvent(id, PythonStringEventCallback, (void*)pud, addToBack);
	    return (long)pud;
    }

    long RegisterForAgentEvent(sml::smlAgentEventId id, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
	    pud->callbackid = self->RegisterForAgentEvent(id, PythonAgentEventCallback, (void*)pud, addToBack);
	    return (long)pud;
    }

    long AddRhsFunction(char const* pRhsFunctionName, PyObject* func, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(func, userData);
        pud->callbackid = self->AddRhsFunction(pRhsFunctionName, getPythonRhsEventCallback((void*)pud), addToBack);
	    return (long)pud;
    }

    long RegisterForClientMessageEvent(char const* pClientName, PyObject* pMessageHandler, PyObject* userData, bool addToBack = true) {
	    PythonUserData* pud = CreatePythonUserData(pMessageHandler, userData);
	    pud->callbackid = self->RegisterForClientMessageEvent(pClientName, PythonClientMessageEventCallback, (void*)pud, addToBack);
	    return (long)pud;
    }

    bool UnregisterForSystemEvent(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForSystemEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool UnregisterForUpdateEvent(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForUpdateEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool UnregisterForStringEvent(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForStringEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool UnregisterForAgentEvent(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->UnregisterForAgentEvent(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool RemoveRhsFunction(long id) {
		PythonUserData* pud = (PythonUserData *)id;
		if(!IsValidCallbackData(pud)) return false;
		self->RemoveRhsFunction(pud->callbackid);
		ReleaseCallbackData(pud);
		return true;
    }

    bool UnregisterForClientMessageEvent(long id) {
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

%include "../sml_ClientInterface.i"

//%newobject sml::Kernel::CreateKernelInCurrentThread;
//%newobject sml::Kernel::CreateKernelInNewThread;
//%newobject sml::Kernel::CreateRemoteConnection;

