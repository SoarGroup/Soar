//
//  MultiAgentTest.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "MultiAgentTest.hpp"

#include "SoarHelper.hpp"

const int MultiAgentTest::MAX_AGENTS = 100;

void MultiAgentTest::setUp()
{
	updateEventHandler = user_data_struct(std::bind(&MultiAgentTest::MyUpdateEventHandler, this));
}

void MultiAgentTest::tearDown(bool caught)
{
}

void MultiAgentTest::createInput(sml::Agent* agent, int value)
{
	// This agent adds value1 to value2 inside the agent and puts the total on the output link.
	// We take it from the output link and puts it on the input link to generate a running total.
	sml::Identifier* pInputLink = agent->GetInputLink() ;
	assertTrue(pInputLink != NULL);
	
	sml::Identifier* pAdd = pInputLink->CreateIdWME("add") ;
	assertTrue(pAdd != NULL);
	
	sml::WMElement* pValue1 = pAdd->CreateIntWME("value1", 2) ;
	assertTrue(pValue1 != NULL);
	
	sml::WMElement* pValue2 = pAdd->CreateIntWME("value2", value) ;
	assertTrue(pValue2 != NULL);
	
	assertTrue(agent->Commit());
}

void MultiAgentTest::UpdateInput(sml::Agent* agent, int value)
{
	// Set value2 to a new value, triggering a new calculation
	sml::Identifier* pInputLink = agent->GetInputLink() ;
	assertTrue(pInputLink != NULL);
	
	sml::Identifier* pAdd = pInputLink->FindByAttribute("add", 0)->ConvertToIdentifier() ;
	assertTrue(pAdd != NULL);
	
	sml::IntElement* pValue2 = pAdd->FindByAttribute("value2", 0)->ConvertToIntElement() ;
	assertTrue(pValue2 != NULL);
	
	agent->Update(pValue2, value) ;
	
	assertTrue(agent->Commit());
}

void MultiAgentTest::MyUpdateEventHandler()
{
	int agents = pKernel->GetNumberAgents() ;
	for (int agentIndex = 0 ; agentIndex < agents ; ++agentIndex)
	{
		sml::Agent* agent = pKernel->GetAgentByIndex(agentIndex) ;
		assertTrue(agent != NULL);
		
		char const* pIOString = agent->ExecuteCommandLine("print --depth 4 i1") ;
		assertTrue(pIOString != NULL);
		//std::cout << pIOString << std::endl ;
		
		// Make sure we can get the output link (had a bug where this wouldn't always work)
		sml::Identifier* pOutputLink = agent->GetOutputLink() ;
		assertTrue(pOutputLink != NULL);
		
		// Read in the commands
		int numberCommands = agent->GetNumberCommands() ;
		for (int i = 0 ; i < numberCommands ; ++i)
		{
			sml::Identifier* pCommand = agent->GetCommand(i) ;
			assertTrue(pCommand != NULL);
			
			char const* pName = pCommand->GetCommandName() ;
			assertTrue(pName != NULL);
			assertTrue(std::string(pName) == "result");
			
			// Receive the new total
			char const* pTotal = pCommand->GetParameterValue("total") ;
			assertTrue(pTotal != NULL);
			std::stringstream paramValue(pTotal);
			int intTotal = 0;
			paramValue >> intTotal;
			
			// Mark command as completed in working memory
			pCommand->AddStatusComplete() ;
			
			// Place a new addition request on the input link
			UpdateInput(agent, intTotal);
		}
	}
}

void MultiAgentTest::reportAgentStatus(sml::Kernel* pKernel, int numberAgents, std::vector< std::stringstream* >& trace)
{
	for (int agentCounter = 0 ; agentCounter < numberAgents ; agentCounter++)
	{
		sml::Agent* agent = pKernel->GetAgentByIndex(agentCounter) ;
		assertTrue(agent != NULL);
		
		//std::cout << "Trace from agent " << agent->GetAgentName() << std::endl ;
		
		//std::cout << "Input link " << std::endl
		//  << agent->ExecuteCommandLine( "print --depth 3 i2" ) << std::endl ;
		
		//std::cout << trace[agentCounter]->str() << std::endl << std::endl ;
		
		// We need to clear this after it's been printed or the next time we print it
		// we'll get the entire trace from 0
		trace[agentCounter]->clear();
	}
}

void MultiAgentTest::initAll(sml::Kernel* pKernel)
{
	int agents = pKernel->GetNumberAgents() ;
	for (int i = 0 ; i < agents ; i++)
	{
		sml::Agent* agent = pKernel->GetAgentByIndex(i) ;
		assertTrue(agent != NULL);
		
		agent->InitSoar() ;
		//std::string initRes = agent->InitSoar() ;
		//cout << initRes << endl ;
	}
}

void MultiAgentTest::testOneAgentForSanity()
{
	numberAgents = 1;
	doTest();
}

void MultiAgentTest::testTwoAgents()
{
	numberAgents = 2;
	doTest();
}

void MultiAgentTest::testTenAgents()
{
	numberAgents = 10;
	doTest();
}

void MultiAgentTest::testMaxAgents()
{
	numberAgents = MAX_AGENTS-1;
	doTest();
}

void MultiAgentTest::doTest()
{
	pKernel = sml::Kernel::CreateKernelInCurrentThread(true, sml::Kernel::kUseAnyPort);
	no_agent_assertTrue_msg(pKernel->GetLastErrorDescription(), !pKernel->HadError());
	
	// We'll require commits, just so we're testing that path
	pKernel->SetAutoCommit(false) ;
	
	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;
	
	no_agent_assertTrue(numberAgents < MAX_AGENTS);
	
	std::vector< std::string > names;
	std::vector< sml::Agent* > agents;
	std::vector< std::stringstream* > trace;
	std::vector< int > callbackPrint;
	
	// Create the agents
	for (int agentCounter = 0 ; agentCounter < numberAgents ; ++agentCounter)
	{
		std::stringstream name;
		name << "agent" << 1 + agentCounter;
		names.push_back(name.str());
		
		sml::Agent* agent   = pKernel->CreateAgent(name.str().c_str()) ;
		assertTrue(agent != NULL);
		assertTrue_msg(pKernel->GetLastErrorDescription(), !pKernel->HadError());
		
		agents.push_back(agent);
		
		std::stringstream path;
		// TODO: use boost filesystem
		assertTrue(agent->LoadProductions(SoarHelper::GetResource("testmulti.soar").c_str()));
		createInput(agent, 0);
		
		// Collect the trace output from the run
		trace.push_back(new std::stringstream());
		
		auto lambda = [](sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage)
		{
			std::stringstream* pTrace = static_cast<std::stringstream*>(pUserData) ;
			
			(*pTrace) << pMessage;
		};
		
		callbackPrint.push_back(agent->RegisterForPrintEvent(sml::smlEVENT_PRINT,
															 lambda,
															 trace[agentCounter]));
	}
	
	auto lambda = [](sml::smlUpdateEventId, void* data, sml::Kernel*, sml::smlRunFlags)
	{
		static_cast<user_data_struct*>(data)->function();
	};
	
	pKernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_GENERATED_OUTPUT, lambda, &updateEventHandler);
	
	// Run for a first set of output, so we can see whether that worked
	pKernel->RunAllTilOutput() ;
	
	// Print out some information
	reportAgentStatus(pKernel, numberAgents, trace) ;
	
	// Now get serious about a decent run
	const int kFirstRun = 5 ;
	for (int i = 0 ; i < kFirstRun ; i++)
	{
		// Run for a bit
		pKernel->RunAllTilOutput() ;
	}
	
	reportAgentStatus(pKernel, numberAgents, trace) ;
	
	// Toss in an init-soar and then go on a bit further
	initAll(pKernel) ;
	
	// Second run
	const int kSecondRun = 5 ;
	for (int i = 0 ; i < kSecondRun ; i++)
	{
		// Run for a bit
		pKernel->RunAllTilOutput() ;
	}
	
	reportAgentStatus(pKernel, numberAgents, trace) ;
	
	for (std::vector< std::stringstream* >::iterator iter = trace.begin(); iter != trace.end(); ++iter)
	{
		delete *iter;
	}
	
	//cout << "Calling shutdown on the kernel now" << endl ;
	pKernel->Shutdown() ;
	//cout << "Shutdown completed now" << endl ;
	
	// Delete the kernel.  If this is an embedded connection this destroys the kernel.
	// If it's a remote connection we just disconnect.
	delete pKernel ;
}
