//
//  IOTests.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "IOTests.hpp"

#include "SoarHelper.hpp"

void IOTests::testInputLeak()
{
	agent->ExecuteCommandLine("set-stop-phase --before --input") ;
	assertTrue_msg("set-stop-phase --before --input", agent->GetLastCommandLineResult());
	
	agent->ExecuteCommandLine("watch 0") ;
	agent->ExecuteCommandLine("waitsnc --on") ;
	
	sml::Identifier* pInputLink = agent->GetInputLink();
	sml::StringElement* pFooBar = 0;
	
	for (int count = 0; count < 50000; ++count)
	{
		if (count % 2 == 0)
		{
			// even case
			
			// creating the wme
			assertTrue(pFooBar == 0);
			
			pFooBar = pInputLink->CreateStringWME("foo", "bar");
			assertTrue(pFooBar != 0);
			
			kernel->RunAllAgents(1);
		}
		else
		{
			// odd case
			// deleting the wme
			assertTrue(pFooBar != 0);
			pFooBar->DestroyWME();
			
			pFooBar = 0;
			
			kernel->RunAllAgents(1);
		}
	}
}

void IOTests::testInputLeak2()
{
	agent->ExecuteCommandLine("set-stop-phase --before --input") ;
	assertTrue_msg("set-stop-phase --before --input", agent->GetLastCommandLineResult());
	
	agent->ExecuteCommandLine("watch 0") ;
	agent->ExecuteCommandLine("waitsnc --on") ;
	
	sml::Identifier* pInputLink = agent->GetInputLink();
	sml::Identifier* pIdentifier = 0;
	sml::StringElement* pFooBar = 0;
	
	for (int count = 0; count < 50000; ++count)
	{
		if (count % 2 == 0)
		{
			// even case
			
			// creating the wme
			assertTrue(pIdentifier == 0);
			assertTrue(pFooBar == 0);
			
			pIdentifier = pInputLink->CreateIdWME("alpha");
			assertTrue(pIdentifier != 0);
			
			pFooBar = pIdentifier->CreateStringWME("foo", "bar");
			assertTrue(pFooBar != 0);
			
			kernel->RunAllAgents(1);
		}
		else
		{
			// odd case
			// deleting the wme
			assertTrue(pFooBar != 0);
			pFooBar->DestroyWME();
			
			assertTrue(pIdentifier != 0);
			pIdentifier->DestroyWME();
			
			pIdentifier = 0;
			pFooBar = 0;
			
			kernel->RunAllAgents(1);
		}
	}
}

void IOTests::testInputLeak3()
{
	agent->ExecuteCommandLine("set-stop-phase --before --input") ;
	assertTrue_msg("set-stop-phase --before --input", agent->GetLastCommandLineResult());
	
	agent->ExecuteCommandLine("watch 0") ;
	agent->ExecuteCommandLine("waitsnc --on") ;
	
	sml::Identifier* pInputLink = agent->GetInputLink();
	sml::Identifier* pIdentifier = 0;
	sml::StringElement* pFooBar = 0;
	
	for (int count = 0; count < 50000; ++count)
	{
		if (count % 2 == 0)
		{
			// even case
			
			// creating the wme
			assertTrue(pIdentifier == 0);
			assertTrue(pFooBar == 0);
			
			pIdentifier = pInputLink->CreateIdWME("alpha");
			assertTrue(pIdentifier != 0);
			
			pFooBar = pIdentifier->CreateStringWME("foo", "bar");
			assertTrue(pFooBar != 0);
			
			kernel->RunAllAgents(1);
		}
		else
		{
			// odd case
			// deleting the wme
			assertTrue(pIdentifier != 0);
			pIdentifier->DestroyWME();
			
			pIdentifier = 0;
			pFooBar = 0;
			
			kernel->RunAllAgents(1);
		}
	}
}

void IOTests::testInputLeak4()
{
	agent->ExecuteCommandLine("set-stop-phase --before --input") ;
	assertTrue_msg("set-stop-phase --before --input", agent->GetLastCommandLineResult());
	
	agent->ExecuteCommandLine("watch 0") ;
	agent->ExecuteCommandLine("waitsnc --on") ;
	
	sml::Identifier* pInputLink = agent->GetInputLink();
	sml::Identifier* pIdentifier = 0;
	sml::StringElement* pFooBar = 0;
	sml::Identifier* pSharedIdentifier = 0;
	
	for (int count = 0; count < 50000; ++count)
	{
		if (count % 2 == 0)
		{
			// even case
			
			// creating the wme
			assertTrue(pIdentifier == 0);
			assertTrue(pFooBar == 0);
			assertTrue(pSharedIdentifier == 0);
			
			pIdentifier = pInputLink->CreateIdWME("alpha");
			assertTrue(pIdentifier != 0);
			
			pFooBar = pIdentifier->CreateStringWME("foo", "bar");
			assertTrue(pFooBar != 0);
			
			pSharedIdentifier = pInputLink->CreateSharedIdWME("alpha", pIdentifier);
			assertTrue(pSharedIdentifier != 0);
			
			kernel->RunAllAgents(1);
		}
		else
		{
			// odd case
			// deleting the wme
			assertTrue(pIdentifier != 0);
			pIdentifier->DestroyWME();
			
			assertTrue(pSharedIdentifier != 0);
			pSharedIdentifier->DestroyWME();
			
			pIdentifier = 0;
			pFooBar = 0;
			pSharedIdentifier = 0;
			
			kernel->RunAllAgents(1);
		}
	}
}

void IOTests::testOutputLeak1()
{
	agent->ExecuteCommandLine("set-stop-phase --before --input") ;
	assertTrue_msg("set-stop-phase --before --input", agent->GetLastCommandLineResult());
	
	agent->ExecuteCommandLine("watch 0") ;
	agent->ExecuteCommandLine("waitsnc --on") ;
	
	agent->LoadProductions(SoarHelper::GetResource("testoutputleak.soar").c_str());
	assertTrue_msg("loadProductions", agent->GetLastCommandLineResult());
	
	/*sml::Identifier* pOutputLink = */agent->GetOutputLink();
	
	kernel->RunAllAgents(1);
	
#ifdef _WIN32
#ifdef _DEBUG
	_CrtMemState memState;
	
	_CrtMemCheckpoint(&memState);
	//_CrtSetBreakAlloc( 3020 );
#endif
#endif
	
	assertTrue(agent->GetNumberCommands() == 1);
	sml::Identifier* pCommand = agent->GetCommand(0) ;
	pCommand->AddStatusComplete();
	
	// need to pass input phase
	kernel->RunAllAgents(2);
	
	assertTrue(kernel->DestroyAgent(agent));
	kernel->Shutdown() ;
	delete kernel ;
	kernel = nullptr;
	
#ifdef _WIN32
#ifdef _DEBUG
	_CrtMemDumpAllObjectsSince(&memState);
#endif
#endif
	
	setUp();
}