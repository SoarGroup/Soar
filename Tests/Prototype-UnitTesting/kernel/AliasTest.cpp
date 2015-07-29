//
//  AliasTest.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/26/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "AliasTest.hpp"

void AliasTest::before()
{
	aliases = new cli::Aliases();
	agent = nullptr;
}

void AliasTest::after(bool caught)
{
	delete aliases;
	aliases = nullptr;
}

void AliasTest::testOne()
{
	std::vector< std::string > a;
	a.push_back("alias");
	a.push_back("1");
	aliases->SetAlias(a);
	std::vector< std::string > b;
	b.push_back("alias");
	assertTrue(aliases->Expand(b));
	assertTrue(b.size() == 1);
	assertTrue(b[0] == "1");
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
	assertTrue(aliases->Expand(b));
	assertTrue(b.size() == 2);
	assertTrue(b[0] == "1");
	assertTrue(b[1] == "2");
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
	assertTrue(aliases->Expand(b));
	assertTrue(b.size() == 3);
	assertTrue(b[0] == "1");
	assertTrue(b[1] == "2");
	assertTrue(b[2] == "3");
}

void AliasTest::testRemove()
{
	std::vector< std::string > a;
	a.push_back("p");
	aliases->SetAlias(a);
	std::vector< std::string > b;
	b.push_back("p");
	assertTrue(!aliases->Expand(b));
	assertTrue(b.size() == 1);
	assertTrue(b[0] == "p");
	
}

void AliasTest::testDefaults()
{
	// Test for some really common defaults that should never go away
	std::vector< std::string > p;
	p.push_back("p");
	assertTrue(aliases->Expand(p));
	assertTrue(p.size() == 1);
	assertTrue(p.front() == "print");
	
	std::vector< std::string > q;
	q.push_back("?");
	assertTrue(aliases->Expand(q));
	assertTrue(q.size() == 1);
	assertTrue(q.front() == "help");
	
	std::vector< std::string > init;
	init.push_back("init");
	assertTrue(aliases->Expand(init));
	assertTrue(init.size() == 1);
	assertTrue(init.front() == "init-soar");
	
	std::vector< std::string > varprint;
	varprint.push_back("varprint");
	assertTrue(aliases->Expand(varprint));
	assertTrue(varprint.size() == 4);
	assertTrue(varprint[0] == "print");
	assertTrue(varprint[1] == "-v");
	assertTrue(varprint[2] == "-d");
	assertTrue(varprint[3] == "100");
	
	std::vector< std::string > step;
	step.push_back("step");
	assertTrue(aliases->Expand(step));
	assertTrue(step.size() == 2);
	assertTrue(step[0] == "run");
	assertTrue(step[1] == "-d");
	
	std::vector< std::string > d;
	d.push_back("d");
	assertTrue(aliases->Expand(d));
	assertTrue(d.size() == 2);
	assertTrue(d[0] == "run");
	assertTrue(d[1] == "-d");
	
	std::vector< std::string > e;
	e.push_back("e");
	assertTrue(aliases->Expand(e));
	assertTrue(e.size() == 2);
	assertTrue(e[0] == "run");
	assertTrue(e[1] == "-e");
	
	std::vector< std::string > stop;
	stop.push_back("stop");
	assertTrue(aliases->Expand(stop));
	assertTrue(stop.size() == 1);
	assertTrue(stop.front() == "stop-soar");
	
	std::vector< std::string > interrupt;
	interrupt.push_back("interrupt");
	assertTrue(aliases->Expand(interrupt));
	assertTrue(interrupt.size() == 1);
	assertTrue(interrupt.front() == "stop-soar");
	
	std::vector< std::string > w;
	w.push_back("w");
	assertTrue(aliases->Expand(w));
	assertTrue(w.size() == 1);
	assertTrue(w.front() == "watch");
}

void AliasTest::testSimpleCommand()
{
	cli::Parser parser;
	soar::tokenizer tok;
	tok.set_handler(&parser);
	
	class TestCommand : public cli::ParserCommand
	{
		TestRunner* runner;
		sml::Agent* agent;
		
	public:
		TestCommand(TestRunner* r, sml::Agent* a)
		: runner(r), agent(a)
		{}
		
		virtual ~TestCommand() {}
		
		virtual const char* GetString() const
		{
			return "test";
		}
		virtual const char* GetSyntax() const
		{
			return "Syntax";
		}
		
		virtual bool Parse(std::vector<std::string>& argv)
		{
			assertTrue(argv.size() == 3);
			assertTrue(argv[0] == "test");
			assertTrue(argv[1] == "one");
			assertTrue(argv[2] == "two");
			return true;
		}
	};
	
	parser.AddCommand(new TestCommand(runner, agent));
	
	tok.evaluate("test one two");
}
