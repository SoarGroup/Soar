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

static double TclAgentEventCallBack(smlEventId id, void* pUserData, Agent* pAgent)
{
	TclUserData* ud = (TclUserData*)(pUserData);
	Tcl_Obj* = SWIG_NewInstanceObj((void *) pAgent, SWIGTYPE_p_sml__Agent,0)
	int errorCode = Tcl_EvalEx(ud->interp, ud->script, -1, 0);
	
	if(errorCode != TCL_OK) {
	  // not sure what we can do
	}
	
   PyObject *func, *arglist;
   PyObject *result;
   double    dres = 0;
   
   func = (PyObject *) clientdata;               // Get Python function
   arglist = Py_BuildValue("(d)",a);             // Build argument list
   result = PyEval_CallObject(func,arglist);     // Call Python
   Py_DECREF(arglist);                           // Trash arglist
   if (result) {                                 // If no errors, return double
     dres = PyFloat_AsDouble(result);
   }
   Py_XDECREF(result);
   return dres;

}
%}

// Attach a new method to our plot widget for adding Python functions
%addmethods PlotWidget {
   // Set a Python function object as a callback function
   // Note : PyObject *pyfunc is remapped with a typempap
   void set_pymethod(PyObject *pyfunc) {
     self->set_method(PythonCallBack, (void *) pyfunc);
     Py_INCREF(pyfunc);
   }
%addmethods Agent {
	void TclRegisterForAgentEvent(ClientData clientData, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[]) {
	
	}
	void TclRegisterForAgentEvent(smlEventId id, AgentEventHandler handler, void* pUserData) {
	}
}
*/