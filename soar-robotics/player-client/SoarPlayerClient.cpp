#include "SoarPlayerClient.h"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"

#include <cassert>

using namespace PlayerCc;
using namespace sml;

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
, m_sp( &m_robot, 0 )
, m_pp( &m_robot, 0 )
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
