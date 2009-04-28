/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  misc.h
 *
 * =======================================================================
 */

#ifndef MISC_H_
#define MISC_H_

#include <iomanip>
#include <sstream>
#include <string>

// Conversion of value to string
template<class T> std::string *to_string( const T &x )
{
	std::string *return_val;
	
	// instantiate stream
	std::ostringstream o;
	
	// get value into stream
	o << std::setprecision( 16 ) << x;
	
	// spit value back as string
	return_val = new std::string( o.str() );
	o.flush();

	return return_val;
}

// Conversion of value to string
template<class T> std::string& to_string( const T &x, std::string& dest )
{
	// instantiate stream
	std::ostringstream o;
	
	// get value into stream
	o << std::setprecision( 16 ) << x;
	
	// spit value back as string
	dest.assign( o.str() );
	o.flush();

	return dest;
}

// Conversion from string to value
template <class T> bool from_string( T &val, const std::string& str )
{
	std::stringstream i( str );
	i >> val;

	return !i.fail();
}

template <class T> bool from_string( T &val, const char* const pStr )
{
	return from_string( val, std::string( pStr ) );
}

template <class T> inline T cast_and_possibly_truncate( void * ptr )
{
	return static_cast<T>( reinterpret_cast<uintptr_t>( ptr ) );
}

#endif /*MISC_H_*/
