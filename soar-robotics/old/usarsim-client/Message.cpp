#include "Message.hxx"

using std::string;
using std::list;
using std::pair;

int Message::s_next_id = 0;

Message::Message( const string& from, const list< pair< string, string > >& message )
: m_from( from )
, m_id( s_next_id++ )
, m_message( message )
{
}

