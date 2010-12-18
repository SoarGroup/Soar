#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include "tokenizer.h"
#include <stdarg.h>
#include <queue>

struct CallData
{
    CallData(const char* input)
        : input(input)
    {
    }

    CallData(const char* input, int expected, ...)
        : input(input)
    {
        va_list args;
        va_start(args, expected);
        std::vector<std::string> argv;
        for (int i = 0; i < expected; ++i)
            argv.push_back(va_arg(args, const char*));
        va_end(args);

        q.push(argv);
    }

    void addResult(int expected, ...)
    {
        va_list args;
        va_start(args, expected);
        std::vector<std::string> argv;
        for (int i = 0; i < expected; ++i)
            argv.push_back(va_arg(args, const char*));
        va_end(args);
        q.push(argv);
    }

    const char* input;
    std::queue< std::vector<std::string> > q;
};

class TokenizerTest : public CPPUNIT_NS::TestCase, public soar::tokenizer_callback
{
	CPPUNIT_TEST_SUITE( TokenizerTest );	// The name of this class

	CPPUNIT_TEST( testTokenizer01 );
	CPPUNIT_TEST( testTokenizer02 );
	CPPUNIT_TEST( testTokenizer03 );
	CPPUNIT_TEST( testTokenizer04 );
	CPPUNIT_TEST( testTokenizer05 );
	CPPUNIT_TEST( testTokenizer06 );
	CPPUNIT_TEST( testTokenizer07 );
	CPPUNIT_TEST( testTokenizer08 );
	CPPUNIT_TEST( testTokenizer09 );
	CPPUNIT_TEST( testTokenizer10 );
	CPPUNIT_TEST( testTokenizer11 );
	CPPUNIT_TEST( testTokenizer12 );
	CPPUNIT_TEST( testTokenizer13 );
	CPPUNIT_TEST( testTokenizer14 );
	CPPUNIT_TEST( testTokenizer15 );
	CPPUNIT_TEST( testTokenizer16 );
	CPPUNIT_TEST( testTokenizer17 );
	CPPUNIT_TEST( testTokenizer18 );
	CPPUNIT_TEST( testTokenizer19 );
	CPPUNIT_TEST( testTokenizer20 );
	CPPUNIT_TEST( testTokenizer21 );
	CPPUNIT_TEST( testTokenizer22 );
	CPPUNIT_TEST( testTokenizer23 );
	CPPUNIT_TEST( testTokenizer24 );
	CPPUNIT_TEST( testTokenizer25 );
	CPPUNIT_TEST( testTokenizer26 );
	CPPUNIT_TEST( testTokenizer27 );
	CPPUNIT_TEST( testTokenizer28 );
	CPPUNIT_TEST( testTokenizer29 );
	CPPUNIT_TEST( testTokenizer30 );
	CPPUNIT_TEST( testTokenizer31 );
	CPPUNIT_TEST( testTokenizer32 );
	CPPUNIT_TEST( testTokenizer33 );
	CPPUNIT_TEST( testTokenizer34 );
	CPPUNIT_TEST( testTokenizer35 );
	CPPUNIT_TEST( testTokenizer36 );
	CPPUNIT_TEST( testTokenizer37 );
	CPPUNIT_TEST( testTokenizer38 );
	CPPUNIT_TEST( testTokenizer39 );
	CPPUNIT_TEST( testTokenizer40 );
	CPPUNIT_TEST( testTokenizer41 );
	CPPUNIT_TEST( testTokenizer42 );
	CPPUNIT_TEST( testTokenizer43 );
	CPPUNIT_TEST( testTokenizer44 );
	CPPUNIT_TEST( testTokenizer45 );
	CPPUNIT_TEST( testTokenizer46 );
	CPPUNIT_TEST( testTokenizer47 );

	CPPUNIT_TEST_SUITE_END();

public:
    TokenizerTest() 
        : cd(0) 
    {}
    virtual ~TokenizerTest() {}
    virtual bool handle_command(std::vector<std::string>& argv);

	void setUp();		// Called before each function outlined by CPPUNIT_TEST
	void tearDown();	// Called after each function outlined by CPPUNIT_TEST

protected:
	void testTokenizer01();
	void testTokenizer02();
	void testTokenizer03();
	void testTokenizer04();
	void testTokenizer05();
	void testTokenizer06();
	void testTokenizer07();
	void testTokenizer08();
	void testTokenizer09();
	void testTokenizer10();
	void testTokenizer11();
	void testTokenizer12();
	void testTokenizer13();
	void testTokenizer14();
	void testTokenizer15();
	void testTokenizer16();
	void testTokenizer17();
	void testTokenizer18();
	void testTokenizer19();
	void testTokenizer20();
	void testTokenizer21();
	void testTokenizer22();
	void testTokenizer23();
	void testTokenizer24();
	void testTokenizer25();
	void testTokenizer26();
	void testTokenizer27();
	void testTokenizer28();
	void testTokenizer29();
	void testTokenizer30();
	void testTokenizer31();
	void testTokenizer32();
	void testTokenizer33();
	void testTokenizer34();
	void testTokenizer35();
	void testTokenizer36();
	void testTokenizer37();
	void testTokenizer38();
	void testTokenizer39();
	void testTokenizer40();
	void testTokenizer41();
	void testTokenizer42();
	void testTokenizer43();
	void testTokenizer44();
	void testTokenizer45();
	void testTokenizer46();
	void testTokenizer47();

    void evaluate(CallData& cd);

    soar::tokenizer* tokenizer;
    CallData* cd;
};

CPPUNIT_TEST_SUITE_REGISTRATION( TokenizerTest ); // Registers the test so it will be used

void TokenizerTest::setUp()
{
    tokenizer = new soar::tokenizer();
    tokenizer->set_handler(this);
}

void TokenizerTest::tearDown()
{
    delete tokenizer;
}

bool TokenizerTest::handle_command(std::vector<std::string>& argv)
{
    CPPUNIT_ASSERT_MESSAGE(cd->input, argv.size() == cd->q.front().size());
    for (int i=0; i < argv.size(); i++)
        CPPUNIT_ASSERT(argv[i] == cd->q.front()[i]);
    cd->q.pop();
    return true;
}

void TokenizerTest::evaluate(CallData& cd)
{
    this->cd = &cd;
    CPPUNIT_ASSERT_MESSAGE(cd.input, tokenizer->evaluate(cd.input));
}

void TokenizerTest::testTokenizer01() 
{ 
    CallData cd("seek", 1, "seek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer02() 
{ 
    CallData cd("\nseek", 1, "seek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer03() 
{ 
    CallData cd(" \nseek", 1, "seek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer04() 
{ 
    CallData cd(" \n seek", 1, "seek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer05() 
{ 
    CallData cd("seek\n", 1, "seek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer06() 
{ 
    CallData cd("seek\n ", 1, "seek"); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer07() 
{ 
    CallData cd("seek \n", 1, "seek"); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer08() 
{ 
    CallData cd("seek \n ", 1, "seek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer09() 
{ 
    CallData cd("s eek", 2, "s", "eek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer10() 
{ 
    CallData cd(" s eek", 2, "s", "eek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer11() 
{ 
    CallData cd("s eek ", 2, "s", "eek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer12() 
{ 
    CallData cd("s  eek", 2, "s", "eek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer13() 
{ 
    CallData cd("s  eek", 2, "s", "eek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer14() 
{
    CallData cd("s   eek", 2, "s", "eek");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer15() 
{ 
    CallData cd(" s ee k", 3, "s", "ee", "k");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer16() 
{ 
    CallData cd("s ee k", 3, "s", "ee", "k");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer17() 
{ 
    CallData cd("s ee k ", 3, "s", "ee", "k");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer18() 
{ 
    CallData cd(" s ee k ", 3, "s", "ee", "k");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer19() 
{ 
    CallData cd(" s ee k ", 3, "s", "ee", "k"); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer20() 
{ 
    CallData cd("s \"ee\" k", 3, "s", "ee", "k"); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer21() 
{ 
    CallData cd("s \"e e\" k", 3, "s", "e e", "k");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer22() 
{ 
    CallData cd("s \" e e\" k", 3, "s", " e e", "k");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer23() 
{ 
    CallData cd("s \"e e \" k", 3, "s", "e e ", "k");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer24() 
{
    CallData a("seek\nseek");
    a.addResult(1, "seek");
    a.addResult(1, "seek");
    evaluate(a);
}
void TokenizerTest::testTokenizer25()
{
    CallData a("\nseek\nseek");
    a.addResult(1, "seek");
    a.addResult(1, "seek");
    evaluate(a);
}
void TokenizerTest::testTokenizer26()
{
    CallData a("seek\nseek\n");
    a.addResult(1, "seek");
    a.addResult(1, "seek");
    evaluate(a);
}
void TokenizerTest::testTokenizer27()
{
    CallData a("seek\nse\nek");
    a.addResult(1, "seek");
    a.addResult(1, "se");
    a.addResult(1, "ek");
    evaluate(a);
}
void TokenizerTest::testTokenizer28()
{
    CallData a("seek\n se  \n ek");
    a.addResult(1, "seek");
    a.addResult(1, "se");
    a.addResult(1, "ek");
    evaluate(a);
}
void TokenizerTest::testTokenizer29() 
{ 
    CallData cd("\"se\nek\"", 1, "se\nek"); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer30() 
{ 
    CallData cd("{{seek}}", 1, "{seek}"); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer31()
{
    const char* body =
        "water-jug*propose*pour\n"
        "   (state <s> ^name water-jug\n"
        "              ^jug <i> { <> <i> <j> })\n"
        "   (<i> ^contents > 0 )\n"
        "   (<j> ^empty > 0)\n"
        "-->\n"
        "   (<s> ^operator <o> + =)\n"
        "   (<o> ^name pour\n"
        "        ^empty-jug <i>\n"
        "        ^fill-jug <j>)";
    std::string rule("sp {");
    rule.append(body);
    rule.append("}");
    CallData cd(rule.c_str(), 2, "sp", body);
    evaluate(cd); 
}
void TokenizerTest::testTokenizer32() 
{ 
    CallData cd("print -i (s1 ^* *)", 5, "print", "-i", "(s1", "^*", "*)"); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer33() 
{ 
    CallData cd("{[seek]}", 1, "[seek]"); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer34()
{
    const char* body =
        "gp*test3\n"
        "   (state <s> ^operator <o> +\n"
        "              ^someflag [ true false ])\n"
        "   (<o> ^name foo\n"
        "        ^att [\n"
        "               val1 # a string value\n"
        "               1.3  # a numeric value\n"
        "               |another val[]][[[]]]][|  # a value with spaces and brackets in it\n"
        "               |\\|another val\\||  # a value with escaped pipes in it\n"
        "               ])\n"
        "-->\n"
        "   (<s> ^operator <o> = 15)\n";
    std::string rule("gp {");
    rule.append(body);
    rule.append("}");
    CallData cd(rule.c_str(), 2, "gp", body);
    evaluate(cd); 
}

void TokenizerTest::testTokenizer35()
{
    const char* body =
        "add*propose*toggle-to-b\n"
        "   (state <s> ^superstate.operator.name add\n"
        "              ^tss <tss>\n"
        "             #-^sum\n"
        "              ^superstate.toggle a)\n"
        "            # -^superstate.toggle b)\n"
        "   (<tss> -^sum)\n"
        "-->\n"
        "   (<s> ^operator <o> +)\n"
        "   (<o> ^name toggle-to-b)\n";
    std::string rule("sp {");
    rule.append(body);
    rule.append("}");
    CallData cd(rule.c_str(), 2, "sp", body);
    evaluate(cd); 
}
void TokenizerTest::testTokenizer36()
{
    CPPUNIT_ASSERT(!tokenizer->evaluate("sp a\" \""));
    CPPUNIT_ASSERT(tokenizer->get_error_string());
    CPPUNIT_ASSERT(tokenizer->get_current_line_number() == 1);
    CPPUNIT_ASSERT(tokenizer->get_command_line_number() == 1);
    CPPUNIT_ASSERT(tokenizer->get_offset() == 8);
}
void TokenizerTest::testTokenizer37()
{
    CPPUNIT_ASSERT(!tokenizer->evaluate(" \t\nsp a\" \""));
    CPPUNIT_ASSERT(tokenizer->get_error_string());
    CPPUNIT_ASSERT(tokenizer->get_current_line_number() == 2);
    CPPUNIT_ASSERT(tokenizer->get_command_line_number() == 2);
    CPPUNIT_ASSERT(tokenizer->get_offset() == 8);
}
void TokenizerTest::testTokenizer38()
{
    CPPUNIT_ASSERT(!tokenizer->evaluate(" \t\n  sp a {\n\\n \""));
    CPPUNIT_ASSERT(tokenizer->get_error_string());
    CPPUNIT_ASSERT(tokenizer->get_current_line_number() == 3);
    CPPUNIT_ASSERT(tokenizer->get_command_line_number() == 2);
    CPPUNIT_ASSERT(tokenizer->get_offset() == 5);
}
void TokenizerTest::testTokenizer39()
{
    CPPUNIT_ASSERT(!tokenizer->evaluate("\\n\\"));
    CPPUNIT_ASSERT(tokenizer->get_error_string());
    CPPUNIT_ASSERT(tokenizer->get_current_line_number() == 1);
    CPPUNIT_ASSERT(tokenizer->get_command_line_number() == 1);
    CPPUNIT_ASSERT(tokenizer->get_offset() == 4);
}
void TokenizerTest::testTokenizer40()
{
    CallData cd("\0");
    evaluate(cd);
}
void TokenizerTest::testTokenizer41()
{
    CallData cd("\\nv\\\n t", 1, "\nv t"); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer42()
{
    CallData cd("sp a\" \\\"", 3, "sp", "a\"", "\""); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer43()
{
    CallData cd("{} { } {  } \"\" \" \" \"  \"", 6, "", " ", "  ", "", " ", "  "); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer44()
{
    CallData cd("{\\\"} \"\\\"\"", 2, "\\\"", "\""); 
    evaluate(cd); 
}
void TokenizerTest::testTokenizer45()
{
    CallData cd("a ; b\n;c;;d"); 
    cd.addResult(1, "a");
    cd.addResult(1, "b");
    cd.addResult(1, "c");
    cd.addResult(1, "d");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer46()
{
    CallData cd("a {b c\\\n \td \\n\n \\}}"); 
    cd.addResult(2, "a", "b c d \\n\n \\}");
    evaluate(cd); 
}
void TokenizerTest::testTokenizer47()
{
    CallData a("w 0; run");
    a.addResult(2, "w", "0");
    a.addResult(1, "run");
    evaluate(a);
}
