#include "OutputLinkManager.hxx"

using namespace sml;
using std::string;

void has_param( const char* parameter, Identifier* id )
{
	if ( !id->GetParameterValue( parameter ) )
	{
		throw std::exception();
	}
}

template < class T >
bool from_string( T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&) )
{
  std::istringstream iss( s );
  return !( iss >> f >> t ).fail();
}

template < class T >
T get_param( const char* parameter, Identifier* id )
{
	has_param( parameter, id );
	
	T param = 0;
	
	if ( !from_string< T >( param, std::string( id->GetParameterValue( parameter ) ), std::dec ) )
	{
		throw std::exception();
	}
	return param;
}

Command::Command( Identifier* command_id )
: m_status ( STATUS_NONE )
{
	std::string command_string( command_id->GetAttribute() );
	if ( command_string == "move" )
	{
		m_type = MOVE;
		has_param( "direction", command_id );
		
		std::string direction( command_id->GetParameterValue( "direction" ) );
		if ( direction == "stop" )
		{
			m_move = MOVE_STOP;
			m_throttle = 0;
		}
		else if ( direction == "forward" )
		{
			m_move = MOVE_FORWARD;
			m_throttle = ::get_param< double >( "throttle", command_id );
		}
		else if ( direction == "backward" )
		{
			m_move = MOVE_BACKWARD;
			m_throttle = ::get_param< double >( "throttle", command_id );
		}
		else
		{
			throw std::exception();
		}
	}
	else if ( command_string == "rotate" )
	{
		m_type = ROTATE;
		has_param( "direction", command_id );
		
		std::string direction( command_id->GetParameterValue( "direction" ) );
		if ( direction == "stop" )
		{
			m_rotate = ROTATE_STOP;
			m_throttle = 0;
		}
		else if ( direction == "left" )
		{
			m_rotate = ROTATE_LEFT;
			m_throttle = ::get_param< double >( "throttle", command_id );
		}
		else if ( direction == "right" )
		{
			m_rotate = ROTATE_RIGHT;
			m_throttle = ::get_param< double >( "throttle", command_id );
		}
		else
		{
			throw std::exception();
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
		has_param( "command", command_id );
		
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
			throw std::exception();
		}
	}
	else if ( command_string == "move-to" )
	{
		m_type = MOVE_TO;
		m_x = get_param< double >( "x", command_id );
		m_y = get_param< double >( "y", command_id );
		m_throttle = ::get_param< double >( "throttle", command_id );
	}
	else if ( command_string == "broadcast-message" )
	{
		m_type = BROADCAST_MESSAGE;
		Identifier* current_identifier = command_id;
		do
		{
			char const* word = command_id->GetParameterValue( "word" );
			WMElement* next_wme = command_id->FindByAttribute( "next", 0 );
			
			if ( word == 0 || next_wme == 0 )
			{
				throw std::exception();
			}
			
			m_sentence.push_back( string( word ) );
			
			if ( string( next_wme->GetValueAsString() ) == string( "nil" ) )
			{
				current_identifier = 0;
			}
			else
			{
				if ( !next_wme->IsIdentifier() )
				{
					throw std::exception();
				}
			
				current_identifier = next_wme->ConvertToIdentifier();
				assert( current_identifier );
			}
		} while( current_identifier );
	}
	else if ( command_string == "remove-message" )
	{
		m_type = REMOVE_MESSAGE;
		m_remove_message_id = get_param< int >( "id", command_id );
	}
	else
	{
		throw std::exception();
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


