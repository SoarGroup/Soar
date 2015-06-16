//
//  TestHelpers.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef TestHelpers_cpp
#define TestHelpers_cpp

#include <string>
#include <iostream>

#include <cassert>

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
            Member_Function(test_t * const &this_, void (test_t::*fun_)()) : m_this(this_), m_fun(fun_) {} \
            void operator()() {(m_this->*m_fun)();} \
        private: \
            test_t * m_this; \
            void (test_t::*m_fun)(); \
    };

#define TEST(X, Y) Test tXY = Test(#X, new Member_Function(this, &test_t::X), Y, m_TestCategory_tests);

#define TEST_DECLARATION(X) X* testX = new X(); tests.push_back(testX);

void assertTrue(std::string errorMessage, bool boolean);
void assertFalse(std::string errorMessage, bool boolean);
void assertEquals(int64_t one, int64_t two);

bool isfile(const char* path);

#endif /* TestHelpers_cpp */
