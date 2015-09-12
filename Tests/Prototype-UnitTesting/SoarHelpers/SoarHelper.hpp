//
//  SoarHelper.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef SoarHelper_cpp
#define SoarHelper_cpp

#include "sml_ClientAgent.h"

#include <string>

class SoarHelper
{
public:
	static int getDecisionPhasesCount(sml::Agent* agent);
	static int getD_CYCLE_COUNT(sml::Agent* agent);
	static int getE_CYCLE_COUNT(sml::Agent* agent);
	static int getPE_CYCLE_COUNT(sml::Agent* agent);
	static int getINNER_E_CYCLE_COUNT(sml::Agent* agent);
	
	static int getUserProductionCount(sml::Agent* agent);
	static int getChunkProductionCount(sml::Agent* agent);
	
	enum class StopPhase
	{
		INPUT,
		PROPOSAL,
		DECISION,
		APPLY,
		OUTPUT
	};
	
	// StopPhase, before=true/false
	static std::tuple<StopPhase, bool> getStopPhase(sml::Agent* agent);
	static void setStopPhase(sml::Agent* agent, StopPhase phase, bool before = true);
	
	static std::vector<std::string> getGoalStack(sml::Agent* agent);
	
	static std::string ResourceDirectory;
	static std::string GetResource(std::string resource);
private:
	static std::string FindFile(std::string filename, std::string path);
	
	static std::string getStats(sml::Agent* agent);
	static int parseForCount(std::string search, std::string countString);
};

std::ostream& operator<<(std::ostream& os, SoarHelper::StopPhase);

#endif /* SoarHelper_cpp */
