#ifndef SOAR_PLAYER_CLIENT_H
#define SOAR_PLAYER_CLIENT_H

#include <libplayerc++/playerc++.h>

class Sedan
{
public:
	Sedan( int port );
	void run();

	struct sedan_exception : public std::exception
	{
		const char* message;
		sedan_exception( const char* error_message )
		: message( error_message ) {}
		virtual const char* what() { return message; }
	};
	
	static bool stop;

private:
	PlayerCc::PlayerClient	  m_robot;
	PlayerCc::Position2dProxy m_write;	
	PlayerCc::Position2dProxy m_read;	
	
};

#endif // SOAR_PLAYER_CLIENT_H
