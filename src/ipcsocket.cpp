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

ipcsocket::ipcsocket() {
	if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		perror("ipcsocket::ipcsocket");
		exit(1);
	}
}

ipcsocket::~ipcsocket() {
	close(fd);
}

void ipcsocket::set_address(const string &path) {
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path.c_str());
}

bool ipcsocket::send(const string &s) const {
	int n;
	
	const char *p = s.c_str();
	
	while (*p) {
		if ((n = sendto(fd, p, strlen(p), 0, (struct sockaddr*) &addr, sizeof(addr))) <= 0) {
			if (errno != EINTR)
				return false;
		} else {
			p += n;
		}
	}
	return true;
}
