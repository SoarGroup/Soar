#ifndef OUTPUT_LINK_MANAGER_H
#define OUTPUT_LINK_MANAGER_H

#include "sml_Client.h"

class Command
{
public:
	Command() {}
	Command( sml::Identifier* commandId );

	enum CommandType { MOVE, ROTATE, STOP };
	CommandType 	get_type() { return m_type; }

	enum MoveDirection { MOVE_STOP, MOVE_FORWARD, MOVE_BACKWARD };
	MoveDirection 	get_move_direction() { return m_move; }

	enum RotateDirection { ROTATE_STOP, ROTATE_LEFT, ROTATE_RIGHT };
	RotateDirection get_rotate_direction() { return m_rotate; }

	double 			get_throttle() { return m_throttle; }
	
	enum Status { STATUS_NONE, STATUS_COMPLETE, STATUS_EXECUTING, STATUS_ERROR };
	Status 			get_status() { return m_status; }
	void 			set_status( Status status ) { m_status = status; }
	
private:
	CommandType m_type;
	MoveDirection m_move;
	RotateDirection m_rotate;
	double m_throttle;
	Status m_status;
};

class OutputLinkManager
{
public:
	OutputLinkManager( sml::Agent& agent );
	~OutputLinkManager();

	void read();
	Command* get_next_command();

	void commit();

private:
	std::list< std::pair< Command, sml::Identifier* > > m_command_list;
	std::list< std::pair< Command, sml::Identifier* > >::iterator m_command_iter;
	
	sml::Agent& m_agent;
};

#endif // OUTPUT_LINK_MANAGER_H

