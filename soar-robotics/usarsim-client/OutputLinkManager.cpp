#include "OutputLinkManager.hxx"

#include "utility.h"

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
	else if ( command_string == "rotate-to" )
	{
		m_type = ROTATE_TO;

		m_a = ::get_param< double >( "yaw", command_id );
		m_a = to_absolute_yaw_player( m_a );
		m_throttle = ::get_param< double >( "throttle", command_id );
	}
	else if ( command_string == "stop" )
	{
		m_type = STOP;
		m_move = MOVE_STOP;
		m_rotate = ROTATE_STOP;
		m_throttle = 0;
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
		std::cout << "broadcast-message" << std::endl;

		m_type = BROADCAST_MESSAGE;
		Identifier* current_identifier = command_id;

		do
		{		
			char const* type = 0;
			char const* word = 0;
			Identifier* next = 0;
			
			for ( std::list<WMElement*>::const_iterator children_iter = current_identifier->GetChildrenBegin();
				children_iter != current_identifier->GetChildrenEnd();
				++children_iter )
			{
				char const* attribute = (*children_iter)->GetAttribute();
				assert( attribute );
				
				if ( attribute == string( "word" ) )
				{
					type = (*children_iter)->GetValueType();
					word = (*children_iter)->GetValueAsString();
					std::cout << "word: " << word << std::endl;
				}
				else if ( attribute == string( "next" ) )
				{
					if ( (*children_iter)->IsIdentifier() )
					{
						next = (*children_iter)->ConvertToIdentifier();
					}
					else if ( (*children_iter)->GetValueType() == string( "string" ) )
					{
						if ( (*children_iter)->GetValueAsString() != string( "nil" ) )
						{
							std::cout << "next string not nil: " << (*children_iter)->GetValueAsString() << std::endl;
							throw std::exception();
						}
						// next is 0
					}
					else
					{
						std::cout << "next type not string or id: " << (*children_iter)->GetValueType() << std::endl;
						throw std::exception();
					}
					std::cout << "next: " << (*children_iter)->GetValueAsString() << std::endl;
				}
				else
				{
					std::cout << "unknown attribute: " << attribute << std::endl;
					throw std::exception();
				}
			}
			
			if ( word == 0 )
			{
				std::cout << "missing word in message" << std::endl;
				throw std::exception();
			}
			
			m_sentence.push_back( std::pair< std::string, std::string >( type, word ) );
			current_identifier = next;

		} while ( current_identifier );

		std::cout << "broadcast-message done" << std::endl;
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


