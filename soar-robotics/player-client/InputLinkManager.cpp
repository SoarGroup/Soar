#include "InputLinkManager.hxx"

#include <cassert>

using namespace sml;

InputLinkManager::InputLinkManager( Agent& agent )
: m_agent( agent )
{
	m_time = 0;
	m_self = 0;
	
	Identifier* il = m_agent.GetInputLink();
	assert( il );
	
	m_time = m_agent.CreateIdWME( il, "time" );
	{
		m_seconds = m_agent.CreateIntWME( m_time, "seconds", 0 );
		m_microseconds = m_agent.CreateIntWME( m_time, "microseconds", 0 );
	}
	
	m_self = m_agent.CreateIdWME( il, "self" );
	{
		m_agent.CreateStringWME( m_self, "name", m_agent.GetAgentName() );
		
		Identifier* current_location = m_agent.CreateIdWME( m_self, "current-location" );
		
		m_x = m_agent.CreateFloatWME( current_location, "x", 0 );
		m_y = m_agent.CreateFloatWME( current_location, "y", 0 );
		m_i = m_agent.CreateFloatWME( current_location, "i", 0 );
		m_j = m_agent.CreateFloatWME( current_location, "j", 0 );
		m_yaw = m_agent.CreateFloatWME( current_location, "yaw", 0 );
	}
	{
		Identifier* current_motion = m_agent.CreateIdWME( m_self, "current-motion" );

		m_motion_x = m_agent.CreateFloatWME( current_motion, "x", 0 );
		m_motion_y = m_agent.CreateFloatWME( current_motion, "y", 0 );
		m_motion_speed = m_agent.CreateFloatWME( current_motion, "speed", 0 );
		m_motion_yaw = m_agent.CreateFloatWME( current_motion, "yaw", 0 );
		m_motion_movement = m_agent.CreateStringWME( current_motion, "movement", "stop" );
		m_motion_rotation = m_agent.CreateStringWME( current_motion, "rotation", "stop" );
	}
	{
		Identifier* gripper = m_agent.CreateIdWME( m_self, "gripper" );
		{
			Identifier* beams = m_agent.CreateIdWME( gripper, "beams" );
			
			m_beams_outer = m_agent.CreateStringWME( beams, "outer", "unblocked" );
			m_beams_inner = m_agent.CreateStringWME( beams, "inner", "unblocked" );
		}
		
		m_gripper_open = m_agent.CreateStringWME( gripper, "open", "false" );
		m_gripper_closed = m_agent.CreateStringWME( gripper, "closed", "false" );
		m_gripper_moving = m_agent.CreateStringWME( gripper, "moving", "false" );
		m_gripper_error = m_agent.CreateStringWME( gripper, "error", "false" );
	}
	{
		m_received_messages = m_agent.CreateIdWME( m_self, "received-messages" );
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


