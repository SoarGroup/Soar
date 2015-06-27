//
//  assert.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "assert.hpp"

SoarAssertionException::SoarAssertionException(const char* message, const char* file, const int line):
msg_(message),
file_(file),
line_(line)
{
}

/** Constructor (C++ STL strings).
 *  @param message The error message.
 */
SoarAssertionException::SoarAssertionException(const std::string& message, const char* file, const int line):
msg_(message),
file_(file),
line_(line)
{}

/** Destructor.
 * Virtual to allow for subclassing.
 */
SoarAssertionException::~SoarAssertionException() throw (){}

/** Returns a pointer to the (constant) error description.
 *  @return A pointer to a \c const \c char*. The underlying memory
 *          is in posession of the \c Exception object. Callers \a must
 *          not attempt to free the memory.
 */
const char* SoarAssertionException::what() const throw (){
	return msg_.c_str();
}

const char* SoarAssertionException::file() const throw (){
	return file_;
}

const int SoarAssertionException::line() const throw (){
	return line_;
}