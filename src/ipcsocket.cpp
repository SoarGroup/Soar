#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <sstream>
#include <string>

#include "ipcsocket.h"

using namespace std;

ipcsocket::ipcsocket() : conn(false) {}

ipcsocket::~ipcsocket() {
	if (conn) {
		disconnect();
	}
}

bool ipcsocket::connect(const string &path) {
	socklen_t len;
	struct sockaddr_un addr;
	
	memset(&addr, 0, sizeof(addr));
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
	const char *p = s.c_str();
	
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
