// SoarToolJavaInterface.cpp : Defines the entry point for the DLL application.
//

/* Java class native method header file */
// If this include fails--check that you have the JDK
// installed and that the project settings (C++ | Preprocessor | Additional Include Paths)
// for this project point to the jdk's include files.
#include "../../Source/MWJava Generated Stubs/threepenny/SoarToolJavaInterface.h"

/* STI interface */
#include "../STI_Interface/STI_CommonAPI.h"
#include "../STI_Interface/STI_Runtime.h"
#include "../STI_Interface/STI_Tool.h"

// Used to indicate a formal parameter is not used in the function.
// Clears a warning
#undef UNUSED
#define UNUSED(x) (void)(x)

// Windows specific pieces
#ifdef _WIN32
#include "stdafx.h"

// Include the STI library
#pragma comment(lib, "..\\STI_Interface\\libSTI1.lib")

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	UNUSED(hModule) ;
	UNUSED(ul_reason_for_call) ;
	UNUSED(lpReserved) ;

    return TRUE;
}
#endif

STI_Handle GetSTIHandleFromInt(jint hSTI)
{
	return (STI_Handle)hSTI;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_InitInterfaceLibrary
 * Signature: (Ljava/lang/String;Z)I
 */
JNIEXPORT jint JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1InitInterfaceLibrary
	(JNIEnv *pEnv, jobject, jstring sName, jboolean bIsRuntime)
{
	// Get the name string from Java
	const char* pszNameString = pEnv->GetStringUTFChars(sName, NULL);

	// Initialize the interface library
	STI_Handle hSTI = STI_InitInterfaceLibrary(pszNameString, bIsRuntime /* Is runtime */) ;
	
	// Release the name string
	pEnv->ReleaseStringUTFChars(sName, pszNameString);

	return (jint)hSTI;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_InitListenPort
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1InitListenPort
	(JNIEnv *, jobject, jint nSTI)
{
	return STI_InitListenPort(GetSTIHandleFromInt(nSTI));
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_EstablishConnections
 * Signature: (ILjava/lang/String;Z)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1EstablishConnections
	(JNIEnv *pEnv, jobject, jint nSTI, jstring sRemoteIPAddress, jboolean bStopOnFirstNotFound)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Get the IP address from Java.  If this is empty, that means local.
	const char* pszRemoteIPAddress = pEnv->GetStringUTFChars(sRemoteIPAddress, NULL);

	// Setup the string pointer that we pass into our establish connections call.
	// This will be null for local machines.
	const char* pRemoteAddress=pszRemoteIPAddress;
	if (pszRemoteIPAddress == NULL || strlen(pszRemoteIPAddress) < 1)
	{
		pRemoteAddress=NULL;
	}

	// Establish our connection
	bool bOk=STI_EstablishConnections(hSTI, pRemoteAddress, bStopOnFirstNotFound);

	// Release the IP address from Java
	pEnv->ReleaseStringUTFChars(sRemoteIPAddress, pszRemoteIPAddress);

	// Return the success bool to the caller
	return bOk;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_GetNumberConnections
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1GetNumberConnections
	(JNIEnv *pEnv, jobject, jint nSTI)
{
	UNUSED(pEnv) ;

	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Return the number of connections
	return STI_GetNumberConnections(hSTI);
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_GetConnectionName
 * Signature: (II)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1GetConnectionName
	(JNIEnv *pEnv, jobject, jint nSTI, jint nIndex)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Create the Jave connection string
	jstring connectionString = pEnv->NewStringUTF(STI_GetConnectionName(hSTI, nIndex));

	// Return the string to the caller
	return connectionString;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_EnableConnectionByIndex
 * Signature: (IIZ)V
 */
JNIEXPORT void JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1EnableConnectionByIndex
	(JNIEnv *pEnv, jobject, jint nSTI, jint nIndex, jboolean bEnable)
{
	UNUSED(pEnv) ;

	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Call STI to enable/disable the connection
	STI_EnableConnectionByIndex(hSTI, nIndex, bEnable);
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_EnableConnectionByName
 * Signature: (ILjava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1EnableConnectionByName
	(JNIEnv *pEnv, jobject, jint nSTI, jstring sConnectionName, jboolean bEnable)

{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Get the connection name string from Java
	const char* pszConnectionName = pEnv->GetStringUTFChars(sConnectionName, NULL);

	// Call STI to enable/disable the connection
	STI_EnableConnectionByName(hSTI, pszConnectionName, bEnable);

	// Release the connection name string from Java
	pEnv->ReleaseStringUTFChars(sConnectionName, pszConnectionName);
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_IsConnectionEnabledByIndex
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1IsConnectionEnabledByIndex
	(JNIEnv *pEnv, jobject, jint nSTI, jint nIndex)
{
	UNUSED(pEnv) ;

	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Call STI to see if our connection is enabled
	return STI_IsConnectionEnabledByIndex(hSTI, nIndex);
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_IsConnectionEnabledByName
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1IsConnectionEnabledByName
	(JNIEnv *pEnv, jobject, jint nSTI, jstring sConnectionName)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Get the connection name string from Java
	const char* pszConnectionName = pEnv->GetStringUTFChars(sConnectionName, NULL);

	// Call STI to see if our connection is enabled
	bool bEnabled=STI_IsConnectionEnabledByName(hSTI, pszConnectionName);

	// Release the connection name string from Java
	pEnv->ReleaseStringUTFChars(sConnectionName, pszConnectionName);

	// Return the status
	return bEnabled;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_TerminateInterfaceLibrary
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1TerminateInterfaceLibrary
	(JNIEnv *pEnv, jobject, jint nSTI)
{
	UNUSED(pEnv) ;

	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Terminate the interface library
	return STI_TerminateInterfaceLibrary(hSTI) ;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_PumpMessages
 * Signature: (IZ)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1PumpMessages
	(JNIEnv *, jobject, jint nSTI, jboolean bProcessAllPendingMessages)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	return STI_PumpMessages(hSTI, bProcessAllPendingMessages);
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_IsIncomingCommandAvailable
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1IsIncomingCommandAvailable
	(JNIEnv *, jobject, jint nSTI)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	return STI_IsIncomingCommandAvailable(hSTI);
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_GetIncomingCommand
 * Signature: (ILthreepenny/SoarToolJavaCommand;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1GetIncomingCommand
	(JNIEnv *pEnv, jobject, jint nSTI, jobject javaCommandObject)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Make sure that we have an incoming command available
	if (!STI_IsIncomingCommandAvailable(hSTI))
	{
		return false;
	}

	// Get our incoming command
	long commandID, commandFlags, dataSize, systemMsg ;
	long params[6] ;
	if (!STI_GetIncomingCommand1(hSTI, &commandID, &commandFlags, &dataSize, &systemMsg, &params[0],
									   &params[1], &params[2], &params[3], &params[4], &params[5]))
	{
		return false;
	}

	// Get our command object class
	jclass    objectClass = pEnv->GetObjectClass(javaCommandObject);
	
	// Note: We use ints here because long values in Java are __int64 in C.
	// So an "int" in Java is like a "long" in C

	// Set the commandID value
	{
		jmethodID methodID = pEnv->GetMethodID(objectClass, "SetCommandID", "(I)V");
		pEnv->CallVoidMethod (javaCommandObject, methodID, commandID);
	}

	// Set the commandFlags value
	{
		jmethodID methodID = pEnv->GetMethodID(objectClass, "SetCommandFlags", "(I)V");
		pEnv->CallVoidMethod (javaCommandObject, methodID, commandFlags);
	}

	// Set the dataSize value
	{
		jmethodID methodID = pEnv->GetMethodID(objectClass, "SetDataSize", "(I)V");
		pEnv->CallVoidMethod (javaCommandObject, methodID, dataSize);
	}

	// Set the systemMessage value
	{
		jmethodID methodID = pEnv->GetMethodID(objectClass, "SetSystemMessage", "(I)V");
		pEnv->CallVoidMethod (javaCommandObject, methodID, systemMsg);
	}

	// Set the integer parameters
	{
		// Get the method ID for the accessor function
		jmethodID methodID = pEnv->GetMethodID(objectClass, "SetIntegerParam", "(II)V");

		// Get the number of parameters
		int nNumParams=sizeof(params)/sizeof(params[0]);

		// Set each parameter
		int i=0;
		for (i=0; i < nNumParams; i++)
		{
			// Create our arguments that we will pass to the function
			jvalue args[2];
			args[0].i=i;			// Param index
			args[1].i=params[i];	// Param value

			pEnv->CallVoidMethodA (javaCommandObject, methodID, args);
		}
	}

	// Set the string parameter value if we have any
	{
		const char* pszStringParam=STI_GetIncomingCommandStringParam1(hSTI);
		if (pszStringParam)
		{
			jmethodID methodID = pEnv->GetMethodID(objectClass, "SetStringParam", "(Ljava/lang/String;)V");
			jstring ourString = pEnv->NewStringUTF(pszStringParam);
			pEnv->CallVoidMethod (javaCommandObject, methodID, ourString);		
		}
	}

	// Set the data buffer if we have one
	{
		// Get the data buffer
		char const* pDataBuffer=STI_GetIncomingCommandData(hSTI);
		if (pDataBuffer && dataSize > 0)
		{
			// Create a new Java byte array to hold the data
			jbyteArray newArray = pEnv->NewByteArray(dataSize);
			jbyte* pTemp = pEnv->GetByteArrayElements( newArray, NULL);
		
			// Copy the data buffer
			int i;
			for (i=0; i < dataSize; i++)
			{
				pTemp[i]=pDataBuffer[i];
			}

			// Release the array elements.  I think that this copies
			// the values into the Java array.
			pEnv->ReleaseByteArrayElements( newArray, pTemp, 0);

			// Set the newly created array in the Java class by calling
			// the accessor function.
			jmethodID methodID = pEnv->GetMethodID(objectClass, "SetDataBuffer", "([B)V");
			pEnv->CallVoidMethod(javaCommandObject, methodID, newArray);
		}
	}

	// Success!
	return true;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_PopIncomingCommand
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1PopIncomingCommand
	(JNIEnv *, jobject, jint nSTI)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	STI_PopIncomingCommand(hSTI);
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendCommand
 * Signature: (IZLthreepenny/SoarToolJavaCommand;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendCommand
	(JNIEnv *pEnv, jobject, jint nSTI, jboolean bSynch, jobject javaCommandObject)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Get our command object class
	jclass    objectClass = pEnv->GetObjectClass(javaCommandObject);
	
	// Note: We use ints here because long values in Java are __int64 in C.
	// So an "int" in Java is like a "long" in C

	// These are the values that will be filled in by calling into the Java class
	long commandID, commandFlags, dataSize, systemMsg ;
	long params[6] ;
	char* pStringParam1=NULL;
	char* pDataBuffer=NULL;

	// Get the commandID value
	{
		jmethodID methodID = pEnv->GetMethodID(objectClass, "GetCommandID", "()I");
		commandID=pEnv->CallIntMethod (javaCommandObject, methodID);
	}

	// Get the commandFlags value
	{
		jmethodID methodID = pEnv->GetMethodID(objectClass, "GetCommandFlags", "()I");
		commandFlags=pEnv->CallIntMethod (javaCommandObject, methodID);
	}

	// Get the dataSize value
	{
		jmethodID methodID = pEnv->GetMethodID(objectClass, "GetDataSize", "()I");
		dataSize=pEnv->CallIntMethod (javaCommandObject, methodID);
	}

	// Get the systemMessage value
	{
		jmethodID methodID = pEnv->GetMethodID(objectClass, "GetSystemMessage", "()I");
		systemMsg=pEnv->CallIntMethod (javaCommandObject, methodID);
	}

	// Get the integer parameters
	{
		// Get the method ID for the accessor function
		jmethodID methodID = pEnv->GetMethodID(objectClass, "GetIntegerParam", "(I)I");

		// Get the number of parameters
		int nNumParams=sizeof(params)/sizeof(params[0]);

		// Get each parameter
		int i=0;
		for (i=0; i < nNumParams; i++)
		{
			params[i]=pEnv->CallIntMethod (javaCommandObject, methodID, i);
		}
	}

	// Get the string parameter value
	{
		jmethodID methodID = pEnv->GetMethodID(objectClass, "GetStringParam", "()Ljava/lang/String;");
		jobject stringObject = pEnv->CallObjectMethod(javaCommandObject, methodID);
		const char* pszString=pEnv->GetStringUTFChars((jstring)stringObject, NULL);

		// Copy the Java string to our local string
		if (pszString)
		{
			pStringParam1=new char[strlen(pszString)+1];
			strcpy(pStringParam1, pszString);
		}
		pEnv->ReleaseStringUTFChars((jstring)stringObject, pszString);
	}

	// Get the data buffer if we have one
	{
		if (dataSize > 0)
		{
			// Get the data array
			jmethodID methodID = pEnv->GetMethodID(objectClass, "GetDataBuffer", "()[B");
			jobject dataObject = pEnv->CallObjectMethod(javaCommandObject, methodID);

			// Get the data array elements
			jbyte* pTemp = pEnv->GetByteArrayElements((jbyteArray)dataObject, NULL);
		
			// Allocate our data buffer
			pDataBuffer=new char[dataSize];

			// Copy the data buffer
			int i;
			for (i=0; i < dataSize; i++)
			{
				pDataBuffer[i]=pTemp[i];
			}

			// Release the array elements.  I think that this copies
			// the values back into the Java array.  This is only
			// a performance problem since we don't actually
			// change the values here.
			pEnv->ReleaseByteArrayElements((jbyteArray)dataObject, pTemp, 0);
		}
	}

	bool bOk;
	if (bSynch)
	{
		bOk=STI_SendCommandSynch1(hSTI, commandID, commandFlags, dataSize,
								  params[0], params[1], params[2], params[3], params[4], params[5],
								  pStringParam1, pDataBuffer);
	}
	else
	{
		bOk=STI_SendCommandAsynch1(hSTI, commandID, commandFlags, dataSize,
								   params[0], params[1], params[2], params[3], params[4], params[5],
								   pStringParam1, pDataBuffer);
	}

	// Delete our string
	if (pStringParam1)
	{
		delete pStringParam1;
		pStringParam1=NULL;
	}

	// Delete our data buffer
	if (pDataBuffer)
	{
		delete pDataBuffer;
		pDataBuffer=NULL;
	}

	// Return the result
	return bOk;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendProduction
 * Signature: (ILjava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendProduction
	(JNIEnv *pEnv, jobject, jint nSTI, jstring sProductionName, jstring sProductionBody)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Get the production name and body strings from Java
	const char* pszProductionName = pEnv->GetStringUTFChars(sProductionName, NULL);
	const char* pszProductionBody = pEnv->GetStringUTFChars(sProductionBody, NULL);
	
	// Send the "Send production" command
	bool bOk = STI_SendProduction(hSTI, pszProductionName, pszProductionBody);

	// Release the Java strings
	pEnv->ReleaseStringUTFChars(sProductionName, pszProductionName);
	pEnv->ReleaseStringUTFChars(sProductionBody, pszProductionBody);

	// Return the result
	return bOk;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendFile
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendFile
	(JNIEnv *pEnv, jobject, jint nSTI, jstring sFilename)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Get the filename string from Java
	const char* pszFilename = pEnv->GetStringUTFChars(sFilename, NULL);

	// Send the file to the runtime
	bool bOk = STI_SendFile(hSTI, pszFilename);

	// Release the Java string
	pEnv->ReleaseStringUTFChars(sFilename, pszFilename);

	// Return the result
	return bOk;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendProductionMatches
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendProductionMatches
	(JNIEnv *pEnv, jobject, jint nSTI, jstring sProductionName)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Get the production name string from Java
	const char* pszProductionName = pEnv->GetStringUTFChars(sProductionName, NULL);

	// Send the production matches command to the runtime
	bool bOk = STI_ProductionMatches(hSTI, pszProductionName);

	// Release the Java string
	pEnv->ReleaseStringUTFChars(sProductionName, pszProductionName);

	// Return the result
	return bOk;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendExciseProduction
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendExciseProduction
	(JNIEnv *pEnv, jobject, jint nSTI, jstring sProductionName)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Get the production name string from Java
	const char* pszProductionName = pEnv->GetStringUTFChars(sProductionName, NULL);

	// Send the production matches command to the runtime
	bool bOk = STI_ExciseProduction(hSTI, pszProductionName);

	// Release the Java string
	pEnv->ReleaseStringUTFChars(sProductionName, pszProductionName);

	// Return the result
	return bOk;
}

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendRawCommand
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendRawCommand
	(JNIEnv *pEnv, jobject, jint nSTI, jstring sCommandString)
{
	// Get our STI handle
	STI_Handle hSTI=GetSTIHandleFromInt(nSTI);

	// Get the raw command string from Java
	const char* pszCommandString = pEnv->GetStringUTFChars(sCommandString, NULL);

	// Send the raw command to the runtime
	bool bOk = STI_SendRawCommand(hSTI, pszCommandString);

	// Release the Java string
	pEnv->ReleaseStringUTFChars(sCommandString, pszCommandString);

	// Return the result
	return bOk;
}
