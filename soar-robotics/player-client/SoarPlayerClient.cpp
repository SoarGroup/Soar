#include "SoarPlayerClient.hxx"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"

#include <cassert>
#include <exception>

using namespace sml;
using namespace PlayerCc;

void updateHandler( smlUpdateEventId, void* pUserData, Kernel*, smlRunFlags )
{
    SoarPlayerClient* pPlayerClient = static_cast< SoarPlayerClient* >( pUserData );
    pPlayerClient->update();
}

void agentHandler( smlAgentEventId id, void* pUserData, Agent* )
{
    SoarPlayerClient* pPlayerClient = static_cast< SoarPlayerClient* >( pUserData );
    pPlayerClient->agent_event( id );
}

SoarPlayerClient::SoarPlayerClient( const std::string& productions )
: m_productions( productions )
, m_robot( "localhost" )
, m_pp( &m_robot, 1 )
, m_fp( &m_robot, 0 )
, m_lp( &m_robot, 0 )
, m_gp( &m_robot, 0 )
{
    m_kernel = sml::Kernel::CreateKernelInNewThread();
    assert( m_kernel );
    
    m_agent = m_kernel->CreateAgent( "player" );
    assert( m_agent );
    
    m_input_link = new InputLinkManager( *m_agent );
    m_output_link = new OutputLinkManager( *m_agent );
    
    if ( reload_productions() )
    {
        std::cout << "loaded productions: " << m_productions << std::endl;
    }
    else
    {
        std::cerr << "error loading productions: " << m_productions << std::endl;
        throw new std::exception();
    }
    
    //m_agent->ExecuteCommandLine( "waitsnc --enable" );
    m_kernel->RegisterForUpdateEvent( smlEVENT_AFTER_ALL_OUTPUT_PHASES, updateHandler, this );
    m_kernel->RegisterForAgentEvent( smlEVENT_BEFORE_AGENT_REINITIALIZED, agentHandler, this );
    m_kernel->RegisterForAgentEvent( smlEVENT_AFTER_AGENT_REINITIALIZED, agentHandler, this );

    m_run_thread = 0;
    m_stop_issued = false;
}

SoarPlayerClient::~SoarPlayerClient()
{
	if ( m_input_link )
	{
		delete m_input_link;
	}
	
	if ( m_output_link )
	{
		delete m_output_link;
	}
	
    if ( m_kernel )
    {
        m_kernel->Shutdown();
        delete m_kernel;
    }
}

void SoarPlayerClient::update()
{
	//std::cout << "update()" << std::endl;
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
			std::cout << "MOVE" << std::endl;
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
			std::cout << "ROTATE" << std::endl;
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
			std::cout << "STOP" << std::endl;
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
			//std::cout << "MOVE_TO(" << command->get_x() << "," << command->get_y() << "," << motion_yaw << ")" << std::endl;
			//m_pp.GoTo( command->get_x(), command->get_y(), motion_yaw );
			player_pose2d_t position;
			player_pose2d_t velocity;
			
			position.px = command->get_x();
			position.py = command->get_y();
			position.pa = motion_yaw;
			
			velocity.px = 5;
			velocity.py = 5;
			velocity.pa = 5;
			
			std::cout << "MOVE_TO p(" << position.px << "," << position.py << "," << position.pa << ")"
				<< " v(" << velocity.px << "," << velocity.py << "," << velocity.pa << ")" << std::endl;
			m_pp.GoTo( position, velocity );
			break;
		}
		
		command->set_status( Command::STATUS_COMPLETE );
	}
	
	if ( motion_command_received )
	{
		m_pp.SetSpeed( motion_x, motion_yaw );
	}

	m_output_link->commit();

    if ( m_stop_issued ) 
    {
        m_kernel->StopAllAgents();
    }
}


