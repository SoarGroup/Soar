#ifndef SOAR_PLAYER_CLIENT_HXX
#define SOAR_PLAYER_CLIENT_HXX

#include "SoarPlayerClient.h"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"
#include "SoarPlayerBot.h"

#include <sstream>
#include <unistd.h>
#include <sys/time.h>

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
    
	for ( std::vector< SoarPlayerBot* >::iterator iter = m_bot_list.begin(); iter != m_bot_list.end(); ++iter )
	{
		(*iter)->reset();
	}

    return std::string();
}

std::string SoarPlayerClient::command_reload()
{
    if ( update_and_check_running() )
    {
        return std::string( "running, stop first" );
    }

	for ( std::vector< SoarPlayerBot* >::iterator iter = m_bot_list.begin(); iter != m_bot_list.end(); ++iter )
	{
		(*iter)->reload_productions();
	}

    return std::string();
}

std::string SoarPlayerClient::command_output( const std::string& command )
{
    if ( update_and_check_running() )
    {
        return std::string( "running, stop first" );
    }
    
    if ( m_bot_list.size() != 1)
    {
    	return std::string( "output command only works with one agent" );
    }
    
    return (*(m_bot_list.begin()))->command_output( command );
}

void SoarPlayerClient::agent_event( sml::smlAgentEventId id )
{
	for ( std::vector< SoarPlayerBot* >::iterator iter = m_bot_list.begin(); iter != m_bot_list.end(); ++iter )
	{
		switch ( id )
		{
		case sml::smlEVENT_BEFORE_AGENT_REINITIALIZED:
			(*iter)->clear_io_links();
			break;
	
		case sml::smlEVENT_AFTER_AGENT_REINITIALIZED:
			(*iter)->create_io_links();
			break;
		
		default:
			assert( false );
			break;
		}
	}
}

#endif // SOAR_PLAYER_CLIENT_HXX
