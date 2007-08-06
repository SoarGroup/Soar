/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  string_tofrom.h
 *
 * =======================================================================
 */

#ifndef STRING_TOFROM_H_
#define STRING_TOFROM_H_

#include <sstream>

//////////////////////////////////////////////////////////
// String conversion functions
//////////////////////////////////////////////////////////

// Conversion of value to string
template<class T> std::string to_string( T &x )
{
	// instantiate stream
	std::ostringstream o;
	
	// get value into stream
	o << x;
	
	// spit value back as string
	return o.str();
}

// Conversion from string to value
template <class T> bool from_string( T &val, std::string str )
{
	std::stringstream i( str );
	return ( i >> val );
}

#endif /*STRING_TOFROM_H_*/
