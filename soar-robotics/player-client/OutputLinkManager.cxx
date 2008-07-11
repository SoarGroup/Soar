#include "OutputLinkManager.h"

#include <exception>
#include <string>
#include <sstream>
#include <iostream>

using namespace sml;

void has_direction( Identifier* id )
{
	if ( !id->GetParameterValue( "direction" ) )
	{
		throw new std::exception();
	}
}

void has_throttle( Identifier* id )
{
	if ( !id->GetParameterValue( "throttle" ) )
	{
		throw new std::exception();
	}
}

void has_command( Identifier* id )
{
	if ( !id->GetParameterValue( "command" ) )
	{
		throw new std::exception();
	}
}

template < class T >
bool from_string( T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&) )
{
  std::istringstream iss( s );
  return !( iss >> f >> t ).fail();
}

double get_throttle( Identifier* id )
{
	has_throttle( id );
	
	double throttle = 0;
	
	if ( !from_string< double >( throttle, std::string( id->GetParameterValue( "throttle" ) ), std::dec ) )
	{
		throw new std::exception();
	}
	return throttle;
}

Command::Command( Identifier* command_id )
: m_status ( STATUS_NONE )
{
	std::string command_string( command_id->GetAttribute() );
	if ( command_string == "move" )
	{
		m_type = MOVE;
		has_direction( command_id );
		
		std::string direction( command_id->GetParameterValue( "direction" ) );
		if ( direction == "stop" )
		{
			m_move = MOVE_STOP;
			m_throttle = 0;
		}
		else if ( direction == "forward" )
		{
			m_move = MOVE_FORWARD;
			m_throttle = ::get_throttle( command_id );
		}
		else if ( direction == "backward" )
		{
			m_move = MOVE_BACKWARD;
			m_throttle = ::get_throttle( command_id );
		}
		else
		{
			throw new std::exception();
		}
	}
	else if ( command_string == "rotate" )
	{
		m_type = ROTATE;
		has_direction( command_id );
		
		std::string direction( command_id->GetParameterValue( "direction" ) );
		if ( direction == "stop" )
		{
			m_rotate = ROTATE_STOP;
			m_throttle = 0;
		}
		else if ( direction == "left" )
		{
			m_rotate = ROTATE_LEFT;
			m_throttle = ::get_throttle( command_id );
		}
		else if ( direction == "right" )
		{
			m_rotate = ROTATE_RIGHT;
			m_throttle = ::get_throttle( command_id );
		}
		else
		{
			throw new std::exception();
		}
	}
	else if ( command_string == "stop" )
	{
		m_type = STOP;
		m_move = MOVE_STOP;
		m_rotate = ROTATE_STOP;
		m_throttle = 0;
	}
	else if ( command_string == "gripper" )
	{
		m_type = GRIPPER;
		has_command( command_id );
		
		std::string command( command_id->GetParameterValue( "command" ) );
		if ( command == "open" )
		{
			m_gripper = GRIPPER_OPEN;
		}
		else if ( command == "close" )
		{
			m_gripper = GRIPPER_CLOSE;
		}
		else if ( command == "stop" )
		{
			m_gripper = GRIPPER_STOP;
		}
		else
		{
			throw new std::exception();
		}
	}
	else
	{
		throw new std::exception();
	}
}

OutputLinkManager::OutputLinkManager( Agent& agent )
: m_agent( agent )
{
}

OutputLinkManager::~OutputLinkManager()
{
	m_command_list.clear();
}

void OutputLinkManager::read()
{
	for ( int index = 0; index < m_agent.GetNumberCommands(); ++index )
	{
		try
		{
			std::pair< Command, Identifier* > data;
			data.second = m_agent.GetCommand( index );
		
			Command command( data.second );
			data.first = command;
			
			m_command_list.push_back( data );
		}
		catch ( ... )
		{
			m_agent.GetCommand( index )->AddStatusError();
			std::cerr << "bad command" << std::endl;
		}
	}
	
	m_command_iter = m_command_list.begin();
}

Command* OutputLinkManager::get_next_command()
{
	++m_command_iter;
	if ( m_command_iter == m_command_list.end() )
	{
		return 0;
	}
	return &( m_command_iter->first );
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

