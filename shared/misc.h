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
template<class T> std::string& to_string( const T& x, std::string& dest )
{
	static std::ostringstream o;
	
	// get value into stream
	o << std::setprecision( 16 ) << x;
	
	dest.assign( o.str() );
	o.str("");
	return dest;
}

// Conversion from string to value
template <class T> bool from_string( T& val, const std::string& str )
{
	std::istringstream i( str );
	i >> val;
	return !i.fail();
}

template <class T> bool from_string( T& val, const char* const pStr )
{
	return from_string( val, std::string( pStr ) );
}

template <class T> inline T cast_and_possibly_truncate( void* ptr )
{
	return static_cast<T>( reinterpret_cast<uintptr_t>( ptr ) );
}

// These functions have proven to be much faster than the c++ style ones above.
// TO
const size_t TO_C_STRING_BUFSIZE = 24; // uint64: 18446744073709551615 plus a few extra
inline const char* const to_c_string( const char& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hhi", v ); 
	return buf; 
}
inline const char* const to_c_string( const unsigned char& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hhu", v ); 
	return buf; 
}
inline const char* const to_c_string( const short& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hd", v ); 
	return buf; 
}
inline const char* const to_c_string( const unsigned short& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hu", v ); 
	return buf; 
}
inline const char* const to_c_string( const int& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%d", v ); 
	return buf; 
}
inline const char* const to_c_string( const unsigned int& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%u", v ); 
	return buf; 
}
inline const char* const to_c_string( const long int& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%ld", v ); 
	return buf; 
}
inline const char* const to_c_string( const unsigned long& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%lu", v ); 
	return buf; 
}
inline const char* const to_c_string( const float& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%f", v ); 
	return buf; 
}
inline const char* const to_c_string( const double& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%lf", v ); 
	return buf; 
}
inline const char* const to_c_string( const long double& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%Lf", v ); 
	return buf; 
}

// FROM
inline bool from_c_string( char& v, const char* const str ) 
{ 
	return sscanf( str, "%hhd", &v ) == 1; 
}
inline bool from_c_string( unsigned char& v, const char* const str ) 
{ 
	return sscanf( str, "%hhu", &v ) == 1; 
}
inline bool from_c_string( short& v, const char* const str ) 
{ 
	return sscanf( str, "%hd", &v ) == 1; 
}
inline bool from_c_string( unsigned short& v, const char* const str ) 
{ 
	return sscanf( str, "%hu", &v ) == 1; 
}
inline bool from_c_string( int& v, const char* const str ) 
{ 
	//v = atoi(str);
	return sscanf( str, "%d", &v ) == 1; 
}
inline bool from_c_string( unsigned int& v, const char* const str ) 
{ 
	return sscanf( str, "%u", &v ) == 1; 
}
inline bool from_c_string( long& v, const char* const str ) 
{ 
	//v = atol(str);
	return sscanf( str, "%ld", &v ) == 1; 
}
inline bool from_c_string( unsigned long& v, const char* const str ) 
{ 
	//v = strtoul(str, NULL, 10);
	return sscanf( str, "%lu", &v ) == 1; 
}
inline bool from_c_string( float& v, const char* const str ) 
{
	//v = strtof(str, NULL);
	return sscanf( str, "%f", &v ) == 1; 
}
inline bool from_c_string( double& v, const char* const str ) 
{
	//v = strtod(str, NULL);
	return sscanf( str, "%lf", &v ) == 1; 
}

inline bool from_c_string( long double& v, const char* const str ) 
{
	//v = strtold(str, NULL);
	return sscanf( str, "%Lf", &v ) == 1; 
}

#endif /*MISC_H_*/
