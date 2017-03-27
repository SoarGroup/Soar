//
//  SoarHelper.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef SoarHelper_cpp
#define SoarHelper_cpp


//#define INIT_AFTER_RUN                    // Tests for Soar identifier leaks on soar init
#define CONFIGURE_SOAR_FOR_UNIT_TESTS     // Insures debug printing and settings.soar file don't interfere
//#define SAVE_LOG_FILES                  // Make sure a log directory exists wherever unit tests are run
#define TURN_EXPLAINER_ON               // Turns the explainer on for all chunking unit tests
//#define NEVER_LEARN                     // Overrides learning settings in many unit tests
//#define ALWAYS_LEARN

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

    static bool source(sml::Agent* agent, const std::string& pCategoryName, const std::string& pTestName);
    static void init_check_to_find_refcount_leaks(sml::Agent* agent);
    static void check_learning_override(sml::Agent* agent);
    static void agent_command(sml::Agent* agent, const char* pCmd);
    static void add_log_dir_if_exists(std::string &lPath);
    static void start_log(sml::Agent* agent, const char* path);
    static void continue_log(sml::Agent* agent, const char* path);
    static void close_log(sml::Agent* agent);

private:
	static std::string FindFile(std::string filename, std::string path);
	
	static std::string getStats(sml::Agent* agent);
	static int parseForCount(std::string search, std::string countString);
};

std::ostream& operator<<(std::ostream& os, SoarHelper::StopPhase);

#endif /* SoarHelper_cpp */
