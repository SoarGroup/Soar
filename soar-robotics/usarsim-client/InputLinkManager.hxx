#ifndef INPUT_LINK_MANAGER_HXX
#define INPUT_LINK_MANAGER_HXX

#include "InputLinkManager.h"

#include "Message.h"
#include "utility.h"

#include <iostream>
#include <sys/time.h>
#include <exception>

const double InputLinkManager::ROTATION_DEAD_ZONE_DEGREES = 0.5;	// FIXME dead zones should be configurable
const double InputLinkManager::MOVEMENT_DEAD_ZONE = 0.001;			// FIXME dead zones should be configurable

void Entity::position_update( double relative_x, double relative_y, double absolute_x, double absolute_y )
{
	if ( m_visible_wme->GetValue() == std::string( "false" ) )
	{
		m_agent->Update( m_visible_wme, "true" );
	}

	m_agent->Update( m_relative_x, relative_x );
	m_agent->Update( m_relative_y, relative_y );

	m_agent->Update( m_absolute_x, absolute_x );
	m_agent->Update( m_absolute_y, absolute_y );
}

void Entity::lost_contact()
{
	if ( m_visible_wme->GetValue() == std::string( "true" ) )
	{
		m_agent->Update( m_visible_wme, "false" );
	}
	// FIXME: remove after 2 sec
}

void InputLinkManager::time_update( const timeval& time )
{
	m_agent.Update( m_seconds, time.tv_sec );	
	m_agent.Update( m_microseconds, time.tv_usec );	
	//std::cout << "t(" << time.tv_sec << "." << time.tv_usec / 1000000.0 << ")\n";
}

void InputLinkManager::position_update( double x, double y, double yaw )
{
	// convert to soar yaw
	yaw = to_absolute_yaw_soar( yaw );
	
	double i = 1;
	double j = 0;

	m_agent.Update( m_x, x );
	m_agent.Update( m_y, y );
	m_agent.Update( m_i, i );
	m_agent.Update( m_j, j );
	m_agent.Update( m_yaw, yaw );

	//std::cout << "p(" << x << "," << y << "," << i << "," << j << "," << yaw << ")\n";
}

void InputLinkManager::motion_update( double motion_x, double motion_y, double motion_yaw )
{
	// convert to degrees
	motion_yaw = to_relative_yaw_soar( motion_yaw );
	
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
	if ( fabs( motion_yaw ) < ROTATION_DEAD_ZONE_DEGREES )
	{
		rotation = "stop";
	} 
	else 
	{
		rotation = ( motion_yaw > 0 ) ? "right" : "left";
	}
	
	m_agent.Update( m_motion_x, motion_x );
	m_agent.Update( m_motion_y, motion_y );
	m_agent.Update( m_motion_speed, speed );
	m_agent.Update( m_motion_yaw, motion_yaw );
	m_agent.Update( m_motion_movement, movement );
	m_agent.Update( m_motion_rotation, rotation );
	
	//std::cout << "m(" << motion_x << "," << motion_y << "," << speed << "," << motion_yaw << "," << movement << "," << rotation << ")\n";
}

void InputLinkManager::clear_expired_fiducials()
{
	// clear untouched feducials
	m_unseen_entities_map = m_entities_map;	
}

void InputLinkManager::feducial_update( int id, double x, double y )
{
	// calculate absolute position
	// rotate relative on to absolute system:
	// x' = x * cos(a) - y * sin(a)
	// y' = x * sin(a) + y * cos(a)
	// then translate by robot position
	// FIXME: this may be off a bit because the relative location is relative to the sensor position on the bot
	
	const double yaw = to_absolute_yaw_player( m_yaw->GetValue() );
	
	double absolute_x = ( x * cos( yaw ) ) - ( y * sin( yaw ) );
	absolute_x += m_x->GetValue();

	double absolute_y = ( x * sin( yaw ) ) + ( y * cos( yaw ) );
	absolute_y += m_y->GetValue();
	
	// have we seen this id?
	std::map< int, Entity* >::iterator iter = m_entities_map.find( id );
	if ( iter != m_entities_map.end() )
	{
		iter->second->position_update( x, y, absolute_x, absolute_y );
		m_unseen_entities_map.erase( iter->first );
	}
	else
	{
		m_entities_map[ id ] = new Entity( &m_agent, m_entities, id, x, y, absolute_x, absolute_y );
	}
}

void InputLinkManager::update_expired_fiducials()
{
	// call lost_contact on all untouched feducials
	for ( std::map< int, Entity* >::iterator iter = m_unseen_entities_map.begin(); iter != m_unseen_entities_map.end(); ++iter )
	{
		iter->second->lost_contact();
	}
		
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

