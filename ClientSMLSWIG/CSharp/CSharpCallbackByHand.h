/////////////////////////////////////////////////////////////////
// CSharp callback support methods
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2005
//
// Some handcoded methods to support registering callbacks for
// events through CSharp.
//
/*
SWIGEXPORT void SWIGSTDCALL CSharp_Kernel_SetTraceCommunications(void * jarg1, unsigned int jarg2) {
    sml::Kernel *arg1 = (sml::Kernel *) 0 ;
    bool arg2 ;
    
    arg1 = (sml::Kernel *)jarg1; 
    arg2 = jarg2 ? true : false; 
    (arg1)->SetTraceCommunications(arg2);
    
}
*/

	/*
	typedef void (__stdcall *PFN_MYCALLBACK)();
	int __stdcall MyFunction(PFN_ MYCALLBACK callback);

	public delegate void MyCallback();
	[DllImport("MYDLL.DLL")]
	public static extern void MyFunction(MyCallback callback);
	*/

//typedef void (__stdcall *MyTestCallback)() ;
/*
SWIGEXPORT void SWIGSTDCALL CSharp_Kernel_RegisterTestMethod(void * jarg1, unsigned int jarg2) {
    sml::Kernel *arg1 = (sml::Kernel *) 0 ;
	MyTestCallback pFunction = (MyTestCallback)jarg2 ;
    
    arg1 = (sml::Kernel *)jarg1; 
    
	// Make the callback right now as a test
	pFunction() ;
}
*/

typedef int	agentPtr ;
typedef int	CallbackDataPtr ;

typedef void (__stdcall *RunEventCallback)(int eventID, CallbackDataPtr callbackData, agentPtr jagent, int phase) ;

/* Callback for deleting GCHandle objects from within C#, so we don't leak them. */
typedef void (SWIGSTDCALL* CSharpHandleHelperCallback)(unsigned int);
static CSharpHandleHelperCallback SWIG_csharp_deletehandle_callback = NULL;

SWIGEXPORT void SWIGSTDCALL CSharp_Kernel_RegisterHandleHelper(CSharpHandleHelperCallback callback) {
  SWIG_csharp_deletehandle_callback = callback;
}

class CSharpCallbackData
{
// Making these public as this is basically just a struct.
public:
	int				m_EventID ;
	agentPtr		m_Agent ;
	void*			m_CallbackFunction ;
	CallbackDataPtr	m_CallbackData ;
	int				m_CallbackID ;

public:
	CSharpCallbackData(agentPtr jagent, int eventID, void* callbackFunction, unsigned int callbackData)
	{
		m_Agent = jagent ;
		m_EventID = eventID ;
		m_CallbackFunction = callbackFunction ;
		m_CallbackData = callbackData ;
		m_CallbackID = 0 ;
	}

	~CSharpCallbackData()
	{
	}
} ;

static CSharpCallbackData* CreateCSharpCallbackData(agentPtr jagent, int eventID, unsigned int callbackFunction, CallbackDataPtr callbackData)
{
	return new CSharpCallbackData(jagent, eventID, (void *)callbackFunction, callbackData) ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to C# to pass back the message.
static void RunEventHandler(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	CSharpCallbackData* pData = (CSharpCallbackData*)pUserData ;

	RunEventCallback callback = (RunEventCallback)pData->m_CallbackFunction ;

	// Now try to call back to CSharp
	callback(pData->m_EventID, pData->m_CallbackData, pData->m_Agent, phase) ;
}

SWIGEXPORT int SWIGSTDCALL CSharp_Agent_RegisterForRunEvent(void * jarg1, int jarg2, agentPtr jagent, unsigned int jarg3, CallbackDataPtr jdata)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlRunEventId arg2 = (sml::smlRunEventId)jarg2;

	// jarg3 is the callback function

	// Create the information we'll need to make a Java call back later
	CSharpCallbackData* pData = CreateCSharpCallbackData(jagent, jarg2, jarg3, jdata) ;
	
	// Register our handler.  When this is called we'll call back to the client method.
	pData->m_CallbackID = arg1->RegisterForRunEvent(arg2, &RunEventHandler, pData) ;

	// Pass the callback info back to the client.  We need to do this so we can delete this later when the method is unregistered
	return (int)pData ;
}


SWIGEXPORT bool SWIGSTDCALL CSharp_Agent_UnregisterForRunEvent(void* jarg1, int jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	CSharpCallbackData* pData = (CSharpCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->UnregisterForRunEvent(pData->m_CallbackID) ;

	// Free the GCHandles created when the callback was registered
	SWIG_csharp_deletehandle_callback(pData->m_Agent) ;
	SWIG_csharp_deletehandle_callback(pData->m_CallbackData) ;

	// Release the callback data
	delete pData ;

	return result ;
}

/*
#include <string>

class JavaCallbackData
{
// Making these public as this is basically just a struct.
public:
	JavaVM*		m_JavaVM ;			// The Java Virtual Machine
	jobject		m_AgentObject ;		// The Java agent object
	jobject		m_KernelObject ;	// The Java kernel object (either this or agent is set usually)
	jobject		m_HandlerObject ;	// The object that contains the method we will call
	std::string m_HandlerMethod ;	// The name of the method we will call
	jobject		m_CallbackData ;	// Arbitrary Java object which we'll pass back in the call.
	int			m_CallbackID ;		// Unique ID for this callback (we use this during unregistering)

private:
	JNIEnv*		m_JavaEnv ;			// The JNI environment (private so we're forced to use the accessor function)

public:
	JavaCallbackData(JavaVM* pVM, JNIEnv* pEnv, jobject agentObject, jobject kernelObject, jobject handlerObject, char const* handlerMethod, jobject callbackData)
	{
		m_JavaVM		= pVM ;
		m_JavaEnv		= pEnv ;
		m_AgentObject	= agentObject ;
		m_KernelObject	= kernelObject ;
		m_HandlerObject = handlerObject ;
		m_HandlerMethod = handlerMethod ;
		m_CallbackData = callbackData ;
		m_CallbackID   = 0 ;
	}

	JNIEnv* GetEnv()
	{
		// We have several options here to get the JNIEnv*.
		// 1) We use the one we were passed originally in the registration call: m_JavaEnv
		// 2) We call m_JavaVM->GetEnv((void**)&penv, JNI_VERSION_1_4) ;
		// 3) We call m_JavaVM->AttachCurrentThread((void**)&penv, 0) ;
		// I think we need to use the 3rd form if we wish to support callbacks on a different thread.
		JNIEnv* penv ;
		int result = m_JavaVM->AttachCurrentThread((void**)&penv, 0) ;

		// Error attaching to the VM
		if (result < 0)
			return 0 ;

		return penv ;
	}

	// We need to clean up the global references that we created earlier
	~JavaCallbackData()
	{
		JNIEnv* penv = GetEnv() ;
		if (m_HandlerObject) penv->DeleteGlobalRef(m_HandlerObject) ;
		if (m_CallbackData)  penv->DeleteGlobalRef(m_CallbackData) ;
		if (m_AgentObject)   penv->DeleteGlobalRef(m_AgentObject) ;
		if (m_KernelObject)  penv->DeleteGlobalRef(m_KernelObject) ;
	}
} ;

// Collect the Java values into a single object which we'll register with our local event handler.
// When this handler is called we'll unpack the Java data and make a callback to the Java process.
static JavaCallbackData* CreateJavaCallbackData(bool storeAgent, JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, char const* pMethodName, jobject jarg6)
{
	// The trick here is we collect up all of the Java information and store that as the callback data
	// for our local handler.  When our handler is called we'll unpack the data and make the Java callback.

	// This is critical: we need to make our references to the objects "global".
	// Otherwise the JNI objects will be deleted when we exit this method making them useless
	// to us when the eventual callback occurs.
	jobject jglobal3 = jenv->NewGlobalRef(jarg3) ;	// The Java agent or kernel object
	jobject jglobal4 = jenv->NewGlobalRef(jarg4) ;	// The Java object which will handle this callback
	jobject jglobal6 = jenv->NewGlobalRef(jarg6) ;	// Arbitrary object passed back to the caller

	// Get the method name from the Java string
	// We used to pass this in as a parameter to the callback, but now its imbedded in the interface definition.
	//const char *pMethodName = jenv->GetStringUTFChars(jarg5, 0);

	// Record the virtual machine we are using
	JavaVM vm ;
	JavaVM* pvm = &vm ;
	jint result = jenv->GetJavaVM(&pvm) ;
	
	if (result != 0)
	{
		printf("Error getting Java VM\n") ;
		return 0 ;
	}

	JavaCallbackData* pJavaData = new JavaCallbackData(pvm, jenv, storeAgent ? jglobal3 : 0, storeAgent ? 0 : jglobal3, jglobal4, pMethodName, jglobal6) ;

	// Release the string we got from Java
	//jenv->ReleaseStringUTFChars(jarg5, pMethodName);

	return pJavaData ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
static void RunEventHandler(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv();

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(ILjava/lang/Object;Lsml/Agent;I)V") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return ;
	}

	// Make the method call.
	jenv->CallVoidMethod(jobj, mid, (int)id, pJavaData->m_CallbackData, pJavaData->m_AgentObject, (int)phase);
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT jint JNICALL Java_sml_smlJNI_Agent_1RegisterForRunEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jobject jarg6)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlRunEventId arg2 = (sml::smlRunEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, jarg2, jarg3, jarg4, "runEventHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForRunEvent(arg2, &RunEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}


JNIEXPORT bool JNICALL Java_sml_smlJNI_Agent_1UnregisterForRunEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->UnregisterForRunEvent(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
static void ProductionEventHandler(sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv() ;

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(ILjava/lang/Object;Lsml/Agent;Ljava/lang/String;Ljava/lang/String;)V") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return ;
	}

	// Convert our C++ strings to Java strings
	jstring prod = pProdName != NULL ? jenv->NewStringUTF(pProdName) : 0 ;
	jstring inst = pInstantiation != NULL ? jenv->NewStringUTF(pInstantiation) : 0 ;

	// Make the method call.
	jenv->CallVoidMethod(jobj, mid, (int)id, pJavaData->m_CallbackData, pJavaData->m_AgentObject, prod, inst);
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT int JNICALL Java_sml_smlJNI_Agent_1RegisterForProductionEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jobject jarg6)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlProductionEventId arg2 = (sml::smlProductionEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, jarg2, jarg3, jarg4, "productionEventHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForProductionEvent(arg2, &ProductionEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}


JNIEXPORT bool JNICALL Java_sml_smlJNI_Agent_1UnregisterForProductionEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->UnregisterForProductionEvent(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
static void PrintEventHandler(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv() ;

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(ILjava/lang/Object;Lsml/Agent;Ljava/lang/String;)V") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return ;
	}

	// Convert our C++ strings to Java strings
	jstring message = pMessage != NULL ? jenv->NewStringUTF(pMessage) : 0 ;

	// Make the method call.
	jenv->CallVoidMethod(jobj, mid, (int)id, pJavaData->m_CallbackData, pJavaData->m_AgentObject, message);
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT jint JNICALL Java_sml_smlJNI_Agent_1RegisterForPrintEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jobject jarg6, jboolean jarg7)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlPrintEventId arg2 = (sml::smlPrintEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, jarg2, jarg3, jarg4, "printEventHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForPrintEvent(arg2, &PrintEventHandler, pJavaData, (jarg7 != JNI_FALSE)) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT bool JNICALL Java_sml_smlJNI_Agent_1UnregisterForPrintEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->UnregisterForPrintEvent(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
static void XMLEventHandler(sml::smlXMLEventId id, void* pUserData, sml::Agent* pAgent, sml::ClientXML* pXML)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv() ;

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(ILjava/lang/Object;Lsml/Agent;Lsml/ClientXML;)V") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return ;
	}

	// Wrap our C++ ClientXML object with a SWIG sml/ClientXML Java object so we can
	// pass it back to Java.
	// OK, time to roll up our JNI sleeves and get dirty.  We need to create a new Java object.
	// Step one is getting the constructor for that class: <init> as the method name and void (V)
	char const* kClassName = "sml/ClientXML" ;
	jclass jsmlClass = jenv->FindClass(kClassName) ;

	if (!jsmlClass)
		return ;

	// We want to grab the SWIG constructor which in Java is:
	//  protected ClientXML(long cPtr, boolean cMemoryOwn) {
	//    swigCMemOwn = cMemoryOwn;
	//    swigCPtr = cPtr;
	//  }
	// J == long, Z == boolean so looking for constructor (long, boolean)
	jmethodID cons = jenv->GetMethodID(jsmlClass, "<init>", "(JZ)V") ;

	if (!cons)
		return ;

	// Call constructor with the address of the C++ object that is being wrapped
	// by the SWIG sml/ClientXML object.  To mimic the C++ semantics we'll create a new
	// SWIG (Java) object which does not own pXML (achieved by passing "false" as second param).
	// That way when the Java object is garbage collected it won't try to release pXML.
	// If a Java user wishes to keep pXML it will have to make a copy, just like in C++.
	// NOTE: Java long == C++ (long long) (i.e. 64-bit)
	// Trying to cast from a pointer to long long in a way that doesn't offend gcc.
	//long long javaPointer = reinterpret_cast<long long>(pXML) ;
	// NOTE: reinterpret_cast doesn't play nice on big-endian machines
	// This code emulates SWIG's method of dealing with this problem.
	jlong javaPointer;
	*(sml::ClientXML **)&javaPointer = pXML;
	jobject jNewObject = jenv->NewObject(jsmlClass, cons, javaPointer, false) ;

	if (!jNewObject)
	{
		return ;
	}

	// Make the method call.
	jenv->CallVoidMethod(jobj, mid, (int)id, pJavaData->m_CallbackData, pJavaData->m_AgentObject, jNewObject);
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT jint JNICALL Java_sml_smlJNI_Agent_1RegisterForXMLEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jobject jarg6)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlXMLEventId arg2 = (sml::smlXMLEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, jarg2, jarg3, jarg4, "xmlEventHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForXMLEvent(arg2, &XMLEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT bool JNICALL Java_sml_smlJNI_Agent_1UnregisterForXMLEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->UnregisterForXMLEvent(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
static void OutputEventHandler(void* pUserData, sml::Agent* pAgent, char const* pAttributeName, sml::WMElement* pWmeAdded)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv() ;

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	// Method sig here is:
	// Object userData, String agentName, String attributeName, WMElement* wmeAdded
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(Ljava/lang/Object;Ljava/lang/String;Ljava/lang/String;Lsml/WMElement;)V") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return ;
	}

	// Convert our C++ strings to Java strings
	jstring agentName = pAgent != NULL ? jenv->NewStringUTF(pAgent->GetAgentName()) : 0 ;
	jstring attributeName = pAttributeName != NULL ? jenv->NewStringUTF(pAttributeName) : 0 ;

	// Wrap our C++ WMElement object with a SWIG sml/WMElement Java object so we can
	// pass it back to Java.
	// OK, time to roll up our JNI sleeves and get dirty.  We need to create a new Java object.
	// Step one is getting the constructor for that class: <init> as the method name and void (V)
	char const* kClassName = "sml/WMElement" ;
	jclass jsmlClass = jenv->FindClass(kClassName) ;

	if (!jsmlClass)
		return ;

	// We want to grab the SWIG constructor which in Java is:
	//  protected WMElement(long cPtr, boolean cMemoryOwn) {
	//    swigCMemOwn = cMemoryOwn;
	//    swigCPtr = cPtr;
	//  }
	// J == long, Z == boolean so looking for constructor (long, boolean)
	jmethodID cons = jenv->GetMethodID(jsmlClass, "<init>", "(JZ)V") ;

	if (!cons)
		return ;

	// Call constructor with the address of the C++ object that is being wrapped
	// by the SWIG sml/WMElement object.  To mimic the C++ semantics we'll create a new
	// SWIG (Java) object which does not own pWmeAdded (achieved by passing "false" as second param).
	// That way when the Java object is garbage collected it won't try to release pWmeAdded.
	// If a Java user wishes to keep pWmeAdded it will have to make a copy, just like in C++.
	// NOTE: Java long == C++ (long long) (i.e. 64-bit)
	// Trying to cast from a pointer to long long in a way that doesn't offend gcc.
	//long long javaPointer = reinterpret_cast<long long>(pXML) ;
	// NOTE: reinterpret_cast doesn't play nice on big-endian machines
	// This code emulates SWIG's method of dealing with this problem.
	jlong javaPointer;
	*(sml::WMElement **)&javaPointer = pWmeAdded;
	jobject jNewObject = jenv->NewObject(jsmlClass, cons, javaPointer, false) ;

	if (!jNewObject)
	{
		return ;
	}

	// Make the method call.
	jenv->CallVoidMethod(jobj, mid, pJavaData->m_CallbackData, agentName, attributeName, jNewObject) ;
}

JNIEXPORT jint JNICALL Java_sml_smlJNI_Agent_1AddOutputHandler(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jobject jarg3, jobject jarg4, jobject jarg6)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// Get the attribute name from the Java string (jarg2 is the attribute name we're registering)
	const char *pAttributeName = jenv->GetStringUTFChars(jarg2, 0);

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, 0, jarg3, jarg4, "outputEventHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->AddOutputHandler(pAttributeName, &OutputEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT bool JNICALL Java_sml_smlJNI_Agent_1RemoveOutputHandler(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->RemoveOutputHandler(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
static void SystemEventHandler(sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv() ;

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(ILjava/lang/Object;Lsml/Kernel;)V") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return ;
	}

	// Make the method call.
	jenv->CallVoidMethod(jobj, mid, (int)id, pJavaData->m_CallbackData, pJavaData->m_KernelObject);
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT int JNICALL Java_sml_smlJNI_Kernel_1RegisterForSystemEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jobject jarg6)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlSystemEventId arg2 = (sml::smlSystemEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(false, jenv, jcls, jarg1, jarg2, jarg3, jarg4, "systemEventHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForSystemEvent(arg2, &SystemEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT bool JNICALL Java_sml_smlJNI_Kernel_1UnregisterForSystemEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->UnregisterForSystemEvent(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
static void UpdateEventHandler(sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv() ;

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(ILjava/lang/Object;Lsml/Kernel;I)V") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return ;
	}

	// Make the method call.
	jenv->CallVoidMethod(jobj, mid, (int)id, pJavaData->m_CallbackData, pJavaData->m_KernelObject, runFlags);
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT int JNICALL Java_sml_smlJNI_Kernel_1RegisterForUpdateEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jobject jarg6)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlUpdateEventId arg2 = (sml::smlUpdateEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(false, jenv, jcls, jarg1, jarg2, jarg3, jarg4, "updateEventHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForUpdateEvent(arg2, &UpdateEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT bool JNICALL Java_sml_smlJNI_Kernel_1UnregisterForUpdateEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->UnregisterForUpdateEvent(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
static void UntypedEventHandler(sml::smlUntypedEventId id, void* pUserData, sml::Kernel* pKernel, void* pData)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv() ;

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(ILjava/lang/Object;Lsml/Kernel;Ljava/lang/Object;)V") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return ;
	}

	// The type of data we pass back for this untyped handler is dependent on the event.
	// It will always be an Object but the caller is required to know the type.
	// This is an experiment to see if it reduces the kernel developer workload for adding new events.
	jobject data = 0 ;
	if (id == sml::smlEVENT_EDIT_PRODUCTION)
	{
		jstring prod = pData != NULL ? jenv->NewStringUTF((char const*)pData) : 0 ;
		data = prod ;
	}

	// Make the method call.
	jenv->CallVoidMethod(jobj, mid, (int)id, pJavaData->m_CallbackData, pJavaData->m_KernelObject, data);
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT int JNICALL Java_sml_smlJNI_Kernel_1RegisterForUntypedEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jobject jarg6)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlUntypedEventId arg2 = (sml::smlUntypedEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(false, jenv, jcls, jarg1, jarg2, jarg3, jarg4, "untypedEventHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForUntypedEvent(arg2, &UntypedEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT bool JNICALL Java_sml_smlJNI_Kernel_1UnregisterForUntypedEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->UnregisterForUntypedEvent(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}

// This is a bit ugly.  We compile this header with extern "C" around it so that the public methods can be
// exposed in a DLL with C naming (not C++ mangled names).  However, RhsEventHandler (below) returns a std::string
// which won't compile under "C"...even though it's a static function and hence won't appear in the DLL anyway.
// The solution is to turn off extern "C" for this method and turn it back on afterwards.
#ifdef __cplusplus
}
#endif

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
static std::string RhsEventHandler(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent,
							char const* pFunctionName, char const* pArgument)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv() ;

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return "Error -- failed to get Java class" ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	// Method sig here is:
	// Int eventID, Object userData, String agentName, String functionName, String argument returning a String.
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(ILjava/lang/Object;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return "Error -- failed to get Java method" ;
	}

	// Convert our C++ strings to Java strings
	jstring agentName = pAgent != NULL ? jenv->NewStringUTF(pAgent->GetAgentName()) : 0 ;
	jstring functionName = pFunctionName != NULL ? jenv->NewStringUTF(pFunctionName) : 0 ;
	jstring argument = pArgument != NULL ? jenv->NewStringUTF(pArgument) : 0 ;

	// Make the method call.
	jstring result = (jstring)jenv->CallObjectMethod(jobj, mid, (int)id, pJavaData->m_CallbackData, agentName, functionName, argument) ;

	// Get the returned string
	std::string resultStr = "" ;

	if (result != 0)
	{
		// Get the C string
		char const* pResult = jenv->GetStringUTFChars(result, 0);

		// Copy it into our std::string
		resultStr = pResult ;

		// Release the Java string
		jenv->ReleaseStringUTFChars(result, pResult);
	}

	// Return the result
	return resultStr ;
}

// This is a bit ugly.  We compile this header with extern "C" around it so that the public methods can be
// exposed in a DLL with C naming (not C++ mangled names).  However, RhsEventHandler (above) returns a std::string
// which won't compile under "C"...even though it's a static function and hence won't appear in the DLL anyway.
// The solution is to turn off extern "C" for this method and turn it back on afterwards.
#ifdef __cplusplus
extern "C" {
#endif

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT int JNICALL Java_sml_smlJNI_Kernel_1AddRhsFunction(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jobject jarg3, jobject jarg4, jobject jarg6)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// Get the function name from the Java string (jarg2 is the RHS function we're registering)
	const char *pFunctionName = jenv->GetStringUTFChars(jarg2, 0);

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(false, jenv, jcls, jarg1, 0, jarg3, jarg4, "rhsFunctionHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->AddRhsFunction(pFunctionName, &RhsEventHandler, pJavaData) ;

	// Release the string we got from Java
	jenv->ReleaseStringUTFChars(jarg2, pFunctionName);

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT bool JNICALL Java_sml_smlJNI_Kernel_1RemoveRhsFunction(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->RemoveRhsFunction(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}

// This is the C++ handler which will be called by clientSML when the event fires.
// Then from here we need to call back to Java to pass back the message.
// NOTE: For the Java method we'll make the event pass back the name of the agent, not an Agent* object
// otherwise we have the thorny problem of finding that Java Agent object (which we weren't passed and might need to create...)
static void AgentEventHandler(sml::smlAgentEventId id, void* pUserData, sml::Agent* pAgent)
{
	// The user data is the class we declared above, where we store the Java data to use in the callback.
	JavaCallbackData* pJavaData = (JavaCallbackData*)pUserData ;

	// Now try to call back to Java
	JNIEnv *jenv = pJavaData->GetEnv() ;

	// We start from the Java object whose method we wish to call.
	jobject jobj = pJavaData->m_HandlerObject ;
	jclass cls = jenv->GetObjectClass(jobj) ;

	if (cls == 0)
	{
		printf("Failed to get Java class\n") ;
		return ;
	}

	// Look up the Java method we want to call.
	// The method name is passed in by the user (and needs to match exactly, including case).
	// The method should be owned by the m_HandlerObject that the user also passed in.
	// Any slip here and you get a NoSuchMethod exception and my Java VM shuts down.
	jmethodID mid = jenv->GetMethodID(cls, pJavaData->m_HandlerMethod.c_str(), "(ILjava/lang/Object;Ljava/lang/String;)V") ;

	if (mid == 0)
	{
		printf("Failed to get Java method\n") ;
		return ;
	}

	// Convert our C++ strings to Java strings
	jstring agentName = pAgent != NULL ? jenv->NewStringUTF(pAgent->GetAgentName()) : 0 ;

	// Make the method call.
	jenv->CallVoidMethod(jobj, mid, (int)id, pJavaData->m_CallbackData, agentName);
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT int JNICALL Java_sml_smlJNI_Kernel_1RegisterForAgentEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jobject jarg6)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlAgentEventId arg2 = (sml::smlAgentEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, jarg2, jarg3, jarg4, "agentEventHandler", jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForAgentEvent(arg2, &AgentEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT bool JNICALL Java_sml_smlJNI_Kernel_1UnregisterForAgentEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2)
{
    // jarg1 is the C++ Agent object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg2 ;

	// Unregister our handler.
	bool result = arg1->UnregisterForAgentEvent(pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;

	return result ;
}
*/
