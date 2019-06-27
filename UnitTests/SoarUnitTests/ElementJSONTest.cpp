//
//  ElementJSONTest.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/26/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "ElementJSONTest.hpp"

#include <string>

#include "ElementJSON.h"

const std::string tag1("tag1");
const std::string att11("att11");
const std::string val11("val11");
const std::string att12("att12");
const std::string val12("val12");
const std::string data1("This is a string of data");
const std::string comment1("This is a comment that contains <>");

const std::string tag2("tag2");
const std::string att21("type");
const std::string val21("call me < & now");
const std::string data2("another string <s> of \"data\"");

const std::string tag3("tag3");

const std::string tag4("tag4");
const std::string att41("att41");
const std::string val41("longer value");
const std::string data4("some data");

const std::string tag5("tag5");

const int ElementJSONTest::BUFFER_LENGTH = 10;

void ElementJSONTest::setUp()
{
	agent = nullptr;
	
	for (int i = 0 ; i < BUFFER_LENGTH ; i++)
	{
		buffer[i] = static_cast< char >(i * 30);
	}
}

bool ElementJSONTest::verifyBuffer(const char* newBuffer) const
{
	for (int i = 0 ; i < BUFFER_LENGTH ; i++)
	{
		if (buffer[i] != newBuffer[i])
		{
			return false;
		}
	}
	return true;
}

void ElementJSONTest::tearDown(bool caught)
{
	for (int i = 0 ; i < BUFFER_LENGTH ; i++)
	{
		buffer[i] = static_cast< char >(0);
	}
}

soarjson::ElementJSON* ElementJSONTest::createJSON1()
{
	soarjson::ElementJSON* pJSON1 = new soarjson::ElementJSON();
	pJSON1->SetTagName(tag1.c_str());
	pJSON1->AddAttribute(att11.c_str(), val11.c_str());
	pJSON1->AddAttribute(att12.c_str(), val12.c_str());
	pJSON1->SetCharacterData(data1.c_str());
	return pJSON1;
}

soarjson::ElementJSON* ElementJSONTest::createJSON2()
{
	soarjson::ElementJSON* pJSON2 = new soarjson::ElementJSON();
	pJSON2->SetTagName(tag2.c_str());
	pJSON2->AddAttribute(att21.c_str(), val21.c_str());
	pJSON2->SetCharacterData(data2.c_str());
	return pJSON2;
}

soarjson::ElementJSON* ElementJSONTest::createJSON3()
{
	soarjson::ElementJSON* pJSON3 = new soarjson::ElementJSON();
	pJSON3->SetTagName(tag3.c_str());
	return pJSON3;
}

soarjson::ElementJSON* ElementJSONTest::createJSON4()
{
	soarjson::ElementJSON* pJSON4 = new soarjson::ElementJSON();
	pJSON4->SetTagName(tag4.c_str());
	pJSON4->AddAttribute(att41.c_str(), val41.c_str());
	pJSON4->SetCharacterData(data4.c_str());
	return pJSON4;
}

soarjson::ElementJSON* ElementJSONTest::createJSON5()
{
	soarjson::ElementJSON* pJSON5 = new soarjson::ElementJSON() ;
	pJSON5->SetTagName(tag5.c_str()) ;
	pJSON5->SetBinaryCharacterData(buffer, BUFFER_LENGTH) ;
	return pJSON5;
}

void ElementJSONTest::testSimple()
{
	soarjson::ElementJSON* pJSON1 = createJSON1();
	
	assertTrue(pJSON1->GetNumberAttributes() == 2);
	assertTrue(pJSON1->GetAttribute(att11.c_str()) != NULL);
	assertTrue(std::string(pJSON1->GetAttribute(att11.c_str())) == val11);
	assertTrue(pJSON1->GetAttribute(att12.c_str()) != NULL);
	assertTrue(std::string(pJSON1->GetAttribute(att12.c_str())) == val12);
	assertTrue(pJSON1->GetAttribute("not att") == NULL);
	assertTrue(pJSON1->GetTagName() != NULL);
	assertTrue(std::string(pJSON1->GetTagName()) == tag1);
	assertTrue(pJSON1->GetCharacterData() != NULL);
	assertTrue(std::string(pJSON1->GetCharacterData()) == data1);
	assertTrue(pJSON1->GetUseCData() == false);
	
	delete pJSON1;
}

void ElementJSONTest::testChildren()
{
	soarjson::ElementJSON* pJSON1 = createJSON1();
	soarjson::ElementJSON* pJSON2 = createJSON2();
	soarjson::ElementJSON* pJSON3 = createJSON3();
	soarjson::ElementJSON* pJSON4 = createJSON4();
	
	pJSON4->AddChild(pJSON1);
	pJSON4->AddChild(pJSON2);
	pJSON4->AddChild(pJSON3);
	
	assertTrue(pJSON4->GetNumberChildren() == 3);
	
	soarjson::ElementJSON child0(NULL) ;
	soarjson::ElementJSON const* pChild0 = &child0 ;
	
	assertTrue(pJSON4->GetChild(&child0, 0));
	assertTrue(pChild0->GetTagName() != NULL);
	assertTrue(std::string(pChild0->GetTagName()) == tag1);
	assertTrue(pChild0->GetCharacterData() != NULL);
	assertTrue(std::string(pChild0->GetCharacterData()) == data1);
	
	assertTrue(pChild0->GetNumberAttributes() == 2);
	assertTrue(pChild0->GetNumberChildren() == 0);
	
	// Let's put this one on the heap so we can control when we delete it.
	soarjson::ElementJSON* pChild1Object = new soarjson::ElementJSON(NULL);
	soarjson::ElementJSON const* pChild1 = pChild1Object;
	
	assertTrue(pJSON4->GetChild(pChild1Object, 1));
	assertTrue(pChild1->GetTagName() != NULL);
	assertTrue(std::string(pChild1->GetTagName()) == tag2);
	assertTrue(pChild1->GetCharacterData() != NULL);
	assertTrue(std::string(pChild1->GetCharacterData()) == data2);
	assertTrue(pChild0->GetNumberChildren() == 0);
	assertTrue(pChild1->GetAttribute(att21.c_str()) != NULL);
	assertTrue(std::string(pChild1->GetAttribute(att21.c_str())) == val21);
	
	//// This test is because I read online about looking up an element in an empty
	//// map causing an exception.  Need to make sure that doesn't happen in our
	//// attribute map implementation.
	soarjson::ElementJSON child2(NULL);
	soarjson::ElementJSON const* pChild2 = &child2;
	assertTrue(pJSON4->GetChild(&child2, 2));
	assertTrue(pChild2->GetTagName() != NULL);
	assertTrue(std::string(pChild2->GetTagName()) == tag3);
	assertTrue(pChild2->GetAttribute("missing") == NULL);
	
	soarjson::ElementJSON test;
	assertTrue(pJSON4->GetChild(&test, 3) == 0);
	assertTrue(pJSON4->GetChild(&test, -3) == 0);
	
	// Create an JSON string and print it out
	char* pStr = pJSON4->GenerateJSONString(true);
	//printf(pStr) ;
	//printf("\n") ;
	pJSON4->DeleteString(pStr);
	
	// Let's play a game.
	// Create another object pointing at the same internal handle
	soarjson::ElementJSON* pChild1Alt = new soarjson::ElementJSON(pChild1->GetJSONHandle());
	pChild1Alt->AddRefOnHandle();
	
	// Delete the entire tree, releasing refs on the children
	delete pJSON4;
	
	// We have to delete this other reference into the tree or its
	// not a proper test of pChild1Alt.  (If pChild1Object was on the stack
	// it wouldn't be deleted yet and of course we could talk to the child).
	delete pChild1Object;
	
	// Since we added a ref to pChild1 it should still exist
	assertTrue(pChild1Alt->GetTagName() != NULL);
	assertTrue(std::string(pChild1Alt->GetTagName()) == tag2);
	assertTrue(pChild1Alt->ReleaseRefOnHandle() == 0) ;
	
	delete pChild1Alt;
}

void ElementJSONTest::testParse()
{
	soarjson::ElementJSON* pJSON1 = createJSON1();
	
	char* pStr = pJSON1->GenerateJSONString(true);
	
	soarjson::ElementJSON* pJSON2 = soarjson::ElementJSON::ParseJSONFromString(pStr);
	
	assertTrue(pJSON2 != NULL) ;
	assertTrue(pJSON2->GetNumberAttributes() == 2) ;
	assertTrue(pJSON2->GetAttribute(att11.c_str()) != NULL);
	assertTrue(std::string(pJSON2->GetAttribute(att11.c_str())) == val11) ;
	assertTrue(pJSON2->GetAttribute(att12.c_str()) != NULL);
	assertTrue(std::string(pJSON2->GetAttribute(att12.c_str())) == val12) ;
	assertTrue(pJSON2->GetAttribute("not att") == NULL) ;
	assertTrue(pJSON2->GetTagName() != NULL);
	assertTrue(std::string(pJSON2->GetTagName()) == tag1) ;
	assertTrue(pJSON2->GetCharacterData() != NULL);
	assertTrue(std::string(pJSON2->GetCharacterData()) == data1) ;
	assertTrue(pJSON2->GetUseCData() == false) ;
	
	soarjson::ElementJSON::DeleteString(pStr) ;
	
	delete pJSON2;
	delete pJSON1;
}

void ElementJSONTest::testBinaryData()
{
	soarjson::ElementJSON* pJSON1 = createJSON1();
	soarjson::ElementJSON* pJSON2 = createJSON2();
	soarjson::ElementJSON* pJSON4 = createJSON4();
	soarjson::ElementJSON* pJSON5 = createJSON5();
	
	pJSON4->AddChild(pJSON1) ;
	pJSON4->AddChild(pJSON2) ;
	pJSON4->AddChild(pJSON5) ;
	
	char* pStr = pJSON4->GenerateJSONString(true) ;
	soarjson::ElementJSON* pParsedJSON = soarjson::ElementJSON::ParseJSONFromString(pStr) ;
	
	assertTrue(pParsedJSON != NULL) ;
	assertTrue(pParsedJSON->GetNumberChildren() == 3) ;
	
	soarjson::ElementJSON child0(NULL) ;
	soarjson::ElementJSON const* pChild0 = &child0 ;
	assertTrue(pParsedJSON->GetChild(&child0, 0));
	assertTrue(pChild0->GetTagName() != NULL);
	assertTrue(std::string(pChild0->GetTagName()) == tag1) ;
	assertTrue(pChild0->GetCharacterData() != NULL);
	assertTrue(std::string(pChild0->GetCharacterData()) == data1) ;
	assertTrue(pChild0->GetNumberAttributes() == 2) ;
	assertTrue(pChild0->GetNumberChildren() == 0) ;
	
	soarjson::ElementJSON child1(NULL) ;
	soarjson::ElementJSON const* pChild1 = &child1 ;
	assertTrue(pParsedJSON->GetChild(&child1, 1)) ;
	assertTrue(pChild1->GetTagName() != NULL);
	assertTrue(std::string(pChild1->GetTagName()) == tag2) ;
	assertTrue(pChild1->GetCharacterData() != NULL);
	assertTrue(std::string(pChild1->GetCharacterData()) == data2) ;
	assertTrue(pChild0->GetNumberChildren() == 0) ;
	assertTrue(pChild1->GetAttribute(att21.c_str()) != NULL);
	assertTrue(std::string(pChild1->GetAttribute(att21.c_str())) == val21) ;
	
	soarjson::ElementJSON child2(NULL) ;
	soarjson::ElementJSON const* pChild2 = &child2 ;
	assertTrue(pParsedJSON->GetChild(&child2, 2));
	assertTrue(pChild2->IsCharacterDataBinary()) ;
	
	const char* pBuffer = pChild2->GetCharacterData() ;
	int bufferLen = pChild2->GetCharacterDataLength() ;
	
	assertTrue(bufferLen == BUFFER_LENGTH) ;
	assertTrue(verifyBuffer(pBuffer)) ;
	
	soarjson::ElementJSON::DeleteString(pStr) ;
	
	delete pJSON4 ;
	delete pParsedJSON ;
}

void ElementJSONTest::testEquals()
{
	soarjson::ElementJSON* element = soarjson::ElementJSON::ParseJSONFromString("<sml><result>=</result></sml>");
	assertTrue_msg(soarjson::ElementJSON::GetLastParseErrorDescription(), element != 0);
	delete element;
}
