/*
How to create a new test suite in a few minutes:
	1) create cpp file for your new test. The class name and file name 
	   should end with the string 'test'. Note: header files are not
	   required.
	2) Include HelperMacros.h
	3) Name the class and inherit TestCase
	4) Declare suite with name of class using macro CPPUNIT_TEST_SUITE
	5) Declare end of suite with macro CPPUNIT_TEST_SUITE_END
	6) Declare public void setUp(void) and void tearDown(void)
	7) Declare protected tests, usually void testSomething(void)
	8) Call macro CPPUNIT_TEST between macros CPPUNIT_TEST_SUITE and 
	   CPPUNIT_TEST_SUITE_END with the name of your test functions
	   (such as testSomething), many tests can be declared, see
	   existing tests for examples.
	9) After your class is defined, call CPPUNIT_TEST_SUITE_REGISTRATION
	   with your class name.
    10) Define your tests, use CPPUNIT_ASSERT and CPPUNIT_ASSERT_MESSAGE

There are many more options and details. See online resources:
	http://cppunit.sourceforge.net/cppunit-wiki
	http://cppunit.sourceforge.net/doc/lastest/cppunit_cookbook.html

 */

#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

class SanityTest : public CPPUNIT_NS::TestCase
{
	CPPUNIT_TEST_SUITE( SanityTest );	// The name of this class

	CPPUNIT_TEST( testSanity );

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();		// Called before each function outlined by CPPUNIT_TEST
	void tearDown();	// Called after each function outlined by CPPUNIT_TEST

protected:
	void testSanity();

	static bool up;
};

CPPUNIT_TEST_SUITE_REGISTRATION( SanityTest ); // Registers the test so it will be used

bool SanityTest::up = false;

void SanityTest::setUp()
{
	CPPUNIT_ASSERT( !up );
	up = true;
}

void SanityTest::tearDown()
{
	CPPUNIT_ASSERT( up );
	up = false;
}

void SanityTest::testSanity()
{
	CPPUNIT_ASSERT( up );
	// TODO: platform independent version of this:
	//CPPUNIT_ASSERT_MESSAGE( "Set working directory to SoarLibrary/bin", cwdEndsWith == "SoarLibrary/bin" );
}
