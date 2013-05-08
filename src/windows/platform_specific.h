#ifndef PLATFORM_SPECIFIC
#define PLATFORM_SPECIFIC

#include <string>
#include <float.h>
#include <limits>
#include <winsock2.h>

#undef min
#undef max

#define NATURAL_LOG_2 0.69314718055994530941723212145818

// socket
#define SOCK SOCKET
#define INVALID_SOCK INVALID_SOCKET

SOCK get_tcp_socket(const std::string &port_or_path);
bool tcp_send(SOCK sock, const std::string &s);
void close_tcp_socket(SOCK sock);

// timing 

long get_time_nanosecs();

/* some functions from C99 that's not in MSVC */
inline double log2(double x) {
	return log(x) / NATURAL_LOG_2;
}

inline double nextafter(double x, double y) {
	return _nextafter(x, y);
}

#endif
