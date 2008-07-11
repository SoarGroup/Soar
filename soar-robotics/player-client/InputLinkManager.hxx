#ifndef INPUT_LINK_MANAGER_HXX
#define INPUT_LINK_MANAGER_HXX

#include "InputLinkManager.h"

#include <iostream>
#include <cmath>
#include <sys/time.h>

const double InputLinkManager::PI = 3.14159265;
const double InputLinkManager::ROTATION_DEAD_ZONE_DEGREES = 0.5;	// FIXME dead zones should be configurable
const double InputLinkManager::MOVEMENT_DEAD_ZONE = 0.001;			// FIXME dead zones should be configurable

void InputLinkManager::time_update( const timeval& time )
{
	m_agent.Update( m_seconds, time.tv_sec );	
	m_agent.Update( m_microseconds, time.tv_usec );	
	//std::cout << "t(" << time.tv_sec << "." << time.tv_usec / 1000000.0 << ")\n";
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

void InputLinkManager::feducial_update( int id, double x, double y )
{
	//std::cout << "f(" << id << "," << x << "," << y << ")\n";
}

void InputLinkManager::beam_update( bool outer, bool inner )
{
	m_agent.Update( m_beams_outer, outer ? "blocked" : "unblocked" );
	m_agent.Update( m_beams_inner, inner ? "blocked" : "unblocked" );

	//std::cout << "gb(" << outer << "," << inner << ")\n";
}

void InputLinkManager::gripper_update( bool open, bool closed, bool moving, bool error )
{
	m_agent.Update( m_gripper_open, open ? "true" : "false" );
	m_agent.Update( m_gripper_closed, closed ? "true" : "false" );
	m_agent.Update( m_gripper_moving, moving ? "true" : "false" );
	m_agent.Update( m_gripper_error, error ? "true" : "false" );

	//std::cout << "gg(" << open << "," << closed << "," << moving << "," << error << ")\n";
}

void InputLinkManager::commit()
{
	m_agent.Commit();
}

#endif // INPUT_LINK_MANAGER_HXX

