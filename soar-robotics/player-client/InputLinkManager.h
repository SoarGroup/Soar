#ifndef INPUT_LINK_MANAGER_H
#define INPUT_LINK_MANAGER_H

#include "sml_Client.h"

class InputLinkManager
{
public:
	InputLinkManager( sml::Agent& agent );
	~InputLinkManager();

private:
	sml::Agent& m_agent;
	
	sml::FloatElement* m_time;
	
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

