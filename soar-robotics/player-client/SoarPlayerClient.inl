#ifndef SOAR_PLAYER_CLIENT_INL
#define SOAR_PLAYER_CLIENT_INL

#include "SoarPlayerClient.h"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"

#include <sstream>
#include <unistd.h>
#include <sys/time.h>

using namespace sml;

bool SoarPlayerClient::update_and_check_running()
{
    if ( m_run_thread )
    {
        if ( m_run_thread->IsStopped() == false )
        {
            return true;
        }
        else
        {
            m_run_thread = 0;
        }
    }
    return false;
}

std::string SoarPlayerClient::command_run()
{
    if ( update_and_check_running() )
    {
        return std::string( "already running" );
    }
    
    m_stop_issued = false;
    m_run_thread = new RunThread( m_kernel );
    m_run_thread->Start();
    
    return std::string();
}

std::string SoarPlayerClient::command_stop()
{
    if ( !update_and_check_running() )
    {
        return std::string( "not running" );
    }
    
    m_stop_issued = true;
    
    return std::string();
}

std::string SoarPlayerClient::command_step()
{
    if ( update_and_check_running() )
    {
        return std::string( "already running" );
    }
    
    m_kernel->RunAllAgents( 1 );

    return std::string();
}

std::string SoarPlayerClient::command_debug()
{
    pid_t pid = fork();

    if ( pid < 0 )
    {
        return std::string( "fork error" );
    }
    
    if ( pid == 0 )
    {
        // child
        execlp( "java", "java", "-jar", "SoarJavaDebugger.jar", "-remote", static_cast< char* >( 0 ) );
        std::cerr << "execlp failed" << std::endl;
        exit(1);
    }
    
    return std::string();
}

std::string SoarPlayerClient::command_reset()
{
    if ( update_and_check_running() )
    {
        return std::string( "running, stop first" );
    }
    
    m_agent->InitSoar();
    return std::string();
}

std::string SoarPlayerClient::command_reload()
{
    if ( update_and_check_running() )
    {
        return std::string( "running, stop first" );
    }

    if ( !reload_productions() )
    {
        std::stringstream error;
        error << "error loading productions: " << m_agent->GetLastErrorDescription();
        return error.str();
    }

    return std::string();
}

bool SoarPlayerClient::reload_productions()
{
    return m_agent->LoadProductions( m_productions.c_str() );
}

void SoarPlayerClient::agent_event( smlAgentEventId id )
{
	switch ( id )
	{
	case smlEVENT_BEFORE_AGENT_REINITIALIZED:
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
		break;
	
	case smlEVENT_AFTER_AGENT_REINITIALIZED:
		{
			m_input_link = new InputLinkManager( *m_agent );
			m_output_link = new OutputLinkManager( *m_agent );
		}
		break;
		
	default:
		break;
	}
}

void SoarPlayerClient::update()
{
    // read from the proxies
    m_robot.Read();
    
   	double x = m_pp.GetXPos();
   	double y = m_pp.GetYPos();
   	double yaw = m_pp.GetYaw();
   	double motion_x = m_pp.GetXSpeed();
   	double motion_y = m_pp.GetYSpeed();
   	double motion_yaw = m_pp.GetYawSpeed();
   	
   	bool outer = m_gp.GetBeams() & 0x1;
   	bool inner = m_gp.GetBeams() & 0x2;
   	std::cout.setf(std::ios_base::hex);
   	std::cout << m_gp.GetBeams() << std::endl;
   	std::cout.setf(std::ios_base::dec);
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
	bool gripper_command_received = false;
	for ( Command* command = m_output_link->get_next_command(); command != 0; command = m_output_link->get_next_command() )
	{
		switch ( command->get_type() )
		{
		case Command::MOVE:
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
			motion_command_received = true;
			motion_x = 0;
			motion_yaw = 0;
			break;
			
		case Command::GRIPPER:
			gripper_command_received = true;
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
		}
		command->set_status( Command::STATUS_COMPLETE );
	}
	
	if ( motion_command_received )
	{
		m_pp.SetSpeed( motion_x, motion_yaw );
		m_output_link->commit();  // status wme update
	}

	if ( gripper_command_received )
	{
		m_output_link->commit(); // status wme update
	}

    if ( m_stop_issued ) 
    {
        m_kernel->StopAllAgents();
    }
}

#endif // SOAR_PLAYER_CLIENT_INL
