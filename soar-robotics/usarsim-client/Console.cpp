#include "Console.hxx"

Console::Console( SoarRunThreadInterface& client, int sedan_port )
: m_client( client )
{
	m_sedan_port = sedan_port;
	m_sedan_connected = false;
}

Console::~Console()
{
	Sedan::stop = true;
}

