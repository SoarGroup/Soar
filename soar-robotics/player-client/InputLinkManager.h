#ifndef INPUT_LINK_MANAGER_H
#define INPUT_LINK_MANAGER_H

#include "sml_Client.h"

struct timeval;

class InputLinkManager
{
public:
	InputLinkManager( sml::Agent& agent );
	~InputLinkManager();
	
	void time_update( const timeval& time );
	void position_update( double x, double y, double yaw );
	void motion_update( double motion_x, double motion_y, double motion_yaw );

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
};

#endif // INPUT_LINK_MANAGER_H

