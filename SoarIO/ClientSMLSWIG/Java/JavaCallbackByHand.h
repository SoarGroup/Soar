/////////////////////////////////////////////////////////////////
// Java callback support methods
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : December 2004
//
// Some handcoded methods to support registering callbacks for
// events through Java.
//
// We hope to eventually have SWIG generate these automatically.
//
// There is an example below this comment that shows
// how to register for an event in Java.  You don't need to do
// anything with that, just read it.
//
/////////////////////////////////////////////////////////////////

/*
User's java code sample:
public class EventListener
{
	public void runEventHandler(int eventID, Object data, Agent agent, int phase)
	{
		System.out.println("Received run event in Java") ;
	}

	public void agentEventHandler(int eventID, Object data, Agent agent)
	{
		System.out.println("Received agent event in Java") ;
	}

	public void productionEventHandler(int eventID, Object data, Agent agent, String prodName, String instantiation)
	{
		System.out.println("Received production event in Java") ;
	}

	public void kernelEventHandler(int eventID, Object data, Kernel kernel)
	{
		System.out.println("Received kernel event in Java") ;
	}
}

// The strings (e.g. "runEventHandler") must match the name of the method in the above class (case sensitive).
// An error in either the name or the type of the handler will lead to a NoSuchMethod exception when the event fires (not when it's registered).
EventListener listener = new EventListener() ;
int jRunCallback    = pAgent.RegisterForRunEvent(pAgent, smlEventId.smlEVENT_AFTER_DECISION_CYCLE, listener, "runEventHandler", this) ;		
int jAgentCallback  = pAgent.RegisterForAgentEvent(pAgent, smlEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, listener, "agentEventHandler", this) ;		
int jProdCallback   = pAgent.RegisterForProductionEvent(pAgent, smlEventId.smlEVENT_AFTER_PRODUCTION_FIRED, listener, "productionEventHandler", this) ;		
int jSystemCallback = pKernel.RegisterForSystemEvent(pKernel, smlEventId.smlEVENT_AFTER_RESTART, listener, "systemEventHandler", this) ;		

*/

// Notes on how to callback on a different thread
// The JNI interface pointer (JNIEnv) is valid only in the current thread.
// Should another thread need to access the Java VM, it must first call AttachCurrentThread() to attach itself to the VM and obtain a JNI interface pointer.
// Once attached to the VM, a native thread works just like an ordinary Java thread running inside a native method.
// The native thread remains attached to the VM until it calls DetachCurrentThread() to detach itself.
//
// jint GetJavaVM(JNIEnv *env, JavaVM **vm);
// Returns the Java VM interface (used in the Invocation API) associated with the current thread. The result is placed at the location pointed to by the second argument, vm. 
//
// jint AttachCurrentThread(JavaVM *vm, JNIEnv **p_env, void *thr_args);
// Attaches the current thread to a Java VM. Returns a JNI interface pointer in the JNIEnv argument.
// Trying to attach a thread that is already attached is a no-op.
// A native thread cannot be attached simultaneously to two Java VMs.
// When a thread is attached to the VM, the context class loader is the bootstrap loader. 

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

// Collect the Java values into a single object which we'll register with our local event handler.
// When this handler is called we'll unpack the Java data and make a callback to the Java process.
static JavaCallbackData* CreateJavaCallbackData(bool storeAgent, JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jstring jarg5, jobject jarg6)
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
	const char *pMethodName = jenv->GetStringUTFChars(jarg5, 0);

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
	jenv->ReleaseStringUTFChars(jarg5, pMethodName);

	return pJavaData ;
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT jint JNICALL Java_sml_smlJNI_Agent_1RegisterForRunEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jstring jarg5, jobject jarg6)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlRunEventId arg2 = (sml::smlRunEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, jarg2, jarg3, jarg4, jarg5, jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForRunEvent(arg2, &RunEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}


JNIEXPORT void JNICALL Java_sml_smlJNI_Agent_1UnregisterForRunEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jint jarg3)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlRunEventId arg2 = (sml::smlRunEventId)jarg2;

	// jarg3 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg3 ;

	// Unregister our handler.
	arg1->UnregisterForRunEvent(arg2, pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT int JNICALL Java_sml_smlJNI_Agent_1RegisterForProductionEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jstring jarg5, jobject jarg6)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlProductionEventId arg2 = (sml::smlProductionEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, jarg2, jarg3, jarg4, jarg5, jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForProductionEvent(arg2, &ProductionEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}


JNIEXPORT void JNICALL Java_sml_smlJNI_Agent_1UnregisterForProductionEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jint jarg3)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlProductionEventId arg2 = (sml::smlProductionEventId)jarg2;

	// jarg3 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg3 ;

	// Unregister our handler.
	arg1->UnregisterForProductionEvent(arg2, pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT jint JNICALL Java_sml_smlJNI_Agent_1RegisterForPrintEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jstring jarg5, jobject jarg6)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlPrintEventId arg2 = (sml::smlPrintEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, jarg2, jarg3, jarg4, jarg5, jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForPrintEvent(arg2, &PrintEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT void JNICALL Java_sml_smlJNI_Agent_1UnregisterForPrintEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jint jarg3)
{
    // jarg1 is the C++ Agent object
	sml::Agent *arg1 = *(sml::Agent **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlPrintEventId arg2 = (sml::smlPrintEventId)jarg2;

	// jarg3 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg3 ;

	// Unregister our handler.
	arg1->UnregisterForPrintEvent(arg2, pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT int JNICALL Java_sml_smlJNI_Kernel_1RegisterForSystemEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jstring jarg5, jobject jarg6)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlSystemEventId arg2 = (sml::smlSystemEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(false, jenv, jcls, jarg1, jarg2, jarg3, jarg4, jarg5, jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForSystemEvent(arg2, &SystemEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT void JNICALL Java_sml_smlJNI_Kernel_1UnregisterForSystemEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jint jarg3)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlSystemEventId arg2 = (sml::smlSystemEventId)jarg2;

	// jarg3 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg3 ;

	// Unregister our handler.
	arg1->UnregisterForSystemEvent(arg2, pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;
}

// This is the hand-written JNI method for registering a callback.
// I'm going to model it after the existing SWIG JNI methods so hopefully it'll be easier to patch this into SWIG eventually.
JNIEXPORT int JNICALL Java_sml_smlJNI_Kernel_1RegisterForAgentEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jobject jarg3, jobject jarg4, jstring jarg5, jobject jarg6)
{
    // jarg1 is the C++ Kernel object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlAgentEventId arg2 = (sml::smlAgentEventId)jarg2;

	// Create the information we'll need to make a Java call back later
	JavaCallbackData* pJavaData = CreateJavaCallbackData(true, jenv, jcls, jarg1, jarg2, jarg3, jarg4, jarg5, jarg6) ;
	
	// Register our handler.  When this is called we'll call back to the Java method.
	pJavaData->m_CallbackID = arg1->RegisterForAgentEvent(arg2, &AgentEventHandler, pJavaData) ;

	// Pass the callback info back to the Java client.  We need to do this so we can delete this later when the method is unregistered
	return (jint)pJavaData ;
}

JNIEXPORT void JNICALL Java_sml_smlJNI_Kernel_1UnregisterForAgentEvent(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jint jarg3)
{
    // jarg1 is the C++ Agent object
	sml::Kernel *arg1 = *(sml::Kernel **)&jarg1 ;

	// jarg2 is the event ID we're registering for
	sml::smlAgentEventId arg2 = (sml::smlAgentEventId)jarg2;

	// jarg3 is the callback data from the registration call
	JavaCallbackData* pJavaData = (JavaCallbackData*)jarg3 ;

	// Unregister our handler.
	arg1->UnregisterForAgentEvent(arg2, pJavaData->m_CallbackID) ;

	// Release the callback data
	delete pJavaData ;
}

