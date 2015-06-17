//
//  SoarHelper.hpp
//  Soar-xcode
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
	static int getD_CYCLE_COUNT(sml::Agent* agent);
	static int getE_CYCLE_COUNT(sml::Agent* agent);
	static int getPE_CYCLE_COUNT(sml::Agent* agent);
	static int getINNER_E_CYCLE_COUNT(sml::Agent* agent);
	
	static int getUserProductionCount(sml::Agent* agent);
	static int getChunkProductionCount(sml::Agent* agent);
	
private:
	static std::string getStats(sml::Agent* agent);
	static int parseForCount(std::string search, std::string countString);
};

#endif /* SoarHelper_cpp */
