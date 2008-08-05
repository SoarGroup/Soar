#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

template < class T >
bool from_string( T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&) )
{
	std::istringstream iss( s );
	return !( iss >> f >> t ).fail();
}

const double PI = 3.14159265358979323846;

inline double to_radians( double radians )
{
	return radians * ( PI / 180 );
}

inline double to_degrees( double degrees )
{
	return degrees * ( 180 / PI );
}

inline double to_absolute_yaw_soar( double yaw )
{
	// s = (PI / 2) - p
	yaw = (PI / 2) - yaw;
	
	// modulus 2 * PI
	yaw = fmod( yaw, 2 * PI );
	
	// convert to degrees
	yaw = to_degrees( yaw );
	
	assert( yaw >= 0 );
	assert( yaw < 360 );
	
	return yaw;
}

inline double to_absolute_yaw_player( double yaw )
{
	// p = (360 - s) + 90
	yaw = ( 360 - yaw ) + 90;
	
	// modulus 360
	yaw = fmod( yaw, 360 );
		
	// convert to radians
	yaw = to_radians( yaw );
	
	assert( yaw >= 0 );
	assert( yaw < ( 2 * PI ) );
	
	return yaw;
}

inline double to_relative_yaw_soar( double yaw )
{
	// flip the sign
	yaw *= -1;
	
	// convert to degrees
	yaw = to_degrees( yaw );
	
	return yaw;
}

inline double to_relative_yaw_player( double yaw )
{
	// flip the sign
	yaw *= -1;
	
	// convert to radians
	yaw = to_radians( yaw );
	
	return yaw;
}


#endif

