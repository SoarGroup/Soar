//
//  SoarHelper.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "SoarHelper.hpp"

#include <sstream>

#include "portability.h"
#include "portable-dirent.h"

std::string SoarHelper::ResourceDirectory = "./Tests/";

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

void SoarHelper::setStopPhase(sml::Agent* agent, StopPhase phase, bool before)
{
	std::string s_before = "--before";
	std::string s_phase = "--input";
	
	if (before == false)
		s_before = "--after";
	
	if (phase == StopPhase::INPUT)
		s_phase = "--input";
	else if (phase == StopPhase::PROPOSAL)
		s_phase = "--proposal";
	else if (phase == StopPhase::DECISION)
		s_phase = "--decision";
	else if (phase == StopPhase::APPLY)
		s_phase = "--apply";
	else if (phase == StopPhase::OUTPUT)
		s_phase = "--output";
	
	agent->ExecuteCommandLine(std::string("set-stop-phase " + s_before + s_phase).c_str());
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

std::string SoarHelper::GetResource(std::string resource)
{
	std::string path =  FindFile(resource, ResourceDirectory);
	const char* workingDirectory = getenv("WORKING_DIRECTORY");
	const char* soarHome = getenv("SOAR_HOME");
	
	if (path.length() == 0 && workingDirectory)
	{
		path = FindFile(resource, workingDirectory);
	}
	
	if (path.length() == 0 && soarHome)
	{
		path = FindFile(resource, soarHome);
	}
	
	if (path.length() == 0)
	{
		path = FindFile(resource, "./");
	}
	
	return path;
}

std::string SoarHelper::FindFile(std::string filename, std::string path)
{
	DIR* dir = opendir(path.c_str());
	
	if (!dir)
		return "";
	
	struct dirent* entry;
	
	while ((entry = readdir(dir)) != nullptr)
	{
#ifndef _WIN32
		if (entry->d_type == DT_UNKNOWN)
		{
			struct stat file_info;
			
			lstat((path + std::string(entry->d_name)).c_str(), &file_info);
			
			if (S_ISDIR(file_info.st_mode))
			{
				entry->d_type = DT_DIR;
			}
			else if (S_ISREG(file_info.st_mode))
			{
				entry->d_type = DT_REG;
			}
		}
#endif
		
		if (entry->d_type == DT_DIR)
		{
			// Another directory
			std::string directory = entry->d_name;
			
			if (directory == "." || directory == "..")
				continue;
			
			std::string result = FindFile(filename, path + directory + "/");
			
			if (result.size() != 0)
			{
				closedir(dir);

				return result;
			}
		}
		else
		{
			std::string result = entry->d_name;
						
			if (filename == result)
			{
				closedir(dir);

				return path + result;
			}
		}
	}
	
	closedir(dir);
	
	return "";
}
