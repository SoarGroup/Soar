#ifndef OUTPUT_LINK_MANAGER_HXX
#define OUTPUT_LINK_MANAGER_HXX

#include "OutputLinkManager.h"

#include <exception>
#include <string>
#include <sstream>
#include <iostream>

void OutputLinkManager::read()
{
	for ( int index = 0; index < m_agent.GetNumberCommands(); ++index )
	{
		try
		{
			std::pair< Command, sml::Identifier* > data;
			data.second = m_agent.GetCommand( index );
		
			Command command( data.second );
			data.first = command;
			
			m_command_list.push_back( data );
		}
		catch ( ... )
		{
			m_agent.GetCommand( index )->AddStatusError();
			std::cerr << "bad command" << std::endl;
		}
	}
	
	m_command_iter = m_command_list.begin();
}

Command* OutputLinkManager::get_next_command()
{
	++m_command_iter;
	if ( m_command_iter == m_command_list.end() )
	{
		return 0;
	}
	return &( m_command_iter->first );
}

void OutputLinkManager::commit()
{
	m_command_iter = m_command_list.begin();
	while ( m_command_iter != m_command_list.end() )
	{
		switch ( m_command_iter->first.get_status() )
		{
		case Command::STATUS_NONE:
			break;
		case Command::STATUS_COMPLETE:
			m_command_iter->second->AddStatusComplete();
			break;
		case Command::STATUS_ERROR:
			m_command_iter->second->AddStatusError();
			break;
		case Command::STATUS_EXECUTING:
			// TODO: m_command_iter->second->AddStatusExecuting();
			break;
		}
	
		m_command_list.erase( m_command_iter );
		m_command_iter = m_command_list.begin();
	}
	m_agent.Commit();
}

#endif // OUTPUT_LINK_MANAGER_HXX

