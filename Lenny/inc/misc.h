
#ifndef MISC_H
#define MISC_H

#include <iomanip>
#include <sstream>
#include <string>
#include <cstdio>

// Conversion of value to string
template<class T> std::string& toString( const T& x, std::string& dest )
{
	static std::ostringstream o;
	
	// get value into stream
	o << std::setprecision( 16 ) << x;
	
	dest.assign( o.str() );
	o.str("");
	return dest;
}

#endif