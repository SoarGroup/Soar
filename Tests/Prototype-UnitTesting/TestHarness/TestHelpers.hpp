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
#include <sstream>
#include <vector>
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

#define TEST_DECLARATION(X) tests.push_back(new X ());

void assertTrue(std::string errorMessage, bool boolean);
void assertTrue(bool boolean);

void assertFalse(std::string errorMessage, bool boolean);
void assertFalse(bool boolean);

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

template<class T>
void assertEquals(T one, T two)
{
	if (one != two)
	{
		std::stringstream ss;
		ss << "Assert: Expected equal values (";
		ss << one;
		ss << ", ";
		ss << two;
		ss << ") but was unequal.";
		throw AssertException(ss.str());
	}
}

template<class T>
void assertEquals(std::vector<T> one, std::vector<T> two)
{
	if (one != two)
	{
		std::stringstream ss;
		ss << "Assert: Expected equal values ([";
		
		auto outputter = [](std::vector<T> one) {
			std::stringstream result;
			
			for (int i = 0;i < one.size();++i)
			{
				result << one[i];
				
				if ((i+1) != one.size())
					result << ", ";
			}
			
			return result;
		};
		
		ss << outputter(one).str();
		
		ss << "], [";
		
		ss << outputter(two).str();
		
		ss << "]) but was unequal.";
		throw AssertException(ss.str());
	}
}

#endif /* TestHelpers_cpp */
