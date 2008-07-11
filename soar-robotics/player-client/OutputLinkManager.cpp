#include "OutputLinkManager.hxx"

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


