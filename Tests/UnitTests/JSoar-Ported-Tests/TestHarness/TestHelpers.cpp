//
//  TestHelpers.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "TestHelpers.hpp"

void assertTrue(std::string errorMessage, bool boolean)
{
	if (!boolean)
	{
		std::cerr << errorMessage << std::endl;
		assert(false);
	}
}

void assertFalse(std::string errorMessage, bool boolean)
{
	if (boolean)
	{
		std::cerr << errorMessage << std::endl;
		assert(false);
	}
}

void assertEquals(int64_t one, int64_t two)
{
	if (one != two)
	{
		assert(false);
	}
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
