//
//  TestHelpers.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "TestHelpers.hpp"

#ifdef _MSC_VER

#include <Windows.h>

#endif

#ifdef __GNUG__

#include <cstdlib>
#include <memory>
#include <cxxabi.h>

#endif

#include <unistd.h>
#include <exception>
#include <sstream>

bool isfile(const char* path)
{
#ifdef _WIN32
	DWORD a = GetFileAttributes(path);
	return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat st;
	return (stat(path, &st) == 0 && !S_ISDIR(st.st_mode));
#endif
}

void setCWDToEnv()
{
    if (getenv("LIBSOAR") != nullptr)
    {
        chdir(getenv("LIBSOAR"));
    }
}

void printDebugInformation(std::stringstream& output, sml::Agent* agent)
{
	if (!agent)
		return;
	
	output << "============================================================" << std::endl << std::endl;
	output << "Debug Information" << std::endl << std::endl;
	output << "============================================================" << std::endl << std::endl;
	
	output << "============================================================" << std::endl << std::endl;
	output << agent->ExecuteCommandLine("print -d 100 s1") << std::endl << std::endl;
	output << "============================================================" << std::endl << std::endl;
	
	output << "============================================================" << std::endl << std::endl;
	
	std::string rules = agent->ExecuteCommandLine("p");
	std::stringstream ss(rules);
	std::string line;
	
	while (std::getline(ss, line, '\n'))
	{
		output << "=======================" << std::endl;
		output << "matches " << line << ":" << std::endl << agent->ExecuteCommandLine(("matches " + line).c_str()) << std::endl << std::endl;
		output << "fc " << line << ":" << std::endl << agent->ExecuteCommandLine(("fc " + line).c_str()) << std::endl << std::endl;
	}
	output << "=======================" << std::endl;
	
	output << std::endl;
	
	output << agent->ExecuteCommandLine("matches") << std::endl << std::endl;
	output << "============================================================" << std::endl << std::endl;
	
	output << "============================================================" << std::endl << std::endl;
	output << agent->ExecuteCommandLine("stats") << std::endl << std::endl;
	output << "============================================================" << std::endl << std::endl;
}

namespace TestHelpers
{
	// Clang++ & G++
#ifdef __GNUG__

	std::string demangle(const char* name) {
		
		int status = -1;
		
		std::unique_ptr<char, void(*)(void*)> res {
			abi::__cxa_demangle(name, NULL, NULL, &status),
			std::free
		};
		
		return (status==0) ? res.get() : name ;
	}

	// VS2013 +
#elif _MSC_VER
	
	std::string demangle(const char* name)
	{
		std::string result = name;
		result = result.substr(result.find_last_of(" ")+1, std::string::npos);
		
		return result;
	}

	// Unknown
#else
	
	// does nothing if not g++
	std::string demangle(const char* name)
	{
		return name;
	}
	
#endif
	
};
