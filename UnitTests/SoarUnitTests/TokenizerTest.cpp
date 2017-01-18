//
//  TokenizerTest.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "TokenizerTest.hpp"

void TokenizerTest::setUp()
{
	tokenizer = new soar::tokenizer();
	tokenizer->set_handler(this);
}

void TokenizerTest::tearDown(bool caught)
{
	delete tokenizer;
}

bool TokenizerTest::handle_command(std::vector<std::string>& argv)
{
	no_agent_assertTrue_msg(cd->input, argv.size() == cd->q.front().size());
	for (std::vector<std::string>::size_type i = 0; i < argv.size(); i++)
	{
		no_agent_assertTrue(argv[i] == cd->q.front()[i]);
	}
	cd->q.pop();
	return true;
}

void TokenizerTest::evaluate(CallData& cd)
{
	this->cd = &cd;
	no_agent_assertTrue_msg(cd.input, tokenizer->evaluate(cd.input));
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
	const char* body_nocomments =
	"gp*test3\n"
	"   (state <s> ^operator <o> +\n"
	"              ^someflag [ true false ])\n"
	"   (<o> ^name foo\n"
	"        ^att [\n"
	"               val1 \n"
	"               1.3  \n"
	"               |another val[]][[[]]]][|  \n"
	"               |\\|another val\\||  \n"
	"               ])\n"
	"-->\n"
	"   (<s> ^operator <o> = 15)\n";
	std::string rule("gp {");
	rule.append(body);
	rule.append("}");
	CallData cd(rule.c_str(), 2, "gp", body_nocomments);
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
	const char* body_nocomments =
	"add*propose*toggle-to-b\n"
	"   (state <s> ^superstate.operator.name add\n"
	"              ^tss <tss>\n"
	"             \n"
	"              ^superstate.toggle a)\n"
	"            \n"
	"   (<tss> -^sum)\n"
	"-->\n"
	"   (<s> ^operator <o> +)\n"
	"   (<o> ^name toggle-to-b)\n";
	std::string rule("sp {");
	rule.append(body);
	rule.append("}");
	CallData cd(rule.c_str(), 2, "sp", body_nocomments);
	evaluate(cd);
}
void TokenizerTest::testTokenizer36()
{
	no_agent_assertTrue(!tokenizer->evaluate("sp a\" \""));
	no_agent_assertTrue(tokenizer->get_error_string());
	no_agent_assertTrue(tokenizer->get_current_line_number() == 1);
	no_agent_assertTrue(tokenizer->get_command_line_number() == 1);
	no_agent_assertTrue(tokenizer->get_offset() == 8);
}
void TokenizerTest::testTokenizer37()
{
	no_agent_assertTrue(!tokenizer->evaluate(" \t\nsp a\" \""));
	no_agent_assertTrue(tokenizer->get_error_string());
	no_agent_assertTrue(tokenizer->get_current_line_number() == 2);
	no_agent_assertTrue(tokenizer->get_command_line_number() == 2);
	no_agent_assertTrue(tokenizer->get_offset() == 8);
}
void TokenizerTest::testTokenizer38()
{
	no_agent_assertTrue(!tokenizer->evaluate(" \t\n  sp a {\n\\n \""));
	no_agent_assertTrue(tokenizer->get_error_string());
	no_agent_assertTrue(tokenizer->get_current_line_number() == 3);
	no_agent_assertTrue(tokenizer->get_command_line_number() == 2);
	no_agent_assertTrue(tokenizer->get_offset() == 5);
}
void TokenizerTest::testTokenizer39()
{
	no_agent_assertTrue(!tokenizer->evaluate("\\n\\"));
	no_agent_assertTrue(tokenizer->get_error_string());
	no_agent_assertTrue(tokenizer->get_current_line_number() == 1);
	no_agent_assertTrue(tokenizer->get_command_line_number() == 1);
	no_agent_assertTrue(tokenizer->get_offset() == 4);
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

void TokenizerTest::testTokenizer48()
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
	std::string rule("sp \"");
	rule.append(body);
	rule.append("\"");
	CallData cd(rule.c_str(), 2, "sp", body);
	evaluate(cd);
}