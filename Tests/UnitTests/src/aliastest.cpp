#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include "cli_Aliases.h"
#include "cli_Parser.h"

class AliasTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE( AliasTest );    // The name of this class

    CPPUNIT_TEST( testOne );
    CPPUNIT_TEST( testTwo );
    CPPUNIT_TEST( testThree );
    CPPUNIT_TEST( testRemove );
    CPPUNIT_TEST( testDefaults );

    CPPUNIT_TEST( testSimpleCommand );

    CPPUNIT_TEST_SUITE_END();

public:
    AliasTest() 
    {}
    virtual ~AliasTest() {}

    void setUp();        // Called before each function outlined by CPPUNIT_TEST
    void tearDown();    // Called after each function outlined by CPPUNIT_TEST

protected:
    cli::Aliases* aliases;

    void testOne();
    void testTwo();
    void testThree();
    void testRemove();
    void testDefaults();

    void testSimpleCommand();
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

void AliasTest::testOne()
{
    std::vector< std::string > a;
    a.push_back("alias");
    a.push_back("1");
    aliases->SetAlias(a);
    std::vector< std::string > b;
    b.push_back("alias");
    CPPUNIT_ASSERT(aliases->Expand(b));
    CPPUNIT_ASSERT(b.size() == 1);
    CPPUNIT_ASSERT(b[0] == "1");
}

void AliasTest::testTwo()
{
    std::vector< std::string > a;
    a.push_back("alias");
    a.push_back("1");
    a.push_back("2");
    aliases->SetAlias(a);
    std::vector< std::string > b;
    b.push_back("alias");
    CPPUNIT_ASSERT(aliases->Expand(b));
    CPPUNIT_ASSERT(b.size() == 2);
    CPPUNIT_ASSERT(b[0] == "1");
    CPPUNIT_ASSERT(b[1] == "2");
}

void AliasTest::testThree()
{
    std::vector< std::string > a;
    a.push_back("alias");
    a.push_back("1");
    a.push_back("2");
    a.push_back("3");
    aliases->SetAlias(a);
    std::vector< std::string > b;
    b.push_back("alias");
    CPPUNIT_ASSERT(aliases->Expand(b));
    CPPUNIT_ASSERT(b.size() == 3);
    CPPUNIT_ASSERT(b[0] == "1");
    CPPUNIT_ASSERT(b[1] == "2");
    CPPUNIT_ASSERT(b[2] == "3");
}

void AliasTest::testRemove()
{
    std::vector< std::string > a;
    a.push_back("p");
    aliases->SetAlias(a);
    std::vector< std::string > b;
    b.push_back("p");
    CPPUNIT_ASSERT(!aliases->Expand(b));
    CPPUNIT_ASSERT(b.size() == 1);
    CPPUNIT_ASSERT(b[0] == "p");

}

void AliasTest::testDefaults()
{
    // Test for some really common defaults that should never go away
    std::vector< std::string > p;
    p.push_back("p");
    CPPUNIT_ASSERT(aliases->Expand(p));
    CPPUNIT_ASSERT(p.size() == 1);
    CPPUNIT_ASSERT(p.front() == "print");

    std::vector< std::string > q;
    q.push_back("?");
    CPPUNIT_ASSERT(aliases->Expand(q));
    CPPUNIT_ASSERT(q.size() == 1);
    CPPUNIT_ASSERT(q.front() == "help");

    std::vector< std::string > init;
    init.push_back("init");
    CPPUNIT_ASSERT(aliases->Expand(init));
    CPPUNIT_ASSERT(init.size() == 1);
    CPPUNIT_ASSERT(init.front() == "init-soar");

    std::vector< std::string > varprint;
    varprint.push_back("varprint");
    CPPUNIT_ASSERT(aliases->Expand(varprint));
    CPPUNIT_ASSERT(varprint.size() == 4);
    CPPUNIT_ASSERT(varprint[0] == "print");
    CPPUNIT_ASSERT(varprint[1] == "-v");
    CPPUNIT_ASSERT(varprint[2] == "-d");
    CPPUNIT_ASSERT(varprint[3] == "100");

    std::vector< std::string > step;
    step.push_back("step");
    CPPUNIT_ASSERT(aliases->Expand(step));
    CPPUNIT_ASSERT(step.size() == 2);
    CPPUNIT_ASSERT(step[0] == "run");
    CPPUNIT_ASSERT(step[1] == "-d");

    std::vector< std::string > d;
    d.push_back("d");
    CPPUNIT_ASSERT(aliases->Expand(d));
    CPPUNIT_ASSERT(d.size() == 2);
    CPPUNIT_ASSERT(d[0] == "run");
    CPPUNIT_ASSERT(d[1] == "-d");

    std::vector< std::string > e;
    e.push_back("e");
    CPPUNIT_ASSERT(aliases->Expand(e));
    CPPUNIT_ASSERT(e.size() == 2);
    CPPUNIT_ASSERT(e[0] == "run");
    CPPUNIT_ASSERT(e[1] == "-e");

    std::vector< std::string > stop;
    stop.push_back("stop");
    CPPUNIT_ASSERT(aliases->Expand(stop));
    CPPUNIT_ASSERT(stop.size() == 1);
    CPPUNIT_ASSERT(stop.front() == "stop-soar");

    std::vector< std::string > interrupt;
    interrupt.push_back("interrupt");
    CPPUNIT_ASSERT(aliases->Expand(interrupt));
    CPPUNIT_ASSERT(interrupt.size() == 1);
    CPPUNIT_ASSERT(interrupt.front() == "stop-soar");

    std::vector< std::string > w;
    w.push_back("w");
    CPPUNIT_ASSERT(aliases->Expand(w));
    CPPUNIT_ASSERT(w.size() == 1);
    CPPUNIT_ASSERT(w.front() == "watch");
}

void AliasTest::testSimpleCommand()
{
    cli::Parser parser;
    soar::tokenizer tok;
    tok.set_handler(&parser);

    class TestCommand : public cli::ParserCommand
    {
    public:
        virtual ~TestCommand() {}

        virtual const char* GetString() const { return "test"; }
        virtual const char* GetSyntax() const { return "Syntax"; }

        virtual bool Parse(std::vector<std::string>& argv)
        {
            CPPUNIT_ASSERT(argv.size() == 3);
            CPPUNIT_ASSERT(argv[0] == "test");
            CPPUNIT_ASSERT(argv[1] == "one");
            CPPUNIT_ASSERT(argv[2] == "two");
            return true;
        }
    };
    parser.AddCommand(new TestCommand());

    tok.evaluate("test one two");
}
