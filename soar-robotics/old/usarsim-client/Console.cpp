#include "Console.hxx"

Console::Console( SoarRunThreadInterface& client, std::string sedan_host,  int sedan_port )
: m_client( client )
, m_sedan_host( sedan_host )
, m_sedan_port( sedan_port )
{
	m_sedan_connected = false;
}

Console::~Console()
{
	Sedan::stop = true;
}

