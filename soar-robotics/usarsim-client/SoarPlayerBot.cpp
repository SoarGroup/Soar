#include "SoarPlayerBot.hxx"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"
#include "Message.h"

#include <cassert>
#include <exception>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>
#include <cmath>

using namespace sml;
using namespace PlayerCc;
using std::string;
using std::list;

SoarPlayerBot::SoarPlayerBot( const std::string& host, int port, Agent& agent, const std::string& productions )
: m_robot( host.c_str(), port )
, m_pp( &m_robot, 2 )
, m_fp( &m_robot, 0 )
, m_lp( &m_robot, 0 )
, m_productions( productions )
, m_agent( agent )
{
	m_input_link = new InputLinkManager( m_agent );
	m_output_link = new OutputLinkManager( m_agent );
	
	reload_productions();
	
	update_cache();
}

SoarPlayerBot::~SoarPlayerBot()
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

void SoarPlayerBot::update_cache()
{
	m_robot.Read();
	m_current_x = m_pp.GetXPos();
	m_current_y = m_pp.GetYPos();
	m_current_yaw = m_pp.GetYaw();
	
	m_motion_x = m_pp.GetXSpeed();
	m_motion_y = m_pp.GetYSpeed();
	m_motion_yaw = m_pp.GetYawSpeed();
}

void SoarPlayerBot::update( std::deque< Message* >& outgoing_message_deque )
{
	timeval time;
	gettimeofday( &time, 0 );
	m_input_link->time_update( time );

	// read from the proxies
	if ( m_robot.Peek() )
	{
		update_cache();	
	
		// update input link
		m_input_link->position_update( m_current_x, m_current_y, m_current_yaw );
		m_input_link->motion_update( m_motion_x, m_motion_y, m_motion_yaw );
		
		m_input_link->clear_expired_fiducials();
		for ( unsigned index = 0; index < m_fp.GetCount(); ++index )
		{
			player_fiducial_item item = m_fp.GetFiducialItem( index );
			m_input_link->feducial_update( item.id, item.pose.px, item.pose.py );
		}
		m_input_link->update_expired_fiducials();
	}
	
	//std::cout << "pos(x,y,yaw), motion(x,y,yaw): ";
	//std::cout << "(" << x << "," << y << "," << yaw << ") (" << motion_x << "," << motion_y << "," << motion_yaw << ")" << std::endl;
	
	// read output link
	m_output_link->read();
	bool motion_command_received = false;
	for ( Command* command = m_output_link->get_next_command(); command != 0; command = m_output_link->get_next_command() )
	{
		switch ( command->get_type() )
		{
		case Command::MOVE:
			std::cout << m_agent.GetAgentName() << ": MOVE" << std::endl;
			motion_command_received = true;
			switch ( command->get_move_direction() )
			{
			case Command::MOVE_STOP:
				m_motion_x = 0;
				break;
			case Command::MOVE_FORWARD:
				m_motion_x = command->get_throttle();
				break;
			case Command::MOVE_BACKWARD:
				m_motion_x = command->get_throttle();
				break;
			}
			break;
			
		case Command::ROTATE:
			std::cout << m_agent.GetAgentName() << ": ROTATE" << std::endl;
			motion_command_received = true;
			switch ( command->get_rotate_direction() )
			{
			case Command::ROTATE_STOP:
				m_motion_yaw = 0;
				break;
			case Command::ROTATE_RIGHT:
				m_motion_yaw = command->get_throttle() * -1;
				break;
			case Command::ROTATE_LEFT:
				m_motion_yaw = command->get_throttle();
				break;
			}
			break;
			
		case Command::STOP:
			std::cout << m_agent.GetAgentName() << ": STOP" << std::endl;
			motion_command_received = true;
			m_motion_x = 0;
			m_motion_yaw = 0;
			break;
			
		case Command::MOVE_TO:
			std::cout << m_agent.GetAgentName() << ": MOVE_TO(" << command->get_x() << "," << command->get_y() << ")" << std::endl;
			player_pose2d move_to_destination;
			move_to_destination.px = command->get_x();
			move_to_destination.py = command->get_y();
			move_to_destination.pa = atan2( command->get_y() - m_current_y, command->get_x() - m_current_x );
			
			m_pp.GoTo( move_to_destination );

			break;
			
		case Command::ROTATE_TO:
			player_pose2d rotate_to_destination;
			rotate_to_destination.px = m_current_x;
			rotate_to_destination.py = m_current_y;
			rotate_to_destination.pa = command->get_a();
			std::cout << m_agent.GetAgentName() << ": ROTATE_TO(" << rotate_to_destination.px 
					<< "," << rotate_to_destination.py << "," << rotate_to_destination.pa << ")" << std::endl;
			
			m_pp.GoTo( rotate_to_destination );

			break;
			
		case Command::BROADCAST_MESSAGE:
			{
				Message* message = new Message( m_agent.GetAgentName(), command->get_sentence() );
				std::cout << m_agent.GetAgentName() << ": SEND_MESSAGE" << *message << std::endl;
				outgoing_message_deque.push_back( message );
			}
			break;
		
		case Command::REMOVE_MESSAGE:
			std::cout << m_agent.GetAgentName() << ": REMOVE_MESSAGE(" << command->get_remove_message_id() << ")" << std::endl;
			m_input_link->remove_message( command->get_remove_message_id() );
			m_input_link->commit();
			break;
		}
		
		command->set_status( Command::STATUS_COMPLETE );
	}
	
	if ( motion_command_received )
	{
		m_pp.SetSpeed( m_motion_x, m_motion_yaw );
	}
	
	m_output_link->commit();
	
#if 1	// make true to check performance
	static int last_time = -1;
	static int count = -1;
	if ( time.tv_sec > last_time )
	{
		if ( count >= 0 ) 
		{
			std::cout << count << " updates/sec\n";
		}
		count = 0;
		last_time = time.tv_sec;
	} 
	else 
	{
		++count;
	}
#endif
}


