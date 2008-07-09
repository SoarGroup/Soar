#include "SoarPlayerClient.h"

using namespace PlayerCc;
using namespace sml;

void updateHandler( smlUpdateEventId, void* pUserData, Kernel*, smlRunFlags )
{
    SoarPlayerClient* pPlayerClient = static_cast< SoarPlayerClient* >( pUserData );
    pPlayerClient->update();
}

SoarPlayerClient::SoarPlayerClient( const std::string& productions )
: m_productions( productions )
, m_robot( "localhost" )
, m_sp( &m_robot, 0 )
, m_pp( &m_robot, 0 )
{
    m_kernel = sml::Kernel::CreateKernelInNewThread();
    m_agent = m_kernel->CreateAgent( "player" );
    
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

    m_run_thread = 0;
    m_stop_issued = false;
}

SoarPlayerClient::~SoarPlayerClient()
{
    if ( m_kernel )
    {
        m_kernel->Shutdown();
        delete m_kernel;
    }
}
