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

std::string rhsPrintHandler( smlRhsEventId, void*, Agent* pAgent, char const*, char const* pArgument )
{
	std::cout << pAgent->GetAgentName() << ": " << pArgument << std::endl;
	return pArgument;
}

SoarPlayerClient::SoarPlayerClient( const std::string& productions, int number_of_bots )
: m_kernel( 0 )
, m_bot_count( 0 )
, m_run_thread( 0 )
, m_stop_issued( false )
{
    m_kernel = sml::Kernel::CreateKernelInNewThread();
	assert( m_kernel );
    
    while ( m_bot_count < number_of_bots )
    {
	    std::stringstream agent_name;
	    agent_name << "player" << m_bot_count;
    
	    Agent* agent = m_kernel->CreateAgent( agent_name.str().c_str() );
	    assert( agent );
	    
	    m_bot_list.push_back( new SoarPlayerBot( 6666 + m_bot_count, *agent, productions ) );
	    m_bot_count += 1;
    }
    
    m_kernel->RegisterForUpdateEvent( smlEVENT_AFTER_ALL_OUTPUT_PHASES, updateHandler, this );
    m_kernel->RegisterForAgentEvent( smlEVENT_BEFORE_AGENT_REINITIALIZED, agentHandler, this );
    m_kernel->RegisterForAgentEvent( smlEVENT_AFTER_AGENT_REINITIALIZED, agentHandler, this );
    m_kernel->AddRhsFunction( "player-print", rhsPrintHandler, 0 );
}

SoarPlayerClient::~SoarPlayerClient()
{
	for ( std::vector< SoarPlayerBot* >::iterator iter = m_bot_list.begin(); iter != m_bot_list.end(); ++iter )
	{
		delete *iter;
	}
	m_bot_list.clear();
	
	if ( m_kernel )
	{
		m_kernel->Shutdown();
		delete m_kernel;
		m_kernel = 0;
	}
}

void SoarPlayerClient::update()
{
	for ( std::vector< SoarPlayerBot* >::iterator iter = m_bot_list.begin(); iter != m_bot_list.end(); ++iter )
	{
		(*iter)->update();
	}

    if ( m_stop_issued ) 
    {
        m_kernel->StopAllAgents();
    }
}


