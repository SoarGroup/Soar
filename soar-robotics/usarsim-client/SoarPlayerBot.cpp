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

SoarPlayerBot::SoarPlayerBot( int port, Agent& agent, const std::string& productions )
: m_robot( "localhost", port )
, m_pp( &m_robot, 2 )
, m_fp( &m_robot, 0 )
, m_lp( &m_robot, 0 )
//, m_gp( &m_robot, 0 )
, m_productions( productions )
, m_agent( agent )
{
	m_input_link = new InputLinkManager( m_agent );
	m_output_link = new OutputLinkManager( m_agent );
	
	reload_productions();
	
	m_robot.Read();
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

void SoarPlayerBot::update( std::deque< Message* >& outgoing_message_deque )
{
	//std::cout << "update()" << std::endl;
	// read from the proxies
	
	m_robot.ReadIfWaiting();
	
	double x = m_pp.GetXPos();
	double y = m_pp.GetYPos();
	double yaw = m_pp.GetYaw();
	double motion_x = m_pp.GetXSpeed();
	double motion_y = m_pp.GetYSpeed();
	double motion_yaw = m_pp.GetYawSpeed();
	
	//std::cout << "pos(x,y,yaw), motion(x,y,yaw): ";
	//std::cout << "(" << x << "," << y << "," << yaw << ") (" << motion_x << "," << motion_y << "," << motion_yaw << ")" << std::endl;
	
	// update input link
	timeval time;
	gettimeofday( &time, 0 );
	m_input_link->time_update( time );
	m_input_link->position_update( x, y, yaw );
	m_input_link->motion_update( motion_x, motion_y, motion_yaw );
	
	m_input_link->clear_expired_fiducials();
	for ( unsigned count = 0; count < m_fp.GetCount(); ++count )
	{
		player_fiducial_item item = m_fp.GetFiducialItem( count );
		m_input_link->feducial_update( item.id, item.pose.px, item.pose.py );
	}
	m_input_link->update_expired_fiducials();
	
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
				motion_x = 0;
				break;
			case Command::MOVE_FORWARD:
				motion_x = command->get_throttle() * 0.300;
				break;
			case Command::MOVE_BACKWARD:
				motion_x = command->get_throttle() * -0.300;
				break;
			}
			break;
			
		case Command::ROTATE:
			std::cout << m_agent.GetAgentName() << ": ROTATE" << std::endl;
			motion_command_received = true;
			switch ( command->get_rotate_direction() )
			{
			case Command::ROTATE_STOP:
				motion_yaw = 0;
				break;
			case Command::ROTATE_RIGHT:
				motion_yaw = command->get_throttle() * -10 * ( 3.14159265 / 180 );
				break;
			case Command::ROTATE_LEFT:
				motion_yaw = command->get_throttle() * 10 * ( 3.14159265 / 180 );
				break;
			}
			break;
			
		case Command::STOP:
			std::cout << m_agent.GetAgentName() << ": STOP" << std::endl;
			motion_command_received = true;
			motion_x = 0;
			motion_yaw = 0;
			break;
			
		case Command::MOVE_TO:
			std::cout << m_agent.GetAgentName() << ": MOVE_TO(" << command->get_x() << "," << command->get_y() << ")" << std::endl;
			player_pose2d move_to_destination;
			move_to_destination.px = command->get_x();
			move_to_destination.py = command->get_y();
			move_to_destination.pa = atan2( command->get_y() - y, command->get_x() - x );
			
			m_pp.GoTo( move_to_destination );

			break;
			
		case Command::ROTATE_TO:
			player_pose2d rotate_to_destination;
			rotate_to_destination.px = x;
			rotate_to_destination.py = y;
			rotate_to_destination.pa = command->get_a();
			std::cout << m_agent.GetAgentName() << ": ROTATE_TO(" << x << "," << y << "," << command->get_a() << ")" << std::endl;
			
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
		m_pp.SetSpeed( motion_x, motion_yaw );
	}
	
	m_output_link->commit();
}


