#include "InputLinkManager.hxx"

#include <cassert>

using namespace sml;

Entity::Entity( sml::Agent* agent, sml::Identifier* m_entities_parent, int id, 
	double relative_x, double relative_y, double absolute_x, double absolute_y )
{
	m_agent = agent;
	
	m_entity_wme = m_agent->CreateIdWME( m_entities_parent, "entity" );
	assert( m_entity_wme );
	
	m_agent->CreateIntWME( m_entity_wme, "id", id );
	m_visible_wme = m_agent->CreateStringWME( m_entity_wme, "visible", "true" );
	m_agent->CreateStringWME( m_entity_wme, "friendly", "false" );
	
	{
		Identifier* relative_location = m_agent->CreateIdWME( m_entity_wme, "relative-location" );
		m_relative_x = m_agent->CreateFloatWME( relative_location, "x", relative_x );
		m_relative_y = m_agent->CreateFloatWME( relative_location, "y", relative_y );
	}
	{
		Identifier* absolute_location = m_agent->CreateIdWME( m_entity_wme, "absolute-location" );
		m_absolute_x = m_agent->CreateFloatWME( absolute_location, "x", absolute_x );
		m_absolute_y = m_agent->CreateFloatWME( absolute_location, "y", absolute_y );
	}
}

Entity::~Entity()
{
	m_agent->DestroyWME( m_entity_wme );
}

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
		
		Identifier* current_location = m_agent.CreateIdWME( m_self, "location" );
		
		m_x = m_agent.CreateFloatWME( current_location, "x", 0 );
		m_y = m_agent.CreateFloatWME( current_location, "y", 0 );
		m_i = m_agent.CreateFloatWME( current_location, "i", 0 );
		m_j = m_agent.CreateFloatWME( current_location, "j", 0 );
		m_yaw = m_agent.CreateFloatWME( current_location, "yaw", 0 );
	}
	{
		Identifier* current_motion = m_agent.CreateIdWME( m_self, "motion" );

		m_motion_x = m_agent.CreateFloatWME( current_motion, "x", 0 );
		m_motion_y = m_agent.CreateFloatWME( current_motion, "y", 0 );
		m_motion_speed = m_agent.CreateFloatWME( current_motion, "speed", 0 );
		m_motion_yaw = m_agent.CreateFloatWME( current_motion, "yaw", 0 );
		m_motion_movement = m_agent.CreateStringWME( current_motion, "movement", "stop" );
		m_motion_rotation = m_agent.CreateStringWME( current_motion, "rotation", "stop" );
	}
	{
		m_received_messages = m_agent.CreateIdWME( m_self, "received-messages" );
	}	
	m_entities = m_agent.CreateIdWME( il, "entities" );
	commit();
}

InputLinkManager::~InputLinkManager()
{
	for ( std::map< int, Entity* >::iterator iter = m_entities_map.begin(); iter != m_entities_map.end(); ++iter )
	{
		delete iter->second;
	}

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
	
	if ( m_entities )
	{
		m_agent.DestroyWME( m_entities );
		m_entities = 0;
	}
	
	commit();
}


