#include "SoarPlayerClient.hxx"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"
#include "RunThread.h"

#include <cassert>
#include <iostream>
#include <sstream>

using namespace sml;
using namespace PlayerCc;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ostringstream;

template < class T >
inline string to_string( const T& t ) 
{
	ostringstream o;
	o << t;
	return o.str();
}

void updateHandler( smlUpdateEventId, void* pUserData, Kernel*, smlRunFlags )
{
	assert( pUserData );
	
    SoarPlayerClient* pPlayerClient = static_cast< SoarPlayerClient* >( pUserData );
    pPlayerClient->update();
}

void agentHandler( smlAgentEventId id, void* pUserData, Agent* )
{
	assert( pUserData );
	
    SoarPlayerClient* pPlayerClient = static_cast< SoarPlayerClient* >( pUserData );
    pPlayerClient->agent_event( id );
}

string rhsPrintHandler( smlRhsEventId, void*, Agent* pAgent, char const*, char const* pArgument )
{
	assert( pAgent );
	assert( pArgument );
	
	cout << pAgent->GetAgentName() << ": " << pArgument << endl;
	return pArgument;
}

SoarPlayerClient::SoarPlayerClient( ConfigFile& config )
: m_config( config )
, m_kernel( 0 )
, m_bot_count( 0 )
, m_run_thread( 0 )
, m_stop_issued( false )
{
	int number_of_bots = config.read<int>( "bots" );

    m_kernel = sml::Kernel::CreateKernelInNewThread();
    if ( m_kernel->HadError() )
    {
    	throw soar_error( m_kernel->GetLastErrorDescription() );
    }
    
    while ( m_bot_count < number_of_bots )
    {
	    string suffix( to_string< int >( m_bot_count ) );
	    string agent_name = config.read( "name" + suffix, "robot" + suffix );
    
	    Agent* agent = m_kernel->CreateAgent( agent_name.c_str() );
	    if ( m_kernel->HadError() )
	    {
	    	throw soar_error( m_kernel->GetLastErrorDescription() );
	    }
	    assert( agent );
	    
	    int port = config.read( "port" + suffix, 6666 + m_bot_count );
	    
	    m_bot_list.push_back( new SoarPlayerBot( port, *agent, config.read< string >( "productions" + suffix ) ) );
	    m_bot_count += 1;
    }
    
    m_kernel->RegisterForUpdateEvent( smlEVENT_AFTER_ALL_OUTPUT_PHASES, updateHandler, this );
    m_kernel->RegisterForAgentEvent( smlEVENT_BEFORE_AGENT_REINITIALIZED, agentHandler, this );
    m_kernel->RegisterForAgentEvent( smlEVENT_AFTER_AGENT_REINITIALIZED, agentHandler, this );
    m_kernel->AddRhsFunction( "player-print", rhsPrintHandler, 0 );
}

SoarPlayerClient::~SoarPlayerClient()
{
	for ( vector< SoarPlayerBot* >::iterator iter = m_bot_list.begin(); iter != m_bot_list.end(); ++iter )
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
	for ( vector< SoarPlayerBot* >::iterator iter = m_bot_list.begin(); iter != m_bot_list.end(); ++iter )
	{
		(*iter)->update();
	}

    if ( m_stop_issued ) 
    {
        m_kernel->StopAllAgents();
    }
}


