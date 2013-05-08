#define WIN32_LEAN_AND_MEAN

#include "platform_specific.h"
#include <windows.h>
#include <iostream>
#include <ws2tcpip.h>
#include "timer.h"

using namespace std;

// copied from http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
LARGE_INTEGER getFILETIMEoffset() {
	SYSTEMTIME s;
	FILETIME f;
	LARGE_INTEGER t;

	s.wYear = 1970;
	s.wMonth = 1;
	s.wDay = 1;
	s.wHour = 0;
	s.wMinute = 0;
	s.wSecond = 0;
	s.wMilliseconds = 0;
	SystemTimeToFileTime(&s, &f);
	t.QuadPart = f.dwHighDateTime;
	t.QuadPart <<= 32;
	t.QuadPart |= f.dwLowDateTime;
	return (t);
}

// adapted from from http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
long get_time_nanosecs() {
	LARGE_INTEGER t;
	double microseconds;
	static LARGE_INTEGER offset;
	static double counts_per_nanosec;
	static int initialized = 0;
	static BOOL use_performance_counter = 0;

	if (!initialized) {
		LARGE_INTEGER freq;
		initialized = 1;
		use_performance_counter = QueryPerformanceFrequency(&freq);
		if (use_performance_counter) {
			QueryPerformanceCounter(&offset);
			counts_per_nanosec = freq.QuadPart / 1e9;
		} else {
			offset = getFILETIMEoffset();
			counts_per_nanosec = .01;     // 10 per microsec
		}
	}
	if (use_performance_counter) {
		QueryPerformanceCounter(&t);
	} else {
		FILETIME f;
		GetSystemTimeAsFileTime(&f);
		t.QuadPart = f.dwHighDateTime;
		t.QuadPart <<= 32;
		t.QuadPart |= f.dwLowDateTime;
	}

	t.QuadPart -= offset.QuadPart;
	return t.QuadPart / counts_per_nanosec;
}

void winsock_cleanup() {
	WSACleanup();
}

SOCKET get_tcp_socket(const string &port) {
	SOCKET sock = INVALID_SOCKET;
	addrinfo hints, *name = NULL;
	
	static bool winsock_initialized = false;
	if (!winsock_initialized) {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
			cerr << "Winsock initialization failed" << endl;
			exit(1);
		}
		atexit(winsock_cleanup);
		winsock_initialized = true;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		cerr << "socket() failed" << endl;
		return INVALID_SOCKET;
	}
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo("localhost", port.c_str(), &hints, &name) != 0) {
		cerr << "getaddrinfo failed" << endl;
		return INVALID_SOCKET;
	}
	
	if (connect(sock, name->ai_addr, static_cast<int>(name->ai_addrlen)) == SOCKET_ERROR) {
		cerr << "couldn't connect" << endl;
		closesocket(sock);
		freeaddrinfo(name);
		return INVALID_SOCKET;
	}
	return sock;
}

bool tcp_send(SOCK sock, const string &s) {
	if (sock == INVALID_SOCKET) {
		return false;
	}
	
	int n;
	const char *p = s.c_str();
	
	while (*p) {
		if ((n = send(sock, p, strlen(p), 0)) == SOCKET_ERROR) {
			if (errno != EINTR) {
				cerr << "tcp_send failed: " << WSAGetLastError() << endl;
				closesocket(sock);
				return false;
			}
		} else {
			p += n;
		}
	}
	return true;
}

void close_tcp_socket(SOCK sock) {
	closesocket(sock);
}
