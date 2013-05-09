#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "viewer.h"

int init_socket(char *path);
int init_file(char *path);

int read_stdin(char *buf, int n);
int read_socket(char *buf, int n);
int read_file(char *buf, int n);

int parse_int(char *s, int *v);

enum Input_type { REGULAR_FILE, SOCKET };

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
			input_type = SOCKET;
			return init_socket(argv[i + 1]);
		} else if (strcmp(argv[i], "-f") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "specify file path\n");
				return 0;
			}
			input_type = REGULAR_FILE;
			return init_file(argv[i + 1]);
		}
	}
	file = stdin;
	input_type = REGULAR_FILE;
	return 1;
}

int get_input(char *buf, int n) {
	switch (input_type) {
		case REGULAR_FILE:
			return read_file(buf, n);
		case SOCKET:
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

int parse_int(char *s, int *v) {
	char *end;
	*v = strtol(s, &end, 10);
	return *s != '\0' && *end == '\0';
}

int init_socket(char *path_or_port) {
	int family, port, yes;
	struct sockaddr_in in_name;
	struct sockaddr_un un_name;
	struct sockaddr *name;
	socklen_t name_size;
	
	if (parse_int(path_or_port, &port)) {
		memset(&in_name, 0, sizeof(in_name));
		family = in_name.sin_family = AF_INET;
		in_name.sin_port = htons(port);
		in_name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		name = (struct sockaddr*) &in_name;
		name_size = sizeof(in_name);
	} else {
		memset(&un_name, 0, sizeof(un_name));
		family = un_name.sun_family = AF_UNIX;
		strncpy(un_name.sun_path, path_or_port, sizeof(un_name.sun_path));
		name = (struct sockaddr*) &un_name;
		name_size = sizeof(un_name);
	}
	if ((listen_fd = socket(family, SOCK_STREAM, 0)) == -1) {
		perror("init_socket");
		exit(1);
	}
	
	/* gets rid of "address in use" errors */
	yes = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (bind(listen_fd, name, name_size) == -1) {
		perror("init_socket");
		exit(1);
	}
	if (listen(listen_fd, 10) == -1) {
		perror("init_socket");
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
	struct sockaddr_storage client_addr;
    socklen_t addr_len;
    
	for (;;) {
		select_fds = all_fds;
		n_active = select(max_fd + 1, &select_fds, NULL, NULL, NULL);
		if (n_active == -1) {
			perror("read_socket");
			exit(1);
		} else if (n_active == 0) {
			continue;
		}
		for (active_fd = 0; active_fd <= max_fd; ++active_fd) {
			if (!FD_ISSET(active_fd, &select_fds)) {
				continue;
			}
			if (active_fd == listen_fd) {
				if ((new_fd = accept(listen_fd, (struct sockaddr*) &client_addr, (socklen_t*) &addr_len)) == -1) {
					perror("read_socket");
					exit(1);
				}
				fprintf(stderr, "client connect\n");
				FD_SET(new_fd, &all_fds);
				if (new_fd > max_fd) {
					max_fd = new_fd;
				}
			} else {
				if ((nrecv = recv(active_fd, buf, n, 0)) == -1) {
					fprintf(stderr, "here\n");
					perror("read_socket");
					exit(1);
				}
				if (nrecv == 0) {
					/* disconnect */
					fprintf(stderr, "client disconnected\n");
					close(active_fd);
					FD_CLR(active_fd, &all_fds);
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

void delay() {
	static struct timespec interval = { .tv_sec = 0, .tv_nsec = 1000 };
	nanosleep(&interval, NULL);
}
