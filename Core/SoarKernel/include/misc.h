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

#include <iomanip>
#include <sstream>
#include <map>
#include <vector>
#include <string>

#include "symtab.h"

//////////////////////////////////////////////////////////
// String functions
//////////////////////////////////////////////////////////

// Conversion of value to string
template<class T> std::string *to_string( T &x )
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

// Conversion from string to value
template <class T> bool from_string( T &val, std::string str )
{
	std::stringstream i( str );
	return ( i >> val );
}

// Determine if a string represents a natural number (i.e. all numbers)
extern bool is_natural_number( std::string *str );

// Create a string that is n copies of src
extern std::string *string_multi_copy( const char *src, unsigned int n );

//////////////////////////////////////////////////////////
// Map functions
//////////////////////////////////////////////////////////

// get a list of all keys of a map
template <class X, class Y> std::vector<X> *map_keys( std::map<X,Y> *my_map )
{
	typename std::vector<X> *return_val = new std::vector<X>();
	typename std::map<X,Y>::iterator b, e;
	
	e = my_map->end();
	
	for ( b = my_map->begin(); b != e; b++ )
		return_val->push_back( b->first );
	
	return return_val;
}

// determine if a key is being used
template <class X, class Y> bool is_set( std::map<X,Y> *my_map, X *key )
{
	return ( my_map->find( *key ) != my_map->end() );
}

//////////////////////////////////////////////////////////
// Number functions
//////////////////////////////////////////////////////////

// get a numeric value from a symbol
extern double get_number_from_symbol( Symbol *sym );

// compares two numbers stored as void pointers
// used for qsort calls
template <class T>
T compare_num( const void *arg1, const void *arg2 )
{
    return *( (T *) arg1 ) - *( (T *) arg2 );
}

#endif /*MISC_H_*/
