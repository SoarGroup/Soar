#ifndef SOAR_PLAYER_BOT_HXX
#define SOAR_PLAYER_BOT_HXX

#include "SoarPlayerBot.h"

#include "InputLinkManager.h"
#include "OutputLinkManager.h"

#include <exception>

void SoarPlayerBot::reload_productions()
{
	if ( !m_agent.LoadProductions( m_productions.c_str() ) )
	{
		std::cerr << "error loading productions: " << m_agent.GetLastErrorDescription() << std::endl;
		throw new std::exception();
	}
}

void SoarPlayerBot::reset()
{
	m_agent.InitSoar();
}

void SoarPlayerBot::clear_io_links()
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

void SoarPlayerBot::create_io_links()
{
	m_input_link = new InputLinkManager( m_agent );
	m_output_link = new OutputLinkManager( m_agent );
}

#endif // SOAR_PLAYER_BOT_HXX
