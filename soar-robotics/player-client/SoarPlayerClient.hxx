#ifndef SOAR_PLAYER_CLIENT_HXX
#define SOAR_PLAYER_CLIENT_HXX

#include "SoarPlayerClient.h"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"

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

void SoarPlayerClient::agent_event( sml::smlAgentEventId id )
{
	switch ( id )
	{
	case sml::smlEVENT_BEFORE_AGENT_REINITIALIZED:
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
	
	case sml::smlEVENT_AFTER_AGENT_REINITIALIZED:
		{
			m_input_link = new InputLinkManager( *m_agent );
			m_output_link = new OutputLinkManager( *m_agent );
		}
		break;
		
	default:
		break;
	}
}

#endif // SOAR_PLAYER_CLIENT_HXX
