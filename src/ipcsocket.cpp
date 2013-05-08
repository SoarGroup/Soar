#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "ipcsocket.h"
#include "common.h"

using namespace std;

ipcsocket::ipcsocket() : sock(INVALID_SOCK) {}

ipcsocket::~ipcsocket() {
	if (sock != INVALID_SOCK) {
		close_tcp_socket(sock);
	}
}

bool ipcsocket::connect(const string &path) {
	if (sock != INVALID_SOCK) {
		close_tcp_socket(sock);
	}
	sock = get_tcp_socket(path);
	return (sock != INVALID_SOCK);
}

void ipcsocket::disconnect() {
	close_tcp_socket(sock);
	sock = INVALID_SOCK;
}

bool ipcsocket::send(const string &s) {
	if (sock == INVALID_SOCK) {
		return false;
	}
	return tcp_send(sock, s);
}
