#ifndef SOAR_PLAYER_BOT_HXX
#define SOAR_PLAYER_BOT_HXX

#include "SoarPlayerBot.h"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"
#include "utility.h"

#include <exception>

void SoarPlayerBot::reload_productions()
{
	if ( !m_agent.LoadProductions( m_productions.c_str() ) )
	{
		std::cerr << "error loading productions: " << m_agent.GetLastErrorDescription() << std::endl;
		throw std::exception();
	}
}

void SoarPlayerBot::reset()
{
	m_agent.InitSoar();
}

void SoarPlayerBot::clear_io_links()
{
	if ( m_input_link )
	{
		delete m_input_link;
		m_input_link = 0;
	}
	if ( m_output_link )
	{
		delete m_output_link;
		m_output_link = 0;
	}
}

void SoarPlayerBot::create_io_links()
{
	m_input_link = new InputLinkManager( m_agent );
	m_output_link = new OutputLinkManager( m_agent );
}

void SoarPlayerBot::add_incoming_messages( const std::deque< Message* >& incoming_messages_deque )
{
	for( std::deque< Message* >::const_iterator iter = incoming_messages_deque.begin(); iter != incoming_messages_deque.end(); ++iter )
	{
		m_input_link->add_message( *(*iter) );
	}
	
	m_input_link->commit();
}

std::string SoarPlayerBot::command_output( const std::string& command )
{
	m_robot.Read();

	size_t split1 = command.find( " " );
	std::string cmd = command.substr( 0, split1 );
	if ( cmd == "stop" )
	{
		std::cout << "(0,0)" << std::endl;
		m_pp.SetSpeed( 0, 0 );
	}
	else if ( cmd == "move" )
	{
		if ( split1 == std::string::npos )
		{
			return "move <x>";
		}
		std::string x_string = command.substr( split1 + 1 );
		double x = 0;
		if ( !from_string( x, x_string, std::dec ) )
		{
			return "need float argument";
		}
		
		std::cout << "(" << x << ")" << std::endl;
		m_pp.SetSpeed( x, 0 );
	}
	else if ( cmd == "rotate" )
	{
		if ( split1 == std::string::npos )
		{
			return "rotate <yaw degrees rate>";
		}
		std::string yaw_string = command.substr( split1 + 1 );
		double yaw = 0;
		if ( !from_string( yaw, yaw_string, std::dec ) )
		{
			return "need float argument";
		}
		
		yaw = to_relative_yaw_player( yaw );
		
		std::cout << "(" << yaw << ")" << std::endl;
		m_pp.SetSpeed( 0, yaw );
	}
	else if ( cmd == "rotate-to" )
	{
		if ( split1 == std::string::npos )
		{
			return "rotate-to <absolute yaw degrees>";
		}
		std::string yaw_string = command.substr( split1 + 1 );
		double yaw = 0;
		if ( !from_string( yaw, yaw_string, std::dec ) )
		{
			return "need float argument";
		}
		
		yaw = to_absolute_yaw_player( yaw );

		player_pose2d rotate_to_destination;
		rotate_to_destination.px = m_pp.GetXPos();
		rotate_to_destination.py = m_pp.GetYPos();
		rotate_to_destination.pa = yaw;
		
		std::cout << "(" << rotate_to_destination.px << "," << rotate_to_destination.py << "," << rotate_to_destination.pa << ")" << std::endl;
		m_pp.GoTo( rotate_to_destination );
	}
	else if ( cmd == "move-to" )
	{
		if ( split1 == std::string::npos )
		{
			return "move-to x y";
		}
		size_t split2 = command.find( " ", split1 + 1 );
		if ( split2 == std::string::npos )
		{
			return "need y float argument";
		}

		std::string x_string = command.substr( split1 + 1, split2 - (split1 + 1) );
		double x = 0;
		if ( !from_string( x, x_string, std::dec ) )
		{
			return "need float x argument";
		}

		std::string y_string = command.substr( split2 + 1 );
		double y = 0;
		if ( !from_string( y, y_string, std::dec ) )
		{
			return "need float y argument";
		}
	
		player_pose2d move_to_destination;
		move_to_destination.px = x;
		move_to_destination.py = y;
		move_to_destination.pa = atan2( y - m_pp.GetYPos(), x - m_pp.GetXPos() );
		
		std::cout << "(" << move_to_destination.px << "," << move_to_destination.py << "," << move_to_destination.pa << ")" << std::endl;
		m_pp.GoTo( move_to_destination );
	}
	else
	{
		return "unknown output command: " + cmd;
	}

	return std::string();
}

#endif // SOAR_PLAYER_BOT_HXX

