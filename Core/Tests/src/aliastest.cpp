#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include "cli_Aliases.h"

class AliasTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE( AliasTest );    // The name of this class

    CPPUNIT_TEST( testDefaults );

    CPPUNIT_TEST_SUITE_END();

public:
    AliasTest() 
    {}
    virtual ~AliasTest() {}

    void setUp();        // Called before each function outlined by CPPUNIT_TEST
    void tearDown();    // Called after each function outlined by CPPUNIT_TEST

protected:
    cli::Aliases* aliases;

    void testDefaults();
};

CPPUNIT_TEST_SUITE_REGISTRATION( AliasTest ); // Registers the test so it will be used

void AliasTest::setUp()
{
    aliases = new cli::Aliases();
}

void AliasTest::tearDown()
{
    delete aliases;
}

void AliasTest::testDefaults()
{
    // Test for some really common defaults
    std::vector< std::string > p;
    p.push_back("p");
    CPPUNIT_ASSERT(aliases->Expand(p));
    CPPUNIT_ASSERT(p.front() == "print");
}

