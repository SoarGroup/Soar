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

ipcsocket::ipcsocket() : conn(false) {}

ipcsocket::~ipcsocket() {
	if (conn) {
		disconnect();
	}
	close(listenfd);
}

bool ipcsocket::accept(const string &socketfile, bool blocklisten) {
	socklen_t len;
	struct sockaddr_un addr, remote;
	
	if (listenfd < 0) {
		bzero((char *) &addr, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, socketfile.c_str());
		len = strlen(addr.sun_path) + sizeof(addr.sun_family);
		
		unlink(addr.sun_path);
		
		if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			perror("ipcsocket::ipcsocket");
			exit(1);
		}
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
	}
	
	len = sizeof(struct sockaddr_un);
	if ((fd = ::accept(listenfd, (struct sockaddr *) &remote, &len)) == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("ipcsocket::accept");
			exit(1);
		}
		return false;
	}
	conn = true;
	return true;
}

bool ipcsocket::connect(const string &path) {
	socklen_t len;
	struct sockaddr_un addr;
	
	bzero((char *) &addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path.c_str());
	len = strlen(addr.sun_path) + sizeof(addr.sun_family) + 1;
	
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("ipcsocket::ipcsocket");
		exit(1);
	}
		
	if (::connect(fd, (struct sockaddr *) &addr, len) == -1) {
		return false;
	}

	conn = true;
	return true;
}

void ipcsocket::disconnect() {
	close(fd);
	conn = false;
}

bool ipcsocket::send(const string &s) {
	int n;
	
	if (!conn) return false;
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

bool ipcsocket::send_line(const string &line) {
	int n;
	
	if (!conn) return false;
	const char *p = line.c_str();
	
	while (*p) {
		if ((n = ::send(fd, p, strlen(p), 0)) <= 0) {
			if (errno != EINTR) {
				disconnect();
				return false;
			}
		} else {
			p += n;
		}
	}
	return true;
}

bool ipcsocket::receive(string &msg) {
	char buf[BUFFERSIZE+1];
	size_t p, n;
	
	if (!conn) return false;
	
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
