#ifndef INPUT_LINK_MANAGER_HXX
#define INPUT_LINK_MANAGER_HXX

#include "InputLinkManager.h"

#include "Message.h"

#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <exception>

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
	std::cout << "f(" << id << "," << x << "," << y << ")" << std::endl;
}

void InputLinkManager::add_message( const Message& message )
{
	sml::Identifier* message_identifier = m_agent.CreateIdWME( m_received_messages, "message" );
	
	m_agent.CreateStringWME( message_identifier, "from", message.from().c_str() );
	m_agent.CreateIntWME( message_identifier, "id", message.id() );
	
	sml::Identifier* time = m_agent.CreateIdWME( message_identifier, "time" );
	m_agent.CreateIntWME( time, "seconds", m_seconds->GetValue() );
	m_agent.CreateIntWME( time, "microseconds", m_microseconds->GetValue() );
	
	sml::Identifier* parent_id_wme = message_identifier;
	bool first = true;
	
	for ( std::list< std::string >::const_iterator iter = message.begin(); iter != message.end(); ++iter )
	{
		parent_id_wme = m_agent.CreateIdWME( parent_id_wme, first ? "first" : "next" );
		first = false;
		m_agent.CreateStringWME( parent_id_wme, "word", (*iter).c_str() );
	}
	
	if ( first )
	{
		throw std::exception();
	}
	
	m_agent.CreateStringWME( parent_id_wme, "next", "nil" );
	
	m_message_map[ message.id() ] = message_identifier;
}

void InputLinkManager::remove_message( int id )
{
	std::map< int, sml::Identifier* >::iterator iter = m_message_map.find( id );
	if ( iter == m_message_map.end() )
	{
		// TODO: report error
		return;
	}
	m_agent.DestroyWME( iter->second );
	m_message_map.erase( iter );
}

void InputLinkManager::commit()
{
	m_agent.Commit();
}

#endif // INPUT_LINK_MANAGER_HXX

