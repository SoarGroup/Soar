//
//  assert.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef assert_cpp
#define assert_cpp

#include <exception>
#include <string>

#include "Export.h"

class EXPORT SoarAssertionException : public std::exception
{
public:
	/** Constructor (C strings).
	 *  @param message C-style string error message.
	 *                 The string contents are copied upon construction.
	 *                 Hence, responsibility for deleting the \c char* lies
	 *                 with the caller.
	 */
	explicit SoarAssertionException(const char* message, const char* file, const int line);
	
	/** Constructor (C++ STL strings).
	 *  @param message The error message.
	 */
	explicit SoarAssertionException(const std::string& message, const char* file, const int line);
	
	/** Destructor.
	 * Virtual to allow for subclassing.
	 */
	virtual ~SoarAssertionException() throw ();
	
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

#ifndef assert

#ifndef NDEBUG

#define assert(X) if (!(X)) throw SoarAssertionException("Failed assertion: '" + std::string(#X) + "'", __FILE__, __LINE__);

#else

#define assert(X)

#endif

#endif

#endif /* assert_cpp */
