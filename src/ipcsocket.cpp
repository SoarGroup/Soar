#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <sstream>
#include <string>

#include "ipcsocket.h"
#include "common.h"

using namespace std;

int get_socket(const string &port_or_path) {
	int family, fd, port, name_size;
	sockaddr_in in_name;
	sockaddr_un un_name;
	sockaddr *name;
	
	if (parse_int(port_or_path, port)) {
		memset(&in_name, 0, sizeof(in_name));
		family = in_name.sin_family = AF_INET;
		in_name.sin_port = htons(port);
		in_name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		name = reinterpret_cast<sockaddr*>(&in_name);
		name_size = sizeof(in_name);
	} else {
		memset(&un_name, 0, sizeof(un_name));
		family = un_name.sun_family = AF_UNIX;
		strncpy(un_name.sun_path, port_or_path.c_str(), sizeof(un_name.sun_path));
		name = reinterpret_cast<sockaddr*>(&un_name);
		name_size = sizeof(un_name);
	}
	if ((fd = socket(family, SOCK_STREAM, 0)) == -1) {
		perror("get_socket");
		exit(1);
	}
	if (connect(fd, name, name_size) == -1) {
		perror("get_socket");
		close(fd);
		return -1;
	}
	return fd;
}

ipcsocket::ipcsocket() : fd(-1) {}

ipcsocket::~ipcsocket() {
	if (fd >= 0) {
		close(fd);
	}
}

bool ipcsocket::connect(const string &path) {
	if (fd >= 0) {
		close(fd);
	}
	fd = get_socket(path);
	return (fd >= 0);
}

void ipcsocket::disconnect() {
	close(fd);
	fd = -1;
}

bool ipcsocket::send(const string &s) {
	if (fd < 0) {
		return false;
	}
	
	int n;
	const char *p = s.c_str();
	
	while (*p) {
		if ((n = ::send(fd, p, strlen(p), 0)) <= 0) {
			if (errno != EINTR) {
				perror("ipcsocket::send");
				close(fd);
				fd = -1;
				return false;
			}
		} else {
			p += n;
		}
	}
	return true;
}
