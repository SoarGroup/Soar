// TestConnectionSML.cpp : Defines the entry point for the console application.
//

#include "sml_Connection.h"
#include "sml_ElementXML.h"
#include "sml_MessageSML.h"
#include "sml_EmbeddedConnection.h"
#include "sml_EmbeddedConnectionAsynch.h"
#include "sml_EmbeddedConnectionSynch.h"
#include "sml_AnalyzeXML.h"
#include "EmbeddedSMLInterface.h"
#include "sml_XMLTrace.h"

#include <stdlib.h>
#include <iostream>

#ifdef _MSC_VER
// Use Visual C++'s memory checking functionality
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _MSC_VER

using namespace sml ;

class Test
{
protected:
	char const* m_pTestName ;

public:
	Test(char const* pTestName)
	{
		m_pTestName = pTestName ;
	}

	bool Check(bool condition, char const* pMsg)
	{
		if (!condition)
			printf("Test %s failed with error: %s\n", m_pTestName, pMsg) ;

		return condition ;
	}

	bool CheckStringEq(char const* pStr1, char const* pStr2, char const* pMsg)
	{
		if (!pStr1)
		{
			printf("Test %s failed with error: %s because first arg was null\n", m_pTestName, pMsg) ;
			return false ;
		}

		if (!pStr2)
		{
			printf("Test %s failed with error: %s because second arg was null\n", m_pTestName, pMsg) ;
			return false ;
		}

		if (strcmp(pStr1, pStr2) != 0)
		{
			printf("Test %s failed with error: %s because str1 was:%s and str2 was:%s\n", m_pTestName, pMsg, pStr1, pStr2) ;
			return false ;
		}

		return true ;
	}

	bool Result(bool ok)
	{
		if (ok)
			printf("Test %s passed\n", m_pTestName) ;
		else
			printf("Test %s failed\n", m_pTestName) ;

		return ok ;
	}

	virtual bool Run() = 0 ;

} ;

class TestElementXML_1 : public Test
{
public:
	TestElementXML_1() : Test("ElementXML_1") { }

	bool Run()
	{
		ElementXML* pXML = new ElementXML() ;

		pXML->SetTagName("test1") ;
		pXML->AddAttribute("att1", "val1") ;
		pXML->AddAttribute("att2", "val2") ;
		pXML->SetCharacterData("This is a string of data") ;
		pXML->SetComment("This is a comment") ;

		bool ok = true ;

		ok = ok && Check(pXML->GetNumberAttributes() == 2, "Atts == 2") ;
		ok = ok && CheckStringEq(pXML->GetAttribute("att1"), "val1", "Val1") ;
		ok = ok && CheckStringEq(pXML->GetAttribute("att2"), "val2", "Val2") ;
		ok = ok && Check(pXML->GetAttribute("not att") == NULL, "missing att") ;
		ok = ok && CheckStringEq(pXML->GetTagName(), "test1", "tag name") ;
		ok = ok && CheckStringEq(pXML->GetCharacterData(), "This is a string of data", "char data") ;
		ok = ok && Check(pXML->GetUseCData() == false, "CData") ;
		ok = ok && CheckStringEq(pXML->GetComment(), "This is a comment", "comment") ;

		delete pXML ;

		return Result(ok) ;
	}
} ;

class TestElementXML_2 : public Test
{
public:
	TestElementXML_2() : Test("ElementXML_2") { }

	bool Run()
	{
		ElementXML* pXML = new ElementXML() ;

		pXML->SetTagName("test1") ;
		pXML->AddAttribute("att1", "val1") ;
		pXML->AddAttribute("att2", "val2") ;
		pXML->SetCharacterData("This is a string of data") ;

		ElementXML* pXML2 = new ElementXML() ;
		pXML2->SetTagName("test2") ;
		pXML2->AddAttribute("type", "call me now") ;
		pXML2->SetCharacterData("another string of data") ;

		ElementXML* pXML3 = new ElementXML() ;
		pXML3->SetTagName("test3") ;

		ElementXML* pTop = new ElementXML() ;
		pTop->SetTagName("top") ;
		pTop->AddAttribute("att", "longer value") ;
		pTop->SetCharacterData("some data") ;
		pTop->AddChild(pXML) ;
		pTop->AddChild(pXML2) ;
		pTop->AddChild(pXML3) ;

		bool ok = true ;

		ok = ok && Check(pTop->GetNumberChildren() == 3, "nChildren") ;
		
		ElementXML child0(NULL) ;
		ElementXML const* pChild0 = &child0 ;
		ok = ok && pTop->GetChild(&child0, 0) ;
		ok = ok && CheckStringEq(pChild0->GetTagName(), "test1", "test1") ;
		ok = ok && CheckStringEq(pChild0->GetCharacterData(), "This is a string of data", "data1") ;
		ok = ok && Check(pChild0->GetNumberAttributes() == 2, "atts1") ;
		ok = ok && Check(pChild0->GetNumberChildren() == 0, "children1") ;

		// Let's put this one on the heap so we can control when we delete it.
		ElementXML* pChild1Object = new ElementXML(NULL) ;
		ElementXML const* pChild1 = pChild1Object ;
		ok = ok && pTop->GetChild(pChild1Object, 1) ;
		ok = ok && CheckStringEq(pChild1->GetTagName(), "test2", "test2") ;
		ok = ok && CheckStringEq(pChild1->GetCharacterData(), "another string of data", "data2") ;
		ok = ok && Check(pChild0->GetNumberChildren() == 0, "children2") ;
		ok = ok && CheckStringEq(pChild1->GetAttribute("type"), "call me now", "val2") ;

		// This test is because I read online about looking up an element in an empty
		// map causing an exception.  Need to make sure that doesn't happen in our
		// attribute map implementation.
		ElementXML child2(NULL) ;
		ElementXML const* pChild2 = &child2 ;
		ok = ok && pTop->GetChild(&child2, 2) ;
		ok = ok && CheckStringEq(pChild2->GetTagName(), "test3", "test3") ;
		ok = ok && Check(pChild2->GetAttribute("missing") == NULL, "missing3") ;

		ElementXML test ;
		ok = ok && Check(pTop->GetChild(&test, 3)  == 0, "missing child +3") ;
		ok = ok && Check(pTop->GetChild(&test, -3) == 0, "missing child -3") ;

		// Create an XML string and print it out
		char* pStr = pTop->GenerateXMLString(true) ;
		printf(pStr) ;
		printf("\n") ;
		pXML->DeleteString(pStr) ;

		// Let's play a game.
		// Create another object pointing at the same internal handle
		ElementXML* pChild1Alt = new ElementXML(pChild1->GetXMLHandle()) ;
		pChild1Alt->AddRefOnHandle() ;

		// Delete the entire tree, releasing refs on the children
		delete pTop ;

		// We have to delete this other reference into the tree or its
		// not a proper test of pChild1Alt.  (If pChild1Object was on the stack
		// it wouldn't be deleted yet and of course we could talk to the child).
		delete pChild1Object ;
	
		// Since we added a ref to pChild1 it should still exist
		ok = ok && CheckStringEq(pChild1Alt->GetTagName(), "test2", "test2") ;
		ok = ok && Check(pChild1Alt->ReleaseRefOnHandle() == 0, "Deleting child1") ;

		delete pChild1Alt ;

		return Result(ok) ;
	}
} ;

class TestElementXML_3 : public Test
{
public:
	TestElementXML_3() : Test("ElementXML_3") { }

	bool Run()
	{
		ElementXML* pOrigXML = new ElementXML() ;

		pOrigXML->SetTagName("test1") ;
		pOrigXML->AddAttribute("att1", "val1") ;
		pOrigXML->AddAttribute("att2", "val2") ;
		pOrigXML->SetCharacterData("This is a string of data") ;
		pOrigXML->SetComment("This is a comment that contains <>") ;

		bool ok = true ;

		char* pStr = pOrigXML->GenerateXMLString(true) ;

		ElementXML* pXML = ElementXML::ParseXMLFromString(pStr) ;

		ok = ok && Check(pXML != NULL, "Parse failed") ;
		ok = ok && Check(pXML->GetNumberAttributes() == 2, "Atts == 2") ;
		ok = ok && CheckStringEq(pXML->GetAttribute("att1"), "val1", "Val1") ;
		ok = ok && CheckStringEq(pXML->GetAttribute("att2"), "val2", "Val2") ;
		ok = ok && Check(pXML->GetAttribute("not att") == NULL, "missing att") ;
		ok = ok && CheckStringEq(pXML->GetTagName(), "test1", "tag name") ;
		ok = ok && CheckStringEq(pXML->GetCharacterData(), "This is a string of data", "char data") ;
		ok = ok && Check(pXML->GetUseCData() == false, "CData") ;
		ok = ok && CheckStringEq(pXML->GetComment(), "This is a comment that contains <>", "comment") ;

		ElementXML::DeleteString(pStr) ;
		delete pXML ;
		delete pOrigXML ;

		return Result(ok) ;
	}
} ;

class TestElementXML_4 : public Test
{
public:
	TestElementXML_4() : Test("ElementXML_4") { }

	bool Run()
	{
		ElementXML* pXML = new ElementXML() ;

		pXML->SetTagName("test1") ;
		pXML->AddAttribute("att1", "val1") ;
		pXML->AddAttribute("att2", "val2") ;
		pXML->SetCharacterData("This is a string of data") ;

		ElementXML* pXML2 = new ElementXML() ;
		pXML2->SetTagName("test2") ;
		pXML2->AddAttribute("type", "call me < & now") ;
		pXML2->SetCharacterData("another string <s> of \"data\"") ;

		// A binary buffer of data
		char buffer[10] ;
		int len = 10 ;

		for (int i = 0 ; i < len ; i++)
		{
			buffer[i] = (char)i * 30 ;
		}

		ElementXML* pXML3 = new ElementXML() ;
		pXML3->SetTagName("test3") ;
		pXML3->SetBinaryCharacterData(buffer, len) ;

		ElementXML* pTop = new ElementXML() ;
		pTop->SetTagName("top") ;
		pTop->AddAttribute("att", "longer value") ;
		pTop->SetCharacterData("some data") ;
		pTop->AddChild(pXML) ;
		pTop->AddChild(pXML2) ;
		pTop->AddChild(pXML3) ;

		char* pStr = pTop->GenerateXMLString(true) ;
		ElementXML* pParsedXML = ElementXML::ParseXMLFromString(pStr) ;

		bool ok = true ;

		ok = ok && Check(pParsedXML != NULL, "Parse failed") ;
		ok = ok && Check(pParsedXML->GetNumberChildren() == 3, "nChildren") ;
		
		ElementXML child0(NULL) ;
		ElementXML const* pChild0 = &child0 ;
		ok = ok && pParsedXML->GetChild(&child0, 0) ;
		ok = ok && CheckStringEq(pChild0->GetTagName(), "test1", "test1") ;
		ok = ok && CheckStringEq(pChild0->GetCharacterData(), "This is a string of data", "data1") ;
		ok = ok && Check(pChild0->GetNumberAttributes() == 2, "atts1") ;
		ok = ok && Check(pChild0->GetNumberChildren() == 0, "children1") ;

		ElementXML child1(NULL) ;
		ElementXML const* pChild1 = &child1 ;
		ok = ok && pParsedXML->GetChild(&child1, 1) ;
		ok = ok && CheckStringEq(pChild1->GetTagName(), "test2", "test2") ;
		ok = ok && CheckStringEq(pChild1->GetCharacterData(), "another string <s> of \"data\"", "data2") ;
		ok = ok && Check(pChild0->GetNumberChildren() == 0, "children2") ;
		ok = ok && CheckStringEq(pChild1->GetAttribute("type"), "call me < & now", "val2") ;

		ElementXML child2(NULL) ;
		ElementXML const* pChild2 = &child2 ;
		ok = ok && pParsedXML->GetChild(&child2, 2) ;
		ok = ok && Check(pChild2->IsCharacterDataBinary(), "Char data is not returned as binary") ;

		char const* pBuffer = pChild2->GetCharacterData() ;
		int bufferLen = pChild2->GetCharacterDataLength() ;

		ok = ok && Check(bufferLen == len, "Binary data is wrong length") ;

		bool same = true ;
		for (int j = 0 ; j < len ; j++)
		{
			char ch = pBuffer[j] ;
			char ch1 = buffer[j] ;
			if (ch != ch1)
				same = false ;
		}

		ok = ok && Check(same, "Binary data is not the same") ;

		ElementXML::DeleteString(pStr) ;
		delete pTop ;
		delete pParsedXML ;

		return Result(ok) ;
	}
} ;


class TestTraceXML_1 : public Test
{
public:
	TestTraceXML_1() : Test("TraceXML_1") { }

	bool Run()
	{
		XMLTrace* pXML = new XMLTrace() ;

		pXML->BeginTag("test") ;
		pXML->AddAttribute("att1", "val1") ;
		pXML->AddAttribute("att2", "val2") ;
		pXML->EndTag("test") ;

		pXML->BeginTag("test2") ;
		pXML->AddAttribute("att3", "val3") ;
		pXML->AddAttribute("att4", "val4") ;

		pXML->BeginTag("test3") ;
		pXML->AddAttribute("att-sub1", "val-sub1") ;
		pXML->AddAttribute("att-sub2", "val-sub2") ;
		pXML->EndTag("test3") ;

		pXML->EndTag("test2") ;

		bool ok = true ;

		ElementXML* pDetach1 = new ElementXML(pXML->Detach()) ;
		char* pStr1 = pDetach1->GenerateXMLString(true) ;

		// Check that we can reuse this object
		pXML->Reset() ;

		pXML->BeginTag("test") ;
		pXML->AddAttribute("att1", "val1") ;
		pXML->AddAttribute("att2", "val2") ;
		pXML->EndTag("test") ;

		pXML->BeginTag("test2") ;
		pXML->AddAttribute("att3", "val3") ;
		pXML->AddAttribute("att4", "val4") ;

		pXML->BeginTag("test3") ;
		pXML->AddAttribute("att-sub1", "val-sub1") ;
		pXML->AddAttribute("att-sub2", "val-sub2") ;
		pXML->EndTag("test3") ;

		pXML->EndTag("test2") ;

		ElementXML* pDetach2 = new ElementXML(pXML->Detach()) ;
		char* pStr2 = pDetach2->GenerateXMLString(true) ;

		// Check that the two strings match
		ok = ok && (strcmp(pStr1, pStr2) == 0) ;

		ElementXML::DeleteString(pStr1) ;
		ElementXML::DeleteString(pStr2) ;

		delete pXML ;
		delete pDetach1 ;
		delete pDetach2 ;

		return Result(ok) ;
	}
} ;

class TestConnection_1 : public Test
{
public:
	TestConnection_1() : Test("Connection_1") { }
	TestConnection_1(char const* pName) : Test(pName) { }

	static ElementXML_Handle ProcessMessage(Connection_Receiver_Handle hReceiverConnection, ElementXML_Handle hIncomingMsg, int action)
	{
		if (action == SML_MESSAGE_ACTION_SYNCH)
		{
			Connection* pConnection = (Connection*)hReceiverConnection ;
			ElementXML msg(hIncomingMsg) ;

			ElementXML* pResponse = pConnection->InvokeCallbacks(&msg) ;

			if (!pResponse)
				return NULL ;

			ElementXML_Handle hResponse = pResponse->Detach() ;
			delete pResponse ;
			return hResponse ;
		}

		return NULL ;
	}

	static ElementXML* Notify1(Connection* pConnection, ElementXML* pIncoming, void* pUserData)
	{
		// Switch from a static callback into one local to the sending object
		TestConnection_1* pMe = (TestConnection_1*)pUserData ;
		pMe->GotNotified(pConnection, pIncoming) ;

		// Since this is a notification, we always return NULL
		return NULL ;
	}

	ElementXML* GotNotified(Connection* pConnection, ElementXML* pIncoming)
	{
		return NULL ;
	}

	static ElementXML* Call2(Connection* pConnection, ElementXML* pIncoming, void* pUserData)
	{
		// Switch from a static callback into one local to the sending object
		TestConnection_1* pMe = (TestConnection_1*)pUserData ;
		return pMe->GotCalled(pConnection, pIncoming) ;
	}

	ElementXML* GotCalled(Connection* pConnection, ElementXML* pIncoming)
	{
		ElementXML* pXML = new ElementXML() ;

		pXML->SetTagName("result") ;
		pXML->AddAttribute("ack", "1") ;
		pXML->AddAttribute("doctype", "response") ;

		return pXML ;
	}

	bool Run()
	{
		ElementXML* pXML1 = new ElementXML() ;

		pXML1->SetTagName("test1") ;
		pXML1->AddAttribute("id", "1") ;
		pXML1->AddAttribute("doctype", "call") ;

		ElementXML* pXML2 = new ElementXML() ;

		pXML2->SetTagName("test2") ;
		pXML2->AddAttribute("doctype", "notify") ;

		ErrorCode error = 0 ;
		// A normal client would use Connection::CreateEmbeddedConnection() which always links the caller
		// to the Soar kernel.  For this test we'll link two arbitrary classes together, so we build the objects directly.

		// First connection sends commands and listens to notifications
		Connection* pConnection1 = EmbeddedConnectionSynch::CreateEmbeddedConnectionSynch() ;
		pConnection1->RegisterCallback(Notify1, this, sml_Names::kDocType_Notify, true) ;

		// Second connection receives commands (and sends responses and notifications).
		Connection* pConnection2 = EmbeddedConnectionSynch::CreateEmbeddedConnectionSynch() ;
		pConnection2->RegisterCallback(Call2, this, sml_Names::kDocType_Call, true) ;

		// Connect the two together
		((EmbeddedConnectionSynch*)pConnection1)->AttachConnectionInternal((Connection_Receiver_Handle)(pConnection2), ProcessMessage) ;
		((EmbeddedConnectionSynch*)pConnection2)->AttachConnectionInternal((Connection_Receiver_Handle)(pConnection1), ProcessMessage) ;

		// Send a "call" over.
		pConnection1->SendMessage(pXML1) ;
		error = pConnection1->GetLastError() ;

		if (error != Error::kNoError)
			printf("Error: %s\n", Error::GetErrorDescription(error)) ;

		// Get the "response" to the "call".
		ElementXML* pResult = pConnection1->GetResponse(pXML1, true) ;
		error = pConnection1->GetLastError() ;

		if (error != Error::kNoError)
			printf("Error: %s\n", Error::GetErrorDescription(error)) ;

		bool ok = true ;

		ok = ok && Check(pResult != NULL, "Error getting response") ;
		ok = ok && CheckStringEq(pResult->GetTagName(), "result", "Problem with response") ;

		// Send a "notify" message.
		pConnection2->SendMessage(pXML2) ;
		pConnection2->GetLastError() ;

		if (error != Error::kNoError)
			printf("Error: %s\n", Error::GetErrorDescription(error)) ;

		delete pXML1 ;
		delete pXML2 ;
		if (pResult) delete pResult ;

		delete pConnection1 ;
		delete pConnection2 ;

		return Result(ok) ;
	}
} ;
/*
class TestConnection_2 : public TestConnection_1
{
public:
	TestConnection_2() : TestConnection_1("TestConnection_2")
	{
	}

	bool Run()
	{
		ElementXML* pXML1 = new ElementXML() ;

		pXML1->SetTagName("sml") ;
		pXML1->AddAttribute("id", "1") ;
		pXML1->AddAttribute("doctype", "call") ;

		ElementXML* pXML2 = new ElementXML() ;

		pXML2->SetTagName("test2") ;
		pXML2->AddAttribute("doctype", "notify") ;

		ErrorCode error = 0 ;
		bool ok = true ;

		// Create an embedded connection to the kernel.
		Connection* pConnection1 = Connection::CreateEmbeddedConnection("SoarKernelSML", &error) ;

		ok = ok && Check(error == Error::kNoError, Error::GetErrorDescription(error)) ;

		// If we fail to create a connection, abort the test
		if (!ok)
			return ok ;

		// Register for notifications from the kernel.
		pConnection1->RegisterCallback(Notify1, this, sml_Names::kDocType_Notify, true) ;

		// Send a "call" over.
		pConnection1->SendMessage(pXML1) ;
		error = pConnection1->GetLastError() ;

		ok = ok && Check(error == Error::kNoError, Error::GetErrorDescription(error)) ;

		// Get the "response" to the "call".
		ElementXML* pResult = pConnection1->GetResponse(pXML1, true) ;
		error = pConnection1->GetLastError() ;

		ok = ok && Check(error == Error::kNoError, Error::GetErrorDescription(error)) ;
		ok = ok && Check(pResult != NULL, "Error getting response") ;
		ok = ok && CheckStringEq(pResult->GetTagName(), "sml", "Problem with response") ;

		delete pXML1 ;
		delete pXML2 ;
		if (pResult) delete pResult ;

		pConnection1->CloseConnection() ;

		delete pConnection1 ;

		return Result(ok) ;
	}

} ;

class TestConnection_3 : public TestConnection_1
{
public:
	TestConnection_3() : TestConnection_1("TestConnection_3")
	{
	}

	bool Run()
	{
		ErrorCode error = 0 ;
		bool ok = true ;

		// Create an embedded connection to the kernel.
		Connection* pConnection1 = Connection::CreateEmbeddedConnection("SoarKernelSML", &error) ;

		ok = ok && Check(error == Error::kNoError, Error::GetErrorDescription(error)) ;

		// If we fail to create a connection, abort the test
		if (!ok)
			return ok ;

		// Register for notifications from the kernel.
		pConnection1->RegisterCallback(Notify1, this, sml_Names::kDocType_Notify, true) ;

		ElementXML* pMsg = pConnection1->CreateSMLCommand("print") ;
		pConnection1->AddParameterToSMLCommand(pMsg, "wme", "s1") ;

		char* pMsgXML = pMsg->GenerateXMLString(true) ;

		// Analyze the message we're about to send, to check we can
		// find args etc.
		AnalyzeXML* analysis = new AnalyzeXML() ;
		analysis->Analyze(pMsg) ;

		ElementXML const* pCommand = analysis->GetCommandTag() ;
		ok = ok && Check(pCommand != NULL, "Failed to find command tag in analysis") ;
		ok = ok && Check(pCommand->IsTag("command"), "Tag is not the command tag") ;

		char const* pWME           = analysis->GetArgValue("wme") ;
		ok = ok && CheckStringEq(pWME, "s1", "Problem with analysis of XML") ;
		delete analysis ;

		ElementXML::DeleteString(pMsgXML) ;

		// Send a "call" over.
		pConnection1->SendMessage(pMsg) ;
		error = pConnection1->GetLastError() ;

		ok = ok && Check(error == Error::kNoError, Error::GetErrorDescription(error)) ;

		// Get the "response" to the "call".
		ElementXML* pResult = pConnection1->GetResponse(pMsg, true) ;
		error = pConnection1->GetLastError() ;

		ok = ok && Check(error == Error::kNoError, Error::GetErrorDescription(error)) ;
		ok = ok && Check(pResult != NULL, "Error getting response") ;
		ok = ok && CheckStringEq(pResult->GetTagName(), "sml", "Problem with response") ;

		delete pMsg ;
		if (pResult) delete pResult ;

		pConnection1->CloseConnection() ;

		delete pConnection1 ;

		return Result(ok) ;
	}

} ;

class TestKernel_1 : public TestConnection_1
{
public:
	TestKernel_1() : TestConnection_1("TestKernel_1")
	{
	}

	bool Run()
	{
		ErrorCode error = 0 ;
		bool ok = true ;

		// Create an embedded connection to the kernel.
		Connection* pConnection1 = Connection::CreateEmbeddedConnection("SoarKernelSML", &error) ;

		ok = ok && Check(error == Error::kNoError, Error::GetErrorDescription(error)) ;

		// If we fail to create a connection, abort the test
		if (!ok)
			return ok ;

		ElementXML* pMsg = pConnection1->CreateSMLCommand(sml_Names::kgSKI_CreateKernelFactory) ;

		// Useful for debugging this test
		char* pMsgXML = pMsg->GenerateXMLString(true) ;
		ElementXML::DeleteString(pMsgXML) ;

		// Send the create factory call over
		pConnection1->SendMessage(pMsg) ;

		// Get the "response" to the "call".
		ElementXML* pResult = pConnection1->GetResponse(pMsg) ;
		error = pConnection1->GetLastError() ;

		AnalyzeXML res ;
		res.Analyze(pResult) ;

		ok = ok && Check(res.GetResultString() != NULL, "No result to CreateKernelFactory") ;

		if (!ok)
			return ok ;

		// Get the id of the kernel factory.
		std::string factory = res.GetResultString() ;

		ElementXML* pMsg1 = pConnection1->CreateSMLCommand(sml_Names::kgSKI_IKernelFactory_Create) ;
		pConnection1->AddParameterToSMLCommand(pMsg1, sml_Names::kParamThis, factory.c_str()) ;

		// Now create the kernel object
		pConnection1->SendMessage(pMsg1) ;
		ElementXML* pResult2 = pConnection1->GetResponse(pMsg1) ;

		AnalyzeXML res2 ;
		res2.Analyze(pResult2) ;

		ok = ok && Check(res2.GetResultString() != NULL, "No result to IKernelFactory::Create") ;

		if (!ok)
			return ok ;

		// Get the id of the kernel object
		std::string kernel = res2.GetResultString() ;

		ElementXML* pMsg2 = pConnection1->CreateSMLCommand(sml_Names::kgSKI_IKernelFactory_DestroyKernel) ;
		pConnection1->AddParameterToSMLCommand(pMsg2, sml_Names::kParamThis, factory.c_str()) ;
		pConnection1->AddParameterToSMLCommand(pMsg2, sml_Names::kParamKernel, kernel.c_str()) ;

		// Now release the kernel object
		pConnection1->SendMessage(pMsg2) ;
		ElementXML* pResult3 = pConnection1->GetResponse(pMsg2) ;

		// And release the factory
		ElementXML* pMsg3 = pConnection1->CreateSMLCommand(sml_Names::kgSKI_IRelease_Release) ;
		pConnection1->AddParameterToSMLCommand(pMsg3, sml_Names::kParamThis, factory.c_str()) ;
		pConnection1->SendMessage(pMsg3) ;

		delete pMsg3 ;
		delete pMsg2 ;
		delete pMsg1 ;
		delete pMsg ;
		if (pResult) delete pResult ;
		if (pResult2) delete pResult2 ;
		if (pResult3) delete pResult3 ;


		pConnection1->CloseConnection() ;

		delete pConnection1 ;

		return Result(ok) ;
	}

} ;
*/
int main(/*int argc, char* argv[]*/)
{
	// Start off with some general tests of ElementXML
	TestElementXML_1 test1 ;
	TestElementXML_2 test2 ;
	TestElementXML_3 test2b ;
	TestElementXML_4 test2c ;
	TestConnection_1 test3 ;
	TestTraceXML_1 testTrace1 ;
	//TestConnection_2 test4 ;
	//TestConnection_3 test5 ;
	//TestKernel_1	 test6 ;

	bool ok = true ;

	ok = ok && test1.Run() ;
	ok = ok && test2.Run() ;
	ok = ok && test2b.Run() ;
	ok = ok && test2c.Run() ;
	ok = ok && test3.Run() ;
	ok = ok && testTrace1.Run() ;
	//ok = ok && test4.Run() ;
	//ok = ok && test5.Run() ;
	//ok = ok && test6.Run() ;

	if (ok)
		printf("\n\nAll tests passed\n") ;
	else
		printf("\n\n*** Error: At least one test failed.  Stopped testing at that point.\n") ;

#ifdef _MSC_VER
//	A deliberate memory leak which I can use to test the memory checking code is working.
//	char* pTest = new char[10] ;

	printf("\nNow checking memory.  Any leaks will appear below.\nNothing indicates no leaks detected.\n") ;
	printf("\nIf no leaks appear here, but some appear in the output\nwindow in the debugger, they have been leaked from a DLL.\nWhich is reporting when it's unloaded.\n\n") ;

	// Set the memory checking output to go to Visual Studio's debug window (so we have a copy to keep)
	// and to stdout so we can see it immediately.
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	// Now check for memory leaks.
	// This will only detect leaks in objects that we allocate within this executable and static libs.
	// If we allocate something in a DLL then this call won't see it because it works by overriding the
	// local implementation of malloc.
	_CrtDumpMemoryLeaks();

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <return> to exit\n") ;
	char line[100] ;
	char* str = gets(line) ;
#endif // _MSC_VER

	return 0;
}

