#ifndef OUTPUT_LINK_MANAGER_HXX
#define OUTPUT_LINK_MANAGER_HXX

#include "OutputLinkManager.h"

#include <exception>
#include <string>
#include <sstream>
#include <iostream>

Command::CommandType Command::get_type() 
{ 
	return m_type; 
}

Command::MoveDirection Command::get_move_direction() 
{ 
	return m_move; 
}

Command::RotateDirection Command::get_rotate_direction() 
{ 
	return m_rotate; 
}

double Command::get_throttle() 
{ 
	return m_throttle; 
}

Command::GripperCommand Command::get_gripper_command() 
{ 
	return m_gripper; 
}

double Command::get_x()
{
	return m_x;
}

double Command::get_y()
{
	return m_y;
}

Command::Status Command::get_status() 
{ 
	return m_status; 
}

void Command::set_status( Status status ) 
{ 
	m_status = status; 
}

void OutputLinkManager::read()
{
	std::cout << "read" << std::endl;
	for ( int index = 0; index < m_agent.GetNumberCommands(); ++index )
	{
		try
		{
			std::pair< Command, sml::Identifier* > data;
			data.second = m_agent.GetCommand( index );
		
			Command command( data.second );
			data.first = command;
			
			m_command_list.push_back( data );
			
			std::cout << "got command type " << command.get_type() << std::endl;
		}
		catch ( ... )
		{
			m_agent.GetCommand( index )->AddStatusError();
			std::cerr << "bad command" << std::endl;
		}
	}
	
	m_agent.ClearOutputLinkChanges();
	
	m_command_iter = m_command_list.begin();
}

Command* OutputLinkManager::get_next_command()
{
	if ( m_command_iter == m_command_list.end() )
	{
		return 0;
	}

	Command* current_command = &( m_command_iter->first );
	++m_command_iter;
	
	return current_command;
}

void OutputLinkManager::commit()
{
	m_command_iter = m_command_list.begin();
	while ( m_command_iter != m_command_list.end() )
	{
		switch ( m_command_iter->first.get_status() )
		{
		case Command::STATUS_NONE:
			break;
		case Command::STATUS_COMPLETE:
			m_command_iter->second->AddStatusComplete();
			break;
		case Command::STATUS_ERROR:
			m_command_iter->second->AddStatusError();
			break;
		case Command::STATUS_EXECUTING:
			// TODO: m_command_iter->second->AddStatusExecuting();
			break;
		}
	
		m_command_list.erase( m_command_iter );
		m_command_iter = m_command_list.begin();
	}
	m_agent.Commit();
}

#endif // OUTPUT_LINK_MANAGER_HXX

