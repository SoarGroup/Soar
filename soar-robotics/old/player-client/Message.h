#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <sstream>
#include <string>
#include <list>

class Message
{
public:
	Message( const std::string& from, const std::list< std::string > message );

	std::string from() const;
	int id() const;
	std::list< std::string >::const_iterator begin() const;
	std::list< std::string >::const_iterator end() const;
	
	friend std::ostream& operator<<( std::ostream& os, const Message& message )
	{
		os << "(" << message.m_from << ", " << message.m_id << ":";
		for ( std::list< std::string >::const_iterator iter = message.begin(); iter != message.end(); ++iter )
		{
			os << " " << *iter;
		}
		os << ")";
		return os;
	}
	
private:
	const std::string m_from;
	const int m_id;
	const std::list< std::string > m_message;
	
	static int s_next_id;
};

#endif // MESSAGE_H

