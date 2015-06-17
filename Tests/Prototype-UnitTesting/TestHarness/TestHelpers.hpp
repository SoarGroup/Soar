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

#define TEST(X, Y) Test t_##X = Test(#X, new Member_Function(this, &test_t::X), Y, m_TestCategory_tests);

#define TEST_DECLARATION(X) X* testX = new X(); tests.push_back(testX);

void assertTrue(std::string errorMessage, bool boolean);
void assertTrue(bool boolean);

void assertFalse(std::string errorMessage, bool boolean);
void assertFalse(bool boolean);

void assertEquals(int64_t one, int64_t two);

bool isfile(const char* path);

class AssertException : public std::exception
{
public:
	/** Constructor (C strings).
	 *  @param message C-style string error message.
	 *                 The string contents are copied upon construction.
	 *                 Hence, responsibility for deleting the \c char* lies
	 *                 with the caller.
	 */
	explicit AssertException(const char* message);
	
	/** Constructor (C++ STL strings).
	 *  @param message The error message.
	 */
	explicit AssertException(const std::string& message);
	
	/** Destructor.
	 * Virtual to allow for subclassing.
	 */
	virtual ~AssertException() throw ();
	
	/** Returns a pointer to the (constant) error description.
	 *  @return A pointer to a \c const \c char*. The underlying memory
	 *          is in posession of the \c Exception object. Callers \a must
	 *          not attempt to free the memory.
	 */
	virtual const char* what() const throw ();
	
protected:
	/** Error message.
	 */
	std::string msg_;
};


#endif /* TestHelpers_cpp */
