#ifndef PLATFORM_SPECIFIC
#define PLATFORM_SPECIFIC
#include <math.h>
#include <string>

// socket
#define SOCK int
#define INVALID_SOCK -1

SOCK get_tcp_socket(const std::string &port_or_path);
bool tcp_send(SOCK sock, const std::string &s);
void close_tcp_socket(SOCK sock);

#endif
