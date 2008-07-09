#include "InputLinkManager.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <sys/time.h>

using namespace sml;

const double PI = 3.14159265;
// FIXME dead zones should be configurable
const double ROTATION_DEAD_ZONE_DEGREES = 0.5;
const double MOVEMENT_DEAD_ZONE = 0.001;

InputLinkManager::InputLinkManager( Agent& agent )
: m_agent( agent )
{
	m_time = 0;
	m_self = 0;
	
	Identifier* il = m_agent.GetInputLink();
	assert( il );
	
	m_time = m_agent.CreateFloatWME( il, "time", 0 );
	m_self = m_agent.CreateIdWME( il, "self" );

	{
		Identifier* current_location = m_agent.CreateIdWME( m_self, "current-location" );
		m_x = m_agent.CreateFloatWME( current_location, "x", 0 );
		m_y = m_agent.CreateFloatWME( current_location, "y", 0 );
		m_i = m_agent.CreateFloatWME( current_location, "i", 0 );
		m_j = m_agent.CreateFloatWME( current_location, "j", 0 );
		m_yaw = m_agent.CreateFloatWME( current_location, "yaw", 0 );
	}
	{
		Identifier* current_motion = m_agent.CreateIdWME( m_self, "current-motion" );
		{
			Identifier* concrete = m_agent.CreateIdWME( current_motion, "concrete" );
	
			m_motion_x = m_agent.CreateFloatWME( concrete, "x", 0 );
			m_motion_y = m_agent.CreateFloatWME( concrete, "y", 0 );
			m_motion_speed = m_agent.CreateFloatWME( concrete, "speed", 0 );
			m_motion_yaw = m_agent.CreateFloatWME( concrete, "yaw", 0 );
		}
		{
			Identifier* abstract = m_agent.CreateIdWME( current_motion, "abstract" );
		
			m_motion_movement = m_agent.CreateStringWME( abstract, "movement", "stop" );
			m_motion_rotation = m_agent.CreateStringWME( abstract, "rotation", "stop" );
		}
	}
	
	commit();
}

InputLinkManager::~InputLinkManager()
{
	if ( m_time )
	{
		m_agent.DestroyWME( m_time );
		m_time = 0;
	}
	
	if ( m_self )
	{
		m_agent.DestroyWME( m_self );
		m_self = 0;
	}
	
	commit();
}

void InputLinkManager::time_update( const timeval& time )
{
	double seconds = time.tv_sec;
	//TODO: seconds += time.tv_usec / 1000000;
	
	m_agent.Update( m_time, seconds );	
	//std::cout << "t(" << seconds << ")\n";
}

void InputLinkManager::position_update( double x, double y, double yaw )
{
	double yaw_degrees = yaw * 180 / PI;
	double i = 1;
	double j = 0;

	m_agent.Update( m_x, x );
	m_agent.Update( m_y, y );
	m_agent.Update( m_i, i );
	m_agent.Update( m_j, j );
	m_agent.Update( m_yaw, yaw_degrees );

	//std::cout << "p(" << x << "," << y << "," << i << "," << j << "," << yaw_degrees << ")\n";
}

void InputLinkManager::motion_update( double motion_x, double motion_y, double motion_yaw )
{
	double motion_yaw_degrees = motion_yaw * 180 / PI;

	double speed = pow( motion_x, 2 );
	speed += pow( motion_y, 2 );
	speed = sqrt( speed );

	const char* movement = 0;
	if ( speed < MOVEMENT_DEAD_ZONE )
	{
		movement = "stop";
	} 
	else 
	{
		movement = ( motion_x > 0 ) ? "forward" : "backward";
	}
	
	const char* rotation = 0;
	if ( fabs( motion_yaw_degrees ) < ROTATION_DEAD_ZONE_DEGREES )
	{
		rotation = "stop";
	} 
	else 
	{
		rotation = ( motion_yaw_degrees > 0 ) ? "left" : "right";
	}
	
	m_agent.Update( m_motion_x, motion_x );
	m_agent.Update( m_motion_y, motion_y );
	m_agent.Update( m_motion_speed, speed );
	m_agent.Update( m_motion_yaw, motion_yaw_degrees );
	m_agent.Update( m_motion_movement, movement );
	m_agent.Update( m_motion_rotation, rotation );
	
	//std::cout << "m(" << motion_x << "," << motion_y << "," << speed << "," << motion_yaw_degrees << "," << movement << "," << rotation << ")\n";
}

void InputLinkManager::commit()
{
	m_agent.Commit();
}

