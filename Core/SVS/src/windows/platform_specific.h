#ifndef PLATFORM_SPECIFIC
#define PLATFORM_SPECIFIC

#include <string>
#include <float.h>
#include <limits>
#include <winsock2.h>

#undef min
#undef max


// socket
#define SOCK SOCKET
#define INVALID_SOCK INVALID_SOCKET

SOCK get_tcp_socket(const std::string &port_or_path);
bool tcp_send(SOCK sock, const std::string &s);
void close_tcp_socket(SOCK sock);

/* defined in C99 but not in MSVC */

inline double nextafter(double x, double y) {
	return _nextafter(x, y);
}

#endif
