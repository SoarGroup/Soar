/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#include <sstream>
#include <map>
#include <vector>

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

//////////////////////////////////////////////////////////
// Map functions
//////////////////////////////////////////////////////////

// get a list of all keys of a map
template <class X, class Y> std::vector<X> *map_keys( std::map<X,Y> *my_map )
{
	typename std::vector<X> *return_val = new std::vector<X>();
	typename std::map<std::string,Y>::iterator b, e;
	
	e = my_map->end();
	
	for ( b = my_map->begin(); b != e; b++ )
		return_val->push_back( b->first );
	
	return return_val;
}


#endif /*MISC_H_*/
