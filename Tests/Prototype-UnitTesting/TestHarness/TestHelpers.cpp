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

#include <exception>
#include <sstream>

AssertException::AssertException(const char* message, const char* file, const int line):
msg_(message),
file_(file),
line_(line)
{
}

/** Constructor (C++ STL strings).
 *  @param message The error message.
 */
AssertException::AssertException(const std::string& message, const char* file, const int line):
msg_(message),
file_(file),
line_(line)
{}

/** Destructor.
 * Virtual to allow for subclassing.
 */
AssertException::~AssertException() throw (){}

/** Returns a pointer to the (constant) error description.
 *  @return A pointer to a \c const \c char*. The underlying memory
 *          is in posession of the \c Exception object. Callers \a must
 *          not attempt to free the memory.
 */
const char* AssertException::what() const throw (){
	return msg_.c_str();
}

const char* AssertException::file() const throw (){
	return file_;
}

const int AssertException::line() const throw (){
	return line_;
}

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

void printDebugInformation(std::stringstream& output, sml::Agent* agent)
{
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
