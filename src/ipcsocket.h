#ifndef IPCSOCKET_H
#define IPCSOCKET_H

#include <string>
#include <list>

class ipcsocket;

class ipc_listener {
public:
	virtual void ipc_connect(ipcsocket *sock) = 0;
	virtual void ipc_disconnect(ipcsocket *sock) = 0;
};

class ipcsocket {
public:
	ipcsocket(char role, std::string socketfile, bool recvfirst, bool blocklisten=false);
	~ipcsocket();
	
	bool send(const std::string &s);
	bool receive(std::string &msg);
	
	void listen(ipc_listener *l);
	void unlisten(ipc_listener *l);
	
private:
	bool accept();
	void disconnect();
	
	std::string recvbuf;
	int listenfd, fd;
	bool connected;
	
	/*
	 Is the incoming connection expecting to perform a send (hence
	 you're receiving) as its first action? This needs to be specified
	 to prevent deadlock.
	*/
	bool recvfirst;
	
	std::list<ipc_listener*> listeners;
	char role;
};

#endif
