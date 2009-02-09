#ifndef MESSAGE_HXX
#define MESSAGE_HXX

#include "Message.h"
#include <cassert>

std::string Message::from() const
{
	return m_from;
}

int Message::id() const
{
	return m_id;
}

std::list< std::string >::const_iterator Message::begin() const
{
	return m_message.begin();
}

std::list< std::string >::const_iterator Message::end() const
{
	return m_message.end();
}

#endif // MESSAGE_HXX

