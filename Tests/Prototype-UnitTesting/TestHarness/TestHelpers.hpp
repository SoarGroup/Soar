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

class AssertException : public std::exception
{
public:
	/** Constructor (C strings).
	 *  @param message C-style string error message.
	 *                 The string contents are copied upon construction.
	 *                 Hence, responsibility for deleting the \c char* lies
	 *                 with the caller.
	 */
	explicit AssertException(const char* message, const char* file, const int line);
	
	/** Constructor (C++ STL strings).
	 *  @param message The error message.
	 */
	explicit AssertException(const std::string& message, const char* file, const int line);
	
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
	
	const char* file() const throw ();
	const int line() const throw ();
	
protected:
	/** Error message.
	 */
	std::string msg_;
	
	const char* file_;
	int line_;
};

#define assertEquals(X, Y) if (X != Y) \
{ \
	std::stringstream ss; \
	ss << "Assert: Expected equal values ("; \
	ss << X; \
	ss << ", "; \
	ss << Y; \
	ss << ") but was unequal."; \
	throw AssertException(ss.str(), __FILE__, __LINE__); \
	}

#define assertEquals_vector(X, Y) if (X != Y) \
{ \
	std::stringstream ss; \
	ss << "Assert: Expected equal values (["; \
	\
	for (int i = 0;i < X.size();++i)\
	{\
		ss << X[i];\
		\
		if ((i+1) != X.size())\
			ss << ", ";\
	}\
	\
	ss << "], [";\
	\
	for (int i = 0;i < Y.size();++i)\
	{\
		ss << Y[i];\
		\
		if ((i+1) != Y.size())\
			ss << ", ";\
	}\
	\
	ss << "]) but was unequal.";\
	throw AssertException(ss.str(), __FILE__, __LINE__);\
	}

#define assertNonZeroSize_msg(X, Y) if ((Y).size() == 0) \
{ \
throw AssertException(X, __FILE__, __LINE__); \
}

#define assertNonZeroSize(Y) if ((Y).size() == 0) \
{ \
throw AssertException("Assert: Expected container to be non-zero in size.", __FILE__, __LINE__); \
}

#define assertTrue(Y) if (!(Y)) \
{ \
throw AssertException("Assert: Boolean true check failed.", __FILE__, __LINE__); \
}

#define assertTrue_msg(X, Y) if (!(Y)) \
{ \
throw AssertException(std::string("Assert: ") + std::string(X), __FILE__, __LINE__); \
}

#define assertFalse(Y) if (Y) \
{ \
throw AssertException("Assert: Boolean false check failed.", __FILE__, __LINE__); \
}

#define assertFalse_msg(X, Y) if (Y) \
{ \
throw AssertException(std::string("Assert: ") + std::string(X), __FILE__, __LINE__); \
}

#define assertNotNull(Y) if (Y == nullptr) \
{ \
throw AssertException("Assert: Null pointer check failed.", __FILE__, __LINE__); \
}

#define assertNotNull_msg(X, Y) if (Y == nullptr) \
{ \
throw AssertException(std::string("Assert: ") + std::string(X), __FILE__, __LINE__); \
}

#endif /* TestHelpers_cpp */
