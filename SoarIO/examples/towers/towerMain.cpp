

#include "sgioTowers.h"
#include "sgioTowersSoarAgent.h"

//SGIO directives
#include "sgio_wmemem.h"
#include "sgio_soar.h"
#include "sgio_agent.h"
#include "sgio_apisoar.h"
#include "sgio_siosoar.h"

using sgio::Soar;
using sgio::Agent;
using sgio::SIOSoar;
using sgio::APISoar;
using sgio::WorkingMemory;

//standard directives
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

const int defaultNumTowers = 3;
const int defaultNumdisks = 11;

int main(int argc, char* argv[])
{	
	
	bool doPrinting = true;
	string sgioMode = "api";
	int numTowers = defaultNumTowers;
	int numdisks = defaultNumdisks;

	if(argc > 1)
	{
		if(!strcmp(argv[1], "false"))
			doPrinting = false;
		// @TODO more checking, for robustness 
	}

	if(argc > 2)
	{
		numTowers = atoi(argv[3]);
		if(numTowers < 3)
			numTowers = 3;
	}

	if(argc > 3)
	{
		if(!strcmp(argv[4], "sio"))
			sgioMode = "sio";
	}

	if(doPrinting)
		cout << "***Welcome to Towers of Hanoi***" << endl << endl;

	//===================================================================
	//this block scope is unrelated to the above 'if' statement
	//===================================================================
	{
		sgio::Soar* soar;

		if(sgioMode == "api")
			soar = new APISoar();
		else
			soar = new SIOSoar("127.0.0.1", 6969, true);

		Agent* agent = soar->CreateAgent("sgioTowersAgent");
		agent->LoadProductions("towers-of-hanoi-86.soar");
		WorkingMemory* mem = new WorkingMemory(agent);

		HanoiWorld hanoi(mem, doPrinting, numTowers);

		SGIOTowersSoarAgent soarAgent(agent, &hanoi);

		//register the SoarAgent as the output processor
//		IOutputLink* oLink = agent->GetOutputLink();


		if(doPrinting)
			hanoi.Print();

		while(!hanoi.AtGoalState())
		{
			soarAgent.MakeMove();

			if (doPrinting)
				hanoi.Print();
		}
		soarAgent.MakeMove();//this allows the agent to catch up to the game world
		//after the goal state
	}




	return 0;
}