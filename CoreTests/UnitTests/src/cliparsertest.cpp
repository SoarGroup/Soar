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

    CPPUNIT_TEST( testMaxDCTime1 );
    CPPUNIT_TEST( testMaxDCTime2 );
    CPPUNIT_TEST( testMaxDCTime3 );
    CPPUNIT_TEST( testMaxDCTime4 );
    CPPUNIT_TEST( testMaxDCTime5 );
    CPPUNIT_TEST( testMaxDCTime6 );
    CPPUNIT_TEST( testMaxDCTime7 );
    CPPUNIT_TEST( testMaxDCTime8 );
    CPPUNIT_TEST( testMaxDCTime9 );
    CPPUNIT_TEST( testMaxDCTime10 );
    CPPUNIT_TEST( testMaxDCTime11 );
    CPPUNIT_TEST( testMaxDCTime12 );

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

    CliMaxDCTime maxdctime;

    void testMaxDCTime1();
    void testMaxDCTime2();
    void testMaxDCTime3();
    void testMaxDCTime4();
    void testMaxDCTime5();
    void testMaxDCTime6();
    void testMaxDCTime7();
    void testMaxDCTime8();
    void testMaxDCTime9();
    void testMaxDCTime10();
    void testMaxDCTime11();
    void testMaxDCTime12();

    void testSimpleCommand();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CliParserTest );

void CliParserTest::setUp()
{
    parser = new cli::Parser();

    parser->AddCommand(new cli::EchoCommand(echo));
    parser->AddCommand(new cli::MaxDCTimeCommand(maxdctime));

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

void CliParserTest::testMaxDCTime1()
{
    maxdctime.SetExpected(0);
    CPPUNIT_ASSERT(tok.evaluate("max-dc-time"));
}

void CliParserTest::testMaxDCTime2()
{
    CPPUNIT_ASSERT(!tok.evaluate("max-dc-time -1"));
}

void CliParserTest::testMaxDCTime3()
{
    CPPUNIT_ASSERT(!tok.evaluate("max-dc-time 0"));
}

void CliParserTest::testMaxDCTime4()
{
    maxdctime.SetExpected(1);
    CPPUNIT_ASSERT(tok.evaluate("max-dc-time 1"));
}

void CliParserTest::testMaxDCTime5()
{
    maxdctime.SetExpected(-1);
    CPPUNIT_ASSERT(tok.evaluate("max-dc-time -d"));
}

void CliParserTest::testMaxDCTime6()
{
    maxdctime.SetExpected(-1);
    CPPUNIT_ASSERT(tok.evaluate("max-dc-time -o"));
}

void CliParserTest::testMaxDCTime7()
{
    CPPUNIT_ASSERT(!tok.evaluate("max-dc-time -d 0"));
}

void CliParserTest::testMaxDCTime8()
{
    CPPUNIT_ASSERT(!tok.evaluate("max-dc-time -d 1"));
}

void CliParserTest::testMaxDCTime9()
{
    maxdctime.SetExpected(1000000);
    CPPUNIT_ASSERT(tok.evaluate("max-dc-time -s 1"));
}

void CliParserTest::testMaxDCTime10()
{
    maxdctime.SetExpected(500000);
    CPPUNIT_ASSERT(tok.evaluate("max-dc-time -s 0.5"));
}

void CliParserTest::testMaxDCTime11()
{
    maxdctime.SetExpected(500000);
    CPPUNIT_ASSERT(tok.evaluate("max-dc-time 0.5 -s"));
}

void CliParserTest::testMaxDCTime12()
{
    CPPUNIT_ASSERT(!tok.evaluate("max-dc-time 0.5"));
}
