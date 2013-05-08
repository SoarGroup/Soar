#ifndef IPCSOCKET_H
#define IPCSOCKET_H

#include <string>
#include "platform_specific.h"

class ipcsocket {
public:
	ipcsocket();
	~ipcsocket();
	
	bool connect(const std::string &path);
	void disconnect();
	bool send(const std::string &s);
	
private:
	SOCK sock;
};

#endif
