//
//  TestHelpers.cpp
//  Soar-xcode
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

AssertException::AssertException(const char* message):
msg_(message)
{
}

/** Constructor (C++ STL strings).
 *  @param message The error message.
 */
AssertException::AssertException(const std::string& message):
msg_(message)
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

void assertTrue(bool boolean)
{
	return assertTrue("Boolean true check failed.", boolean);
}

void assertTrue(std::string errorMessage, bool boolean)
{
	if (!boolean)
	{
		throw AssertException("Assert: " + errorMessage);
	}
}

void assertFalse(bool boolean)
{
	return assertFalse("Boolean false check failed.", boolean);
}

void assertFalse(std::string errorMessage, bool boolean)
{
	if (boolean)
	{
		throw AssertException("Assert: " + errorMessage);
	}
}

void assertNotNull(std::string errorMessage, void* pointer)
{
	if (pointer == nullptr)
	{
		throw AssertException("Assert: " + errorMessage);
	}
}

void assertNotNull(void* pointer)
{
	return assertNotNull("Null pointer check failed.", pointer);
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
