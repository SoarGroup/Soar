#include <portability.h>

#include "cliparsertest.h"
#include <cppunit/extensions/HelperMacros.h>

class CliParserTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE( CliParserTest );

    CPPUNIT_TEST( testEcho1 );
    CPPUNIT_TEST( testEcho2 );
    CPPUNIT_TEST( testEcho3 );
    CPPUNIT_TEST( testEcho4 );
    CPPUNIT_TEST( testEcho5 );
    CPPUNIT_TEST( testEcho6 );
    CPPUNIT_TEST( testEcho7 );

    CPPUNIT_TEST_SUITE_END();

public:
    CliParserTest() 
    {}
    virtual ~CliParserTest() {}

    void setUp();
    void tearDown();

protected:
    soar::tokenizer tok;
    cli::Parser* parser;

    CliEcho echo;

    void testEcho1();
    void testEcho2();
    void testEcho3();
    void testEcho4();
    void testEcho5();
    void testEcho6();
    void testEcho7();

    void testSimpleCommand();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CliParserTest );

void CliParserTest::setUp()
{
    parser = new cli::Parser();
    parser->AddCommand(new cli::EchoCommand(echo));

    tok.set_handler(parser);
}

void CliParserTest::tearDown()
{
    tok.set_handler(0);
    delete parser;
    parser = 0;
}

void CliParserTest::testEcho1()
{
    echo.SetExpected(1, true);
    CPPUNIT_ASSERT(tok.evaluate("echo"));
}

void CliParserTest::testEcho2()
{
    echo.SetExpected(1, false);
    CPPUNIT_ASSERT(tok.evaluate("echo -n"));
}

void CliParserTest::testEcho3()
{
    echo.SetExpected(2, true);
    CPPUNIT_ASSERT(tok.evaluate("echo stuff"));
}

void CliParserTest::testEcho4()
{
    echo.SetExpected(2, false);
    CPPUNIT_ASSERT(tok.evaluate("echo -n stuff"));
}

void CliParserTest::testEcho5()
{
    echo.SetExpected(2, false);
    CPPUNIT_ASSERT(tok.evaluate("echo stuff -n"));
}

void CliParserTest::testEcho6()
{
    echo.SetExpected(3, true);
    CPPUNIT_ASSERT(tok.evaluate(" echo other \"things\" "));
}

void CliParserTest::testEcho7()
{
    echo.SetExpected(3, false);
    CPPUNIT_ASSERT(tok.evaluate("echo {hmm -n} \"-n\" indeed"));
}
