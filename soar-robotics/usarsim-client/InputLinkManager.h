#ifndef INPUT_LINK_MANAGER_H
#define INPUT_LINK_MANAGER_H

#include "sml_Client.h"

struct timeval;
class Message;

class Entity
{
public:
	Entity( sml::Agent* agent, sml::Identifier* m_entities_parent, int id, 
		double relative_x, double relative_y, double absolute_x, double absolute_y );
	~Entity();
	
	void position_update( double relative_x, double relative_y, double absolute_x, double absolute_y );
	void lost_contact();
	
private:
	sml::Agent* m_agent;

	sml::Identifier* m_entity_wme;
	sml::StringElement* m_visible_wme;
	sml::FloatElement* m_absolute_x;
	sml::FloatElement* m_absolute_y;
	sml::FloatElement* m_relative_x;
	sml::FloatElement* m_relative_y;
};

class InputLinkManager
{
public:
	InputLinkManager( sml::Agent& agent );
	~InputLinkManager();
	
	void time_update( const timeval& time );
	void position_update( double x, double y, double yaw_player );
	void motion_update( double motion_x, double motion_y, double motion_yaw_player );
	void clear_expired_fiducials();
	void feducial_update( int id, double x, double y );
	void update_expired_fiducials();
	void add_message( const Message& message );
	void remove_message( int id );

	void commit();

private:
	sml::Agent& m_agent;
	
	sml::Identifier* m_time;
	sml::IntElement* m_seconds;
	sml::IntElement* m_microseconds;
	
	sml::Identifier* m_self;
	sml::FloatElement* m_x;
	sml::FloatElement* m_y;
	sml::FloatElement* m_i;
	sml::FloatElement* m_j;
	sml::FloatElement* m_yaw;
	sml::FloatElement* m_motion_x;
	sml::FloatElement* m_motion_y;
	sml::FloatElement* m_motion_speed;
	sml::FloatElement* m_motion_yaw;
	sml::StringElement* m_motion_movement;
	sml::StringElement* m_motion_rotation;
	sml::Identifier* m_received_messages;
	sml::Identifier* m_entities;
	
	std::map< int, sml::Identifier* > m_message_map;
	std::map< int, Entity* > m_entities_map;
	std::map< int, Entity* > m_unseen_entities_map;

	static const double ROTATION_DEAD_ZONE_DEGREES;
	static const double MOVEMENT_DEAD_ZONE;
};

#endif // INPUT_LINK_MANAGER_H

