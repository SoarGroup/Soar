#include "InputLinkManager.h"

#include <assert.h>

using namespace sml;

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
}

