#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>

#include "ipcsocket.h"
#include "linalg.h"

using namespace std;

const char TERMSTRING[] = "\n***\n";
const int BUFFERSIZE = 10240;

ipcsocket::ipcsocket(char role, string socketfile, bool recvfirst, bool blocklisten) 
: recvbuf(), recvfirst(recvfirst), role(role)
{
	socklen_t len;
	struct sockaddr_un addr;
	
	bzero((char *) &addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, socketfile.c_str());
	len = strlen(addr.sun_path) + sizeof(addr.sun_family);
	
	if (role == 's') {
		if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			perror("ipcsocket::ipcsocket");
			exit(1);
		}
		
		unlink(addr.sun_path);
		if (bind(listenfd, (struct sockaddr *) &addr, len) == -1) {
			perror("ipcsocket::ipcsocket");
			exit(1);
		}
	
		if (::listen(listenfd, 1) == -1) {
			perror("ipcsocket::ipcsocket");
			exit(1);
		}
		if (!blocklisten) {
			fcntl(listenfd, F_SETFL, O_NONBLOCK);
		}
		connected = false;
	} else {
		if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			perror("ipcsocket::ipcsocket");
			exit(1);
		}
		
		while (connect(fd, (struct sockaddr *) &addr, len) == -1) {
			sleep(1);
		}
		connected = true;
	}
}

ipcsocket::~ipcsocket() {
	if (connected) {
		disconnect();
	}
	close(listenfd);
}

bool ipcsocket::accept() {
	socklen_t len;
	struct sockaddr_un remote;
	list<ipc_listener*>::iterator i;
	
	if (role != 's') {
		return false;
	}
	len = sizeof(struct sockaddr_un);
	if ((fd = ::accept(listenfd, (struct sockaddr *) &remote, &len)) == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("ipcsocket::accept");
			exit(1);
		}
		return false;
	}
	connected = true;
	
	for (i = listeners.begin(); i != listeners.end(); ++i) {
		(**i).ipc_connect(this);
	}
	return true;
}

void ipcsocket::disconnect() {
	list<ipc_listener*>::iterator i;
	
	close(fd);
	connected = false;
	for (i = listeners.begin(); i != listeners.end(); ++i) {
		(**i).ipc_disconnect(this);
	}
}

bool ipcsocket::send(const string &s) {
	int n;
	
	if (!connected && (recvfirst || !accept())) return false;
	string t = s + TERMSTRING;
	
	while (t.size() > 0) {
		if ((n = ::send(fd, t.c_str(), t.size(), 0)) <= 0) {
			if (errno != EINTR) {
				disconnect();
				return false;
			}
		} else {
			t.erase(0, n);
		}
	}
	return true;
}

bool ipcsocket::receive(string &msg) {
	char buf[BUFFERSIZE+1];
	size_t p, n;
	
	if (!connected && (!recvfirst || !accept())) return false;
	
	while(true) {
		if (recvbuf.find(TERMSTRING+1) == 0) { // +1 to skip initial \n
			// empty message (first line is ***)
			msg = "";
			recvbuf.erase(0, strlen(TERMSTRING) - 1);
			return true;
		}
		if ((p = recvbuf.find(TERMSTRING)) != string::npos) {
			msg.assign(recvbuf.substr(0, p));
			recvbuf.erase(0, p+strlen(TERMSTRING));
			return true;
		}
		
		if ((n = recv(fd, buf, BUFFERSIZE, 0)) <= 0) {
			if (errno != EINTR) {
				disconnect();
				return false;
			}
		} else {
			buf[n] = '\0';
			recvbuf += buf;
		}
	}
}

void ipcsocket::listen(ipc_listener *l) {
	listeners.push_back(l);
}

void ipcsocket::unlisten(ipc_listener *l) {
	listeners.remove(l);
}
