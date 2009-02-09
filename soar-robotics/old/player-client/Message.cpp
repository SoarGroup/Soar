#include "Message.hxx"

using std::string;
using std::list;

int Message::s_next_id = 0;

Message::Message( const string& from, const list< string > message )
: m_from( from )
, m_id( s_next_id++ )
, m_message( message )
{
}

