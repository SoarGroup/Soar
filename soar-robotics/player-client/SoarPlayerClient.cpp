#include "SoarPlayerClient.h"

using namespace PlayerCc;
using namespace sml;

SoarPlayerClient::SoarPlayerClient( Kernel* kernel, Agent* agent )
: m_robot( "localhost" )
, m_sp( &m_robot, 0 )
, m_pp( &m_robot, 0 )
{
    m_kernel = kernel;
    m_agent = agent;
    m_run_thread = 0;
    m_stop_issued = false;
}


