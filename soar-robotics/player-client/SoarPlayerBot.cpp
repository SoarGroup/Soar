#include "SoarPlayerBot.hxx"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"

#include <cassert>
#include <exception>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>
#include <cmath>

using namespace sml;
using namespace PlayerCc;

const double SoarPlayerBot::MOVE_TO_TOLERANCE = 0.349065556; // twenty degrees

SoarPlayerBot::SoarPlayerBot( int port, Agent& agent, const std::string& productions )
: m_robot( "localhost", port )
, m_pp( &m_robot, 1 )
, m_fp( &m_robot, 0 )
, m_lp( &m_robot, 0 )
, m_gp( &m_robot, 0 )
//, m_move_to( false )
, m_productions( productions )
, m_agent( agent )
{
    m_input_link = new InputLinkManager( m_agent );
    m_output_link = new OutputLinkManager( m_agent );
    
    reload_productions();
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

void SoarPlayerBot::update()
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
   	
   	bool outer = m_gp.GetBeams() & 0x1;
   	bool inner = m_gp.GetBeams() & 0x2;
   	bool gripper_open = false;
   	bool gripper_closed = false;
   	bool gripper_moving = false;
   	bool gripper_error = false;
   	switch ( m_gp.GetState() )
   	{
   	case PLAYER_GRIPPER_STATE_OPEN:
   		gripper_open = true;
	   	break;
   	case PLAYER_GRIPPER_STATE_CLOSED:
   		gripper_closed = true;
	   	break;
   	case PLAYER_GRIPPER_STATE_MOVING:
   		gripper_moving = true;
	   	break;
	default:
   	case PLAYER_GRIPPER_STATE_ERROR:
   		gripper_error = true;
	   	break;
	}
   	
	// update input link
	timeval time;
	gettimeofday( &time, 0 );
	m_input_link->time_update( time );
	m_input_link->position_update( x, y, yaw );
	m_input_link->motion_update( motion_x, motion_y, motion_yaw );
	m_input_link->beam_update( outer, inner );
	m_input_link->gripper_update( gripper_open, gripper_closed, gripper_moving, gripper_error );
	
	for ( unsigned count = 0; count < m_fp.GetCount(); ++count )
	{
		player_fiducial_item item = m_fp.GetFiducialItem( count );
		m_input_link->feducial_update( item.id, item.pose.px, item.pose.py );
	}
	
	m_input_link->commit();
	
	// read output link
	m_output_link->read();
	bool motion_command_received = false;
	for ( Command* command = m_output_link->get_next_command(); command != 0; command = m_output_link->get_next_command() )
	{
		switch ( command->get_type() )
		{
		case Command::MOVE:
			//std::cout << m_agent.GetAgentName() << ": MOVE" << std::endl;
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
			//std::cout << m_agent.GetAgentName() << ": ROTATE" << std::endl;
			//if ( m_move_to )
			//{
			//	m_move_to = false;
			//	m_pp.SetSpeed( motion_x, 0 );
			//}
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
			//std::cout << m_agent.GetAgentName() << ": STOP" << std::endl;
			//m_move_to = false;
			motion_command_received = true;
			motion_x = 0;
			motion_yaw = 0;
			break;
			
		case Command::GRIPPER:
			switch ( command->get_gripper_command() )
			{
			case Command::GRIPPER_OPEN:
				m_gp.Open();
				break;
			case Command::GRIPPER_CLOSE:
				m_gp.Close();
				break;
			case Command::GRIPPER_STOP:
				m_gp.Stop();
				break;
			}
			break;
			
		case Command::MOVE_TO:
			std::cout << m_agent.GetAgentName() << ": MOVE_TO(" << command->get_x() << "," << command->get_y() << ")" << std::endl;
			//if ( m_move_to )
			//{
			//	m_pp.SetSpeed( motion_x, 0 );
			//}
			//m_move_to = true;
			m_move_to_destination.px = command->get_x();
			m_move_to_destination.py = command->get_y();
			m_move_to_destination.pa = atan2( command->get_y() - y, command->get_x() - x );
			
			//m_pp.GoTo( x, y, m_move_to_destination.pa );
			m_pp.GoTo( m_move_to_destination );

			break;
		}
		
		command->set_status( Command::STATUS_COMPLETE );
	}
	
	if ( motion_command_received )
	{
		m_pp.SetSpeed( motion_x, motion_yaw );
	}
	
	//if ( m_move_to )
	//{
	//	if ( fabs( yaw - m_move_to_destination.pa ) < MOVE_TO_TOLERANCE )
	//	{
	//		m_move_to = false;
	//		m_pp.GoTo( m_move_to_destination );
	//	}
	//}

	m_output_link->commit();
}


