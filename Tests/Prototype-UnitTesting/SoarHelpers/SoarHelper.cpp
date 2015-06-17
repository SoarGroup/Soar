//
//  SoarHelper.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "SoarHelper.hpp"

#include <sstream>

int SoarHelper::getD_CYCLE_COUNT(sml::Agent* agent)
{
	return SoarHelper::parseForCount("decisions", SoarHelper::getStats(agent));
}

int SoarHelper::getDecisionPhasesCount(sml::Agent* agent)
{
	std::stringstream ss(agent->ExecuteCommandLine("stats --decision"));
	int result;
	ss >> result;
	return result;
}

int SoarHelper::getE_CYCLE_COUNT(sml::Agent* agent)
{
	return SoarHelper::parseForCount("elaboration cycles", SoarHelper::getStats(agent));
}

int SoarHelper::getPE_CYCLE_COUNT(sml::Agent* agent)
{
	return SoarHelper::parseForCount("p-elaboration cycles", SoarHelper::getStats(agent));
}

int SoarHelper::getINNER_E_CYCLE_COUNT(sml::Agent* agent)
{
	return SoarHelper::parseForCount("inner elaboration cycles", SoarHelper::getStats(agent));
}


int SoarHelper::getUserProductionCount(sml::Agent* agent)
{
	std::string chunkPrefix = agent->ExecuteCommandLine("chunk-name-format -p");
	chunkPrefix = chunkPrefix.substr(8, 5);
	
	std::string rules = agent->ExecuteCommandLine("p");
	
	std::stringstream ss(rules);
	std::string line;
	
	int count = 0;
	
	while (std::getline(ss, line, '\n'))
	{
		if (line.find(chunkPrefix) != 0)
			++count;
	}
	
	return count;
}

int SoarHelper::getChunkProductionCount(sml::Agent* agent)
{
	std::string chunkPrefix = agent->ExecuteCommandLine("chunk-name-format -p");
	chunkPrefix = chunkPrefix.substr(8, 5);
	
	std::string rules = agent->ExecuteCommandLine("p");
	
	std::stringstream ss(rules);
	std::string line;
	
	int count = 0;
	
	while (std::getline(ss, line, '\n'))
	{
		if (line.find(chunkPrefix) == 0)
			++count;
	}
	
	return count;
}

std::string SoarHelper::getStats(sml::Agent* agent)
{
	return agent->ExecuteCommandLine("stats");
}

int SoarHelper::parseForCount(std::string search, std::string countString)
{
	std::stringstream ss(countString);
	std::string line;
	
	while (std::getline(ss, line, '\n'))
	{
		if (line.size() > 0 && isdigit(line.front()))
		{
			// Potential match
			// Check for match first
			size_t first_space = line.find(' ');
			
			if (first_space != std::string::npos)
			{
				++first_space;
				
				int matchCount = 0;
				for (size_t i = first_space;i < line.size() && line[i] == search[i-first_space];++i,++matchCount);
				
				if (matchCount == search.size())
				{
					// Match
					// Get the number
					
					// Get the number first
					std::string s_number = line.substr(0, first_space-1);
					int number;
					
					std::stringstream ss_number(s_number);
					ss_number >> number;
					
					return number;
				}
			}
		}
	}
	
	return -1;
}

std::tuple<SoarHelper::StopPhase, bool> SoarHelper::getStopPhase(sml::Agent* agent)
{
	std::istringstream iss(agent->ExecuteCommandLine("set-stop-phase"));
	
	std::string s_before;
	std::string s_phase;
	std::string temp;
	
	iss >> temp >> s_before >> s_phase >> temp;
	
	bool before = false;
	StopPhase phase = StopPhase::INPUT;
	
	if (s_before == "before")
		before = true;
	
	if (s_phase == "input")
		phase = StopPhase::INPUT;
	else if (s_phase == "proposal")
		phase = StopPhase::PROPOSAL;
	else if (s_phase == "decision")
		phase = StopPhase::DECISION;
	else if (s_phase == "apply")
		phase = StopPhase::APPLY;
	else if (s_phase == "output")
		phase = StopPhase::OUTPUT;
	
	return std::make_tuple(phase, before);
}

std::vector<std::string> SoarHelper::getGoalStack(sml::Agent* agent)
{
	std::string s_result = agent->ExecuteCommandLine("p --stack");
	
	std::stringstream ss(s_result);
	
	std::vector<std::string> v_result;
	std::string line;
	
	while (std::getline(ss, line, '\n'))
	{
		size_t start = line.find("==>S: ")+6;
		size_t end = line.find(" ", start);
		
		if (end == std::string::npos)
			end = line.size();
		
		std::string goal = line.substr(start, end-start);
		
		v_result.push_back(goal);
	}
	
	return v_result;
}

std::ostream& operator<<(std::ostream& os, SoarHelper::StopPhase phase)
{
	switch (phase)
	{
		case SoarHelper::StopPhase::INPUT: os << "INPUT";
		case SoarHelper::StopPhase::PROPOSAL: os << "PROPOSAL";
		case SoarHelper::StopPhase::DECISION: os << "DECISION";
		case SoarHelper::StopPhase::APPLY: os << "APPLY";
		case SoarHelper::StopPhase::OUTPUT: os << "OUTPUT";
	}
	
	return os;
}
