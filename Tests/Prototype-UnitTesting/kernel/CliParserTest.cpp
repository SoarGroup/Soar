//
//  CliParserTest.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/26/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "CliParserTest.hpp"

void CliParserTest::setUp()
{
	parser = new cli::Parser();
	
	parser->AddCommand(new cli::EchoCommand(echo));
	parser->AddCommand(new cli::MaxDCTimeCommand(maxdctime));
	
	tok.set_handler(parser);
	
	agent = nullptr;
}

void CliParserTest::tearDown(bool caught)
{
	tok.set_handler(0);
	delete parser;
	parser = 0;
}

void CliParserTest::testEcho1()
{
	echo.SetExpected(1, true);
	assertTrue(tok.evaluate("echo"));
}

void CliParserTest::testEcho2()
{
	echo.SetExpected(1, false);
	assertTrue(tok.evaluate("echo -n"));
}

void CliParserTest::testEcho3()
{
	echo.SetExpected(2, true);
	assertTrue(tok.evaluate("echo stuff"));
}

void CliParserTest::testEcho4()
{
	echo.SetExpected(2, false);
	assertTrue(tok.evaluate("echo -n stuff"));
}

void CliParserTest::testEcho5()
{
	echo.SetExpected(2, false);
	assertTrue(tok.evaluate("echo stuff -n"));
}

void CliParserTest::testEcho6()
{
	echo.SetExpected(3, true);
	assertTrue(tok.evaluate(" echo other \"things\" "));
}

void CliParserTest::testEcho7()
{
	echo.SetExpected(3, false);
	assertTrue(tok.evaluate("echo {hmm -n} \"-n\" indeed"));
}

void CliParserTest::testMaxDCTime1()
{
	maxdctime.SetExpected(0);
	assertTrue(tok.evaluate("max-dc-time"));
}

void CliParserTest::testMaxDCTime2()
{
	assertTrue(!tok.evaluate("max-dc-time -1"));
}

void CliParserTest::testMaxDCTime3()
{
	assertTrue(!tok.evaluate("max-dc-time 0"));
}

void CliParserTest::testMaxDCTime4()
{
	maxdctime.SetExpected(1);
	assertTrue(tok.evaluate("max-dc-time 1"));
}

void CliParserTest::testMaxDCTime5()
{
	maxdctime.SetExpected(-1);
	assertTrue(tok.evaluate("max-dc-time -d"));
}

void CliParserTest::testMaxDCTime6()
{
	maxdctime.SetExpected(-1);
	assertTrue(tok.evaluate("max-dc-time -o"));
}

void CliParserTest::testMaxDCTime7()
{
	assertTrue(!tok.evaluate("max-dc-time -d 0"));
}

void CliParserTest::testMaxDCTime8()
{
	assertTrue(!tok.evaluate("max-dc-time -d 1"));
}

void CliParserTest::testMaxDCTime9()
{
	maxdctime.SetExpected(1000000);
	assertTrue(tok.evaluate("max-dc-time -s 1"));
}

void CliParserTest::testMaxDCTime10()
{
	maxdctime.SetExpected(500000);
	assertTrue(tok.evaluate("max-dc-time -s 0.5"));
}

void CliParserTest::testMaxDCTime11()
{
	maxdctime.SetExpected(500000);
	assertTrue(tok.evaluate("max-dc-time 0.5 -s"));
}

void CliParserTest::testMaxDCTime12()
{
	assertTrue(!tok.evaluate("max-dc-time 0.5"));
}
