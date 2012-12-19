#ifndef IPCSOCKET_H
#define IPCSOCKET_H

#include <string>
#include <sys/un.h>

class ipcsocket {
public:
	ipcsocket();
	~ipcsocket();
	
	void set_address(const std::string &path);
	bool send(const std::string &s) const;
	
private:
	struct sockaddr_un addr;
	int fd;
};

#endif
