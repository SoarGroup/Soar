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
