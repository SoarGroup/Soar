#ifndef IPCSOCKET_H
#define IPCSOCKET_H

#include <string>

class ipcsocket {
public:
	ipcsocket();
	~ipcsocket();
	
	bool send(const std::string &s);
	
	bool connect(const std::string &path);
	
	bool connected() const { return conn; }
	void disconnect();
	
private:
	int fd;
	bool conn;
};

#endif
