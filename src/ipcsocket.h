#ifndef IPCSOCKET_H
#define IPCSOCKET_H

#include <string>
#include <list>

class ipcsocket {
public:
	ipcsocket();
	~ipcsocket();
	
	bool send(const std::string &s);
	bool send_line(const std::string &line);
	bool receive(std::string &msg);
	
	bool accept(const std::string &path, bool blocklisten=false);
	bool connect(const std::string &path);
	
	bool connected() const { return conn; }
	void disconnect();
	
private:
	std::string recvbuf;
	int listenfd, fd;
	bool conn;
	
	/*
	 Is the incoming connection expecting to perform a send (hence
	 you're receiving) as its first action? This needs to be specified
	 to prevent deadlock.
	*/
	bool recvfirst;
	
};

#endif
