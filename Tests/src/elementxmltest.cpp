#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>

namespace sml
{
	class ElementXML;
}

class ElementXMLTest : public CPPUNIT_NS::TestCase
{
	CPPUNIT_TEST_SUITE( ElementXMLTest );

	CPPUNIT_TEST( testSimple );
	CPPUNIT_TEST( testChildren );
	CPPUNIT_TEST( testParse );
	CPPUNIT_TEST( testBinaryData );

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

protected:
	void testSimple();
	void testChildren();
	void testParse();
	void testBinaryData();

private:
	sml::ElementXML* createXML1();
	sml::ElementXML* createXML2();
	sml::ElementXML* createXML3();
	sml::ElementXML* createXML4();
	sml::ElementXML* createXML5();

	bool verifyBuffer( const char* buffer ) const;

	static const int BUFFER_LENGTH;
	char buffer[10] ;
};

CPPUNIT_TEST_SUITE_REGISTRATION( ElementXMLTest );

#include <string>

#include <sml_ElementXML.h>

const std::string tag1( "tag1" );
const std::string att11( "att11" );
const std::string val11( "val11" );
const std::string att12( "att12" );
const std::string val12( "val12" );
const std::string data1( "This is a string of data" );
const std::string comment1( "This is a comment that contains <>" );

const std::string tag2( "tag2" );
const std::string att21( "type" );
const std::string val21( "call me < & now" );
const std::string data2( "another string <s> of \"data\"" );

const std::string tag3( "tag3" );

const std::string tag4( "tag4" );
const std::string att41( "att41" );
const std::string val41( "longer value" );
const std::string data4( "some data" );

const std::string tag5( "tag5" );

const int ElementXMLTest::BUFFER_LENGTH = 10;

void ElementXMLTest::setUp()
{
	for (int i = 0 ; i < BUFFER_LENGTH ; i++)
	{
		buffer[i] = static_cast< char >( i * 30 );
	}
}

bool ElementXMLTest::verifyBuffer( const char* newBuffer ) const
{
	for (int i = 0 ; i < BUFFER_LENGTH ; i++)
	{
		if ( buffer[i] != newBuffer[i] ) return false;
	}
	return true;
}

void ElementXMLTest::tearDown()
{
	for (int i = 0 ; i < BUFFER_LENGTH ; i++)
	{
		buffer[i] = static_cast< char >( 0 );
	}
}

sml::ElementXML* ElementXMLTest::createXML1()
{
	sml::ElementXML* pXML1 = new sml::ElementXML();
	pXML1->SetTagName( tag1.c_str() );
	pXML1->AddAttribute( att11.c_str(), val11.c_str() );
	pXML1->AddAttribute( att12.c_str(), val12.c_str() );
	pXML1->SetCharacterData( data1.c_str() );
	pXML1->SetComment( comment1.c_str() );
	return pXML1;
}

sml::ElementXML* ElementXMLTest::createXML2()
{
	sml::ElementXML* pXML2 = new sml::ElementXML();
	pXML2->SetTagName( tag2.c_str() );
	pXML2->AddAttribute( att21.c_str(), val21.c_str() );
	pXML2->SetCharacterData( data2.c_str() );
	return pXML2;
}

sml::ElementXML* ElementXMLTest::createXML3()
{
	sml::ElementXML* pXML3 = new sml::ElementXML();
	pXML3->SetTagName( tag3.c_str() );
	return pXML3;
}

sml::ElementXML* ElementXMLTest::createXML4()
{
	sml::ElementXML* pXML4 = new sml::ElementXML();
	pXML4->SetTagName( tag4.c_str() );
	pXML4->AddAttribute( att41.c_str(), val41.c_str() );
	pXML4->SetCharacterData( data4.c_str() );
	return pXML4;
}

sml::ElementXML* ElementXMLTest::createXML5()
{
	sml::ElementXML* pXML5 = new sml::ElementXML() ;
	pXML5->SetTagName( tag5.c_str() ) ;
	pXML5->SetBinaryCharacterData( buffer, BUFFER_LENGTH ) ;
	return pXML5;
}

void ElementXMLTest::testSimple()
{
	sml::ElementXML* pXML1 = createXML1();

	CPPUNIT_ASSERT( pXML1->GetNumberAttributes() == 2 );
	CPPUNIT_ASSERT( pXML1->GetAttribute( att11.c_str() ) != NULL );
	CPPUNIT_ASSERT( std::string( pXML1->GetAttribute( att11.c_str() ) ) == val11 );
	CPPUNIT_ASSERT( pXML1->GetAttribute( att12.c_str() ) != NULL );
	CPPUNIT_ASSERT( std::string( pXML1->GetAttribute( att12.c_str() ) ) == val12 );
	CPPUNIT_ASSERT( pXML1->GetAttribute( "not att" ) == NULL );
	CPPUNIT_ASSERT( pXML1->GetTagName() != NULL );
	CPPUNIT_ASSERT( std::string( pXML1->GetTagName() ) == tag1 );
	CPPUNIT_ASSERT( pXML1->GetCharacterData() != NULL );
	CPPUNIT_ASSERT( std::string( pXML1->GetCharacterData() ) == data1 );
	CPPUNIT_ASSERT( pXML1->GetUseCData() == false );
	CPPUNIT_ASSERT( pXML1->GetComment() != NULL );
	CPPUNIT_ASSERT( std::string( pXML1->GetComment() ) == comment1 );

	delete pXML1;
}

void ElementXMLTest::testChildren()
{
	sml::ElementXML* pXML1 = createXML1();
	sml::ElementXML* pXML2 = createXML2();
	sml::ElementXML* pXML3 = createXML3();
	sml::ElementXML* pXML4 = createXML4();

	pXML4->AddChild( pXML1 );
	pXML4->AddChild( pXML2 );
	pXML4->AddChild( pXML3 );

	CPPUNIT_ASSERT( pXML4->GetNumberChildren() == 3 );

	sml::ElementXML child0( NULL ) ;
	sml::ElementXML const* pChild0 = &child0 ;

	CPPUNIT_ASSERT( pXML4->GetChild( &child0, 0 ) );
	CPPUNIT_ASSERT( pChild0->GetTagName() != NULL );
	CPPUNIT_ASSERT( std::string( pChild0->GetTagName() ) == tag1 );
	CPPUNIT_ASSERT( pChild0->GetCharacterData() != NULL );
	CPPUNIT_ASSERT( std::string( pChild0->GetCharacterData() ) == data1 );

	CPPUNIT_ASSERT( pChild0->GetNumberAttributes() == 2 );
	CPPUNIT_ASSERT( pChild0->GetNumberChildren() == 0 );

	// Let's put this one on the heap so we can control when we delete it.
	sml::ElementXML* pChild1Object = new sml::ElementXML( NULL );
	sml::ElementXML const* pChild1 = pChild1Object;

	CPPUNIT_ASSERT( pXML4->GetChild( pChild1Object, 1 ) );
	CPPUNIT_ASSERT( pChild1->GetTagName() != NULL );
	CPPUNIT_ASSERT( std::string( pChild1->GetTagName() ) == tag2 );
	CPPUNIT_ASSERT( pChild1->GetCharacterData() != NULL );
	CPPUNIT_ASSERT( std::string( pChild1->GetCharacterData() ) == data2 );
	CPPUNIT_ASSERT( pChild0->GetNumberChildren() == 0 );
	CPPUNIT_ASSERT( pChild1->GetAttribute( att21.c_str() ) != NULL );
	CPPUNIT_ASSERT( std::string( pChild1->GetAttribute( att21.c_str() ) ) == val21 );

	//// This test is because I read online about looking up an element in an empty
	//// map causing an exception.  Need to make sure that doesn't happen in our
	//// attribute map implementation.
	sml::ElementXML child2(NULL);
	sml::ElementXML const* pChild2 = &child2;
	CPPUNIT_ASSERT ( pXML4->GetChild( &child2, 2 ) );
	CPPUNIT_ASSERT( pChild2->GetTagName() != NULL );
	CPPUNIT_ASSERT( std::string( pChild2->GetTagName() ) == tag3 );
	CPPUNIT_ASSERT( pChild2->GetAttribute( "missing" ) == NULL );

	sml::ElementXML test;
	CPPUNIT_ASSERT( pXML4->GetChild( &test, 3 ) == 0 );
	CPPUNIT_ASSERT( pXML4->GetChild( &test, -3 ) == 0 );

	// Create an XML string and print it out
	char* pStr = pXML4->GenerateXMLString( true );
	//printf(pStr) ;
	//printf("\n") ;
	pXML4->DeleteString( pStr );

	// Let's play a game.
	// Create another object pointing at the same internal handle
	sml::ElementXML* pChild1Alt = new sml::ElementXML( pChild1->GetXMLHandle() );
	pChild1Alt->AddRefOnHandle();

	// Delete the entire tree, releasing refs on the children
	delete pXML4;

	// We have to delete this other reference into the tree or its
	// not a proper test of pChild1Alt.  (If pChild1Object was on the stack
	// it wouldn't be deleted yet and of course we could talk to the child).
	delete pChild1Object;

	// Since we added a ref to pChild1 it should still exist
	CPPUNIT_ASSERT( pChild1Alt->GetTagName() != NULL );
	CPPUNIT_ASSERT( std::string( pChild1Alt->GetTagName() ) == tag2 );
	CPPUNIT_ASSERT( pChild1Alt->ReleaseRefOnHandle() == 0 ) ;

	delete pChild1Alt;
}

void ElementXMLTest::testParse()
{
	sml::ElementXML* pXML1 = createXML1();

	char* pStr = pXML1->GenerateXMLString( true );

	sml::ElementXML* pXML2 = sml::ElementXML::ParseXMLFromString( pStr );

	CPPUNIT_ASSERT( pXML2 != NULL ) ;
	CPPUNIT_ASSERT( pXML2->GetNumberAttributes() == 2 ) ;
	CPPUNIT_ASSERT( pXML2->GetAttribute( att11.c_str() ) != NULL );
	CPPUNIT_ASSERT( std::string( pXML2->GetAttribute( att11.c_str() ) ) == val11 ) ;
	CPPUNIT_ASSERT( pXML2->GetAttribute( att12.c_str() ) != NULL );
	CPPUNIT_ASSERT( std::string( pXML2->GetAttribute( att12.c_str() ) ) == val12 ) ;
	CPPUNIT_ASSERT( pXML2->GetAttribute("not att") == NULL ) ;
	CPPUNIT_ASSERT( pXML2->GetTagName() != NULL );
	CPPUNIT_ASSERT( std::string( pXML2->GetTagName() ) == tag1 ) ;
	CPPUNIT_ASSERT( pXML2->GetCharacterData() != NULL );
	CPPUNIT_ASSERT( std::string( pXML2->GetCharacterData() ) == data1 ) ;
	CPPUNIT_ASSERT( pXML2->GetUseCData() == false ) ;
	CPPUNIT_ASSERT( pXML2->GetComment() != NULL );
	CPPUNIT_ASSERT( std::string( pXML2->GetComment() ) == comment1 ) ;

	sml::ElementXML::DeleteString(pStr) ;

	delete pXML2;
	delete pXML1;
}

void ElementXMLTest::testBinaryData()
{
	sml::ElementXML* pXML1 = createXML1();
	sml::ElementXML* pXML2 = createXML2();
	sml::ElementXML* pXML4 = createXML4();
	sml::ElementXML* pXML5 = createXML5();

	pXML4->AddChild( pXML1 ) ;
	pXML4->AddChild( pXML2 ) ;
	pXML4->AddChild( pXML5 ) ;

	char* pStr = pXML4->GenerateXMLString( true ) ;
	sml::ElementXML* pParsedXML = sml::ElementXML::ParseXMLFromString( pStr ) ;

	CPPUNIT_ASSERT( pParsedXML != NULL ) ;
	CPPUNIT_ASSERT( pParsedXML->GetNumberChildren() == 3 ) ;

	sml::ElementXML child0(NULL) ;
	sml::ElementXML const* pChild0 = &child0 ;
	CPPUNIT_ASSERT( pParsedXML->GetChild(&child0, 0) );
	CPPUNIT_ASSERT( pChild0->GetTagName() != NULL );
	CPPUNIT_ASSERT( std::string( pChild0->GetTagName() ) == tag1 ) ;
	CPPUNIT_ASSERT( pChild0->GetCharacterData() != NULL );
	CPPUNIT_ASSERT( std::string( pChild0->GetCharacterData() ) == data1 ) ;
	CPPUNIT_ASSERT( pChild0->GetNumberAttributes() == 2 ) ;
	CPPUNIT_ASSERT( pChild0->GetNumberChildren() == 0 ) ;

	sml::ElementXML child1(NULL) ;
	sml::ElementXML const* pChild1 = &child1 ;
	CPPUNIT_ASSERT( pParsedXML->GetChild(&child1, 1) ) ;
	CPPUNIT_ASSERT( pChild1->GetTagName() != NULL );
	CPPUNIT_ASSERT( std::string( pChild1->GetTagName() ) == tag2 ) ;
	CPPUNIT_ASSERT( pChild1->GetCharacterData() != NULL );
	CPPUNIT_ASSERT( std::string( pChild1->GetCharacterData() ) == data2 ) ;
	CPPUNIT_ASSERT( pChild0->GetNumberChildren() == 0 ) ;
	CPPUNIT_ASSERT( pChild1->GetAttribute( att21.c_str() ) != NULL );
	CPPUNIT_ASSERT( std::string( pChild1->GetAttribute( att21.c_str() ) ) == val21 ) ;

	sml::ElementXML child2(NULL) ;
	sml::ElementXML const* pChild2 = &child2 ;
	CPPUNIT_ASSERT( pParsedXML->GetChild(&child2, 2) );
	CPPUNIT_ASSERT( pChild2->IsCharacterDataBinary() ) ;

	const char* pBuffer = pChild2->GetCharacterData() ;
	int bufferLen = pChild2->GetCharacterDataLength() ;

	CPPUNIT_ASSERT( bufferLen == BUFFER_LENGTH ) ;
	CPPUNIT_ASSERT( verifyBuffer( pBuffer ) ) ;

	sml::ElementXML::DeleteString(pStr) ;

	delete pXML4 ;
	delete pParsedXML ;
}