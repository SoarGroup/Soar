#include "Sedan.hxx"

bool Sedan::stop = false;

Sedan::Sedan( int port )
: m_robot( "localhost", port )
, m_write( &m_robot, 0 )
, m_read( &m_robot, 1 )
{
}

