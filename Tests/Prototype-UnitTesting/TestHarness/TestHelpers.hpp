//
//  TestHelpers.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef TestHelpers_cpp
#define TestHelpers_cpp

#include <string>
#include <sstream>
#include <vector>
#include <iostream>

#include <cassert>

#include "sml_ClientAgent.h"

#include "assert.hpp"

#ifndef _WIN32
#include <sys/stat.h>
#endif

class TestFunction
{
public:
	virtual ~TestFunction() {}
	virtual void operator()() = 0;
};

#define TEST_CATEGORY(X) X() : TestCategory(#X) {} \
	typedef X test_t; \
	class Member_Function : public TestFunction { \
        public: \
            Member_Function(test_t * const this_, void (test_t::*fun_)()) : m_this(this_), m_fun(fun_) {} \
            void operator()() {(m_this->*m_fun)();} \
        private: \
            test_t * m_this; \
            void (test_t::*m_fun)(); \
    };

#define TEST_DECLARATION(X) tests.push_back(new X ());
#define TEST(X, Y) Test t_##X = Test(#X, new Member_Function(this, &test_t::X), Y, m_TestCategory_tests);

bool isfile(const char* path);

void printDebugInformation(std::stringstream& output, sml::Agent* agent);

#define assertEquals(X, Y) if (X != Y) \
{ \
	std::stringstream ss; \
	ss << "Assert: Expected equal values ("; \
	ss << X; \
	ss << ", "; \
	ss << Y; \
	ss << ") but was unequal."; \
	printDebugInformation(runner->output, agent); \
	throw SoarAssertionException(ss.str(), __FILE__, __LINE__); \
	}

#define assertEquals_vector(X, Y) if (X != Y) \
{ \
	std::stringstream ss; \
	ss << "Assert: Expected equal values (["; \
	\
	for (size_t i = 0;i < X.size();++i)\
	{\
		ss << X[i];\
		\
		if ((i+1) != X.size())\
			ss << ", ";\
	}\
	\
	ss << "], [";\
	\
	for (size_t i = 0;i < Y.size();++i)\
	{\
		ss << Y[i];\
		\
		if ((i+1) != Y.size())\
			ss << ", ";\
	}\
	\
	ss << "]) but was unequal.";\
	printDebugInformation(runner->output, agent); \
	throw SoarAssertionException(ss.str(), __FILE__, __LINE__);\
	}

#define assertNonZeroSize_msg(X, Y) if ((Y).size() == 0) \
{ \
printDebugInformation(runner->output, agent); \
throw SoarAssertionException(X, __FILE__, __LINE__); \
}

#define assertNonZeroSize(Y) if ((Y).size() == 0) \
{ \
printDebugInformation(runner->output, agent); \
throw SoarAssertionException("Assert: Expected container to be non-zero in size.", __FILE__, __LINE__); \
}

#define assertTrue(Y) if (!(Y)) \
{ \
printDebugInformation(runner->output, agent); \
throw SoarAssertionException("Assert: Boolean true check failed.", __FILE__, __LINE__); \
}

#define assertTrue_msg(X, Y) if (!(Y)) \
{ \
printDebugInformation(runner->output, agent); \
throw SoarAssertionException(std::string("Assert: ") + std::string(X), __FILE__, __LINE__); \
}

#define assertFalse(Y) if (Y) \
{ \
printDebugInformation(runner->output, agent); \
throw SoarAssertionException("Assert: Boolean false check failed.", __FILE__, __LINE__); \
}

#define assertFalse_msg(X, Y) if (Y) \
{ \
printDebugInformation(runner->output, agent); \
throw SoarAssertionException(std::string("Assert: ") + std::string(X), __FILE__, __LINE__); \
}

#define assertNotNull(Y) if (Y == nullptr) \
{ \
printDebugInformation(runner->output, agent); \
throw SoarAssertionException("Assert: Null pointer check failed.", __FILE__, __LINE__); \
}

#define assertNotNull_msg(X, Y) if (Y == nullptr) \
{ \
printDebugInformation(runner->output, agent); \
throw SoarAssertionException(std::string("Assert: ") + std::string(X), __FILE__, __LINE__); \
}

#define no_agent_assertEquals(X, Y) if (X != Y) \
{ \
	std::stringstream ss; \
	ss << "Assert: Expected equal values ("; \
	ss << X; \
	ss << ", "; \
	ss << Y; \
	ss << ") but was unequal."; \
	throw SoarAssertionException(ss.str(), __FILE__, __LINE__); \
	}

#define no_agent_assertEquals_vector(X, Y) if (X != Y) \
{ \
	std::stringstream ss; \
	ss << "Assert: Expected equal values (["; \
	\
	for (size_t i = 0;i < X.size();++i)\
	{\
		ss << X[i];\
		\
		if ((i+1) != X.size())\
			ss << ", ";\
	}\
	\
	ss << "], [";\
	\
	for (size_t i = 0;i < Y.size();++i)\
	{\
		ss << Y[i];\
		\
		if ((i+1) != Y.size())\
			ss << ", ";\
	}\
	\
	ss << "]) but was unequal.";\
	throw SoarAssertionException(ss.str(), __FILE__, __LINE__);\
	}

#define no_agent_assertNonZeroSize_msg(X, Y) if ((Y).size() == 0) \
{ \
throw SoarAssertionException(X, __FILE__, __LINE__); \
}

#define no_agent_assertNonZeroSize(Y) if ((Y).size() == 0) \
{ \
throw SoarAssertionException("Assert: Expected container to be non-zero in size.", __FILE__, __LINE__); \
}

#define no_agent_assertTrue(Y) if (!(Y)) \
{ \
throw SoarAssertionException("Assert: Boolean true check failed.", __FILE__, __LINE__); \
}

#define no_agent_assertTrue_msg(X, Y) if (!(Y)) \
{ \
throw SoarAssertionException(std::string("Assert: ") + std::string(X), __FILE__, __LINE__); \
}

#define no_agent_assertFalse(Y) if (Y) \
{ \
throw SoarAssertionException("Assert: Boolean false check failed.", __FILE__, __LINE__); \
}

#define no_agent_assertFalse_msg(X, Y) if (Y) \
{ \
throw SoarAssertionException(std::string("Assert: ") + std::string(X), __FILE__, __LINE__); \
}

#define no_agent_assertNotNull(Y) if (Y == nullptr) \
{ \
throw SoarAssertionException("Assert: Null pointer check failed.", __FILE__, __LINE__); \
}

#define no_agent_assertNotNull_msg(X, Y) if (Y == nullptr) \
{ \
throw SoarAssertionException(std::string("Assert: ") + std::string(X), __FILE__, __LINE__); \
}

#endif /* TestHelpers_cpp */
