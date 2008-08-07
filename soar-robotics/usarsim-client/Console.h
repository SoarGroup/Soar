#ifndef CONSOLE_H
#define CONSOLE_H

#include "SoarRunThreadInterface.h"

#include <map>

class Console
{
public:
	Console( SoarRunThreadInterface& client, std::string sedan_host, int sedan_port );
	~Console();

	int run();

private:
	SoarRunThreadInterface& m_client;
	const std::string& m_sedan_host;
	int m_sedan_port;
	bool m_sedan_connected;
};

#endif // CONSOLE_H

