#include "Sedan.hxx"

bool Sedan::stop = false;

Sedan::Sedan( const std::string& host, int port )
: m_robot( host.c_str(), port )
, m_write( &m_robot, 0 )
, m_read( &m_robot, 1 )
{
}

