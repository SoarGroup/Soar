#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

int init_socket(char *port);
int init_file(char *path);

int read_socket(char *buf, int n);
int read_file(char *buf, int n);
void socket_error(char *msg);

enum Input_type { FILE_INPUT, SOCKET_INPUT };

static enum Input_type input_type;
static FILE *file = NULL;

/* persistent vars for socket */
static fd_set all_fds;
static int listen_fd, max_fd;

int init_input(int argc, char *argv[]) {
	int i;
	
	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-s") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "specify a socket path or port\n");
				return 0;
			}
			input_type = SOCKET_INPUT;
			return init_socket(argv[i + 1]);
		} else if (strcmp(argv[i], "-f") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "specify file path\n");
				return 0;
			}
			input_type = FILE_INPUT;
			return init_file(argv[i + 1]);
		}
	}
	file = stdin;
	input_type = FILE_INPUT;
	return 1;
}

int get_input(char *buf, int n) {
	switch (input_type) {
		case FILE_INPUT:
			return read_file(buf, n);
		case SOCKET_INPUT:
			return read_socket(buf, n);
	}
	return 0;
}

int read_file(char *buf, int n) {
	if (!fgets(buf, n, file)) {
		if (feof(file)) {
			fprintf(stderr, "EOF\n");
		} else if (ferror(file)) {
			perror("get_input: ");
		}
		return -1;
	}
	return strlen(buf);
}

void winsock_cleanup(void) {
	WSACleanup();
}

int init_socket(char *port) {
	struct addrinfo hints, *name = NULL;
	WSADATA wsaData;
	
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
		fprintf(stderr, "Winsock initialization failed\n");
		exit(1);
	}
	atexit(winsock_cleanup);

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		fprintf(stderr, "socket() failed\n");
		exit(1);
	}
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo("localhost", port, &hints, &name) != 0) {
		fprintf(stderr, "getaddrinfo failed\n");
		exit(1);
	}
	
	if (bind(listen_fd, name->ai_addr, (int)name->ai_addrlen) == SOCKET_ERROR) {
		fprintf(stderr, "bind failed\n");
		exit(1);
	}
	
	if (listen(listen_fd, 10) == SOCKET_ERROR) {
		fprintf(stderr, "listen failed\n");
		exit(1);
    }
    
	FD_ZERO(&all_fds);
	FD_SET(listen_fd, &all_fds);
	max_fd = listen_fd;
	return 1;
}

int read_socket(char *buf, int n) {
	fd_set select_fds;
	int n_active, active_fd, new_fd, nrecv;
	struct sockaddr client_addr;
    int addr_len;
    
	for (;;) {
		select_fds = all_fds;
		n_active = select(max_fd + 1, &select_fds, NULL, NULL, NULL);
		if (n_active == SOCKET_ERROR) {
			fprintf(stderr, "select failed\n");
			exit(1);
		} else if (n_active == 0) {
			continue;
		}
		for (active_fd = 0; active_fd <= max_fd; ++active_fd) {
			if (!FD_ISSET(active_fd, &select_fds)) {
				continue;
			}
			if (active_fd == listen_fd) {
				addr_len = sizeof(client_addr);
				if ((new_fd = accept(listen_fd, &client_addr, (socklen_t*) &addr_len)) == INVALID_SOCKET) {
					socket_error("accept failed");
				}
				fprintf(stderr, "client connect\n");
				FD_SET(new_fd, &all_fds);
				if (new_fd > max_fd) {
					max_fd = new_fd;
				}
			} else {
				if ((nrecv = recv(active_fd, buf, n, 0)) == SOCKET_ERROR) {
					fprintf(stderr, "recv failed: %d\n", WSAGetLastError());
					/* disconnect */
					closesocket(active_fd);
					FD_CLR(active_fd, &all_fds);
					return 0;
				}
				return nrecv;
			}
		}
	}
}

int init_file(char *path) {
	if ((file = fopen(path, "r")) == NULL)
		return 0;
	return 1;
}

void socket_error(char *msg) {
	fprintf(stderr, "%s: %d\n", msg, WSAGetLastError());
	exit(1);
}

int run_shell(const char *cmd) {
	return system(cmd);
}

char *get_temp(const char *prefix) {
	static char tempdir[MAX_PATH] = {'\0'};
	char *path;
	
	if (strlen(tempdir) == 0) {
		if (GetTempPath(MAX_PATH, tempdir) == 0) {
			fprintf(stderr, "can't get temporary directory\n");
			exit(1);
		}
	}
	
	path = (char *) malloc(MAX_PATH);
	GetTempFileName(tempdir, prefix, 0, path);
	return path;
}

void delete_file(const char *path) {
	DeleteFile(path);
}
